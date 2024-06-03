#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/internet.hpp>
#include <asio/ts/buffer.hpp>
#include <nlohmann/json.hpp>
#include "Transaction.h"
#include "Block.h"
#include "DBConn.h"

using json = nlohmann::json;

const char TX_PROPAGATION[] = "01";
const char BLOCK_PROPAGATION[] = "02";
const char SHARE_BLOCKCHAIN_DATA[] = "03";
const char PEER_C[] = "04";

enum class PeerType {
	Light, // Doesn't keep db. Connects to 20 peers max
	Semi,  // Keeps headers? 35 peers max
	Full   // Keeps full database. 50 peers max
};


struct Peer {
    std::string public_key;                 // Public key of the peer
    asio::ip::udp::endpoint endpoint;       // Network endpoint (IP address and port)
    PeerType peer_type = PeerType::Light;   // Type of the peer (Light, Semi, Full)

    Peer() {};
    Peer(const std::string& pub_key, const asio::ip::udp::endpoint& ep, PeerType type)
        : public_key(pub_key), endpoint(ep), peer_type(type) {}
};


inline void to_json(json& j, const Peer& p) {
    j = json{
        {"public_key", p.public_key},
        {"ip_address", p.endpoint.address().to_string()},
        {"port", p.endpoint.port()},
        {"peer_type", static_cast<int>(p.peer_type)}
    };
}

inline void from_json(const json& j, Peer& p) {
    std::string ip_address = j.at("ip_address").get<std::string>();
    uint16_t port = j.at("port").get<uint16_t>();
    p.public_key = j.at("public_key").get<std::string>();
    p.endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(ip_address), port);
    p.peer_type = static_cast<PeerType>(j.at("peer_type").get<int>());
}