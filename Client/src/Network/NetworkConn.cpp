#include "NetworkConn.h"

namespace NetConn {
	Connection::Connection(std::shared_ptr<DBConn> _db_connection)
		:db_(_db_connection){
		loadCPL();

		for (const auto& peer : certifiedPeerList) {
			// 6001 - default server port for every node
			m_Clients.emplace_back(std::make_unique<Client>(io_ctx, _db_connection, peer.endpoint.address().to_string(), peer.endpoint.port()));

		}

	}

	Connection::~Connection() {
		io_ctx.stop();
	}

	void Connection::loadCPL() {
		std::ifstream cplFile("src/Network/certifiedPeerList.json");

		if (!cplFile.is_open()) {
			// File doesn't exist, create it with BOOTSTRAP nodes
			std::ofstream outFile("src/Network/certifiedPeerList.json");
			json bootstrapData = json::array({
					{ 
						{"public_key", "ba2f29ea14dcbaad2e0dd70cefad229f6eda501e"}, 
						{"ip_address", "127.0.0.1"},
						{"port", 6001},
						{"peer_type", static_cast<int>(PeerType::Full)}
					},
					{ 
						{"public_key", "3eabbe9441b1f258f8b1ed9e4df4f904975f2ec3"},
						{"ip_address", "192.168.1.11"},
						{"port", 6001},
						{"peer_type", static_cast<int>(PeerType::Light)}
					}
				});

			outFile << bootstrapData.dump(4);
			outFile.close();

			for (const auto& item : bootstrapData) {
				Peer peer;
				from_json(item, peer);
				certifiedPeerList.push_back(peer);
			}
		}
		else {
			json cplJson;
			cplFile >> cplJson;
			for (const auto& item : cplJson) {
				Peer peer;
				from_json(item, peer);
				certifiedPeerList.push_back(peer);
			}
		}
	}

	void Connection::startServer() {
		if (!m_Server) {
			m_Server = std::make_unique<Server>(io_ctx, db_);
			serverThread = std::thread([&] {
					m_Server->start();
					io_ctx.run();
				});

			serverThread.detach();
		}
	}

	bool Connection::propogateTransaction(const Transaction& tx){
		int count_of_success{ 0 };
		// Propogate the tx to other peers from Client connections to them.
		for (int i = 0; i < m_Clients.size(); i++) {
			bool propogated = m_Clients.at(i)->sendTransaction(tx);
			if (propogated) {
				std::cout << "Propogated to Peer (" << i << ") Successfuly\n";
				count_of_success++;
			}
			else {
				std::cout << "ERROR of propogationg to Peer (" << i << ")\n";
			}
		}
		if (count_of_success > 0)
			return true;
		return false;
	}
}
