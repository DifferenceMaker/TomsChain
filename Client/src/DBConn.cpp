#include "DBConn.h"

DBConn::DBConn()
    :m_stateTrieDB("databases/stateTrie_db"), m_transactionTrieDB("databases/transactionTrie_db"){
    /*leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB* temp_stateTrie;
    leveldb::DB* temp_transactionTrie;
    leveldb::DB* temp_blockDB;


    leveldb::Status statusOfStateTrie = leveldb::DB::Open(options, "stateTrie_db", &temp_stateTrie);
    if (!statusOfStateTrie.ok()) {
        std::cerr << "Unable to open/create test database 'stateTrie_db'" << std::endl;
        std::cerr << statusOfStateTrie.ToString() << std::endl;
        throw std::runtime_error("Failed to open database");
    }

    leveldb::Status statusOfTransactionTrie = leveldb::DB::Open(options, "transactionTrie_db", &temp_transactionTrie);
    if (!statusOfTransactionTrie.ok()) {
        std::cerr << "Unable to open/create test database 'transactionTrie_db'" << std::endl;
        std::cerr << statusOfTransactionTrie.ToString() << std::endl;
        throw std::runtime_error("Failed to open database");
    }

    // block database
    leveldb::Status statusOfBlockDB = leveldb::DB::Open(options, "block_db", &temp_blockDB);
    if (!statusOfBlockDB.ok()) {
        std::cerr << "Unable to open/create block_db" << std::endl;
        std::cerr << statusOfBlockDB.ToString() << std::endl;
        throw std::runtime_error("Failed to open database");
    }

    m_stateTrieDB = std::unique_ptr<leveldb::DB>(temp_stateTrie);
    m_transactionTrieDB = std::unique_ptr<leveldb::DB>(temp_transactionTrie);
    m_blockDB = std::unique_ptr<leveldb::DB>(temp_blockDB);*/

    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::DB* temp_blockDB;

    leveldb::Status statusOfBlockDB = leveldb::DB::Open(options, "databases/block_db", &temp_blockDB);
    if (!statusOfBlockDB.ok()) {
        std::cerr << "Unable to open/create block_db" << std::endl;
        std::cerr << statusOfBlockDB.ToString() << std::endl;
        throw std::runtime_error("Failed to open database");
    }

    m_blockDB = std::unique_ptr<leveldb::DB>(temp_blockDB);
}

DBConn::~DBConn() {}

/*
    std::string value;
    leveldb::Status s = db->Get(leveldb::ReadOptions(), key1, &value);
    if (s.ok()) s = db->Put(leveldb::WriteOptions(), key2, value);
    if (s.ok()) s = db->Delete(leveldb::WriteOptions(), key1);

    https://github.com/google/leveldb/blob/main/doc/index.md
*/

void DBConn::AddBlockToChain(Block _block){
    // Add block to blockchain which contains header, transactions, receipts...


}

void DBConn::UpdateStateTrie(std::vector<Transaction> _transactions){

}
