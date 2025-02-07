// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_STRATUMMINERTABLEMODEL_H
#define BITCOIN_QT_STRATUMMINERTABLEMODEL_H

#include <qt/miningmodel.h>

#include <QAbstractTableModel>
#include <QStringList>
#include <QDateTime>

class MinerEntry
{
public:
    MinerEntry() { }

    std::string id;
    std::string ip;
    double hr;
    double hr_24;
    int accepted;
    int stale;
    int rejected;
    int blocks_accepted;
    int blocks_rejected;
    uint64_t average_share_difficulty;
    QDateTime last_beat;
    
};

class StratumMinerTableModel: public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit StratumMinerTableModel(MiningModel *parent);
    ~StratumMinerTableModel();

    enum ColumnIndex {
        ID = 0,
        IP = 1,
        HR = 2,
        HR_24 = 3,
        Accepted = 4,
        Stale = 5,
        Rejected = 6,
        Blocks_Accepted = 7,
        Blocks_Rejected = 8,
        Average_Share_Difficulty = 9,
        Last_Beat = 10,
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

    const MinerEntry &entry(int row) const { return list[row]; }
    void setMiner(std::vector<MinerEntry> vKevaEntries);

public Q_SLOTS:
    // void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    void updateDisplayUnit();

private:
    MiningModel *miningModel;
    QStringList columns;
    QList<MinerEntry> list;
    int64_t nReceiveRequestsMaxId;
};

#endif // BITCOIN_QT_STRATUMMINERTABLEMODEL_H
