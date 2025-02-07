// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/stratumminertablemodel.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>

#include <clientversion.h>
#include <streams.h>


StratumMinerTableModel::StratumMinerTableModel(MiningModel *parent) :
    QAbstractTableModel(parent), miningModel(parent)
{
    /* These columns must match the indices in the ColumnIndex enumeration */
    columns << tr("ID") << tr("IP") << tr("HR") << tr("HR/24") << tr("S. Accepted") << tr("S. Stale") << tr("S. Rejected") << tr("B. Accepted") << tr("B. Rejected") << tr ("Avg. Share Diff") << tr("Last Beat");
}

StratumMinerTableModel::~StratumMinerTableModel()
{
    /* Intentionally left empty */
}

int StratumMinerTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return list.length();
}

int StratumMinerTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return columns.length();
}

QVariant StratumMinerTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= list.length())
        return QVariant();

    if (role == Qt::TextColorRole)
    {
        const MinerEntry *rec = &list[index.row()];
        return QVariant();
    }
    else if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        const MinerEntry *rec = &list[index.row()];
        switch(index.column())
        {
        case ID:
            return QString::fromStdString(rec->id);
        case IP:
            return QString::fromStdString(rec->ip);
        case HR:
            return QString::number(rec->hr);
        case HR_24:
            return QString::number(rec->hr_24);
        case Accepted:
            return QString::number(rec->accepted);
        case Stale:
            return QString::number(rec->stale);
        case Rejected:
            return QString::number(rec->rejected);
        case Blocks_Accepted:
            return QString::number(rec->blocks_accepted);
        case Blocks_Rejected:
            return QString::number(rec->blocks_rejected);
        case Average_Share_Difficulty:
            return QString::number(rec->average_share_difficulty);
        case Last_Beat:
            return rec->last_beat;
        }

    }
    return QVariant();
}

bool StratumMinerTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return true;
}

QVariant StratumMinerTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole && section < columns.size())
        {
            return columns[section];
        }
    }
    return QVariant();
}

QModelIndex StratumMinerTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column);
}

bool StratumMinerTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if(count > 0 && row >= 0 && (row+count) <= list.size())
    {
        beginRemoveRows(parent, row, row + count - 1);
        list.erase(list.begin() + row, list.begin() + row + count);
        endRemoveRows();
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags StratumMinerTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}


// actually add to table in GUI
void StratumMinerTableModel::setMiner(std::vector<MinerEntry> vKevaEntries)
{
    // Remove the old ones.
    removeRows(0, list.size());
    list.clear();

    for (auto it = vKevaEntries.begin(); it != vKevaEntries.end(); it++) {
        beginInsertRows(QModelIndex(), 0, 0);
        list.prepend(*it);
        endInsertRows();
    }
}

// void StratumMinerTableModel::sort(int column, Qt::SortOrder order)
// {
//     qSort(list.begin(), list.end(), KevaEntryLessThan(column, order));
//     Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(list.size() - 1, NUMBER_OF_COLUMNS - 1, QModelIndex()));
// }

void StratumMinerTableModel::updateDisplayUnit()
{
}

// bool KevaEntryLessThan::operator()(KevaEntry &left, KevaEntry &right) const
// {
//     KevaEntry *pLeft = &left;
//     KevaEntry *pRight = &right;
//     if (order == Qt::DescendingOrder)
//         std::swap(pLeft, pRight);

//     switch(column)
//     {
//     case StratumMinerTableModel::Date:
//         return pLeft->date.toTime_t() < pRight->date.toTime_t();
//     case StratumMinerTableModel::Block:
//         return pLeft->block < pRight->block;
//     case StratumMinerTableModel::Key:
//         return pLeft->key < pRight->key;
//     case StratumMinerTableModel::Value:
//         return pLeft->value < pRight->value;
//     default:
//         return pLeft->block < pRight->block;
//     }
// }
