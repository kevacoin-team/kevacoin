// endpoint.h
#ifndef ENDPOINT_H
#define ENDPOINT_H

#include "stratum/util/util.h"
#include "stratum_server.h"
#include "structures.h"
#include <boost/asio.hpp>

namespace stratum
{

class Endpoint
{
public:
    Endpoint(Port* config, boost::asio::io_context& io_context);
    ~Endpoint();

    void Listen(StratumServer* server_, Port* cfg);

private:
    void handle_accept(StratumServer* server, std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error);

    Port* config_;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::asio::io_context& io_context_;
};

} // namespace stratum

#endif // ENDPOINT_H
