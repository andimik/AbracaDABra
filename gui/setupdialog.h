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

#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QAbstractButton>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGeoCoordinate>
#include <QLabel>
#include <QList>
#include <QLocale>
#include <QNetworkReply>
#include <QWidget>

#include "config.h"
#include "dabtables.h"
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class SetupDialog;
}
QT_END_NAMESPACE

class SetupDialog : public QDialog
{
    Q_OBJECT
public:
    SetupDialog(QWidget *parent = nullptr);
    void setupDarkMode(bool darkModeEna);
    void setInputDeviceEnabled(bool ena, InputDevice::Id id = InputDevice::Id::UNDEFINED);
    void setInputDevice(InputDevice::Id id, InputDevice *device);
    void resetInputDevice();
    void setSettings(Settings *settings);
    void onFileLength(int msec);
    void onFileProgress(int msec);
    void setAudioRecAutoStop(bool ena);
    void setCheckUpdatesEna(bool ena);
    QLocale::Language applicationLanguage() const;
    void setSlsDumpPaternDefault(const QString &newSlsDumpPaternDefault);
    void setSpiDumpPaternDefault(const QString &newSpiDumpPaternDefault);
    void onTiiUpdateFinished(QNetworkReply::NetworkError err);

signals:
    void inputDeviceChanged(const InputDevice::Id &inputDevice, QVariant &id);
    void newAnnouncementSettings();
    void expertModeToggled(bool enabled);
    void trayIconToggled(bool enabled);
    void applicationStyleChanged(Settings::ApplicationStyle style);
    void noiseConcealmentLevelChanged(int level);
    void xmlHeaderToggled(bool enabled);
    void spiApplicationEnabled(bool enabled);
    void spiApplicationSettingsChanged(bool useInternet, bool enaRadioDNS);
    void spiIconSettingsChanged();
    void audioRecordingSettings(const QString &folder, bool doOutputRecording);
    void uaDumpSettings(const Settings::UADumpSettings &settings);
    void tiiSettingsChanged();
    void tiiModeChanged(int mode);
    void rawFileSeek(int msec);
    void updateTxDb();
    void proxySettingsChanged();
    void slsBgChanged(const QColor &color);
    void restartRequested();
    void showSystemTimeToggled(bool enabled);
    void showCountryFlagToggled();

protected:
    void showEvent(QShowEvent *event);

private:
    enum SetupDialogTabs
    {
        Device = 0,
        Audio,
        Announcement,
        UserApps,
        Tii,
        Other
    };
    enum SetupDialogXmlHeader
    {
        XMLDate = 0,
        XMLRecorder,
        XMLDevice,
        XMLModel,
        XMLSampleRate,
        XMLFreq,
        XMLLength,
        XMLFormat,
        XMLNumLabels
    };
    // enum SetupDialogDeviceInfo
    // {
    //     DevInfoDevice = 0,
    //     DevInfoTuner,
    //     DevInfoSampleFormat,
    //     DevInfoLables,
    //     DevInfoSoapySdrLables = DevInfoDevice + 1,
    // };
    enum SetupDialogConnectButtonState
    {
        ConnectButtonOn = 0,
        ConnectButtonOff,
        ConnectButtonAuto
    };

    const QList<QLocale::Language> m_supportedLocalization = {QLocale::Czech, QLocale::German, QLocale::Polish};
    const QString m_noFileString = tr("No file selected");

