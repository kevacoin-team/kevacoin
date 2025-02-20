// main.cpp
#include "stratum/stratum_server.h"
#include "stratum/config.h"
#include "stratum/structures.h"
#include <iostream>
#include "main.h"
#include <fstream>
#include <thread>
#include <chrono>
#include <random>
#include <cstdlib>
#include <string>
#include "stratum/util/util.h"
#include <util.h>
#include <boost/asio.hpp>

using namespace stratum;

stratum::StratumServer* g_stratumServer = nullptr;

void signalHandler(int signal) {
    if (g_stratumServer) {
        LogPrintf("Stratum services shutting down...\n");
        delete g_stratumServer;
        g_stratumServer = nullptr;
    }
    std::exit(signal);
}

void setConfig(stratum::Config *cfg)
{
    try
    {
        Port ep;
        cfg->Address = gArgs.GetArg("-stratumaddr", "");
        for (auto &th : gArgs.GetArgs("-stratumep"))
        {
            std::vector<std::string> a = util::split(th, '@');
            ep.diff = std::stoll(a[0]);
            std::vector<std::string> b = util::split(a[1], ':');
            ep.host = b[0];
            ep.port = std::stoi(b[1]);
            cfg->Stratum.listen.push_back(ep);
        }
        cfg->Stratum.timeout = gArgs.GetArg("-stratumeptimeout", 0);
        cfg->BypassShareValidation = gArgs.GetArg("-stratumvalshare", 1);
        cfg->BypassAddressValidation = gArgs.GetArg("-stratumvaladdr", 1);
    }
    catch (const std::exception &e)
    {
        LogPrintf("Config error: %s\n", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void startStratum()
{
    stratum::Config cfg;
    setConfig(&cfg);

    try
    {
        g_stratumServer = new stratum::StratumServer(&cfg);
        g_stratumServer->Listen();

        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](const boost::system::error_code &error, int signal_number)
                           {
            if (!error) {
                LogPrintf("Stratum services received shutdown signal.\n");
                signalHandler(signal_number);
            } });

        LogPrintf("Stratum services are running.\n");
        io_context.run();
        LogPrintf("Stratum shutdown complete.\n");
    }
    catch (const std::exception &e)
    {
        LogPrintf("Failed to start Stratum services: %s\n", e.what());
    }
}
