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

#include <QBoxLayout>
#include <QMenu>
#include <QQmlContext>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)) && QT_CONFIG(permissions)
#include <QPermissions>
#endif
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QMessageBox>

#include "txmapdialog.h"

Q_LOGGING_CATEGORY(txMap, "TII", QtInfoMsg)

TxMapDialog::TxMapDialog(Settings *settings, bool isTii, QWidget *parent) : QDialog(parent), m_settings(settings), m_isTii(isTii)
{
    m_model = new TxTableModel(this);
    m_sortedFilteredModel = new TxTableProxyModel(this);
    m_sortedFilteredModel->setSourceModel(m_model);
    m_tableSelectionModel = new QItemSelectionModel(m_sortedFilteredModel, this);
    m_mapCenter = QGeoCoordinate(50.08804, 14.42076);  // Prague
    connect(m_tableSelectionModel, &QItemSelectionModel::selectionChanged, this, &TxMapDialog::onSelectionChanged, Qt::QueuedConnection);
    connect(this, &TxMapDialog::selectedRowChanged, this, &TxMapDialog::onSelectedRowChanged);
}

void TxMapDialog::positionUpdated(const QGeoPositionInfo &position)
{
    setCurrentPosition(position.coordinate());
    m_model->setCoordinates(m_currentPosition);
    setPositionValid(true);
}

TxMapDialog::~TxMapDialog()
{
    delete m_tableSelectionModel;
    delete m_model;
}

void TxMapDialog::showEvent(QShowEvent *event)
{
    if (!isVisible())
    {
        reset();
        startLocationUpdate();
        setIsVisible(true);
    }

    QDialog::showEvent(event);
}

void TxMapDialog::closeEvent(QCloseEvent *event)
{
    setIsVisible(false);
    stopLocationUpdate();

    QDialog::closeEvent(event);
}

int TxMapDialog::selectedRow() const
{
    return m_selectedRow;
}

void TxMapDialog::setSelectedRow(int newSelectedRow)
{
    if (m_selectedRow == newSelectedRow)
    {
        return;
    }
    m_selectedRow = newSelectedRow;
    emit selectedRowChanged();
}

void TxMapDialog::reset()
{
    m_model->clear();
    m_tableSelectionModel->clear();
    m_txInfo.clear();
    emit txInfoChanged();
}

void TxMapDialog::startLocationUpdate()
{
    switch (m_settings->tii.locationSource)
    {
        case Settings::GeolocationSource::System:
        {
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
            // ask for permission
#if QT_CONFIG(permissions)
            QLocationPermission locationsPermission;
            locationsPermission.setAccuracy(QLocationPermission::Precise);
            locationsPermission.setAvailability(QLocationPermission::WhenInUse);
            switch (qApp->checkPermission(locationsPermission))
            {
                case Qt::PermissionStatus::Undetermined:
                    qApp->requestPermission(locationsPermission, this, [this]() { startLocationUpdate(); });
                    qCDebug(txMap) << "LocationPermission Undetermined";
                    return;
                case Qt::PermissionStatus::Denied:
                {
                    qCInfo(txMap) << "LocationPermission Denied";
                    QMessageBox msgBox(QMessageBox::Warning, tr("Warning"), tr("Device location access is denied."), {}, this);
                    msgBox.setInformativeText(
                        tr("If you want to display current position on map, grant the location permission in Settings then open the app again."));
                    msgBox.exec();
                }
                    return;
                case Qt::PermissionStatus::Granted:
                    qCInfo(txMap) << "LocationPermission Granted";
                    break;  // Proceed
            }
#endif
#endif
            // start location update
            if (m_geopositionSource == nullptr)
            {
                m_geopositionSource = QGeoPositionInfoSource::createDefaultSource(this);
            }
            if (m_geopositionSource != nullptr)
            {
                qCDebug(txMap) << "Start position update";
                connect(m_geopositionSource, &QGeoPositionInfoSource::positionUpdated, this, &TxMapDialog::positionUpdated);
                m_geopositionSource->startUpdates();
                qCInfo(txMap) << "Location source system";
            }
            else
            {
                qCWarning(txMap) << "Failed to create location source: system";
            }
        }
        break;
        case Settings::GeolocationSource::Manual:
            if (m_geopositionSource != nullptr)
            {
                delete m_geopositionSource;
                m_geopositionSource = nullptr;
            }
            positionUpdated(QGeoPositionInfo(m_settings->tii.coordinates, QDateTime::currentDateTime()));
            qCInfo(txMap) << "Location source manual: latitude" << m_settings->tii.coordinates.latitude() << "| longitude"
                          << m_settings->tii.coordinates.longitude();
            break;
        case Settings::GeolocationSource::SerialPort:
        {
            // serial port
            QVariantMap params;
            params["nmea.source"] = "serial:" + m_settings->tii.serialPort;
            params["nmea.baudrate"] = m_settings->tii.serialPortBaudrate;
            m_geopositionSource = QGeoPositionInfoSource::createSource("nmea", params, this);
            if (m_geopositionSource != nullptr)
            {
                qCDebug(txMap) << "Start position update";
                connect(m_geopositionSource, &QGeoPositionInfoSource::positionUpdated, this, &TxMapDialog::positionUpdated);
                m_geopositionSource->startUpdates();
                qCInfo(txMap) << "Location source serial port" << m_settings->tii.serialPort << "@" << m_settings->tii.serialPortBaudrate;
            }
            else
            {
                qCWarning(txMap) << "Failed to create location source: serial port" << m_settings->tii.serialPort << "@"
                                 << m_settings->tii.serialPortBaudrate;
            }
        }
        break;
    }
}

