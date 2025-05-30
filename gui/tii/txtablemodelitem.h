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

#ifndef TXTABLEMODELITEM_H
#define TXTABLEMODELITEM_H

#include <stdint.h>
#include <txdataitem.h>

class TxTableModelItem
{
public:
    TxTableModelItem();
    TxTableModelItem(int8_t mainId, int8_t subId, float level, const QGeoCoordinate &coordinates, const QList<TxDataItem *> txItemList);

    bool hasTxData() const { return m_transmitterData.isValid(); }

    float level() const;
    void setLevel(float newLevel);

    const TxDataItem &transmitterData() const;
    void setTransmitterData(const TxDataItem &newTransmitterData);

    void setTII(int8_t newMainId, int8_t newSubId);
    int8_t mainId() const;
    int8_t subId() const;

    float distance() const;
    void setDistance(float newDistance);

    float azimuth() const;
    void setAzimuth(float newAzimuth);

    // used for scanner
    int id() const;
    void updateGeo(const QGeoCoordinate &coordinates);

    float power() const;

    void setEnsData(const ServiceListId &ensId, const QString &ensLabel, int numServices, float snr);
    void setEnsConfig(const QString &config, const QString &configCSV);
    ServiceListId ensId() const;
    QString ensLabel() const;
    int numServices() const;
    float snr() const;
    QDateTime rxTime() const;
    void setRxTime(const QDateTime &newRxTime);
    QString ensConfig() const;
    QString ensConfigCSV() const;

    bool isActive() const;
    void setInactive();

private:
    int m_id;                      // subId << 8 | mainId (this is unique)
    int8_t m_mainId = -1;          // main ID
    int8_t m_subId = -1;           // sub ID
    float m_level = 0;             // signal level
    float m_distance = -1.0;       // distance of the transmitter from current position (must be >= 0)
    float m_azimuth = -1.0;        // azimuth of the transmitter from current position
    TxDataItem m_transmitterData;  // information about transmitter

    // used for scanner
    ServiceListId m_ensId;   // ensemble id
    QString m_ensLabel;      // ensemble label
    QString m_ensConfig;     // ensemble configuration
    QString m_ensConfigCSV;  // emsemble configuration CSV
    int m_numServices = 0;   // number of services in ensemble
    QDateTime m_rxTime;      // reception time
    float m_snr = 0.0;       // ensemble SNR
};

#endif  // TXTABLEMODELITEM_H
