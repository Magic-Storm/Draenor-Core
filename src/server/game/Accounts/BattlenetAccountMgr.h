#ifndef BattlenetAccountMgr_h__
#define BattlenetAccountMgr_h__

#include "AccountMgr.h"
#include "Define.h"
#include <string>

#define MAX_BNET_EMAIL_STR 320

namespace Battlenet
{
    namespace AccountMgr
    {
        AccountOpResult CreateBattlenetAccount(std::string email, std::string password, bool withGameAccount = true);
        AccountOpResult ChangePassword(uint32 accountId, std::string newPassword);
        bool CheckPassword(uint32 accountId, std::string password);
        AccountOpResult LinkWithGameAccount(std::string const& email, std::string const& gameAccountName);
        AccountOpResult UnlinkGameAccount(std::string const& gameAccountName);

        uint32 GetId(std::string const& username);
        bool GetName(uint32 accountId, std::string& name);
        uint32 GetIdByGameAccount(uint32 gameAccountId);
        uint8 GetMaxIndex(uint32 accountId);

        std::string CalculateShaPassHash(std::string const& name, std::string const& password);
    }
}

#endif
