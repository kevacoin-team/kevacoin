// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_MiningModel_H
#define BITCOIN_QT_MiningModel_H

#include "stratum/stratum/stratum_server.h"
#include "stratum/stratum/config.h"
#include <map>
#include <vector>
#include <QObject>

enum OutputType : int;

class OptionsModel;
class PlatformStyle;
class StratumEndpointTableModel;
class StratumMinerTableModel;
class StratumServer;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class MiningModel : public QObject
{
    Q_OBJECT

public:
    explicit MiningModel(stratum::StratumServer* srv, QObject *parent = 0);
    ~MiningModel();

    StratumEndpointTableModel *getStratumEndpointTableModel();
    StratumMinerTableModel *getStratumMinerTableModel();
    stratum::StratumServer* getStratumServer() const;
    stratum::MinersMap getMiners() const;
    bool validateAddress(const QString &address);

private:
    stratum::StratumServer* server;
    StratumEndpointTableModel *stratumEndpointTableModel;
    StratumMinerTableModel *stratumMinerTableModel;
    QTimer *pollTimer;

Q_SIGNALS:

public Q_SLOTS:

};

#endif // BITCOIN_QT_MiningModel_H
