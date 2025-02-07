#include "util.h"
#include "cn_utils/cnutils.h"
#include "stratum/stratum/rpc.h"
#include "stratum/stratum/structures.h"
#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>
#include <cctype>
#include <chrono>
#include <crypto/common.h>
#include <crypto/hash-ops.h>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace boost::multiprecision;


namespace util
{

std::string stripHexPrefix(const std::string& hex)
{
    if (hex.size() > 2 && (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))) {
        return hex.substr(2);
    }
    return hex;
}

bool random_bytes(uint8_t* buffer, size_t length)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = static_cast<uint8_t>(dis(gen));
    }
    return true;
}

int64_t MakeTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<int64_t>(ms.count());
}

std::vector<unsigned char> bigIntToBytes(const mp::uint256_t& num, size_t byte_size)
{
    // uint256_t is 256 bits, which is 32 bytes
    const size_t max_byte_size = 32;
    if (byte_size > max_byte_size) {
        throw std::invalid_argument("byte_size cannot exceed 32 bytes for uint256_t.");
    }

    std::vector<unsigned char> bytes(byte_size, 0);

    mp::uint256_t temp = num;
    for (size_t i = 0; i < byte_size; ++i) {
        bytes[byte_size - 1 - i] = static_cast<unsigned char>(temp & 0xFF);
        temp >>= 8;
    }

    return bytes;
}

std::pair<mp::uint256_t, bool> GetHashDifficulty(const std::vector<unsigned char>& hashBytes)
{
    if (hashBytes.empty()) {
        return {mp::uint256_t(0), false};
    }

    std::vector<unsigned char> rev(hashBytes.rbegin(), hashBytes.rend());

    if (rev.size() < 32) {
        rev.insert(rev.begin(), 32 - rev.size(), 0);
    } else if (rev.size() > 32) {
        rev.erase(rev.begin(), rev.begin() + (rev.size() - 32));
    }

    mp::uint256_t val = 0;
    for (size_t i = 0; i < 32; ++i) {
        val = (val << 8) | static_cast<uint8_t>(rev[i]);
    }

    if (val == 0) {
        return {mp::uint256_t(0), false};
    }

    mp::uint256_t Diff1("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    mp::uint256_t result = Diff1 / val;

    return {result, true};
}

uint32_t bytesToUint32(const std::vector<uint8_t>& bytes)
{
    uint32_t value = 0;
    if (bytes.size() >= 4) {
        std::memcpy(&value, bytes.data(), sizeof(uint32_t));
    }
    return value;
}

std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t pos = s.find(delimiter);

    while (pos != std::string::npos) {
        tokens.emplace_back(s.substr(start, pos - start));
        start = pos + 1;
        pos = s.find(delimiter, start);
    }

    tokens.emplace_back(s.substr(start));
    return tokens;
}

std::vector<uint8_t> hexDecode(const std::string& hexStr)
{
    std::vector<uint8_t> bytes;

    if (hexStr.length() % 2 != 0) {
        throw std::invalid_argument("hexDecode: Hex string has an odd length.");
    }

    // Preallocate space for efficiency
    bytes.reserve(hexStr.length() / 2);

    // Lambda to convert a single hex character to its integer value
    auto hexCharToInt = [](char c) -> uint8_t {
        if ('0' <= c && c <= '9') {
            return c - '0';
        } else if ('a' <= c && c <= 'f') {
            return 10 + (c - 'a');
        } else if ('A' <= c && c <= 'F') {
            return 10 + (c - 'A');
        } else {
            throw std::invalid_argument("hexDecode: Invalid hexadecimal character encountered.");
        }
    };

    for (size_t i = 0; i < hexStr.length(); i += 2) {
        char highChar = hexStr[i];
        char lowChar = hexStr[i + 1];

        // Convert each pair of hex characters to a byte
        uint8_t highNibble = hexCharToInt(highChar);
        uint8_t lowNibble = hexCharToInt(lowChar);
        uint8_t byte = (highNibble << 4) | lowNibble;

        bytes.push_back(byte);
    }

    return bytes;
}

