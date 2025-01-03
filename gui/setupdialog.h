/*
 * This file is part of the AbracaDABra project
 *
 * MIT License
 *
 * Copyright (c) 2019-2024 Petr Kopecký <xkejpi (at) gmail (dot) com>
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

#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QList>
#include <QAbstractButton>
#include <QLocale>
#include <QGeoCoordinate>
#include <QCheckBox>
#include <QLabel>
#include <QNetworkReply>
#include "config.h"
#include "settings.h"
#include "dabtables.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SetupDialog; }
QT_END_NAMESPACE

class SetupDialog : public QDialog
{
    Q_OBJECT
public:
    SetupDialog(QWidget *parent = nullptr);
    void setupDarkMode(bool darkModeEna);
    void setGainValues(const QList<float> & gainList);
    void setInputDeviceEnabled(bool ena);
    void resetInputDevice();
    void setSettings(Settings  * settings);
    void setXmlHeader(const InputDeviceDescription & desc);
    void onFileLength(int msec);
    void onFileProgress(int msec);
    void setAudioRecAutoStop(bool ena);
    void setCheckUpdatesEna(bool ena);
    QLocale::Language applicationLanguage() const;
    void setSlsDumpPaternDefault(const QString &newSlsDumpPaternDefault);
    void setSpiDumpPaternDefault(const QString &newSpiDumpPaternDefault);
    void onTiiUpdateFinished(QNetworkReply::NetworkError err);
    void setDeviceDescription(const InputDeviceDescription & desc);

signals:
    void inputDeviceChanged(const InputDeviceId & inputDevice);
    void newInputDeviceSettings();
    void newAnnouncementSettings();
    void expertModeToggled(bool enabled);
    void trayIconToggled(bool enabled);
    void applicationStyleChanged(Settings::ApplicationStyle style);
    void noiseConcealmentLevelChanged(int level);
    void xmlHeaderToggled(bool enabled);
    void spiApplicationEnabled(bool enabled);
    void spiApplicationSettingsChanged(bool useInterent, bool enaRadioDNS);
    void audioRecordingSettings(const QString &folder, bool doOutputRecording);
    void uaDumpSettings(const Settings::UADumpSettings & settings);
    void tiiSettingsChanged();
    void rawFileSeek(int msec);
    void updateTxDb();
    void proxySettingsChanged();
    void slsBgChanged(const QColor & color);

protected:
    void showEvent(QShowEvent *event);

private:
    enum SetupDialogTabs { Device = 0, Audio, Announcement, UserApps, Tii, Other };
    enum SetupDialogXmlHeader { XMLDate = 0, XMLRecorder, XMLDevice,
                                XMLSampleRate, XMLFreq, XMLLength, XMLFormat,
                                XMLNumLabels};
    enum SetupDialogDeviceInfo { DevInfoDevice, DevInfoTuner, DevInfoSampleFormat, DevInfoLables};


    const QList<QLocale::Language> m_supportedLocalization = { QLocale::Czech, QLocale::German, QLocale::Polish };
    const QString m_noFileString = tr("No file selected");

    Ui::SetupDialog *ui;
    Settings * m_settings;
    QString m_rawfilename;
    QList<float> m_rtlsdrGainList;
    QList<float> m_rtltcpGainList;
    QList<float> m_soapysdrGainList;
    QCheckBox * m_announcementCheckBox[static_cast<int>(DabAnnouncement::Undefined)];
    QCheckBox * m_bringWindowToForegroundCheckbox;
    QLabel * m_xmlHeaderLabel[SetupDialogXmlHeader::XMLNumLabels];
    QLabel * m_rtlSdrLabel[SetupDialogDeviceInfo::DevInfoLables];
    QLabel * m_rtlTcpLabel[SetupDialogDeviceInfo::DevInfoLables];
    QString m_slsDumpPaternDefault;
    QString m_spiDumpPaternDefault;
    QMovie * m_spinner;

    void setUiState();
    void setStatusLabel();

    void onButtonClicked(QAbstractButton *button);
    void onInputChanged(int index);
    void onOpenFileButtonClicked();

    void onConnectDeviceClicked();

    void onRtlSdrGainModeToggled(bool checked);
    void onRtlSdrGainSliderChanged(int val);
    void onRtlSdrBandwidthChanged(int val);
    void onRtlSdrSwAgcMaxLevelChanged(int val);
    void onRtlSdrBiasTCurrentIdxChanged(int);
    void activateRtlSdrControls(bool en);
    void onRtlSdrPPMChanged(int val);

    void onTcpGainModeToggled(bool checked);
    void onRtlTcpGainSliderChanged(int val);
    void onRtlTcpIpAddrEditFinished();
    void onRtlTcpPortValueChanged(int val);
    void onRtlTcpSwAgcMaxLevelChanged(int val);
    void activateRtlTcpControls(bool en);
    void onRtlTcpPPMChanged(int val);

    void onRawFileFormatChanged(int idx);    
    void onAnnouncementClicked();
    void onBringWindowToForegroundClicked(bool checked);

    void onStyleChecked(bool checked);
    void onExpertModeChecked(bool checked);
    void onTrayIconChecked(bool checked);
    void onDLPlusChecked(bool checked);
    void onLanguageChanged(int index);
    void onNoiseLevelChanged(int index);    
    void onXmlHeaderChecked(bool checked);
    void onRawFileProgressChanged(int val);
    void onSpiAppChecked(bool checked);
    void onUseInternetChecked(bool checked);
    void onRadioDnsChecked(bool checked);
    void onAudioRecordingFolderButtonClicked();
    void onAudioRecordingChecked(bool checked);
    void onDataDumpFolderButtonClicked();
    void onDataDumpCheckboxToggled(bool);
    void onDataDumpPatternEditingFinished();
    void onDataDumpResetClicked();
    void onDlRecordingChecked(bool checked);
    void onDlAbsTimeChecked(bool checked);

    void onGeolocationSourceChanged(int index);
    void onCoordinateEditFinished();
    void onSerialPortEditFinished();
    void onTiiSpectPlotClicked(bool checked);
    void onTiiUpdateDbClicked();
    void onTiiLogFolderButtonClicked();

    void onProxyConfigChanged(int index);
    void onProxyApplyButtonClicked();
    void onProxyConfigEdit();

    void onSlsBgButtonClicked();

#if HAVE_AIRSPY
    void onAirspyModeToggled(bool checked);
    void onAirspySensitivityGainSliderChanged(int val);
    void onAirspyIFGainSliderChanged(int val);
    void onAirspyLNAGainSliderChanged(int val);
    void onAirspyMixerGainSliderChanged(int val);
    void onAirspyLNAAGCstateChanged(int state);
    void onAirspyMixerAGCstateChanged(int state);
    void onAirspyBiasTCurrentIdxChanged(int);
    void activateAirspyControls(bool en);
#endif
#if HAVE_SOAPYSDR
    void onSoapySdrGainSliderChanged(int val);
    void onSoapySdrDevArgsEditFinished();
    void onSoapySdrAntennaEditFinished();
    void onSoapySdrChannelEditFinished();
    void onSoapySdrGainModeToggled(bool checked);
    void onSoapySdrBandwidthChanged(int val);
    void activateSoapySdrControls(bool en);
#endif
};

#endif // SETUPDIALOG_H
