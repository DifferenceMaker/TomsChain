#pragma once
#include <iostream>
#include <array>
#include <openssl/sha.h>
#include <sstream>
#include <chrono>

class Transaction {
public:

	Transaction() = default;
	Transaction(const uint32_t _nonce, const std::array<uint8_t, 20> _sender, const std::array<uint8_t, 20> _recipient,
		const uint32_t _gasPrice, const uint32_t _gasLimit, const uint32_t _value)
		: nonce(_nonce), sender(_sender), recipient(_recipient),
		gasPrice(_gasPrice), gasLimit(_gasLimit), value(_value),
		v(0), r({}), s({}) {

		const auto p1 = std::chrono::system_clock::now();
		timestamp = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
	}

	Transaction(const std::array<uint8_t, 125>& buffer);

	void signTransaction(uint8_t _v, std::array<uint8_t, 32> _r, std::array<uint8_t, 32> _s) {
		v = _v;
		r = _r;
		s = _s;
	}

	std::array<uint8_t, 32> computeTXHash() const; // External function
	std::array<uint8_t, 125> Serialize() const;
	void Deserialize(const std::array<uint8_t, 125>& buffer);
	
	void printTx() const;

	std::array<uint8_t, 20> sender;
	uint32_t nonce;
	std::array<uint8_t, 20> recipient;

	uint32_t gasPrice;
	uint32_t gasLimit;
	uint32_t value;
	uint32_t timestamp;

	uint8_t v;
	std::array<uint8_t, 32> r;
	std::array<uint8_t, 32> s;
};