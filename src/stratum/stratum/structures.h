#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <boost/multiprecision/cpp_int.hpp>
#include <memory>
#include <string>
#include <vector>
#include "univalue.h"

namespace stratum
{

struct Port {
    int64_t diff;
    std::string host;
    int port;
    int maxConn;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("diff", diff);
        obj.pushKV("host", host);
        obj.pushKV("port", port);
        obj.pushKV("maxConn", maxConn);
        return obj;
    }

    static Port fromUniValue(const UniValue &obj) {
        Port p;
        if (!obj.isObject()) throw std::runtime_error("Port: not an object");
        p.diff = obj["diff"].get_int64();
        p.host = obj["host"].get_str();
        p.port = obj["port"].get_int();
        p.maxConn = obj["maxConn"].get_int();
        return p;
    }
};

struct Stratum {
    int64_t timeout;
    std::vector<Port> listen;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("timeout", timeout);
        UniValue arr(UniValue::VARR);
        for (const auto &p : listen) {
            arr.push_back(p.toUniValue());
        }
        obj.pushKV("listen", arr);
        return obj;
    }

    static Stratum fromUniValue(const UniValue &obj) {
        Stratum s;
        if (!obj.isObject()) throw std::runtime_error("Stratum: not an object");
        s.timeout = obj["timeout"].get_int64();
        UniValue arr = obj["listen"];
        for (unsigned int i = 0; i < arr.size(); i++) {
            s.listen.push_back(Port::fromUniValue(arr[i]));
        }
        return s;
    }
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
// };

struct LoginParams {
    std::string login;
    std::string pass;
    std::string agent;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("login", login);
        obj.pushKV("pass", pass);
        obj.pushKV("agent", agent);
        return obj;
    }

    static LoginParams fromUniValue(const UniValue &obj) {
        LoginParams lp;
        if (!obj.isObject()) throw std::runtime_error("LoginParams: not an object");
        lp.login = obj["login"].get_str();
        lp.pass  = obj["pass"].get_str();
        lp.agent = obj["agent"].get_str();
        return lp;
    }
};

struct GetJobParams {
    std::string id;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("id", id);
        return obj;
    }

    static GetJobParams fromUniValue(const UniValue &obj) {
        GetJobParams p;
        if (!obj.isObject()) throw std::runtime_error("GetJobParams: not an object");
        p.id = obj["id"].get_str();
        return p;
    }
};

struct SubmitParams {
    std::string id;
    std::string job_id;
    std::string nonce;
    std::string result;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("id", id);
        obj.pushKV("job_id", job_id);
        obj.pushKV("nonce", nonce);
        obj.pushKV("result", result);
        return obj;
    }

    static SubmitParams fromUniValue(const UniValue &obj) {
        SubmitParams sp;
        if (!obj.isObject()) throw std::runtime_error("SubmitParams: not an object");
        sp.id     = obj["id"].get_str();
        sp.job_id = obj["job_id"].get_str();
        sp.nonce  = obj["nonce"].get_str();
        sp.result = obj["result"].get_str();
        return sp;
    }
};

struct blockEntry {
    uint64_t height;
    double variance;
    std::string hash;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("height", static_cast<int64_t>(height));
        obj.pushKV("variance", variance);
        obj.pushKV("hash", hash);
        return obj;
    }

    static blockEntry fromUniValue(const UniValue &obj) {
        blockEntry be;
        if (!obj.isObject()) throw std::runtime_error("blockEntry: not an object");
        be.height = obj["height"].get_int64();
        be.variance = obj["variance"].get_real();
        be.hash = obj["hash"].get_str();
        return be;
    }
};

struct ErrorReply {
    int code;
    std::string message;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("code", code);
        obj.pushKV("message", message);
        return obj;
    }

    static ErrorReply fromUniValue(const UniValue &obj) {
        ErrorReply er;
        if (!obj.isObject()) throw std::runtime_error("ErrorReply: not an object");
        er.code = obj["code"].get_int();
        er.message = obj["message"].get_str();
        return er;
    }
};