    Ui::SetupDialog *ui;
    Settings *m_settings = nullptr;
    InputDevice::Id m_inputDeviceId = InputDevice::Id::UNDEFINED;
    InputDevice *m_device = nullptr;
    QString m_rawfilename;
    QList<float> m_rtlsdrGainList;
    QList<float> m_rtltcpGainList;
    QCheckBox *m_announcementCheckBox[static_cast<int>(DabAnnouncement::Undefined)];
    QCheckBox *m_bringWindowToForegroundCheckbox;
    QLabel *m_xmlHeaderLabel[SetupDialogXmlHeader::XMLNumLabels];
    QList<QLabel *> m_rtlSdrLabel;
    QList<QLabel *> m_rtlTcpLabel;
#if HAVE_AIRSPY
    QList<QLabel *> m_airspyLabel;
#endif
#if HAVE_SOAPYSDR
    QList<float> m_sdrplayGainList;
    QList<QLabel *> m_soapySdrLabel;
    QList<QLabel *> m_sdrPlayLabel;
#endif
    QString m_slsDumpPaternDefault;
    QString m_spiDumpPaternDefault;
    QMovie *m_spinner;

    void setUiState();
    void connectDeviceControlSignals();
    void setStatusLabel(bool clearLabel = false);
    void setFmlistUploadInfoText();

    void onInputChanged(int index);
    void onOpenFileButtonClicked();

    void onConnectDeviceClicked();

    void setGainValues(const QList<float> &gainList);
    void setDeviceDescription(const InputDevice::Description &desc);
    void reloadDeviceList(const InputDevice::Id inputDeviceId, QComboBox *combo);
    void setConnectButton(SetupDialogConnectButtonState state);

    void onBandwidthChanged(int val);
    void onPPMChanged(int val);
    void onBiasTChanged(int val);
    void onRfLevelOffsetChanged(double val);

    void onRtlSdrGainModeToggled(bool checked);
    void onRtlSdrGainSliderChanged(int val);
    void onRtlSdrSwAgcMaxLevelChanged(int val);
    void activateRtlSdrControls(bool en);

    void onTcpGainModeToggled(bool checked);
    void onRtlTcpGainSliderChanged(int val);
    void onRtlTcpIpAddrEditFinished();
    void onRtlTcpControlSocketChecked(bool checked);
    void onRtlTcpPortValueChanged(int val);
    void onRtlTcpSwAgcMaxLevelChanged(int val);
    void activateRtlTcpControls(bool en);

    void onRawFileFormatChanged(int idx);
    void onAnnouncementClicked();
    void onBringWindowToForegroundClicked(bool checked);

    void onStyleChecked(bool checked);
    void onExpertModeChecked(bool checked);
    void onTrayIconChecked(bool checked);
    void onShowSystemTimeChecked(bool checked);
    void onCountryFlagChecked(bool);
    void onDLPlusChecked(bool checked);
    void onLanguageChanged(int index);
    void onNoiseLevelChanged(int index);
    void onAudioOutChanged(int index);
    void onAudioDecChanged(int index);
    void onXmlHeaderChecked(bool checked);
    void onRawFileProgressChanged(int val);
    void onSpiAppChecked(bool checked);
    void onUseInternetChecked(bool checked);
    void onRadioDnsChecked(bool checked);
    void onSpiIconChecked(bool checked);
    void onSpiIconHideChecked(bool checked);
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
    void onTiiModeChanged(int value);
    void onTiiShowInactiveToggled(bool checked);
    void onTiiInactiveTimeoutToggled(bool checked);
    void onTiiInactiveTimeoutValueChanged(int value);

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
    void activateAirspyControls(bool en);
#endif
#if HAVE_SOAPYSDR
    void onSoapySdrGainModeToggled(bool checked);
    void activateSoapySdrControls(bool en);
    void setSoapySdrGainWidget(bool activate);

    void activateSdrplayControls(bool en);
    void onSdrplayReloadButtonClicked();
    void onSdrplayDeviceChanged(int idx);
    void onSdrplayChannelChanged(int idx);
    void onSdrplayAntennaChanged(int idx);

    void onSdrplayModeToggled(bool checked);
    void onSdrplayAGCstateChanged(int state);
    void onSdrplayIFGainSliderChanged(int val);
    void onSdrplayRFGainSliderChanged(int val);
#endif
};

#endif  // SETUPDIALOG_H
