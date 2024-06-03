#pragma once

#include <iostream>
#include "ClientConn.h"
#include "ServerConn.h"
#include <vector>
#include <thread>
#include <fstream>

using json = nlohmann::json;
// Conclude all network connection here.
/*
 1. PEER DISCOVERY (multicast DNS / peer exchange (PEX) / initial peer bootstrapping)
 2. Use Boost.Asio to create UDP sockets and establish connections between peers. 
 3. Handle NETWORK TRAVERSAL challenges (NAT traversal, firewall traversal) - using techniques like NAT-PMP, UPnP, or STUN/TURN servers.
 4. Exchange data between peers.

And save your public key on their device as a "known keys"

*/

namespace NetConn {
	class Connection {
	public:
		Connection(std::shared_ptr<DBConn> _db_connection);
		~Connection();

		void startServer();
		bool propogateTransaction(const Transaction& tx);

		// getPeers

	private:
		void loadCPL();

	private:
		asio::io_context io_ctx;

		// Clients and Server are for network functionality
		std::vector<std::unique_ptr<Client>> m_Clients; // Essentialy a vector of Peers
		std::unique_ptr<Server> m_Server;
		std::thread serverThread;
		std::shared_ptr<DBConn> db_;	
		
		// CPL - general information about peers. Gets shared further
		std::vector<Peer> certifiedPeerList;
	};
}