#pragma once
#include "Block.h"

Block::Block(Hash32 _parentHash, std::vector<Transaction> _transactions, uint64_t _timestamp)
	:m_Transactions(_transactions){

	BLOCK_HEADER.parentHash = _parentHash;
    BLOCK_HEADER.transactionsRoot = hashTransactions();
    
    //BLOCK_HEADER.stateRoot = ...;

}

std::array<uint8_t, 32> Block::hashTransactions(){
    std::vector<std::array<uint8_t, 32>> transactionHashes; 

    for (const Transaction& transaction : m_Transactions) {
		std::array<uint8_t, 32> txHash = transaction.computeTXHash();
		transactionHashes.push_back(txHash);
    }

    while (transactionHashes.size() > 1) {
        std::vector<std::array<uint8_t, 32>> tempHashes;

        // Combine adjacent hashes to form parent hashes in the tree
        for (size_t i = 0; i < transactionHashes.size(); i += 2) {
            std::array<uint8_t, 64> combinedHashes;
            memcpy(&combinedHashes[0], transactionHashes[i].data(), 32);
            if (i + 1 < transactionHashes.size()) {
                memcpy(&combinedHashes[32], transactionHashes[i + 1].data(), 32);
            }
            else {
                memcpy(&combinedHashes[32], transactionHashes[i].data(), 32);
            }

            // Hash the combined hashes
            std::array<uint8_t, 32> parentHash;
            SHA256(combinedHashes.data(), combinedHashes.size(), parentHash.data());
            tempHashes.push_back(parentHash);
        }

        // Update the transactionHashes vector with the new set of hashes
        transactionHashes = tempHashes;
    }

    // Return the root hash of the Merkle tree (or the transaction root)
    return transactionHashes.empty() ? std::array<uint8_t, 32>{} : transactionHashes[0];
}
