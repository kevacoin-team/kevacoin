#include <boost/multiprecision/cpp_int.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#ifndef STRUCTURES_H
#define STRUCTURES_H

namespace stratum
{

struct Stratum {
    int64_t timeout;
    std::vector<class Port> listen;
};

struct Port {
    int64_t diff;
    std::string host;
    int port;
    int maxConn;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Port, diff, host, port, maxConn);
};

// struct RpcGet_Info
// {
//     bool mainnet;                   // true,
//     bool testnet;                   //": false,
//     int64_t height;                 // ": 1215433,
//     int incoming_connections_count; //": 0,
//     int outgoing_connections_count; //": 8,
//     int64_t difficulty;             //": 19202467,
//     int tx_pool_size;               //": 2,
//     std::string status;             //": "OK"

//     NLOHMANN_DEFINE_TYPE_INTRUSIVE(RpcGet_Info, mainnet, testnet, height, incoming_connections_count, outgoing_connections_count, difficulty, tx_pool_size, status);
// };

struct LoginParams {
    std::string login;
    std::string pass;
    std::string agent;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginParams, login, pass, agent);
};

struct GetJobParams {
    std::string id;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GetJobParams, id);
};

struct SubmitParams {
    std::string id;
    std::string job_id;
    std::string nonce;
    std::string result;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SubmitParams, id, job_id, nonce, result);
};

struct blockEntry {
    uint64_t height;
    double variance;
    std::string hash;
};

struct ErrorReply {
    int code;
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ErrorReply, code, message);
};

struct StatusReply {
    std::string status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(StatusReply, status);
};

struct SubmitBlockResult {
    nlohmann::json error;
};

struct JobReplyData {
    std::string blob;
    std::string job_id;
    std::string target;
    int64_t height;
    std::string algo;
    std::string seed_hash;
    std::string next_seed_hash;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JobReplyData, blob, job_id, target, height, algo, seed_hash, next_seed_hash);
};

struct JobReply {
    std::string id;
    nlohmann::json job;
    std::string status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JobReply, id, job, status);
};

struct JobData {
    std::string blob;
    std::string job_id;
    std::string target;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JobData, blob, job_id, target);
};

struct GetBlockTemplateReply {
    int64_t difficulty;
    int64_t height;
    std::string blocktemplate_blob;
    int reserved_offset;
    std::string prev_hash;
    std::string seed_hash;
    std::string next_seed_hash;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GetBlockTemplateReply, difficulty, height, blocktemplate_blob, reserved_offset, prev_hash, seed_hash, next_seed_hash)
};

struct GetInfoReply {
    int64_t incoming_connections_count;
    int64_t outgoing_connections_count;
    int64_t height;
    int64_t tx_pool_size;
    std::string status;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GetInfoReply, incoming_connections_count, outgoing_connections_count, height, tx_pool_size, status)
};

struct GetBlockHashReply {
    std::string hash;
};

struct ValidateAddressReply {
    bool isvalid;
    bool ismine;
};

struct JSONRpcReq {
    nlohmann::json id;
    std::string method;
    nlohmann::json params;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JSONRpcReq, id, method, params);
};

struct JSONRpcPush {
    std::string jsonrpc = "2.0";
    std::string method;
    JobReplyData params;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JSONRpcPush, jsonrpc, method, params);
};

struct JSONRpcResp {
    nlohmann::json id;
    std::string version;
    nlohmann::json result;
    nlohmann::json error;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(JSONRpcResp, id, version, result, error);
};

}; // namespace stratum

#endif // STRUCTURES_H