struct StatusReply {
    std::string status;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("status", status);
        return obj;
    }

    static StatusReply fromUniValue(const UniValue &obj) {
        StatusReply sr;
        if (!obj.isObject()) throw std::runtime_error("StatusReply: not an object");
        sr.status = obj["status"].get_str();
        return sr;
    }
};

struct SubmitBlockResult {
    UniValue error;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("error", error);
        return obj;
    }

    static SubmitBlockResult fromUniValue(const UniValue &obj) {
        SubmitBlockResult sbr;
        if (!obj.isObject()) throw std::runtime_error("SubmitBlockResult: not an object");
        sbr.error = obj["error"];
        return sbr;
    }
};

struct JobReplyData {
    std::string blob;
    std::string job_id;
    std::string target;
    int64_t height;
    std::string algo;
    std::string seed_hash;
    std::string next_seed_hash;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("blob", blob);
        obj.pushKV("job_id", job_id);
        obj.pushKV("target", target);
        obj.pushKV("height", height);
        obj.pushKV("algo", algo);
        obj.pushKV("seed_hash", seed_hash);
        obj.pushKV("next_seed_hash", next_seed_hash);
        return obj;
    }

    static JobReplyData fromUniValue(const UniValue &obj) {
        JobReplyData jrd;
        if (!obj.isObject()) throw std::runtime_error("JobReplyData: not an object");
        jrd.blob = obj["blob"].get_str();
        jrd.job_id = obj["job_id"].get_str();
        jrd.target = obj["target"].get_str();
        jrd.height = obj["height"].get_int64();
        jrd.algo = obj["algo"].get_str();
        jrd.seed_hash = obj["seed_hash"].get_str();
        jrd.next_seed_hash = obj["next_seed_hash"].get_str();
        return jrd;
    }
};

struct JobReply {
    std::string id;
    UniValue job;
    std::string status;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("id", id);
        obj.pushKV("job", job);
        obj.pushKV("status", status);
        return obj;
    }

    static JobReply fromUniValue(const UniValue &obj) {
        JobReply jr;
        if (!obj.isObject()) throw std::runtime_error("JobReply: not an object");
        jr.id = obj["id"].get_str();
        jr.job = obj["job"];
        jr.status = obj["status"].get_str();
        return jr;
    }
};

struct JobData {
    std::string blob;
    std::string job_id;
    std::string target;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("blob", blob);
        obj.pushKV("job_id", job_id);
        obj.pushKV("target", target);
        return obj;
    }

    static JobData fromUniValue(const UniValue &obj) {
        JobData jd;
        if (!obj.isObject()) throw std::runtime_error("JobData: not an object");
        jd.blob = obj["blob"].get_str();
        jd.job_id = obj["job_id"].get_str();
        jd.target = obj["target"].get_str();
        return jd;
    }
};

struct GetBlockTemplateReply {
    int64_t difficulty;
    int64_t height;
    std::string blocktemplate_blob;
    int reserved_offset;
    std::string prev_hash;
    std::string seed_hash;
    std::string next_seed_hash;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("difficulty", difficulty);
        obj.pushKV("height", height);
        obj.pushKV("blocktemplate_blob", blocktemplate_blob);
        obj.pushKV("reserved_offset", reserved_offset);
        obj.pushKV("prev_hash", prev_hash);
        obj.pushKV("seed_hash", seed_hash);
        obj.pushKV("next_seed_hash", next_seed_hash);
        return obj;
    }

    static GetBlockTemplateReply fromUniValue(const UniValue &obj) {
        GetBlockTemplateReply gbr;
        if (!obj.isObject()) throw std::runtime_error("GetBlockTemplateReply: not an object");
        gbr.difficulty = obj["difficulty"].get_int64();
        gbr.height = obj["height"].get_int64();
        gbr.blocktemplate_blob = obj["blocktemplate_blob"].get_str();
        gbr.reserved_offset = obj["reserved_offset"].get_int();
        gbr.prev_hash = obj["prev_hash"].get_str();
        gbr.seed_hash = obj["seed_hash"].get_str();
        gbr.next_seed_hash = obj["next_seed_hash"].get_str();
        return gbr;
    }
};

