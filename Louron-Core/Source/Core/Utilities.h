#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <string>
#include <cstdint>

// External Vendor Library Headers

namespace Louron::Utils
{
    uint32_t fnv1a_hash(const std::string& str);
}