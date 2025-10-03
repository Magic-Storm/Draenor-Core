////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _GROUPMGR_H
#define _GROUPMGR_H

#include "Group.h"

class GroupMgr
{
private:
    GroupMgr();
    ~GroupMgr();

public:
    static GroupMgr* instance()
    {
        static GroupMgr* instance = new GroupMgr();
        return instance;
    }
    typedef std::map<uint32, Group*> GroupContainer;
    typedef std::vector<Group*>      GroupDbContainer;
    typedef std::multimap<uint64, Group*> PlayerGroups;

    Group* GetGroupByGUID(uint32 guid) const;

    uint32 GenerateNewGroupDbStoreId();
    void   RegisterGroupDbStoreId(uint32 storageId, Group* group);
    void   FreeGroupDbStoreId(Group* group);
    void   SetNextGroupDbStoreId(uint32 storageId) { NextGroupDbStoreId = storageId; };
    Group* GetGroupByDbStoreId(uint32 storageId) const;
    void   SetGroupDbStoreSize(uint32 newSize) { GroupDbStore.resize(newSize); }

    void BindGroupToPlayer(uint64 playerGuid, Group* group);
    void UnbindGroupFromPlayer(uint64 playerGuid, Group* group);

#ifndef CROSS
    void   LoadGroups();
#endif /* not CROSS */
    uint32 GenerateGroupId();
    void   AddGroup(Group* group);
    void   RemoveGroup(Group* group);


protected:
    uint32           NextGroupId;
    uint32           NextGroupDbStoreId;
    GroupContainer   GroupStore;
    GroupDbContainer GroupDbStore;
    PlayerGroups     GroupByPlayerStore;
};

#define sGroupMgr GroupMgr::instance()

#endif
