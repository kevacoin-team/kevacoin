// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/miningmodel.h>
#include <qt/stratumendpointtablemodel.h>
#include <qt/stratumminertablemodel.h>
#include <base58.h>
#include <rpc/protocol.h>
#include <stratum/stratum/stratum_server.h>
#include <stratum/stratum/config.h>
#include "stratum/stratum/miners_map.h"

#include <stdint.h>

#include <QDebug>
#include <QMessageBox>
#include <QSet>
#include <QTimer>

const int NAMESPACE_LENGTH           =  21;
const std::string DUMMY_NAMESPACE    =  "___DUMMY_NAMESPACE___";

MiningModel::MiningModel(stratum::StratumServer* srv, QObject *parent) :
    QObject(parent),
    server(srv),
    stratumEndpointTableModel(0),
    stratumMinerTableModel(0)
{
    stratumEndpointTableModel = new StratumEndpointTableModel(this);
    stratumMinerTableModel = new StratumMinerTableModel(this);
}

MiningModel::~MiningModel()
{
}

StratumEndpointTableModel *MiningModel::getStratumEndpointTableModel()
{
    return stratumEndpointTableModel;
}

StratumMinerTableModel *MiningModel::getStratumMinerTableModel()
{
    return stratumMinerTableModel;
}

stratum::StratumServer* MiningModel::getStratumServer() const
{
    return server;
}

stratum::MinersMap MiningModel::getMiners() const
{
    if (server) {
        return server->getMinersMap();
    }
    return {};
}

bool MiningModel::validateAddress(const QString &address)
{
    return IsValidDestinationString(address.toStdString());
}
