#include "MerklePatriciaTrie.h"

MerklePatriciaTrie::MerklePatriciaTrie(const std::string& dbPath) {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::DB* temp_db;
    leveldb::Status status = leveldb::DB::Open(options, dbPath, &temp_db);
    if (!status.ok()) {
        std::cerr << "Unable to open/create test database: " << dbPath << std::endl;
        std::cerr << status.ToString() << std::endl;
        throw std::runtime_error("Failed to open database");
    }

    db_.reset(temp_db);
    root = std::make_shared<TrieNode>(NodeType::BRANCH);  // Start with an empty branch node
}

std::vector<uint8_t> MerklePatriciaTrie::toBytes(const std::string& str) {
    return std::vector<uint8_t>(str.begin(), str.end());
}

std::string MerklePatriciaTrie::fromBytes(const std::vector<uint8_t>& bytes) {
    return std::string(bytes.begin(), bytes.end());
}

std::vector<uint8_t> MerklePatriciaTrie::serializeNode(const std::shared_ptr<TrieNode>& node) {
    RLP::RecursiveList rlpList;
    if (node->type == NodeType::LEAF || node->type == NodeType::EXTENSION) {
        rlpList.items.push_back(RLP::encode(node->key));
        rlpList.items.push_back(RLP::encode(node->value));
    }
    if (node->type == NodeType::BRANCH) {
        for (const auto& child : node->children) {
            if (child) {
                rlpList.items.push_back(RLP::encode(serializeNode(child)));
            } else {
                rlpList.items.push_back(RLP::encode(""));
            }
        }
    }
    return RLP::encode(rlpList);
}

std::shared_ptr<TrieNode> MerklePatriciaTrie::deserializeNode(const std::vector<uint8_t>& data) {
    RLP::Input rlpInput = RLP::decode(data);

    if (!std::holds_alternative<RLP::RecursiveList>(rlpInput)) {
        throw std::runtime_error("Invalid RLP for TrieNode");
    }

    const RLP::RecursiveList& rlpList = std::get<RLP::RecursiveList>(rlpInput);
    auto node = std::make_shared<TrieNode>(NodeType::BRANCH);

    if (rlpList.items.size() == 2) {
        if (std::holds_alternative<std::vector<uint8_t>>(rlpList.items[0])) {
            node = std::make_shared<TrieNode>(NodeType::EXTENSION);
        }
        else {
            node = std::make_shared<TrieNode>(NodeType::LEAF);
        }
        node->key = std::get<std::vector<uint8_t>>(rlpList.items[0]);
        node->value = std::get<std::vector<uint8_t>>(rlpList.items[1]);
    }
    else {
        for (size_t i = 0; i < rlpList.items.size(); ++i) {
            if (std::holds_alternative<std::vector<uint8_t>>(rlpList.items[i])) {
                node->children[i] = deserializeNode(std::get<std::vector<uint8_t>>(rlpList.items[i]));
            }
            else {
                node->children[i] = nullptr;
            }
        }
    }
    return node;
}

// Insert a key-value pair into the trie
void MerklePatriciaTrie::insert(const std::string& key, const std::string& value) {
    std::vector<uint8_t> keyBytes = toBytes(key);
    std::vector<uint8_t> valueBytes = toBytes(value);
    root = insertRecursive(root, keyBytes, valueBytes, 0);
}

// Recursive helper function to insert a key-value pair into the trie
std::shared_ptr<TrieNode> MerklePatriciaTrie::insertRecursive(
    std::shared_ptr<TrieNode> node,
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& value,
    size_t depth) {

    if (!node) {
        // Create a new LEAF node if the current node is null
        auto leafNode = std::make_shared<TrieNode>(NodeType::LEAF);
        leafNode->key = std::vector<uint8_t>(key.begin() + depth, key.end());
        leafNode->value = value;
        std::vector<uint8_t> serialized = serializeNode(leafNode);
        db_->Put(leveldb::WriteOptions(), fromBytes(key), fromBytes(serialized));
        return leafNode;
    }

    if (node->type == NodeType::LEAF) {
        // Handle collision with an existing leaf node
        if (node->key == std::vector<uint8_t>(key.begin() + depth, key.end())) {
            node->value = value;
            std::vector<uint8_t> serialized = serializeNode(node);
            db_->Put(leveldb::WriteOptions(), fromBytes(key), fromBytes(serialized));
            return node;
        }
        else {
            // Split the node to handle the collision
            auto newBranch = std::make_shared<TrieNode>(NodeType::BRANCH);
            size_t commonPrefix = 0;
            while (commonPrefix < node->key.size() && commonPrefix < key.size() - depth && 
                    node->key[commonPrefix] == key[depth + commonPrefix]) {
                commonPrefix++;
            }

            if (commonPrefix > 0) {
                auto extensionNode = std::make_shared<TrieNode>(NodeType::EXTENSION);
                extensionNode->key = std::vector<uint8_t>(key.begin() + depth, key.begin() + depth + commonPrefix);
                extensionNode->children[0] = newBranch;
                node = extensionNode;
            }

            newBranch->children[node->key[commonPrefix]] = node;
            newBranch->children[key[depth + commonPrefix]] = insertRecursive(nullptr, key, value, depth + commonPrefix + 1);
            return newBranch;
        }
    }

    if (node->type == NodeType::BRANCH) {
        size_t index = key[depth];
        node->children[index] = insertRecursive(node->children[index], key, value, depth + 1);
    }

    if (node->type == NodeType::EXTENSION) {
        size_t commonPrefix = 0;
        while (commonPrefix < node->key.size() && commonPrefix < key.size() - depth &&
            node->key[commonPrefix] == key[depth + commonPrefix]) {
            commonPrefix++;
        }

        if (commonPrefix == node->key.size()) {
            node->children[0] = insertRecursive(node->children[0], key, value, depth + commonPrefix);
        }
        else {
            auto newBranch = std::make_shared<TrieNode>(NodeType::BRANCH);
            newBranch->children[node->key[commonPrefix]] = node;
            newBranch->children[key[depth + commonPrefix]] = insertRecursive(nullptr, key, value, depth + commonPrefix + 1);
            node->key = std::vector<uint8_t>(node->key.begin() + commonPrefix, node->key.end());
            node = newBranch;
        }
    }

    std::vector<uint8_t> serialized = serializeNode(node);
    db_->Put(leveldb::WriteOptions(), fromBytes(key), fromBytes(serialized));
    return node;
}

// Get the value associated with a key
std::string MerklePatriciaTrie::get(const std::string& key) {
    std::vector<uint8_t> keyBytes = toBytes(key);
    auto node = getNode(keyBytes, 0);
    return fromBytes(node->value);
}

// Recursive helper function to get the node associated with a key
std::shared_ptr<TrieNode> MerklePatriciaTrie::getNode(const std::vector<uint8_t>& key, size_t depth) {
    std::string serializedNode;
    db_->Get(leveldb::ReadOptions(), fromBytes(key), &serializedNode);
    return deserializeNode(toBytes(serializedNode));
}
