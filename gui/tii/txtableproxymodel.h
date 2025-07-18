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

#ifndef TXTABLEPROXYMODEL_H
#define TXTABLEPROXYMODEL_H

#include <QAbstractTableModel>
#include <QGeoPositionInfo>
#include <QObject>
#include <QSortFilterProxyModel>

class TxTableProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
public:
    TxTableProxyModel(QObject *parent = nullptr);
    void setColumnsFilter(bool filterCols);
    void setInactiveTxFilter(bool filterInactiveTx);
    void setLocalTxFilter(bool filterLocalTx);

signals:
    void rowCountChanged();

protected:
    bool m_filterCols = true;
    bool m_filterInactiveTx = true;
    bool m_filterLocalTx = false;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif  // TXTABLEPROXYMODEL_H
