// blocktemplate.h
#ifndef BLOCK_TEMPLATE_H
#define BLOCK_TEMPLATE_H

#include <array>
#include <string>
#include <vector>

namespace stratum
{
class BlockTemplate
{
public:
    std::vector<uint8_t> blob_buffer;
    std::string seed_hash;
    std::string next_seed_hash;
    std::string blocktemplate_blob;
    uint64_t difficulty;
    uint64_t height;
    std::string prev_hash;
    uint64_t reserved_offset;

    std::string nextBlob(uint32_t extraNonce, const std::array<uint8_t, 3>& instanceId);
};
} // namespace stratum

#endif // BLOCK_TEMPLATE_H
