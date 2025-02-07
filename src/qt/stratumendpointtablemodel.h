// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_STRATUMENDPOINTTABLEMODEL_H
#define BITCOIN_QT_STRATUMENDPOINTTABLEMODEL_H

#include <qt/miningmodel.h>

#include <QAbstractTableModel>
#include <QStringList>
#include <QDateTime>

class EndpointEntry
{
public:
    EndpointEntry() { }

    std::string host;
    std::string port;
    std::string difficulty;
    // std::string miners;
};

class StratumEndpointTableModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StratumEndpointTableModel(MiningModel *parent);
    ~StratumEndpointTableModel();

    enum ColumnIndex {
        Host = 0,
        Port = 1,
        Difficulty = 2,
        // Miners = 3,
        NUMBER_OF_COLUMNS
    };

    /** @name Methods overridden from QAbstractTableModel
        @{*/
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Qt::ItemFlags flags(const QModelIndex &index) const;
    /*@}*/

    const EndpointEntry &entry(int row) const { return list[row]; }
    void setEndpoint(std::vector<EndpointEntry> vEndpointEntries);

public Q_SLOTS:
    // void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void updateDisplayUnit();

private:
    MiningModel *miningModel;
    QStringList columns;
    QList<EndpointEntry> list;
    int64_t nReceiveRequestsMaxId;
};

#endif // BITCOIN_QT_STRATUMENDPOINTTABLEMODEL_H
