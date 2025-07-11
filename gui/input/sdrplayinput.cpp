/*
 * This file is part of the AbracaDABra project
 *
 * MIT License
 *
 * Copyright (c) 2019-2025 Petr Kopecký <xkejpi (at) gmail (dot) com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sdrplayinput.h"

#include <QDebug>
#include <QDir>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(sdrPlayInput, "SDRPlayInput", QtInfoMsg)

InputDeviceList SdrPlayInput::getDeviceList()
{
    InputDeviceList list;

    SoapySDR::KwargsList devs = SoapySDR::Device::enumerate("driver=sdrplay");
    if (devs.size() == 0)
    {
        qCInfo(sdrPlayInput) << "No devices found";
        return list;
    }
    else
    {
        qCInfo(sdrPlayInput) << "Found" << devs.size() << "devices.";
    }
    for (int n = 0; n < devs.size(); ++n)
    {
        auto it = devs[n].find(std::string("label"));
        if (it != devs[n].end())
        {
            list.append({.diplayName = QString::fromStdString(it->second), .id = QVariant(QString::fromStdString(devs[n]["serial"]))});
            // list.append({.diplayName = "Test1", .id = QVariant(QString::fromStdString(devs[n]["serial"]))});
            // list.append({.diplayName = "Test2", .id = QVariant(QString::fromStdString(devs[n]["serial"]))});
        }
    }
    return list;
}

int SdrPlayInput::getNumRxChannels(const QVariant &hwId)
{
    size_t numChannels = 0;
    try
    {
        QString argStr = QString("driver=sdrplay,serial=%1").arg(hwId.toString());
        auto device = SoapySDR::Device::make(argStr.toStdString());
        numChannels = device->getNumChannels(SOAPY_SDR_RX);
        SoapySDR::Device::unmake(device);
    }
    catch (const std::exception &ex)
    {
        qCCritical(sdrPlayInput) << "Error probing device: " << ex.what();
    }

    // qDebug() << "-----------" << hwId << numChannels;

    return numChannels;
}

QStringList SdrPlayInput::getRxAntennas(const QVariant &hwId, const int channel)
{
    QStringList antList;
    try
    {
        QString argStr = QString("driver=sdrplay,serial=%1").arg(hwId.toString());
        auto device = SoapySDR::Device::make(argStr.toStdString());
        auto ant = device->listAntennas(SOAPY_SDR_RX, channel);
        for (auto it = ant.cbegin(); it != ant.cend(); ++it)
        {
            antList.append(QString::fromStdString(*it));
        }
        SoapySDR::Device::unmake(device);
    }
    catch (const std::exception &ex)
    {
        qCWarning(sdrPlayInput) << "Error probing device: " << ex.what();
    }
    return antList;
}

SdrPlayInput::SdrPlayInput(QObject *parent)
    : SoapySdrInput(parent),
      m_rfGainMap{
          {"RSP1", {-43, -19, -24, 0}},
          {"RSP1A", {-62, -57, -38, -32, -26, -20, -18, -12, -6, 0}},
          {"RSP1B", {-62, -57, -38, -32, -26, -20, -18, -12, -6, 0}},
          {"RSP2", {-64, -45, -39, -34, -24, -21, -15, -10, 0}},
          {"RSPduo", {-62, -57, -38, -32, -26, -20, -18, -12, -6, 0}},
          {"RSPdx",
           {-84, -81, -78, -75, -72, -69, -66, -63, -60, -57, -54, -51, -48, -45, -42, -39, -36, -33, -30, -27, -24, -18, -15, -12, -9, -6, -3, 0}},
          {"RSPdx-R2",
           {-84, -81, -78, -75, -72, -69, -66, -63, -60, -57, -54, -51, -48, -45, -42, -39, -36, -33, -30, -27, -24, -18, -15, -12, -9, -6, -3, 0}},
      }
{
    m_devArgs = "driver=sdrplay,rfnotch_ctrl=true,dabnotch_ctrl=false";
    m_biasT = false;
}

bool SdrPlayInput::openDevice(const QVariant &hwId, bool fallbackConnection)
{
    // find all SDRplay devices
    const auto list = SdrPlayInput::getDeviceList();

    bool foundDevice = false;
    if (hwId.isValid() && !hwId.toString().isEmpty())
    {
        // first check if device with given SN is available
        for (auto it = list.cbegin(); it != list.cend(); ++it)
        {
            if (it->id.toString() == hwId.toString())
            {
                foundDevice = true;
                break;
            }
        }
        if (foundDevice == false && fallbackConnection == false)
        {
            qCCritical(sdrPlayInput, "Selected SDRplay device SN %s not found", hwId.toString().toLatin1().data());
            return false;
        }
    }

    bool isConnected = false;
    if (foundDevice)
    {
        SoapySdrInput::setDevArgs(QString("driver=sdrplay,serial=%1,rfnotch_ctrl=true,dabnotch_ctrl=false").arg(hwId.toString()));
        isConnected = SoapySdrInput::openDevice(hwId);
        if (isConnected)
        {
            m_deviceDescription.device.sn = hwId.toString();
            m_hwId = hwId;
        }
        else if (fallbackConnection == false)
        {
            qCCritical(sdrPlayInput, "Unable to open selected SDRplay device: SN %s", hwId.toString().toLatin1().data());
            return false;
        }
    }

    if (!isConnected)
    {  // either SN was not found or connection not successful
        // go through device list and try to connect to first working device
        for (auto it = list.cbegin(); it != list.cend(); ++it)
        {
            if (it->id.isValid() && !it->id.toString().isEmpty())
            {
                SoapySdrInput::setDevArgs(QString("driver=sdrplay,serial=%1,rfnotch_ctrl=true,dabnotch_ctrl=false").arg(it->id.toString()));
                isConnected = SoapySdrInput::openDevice(hwId);
                if (isConnected)
                {
                    m_deviceDescription.device.sn = it->id.toString();
                    m_hwId = it->id;
                    break;
                }
            }
        }
    }

    if (isConnected)
    {
        m_device->setGainMode(SOAPY_SDR_RX, m_rxChannel, false);
        if (m_rfGainMap.contains(m_deviceDescription.device.model))
        {
            m_rfGainList = m_rfGainMap.value(m_deviceDescription.device.model);
            return true;
        }
    }
    return false;
}

void SdrPlayInput::setGainMode(const SdrPlayGainStruct &gain)
{
    // qDebug() << Q_FUNC_INFO << int(gain.mode);
    switch (gain.mode)
    {
        case SdrPlayGainMode::Software:
            if (m_gainMode == gain.mode)
            {  // do nothing -> mode does not change
                break;
            }
            m_gainMode = gain.mode;
            m_device->setGainMode(SOAPY_SDR_RX, m_rxChannel, false);
            resetAgc();
            break;
        case SdrPlayGainMode::Manual:
            m_gainMode = gain.mode;
            setRFGR(m_rfGainList.size() - 1 - gain.rfGain);
            m_ifAgcEna = gain.ifAgcEna;
            if (!m_ifAgcEna)
            {
                setIFGR(-gain.ifGain);
            }
            else
            {
                resetAgc();
            }
            emit agcGain(getRFGain() - m_ifGR);
            break;
    }
    emit rfLevel(NAN, NAN);
}

void SdrPlayInput::setBiasT(bool ena)
{
    if (ena != m_biasT)
    {
        m_device->writeSetting("biasT_ctrl", ena ? "true" : "false");
        m_biasT = ena;
        qCInfo(sdrPlayInput) << "Bias-T" << (ena ? "on" : "off");
    }
}

void SdrPlayInput::setAntenna(const QString &antenna)
{
    SoapySdrInput::setAntenna(antenna);
    if (m_device)
    {
        try
        {
            m_device->setAntenna(SOAPY_SDR_RX, m_rxChannel, m_antenna.toStdString());
            qCInfo(sdrPlayInput) << "Antenna:" << m_device->getAntenna(SOAPY_SDR_RX, m_rxChannel);
        }
        catch (const std::exception &ex)
        {
            qCWarning(sdrPlayInput) << "Failed to set antenna to" << m_antenna << ex.what();
            return;
        }
    }
}

void SdrPlayInput::resetAgc()
{
    if (SdrPlayGainMode::Software == m_gainMode)
    {
        int idx = 0;
        do
        {
            if (m_rfGainList.at(idx) > -40.0)
            {
                break;
            }
        } while (++idx > 0);

        m_rfGR = -1;  // this is to force RF gain settings
        m_ifGR = -1;  // this is to force IF gain settings

        setRFGR(m_rfGainList.size() - 1 - idx);
        setIFGR(40);
        m_agcState = SwAgcState::Converging;
        m_rfGRchangeCntr = 2;
        emit agcGain(getRFGain() - m_ifGR);
    }
    else if (m_ifAgcEna)
    {
        setIFGR(40);
    }
    m_levelEmitCntr = 0;
    emit rfLevel(NAN, NAN);
}

void SdrPlayInput::setRFGR(int rfGR)
{
    if (rfGR < 0)
    {
        rfGR = 0;
    }
    if (rfGR >= m_rfGainList.count())
    {
        rfGR = m_rfGainList.count() - 1;
    }
    if (rfGR != m_rfGR)
    {
        m_rfGR = rfGR;
        try
        {
            m_device->setGain(SOAPY_SDR_RX, m_rxChannel, "RFGR", m_rfGR);
            qCDebug(sdrPlayInput) << "RF gain =" << getRFGain();
            emit gainIdx(m_rfGainList.size() - 1 - m_rfGR);
        }
        catch (const std::exception &ex)
        {
            qCWarning(sdrPlayInput) << "Failed to set RFGR to" << m_rfGR << ex.what();
            return;
        }
    }
}

void SdrPlayInput::setIFGR(int ifGR)
{
    if (ifGR < m_ifGRmin)
    {
        ifGR = m_ifGRmin;
    }
    if (ifGR > m_ifGRmax)
    {
        ifGR = m_ifGRmax;
    }
    if (ifGR != m_ifGR)
    {
        m_ifGR = ifGR;
        try
        {
            m_device->setGain(SOAPY_SDR_RX, m_rxChannel, "IFGR", m_ifGR);
            qCDebug(sdrPlayInput) << "IF gain =" << -m_ifGR;
            emit ifGain(-m_ifGR);
        }
        catch (const std::exception &ex)
        {
            qCWarning(sdrPlayInput) << "Failed to set IFGR to" << m_ifGR << ex.what();
            return;
        }
    }
}

void SdrPlayInput::onAgcLevel(float agcLevel)
{
    m_rfGRchangeCntr = (m_rfGRchangeCntr > 0 ? m_rfGRchangeCntr - 1 : 0);
    if (SdrPlayGainMode::Software == m_gainMode)
    {
        if (m_agcState == SwAgcState::Running)
        {
            if (agcLevel > SDRPLAY_LEVEL_THR_MAX)
            {  // decrease gain
                setIFGR(m_ifGR + 1);
                if (m_ifGR >= SDRPLAY_RFGR_UP_THR)
                {
                    if (m_rfGRchangeCntr <= 0)
                    {
                        m_rfGRchangeCntr = 2;
                        float gain = getRFGain();
                        setRFGR(m_rfGR + 1);
                        setIFGR(m_ifGR - (gain - getRFGain()) - 1);
                    }
                }
            }
            else if (agcLevel < SDRPLAY_LEVEL_THR_MIN)
            {
                setIFGR(m_ifGR - 1);
                if ((m_ifGR < SDRPLAY_RFGR_DOWN_THR) && (m_rfGR > 0))
                {
                    if (m_rfGRchangeCntr <= 0)
                    {
                        m_rfGRchangeCntr = 2;
                        float gain = getRFGain();
                        setRFGR(m_rfGR - 1);
                        setIFGR(m_ifGR + (gain - getRFGain()) + 1);
                    }
                }
            }
            else if (m_rfGRchangeCntr <= 0)
            {  // maintenance
                if (m_ifGR >= SDRPLAY_RFGR_UP_THR)
                {
                    m_rfGRchangeCntr = 4;
                    float gain = getRFGain();
                    setRFGR(m_rfGR + 1);
                    setIFGR(m_ifGR - (gain - getRFGain()));
                }
                else if ((m_ifGR < SDRPLAY_RFGR_DOWN_THR) && (m_rfGR > 0))
                {
                    m_rfGRchangeCntr = 4;
                    float gain = getRFGain();
                    setRFGR(m_rfGR - 1);
                    setIFGR(m_ifGR + (gain - getRFGain()));
                }
            }
        }
        else
        {
            if (m_rfGRchangeCntr > 0)
            {
                return;
            }

            // calculate initial value of RF gain
            float gain = getRFGain() - 10 * std::log10(2 * agcLevel / (SDRPLAY_LEVEL_THR_MAX - SDRPLAY_LEVEL_THR_MIN));
            int idx = 0;
            do
            {
                if (m_rfGainList.at(idx) > gain)
                {  // found
                    break;
                }
                else
                {  // not found
                }
                idx += 1;
            } while (idx < m_rfGainList.count());

            setRFGR(m_rfGainList.size() - 1 - idx);
            m_rfGRchangeCntr = 4;
            m_agcState = SwAgcState::Running;
        }
    }
    else if (m_ifAgcEna)
    {  // IF gain control
        if (agcLevel > SDRPLAY_LEVEL_THR_MAX)
        {  // decrease gain
            setIFGR(m_ifGR + 1);
        }
        else if (agcLevel < SDRPLAY_LEVEL_THR_MIN)
        {
            setIFGR(m_ifGR - 1);
        }
    }

    // qDebug() << agcLevel << -m_rfGainList.at(m_rfGainList.size() - 1 - m_rfGR) << -m_ifGR
    //          << 10 * std::log10(2 * agcLevel / (SDRPLAY_LEVEL_THR_MAX - SDRPLAY_LEVEL_THR_MIN));

    // qDebug() << Q_FUNC_INFO << agcLevel << -m_rfGainList.at(m_rfGainList.size() - 1 - m_rfGR) << m_ifGR << 10 * std::log10(agcLevel)
    //          << m_rfGainList.at(m_rfGainList.size() - 1 - m_rfGR) - m_ifGR
    //          << 10 * std::log10(agcLevel) - m_rfGainList.at(m_rfGainList.size() - 1 - m_rfGR) + m_ifGR - 114;

    if (++m_levelEmitCntr > 4)
    {
        m_levelEmitCntr = 0;
        float gain = 112 + getRFGain() - m_ifGR;
        emit agcGain(gain);
        emit rfLevel(10 * std::log10(agcLevel) - gain, gain);
    }
}
