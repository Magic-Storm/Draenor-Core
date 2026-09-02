#ifndef BattlenetAccountMgr_h__
#define BattlenetAccountMgr_h__

#include "DatabaseEnv.h"

namespace Battlenet
{
    namespace AccountMgr
    {
        inline uint32 GetIdByGameAccount(uint32 gameAccountId)
        {
            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_ACCOUNT_ID_BY_GAME_ACCOUNT);
            stmt->setUInt32(0, gameAccountId);
            if (PreparedQueryResult result = LoginDatabase.Query(stmt))
                return (*result)[0].GetUInt32();
            return 0;
        }
    }
}

#endif