void TxMapDialog::stopLocationUpdate()
{
    if (m_geopositionSource != nullptr)
    {
        m_geopositionSource->stopUpdates();
    }
}

void TxMapDialog::onSettingsChanged()
{
    if (m_geopositionSource != nullptr)
    {
        delete m_geopositionSource;
        m_geopositionSource = nullptr;
    }
    if (m_isVisible)
    {
        startLocationUpdate();
    }
    m_model->setDisplayTimeInUTC(m_settings->tii.timestampInUTC);
    m_sortedFilteredModel->setInactiveTxFilter(m_settings->tii.showInactiveTx == false);
}

QGeoCoordinate TxMapDialog::currentPosition() const
{
    return m_currentPosition;
}

void TxMapDialog::setCurrentPosition(const QGeoCoordinate &currentPosition)
{
    if (m_currentPosition == currentPosition)
    {
        return;
    }
    m_currentPosition = currentPosition;
    emit currentPositionChanged();

    if (m_centerToCurrentPosition)
    {
        setMapCenter(m_currentPosition);
    }
}

bool TxMapDialog::positionValid() const
{
    return m_positionValid;
}

void TxMapDialog::setPositionValid(bool newPositionValid)
{
    if (m_positionValid == newPositionValid)
    {
        return;
    }
    m_positionValid = newPositionValid;
    emit positionValidChanged();
}

bool TxMapDialog::isVisible() const
{
    return m_isVisible;
}

void TxMapDialog::setIsVisible(bool newIsVisible)
{
    if (m_isVisible == newIsVisible)
    {
        return;
    }
    m_isVisible = newIsVisible;
    emit isVisibleChanged();
}

QStringList TxMapDialog::txInfo() const
{
    return m_txInfo;
}

