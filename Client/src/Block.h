#pragma once
#include <array>
#include "Transaction.h"
#include <vector>

// TO CHECK
// create a transaction pool / mempool / pending transaction list 
// implement Merkle tree rooting for block's hash

// Generate / calculate within class - stateRoot, transactionRoot, receiptRoot, nonce, difficulty, number, gasLimit, gasUsed

// "This storage can be implemented using databases like LevelDB, RocksDB, or custom databases optimized for blockchain data."

/*
LevelDB Key-Value Pairs:

Each key-value pair in LevelDB represents a node in the Merkle Trie.
Keys might be hashes or pointers to specific nodes in the trie.
Values would contain the actual account balances or contract data, hashed or organized to fit the trie structure.
*/

/*
!!
Save the blocks numerated - 1, 2, 3, 4 and their value is the block hash.
then the value of the block hash (key) is the block header. 
!!


*/ 

using Hash32 = std::array<uint8_t, 32>; // 256-bit hash represented as an array of bytes

struct BlockHeader {
	Hash32 parentHash;
	Hash32 OmmerHash;
	Hash32 transactionsRoot; // The root hash of the TRANSACTION TRIE
	Hash32 stateRoot; //  The root hash of the state trie AFTER ALL transactions in the block are processed

	std::array<uint8_t, 20> beneficiary; // Coinbase

	uint64_t number; // number of ancestor blocks;
	uint64_t gasLimit;
	uint64_t gasUsed;

	uint64_t timestamp;
	uint64_t nonce; // to find certain number of leading 0s
	uint64_t difficulty;
};

class Block {
public:
	Block(Hash32 _parentHash, std::vector<Transaction> _transactions, uint64_t timestamp);

private:
	BlockHeader BLOCK_HEADER;
	std::vector<Transaction> m_Transactions;
	std::vector<Block> m_Uncles;

	Hash32 hashTransactions(); // KECCAK 256-bit HASH // Is this necessary?
	//Hash32 calculateStateRoot();
	// FOR THIS ^ :
	// 
	
};