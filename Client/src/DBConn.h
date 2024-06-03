#pragma once
#include <cassert>
#include "Transaction.h"
#include "Block.h"
#include "MerklePatriciaTrie.h"

// IMPLEMENT CONSENSUS MECHANISM

class DBConn
{
public:
	DBConn();
	~DBConn();

	void AddBlockToChain(Block _block);
	void UpdateStateTrie(std::vector<Transaction> _transactions);

	//bool verifyTransaction(Transaction _tx);
private:

	MerklePatriciaTrie m_stateTrieDB;
	MerklePatriciaTrie m_transactionTrieDB;
	
	std::unique_ptr<leveldb::DB> m_blockDB;
	//MerklePatriciaTrie m_blockDB;
};