std::vector<uint8_t> hexStringToBytes(const std::string& hex)
{
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.length() / 2);
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string bytesToHexString(const std::vector<uint8_t>& bytes)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (auto byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }
    return ss.str();
}

std::vector<uint8_t> reverseBytes(const std::vector<uint8_t>& bytes)
{
    std::vector<uint8_t> reversed = bytes;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

std::string GetTargetHex(int64_t diff)
{
    if (diff <= 0) {
        // Fallback target if difficulty is invalid
        return "00000000";
    }

    mp::uint256_t Diff1 = mp::uint256_t("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    mp::uint256_t targetBig = Diff1 / diff;
    std::vector<unsigned char> padded = bigIntToBytes(targetBig, 32);
    std::vector<unsigned char> buff(padded.begin(), padded.begin() + 4);
    std::reverse(buff.begin(), buff.end());

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned char c : buff) {
        oss << std::setw(2) << static_cast<int>(c);
    }

    return oss.str();
}

mp::uint256_t bytesToBigInt(const std::vector<uint8_t>& bytes)
{
    mp::uint256_t value = 0;
    for (auto byte : bytes) {
        value = (value << 8) | byte;
    }
    return value;
}

int64_t GetDiffFromHex(const std::string& targetHex)
{
    std::vector<uint8_t> reversedBuff = hexStringToBytes(targetHex);
    std::reverse(reversedBuff.begin(), reversedBuff.end());

    std::vector<uint8_t> padded(32, 0);
    if (reversedBuff.size() > 32) {
        throw std::runtime_error("Hex input too long.");
    }
    std::copy(reversedBuff.begin(), reversedBuff.end(), padded.begin());
    mp::uint256_t targetBig = bytesToBigInt(padded);
    mp::uint256_t Diff1("0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    mp::uint256_t diffBig = Diff1 / targetBig;

    if (diffBig > std::numeric_limits<int64_t>::max()) {
        throw std::runtime_error("Decoded diff exceeds int64_t limit.");
    }
    return static_cast<int64_t>(diffBig);
}

std::vector<uint8_t> ConvertBlob(const std::vector<uint8_t>& blob)
{
    if (blob.empty()) {
        throw std::invalid_argument("Blob is empty");
    }

    std::vector<uint8_t> output(76);
    convert_blob(reinterpret_cast<const char*>(blob.data()), static_cast<uint32_t>(blob.size()), reinterpret_cast<char*>(output.data()));
    return output;
}

bool ValidateAddress(const std::string& addr)
{
    if (addr.empty()) {
        return false;
    }
    return validate_address(addr.c_str(), static_cast<uint32_t>(addr.size()));
}

std::vector<uint8_t> Hash(const std::vector<uint8_t>& blob, bool fast, int height, const std::string& seedHash)
{
    std::vector<uint8_t> output(32);
    if (fast) {
        crypto::cn_fast_hash(reinterpret_cast<const char*>(blob.data()), static_cast<uint32_t>(blob.size()), reinterpret_cast<char*>(output.data()));
    } else if (!seedHash.empty()) {
        uint64_t seedHeight = crypto::rx_seedheight(static_cast<uint64_t>(height));
        std::vector<uint8_t> cnHash = util::hexDecode(seedHash);
        crypto::rx_slow_hash(static_cast<uint64_t>(height), seedHeight, reinterpret_cast<const char*>(cnHash.data()),
            reinterpret_cast<const char*>(blob.data()), static_cast<uint64_t>(blob.size()),
            reinterpret_cast<char*>(output.data()), 0, 0);
    } else {
        crypto::cn_slow_hash(reinterpret_cast<const char*>(blob.data()), static_cast<uint32_t>(blob.size()), reinterpret_cast<char*>(output.data()), 0, 0, height);
    }
    return output;
}

std::vector<uint8_t> FastHash(const std::vector<uint8_t>& blob)
{
    return Hash(blob, true, 0, "");
}

} // namespace util
