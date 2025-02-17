// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "structures.h"
#include <vector>

namespace stratum
{

struct Config {
    struct StratumConfig {
        int64_t timeout;
        std::vector<Port> listen;
    } Stratum;
    std::string Address;
    bool BypassShareValidation;
    bool BypassAddressValidation;
    int64_t Timeout;
};

} // namespace stratum

#endif // CONFIG_H
