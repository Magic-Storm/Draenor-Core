#include "BattlenetAccountMgr.h"
#include "AccountMgr.h"
#include "DatabaseEnv.h"
#include "Field.h"
#include "Util.h"
#include "SHA1.h"
#include "SHA256.h"

static std::string CalculateGameShaPassHash(std::string const& name, std::string const& password)
{
    SHA1Hash sha;
    sha.Initialize();
    sha.UpdateData(name);
    sha.UpdateData(":");
    sha.UpdateData(password);
    sha.Finalize();
    return ByteArrayToHexStr(sha.GetDigest(), sha.GetLength());
}

AccountOpResult Battlenet::AccountMgr::CreateBattlenetAccount(std::string email, std::string password, bool withGameAccount)
{
    if (utf8length(email) > MAX_BNET_EMAIL_STR)
        return AOR_NAME_TOO_LONG;

    if (utf8length(password) > MAX_PASS_STR)
        return AOR_PASS_TOO_LONG;

    ::AccountMgr::normalizeString(email);
    ::AccountMgr::normalizeString(password);

    if (GetId(email))
        return AOR_NAME_ALREDY_EXIST;

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_BNET_ACCOUNT);
    stmt->setString(0, email);
    stmt->setString(1, CalculateShaPassHash(email, password));
    LoginDatabase.DirectExecute(stmt);

    uint32 newAccountId = GetId(email);
    if (!newAccountId)
        return AOR_DB_INTERNAL_ERROR;

    if (!withGameAccount)
        return AOR_OK;

    std::string gameUsername = std::to_string(newAccountId) + "#1";
    if (::AccountMgr::GetId(gameUsername))
        return AOR_NAME_ALREDY_EXIST;

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT);
    stmt->setString(0, gameUsername);
    stmt->setString(1, CalculateGameShaPassHash(gameUsername, password));
    stmt->setString(2, email);
    stmt->setUInt32(3, newAccountId);
    stmt->setUInt8(4, 1);
    LoginDatabase.DirectExecute(stmt);

    uint32 gameAccountId = ::AccountMgr::GetId(gameUsername);
    if (!gameAccountId)
        return AOR_DB_INTERNAL_ERROR;

    LoginDatabase.DirectPExecute(
        "INSERT INTO battlenet_account_gameaccounts (battlenetAccountId, gameAccountId) VALUES (%u, %u)",
        newAccountId, gameAccountId);

    stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS_INIT);
    LoginDatabase.Execute(stmt);

    return AOR_OK;
}

AccountOpResult Battlenet::AccountMgr::ChangePassword(uint32 accountId, std::string newPassword)
{
    std::string username;
    if (!GetName(accountId, username))
        return AOR_NAME_NOT_EXIST;

    ::AccountMgr::normalizeString(username);
    ::AccountMgr::normalizeString(newPassword);
    if (utf8length(newPassword) > MAX_PASS_STR)
        return AOR_PASS_TOO_LONG;

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BNET_PASSWORD);
    stmt->setString(0, CalculateShaPassHash(username, newPassword));
    stmt->setUInt32(1, accountId);
    LoginDatabase.Execute(stmt);

    if (QueryResult games = LoginDatabase.PQuery("SELECT id, username FROM account WHERE battlenet_account = %u", accountId))
    {
        do
        {
            Field* fields = games->Fetch();
            LoginDatabase.DirectPExecute("UPDATE account SET sha_pass_hash = '%s' WHERE id = %u",
                CalculateGameShaPassHash(fields[1].GetString(), newPassword).c_str(), fields[0].GetUInt32());
        } while (games->NextRow());
    }

    return AOR_OK;
}

bool Battlenet::AccountMgr::CheckPassword(uint32 accountId, std::string password)
{
    std::string username;
    if (!GetName(accountId, username))
        return false;

    ::AccountMgr::normalizeString(username);
    ::AccountMgr::normalizeString(password);

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_CHECK_PASSWORD);
    stmt->setUInt32(0, accountId);
    stmt->setString(1, CalculateShaPassHash(username, password));
    return LoginDatabase.Query(stmt) != nullptr;
}

AccountOpResult Battlenet::AccountMgr::LinkWithGameAccount(std::string const& email, std::string const& gameAccountName)
{
    uint32 bnetAccountId = GetId(email);
    if (!bnetAccountId)
        return AOR_NAME_NOT_EXIST;

    uint32 gameAccountId = ::AccountMgr::GetId(gameAccountName);
    if (!gameAccountId)
        return AOR_NAME_NOT_EXIST;

    if (GetIdByGameAccount(gameAccountId))
        return AOR_ACCOUNT_BAD_LINK;

    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_BNET_GAME_ACCOUNT_LINK);
    stmt->setUInt32(0, bnetAccountId);
    stmt->setUInt8(1, GetMaxIndex(bnetAccountId) + 1);
    stmt->setUInt32(2, gameAccountId);
    LoginDatabase.Execute(stmt);
    return AOR_OK;
}

AccountOpResult Battlenet::AccountMgr::UnlinkGameAccount(std::string const& gameAccountName)
{
    uint32 gameAccountId = ::AccountMgr::GetId(gameAccountName);
    if (!gameAccountId)
        return AOR_NAME_NOT_EXIST;

    if (!GetIdByGameAccount(gameAccountId))
        return AOR_ACCOUNT_BAD_LINK;

    LoginDatabase.DirectPExecute(
        "UPDATE account SET battlenet_account = NULL, battlenet_index = NULL WHERE id = %u",
        gameAccountId);
    return AOR_OK;
}

uint32 Battlenet::AccountMgr::GetId(std::string const& username)
{
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_ACCOUNT_ID_BY_EMAIL);
    stmt->setString(0, username);
    if (PreparedQueryResult result = LoginDatabase.Query(stmt))
        return (*result)[0].GetUInt32();
    return 0;
}

bool Battlenet::AccountMgr::GetName(uint32 accountId, std::string& name)
{
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_ACCOUNT_EMAIL_BY_ID);
    stmt->setUInt32(0, accountId);
    if (PreparedQueryResult result = LoginDatabase.Query(stmt))
    {
        name = (*result)[0].GetString();
        return true;
    }
    return false;
}

uint32 Battlenet::AccountMgr::GetIdByGameAccount(uint32 gameAccountId)
{
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_ACCOUNT_ID_BY_GAME_ACCOUNT);
    stmt->setUInt32(0, gameAccountId);
    if (PreparedQueryResult result = LoginDatabase.Query(stmt))
        return (*result)[0].GetUInt32();
    return 0;
}

uint8 Battlenet::AccountMgr::GetMaxIndex(uint32 accountId)
{
    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_BNET_MAX_ACCOUNT_INDEX);
    stmt->setUInt32(0, accountId);
    if (PreparedQueryResult result = LoginDatabase.Query(stmt))
        return (*result)[0].GetUInt8();
    return 0;
}

std::string Battlenet::AccountMgr::CalculateShaPassHash(std::string const& name, std::string const& password)
{
    return ::AccountMgr::CalculateShaPassHash(name, password);
}
