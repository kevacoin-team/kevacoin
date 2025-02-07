// rpc.cpp
#include "rpc.h"
#include "stratum/util/util.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <iostream>
#include <rpc/client.cpp>
#include <rpc/protocol.h>
#include <rpc/server.h>
#include <stdexcept>
#include <univalue.h>
#include <util.h>

namespace stratum
{
bool RPCClient::RPCParseCommandLine(std::string& strResult, const std::string& strCommand, const bool fExecute, std::string* const pstrFilteredOut)
{
    std::vector<std::vector<std::string>> stack;
    stack.push_back(std::vector<std::string>());

    enum CmdParseState {
        STATE_EATING_SPACES,
        STATE_EATING_SPACES_IN_ARG,
        STATE_EATING_SPACES_IN_BRACKETS,
        STATE_ARGUMENT,
        STATE_SINGLEQUOTED,
        STATE_DOUBLEQUOTED,
        STATE_ESCAPE_OUTER,
        STATE_ESCAPE_DOUBLEQUOTED,
        STATE_COMMAND_EXECUTED,
        STATE_COMMAND_EXECUTED_INNER
    } state = STATE_EATING_SPACES;
    std::string curarg;
    UniValue lastResult;
    unsigned nDepthInsideSensitive = 0;
    size_t filter_begin_pos = 0, chpos;
    std::vector<std::pair<size_t, size_t>> filter_ranges;

    auto add_to_current_stack = [&](const std::string& strArg) {
        if (stack.back().empty() && (!nDepthInsideSensitive)) {
            nDepthInsideSensitive = 1;
            filter_begin_pos = chpos;
        }
        // Make sure stack is not empty before adding something
        if (stack.empty()) {
            stack.push_back(std::vector<std::string>());
        }
        stack.back().push_back(strArg);
    };

    auto close_out_params = [&]() {
        if (nDepthInsideSensitive) {
            if (!--nDepthInsideSensitive) {
                assert(filter_begin_pos);
                filter_ranges.push_back(std::make_pair(filter_begin_pos, chpos));
                filter_begin_pos = 0;
            }
        }
        stack.pop_back();
    };

    std::string strCommandTerminated = strCommand;
    if (strCommandTerminated.back() != '\n')
        strCommandTerminated += "\n";
    for (chpos = 0; chpos < strCommandTerminated.size(); ++chpos) {
        char ch = strCommandTerminated[chpos];
        switch (state) {
        case STATE_COMMAND_EXECUTED_INNER:
        case STATE_COMMAND_EXECUTED: {
            bool breakParsing = true;
            switch (ch) {
            case '[':
                curarg.clear();
                state = STATE_COMMAND_EXECUTED_INNER;
                break;
            default:
                if (state == STATE_COMMAND_EXECUTED_INNER) {
                    if (ch != ']') {
                        // append char to the current argument (which is also used for the query command)
                        curarg += ch;
                        break;
                    }
                    if (curarg.size() && fExecute) {
                        // if we have a value query, query arrays with index and objects with a string key
                        UniValue subelement;
                        if (lastResult.isArray()) {
                            for (char argch : curarg)
                                if (!std::isdigit(argch))
                                    throw std::runtime_error("Invalid result query");
                            subelement = lastResult[atoi(curarg.c_str())];
                        } else if (lastResult.isObject())
                            subelement = find_value(lastResult, curarg);
                        else
                            throw std::runtime_error("Invalid result query"); // no array or object: abort
                        lastResult = subelement;
                    }

                    state = STATE_COMMAND_EXECUTED;
                    break;
                }
                // don't break parsing when the char is required for the next argument
                breakParsing = false;

                // pop the stack and return the result to the current command arguments
                close_out_params();

                // don't stringify the json in case of a string to avoid doublequotes
                if (lastResult.isStr())
                    curarg = lastResult.get_str();
                else
                    curarg = lastResult.write(2);

                // if we have a non empty result, use it as stack argument otherwise as general result
                if (curarg.size()) {
                    if (stack.size())
                        add_to_current_stack(curarg);
                    else
                        strResult = curarg;
                }
                curarg.clear();
                // assume eating space state
                state = STATE_EATING_SPACES;
            }
            if (breakParsing)
                break;
        }
        case STATE_ARGUMENT: // In or after argument
        case STATE_EATING_SPACES_IN_ARG:
        case STATE_EATING_SPACES_IN_BRACKETS:
        case STATE_EATING_SPACES: // Handle runs of whitespace
            switch (ch) {
            case '"':
                state = STATE_DOUBLEQUOTED;
                break;
            case '\'':
                state = STATE_SINGLEQUOTED;
                break;
            case '\\':
                state = STATE_ESCAPE_OUTER;
                break;
            case '(':
            case ')':
            case '\n':
                if (state == STATE_EATING_SPACES_IN_ARG)
                    throw std::runtime_error("Invalid Syntax");
                if (state == STATE_ARGUMENT) {
                    if (ch == '(' && stack.size() && stack.back().size() > 0) {
                        if (nDepthInsideSensitive) {
                            ++nDepthInsideSensitive;
                        }
                        stack.push_back(std::vector<std::string>());
                    }

                    // don't allow commands after executed commands on baselevel
                    if (!stack.size())
                        throw std::runtime_error("Invalid Syntax");

                    add_to_current_stack(curarg);
                    curarg.clear();
                    state = STATE_EATING_SPACES_IN_BRACKETS;
                }
                if ((ch == ')' || ch == '\n') && stack.size() > 0) {
                    if (fExecute) {
                        // Convert argument list to JSON objects in method-dependent way,
                        // and pass it along with the method name to the dispatcher.
                        JSONRPCRequest req;
                        req.params = RPCConvertValues(stack.back()[0], std::vector<std::string>(stack.back().begin() + 1, stack.back().end()));
                        req.strMethod = stack.back()[0];
                        lastResult = tableRPC.execute(req);
                    }

                    state = STATE_COMMAND_EXECUTED;
                    curarg.clear();
                }
                break;
            case ' ':
            case ',':
            case '\t':
                if (state == STATE_EATING_SPACES_IN_ARG && curarg.empty() && ch == ',')
                    throw std::runtime_error("Invalid Syntax");

                else if (state == STATE_ARGUMENT) // Space ends argument
                {
                    add_to_current_stack(curarg);
                    curarg.clear();
                }
                if ((state == STATE_EATING_SPACES_IN_BRACKETS || state == STATE_ARGUMENT) && ch == ',') {
                    state = STATE_EATING_SPACES_IN_ARG;
                    break;
                }
                state = STATE_EATING_SPACES;
                break;
            default:
                curarg += ch;
                state = STATE_ARGUMENT;
            }
            break;
        case STATE_SINGLEQUOTED: // Single-quoted string
            switch (ch) {
            case '\'':
                state = STATE_ARGUMENT;
                break;
            default:
                curarg += ch;
            }
            break;
        case STATE_DOUBLEQUOTED: // Double-quoted string
            switch (ch) {
            case '"':
                state = STATE_ARGUMENT;
                break;
            case '\\':
                state = STATE_ESCAPE_DOUBLEQUOTED;
                break;
            default:
                curarg += ch;
            }
            break;
        case STATE_ESCAPE_OUTER: // '\' outside quotes
            curarg += ch;
            state = STATE_ARGUMENT;
            break;
        case STATE_ESCAPE_DOUBLEQUOTED:                  // '\' in double-quoted text
            if (ch != '"' && ch != '\\') curarg += '\\'; // keep '\' for everything but the quote and '\' itself
            curarg += ch;
            state = STATE_DOUBLEQUOTED;
            break;
        }
    }
    if (pstrFilteredOut) {
        if (STATE_COMMAND_EXECUTED == state) {
            assert(!stack.empty());
            close_out_params();
        }
        *pstrFilteredOut = strCommand;
        for (auto i = filter_ranges.rbegin(); i != filter_ranges.rend(); ++i) {
            pstrFilteredOut->replace(i->first, i->second - i->first, "(…)");
        }
    }
    switch (state) // final state
    {
    case STATE_COMMAND_EXECUTED:
        if (lastResult.isStr())
            strResult = lastResult.get_str();
        else
            strResult = lastResult.write(2);
    case STATE_ARGUMENT:
    case STATE_EATING_SPACES:
        return true;
    default: // ERROR to end in one of the other states
        return false;
    }
}

bool RPCClient::RPCExecuteCommandLine(std::string& strResult, const std::string& strCommand, std::string* const pstrFilteredOut)
{
    return RPCParseCommandLine(strResult, strCommand, true, pstrFilteredOut);
};

std::string RPCClient::request(const std::string& command)
{
    try {
        std::string result;
        std::string executableCommand = command + "\n";

        if (!RPCClient::RPCExecuteCommandLine(result, executableCommand)) {
            throw std::runtime_error(("Parse error: unbalanced ' or \""));
        }

        return result;
    } catch (UniValue& objError) {
        try {
            int code = find_value(objError, "code").get_int();
            std::string message = find_value(objError, "message").get_str();
            throw std::runtime_error("CMD_ERROR 01: " + message + " (code " + std::to_string(code) + ")");
        } catch (const std::runtime_error& rk)
        {
            throw std::runtime_error("CMD_ERROR 02: " + objError.write() + "\n" + rk.what());
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error: " + *e.what());
    }
}


RPCClient::RPCClient()
{
}

RPCClient::~RPCClient()
{
}

GetBlockTemplateReply RPCClient::GetBlockTemplate(int reserveSize, const std::string& address)
{
    if (address.empty()) {
        throw std::runtime_error("Failed to get block template, stratum address not set.");
    }
    const std::string command = "getblocktemplate " + std::to_string(reserveSize) + " " + address;
    auto rpcResp = request(command);
    if (rpcResp.empty()) {
        throw std::runtime_error("Failed to get block template");
    }

    GetBlockTemplateReply reply;
    try {
        reply.difficulty = nlohmann::json::parse(rpcResp)["difficulty"];
        reply.height = nlohmann::json::parse(rpcResp)["height"];
        reply.blocktemplate_blob = nlohmann::json::parse(rpcResp)["blocktemplate_blob"];
        reply.reserved_offset = nlohmann::json::parse(rpcResp)["reserved_offset"];
        reply.prev_hash = nlohmann::json::parse(rpcResp)["prev_hash"];
        reply.seed_hash = nlohmann::json::parse(rpcResp)["seed_hash"];
        reply.next_seed_hash = nlohmann::json::parse(rpcResp)["next_seed_hash"];
    } catch (...) {
        throw std::runtime_error("Failed to parse GetBlockTemplateReply");
    }

    return reply;
}

std::string RPCClient::GetInfo()
{
    const std::string command = "get_info";
    auto rpcResp = request(command);

    // RpcGet_Info reply;
    // try {
    //     reply.mainnet = nlohmann::json::parse(rpcResp)["mainnet"]; // true,
    //     reply.testnet = nlohmann::json::parse(rpcResp)["testnet"]; //": false,
    //     reply.height = nlohmann::json::parse(rpcResp)["height"]; // ": 1215433,
    //     reply.incoming_connections_count = nlohmann::json::parse(rpcResp)["incoming_connections_count"]; //": 0,
    //     reply.outgoing_connections_count = nlohmann::json::parse(rpcResp)["outgoing_connections_count"]; //": 8,
    //     reply.difficulty = nlohmann::json::parse(rpcResp)["difficulty"]; //": 19202467,
    //     reply.tx_pool_size = nlohmann::json::parse(rpcResp)["tx_pool_size"]; //": 2,
    //     reply.status = nlohmann::json::parse(rpcResp)["status"]; //": "OK"
    // } catch (...) {
    //     throw std::runtime_error("Failed to parse GetInfoReply");
    // }

    return rpcResp;
}

nlohmann::json RPCClient::SubmitBlock(const std::string& hash)
{
    const std::string command = "submitblock " + hash;
    auto rpcResp = request(command);
    // "{\n  \"status\": \"OK\"\n}"
    return nlohmann::json(rpcResp);
}

JSONRpcResp RPCClient::ValidateAddress(const std::string& addr)
{
    const std::string command = "validateaddress " + addr;
    JSONRpcResp rpcResp = nlohmann::json(request(command));
    return rpcResp;
}

JSONRpcResp RPCClient::GetBlockHash(int height)
{
    const std::string command = "getblockhash";
    JSONRpcResp rpcResp = nlohmann::json(request(command));

    if (!rpcResp.result) {
        throw std::runtime_error("Failed to get block hash");
    }

    return rpcResp.result;
}

// std::shared_ptr<JSONRpcResp> RPCClient::UpdateInfo() {
//     try {
//         JSONRpcResp infoReply = GetInfo();
//         std::shared_ptr<JSONRpcResp> infoPtr = std::make_shared<JSONRpcResp>(infoReply);
//         // info.store(infoPtr);
//         info = infoPtr;
//         return infoPtr;
//     } catch (...) {
//         return nullptr;
//     }
// }

} // namespace stratum
