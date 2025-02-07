// blocktemplate.cpp
#include "blocktemplate.h"
#include "stratum/util/util.h"
#include <array>
#include <vector>

namespace stratum
{

std::string BlockTemplate::nextBlob(uint32_t extraNonce, const std::array<uint8_t, 3>& instanceId)
{
    // Convert extraNonce to big-endian bytes
    std::vector<uint8_t> extraBuff(4);
    extraBuff[0] = (extraNonce >> 24) & 0xFF;
    extraBuff[1] = (extraNonce >> 16) & 0xFF;
    extraBuff[2] = (extraNonce >> 8) & 0xFF;
    extraBuff[3] = extraNonce & 0xFF;

    // Prepare blob buffer
    std::vector<uint8_t> blobBuff = blob_buffer;
    if (blobBuff.size() < static_cast<size_t>(reserved_offset + 7)) {
        throw std::runtime_error("Buffer size is smaller than expected reservedOffset + 7");
    }
    // Copy instanceId into blobBuff at reservedOffset + 4 to reservedOffset + 7
    std::copy(instanceId.begin(), instanceId.end(), blobBuff.begin() + reserved_offset + 4);
    // Copy extraNonce bytes into blobBuff at reservedOffset
    std::copy(extraBuff.begin(), extraBuff.end(), blobBuff.begin() + reserved_offset);

    // Convert blob using cnutil::ConvertBlob
    std::vector<uint8_t> convertedBlob = util::ConvertBlob(blobBuff);

    // Convert to hex string
    std::stringstream ss;
    for (const auto& byte : convertedBlob) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return ss.str();
}

} // namespace stratum
