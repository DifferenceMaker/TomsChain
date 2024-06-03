#include "RLP.h"

// Helper function to convert integer to hexadecimal string
static std::string intToHex(uint64_t input) {
    std::ostringstream ss;
    ss << std::hex << input;
    return ss.str();
}

// Helper function to convert hexadecimal string to byte array
std::vector<uint8_t> RLP::hexStringToByteArr(const std::string& input) {
    std::string hexStr = stripHexPrefix(input);
    size_t len = hexStr.length();
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < len; i += 2) {
        std::string byteString = hexStr.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

// Helper function to remove '0x' prefix from hexadecimal string
std::string RLP::stripHexPrefix(const std::string& input) {
    if (input.rfind("0x", 0) == 0) {
        return input.substr(2);
    }
    return input;
}

// Main encoding function
std::vector<uint8_t> RLP::encode(const Input& input) {
    if (std::holds_alternative<std::string>(input)) {
        const std::string& strInput = std::get<std::string>(input);
        if (strInput.empty()) {
            return { 0x80 };
        }
        return encodeString(strInput);
    }
    else if (std::holds_alternative<int>(input) || std::holds_alternative<uint64_t>(input)) {
        uint64_t intInput = std::holds_alternative<int>(input) ? std::get<int>(input) : std::get<uint64_t>(input);
        return encodeInteger(intInput);
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(input)) {
        return encodeBytes(std::get<std::vector<uint8_t>>(input));
    }
    else if (std::holds_alternative<RecursiveList>(input)) {
        return encodeList(std::get<RecursiveList>(input));
    }
    else {
        throw std::invalid_argument("Unhandled input type");
    }
}

std::vector<uint8_t> RLP::encodeString(const std::string& input) {
    std::vector<uint8_t> result;
    if (input.length() == 1 && static_cast<uint8_t>(input[0]) <= 0x7f) {
        result.push_back(static_cast<uint8_t>(input[0]));
    }
    else {
        std::vector<uint8_t> bytes(input.begin(), input.end());
        if (bytes.size() <= 55) {
            result.push_back(0x80 + bytes.size());
        }
        else {
            std::string lenStr = intToHex(bytes.size());
            std::vector<uint8_t> lenBytes = hexStringToByteArr(lenStr);
            result.push_back(0xb7 + lenBytes.size());
            result.insert(result.end(), lenBytes.begin(), lenBytes.end());
        }
        result.insert(result.end(), bytes.begin(), bytes.end());
    }
    return result;
}

std::vector<uint8_t> RLP::encodeInteger(uint64_t input) {
    std::vector<uint8_t> result;
    if (input == 0) {
        result.push_back(0x80);
    }
    else if (input <= 0x7f) {
        result.push_back(static_cast<uint8_t>(input));
    }
    else {
        std::string hexStr = intToHex(input);
        std::vector<uint8_t> bytes = hexStringToByteArr(hexStr);
        result.push_back(0x80 + bytes.size());
        result.insert(result.end(), bytes.begin(), bytes.end());
    }
    return result;
}

std::vector<uint8_t> RLP::encodeBytes(const std::vector<uint8_t>& input) {
    std::vector<uint8_t> result;
    if (input.empty()) {
        result.push_back(0x80);
    }
    else {
        if (input.size() <= 55) {
            result.push_back(0x80 + input.size());
        }
        else {
            std::string lenStr = intToHex(input.size());
            std::vector<uint8_t> lenBytes = hexStringToByteArr(lenStr);
            result.push_back(0xb7 + lenBytes.size());
            result.insert(result.end(), lenBytes.begin(), lenBytes.end());
        }
        result.insert(result.end(), input.begin(), input.end());
    }
    return result;
}

std::vector<uint8_t> RLP::encodeList(const RecursiveList& input) {
    std::vector<uint8_t> result;
    std::vector<uint8_t> encodedItems;
    for (const auto& item : input.items) {
        std::vector<uint8_t> encodedItem = encode(item);
        encodedItems.insert(encodedItems.end(), encodedItem.begin(), encodedItem.end());
    }
    if (encodedItems.size() <= 55) {
        result.push_back(0xc0 + encodedItems.size());
    }
    else {
        std::string lenStr = intToHex(encodedItems.size());
        std::vector<uint8_t> lenBytes = hexStringToByteArr(lenStr);
        result.push_back(0xf7 + lenBytes.size());
        result.insert(result.end(), lenBytes.begin(), lenBytes.end());
    }
    result.insert(result.end(), encodedItems.begin(), encodedItems.end());
    return result;
}


RLP::Input RLP::decode(const std::vector<uint8_t>& data) {
    size_t offset = 0;
    return decodeItem(data, offset);
}

RLP::Input RLP::decodeItem(const std::vector<uint8_t>& data, size_t& offset) {
    if (offset >= data.size()) {
        throw std::runtime_error("RLP decode error: out of bounds");
    }

    uint8_t prefix = data[offset++];

    if (prefix <= 0x7F) {
        return std::string(1, prefix);
    }
    else if (prefix <= 0xB7) {
        size_t len = prefix - 0x80;
        if (offset + len > data.size()) {
            throw std::runtime_error("RLP decode error: length out of bounds");
        }
        std::vector<uint8_t> bytes(data.begin() + offset, data.begin() + offset + len);
        offset += len;
        return bytes;
    }
    else if (prefix <= 0xBF) {
        size_t lenLen = prefix - 0xB7;
        if (offset + lenLen > data.size()) {
            throw std::runtime_error("RLP decode error: length out of bounds");
        }
        size_t len = 0;
        for (size_t i = 0; i < lenLen; ++i) {
            len = (len << 8) | data[offset++];
        }
        if (offset + len > data.size()) {
            throw std::runtime_error("RLP decode error: length out of bounds");
        }
        std::vector<uint8_t> bytes(data.begin() + offset, data.begin() + offset + len);
        offset += len;
        return bytes;
    }
    else if (prefix <= 0xF7) {
        size_t len = prefix - 0xC0;
        std::vector<RLP::Input> list;
        size_t end = offset + len;
        while (offset < end) {
            list.push_back(decodeItem(data, offset));
        }
        return RecursiveList{ list };
    }
    else {
        size_t lenLen = prefix - 0xF7;
        if (offset + lenLen > data.size()) {
            throw std::runtime_error("RLP decode error: length out of bounds");
        }
        size_t len = 0;
        for (size_t i = 0; i < lenLen; ++i) {
            len = (len << 8) | data[offset++];
        }
        if (offset + len > data.size()) {
            throw std::runtime_error("RLP decode error: length out of bounds");
        }
        std::vector<RLP::Input> list;
        size_t end = offset + len;
        while (offset < end) {
            list.push_back(decodeItem(data, offset));
        }
        return RecursiveList{ list };
    }
}