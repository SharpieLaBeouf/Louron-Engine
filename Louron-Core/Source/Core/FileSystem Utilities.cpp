#include "FileSystem Utilities.h"

namespace Louron::Utils
{
    std::filesystem::path NormalisePath(const std::filesystem::path &file_path)
    {
        std::string normalized = file_path.string();
        std::replace(normalized.begin(), normalized.end(), '\\', '/');
        return normalized;
    }
}
