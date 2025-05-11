#include "Utilities.h"

namespace Louron::Utils
{
	uint32_t fnv1a_hash(const std::string &str)
	{
		uint32_t hash = 2166136261u;
		for (char c : str)
			hash ^= static_cast<uint8_t>(c), hash *= 16777619u;
		return hash;
	}
}