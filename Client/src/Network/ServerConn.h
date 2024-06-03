#pragma once
#include "Net_Common.hpp"

class Server {
public:
    Server(asio::io_context& io_context, std::shared_ptr<DBConn> _db_connection);
    ~Server();
    void start();

private:
    void receiveMessage();
    void handleMessage(std::size_t bytes_received);
    void ServerOnTxPropagation(const Transaction tx);

    asio::io_context& io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint sender_endpoint_;
    std::array<char, 1024> recv_buffer_;
    std::shared_ptr<DBConn> db_;

    std::vector<Peer> connected_peers;

    std::vector<Transaction> mempool;

    // bool createSocket() <- Creates a new socket (besides the one being created and constructor)
    // void actAsRelay(peerA, peerB)
    // void propogateCPL(Peer connectedPeerList[]) - sends back a response of these peers
};


/*
  4x6 model. 4 server instances for 6 peers
RESPONIBILITES:
 + - Create A socket(s?) and listen for incoming connections.
 + - Listen to connected peers and process their sent data (sharing of transactions or just blocks for blockchain in general).
 - provide access to the blockchain data for other peers.
 - Validation and Consensus (They verify the validity of incoming transactions, propose new blocks,
   and participate in reaching consensus on the state of the blockchain.)
 - Servers serve as nodes in the P2P network, facilitating communication and data exchange among peers.
   They listen for incoming connections, respond to requests from other peers, and relay information across the network.
 + - Propogate your Connected_Peers_List to new clients asking for it. Don't keep them for long, just send a response back and disregard them
   if you are full of peers. When a new peer wants to connect to you - authenticate them. (How? How does it look like?)
 + - Act as a Peer C (relay server) for other peers who are trying to connect to each other.

 */
 /*
   STEP-BY-STEP for servers:
      I. listen to connections
     II. listen to incoming requests (new transaction being propogated, request for blockchain data)
    III. Verify propogated transactions and blocks.
     IV. If Verification successful -> Make the dbConn add those.
 */


 // Server - Share
 // */
