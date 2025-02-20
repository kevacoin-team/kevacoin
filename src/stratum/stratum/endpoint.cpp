// endpoint.cpp
#include "endpoint.h"
#include "stratum/util/util.h"
#include "structures.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <util.h>

namespace stratum
{

Endpoint::Endpoint(Port* config, boost::asio::io_context& io_context)
    : config_(config), io_context_(io_context), acceptor(nullptr)
{
    //
}

Endpoint::~Endpoint()
{
    // Cleanup
}

void Endpoint::Listen(StratumServer* server, Port* config)
{
    try {
        boost::asio::ip::tcp::endpoint tcp_endpoint(
            boost::asio::ip::make_address(config_->host),
            config_->port);

        acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(io_context_, tcp_endpoint);

        LogPrintf("Endpoint listening on %s:%s.\n", config_->host, config_->port);

        auto socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor->async_accept(*socket_,
            boost::bind(&Endpoint::handle_accept, this, server, socket_, boost::asio::placeholders::error));
    } catch (const std::exception& e) {
        LogPrintf("Error in Endpoint::Listen: %s\n", e.what());
    }
}

void Endpoint::handle_accept(StratumServer* server_, std::shared_ptr<boost::asio::ip::tcp::socket> socket_, const boost::system::error_code& error)
{
    if (!error) {
        LogPrintf("Accepted new connection from: %s:%s\n", socket_->remote_endpoint().address().to_string(), socket_->remote_endpoint().port());

        auto session = std::make_shared<Session>(socket_, socket_->remote_endpoint().address().to_string(), config_->diff);
        session->start(server_);

        server_->addSession(session);
    } else {
        LogPrintf("Accept error: %s\n", error.message());
    }

    // Continue accepting other connections
    try {
        auto socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_context_);
        acceptor->async_accept(*socket_,
            boost::bind(&Endpoint::handle_accept, this, server_, socket_, boost::asio::placeholders::error));
    } catch (const std::exception& e) {
        LogPrintf("Error in Endpoint::handle_accept: %s\n", e.what());
    }
}

} // namespace stratum
