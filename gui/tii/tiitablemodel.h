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

#ifndef TIITABLEMODEL_H
#define TIITABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QGeoPositionInfo>

#include "dabsdr.h"
#include "servicelistid.h"
#include "tiitablemodelitem.h"

class TxDataItem;

enum TiiTableModelRoles {
    CoordinatesRole = Qt::UserRole,
    TiiRole,
    MainIdRole,
    SubIdRole,
    LevelColorRole,
};

class TiiTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum { ColMainId, ColSubId, ColLevel, ColDist, ColAzimuth, NumCols};

    explicit TiiTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    //Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void clear();

    void populateModel(const QList<dabsdrTii_t> & data, const ServiceListId & ensId);

    void setCoordinates(const QGeoCoordinate &newCoordinates);

private:
    QList<TiiTableModelItem> m_modelData;
    QMultiHash<ServiceListId, TxDataItem*> m_txList;
    QGeoCoordinate m_coordinates;
};

#endif // TIITABLEMODEL_H
