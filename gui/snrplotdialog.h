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

#ifndef SNRPLOTDIALOG_H
#define SNRPLOTDIALOG_H

#include <QDateTime>
#include <QDialog>
#include <QTimer>
#include "settings.h"

namespace Ui {
class SNRPlotDialog;
}

class QCPItemStraightLine;
class SNRPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SNRPlotDialog(Settings *settings, int freq, QWidget *parent = nullptr);
    ~SNRPlotDialog();
    void setSignalState(uint8_t sync, float snr);
    void setupDarkMode(bool darkModeEna);
    void onTuneDone(uint32_t freq);
    void updateRfLevel(float rfLevel, float gain);
    void updateFreqOffset(float offset);
    void onSignalSpectrum(std::shared_ptr<std::vector<float>> data);

signals:
    void setSignalSpectrum(bool ena);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    enum { xPlotRange = 2*60 };
    static const char * syncLevelLabels[];
    static const QStringList snrLevelIcons;
    Ui::SNRPlotDialog *ui;
    Settings * m_settings = nullptr;
    QTime m_startTime;
    QTimer * m_timer = nullptr;
    int m_avrgCntr = 0;
    std::vector<float> m_spectrumBuffer;
    QList<QCPItemStraightLine *> m_spectLineList;
    int m_frequency = 0;
    float m_rfLevel = NAN;
    float m_tunerGain = NAN;
    int m_snrLevel = -1;

    void addToPlot(float snr);
    void setFreqRange();
    void reset();

    void setRfLevelVisible(bool visible);
    void setGainVisible(bool visible);
};

#endif // SNRPLOTDIALOG_H
