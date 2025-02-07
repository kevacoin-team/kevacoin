// main.h
#ifndef MAIN_H
#define MAIN_H

#include "stratum/config.h"
#include "stratum/stratum/stratum_server.h"
#include <csignal>

extern stratum::StratumServer* g_stratumServer;

void startStratum();
// void readConfig(stratum::Config *cfg);
void setConfig(stratum::Config *cfg);
void signalHandler(int signal);

#endif // MAIN_H
