#include "Transaction.h"

Transaction::Transaction(const std::array<uint8_t, 125>& buffer) {
    Deserialize(buffer);
}

std::array<uint8_t, 32> Transaction::computeTXHash() const {

    std::ostringstream txStream;

    txStream << nonce << sender.data() << recipient.data() << gasPrice << gasLimit << value << timestamp;
    std::string txData = txStream.str();

    std::array<uint8_t, 32> hash;
    SHA256(reinterpret_cast<const unsigned char*>(txData.c_str()), txData.size(), hash.data());

    return hash;
}

std::array<uint8_t, 125> Transaction::Serialize() const{
    // Calculate the size of the buffer based on the size of each attribute
    constexpr size_t bufferSize = sizeof(uint32_t) * 5 + sizeof(sender) + sizeof(recipient) + sizeof(uint8_t) + sizeof(r) + sizeof(s);
    std::array<uint8_t, bufferSize> buffer;

    size_t offset = 0;
    memcpy(buffer.data() + offset, &nonce, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer.data() + offset, sender.data(), sender.size());
    offset += sender.size();

    memcpy(buffer.data() + offset, recipient.data(), recipient.size());
    offset += recipient.size();

    memcpy(buffer.data() + offset, &gasPrice, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer.data() + offset, &gasLimit, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer.data() + offset, &value, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer.data() + offset, &timestamp, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(buffer.data() + offset, &v, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    memcpy(buffer.data() + offset, r.data(), r.size());
    offset += r.size();

    memcpy(buffer.data() + offset, s.data(), s.size());
    offset += s.size();

    return buffer;
}

void Transaction::Deserialize(const std::array<uint8_t, 125>& buffer) {
    size_t offset = 0;

    memcpy(&nonce, buffer.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(sender.data(), buffer.data() + offset, sender.size());
    offset += sender.size();

    memcpy(recipient.data(), buffer.data() + offset, recipient.size());
    offset += recipient.size();

    memcpy(&gasPrice, buffer.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&gasLimit, buffer.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&value, buffer.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&timestamp, buffer.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(&v, buffer.data() + offset, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    memcpy(r.data(), buffer.data() + offset, r.size());
    offset += r.size();

    memcpy(s.data(), buffer.data() + offset, s.size());
    offset += s.size();

    //std::cout << "Offset = " << offset << std::endl;
}

void Transaction::printTx() const {
    // Save current format flags
    std::ios_base::fmtflags f(std::cout.flags());

    // Convert wallet_address to a hexadecimal string
    std::ostringstream sender_address_str;
    std::ostringstream recipient_address_str;
    sender_address_str << std::hex << std::setfill('0');
    recipient_address_str << std::hex << std::setfill('0');

    for (const auto& byte : sender) {
        sender_address_str << std::setw(2) << static_cast<int>(byte);
    }
    for (const auto& byte : recipient) {
        recipient_address_str << std::setw(2) << static_cast<int>(byte);
    }

    std::cout << "\nNonce: " << nonce <<
        "\nSender: " << sender_address_str.str() <<
        "\nRecipient: " << recipient_address_str.str() <<
        "\nGas Price: " << gasPrice <<
        "\nGas Limit: " << gasLimit <<
        "\nAmount: " << value <<
        "\nTimestamp: " << timestamp << "\n";

    std::cout << "v: " << static_cast<int>(v) << "\n";
    std::cout << "r: ";
    for (const uint8_t& byte : r) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << "\n";

    std::cout << "s: ";
    for (const uint8_t& byte : s) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << "\n";

    std::array<uint8_t, 32> transactionHash = computeTXHash();
    std::cout << "Transaction Hash: ";
    // Print the hash as a hexadecimal string
    for (const uint8_t& byte : transactionHash) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << "\n\n";

    // Restore original format flags
    std::cout.flags(f);
}