struct GetInfoReply {
    int64_t incoming_connections_count;
    int64_t outgoing_connections_count;
    int64_t height;
    int64_t tx_pool_size;
    std::string status;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("incoming_connections_count", incoming_connections_count);
        obj.pushKV("outgoing_connections_count", outgoing_connections_count);
        obj.pushKV("height", height);
        obj.pushKV("tx_pool_size", tx_pool_size);
        obj.pushKV("status", status);
        return obj;
    }

    static GetInfoReply fromUniValue(const UniValue &obj) {
        GetInfoReply gir;
        if (!obj.isObject()) throw std::runtime_error("GetInfoReply: not an object");
        gir.incoming_connections_count = obj["incoming_connections_count"].get_int64();
        gir.outgoing_connections_count = obj["outgoing_connections_count"].get_int64();
        gir.height = obj["height"].get_int64();
        gir.tx_pool_size = obj["tx_pool_size"].get_int64();
        gir.status = obj["status"].get_str();
        return gir;
    }
};

struct GetBlockHashReply {
    std::string hash;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("hash", hash);
        return obj;
    }

    static GetBlockHashReply fromUniValue(const UniValue &obj) {
        GetBlockHashReply gbr;
        if (!obj.isObject()) throw std::runtime_error("GetBlockHashReply: not an object");
        gbr.hash = obj["hash"].get_str();
        return gbr;
    }
};

struct ValidateAddressReply {
    bool isvalid;
    bool ismine;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("isvalid", isvalid);
        obj.pushKV("ismine", ismine);
        return obj;
    }

    static ValidateAddressReply fromUniValue(const UniValue &obj) {
        ValidateAddressReply var;
        if (!obj.isObject()) throw std::runtime_error("ValidateAddressReply: not an object");
        var.isvalid = obj["isvalid"].get_bool();
        var.ismine = obj["ismine"].get_bool();
        return var;
    }
};

struct JSONRpcReq {
    UniValue id;
    std::string method;
    UniValue params;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("id", id);
        obj.pushKV("method", method);
        obj.pushKV("params", params);
        return obj;
    }

    static JSONRpcReq fromUniValue(const UniValue &obj) {
        JSONRpcReq req;
        if (!obj.isObject()) throw std::runtime_error("JSONRpcReq: not an object");
        req.id = obj["id"];
        req.method = obj["method"].get_str();
        req.params = obj["params"];
        return req;
    }
};

struct JSONRpcPush {
    std::string jsonrpc = "2.0";
    std::string method;
    JobReplyData params;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("jsonrpc", jsonrpc);
        obj.pushKV("method", method);
        obj.pushKV("params", params.toUniValue());
        return obj;
    }

    static JSONRpcPush fromUniValue(const UniValue &obj) {
        JSONRpcPush push;
        if (!obj.isObject()) throw std::runtime_error("JSONRpcPush: not an object");
        push.jsonrpc = obj["jsonrpc"].get_str();
        push.method = obj["method"].get_str();
        push.params = JobReplyData::fromUniValue(obj["params"]);
        return push;
    }
};

struct JSONRpcResp {
    UniValue id;
    std::string version;
    UniValue result;
    UniValue error;

    UniValue toUniValue() const {
        UniValue obj(UniValue::VOBJ);
        obj.pushKV("id", id);
        obj.pushKV("jsonrpc", version);
        obj.pushKV("result", result);
        obj.pushKV("error", error);
        return obj;
    }

    static JSONRpcResp fromUniValue(const UniValue &obj) {
        JSONRpcResp resp;
        if (!obj.isObject()) throw std::runtime_error("JSONRpcResp: not an object");
        resp.id = obj["id"];
        resp.version = obj["jsonrpc"].get_str();
        resp.result = obj["result"];
        resp.error = obj["error"];
        return resp;
    }
};

} // namespace stratum

#endif // STRUCTURES_H
