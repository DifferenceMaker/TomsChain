#ifndef RLP_H
#define RLP_H

#include <string>
#include <vector>
#include <variant>
#include <stdexcept>
#include <sstream>
#include <iomanip>

class RLP {
public:
    struct RecursiveList;

    using Input = std::variant<std::string, int, uint64_t, std::vector<uint8_t>, RecursiveList>;

    struct RecursiveList {
        std::vector<Input> items;
    };

    static std::vector<uint8_t> encode(const Input& input);
    static Input decode(const std::vector<uint8_t>& data);

private:
    static std::vector<uint8_t> encodeString(const std::string& input);
    static std::vector<uint8_t> encodeInteger(uint64_t input);
    static std::vector<uint8_t> encodeList(const RecursiveList& input);
    static std::vector<uint8_t> encodeBytes(const std::vector<uint8_t>& input);

    static Input decodeItem(const std::vector<uint8_t>& data, size_t& offset);

    static std::vector<uint8_t> hexStringToByteArr(const std::string& input);
    static std::string stripHexPrefix(const std::string& input);
};

#endif // RLP_H
