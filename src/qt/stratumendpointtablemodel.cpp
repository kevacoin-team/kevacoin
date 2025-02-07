// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/stratumendpointtablemodel.h>

#include <qt/bitcoinunits.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>

#include <clientversion.h>
#include <streams.h>


StratumEndpointTableModel::StratumEndpointTableModel(MiningModel *parent) :
    QAbstractTableModel(parent), miningModel(parent)
{
    /* These columns must match the indices in the ColumnIndex enumeration */
    columns << tr("Host") << tr("Port") << tr("Difficulty"); // << tr("Miners");
}

StratumEndpointTableModel::~StratumEndpointTableModel()
{
    /* Intentionally left empty */
}

int StratumEndpointTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return list.length();
}

int StratumEndpointTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return columns.length();
}

QVariant StratumEndpointTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= list.length())
        return QVariant();

    if (role == Qt::TextColorRole)
    {
        return QVariant();
    }
    else if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        const EndpointEntry *rec = &list[index.row()];
        switch(index.column())
        {
        case Host:
            return QString::fromStdString(rec->host);
        case Port:
            return QString::fromStdString(rec->port);
        case Difficulty:
            return QString::fromStdString(rec->difficulty);
        // case Miners:
        //     return QString::fromStdString(rec->miners);
        }
    }
    return QVariant();
}

bool StratumEndpointTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return true;
}

QVariant StratumEndpointTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QModelIndex StratumEndpointTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return createIndex(row, column);
}

bool StratumEndpointTableModel::removeRows(int row, int count, const QModelIndex &parent)
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

Qt::ItemFlags StratumEndpointTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}


// actually add to table in GUI
void StratumEndpointTableModel::setEndpoint(std::vector<EndpointEntry> vEndpointEntries)
{
    // Remove the old ones.
    removeRows(0, list.size());
    list.clear();

    for (auto it = vEndpointEntries.begin(); it != vEndpointEntries.end(); it++) {
        beginInsertRows(QModelIndex(), 0, 0);
        list.prepend(*it);
        endInsertRows();
    }
}

// void StratumEndpointTableModel::sort(int column, Qt::SortOrder order)
// {
//     qSort(list.begin(), list.end(), KevaEntryLessThan(column, order));
//     Q_EMIT dataChanged(index(0, 0, QModelIndex()), index(list.size() - 1, NUMBER_OF_COLUMNS - 1, QModelIndex()));
// }

void StratumEndpointTableModel::updateDisplayUnit()
{
}