QStringList TxMapDialog::ensembleInfo() const
{
    if (!m_currentEnsemble.isValid())
    {
        return QStringList{"", "", ""};
    }

    QStringList info;
    info.append(tr("Ensemble: <b>%1</b>").arg(m_currentEnsemble.label));
    int numTx = m_settings->tii.showInactiveTx ? m_model->rowCount() : m_model->activeCount();
    if (m_isTii && numTx > 0)
    {
        info.append(tr("ECC: <b>%1</b> | EID: <b>%2</b> | TX: <b>%3</b>")
                        .arg(m_currentEnsemble.ecc(), 2, 16, QChar('0'))
                        .arg(m_currentEnsemble.eid(), 4, 16, QChar('0'))
                        .arg(numTx)
                        .toUpper());
    }
    else
    {
        info.append(tr("ECC: <b>%1</b> | EID: <b>%2</b>")
                        .arg(m_currentEnsemble.ecc(), 2, 16, QChar('0'))
                        .arg(m_currentEnsemble.eid(), 4, 16, QChar('0'))
                        .toUpper());
    }
    info.append(QString(tr("Channel: <b>%1 (%2 kHz)</b>")).arg(DabTables::channelList[m_currentEnsemble.frequency]).arg(m_currentEnsemble.frequency));
    return info;
}

// int TxMapDialog::selectedRow() const
// {
//     return m_selectedRow;
// }

bool TxMapDialog::isTii() const
{
    return m_isTii;
}

void TxMapDialog::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QModelIndexList selectedRows = m_tableSelectionModel->selectedRows();

    // selection is in proxy model => maping indexes back to source model
    QSet<int> selectedTx;
    for (const auto &index : selectedRows)
    {
        auto srcIdx = m_sortedFilteredModel->mapToSource(index);
        if (srcIdx.isValid())
        {
            // qDebug() << m_model->data(srcIdx, TxTableModel::TiiRole);
            selectedTx.insert(srcIdx.row());
        }
    }
    m_model->setSelectedRows(selectedTx);

    if (selectedRows.count() != 1)
    {  // no selection => return
        setSelectedRow(-1);
        return;
    }

    QModelIndex currentIndex = selectedRows.at(0);
    currentIndex = m_sortedFilteredModel->mapToSource(currentIndex);
    setSelectedRow(currentIndex.row());
}

void TxMapDialog::selectTx(int index)
{
    if (index == -1)
    {
        m_tableSelectionModel->clear();
        return;
    }

    QModelIndexList selection = m_tableSelectionModel->selectedRows();
    QModelIndex idx = m_sortedFilteredModel->index(index, 0);
    if (idx.isValid() && (selection.isEmpty() || selection.at(0) != idx))
    {
        m_tableSelectionModel->setCurrentIndex(idx, QItemSelectionModel::NoUpdate);  // Set current index
        m_tableSelectionModel->select(idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }
}

bool TxMapDialog::isRecordingLog() const
{
    return m_isRecordingLog;
}

void TxMapDialog::setIsRecordingLog(bool newIsRecordingLog)
{
    if (m_isRecordingLog == newIsRecordingLog)
    {
        return;
    }
    m_isRecordingLog = newIsRecordingLog;
    emit isRecordingLogChanged();
}

float TxMapDialog::zoomLevel() const
{
    return m_zoomLevel;
}

void TxMapDialog::setZoomLevel(float newZoomLevel)
{
    if (qFuzzyCompare(m_zoomLevel, newZoomLevel))
    {
        return;
    }
    m_zoomLevel = newZoomLevel;
    emit zoomLevelChanged();
}

QGeoCoordinate TxMapDialog::mapCenter() const
{
    return m_mapCenter;
}

void TxMapDialog::setMapCenter(const QGeoCoordinate &mapCenter)
{
    if (m_mapCenter == mapCenter)
    {
        return;
    }
    m_mapCenter = mapCenter;
    emit mapCenterChanged();
}

bool TxMapDialog::centerToCurrentPosition() const
{
    return m_centerToCurrentPosition;
}

void TxMapDialog::setCenterToCurrentPosition(bool centerToCurrentPosition)
{
    if (m_centerToCurrentPosition == centerToCurrentPosition)
    {
        return;
    }
    m_centerToCurrentPosition = centerToCurrentPosition;

    if (m_centerToCurrentPosition)
    {
        if (m_positionValid)
        {
            setMapCenter(m_currentPosition);  // Prague
        }
        else
        {
            setMapCenter(QGeoCoordinate(50.08804, 14.42076));  // Prague
        }
    }

    emit centerToCurrentPositionChanged();
}
