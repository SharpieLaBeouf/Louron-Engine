#include "Skeleton.h"

namespace Louron
{
	size_t BoneLayout::size() const
	{
		size_t totalSize = 1;

		for (const auto& child : BoneChildren) 
		{
			totalSize += child.size();
		}

		return totalSize;
	}

	BoneLayout* BoneLayout::find(const std::string& bone_name)
	{
		if (BoneName == bone_name)
			return this;

		for (auto& child_bone : BoneChildren)
		{
			BoneLayout* bone_found = child_bone.find(bone_name);
			if (bone_found) return bone_found;
		}

		return nullptr;
	}
}