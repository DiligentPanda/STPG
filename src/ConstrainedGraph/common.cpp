#include "ConstrainedGraph/common.h"

namespace ConstrainedGraph {

std::ostream& operator<<(std::ostream& os, const Path& path)
{
	for (const auto& state : path)
	{
		os << state.Loc.location << "\t"; // << "(" << state.is_single() << "),";
	}
	return os;
}


bool isSamePath(const Path& p1, const Path& p2)
{
	if (p1.size() != p2.size())
		return false;
	for (unsigned i = 0; i < p1.size(); i++)
	{
		if (p1[i].Loc.location != p2[i].Loc.location)
			return false;
	}
	return true;
}

};