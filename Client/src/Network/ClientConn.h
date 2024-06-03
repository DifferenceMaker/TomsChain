#pragma once
#include "Net_Common.hpp"

class Client {
public:
    Client(asio::io_context& io_context, std::shared_ptr<DBConn> _db_connection, const std::string& server_address, unsigned short server_port);
    ~Client();
    bool sendTransaction(const Transaction& tx);

private:
    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint server_endpoint_;
    std::shared_ptr<DBConn>  db_;
};


// Initiate connection to a server (other peer server side).
/*
Client is responsible for:
  - Search for a network. Request Connected_Peers_List from nodes
  - propogating transactions (broadcast them to the connected peers)
  - propogating the state of the blockchain
  - Clients interact with servers to access blockchain data and perform operations such as querying transaction history,
    retrieving block information, and submitting new transactions.
*/

/*
  STEP-BY-STEP for clients:
     I. Connect to other peers (their servers). initiate communication sessions with servers
    II. Send new transactions to them
   III. Ask for blockchain information

*/

// Client - Request
