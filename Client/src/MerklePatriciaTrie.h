#ifndef MERKLEPATRICIATRIE_H
#define MERKLEPATRICIATRIE_H

#include <memory>
#include <iostream>
#include <leveldb/db.h>
#include "TrieNode.h"
#include "Utils/RLP.h"

// An "Abstract" class for Merkle Patricia Trie and leveldb functionality with RLP encoding
// DBConn.h uses this to add RLP encoded data to the blockchain databases.

class MerklePatriciaTrie {
public:
    MerklePatriciaTrie(const std::string& dbPath);

    void insert(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    // update
    // delete

private:
    std::unique_ptr<leveldb::DB> db_;
    std::shared_ptr<TrieNode> root;

    std::shared_ptr<TrieNode> insertRecursive(std::shared_ptr<TrieNode> node, const std::vector<uint8_t>& key, const std::vector<uint8_t>& value, size_t depth);
    std::shared_ptr<TrieNode> getNode(const std::vector<uint8_t>& key, size_t depth);

    std::vector<uint8_t> serializeNode(const std::shared_ptr<TrieNode>& node);
    std::shared_ptr<TrieNode> deserializeNode(const std::vector<uint8_t>& data);
    std::vector<uint8_t> toBytes(const std::string& str);
    std::string fromBytes(const std::vector<uint8_t>& bytes);
};

#endif // MERKLEPATRICIATRIE_H
