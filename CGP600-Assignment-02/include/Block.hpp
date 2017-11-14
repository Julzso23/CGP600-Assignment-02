#pragma once

#include <cstdint>
#include "BlockDetails.hpp"

struct Block
{
    std::shared_ptr<BlockDetails> details;
};
