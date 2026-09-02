#ifndef CollectionMgr_h__
#define CollectionMgr_h__

#include "Define.h"
#include <unordered_map>
#include <map>

struct HeirloomData
{
    uint32 flags = 0;
    uint32 bonusId = 0;
};

using HeirloomContainer = std::unordered_map<uint32, HeirloomData>;
using ToyBoxContainer = std::map<uint32, bool>;

class CollectionMgr
{
};

#endif
