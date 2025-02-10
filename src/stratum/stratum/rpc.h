// rpc.h
#ifndef RPC_H
#define RPC_H

#include "config.h"
#include "structures.h"
#include "univalue.h"
#include <string>

namespace stratum
{
class RPCClient
{
public:
    RPCClient();
    ~RPCClient();

    GetBlockTemplateReply GetBlockTemplate(int reserveSize, const std::string& address);
    std::string GetInfo();
    UniValue SubmitBlock(const std::string& hash);
    JSONRpcResp ValidateAddress(const std::string& addr);
    UniValue GetBlockHash(int height);

    Config* config;

private:
    // std::shared_ptr<JSONRpcResp> UpdateInfo();

    // boost::shared_mutex rw_mutex;
    std::shared_ptr<JSONRpcResp> info;
    std::string request(const std::string& command);
    bool RPCParseCommandLine(std::string& strResult, const std::string& strCommand, const bool fExecute, std::string* const pstrFilteredOut);
    bool RPCExecuteCommandLine(std::string& strResult, const std::string& strCommand, std::string* const pstrFilteredOut = nullptr);
};

} // namespace stratum

#endif // RPC_H
