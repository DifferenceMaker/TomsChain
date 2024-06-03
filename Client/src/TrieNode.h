#ifndef TRIENODE_H
#define TRIENODE_H

#include <vector>
#include <memory>

enum class NodeType { LEAF, EXTENSION, BRANCH };

struct TrieNode {
    NodeType type;
    std::vector<uint8_t> key;
    std::vector<uint8_t> value;
    std::vector<std::shared_ptr<TrieNode>> children;

    TrieNode(NodeType type) : type(type) {
        if (type == NodeType::BRANCH) {
            children.resize(16);
        }
    }
};

#endif // TRIENODE_H
