#pragma once

#include <filesystem>
#include <algorithm>

namespace Louron
{
    namespace Utils
    {
        std::filesystem::path NormalisePath(const std::filesystem::path& file_path);
    }
}