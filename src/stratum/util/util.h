#ifndef UTIL_H
#define UTIL_H

#include "cn_utils/cnutils.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace mp = boost::multiprecision;

namespace rpc
{
class RPCClient;
struct ValidateAddressReply;
}

namespace util
{

int64_t MakeTimestamp();
std::pair<mp::uint256_t, bool> GetHashDifficulty(const std::vector<unsigned char>& hashBytes);

bool random_bytes(uint8_t* buffer, size_t length);
uint32_t bytesToUint32(const std::vector<uint8_t>& bytes);
std::vector<std::string> split(const std::string& s, char delimiter);
std::vector<uint8_t> hexDecode(const std::string& hexStr);
std::string bytesToHex(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> hexStringToBytes(const std::string& hex);
mp::uint256_t bytesToBigInt(const std::vector<uint8_t>& bytes);
std::string bytesToHexString(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> reverseBytes(const std::vector<uint8_t>& bytes);
std::string GetTargetHex(int64_t diff);
int64_t GetDiffFromHex(const std::string& targetHex);
std::vector<uint8_t> ConvertBlob(const std::vector<uint8_t>& blob);
bool ValidateAddress(const std::string& addr);
std::vector<uint8_t> Hash(const std::vector<uint8_t>& blob, bool fast, int height, const std::string& seedHash);
std::vector<uint8_t> FastHash(const std::vector<uint8_t>& blob);

} // namespace util

#endif // UTIL_H
