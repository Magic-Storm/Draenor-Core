////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

/** \file
    \ingroup world
*/

#include <atomic>

// Ugly hack allowing to use a version of libcurl built by VS2013
#if defined(_MSC_VER) && _MSC_VER >= 1900
#pragma comment(lib, "legacy_stdio_definitions.lib")
#endif

#ifndef CROSS
# include "AuctionHouseMgr.h"
# include "GuildFinderMgr.h"
# include "TicketMgr.h"
# include "CharacterDatabaseCleaner.h"
# include "CalendarMgr.h"
# include "BlackMarketMgr.h"
# include "PlayerDump.h"
# include "BattlepayMgr.h"
# include "GarrisonShipmentManager.hpp"
# include "GarrisonMgr.hpp"
#else
# include "InterRealmMgr.h"
#endif

#include "AnticheatMgr.h"
#include "Common.h"
#include "Memory.h"
#include "DatabaseEnv.h"
#include "Config.h"
#include "GitRevision.h"
#include "Log.h"
#include "Opcodes.h"
#include "WorldSession.h"
#include "WorldPacket.h"
#include "Player.h"
#include "Vehicle.h"
#include "SkillExtraItems.h"
#include "SkillDiscovery.h"
#include "World.h"
#include "AccountMgr.h"
#include "AchievementMgr.h"
#include "ObjectMgr.h"
#include "GuildMgr.h"
#include "SpellMgr.h"
#include "GroupMgr.h"
#include "Chat.h"
#include "DBCStores.h"
#include "DB2Stores.h"
#include "LootMgr.h"
#include "ItemEnchantmentMgr.h"
#include "MapManager.h"
#include "CreatureAIRegistry.h"
#include "BattlegroundMgr.hpp"
#include "OutdoorPvPMgr.h"
#include "TemporarySummon.h"
#include "WaypointMovementGenerator.h"
#include "VMapFactory.h"
#include "MMapFactory.h"
#include "GameEventMgr.h"
#include "PoolMgr.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "InstanceSaveMgr.h"
#include "Util.h"
#include "Language.h"
#include "CreatureGroups.h"
#include "Transport.h"
#include "ScriptMgr.h"
#include "AddonMgr.h"
#include "LFGMgr.h"
#include "ConditionMgr.h"
#include "DisableMgr.h"
#include "ScriptMgr.h"
#include "WeatherMgr.h"
#include "CreatureTextMgr.h"
#include "SmartAI.h"
#include "Channel.h"
#include "WardenCheckMgr.h"
#include "Warden.h"
#include "BattlefieldMgr.h"
#include "CinematicPathMgr.h"
#include "WildBattlePet.h"
#include "TransportMgr.h"
#include "InterRealmOpcodes.h"
#include "MMapFactory.h"
#include "TaxiPathGraph.h"
#include "ChatLexicsCutter.h"
#include <ctime>
#include "../scripts/Custom/SpellRegulator.h"

uint32 gOnlineGameMaster = 0;


std::atomic<bool> World::m_stopEvent(false);
uint8 World::m_ExitCode = SHUTDOWN_EXIT_CODE;
std::atomic_uint32_t World::m_worldLoopCounter = 0;

float World::m_MaxVisibleDistanceOnContinents = DEFAULT_VISIBILITY_DISTANCE;
float World::m_MaxVisibleDistanceInInstances  = DEFAULT_VISIBILITY_INSTANCE;
float World::m_MaxVisibleDistanceInBG         = DEFAULT_VISIBILITY_BGARENAS;
float World::m_MaxVisibleDistanceInArenas     = DEFAULT_VISIBILITY_BGARENAS;

int32 World::m_visibility_notify_periodOnContinents = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
int32 World::m_visibility_notify_periodInInstances  = DEFAULT_VISIBILITY_NOTIFY_PERIOD;
int32 World::m_visibility_notify_periodInBGArenas   = DEFAULT_VISIBILITY_NOTIFY_PERIOD;

float World::Visibility_RelocationLowerLimit = 20.0f;
uint32 World::Visibility_AINotifyDelay = 1000;

/// World constructor
World::World()
{
#ifndef CROSS
    m_NextCurrencyReset = 0;
    m_NextDailyLootReset = 0;
    m_NextGuildChallengesReset = 0;
    m_NextBossLootedReset = 0;
    m_InterRealmSession = nullptr;

#else /* CROSS */
    m_NextRandomBGReset = 0;
    m_update_online_timer = 60000;
#endif

    m_playerLimit = 0;
    m_allowedSecurityLevel = SEC_PLAYER;
    m_allowMovement = true;
    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_gameTime = time(NULL);
    m_startTime = m_gameTime;
    m_maxActiveSessionCount = 0;
    m_maxQueuedSessionCount = 0;
    m_PlayerCount = 0;
    m_MaxPlayerCount = 0;

    m_NextDailyQuestReset = 0;
    m_NextWeeklyQuestReset = 0;


    m_defaultDbcLocale = LOCALE_enUS;
    m_availableDbcLocaleMask = 0;

    m_updateTimeSum = 0;
    m_updateTimeCount = 0;

    m_serverDelaySum = 0;
    m_serverDelayTimer = 0;
    m_serverUpdateCount = 0;

    m_isClosed = false;

    m_CleaningFlags = 0;

    for (uint8 i = 0; i < RECORD_DIFF_MAX; i++)
        m_recordDiff[i] = 0;

    m_lexicsCutter = nullptr;

    m_LastAccountLogId = 0;

    m_QueryHolderCallbacks             = std::unique_ptr<QueryHolderCallbacks>(new QueryHolderCallbacks());
    m_QueryHolderCallbacksBuffer       = std::unique_ptr<QueryHolderCallbacks>(new QueryHolderCallbacks());
    m_TransactionCallbacks             = std::unique_ptr<TransactionCallbacks>(new TransactionCallbacks());
    m_TransactionCallbacksBuffer       = std::unique_ptr<TransactionCallbacks>(new TransactionCallbacks());
    m_PreparedStatementCallbacks       = std::unique_ptr<PreparedStatementCallbacks>(new PreparedStatementCallbacks());
    m_PreparedStatementCallbacksBuffer = std::unique_ptr<PreparedStatementCallbacks>(new PreparedStatementCallbacks());
}

/// World destructor
World::~World()
{
#ifndef CROSS
    ///- Empty the kicked session set
    while (!m_sessions.empty())
    {
        // not remove from queue, prevent loading new sessions
        delete m_sessions.begin()->second;
        m_sessions.erase(m_sessions.begin());
    }
#endif

    CliCommandHolder* command = NULL;
    while (cliCmdQueue.next(command))
        delete command;

    VMAP::VMapFactory::clear();
    MMAP::MMapFactory::clear();

    //TODO free addSessQueue
}

/// Find a player in a specified zone
Player* World::FindPlayerInZone(uint32 zone)
{
    ///- circle through active sessions and return the first player found in the zone
#ifndef CROSS
    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
#else /* CROSS */
    PlayerMap::const_iterator itr;
    for (itr = m_players.begin(); itr != m_players.end(); ++itr)
#endif /* CROSS */
    {
        if (!itr->second)
            continue;

#ifndef CROSS
        Player* player = itr->second->GetPlayer();
#else /* CROSS */
        Player* player = itr->second;
#endif /* CROSS */
        if (!player)
            continue;

        if (player->IsInWorld() && player->GetZoneId() == zone)
        {
            // Used by the weather system. We return the player to broadcast the change weather message to him and all players in the zone.
            return player;
        }
    }
    return NULL;
}

bool World::IsClosed() const
{
    return m_isClosed;
}

void World::SetClosed(bool val)
{
    m_isClosed = val;

    // Invert the value, for simplicity for scripters.
    sScriptMgr->OnOpenStateChange(!val);
}

MotdText const& World::GetMotd() const
{
    return m_Motd;
}

#ifndef CROSS
/// Find a session by its id
WorldSession* World::FindSession(uint32 id) const
{
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end())
        return itr->second;                                 // also can return NULL for kicked session
    else
        return NULL;
}

/// Remove a given session
bool World::RemoveSession(uint32 id)
{
    ///- Find the session, kick the user, but we can't delete session at this moment to prevent iterator invalidation
    SessionMap::const_iterator itr = m_sessions.find(id);

    if (itr != m_sessions.end() && itr->second)
    {
        if (itr->second->PlayerLoading())
            return false;

        itr->second->KickPlayer();
    }

    return true;
}

void World::AddSession(WorldSession* s)
{
    addSessQueue.add(s);
}

void World::AddSession_(WorldSession* s)
{
    ASSERT (s);

    //NOTE - Still there is race condition in WorldSession* being used in the Sockets

    ///- kick already loaded player with same account (if any) and remove session
    ///- if player is in loading and want to load again, return
    if (!RemoveSession (s->GetAccountId()))
    {
        s->KickPlayer();
        delete s;                                           // session not added yet in session list, so not listed in queue
        return;
    }

    // decrease session counts only at not reconnection case
    bool decrease_session = true;

    // if session already exist, prepare to it deleting at next world update
    // NOTE - KickPlayer() should be called on "old" in RemoveSession()
    {
        SessionMap::const_iterator old = m_sessions.find(s->GetAccountId());

        if (old != m_sessions.end())
        {
            // prevent decrease sessions count if session queued
            if (RemoveQueuedPlayer(old->second))
                decrease_session = false;
            // not remove replaced session form queue if listed
            delete old->second;
        }
    }

    m_sessions[s->GetAccountId()] = s;

    uint32 Sessions = GetActiveAndQueuedSessionCount();
    uint32 pLimit = GetPlayerAmountLimit();
    uint32 QueueSize = GetQueuedSessionCount();             //number of players in the queue

    //so we don't count the user trying to
    //login as a session and queue the socket that we are using
    if (decrease_session)
        --Sessions;

    if (pLimit > 0 && Sessions >= pLimit && AccountMgr::IsPlayerAccount(s->GetSecurity()) && !HasRecentlyDisconnected(s) && !s->IsPremium())
    {
        AddQueuedPlayer (s);
        UpdateMaxSessionCounters();
        TC_LOG_INFO("misc", "PlayerQueue: Account id %u is in Queue Position (%u).", s->GetAccountId(), ++QueueSize);
        return;
    }

    s->SendAuthResponse(AUTH_OK, false);
    s->SendTimeZoneInformations();
    s->SendAddonsInfo();
    s->SendFeatureSystemStatus();
    s->SendClientCacheVersion(sWorld->getIntConfig(CONFIG_CLIENTCACHE_VERSION));
    s->SendTutorialsData();

    UpdateMaxSessionCounters();

    // Updates the population
    if (pLimit > 0)
    {
        float popu = (float)GetActiveSessionCount();              // updated number of users on the server
        popu /= pLimit;
        popu *= 2;
        TC_LOG_INFO("misc", "Server Population (%f).", popu);
    }
}

bool World::HasRecentlyDisconnected(WorldSession* session)
{
    if (!session)
        return false;

    if (uint32 tolerance = getIntConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
    {
        for (DisconnectMap::iterator i = m_disconnects.begin(); i != m_disconnects.end();)
        {
            if (difftime(i->second, time(NULL)) < tolerance)
            {
                if (i->first == session->GetAccountId())
                    return true;
                ++i;
            }
            else
                m_disconnects.erase(i++);
        }
    }
    return false;
 }

int32 World::GetQueuePos(WorldSession* sess)
{
    uint32 position = 1;

    for (Queue::const_iterator iter = m_QueuedPlayer.begin(); iter != m_QueuedPlayer.end(); ++iter, ++position)
        if ((*iter) == sess)
            return position;

    return 0;
}

void World::AddQueuedPlayer(WorldSession* sess)
{
    sess->SetInQueue(true);
    m_QueuedPlayer.push_back(sess);

    // The 1st SMSG_AUTH_RESPONSE needs to contain other info too.
    sess->SendAuthResponse(AUTH_OK, true, GetQueuePos(sess));
}

bool World::RemoveQueuedPlayer(WorldSession* sess)
{
    // sessions count including queued to remove (if removed_session set)
    uint32 sessions = GetActiveSessionCount();

    uint32 position = 1;
    Queue::iterator iter = m_QueuedPlayer.begin();

    // search to remove and count skipped positions
    bool found = false;

    for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
    {
        if (*iter == sess)
        {
            sess->SetInQueue(false);
            sess->ResetTimeOutTime();
            iter = m_QueuedPlayer.erase(iter);
            found = true;                                   // removing queued session
            break;
        }
    }

    // iter point to next socked after removed or end()
    // position store position of removed socket and then new position next socket after removed

    // if session not queued then we need decrease sessions count
    if (!found && sessions)
        --sessions;

    // accept first in queue
    if ((!m_playerLimit || sessions < m_playerLimit) && !m_QueuedPlayer.empty())
    {
        WorldSession* pop_sess = m_QueuedPlayer.front();
        pop_sess->SetInQueue(false);
        pop_sess->ResetTimeOutTime();
        pop_sess->SendAuthWaitQue(0);
        pop_sess->SendTimeZoneInformations();
        pop_sess->SendAddonsInfo();
        pop_sess->SendFeatureSystemStatus();

        pop_sess->SendClientCacheVersion(sWorld->getIntConfig(CONFIG_CLIENTCACHE_VERSION));
        pop_sess->SendAccountDataTimes(GLOBAL_CACHE_MASK);
        pop_sess->SendTutorialsData();

        m_QueuedPlayer.pop_front();

        // update iter to point first queued socket or end() if queue is empty now
        iter = m_QueuedPlayer.begin();
        position = 1;
    }

    // update position from iter to end()
    // iter point to first not updated socket, position store new position
    for (; iter != m_QueuedPlayer.end(); ++iter, ++position)
        (*iter)->SendAuthWaitQue(position);

    return found;
}

#endif

/// Initialize config values
void World::LoadConfigSettings(bool reload)
{
    if (reload)
    {
        if (!sConfigMgr->Reload())
        {
            TC_LOG_ERROR("server.worldserver", "World settings reload fail: can't read settings from %s.", sConfigMgr->GetFilename().c_str());
            return;
        }
        sLog->LoadFromConfig();
    }

    ///- Read the player limit and the Message of the day from the config file
    SetPlayerAmountLimit(sConfigMgr->GetIntDefault("PlayerLimit", 100));

    ///- Read ticket system setting from the config file
    m_bool_configs[CONFIG_ALLOW_TICKETS] = sConfigMgr->GetBoolDefault("AllowTickets", true);

    ///- Get string for new logins (newly created characters)
    SetNewCharString(sConfigMgr->GetStringDefault("PlayerStart.String", ""));

    ///- Send server info on login?
    m_int_configs[CONFIG_ENABLE_SINFO_LOGIN] = sConfigMgr->GetIntDefault("Server.LoginInfo", 0);

    ///- Read all rates from the config file
    rate_values[RATE_HEALTH] = sConfigMgr->GetFloatDefault("Rate.Health", 1);
    if (rate_values[RATE_HEALTH] < 0)
    {
        TC_LOG_ERROR("server.loading", "Rate.Health (%f) must be > 0. Using 1 instead.", rate_values[RATE_HEALTH]);
        rate_values[RATE_HEALTH] = 1;
    }
    rate_values[RATE_POWER_MANA] = sConfigMgr->GetFloatDefault("Rate.Mana", 1);
    if (rate_values[RATE_POWER_MANA] < 0)
    {
        TC_LOG_ERROR("server.loading", "Rate.Mana (%f) must be > 0. Using 1 instead.", rate_values[RATE_POWER_MANA]);
        rate_values[RATE_POWER_MANA] = 1;
    }
    rate_values[RATE_POWER_RAGE_INCOME] = sConfigMgr->GetFloatDefault("Rate.Rage.Income", 1);
    rate_values[RATE_POWER_RAGE_LOSS] = sConfigMgr->GetFloatDefault("Rate.Rage.Loss", 1);
    if (rate_values[RATE_POWER_RAGE_LOSS] < 0)
    {
        TC_LOG_ERROR("server.loading", "Rate.Rage.Loss (%f) must be > 0. Using 1 instead.", rate_values[RATE_POWER_RAGE_LOSS]);
        rate_values[RATE_POWER_RAGE_LOSS] = 1;
    }
    rate_values[RATE_POWER_RUNICPOWER_INCOME] = sConfigMgr->GetFloatDefault("Rate.RunicPower.Income", 1);
    rate_values[RATE_POWER_RUNICPOWER_LOSS] = sConfigMgr->GetFloatDefault("Rate.RunicPower.Loss", 1);
    if (rate_values[RATE_POWER_RUNICPOWER_LOSS] < 0)
    {
        TC_LOG_ERROR("server.loading", "Rate.RunicPower.Loss (%f) must be > 0. Using 1 instead.", rate_values[RATE_POWER_RUNICPOWER_LOSS]);
        rate_values[RATE_POWER_RUNICPOWER_LOSS] = 1;
    }
    rate_values[RATE_POWER_FOCUS] = sConfigMgr->GetFloatDefault("Rate.Focus", 1.0f);
    rate_values[RATE_POWER_ENERGY] = sConfigMgr->GetFloatDefault("Rate.Energy", 1.0f);

    rate_values[RATE_SKILL_DISCOVERY] = sConfigMgr->GetFloatDefault("Rate.Skill.Discovery", 1.0f);

    rate_values[RATE_DROP_ITEM_POOR] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Poor", 1.0f);
    rate_values[RATE_DROP_ITEM_NORMAL] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Normal", 1.0f);
    rate_values[RATE_DROP_ITEM_UNCOMMON] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Uncommon", 1.0f);
    rate_values[RATE_DROP_ITEM_RARE] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Rare", 1.0f);
    rate_values[RATE_DROP_ITEM_EPIC] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Epic", 1.0f);
    rate_values[RATE_DROP_ITEM_LEGENDARY] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Legendary", 1.0f);
    rate_values[RATE_DROP_ITEM_ARTIFACT] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Artifact", 1.0f);
    rate_values[RATE_DROP_ITEM_REFERENCED] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.Referenced", 1.0f);
    rate_values[RATE_DROP_ITEM_REFERENCED_AMOUNT] = sConfigMgr->GetFloatDefault("Rate.Drop.Item.ReferencedAmount", 1.0f);
    rate_values[RATE_DROP_MONEY] = sConfigMgr->GetFloatDefault("Rate.Drop.Money", 1.0f);
    rate_values[RATE_XP_KILL] = sConfigMgr->GetFloatDefault("Rate.XP.Kill", 1.0f);
    rate_values[RATE_XP_QUEST] = sConfigMgr->GetFloatDefault("Rate.XP.Quest", 1.0f);
    rate_values[RATE_XP_EXPLORE] = sConfigMgr->GetFloatDefault("Rate.XP.Explore", 1.0f);
    rate_values[RATE_XP_GATHERING] = sConfigMgr->GetFloatDefault("Rate.XP.Gathering", 1.0f);
    rate_values[RATE_REPAIRCOST] = sConfigMgr->GetFloatDefault("Rate.RepairCost", 1.0f);
    if (rate_values[RATE_REPAIRCOST] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "Rate.RepairCost (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_REPAIRCOST]);
        rate_values[RATE_REPAIRCOST] = 0.0f;
    }

    rate_values[RATE_XP_KILL_PREMIUM] = sConfigMgr->GetFloatDefault("Rate.XP.Kill.Premium", 1.0f);
    rate_values[RATE_XP_QUEST_PREMIUM] = sConfigMgr->GetFloatDefault("Rate.XP.Quest.Premium", 1.0f);
    rate_values[RATE_XP_EXPLORE_PREMIUM] = sConfigMgr->GetFloatDefault("Rate.XP.Explore.Premium", 1.0f);
    rate_values[RATE_REPUTATION_GAIN_PREMIUM] = sConfigMgr->GetFloatDefault("Rate.Reputation.Gain.Premium", 1.0f);
    rate_values[RATE_HONOR_PREMIUM] = sConfigMgr->GetFloatDefault("Rate.Honor.Premium", 1.0f);

    rate_values[RATE_REPUTATION_GAIN] = sConfigMgr->GetFloatDefault("Rate.Reputation.Gain", 1.0f);
    rate_values[RATE_REPUTATION_LOWLEVEL_KILL] = sConfigMgr->GetFloatDefault("Rate.Reputation.LowLevel.Kill", 1.0f);
    rate_values[RATE_REPUTATION_LOWLEVEL_QUEST] = sConfigMgr->GetFloatDefault("Rate.Reputation.LowLevel.Quest", 1.0f);
    rate_values[RATE_REPUTATION_RECRUIT_A_FRIEND_BONUS] = sConfigMgr->GetFloatDefault("Rate.Reputation.RecruitAFriendBonus", 0.1f);
    rate_values[RATE_CREATURE_NORMAL_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Normal.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_ELITE_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.Elite.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RAREELITE.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.Damage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RARE_DAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RARE.Damage", 1.0f);
    rate_values[RATE_CREATURE_NORMAL_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Normal.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_ELITE_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.Elite.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RAREELITE.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.HP", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RARE_HP] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RARE.HP", 1.0f);
    rate_values[RATE_CREATURE_NORMAL_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Normal.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_ELITE_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.Elite.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RAREELITE_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RAREELITE.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_WORLDBOSS_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.WORLDBOSS.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_ELITE_RARE_SPELLDAMAGE] = sConfigMgr->GetFloatDefault("Rate.Creature.Elite.RARE.SpellDamage", 1.0f);
    rate_values[RATE_CREATURE_AGGRO] = sConfigMgr->GetFloatDefault("Rate.Creature.Aggro", 1.0f);
    rate_values[RATE_REST_INGAME] = sConfigMgr->GetFloatDefault("Rate.Rest.InGame", 1.0f);
    rate_values[RATE_REST_OFFLINE_IN_TAVERN_OR_CITY] = sConfigMgr->GetFloatDefault("Rate.Rest.Offline.InTavernOrCity", 1.0f);
    rate_values[RATE_REST_OFFLINE_IN_WILDERNESS] = sConfigMgr->GetFloatDefault("Rate.Rest.Offline.InWilderness", 1.0f);
    rate_values[RATE_DAMAGE_FALL] = sConfigMgr->GetFloatDefault("Rate.Damage.Fall", 1.0f);
    rate_values[RATE_AUCTION_TIME] = sConfigMgr->GetFloatDefault("Rate.Auction.Time", 1.0f);
    rate_values[RATE_AUCTION_DEPOSIT] = sConfigMgr->GetFloatDefault("Rate.Auction.Deposit", 1.0f);
    rate_values[RATE_AUCTION_CUT] = sConfigMgr->GetFloatDefault("Rate.Auction.Cut", 1.0f);
    rate_values[RATE_HONOR] = sConfigMgr->GetFloatDefault("Rate.Honor", 1.0f);
    rate_values[RATE_MINING_AMOUNT] = sConfigMgr->GetFloatDefault("Rate.Mining.Amount", 1.0f);
    rate_values[RATE_MINING_NEXT] = sConfigMgr->GetFloatDefault("Rate.Mining.Next", 1.0f);
    rate_values[RATE_INSTANCE_RESET_TIME] = sConfigMgr->GetFloatDefault("Rate.InstanceResetTime", 1.0f);
    rate_values[RATE_TALENT] = sConfigMgr->GetFloatDefault("Rate.Talent", 1.0f);
    if (rate_values[RATE_TALENT] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "Rate.Talent (%f) must be > 0. Using 1 instead.", rate_values[RATE_TALENT]);
        rate_values[RATE_TALENT] = 1.0f;
    }
    rate_values[RATE_MOVESPEED] = sConfigMgr->GetFloatDefault("Rate.MoveSpeed", 1.0f);
    rate_values[RATE_ONLINE] = sConfigMgr->GetFloatDefault("Rate.Online", 1.0f);
    if (rate_values[RATE_MOVESPEED] < 0)
    {
        TC_LOG_ERROR("server.loading", "Rate.MoveSpeed (%f) must be > 0. Using 1 instead.", rate_values[RATE_MOVESPEED]);
        rate_values[RATE_MOVESPEED] = 1.0f;
    }
    for (uint8 i = 0; i < MAX_MOVE_TYPE; ++i) playerBaseMoveSpeed[i] = baseMoveSpeed[i] * rate_values[RATE_MOVESPEED];
    rate_values[RATE_CORPSE_DECAY_LOOTED] = sConfigMgr->GetFloatDefault("Rate.Corpse.Decay.Looted", 0.5f);

    rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = sConfigMgr->GetFloatDefault("TargetPosRecalculateRange", 1.5f);
    if (rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] < CONTACT_DISTANCE)
    {
        TC_LOG_ERROR("server.loading", "TargetPosRecalculateRange (%f) must be >= %f. Using %f instead.", rate_values[RATE_TARGET_POS_RECALCULATION_RANGE], CONTACT_DISTANCE, CONTACT_DISTANCE);
        rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = CONTACT_DISTANCE;
    }
    else if (rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] > 1.5f)
    {
        TC_LOG_ERROR("server.loading", "TargetPosRecalculateRange (%f) must be <= %f. Using %f instead.",
            rate_values[RATE_TARGET_POS_RECALCULATION_RANGE], 1.5f, 1.5f);
        rate_values[RATE_TARGET_POS_RECALCULATION_RANGE] = 1.5f;
    }

    rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = sConfigMgr->GetFloatDefault("DurabilityLoss.OnDeath", 10.0f);
    if (rate_values[RATE_DURABILITY_LOSS_ON_DEATH] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "DurabilityLoss.OnDeath (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_ON_DEATH]);
        rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = 0.0f;
    }
    if (rate_values[RATE_DURABILITY_LOSS_ON_DEATH] > 100.0f)
    {
        TC_LOG_ERROR("server.loading", "DurabilityLoss.OnDeath (%f) must be <= 100. Using 100.0 instead.", rate_values[RATE_DURABILITY_LOSS_ON_DEATH]);
        rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_ON_DEATH] = rate_values[RATE_DURABILITY_LOSS_ON_DEATH] / 100.0f;

    rate_values[RATE_DURABILITY_LOSS_DAMAGE] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Damage", 0.5f);
    if (rate_values[RATE_DURABILITY_LOSS_DAMAGE] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "DurabilityLossChance.Damage (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_DAMAGE]);
        rate_values[RATE_DURABILITY_LOSS_DAMAGE] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_ABSORB] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Absorb", 0.5f);
    if (rate_values[RATE_DURABILITY_LOSS_ABSORB] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "DurabilityLossChance.Absorb (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_ABSORB]);
        rate_values[RATE_DURABILITY_LOSS_ABSORB] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_PARRY] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Parry", 0.05f);
    if (rate_values[RATE_DURABILITY_LOSS_PARRY] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "DurabilityLossChance.Parry (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_PARRY]);
        rate_values[RATE_DURABILITY_LOSS_PARRY] = 0.0f;
    }
    rate_values[RATE_DURABILITY_LOSS_BLOCK] = sConfigMgr->GetFloatDefault("DurabilityLossChance.Block", 0.05f);
    if (rate_values[RATE_DURABILITY_LOSS_BLOCK] < 0.0f)
    {
        TC_LOG_ERROR("server.loading", "DurabilityLossChance.Block (%f) must be >=0. Using 0.0 instead.", rate_values[RATE_DURABILITY_LOSS_BLOCK]);
        rate_values[RATE_DURABILITY_LOSS_BLOCK] = 0.0f;
    }
    ///- Read other configuration items from the config file

    m_bool_configs[CONFIG_DURABILITY_LOSS_IN_PVP] = sConfigMgr->GetBoolDefault("DurabilityLoss.InPvP", false);

    m_int_configs[CONFIG_COMPRESSION] = sConfigMgr->GetIntDefault("Compression", 1);
    if (m_int_configs[CONFIG_COMPRESSION] < 1 || m_int_configs[CONFIG_COMPRESSION] > 9)
    {
        TC_LOG_ERROR("server.loading", "Compression level (%i) must be in range 1..9. Using default compression level (1).", m_int_configs[CONFIG_COMPRESSION]);
        m_int_configs[CONFIG_COMPRESSION] = 1;
    }
    m_bool_configs[CONFIG_ADDON_CHANNEL] = sConfigMgr->GetBoolDefault("AddonChannel", true);
    m_bool_configs[CONFIG_CLEAN_CHARACTER_DB] = sConfigMgr->GetBoolDefault("CleanCharacterDB", false);
    m_int_configs[CONFIG_PERSISTENT_CHARACTER_CLEAN_FLAGS] = sConfigMgr->GetIntDefault("PersistentCharacterCleanFlags", 0);
    m_int_configs[CONFIG_CHAT_CHANNEL_LEVEL_REQ] = sConfigMgr->GetIntDefault("ChatLevelReq.Channel", 1);
    m_int_configs[CONFIG_CHAT_WHISPER_LEVEL_REQ] = sConfigMgr->GetIntDefault("ChatLevelReq.Whisper", 1);
    m_int_configs[CONFIG_CHAT_SAY_LEVEL_REQ] = sConfigMgr->GetIntDefault("ChatLevelReq.Say", 1);
    m_int_configs[CONFIG_TRADE_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Trade", 1);
    m_int_configs[CONFIG_TICKET_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Ticket", 1);
    m_int_configs[CONFIG_AUCTION_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Auction", 1);
    m_int_configs[CONFIG_MAIL_LEVEL_REQ] = sConfigMgr->GetIntDefault("LevelReq.Mail", 1);
    m_bool_configs[CONFIG_ALLOW_PLAYER_COMMANDS] = sConfigMgr->GetBoolDefault("AllowPlayerCommands", 1);
    m_bool_configs[CONFIG_PRESERVE_CUSTOM_CHANNELS] = sConfigMgr->GetBoolDefault("PreserveCustomChannels", false);
    m_int_configs[CONFIG_PRESERVE_CUSTOM_CHANNEL_DURATION] = sConfigMgr->GetIntDefault("PreserveCustomChannelDuration", 14);
    m_bool_configs[CONFIG_GRID_UNLOAD] = sConfigMgr->GetBoolDefault("GridUnload", true);
    m_int_configs[CONFIG_INTERVAL_SAVE] = sConfigMgr->GetIntDefault("PlayerSaveInterval", 15 * MINUTE * IN_MILLISECONDS);
    m_int_configs[CONFIG_INTERVAL_DISCONNECT_TOLERANCE] = sConfigMgr->GetIntDefault("DisconnectToleranceInterval", 0);
    m_bool_configs[CONFIG_STATS_SAVE_ONLY_ON_LOGOUT] = sConfigMgr->GetBoolDefault("PlayerSave.Stats.SaveOnlyOnLogout", true);

    m_int_configs[CONFIG_MIN_LEVEL_STAT_SAVE] = sConfigMgr->GetIntDefault("PlayerSave.Stats.MinLevel", 0);
    if (m_int_configs[CONFIG_MIN_LEVEL_STAT_SAVE] > MAX_LEVEL)
    {
        TC_LOG_ERROR("server.loading", "PlayerSave.Stats.MinLevel (%i) must be in range 0..80. Using default, do not save character stats (0).", m_int_configs[CONFIG_MIN_LEVEL_STAT_SAVE]);
        m_int_configs[CONFIG_MIN_LEVEL_STAT_SAVE] = 0;
    }

    m_int_configs[CONFIG_INTERVAL_GRIDCLEAN] = sConfigMgr->GetIntDefault("GridCleanUpDelay", 5 * MINUTE * IN_MILLISECONDS);
    if (m_int_configs[CONFIG_INTERVAL_GRIDCLEAN] < MIN_GRID_DELAY)
    {
        TC_LOG_ERROR("server.loading", "GridCleanUpDelay (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_GRIDCLEAN], MIN_GRID_DELAY);
        m_int_configs[CONFIG_INTERVAL_GRIDCLEAN] = MIN_GRID_DELAY;
    }
    if (reload)
        sMapMgr->SetGridCleanUpDelay(m_int_configs[CONFIG_INTERVAL_GRIDCLEAN]);

    m_int_configs[CONFIG_INTERVAL_MAPUPDATE] = sConfigMgr->GetIntDefault("MapUpdateInterval", 100);
    if (m_int_configs[CONFIG_INTERVAL_MAPUPDATE] < MIN_MAP_UPDATE_DELAY)
    {
        TC_LOG_ERROR("server.loading", "MapUpdateInterval (%i) must be greater %u. Use this minimal value.", m_int_configs[CONFIG_INTERVAL_MAPUPDATE], MIN_MAP_UPDATE_DELAY);
        m_int_configs[CONFIG_INTERVAL_MAPUPDATE] = MIN_MAP_UPDATE_DELAY;
    }
    if (reload)
        sMapMgr->SetMapUpdateInterval(m_int_configs[CONFIG_INTERVAL_MAPUPDATE]);

    m_int_configs[CONFIG_INTERVAL_CHANGEWEATHER] = sConfigMgr->GetIntDefault("ChangeWeatherInterval", 10 * MINUTE * IN_MILLISECONDS);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("WorldServerPort", 8085);
        if (val != m_int_configs[CONFIG_PORT_WORLD])
            TC_LOG_ERROR("server.loading", "WorldServerPort option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_PORT_WORLD]);
    }
    else
        m_int_configs[CONFIG_PORT_WORLD] = sConfigMgr->GetIntDefault("WorldServerPort", 8085);

    m_int_configs[CONFIG_SOCKET_TIMEOUTTIME] = sConfigMgr->GetIntDefault("SocketTimeOutTime", 900000);
    m_int_configs[CONFIG_SESSION_ADD_DELAY] = sConfigMgr->GetIntDefault("SessionAddDelay", 10000);

    m_float_configs[CONFIG_GROUP_XP_DISTANCE] = sConfigMgr->GetFloatDefault("MaxGroupXPDistance", 74.0f);
    m_float_configs[CONFIG_INSTANCE_GROUP_XP_DISTANCE] = sConfigMgr->GetFloatDefault("MaxInstanceGroupXPDistance", 150.0f);
    m_float_configs[CONFIG_MAX_RECRUIT_A_FRIEND_DISTANCE] = sConfigMgr->GetFloatDefault("MaxRecruitAFriendBonusDistance", 100.0f);

    /// \todo Add MonsterSight and GuarderSight (with meaning) in worldserver.conf or put them as define
    m_float_configs[CONFIG_SIGHT_MONSTER] = sConfigMgr->GetFloatDefault("MonsterSight", 50);
    m_float_configs[CONFIG_SIGHT_GUARDER] = sConfigMgr->GetFloatDefault("GuarderSight", 50);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("GameType", 0);
        if (val != m_int_configs[CONFIG_GAME_TYPE])
            TC_LOG_ERROR("server.loading", "GameType option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_GAME_TYPE]);
    }
    else
        m_int_configs[CONFIG_GAME_TYPE] = sConfigMgr->GetIntDefault("GameType", 0);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);
        if (val != m_int_configs[CONFIG_REALM_ZONE])
            TC_LOG_ERROR("server.loading", "RealmZone option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_REALM_ZONE]);
    }
    else
        m_int_configs[CONFIG_REALM_ZONE] = sConfigMgr->GetIntDefault("RealmZone", REALM_ZONE_DEVELOPMENT);

    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_ACCOUNTS]                      = sConfigMgr->GetBoolDefault("AllowTwoSide.Accounts", true);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CALENDAR]          = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Calendar", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHAT]              = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Chat", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_LFG]               = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.DungeonFinder", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_CHANNEL]           = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Channel", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_GROUP]             = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Group", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_GUILD]             = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Guild", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_AUCTION]           = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Auction", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_MAIL]              = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Mail", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_MOUNT]             = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.Mount", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_INTERACTION_MOUNT_CAPITALS]    = sConfigMgr->GetBoolDefault("AllowTwoSide.Interaction.MountInCapitals", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_WHO_LIST]                      = sConfigMgr->GetBoolDefault("AllowTwoSide.WhoList", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_ADD_FRIEND]                    = sConfigMgr->GetBoolDefault("AllowTwoSide.AddFriend", false);
    m_bool_configs[CONFIG_ALLOW_TWO_SIDE_TRADE]                         = sConfigMgr->GetBoolDefault("AllowTwoSide.trade", false);
    m_int_configs[CONFIG_STRICT_PLAYER_NAMES]                           = sConfigMgr->GetIntDefault("StrictPlayerNames", 0);
    m_int_configs[CONFIG_STRICT_CHARTER_NAMES]                          = sConfigMgr->GetIntDefault("StrictCharterNames", 0);
    m_int_configs[CONFIG_STRICT_PET_NAMES]                              = sConfigMgr->GetIntDefault("StrictPetNames", 0);

    m_int_configs[CONFIG_MIN_PLAYER_NAME] = sConfigMgr->GetIntDefault("MinPlayerName", 2);
    if (m_int_configs[CONFIG_MIN_PLAYER_NAME] < 1 || m_int_configs[CONFIG_MIN_PLAYER_NAME] > MAX_PLAYER_NAME)
    {
        TC_LOG_ERROR("server.loading", "MinPlayerName (%i) must be in range 1..%u. Set to 2.", m_int_configs[CONFIG_MIN_PLAYER_NAME], MAX_PLAYER_NAME);
        m_int_configs[CONFIG_MIN_PLAYER_NAME] = 2;
    }

    m_int_configs[CONFIG_MIN_CHARTER_NAME] = sConfigMgr->GetIntDefault("MinCharterName", 2);
    if (m_int_configs[CONFIG_MIN_CHARTER_NAME] < 1 || m_int_configs[CONFIG_MIN_CHARTER_NAME] > MAX_CHARTER_NAME)
    {
        TC_LOG_ERROR("server.loading", "MinCharterName (%i) must be in range 1..%u. Set to 2.", m_int_configs[CONFIG_MIN_CHARTER_NAME], MAX_CHARTER_NAME);
        m_int_configs[CONFIG_MIN_CHARTER_NAME] = 2;
    }

    m_int_configs[CONFIG_MIN_PET_NAME] = sConfigMgr->GetIntDefault("MinPetName", 2);
    if (m_int_configs[CONFIG_MIN_PET_NAME] < 1 || m_int_configs[CONFIG_MIN_PET_NAME] > MAX_PET_NAME)
    {
        TC_LOG_ERROR("server.loading", "MinPetName (%i) must be in range 1..%u. Set to 2.", m_int_configs[CONFIG_MIN_PET_NAME], MAX_PET_NAME);
        m_int_configs[CONFIG_MIN_PET_NAME] = 2;
    }

    m_int_configs[CONFIG_CHARACTER_CREATING_DISABLED] = sConfigMgr->GetIntDefault("CharacterCreating.Disabled", 0);
    m_int_configs[CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK] = sConfigMgr->GetIntDefault("CharacterCreating.Disabled.RaceMask", 0);
    m_int_configs[CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK] = sConfigMgr->GetIntDefault("CharacterCreating.Disabled.ClassMask", 0);

    m_int_configs[CONFIG_CHARACTERS_PER_REALM] = sConfigMgr->GetIntDefault("CharactersPerRealm", 11);
    if (m_int_configs[CONFIG_CHARACTERS_PER_REALM] < 1 || m_int_configs[CONFIG_CHARACTERS_PER_REALM] > 11)
    {
        TC_LOG_ERROR("server.loading", "CharactersPerRealm (%i) must be in range 1..11. Set to 11.", m_int_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_int_configs[CONFIG_CHARACTERS_PER_REALM] = 11;
    }

    // must be after CONFIG_CHARACTERS_PER_REALM
    m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = sConfigMgr->GetIntDefault("CharactersPerAccount", 50);
    if (m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT] < m_int_configs[CONFIG_CHARACTERS_PER_REALM])
    {
        TC_LOG_ERROR("server.loading", "CharactersPerAccount (%i) can't be less than CharactersPerRealm (%i).", m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT], m_int_configs[CONFIG_CHARACTERS_PER_REALM]);
        m_int_configs[CONFIG_CHARACTERS_PER_ACCOUNT] = m_int_configs[CONFIG_CHARACTERS_PER_REALM];
    }

    m_int_configs[CONFIG_CHARACTER_CREATING_MIN_LEVEL_FOR_HEROIC_CHARACTER] = sConfigMgr->GetIntDefault("CharacterCreating.MinLevelForHeroicCharacter", 55);

    m_int_configs[CONFIG_SKIP_CINEMATICS] = sConfigMgr->GetIntDefault("SkipCinematics", 0);
    if (int32(m_int_configs[CONFIG_SKIP_CINEMATICS]) < 0 || m_int_configs[CONFIG_SKIP_CINEMATICS] > 2)
    {
        TC_LOG_ERROR("server.loading", "SkipCinematics (%i) must be in range 0..2. Set to 0.", m_int_configs[CONFIG_SKIP_CINEMATICS]);
        m_int_configs[CONFIG_SKIP_CINEMATICS] = 0;
    }

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("MaxPlayerLevel", DEFAULT_MAX_LEVEL);
        if (val != m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
            TC_LOG_ERROR("server.loading", "MaxPlayerLevel option can't be changed at config reload, using current value (%u).", m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
    }
    else
        m_int_configs[CONFIG_MAX_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("MaxPlayerLevel", DEFAULT_MAX_LEVEL);

    if (m_int_configs[CONFIG_MAX_PLAYER_LEVEL] > MAX_LEVEL)
    {
        TC_LOG_ERROR("server.loading", "MaxPlayerLevel (%i) must be in range 1..%u. Set to %u.", m_int_configs[CONFIG_MAX_PLAYER_LEVEL], MAX_LEVEL, MAX_LEVEL);
        m_int_configs[CONFIG_MAX_PLAYER_LEVEL] = MAX_LEVEL;
    }

    m_int_configs[CONFIG_MIN_DUALSPEC_LEVEL] = sConfigMgr->GetIntDefault("MinDualSpecLevel", 40);

    m_int_configs[CONFIG_START_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("StartPlayerLevel", 1);
    if (m_int_configs[CONFIG_START_PLAYER_LEVEL] < 1)
    {
        TC_LOG_ERROR("server.loading", "StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to 1.", m_int_configs[CONFIG_START_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_PLAYER_LEVEL] = 1;
    }
    else if (m_int_configs[CONFIG_START_PLAYER_LEVEL] > m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        TC_LOG_ERROR("server.loading", "StartPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to %u.", m_int_configs[CONFIG_START_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_PLAYER_LEVEL] = m_int_configs[CONFIG_MAX_PLAYER_LEVEL];
    }

    m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("StartHeroicPlayerLevel", 55);
    if (m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL] < 1)
    {
        TC_LOG_ERROR("server.loading", "StartHeroicPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to 55.",
            m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL] = 55;
    }
    else if (m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL] > m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        TC_LOG_ERROR("server.loading", "StartHeroicPlayerLevel (%i) must be in range 1..MaxPlayerLevel(%u). Set to %u.",
            m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_HEROIC_PLAYER_LEVEL] = m_int_configs[CONFIG_MAX_PLAYER_LEVEL];
    }

    m_int_configs[CONFIG_START_PLAYER_MONEY] = sConfigMgr->GetIntDefault("StartPlayerMoney", 0);
    if (int32(m_int_configs[CONFIG_START_PLAYER_MONEY]) < 0)
    {
        TC_LOG_ERROR("server.loading", "StartPlayerMoney (%i) must be in range 0.." UI64FMTD ". Set to %u.", m_int_configs[CONFIG_START_PLAYER_MONEY], MAX_MONEY_AMOUNT, 0);
        m_int_configs[CONFIG_START_PLAYER_MONEY] = 0;
    }
    else if (m_int_configs[CONFIG_START_PLAYER_MONEY] > 0x7FFFFFFF - 1) // TODO: (See MAX_MONEY_AMOUNT)
    {
        TC_LOG_ERROR("server.loading", "StartPlayerMoney (%i) must be in range 0..%u. Set to %u.",
            m_int_configs[CONFIG_START_PLAYER_MONEY], 0x7FFFFFFF - 1, 0x7FFFFFFF - 1);
        m_int_configs[CONFIG_START_PLAYER_MONEY] = 0x7FFFFFFF - 1;
    }

    m_int_configs[CONFIG_CURRENCY_RESET_HOUR] = sConfigMgr->GetIntDefault("Currency.ResetHour", 3);
    if (m_int_configs[CONFIG_CURRENCY_RESET_HOUR] > 23)
    {
        TC_LOG_ERROR("server.loading", "Currency.ResetHour (%i) can't be load. Set to 6.", m_int_configs[CONFIG_CURRENCY_RESET_HOUR]);
        m_int_configs[CONFIG_CURRENCY_RESET_HOUR] = 3;
    }
    m_int_configs[CONFIG_CURRENCY_RESET_DAY] = sConfigMgr->GetIntDefault("Currency.ResetDay", 3);
    if (m_int_configs[CONFIG_CURRENCY_RESET_DAY] > 6)
    {
        TC_LOG_ERROR("server.loading", "Currency.ResetDay (%i) can't be load. Set to 3.", m_int_configs[CONFIG_CURRENCY_RESET_DAY]);
        m_int_configs[CONFIG_CURRENCY_RESET_DAY] = 3;
    }
    m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL] = sConfigMgr->GetIntDefault("Currency.ResetInterval", 7);
    if (int32(m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.ResetInterval (%i) must be > 0, set to default 7.", m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL]);
        m_int_configs[CONFIG_CURRENCY_RESET_INTERVAL] = 7;
    }

    m_int_configs[CONFIG_CURRENCY_START_HONOR_POINTS] = sConfigMgr->GetIntDefault("Currency.StartHonorPoints", 0);
    if (int32(m_int_configs[CONFIG_CURRENCY_START_HONOR_POINTS]) < 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.StartHonorPoints (%i) must be >= 0, set to default 0.", m_int_configs[CONFIG_CURRENCY_START_HONOR_POINTS]);
        m_int_configs[CONFIG_CURRENCY_START_HONOR_POINTS] = 0;
    }
    m_int_configs[CONFIG_CURRENCY_MAX_HONOR_POINTS] = sConfigMgr->GetIntDefault("Currency.MaxHonorPoints", 4000);
    if (int32(m_int_configs[CONFIG_CURRENCY_MAX_HONOR_POINTS]) < 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.MaxHonorPoints (%i) can't be negative. Set to default 4000.", m_int_configs[CONFIG_CURRENCY_MAX_HONOR_POINTS]);
        m_int_configs[CONFIG_CURRENCY_MAX_HONOR_POINTS] = 4000;
    }
    m_int_configs[CONFIG_CURRENCY_MAX_HONOR_POINTS] *= CURRENCY_PRECISION;     //precision mod

    m_int_configs[CONFIG_CURRENCY_START_JUSTICE_POINTS] = sConfigMgr->GetIntDefault("Currency.StartJusticePoints", 0);
    if (int32(m_int_configs[CONFIG_CURRENCY_START_JUSTICE_POINTS]) < 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.StartJusticePoints (%i) must be >= 0, set to default 0.", m_int_configs[CONFIG_CURRENCY_START_JUSTICE_POINTS]);
        m_int_configs[CONFIG_CURRENCY_START_JUSTICE_POINTS] = 0;
    }
    m_int_configs[CONFIG_CURRENCY_MAX_JUSTICE_POINTS] = sConfigMgr->GetIntDefault("Currency.MaxJusticePoints", 4000);
    if (int32(m_int_configs[CONFIG_CURRENCY_MAX_JUSTICE_POINTS]) < 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.MaxJusticePoints (%i) can't be negative. Set to default 4000.", m_int_configs[CONFIG_CURRENCY_MAX_JUSTICE_POINTS]);
        m_int_configs[CONFIG_CURRENCY_MAX_JUSTICE_POINTS] = 4000;
    }
    m_int_configs[CONFIG_CURRENCY_MAX_JUSTICE_POINTS] *= CURRENCY_PRECISION;     //precision mod

    m_int_configs[CONFIG_CURRENCY_START_CONQUEST_POINTS] = sConfigMgr->GetIntDefault("Currency.StartConquestPoints", 0);
    if (int32(m_int_configs[CONFIG_CURRENCY_START_CONQUEST_POINTS]) < 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.StartConquestPoints (%i) must be >= 0, set to default 0.", m_int_configs[CONFIG_CURRENCY_START_CONQUEST_POINTS]);
        m_int_configs[CONFIG_CURRENCY_START_CONQUEST_POINTS] = 0;
    }
    m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_WEEK_CAP] = sConfigMgr->GetIntDefault("Currency.ConquestPointsWeekCap", 1500);
    if (int32(m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_WEEK_CAP]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.ConquestPointsWeekCap (%i) must be > 0, set to default 1500.", m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_WEEK_CAP]);
        m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_WEEK_CAP] = 1500;
    }
    m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_WEEK_CAP] *= CURRENCY_PRECISION;     //precision mod

    m_int_configs[CONFIG_CURRENCY_VIP_CONQUEST_POINTS_WEEK_CAP] = sConfigMgr->GetIntDefault("Currency.ConquestVipPointsWeekCap", 3000) * CURRENCY_PRECISION;

    m_int_configs[CONFIG_CURRENCY_ASHRAN_CONQUEST_POINTS_WEEK_CAP] = sConfigMgr->GetIntDefault("Currency.ConquestPoints.Ashran.WeekCap", 200);
    if (int32(m_int_configs[CONFIG_CURRENCY_ASHRAN_CONQUEST_POINTS_WEEK_CAP]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.ConquestPointsWeekCap (%i) must be > 0, set to default 200.", m_int_configs[CONFIG_CURRENCY_ASHRAN_CONQUEST_POINTS_WEEK_CAP]);
        m_int_configs[CONFIG_CURRENCY_ASHRAN_CONQUEST_POINTS_WEEK_CAP] = 200;
    }
    m_int_configs[CONFIG_CURRENCY_ASHRAN_CONQUEST_POINTS_WEEK_CAP] *= CURRENCY_PRECISION;     //precision mod

    m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_ARENA_REWARD] = sConfigMgr->GetIntDefault("Currency.ConquestPointsArenaReward", 180);
    if (int32(m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_ARENA_REWARD]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.ConquestPointsArenaReward (%i) must be > 0, set to default 180.", m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_ARENA_REWARD]);
        m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_ARENA_REWARD] = 180;
    }
    m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_ARENA_REWARD] *= CURRENCY_PRECISION;     //precision mod

    m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_RATED_BG_REWARD] = sConfigMgr->GetIntDefault("Currency.ConquestPointsRatedBGReward", 400);
    if (int32(m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_RATED_BG_REWARD]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "Currency.ConquestPointsRatedBGReward (%i) must be > 0, set to default 400.", m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_RATED_BG_REWARD]);
        m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_RATED_BG_REWARD] = 400;
    }
    m_int_configs[CONFIG_CURRENCY_CONQUEST_POINTS_RATED_BG_REWARD] *= CURRENCY_PRECISION;     //precision mod

    m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL] = sConfigMgr->GetIntDefault("RecruitAFriend.MaxLevel", 60);
    if (m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL] > m_int_configs[CONFIG_MAX_PLAYER_LEVEL])
    {
        TC_LOG_ERROR("server.loading", "RecruitAFriend.MaxLevel (%i) must be in the range 0..MaxLevel(%u). Set to %u.",
            m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL], m_int_configs[CONFIG_MAX_PLAYER_LEVEL], 60);
        m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL] = 60;
    }

    m_int_configs[CONFIG_MAX_RECRUIT_A_FRIEND_BONUS_PLAYER_LEVEL_DIFFERENCE] = sConfigMgr->GetIntDefault("RecruitAFriend.MaxDifference", 4);
    m_bool_configs[CONFIG_ALL_TAXI_PATHS] = sConfigMgr->GetBoolDefault("AllFlightPaths", false);
	m_int_configs[CONFIG_INSTANT_TAXI] = sConfigMgr->GetIntDefault("InstantFlightPaths", 0); //Instant flight mod 瞬飛系統

    m_bool_configs[CONFIG_INSTANCE_IGNORE_LEVEL] = sConfigMgr->GetBoolDefault("Instance.IgnoreLevel", false);
    m_bool_configs[CONFIG_INSTANCE_IGNORE_RAID] = sConfigMgr->GetBoolDefault("Instance.IgnoreRaid", false);

    m_bool_configs[CONFIG_CAST_UNSTUCK] = sConfigMgr->GetBoolDefault("CastUnstuck", true);
    m_int_configs[CONFIG_INSTANCE_RESET_TIME_HOUR]  = sConfigMgr->GetIntDefault("Instance.ResetTimeHour", 4);
    m_int_configs[CONFIG_INSTANCE_UNLOAD_DELAY] = sConfigMgr->GetIntDefault("Instance.UnloadDelay", 30 * MINUTE * IN_MILLISECONDS);

    m_int_configs[CONFIG_MAX_PRIMARY_TRADE_SKILL] = sConfigMgr->GetIntDefault("MaxPrimaryTradeSkill", 2);
    m_int_configs[CONFIG_MIN_PETITION_SIGNS] = sConfigMgr->GetIntDefault("MinPetitionSigns", 9);
    if (m_int_configs[CONFIG_MIN_PETITION_SIGNS] > 9)
    {
        TC_LOG_ERROR("server.loading", "MinPetitionSigns (%i) must be in range 0..9. Set to 9.", m_int_configs[CONFIG_MIN_PETITION_SIGNS]);
        m_int_configs[CONFIG_MIN_PETITION_SIGNS] = 9;
    }

    m_int_configs[CONFIG_GM_LOGIN_STATE]        = sConfigMgr->GetIntDefault("GM.LoginState", 2);
    m_int_configs[CONFIG_GM_VISIBLE_STATE]      = sConfigMgr->GetIntDefault("GM.Visible", 2);
    m_int_configs[CONFIG_GM_CHAT]               = sConfigMgr->GetIntDefault("GM.Chat", 2);
    m_int_configs[CONFIG_GM_WHISPERING_TO]      = sConfigMgr->GetIntDefault("GM.WhisperingTo", 2);

    m_int_configs[CONFIG_GM_LEVEL_IN_GM_LIST]   = sConfigMgr->GetIntDefault("GM.InGMList.Level", SEC_ADMINISTRATOR);
    m_int_configs[CONFIG_GM_LEVEL_IN_WHO_LIST]  = sConfigMgr->GetIntDefault("GM.InWhoList.Level", SEC_ADMINISTRATOR);
    m_bool_configs[CONFIG_GM_LOG_TRADE]         = sConfigMgr->GetBoolDefault("GM.LogTrade", false);
    m_int_configs[CONFIG_START_GM_LEVEL]        = sConfigMgr->GetIntDefault("GM.StartLevel", 1);
    if (m_int_configs[CONFIG_START_GM_LEVEL] < m_int_configs[CONFIG_START_PLAYER_LEVEL])
    {
        TC_LOG_ERROR("server.loading", "GM.StartLevel (%i) must be in range StartPlayerLevel(%u)..%u. Set to %u.",
            m_int_configs[CONFIG_START_GM_LEVEL], m_int_configs[CONFIG_START_PLAYER_LEVEL], MAX_LEVEL, m_int_configs[CONFIG_START_PLAYER_LEVEL]);
        m_int_configs[CONFIG_START_GM_LEVEL] = m_int_configs[CONFIG_START_PLAYER_LEVEL];
    }
    else if (m_int_configs[CONFIG_START_GM_LEVEL] > MAX_LEVEL)
    {
        TC_LOG_ERROR("server.loading", "GM.StartLevel (%i) must be in range 1..%u. Set to %u.", m_int_configs[CONFIG_START_GM_LEVEL], MAX_LEVEL, MAX_LEVEL);
        m_int_configs[CONFIG_START_GM_LEVEL] = MAX_LEVEL;
    }
    m_bool_configs[CONFIG_ALLOW_GM_GROUP]       = sConfigMgr->GetBoolDefault("GM.AllowInvite", false);
    m_bool_configs[CONFIG_ALLOW_GM_FRIEND]      = sConfigMgr->GetBoolDefault("GM.AllowFriend", false);
    m_bool_configs[CONFIG_GM_LOWER_SECURITY] = sConfigMgr->GetBoolDefault("GM.LowerSecurity", false);
    m_float_configs[CONFIG_CHANCE_OF_GM_SURVEY] = sConfigMgr->GetFloatDefault("GM.TicketSystem.ChanceOfGMSurvey", 50.0f);

    m_int_configs[CONFIG_GROUP_VISIBILITY] = sConfigMgr->GetIntDefault("Visibility.GroupMode", 1);

    m_int_configs[CONFIG_MAIL_DELIVERY_DELAY] = sConfigMgr->GetIntDefault("MailDeliveryDelay", HOUR);

    m_int_configs[CONFIG_UPTIME_UPDATE] = sConfigMgr->GetIntDefault("UpdateUptimeInterval", 10);
    if (int32(m_int_configs[CONFIG_UPTIME_UPDATE]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "UpdateUptimeInterval (%i) must be > 0, set to default 10.", m_int_configs[CONFIG_UPTIME_UPDATE]);
        m_int_configs[CONFIG_UPTIME_UPDATE] = 10;
    }
    if (reload)
    {
        m_timers[WUPDATE_UPTIME].SetInterval(m_int_configs[CONFIG_UPTIME_UPDATE]*MINUTE*IN_MILLISECONDS);
        m_timers[WUPDATE_UPTIME].Reset();
    }

    // log db cleanup interval
    m_int_configs[CONFIG_LOGDB_CLEARINTERVAL] = sConfigMgr->GetIntDefault("LogDB.Opt.ClearInterval", 10);
    if (int32(m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]) <= 0)
    {
        TC_LOG_ERROR("server.loading", "LogDB.Opt.ClearInterval (%i) must be > 0, set to default 10.", m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]);
        m_int_configs[CONFIG_LOGDB_CLEARINTERVAL] = 10;
    }
    if (reload)
    {
        m_timers[WUPDATE_CLEANDB].SetInterval(m_int_configs[CONFIG_LOGDB_CLEARINTERVAL] * MINUTE * IN_MILLISECONDS);
        m_timers[WUPDATE_CLEANDB].Reset();
    }
    m_int_configs[CONFIG_LOGDB_CLEARTIME] = sConfigMgr->GetIntDefault("LogDB.Opt.ClearTime", 1209600); // 14 days default
    TC_LOG_INFO("server.loading", "Will clear `logs` table of entries older than %i seconds every %u minutes.",
        m_int_configs[CONFIG_LOGDB_CLEARTIME], m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]);

    m_int_configs[CONFIG_SKILL_CHANCE_ORANGE] = sConfigMgr->GetIntDefault("SkillChance.Orange", 100);
    m_int_configs[CONFIG_SKILL_CHANCE_YELLOW] = sConfigMgr->GetIntDefault("SkillChance.Yellow", 75);
    m_int_configs[CONFIG_SKILL_CHANCE_GREEN]  = sConfigMgr->GetIntDefault("SkillChance.Green", 25);
    m_int_configs[CONFIG_SKILL_CHANCE_GREY]   = sConfigMgr->GetIntDefault("SkillChance.Grey", 0);

    m_int_configs[CONFIG_SKILL_CHANCE_MINING_STEPS]  = sConfigMgr->GetIntDefault("SkillChance.MiningSteps", 75);
    m_int_configs[CONFIG_SKILL_CHANCE_SKINNING_STEPS]   = sConfigMgr->GetIntDefault("SkillChance.SkinningSteps", 75);

    m_bool_configs[CONFIG_SKILL_PROSPECTING] = sConfigMgr->GetBoolDefault("SkillChance.Prospecting", false);
    m_bool_configs[CONFIG_SKILL_MILLING] = sConfigMgr->GetBoolDefault("SkillChance.Milling", false);

    m_int_configs[CONFIG_SKILL_GAIN_CRAFTING]  = sConfigMgr->GetIntDefault("SkillGain.Crafting", 1);

    m_int_configs[CONFIG_SKILL_GAIN_GATHERING]  = sConfigMgr->GetIntDefault("SkillGain.Gathering", 1);

    m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] = sConfigMgr->GetIntDefault("MaxOverspeedPings", 2);
    if (m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] != 0 && m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] < 2)
    {
        TC_LOG_ERROR("server.loading", "MaxOverspeedPings (%i) must be in range 2..infinity (or 0 to disable check). Set to 2.", m_int_configs[CONFIG_MAX_OVERSPEED_PINGS]);
        m_int_configs[CONFIG_MAX_OVERSPEED_PINGS] = 2;
    }

    m_bool_configs[CONFIG_SAVE_RESPAWN_TIME_IMMEDIATELY] = sConfigMgr->GetBoolDefault("SaveRespawnTimeImmediately", true);
    m_bool_configs[CONFIG_WEATHER] = sConfigMgr->GetBoolDefault("ActivateWeather", true);

    m_int_configs[CONFIG_DISABLE_BREATHING] = sConfigMgr->GetIntDefault("DisableWaterBreath", SEC_CONSOLE);

    if (reload)
    {
        uint32 val = sConfigMgr->GetIntDefault("Expansion", 1);
        if (val != m_int_configs[CONFIG_EXPANSION])
            TC_LOG_ERROR("server.loading", "Expansion option can't be changed at worldserver.conf reload, using current value (%u).", m_int_configs[CONFIG_EXPANSION]);
    }
    else
        m_int_configs[CONFIG_EXPANSION] = sConfigMgr->GetIntDefault("Expansion", 1);

    m_int_configs[CONFIG_CHATFLOOD_MESSAGE_COUNT] = sConfigMgr->GetIntDefault("ChatFlood.MessageCount", 10);
    m_int_configs[CONFIG_CHATFLOOD_MESSAGE_DELAY] = sConfigMgr->GetIntDefault("ChatFlood.MessageDelay", 1);
    m_int_configs[CONFIG_CHATFLOOD_MUTE_TIME]     = sConfigMgr->GetIntDefault("ChatFlood.MuteTime", 10);

    m_int_configs[CONFIG_CHATFLOOD_PRIVATE_MESSAGE_COUNT] = sConfigMgr->GetIntDefault("ChatFlood.PrivateMessageCount", 10);
    m_int_configs[CONFIG_CHATFLOOD_PRIVATE_MESSAGE_DELAY] = sConfigMgr->GetIntDefault("ChatFlood.PrivateMessageMessageDelay", 1);

    m_int_configs[CONFIG_EVENT_ANNOUNCE] = sConfigMgr->GetIntDefault("Event.Announce", 0);

    m_float_configs[CONFIG_CREATURE_FAMILY_FLEE_ASSISTANCE_RADIUS] = sConfigMgr->GetFloatDefault("CreatureFamilyFleeAssistanceRadius", 30.0f);
    m_float_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_RADIUS] = sConfigMgr->GetFloatDefault("CreatureFamilyAssistanceRadius", 10.0f);
    m_int_configs[CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY]  = sConfigMgr->GetIntDefault("CreatureFamilyAssistanceDelay", 1500);
    m_int_configs[CONFIG_CREATURE_FAMILY_FLEE_DELAY]        = sConfigMgr->GetIntDefault("CreatureFamilyFleeDelay", 7000);

    m_int_configs[CONFIG_WORLD_BOSS_LEVEL_DIFF] = sConfigMgr->GetIntDefault("WorldBossLevelDiff", 3);

    // note: disable value (-1) will assigned as 0xFFFFFFF, to prevent overflow at calculations limit it to max possible player level MAX_LEVEL(100)
    m_int_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = sConfigMgr->GetIntDefault("Quests.LowLevelHideDiff", 4);
    if (m_int_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_int_configs[CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF] = MAX_LEVEL;
    m_int_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = sConfigMgr->GetIntDefault("Quests.HighLevelHideDiff", 7);
    if (m_int_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] > MAX_LEVEL)
        m_int_configs[CONFIG_QUEST_HIGH_LEVEL_HIDE_DIFF] = MAX_LEVEL;
    m_bool_configs[CONFIG_QUEST_IGNORE_RAID] = sConfigMgr->GetBoolDefault("Quests.IgnoreRaid", false);
    m_bool_configs[CONFIG_QUEST_IGNORE_AUTO_ACCEPT] = sConfigMgr->GetBoolDefault("Quests.IgnoreAutoAccept", false);
    m_bool_configs[CONFIG_QUEST_IGNORE_AUTO_COMPLETE] = sConfigMgr->GetBoolDefault("Quests.IgnoreAutoComplete", false);

    m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR] = sConfigMgr->GetIntDefault("Battleground.Random.ResetHour", 6);
    if (m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR] > 23)
    {
        TC_LOG_ERROR("server.loading", "Battleground.Random.ResetHour (%i) can't be load. Set to 6.", m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR]);
        m_int_configs[CONFIG_RANDOM_BG_RESET_HOUR] = 6;
    }

    m_bool_configs[CONFIG_DETECT_POS_COLLISION] = sConfigMgr->GetBoolDefault("DetectPosCollision", true);

    m_bool_configs[CONFIG_RESTRICTED_LFG_CHANNEL]      = sConfigMgr->GetBoolDefault("Channel.RestrictedLfg", true);
    m_bool_configs[CONFIG_SILENTLY_GM_JOIN_TO_CHANNEL] = sConfigMgr->GetBoolDefault("Channel.SilentlyGMJoin", false);

    m_bool_configs[CONFIG_TALENTS_INSPECTING]           = sConfigMgr->GetBoolDefault("TalentsInspecting", true);
    m_bool_configs[CONFIG_CHAT_FAKE_MESSAGE_PREVENTING] = sConfigMgr->GetBoolDefault("ChatFakeMessagePreventing", false);
    m_int_configs[CONFIG_CHAT_STRICT_LINK_CHECKING_SEVERITY] = sConfigMgr->GetIntDefault("ChatStrictLinkChecking.Severity", 0);
    m_int_configs[CONFIG_CHAT_STRICT_LINK_CHECKING_KICK] = sConfigMgr->GetIntDefault("ChatStrictLinkChecking.Kick", 0);

    m_int_configs[CONFIG_CORPSE_DECAY_NORMAL]    = sConfigMgr->GetIntDefault("Corpse.Decay.NORMAL", 60);
    m_int_configs[CONFIG_CORPSE_DECAY_RARE]      = sConfigMgr->GetIntDefault("Corpse.Decay.RARE", 300);
    m_int_configs[CONFIG_CORPSE_DECAY_ELITE]     = sConfigMgr->GetIntDefault("Corpse.Decay.ELITE", 300);
    m_int_configs[CONFIG_CORPSE_DECAY_RAREELITE] = sConfigMgr->GetIntDefault("Corpse.Decay.RAREELITE", 300);
    m_int_configs[CONFIG_CORPSE_DECAY_WORLDBOSS] = sConfigMgr->GetIntDefault("Corpse.Decay.WORLDBOSS", 3600);

    m_int_configs[CONFIG_DEATH_SICKNESS_LEVEL]           = sConfigMgr->GetIntDefault ("Death.SicknessLevel", 11);
    m_bool_configs[CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVP] = sConfigMgr->GetBoolDefault("Death.CorpseReclaimDelay.PvP", true);
    m_bool_configs[CONFIG_DEATH_CORPSE_RECLAIM_DELAY_PVE] = sConfigMgr->GetBoolDefault("Death.CorpseReclaimDelay.PvE", true);
    m_bool_configs[CONFIG_DEATH_BONES_WORLD]              = sConfigMgr->GetBoolDefault("Death.Bones.World", true);
    m_bool_configs[CONFIG_DEATH_BONES_BG_OR_ARENA]        = sConfigMgr->GetBoolDefault("Death.Bones.BattlegroundOrArena", true);

    m_bool_configs[CONFIG_DIE_COMMAND_MODE] = sConfigMgr->GetBoolDefault("Die.Command.Mode", true);

    m_float_configs[CONFIG_THREAT_RADIUS] = sConfigMgr->GetFloatDefault("ThreatRadius", 60.0f);

    // always use declined names in the russian client
    m_bool_configs[CONFIG_DECLINED_NAMES_USED] = sConfigMgr->GetBoolDefault("DeclinedNames", false);

    m_float_configs[CONFIG_LISTEN_RANGE_SAY]       = sConfigMgr->GetFloatDefault("ListenRange.Say", 25.0f);
    m_float_configs[CONFIG_LISTEN_RANGE_TEXTEMOTE] = sConfigMgr->GetFloatDefault("ListenRange.TextEmote", 25.0f);
    m_float_configs[CONFIG_LISTEN_RANGE_YELL]      = sConfigMgr->GetFloatDefault("ListenRange.Yell", 300.0f);

    m_bool_configs[CONFIG_BATTLEGROUND_CAST_DESERTER]                = sConfigMgr->GetBoolDefault("Battleground.CastDeserter", true);
    m_bool_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_ENABLE]       = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.Enable", false);
    m_bool_configs[CONFIG_BATTLEGROUND_QUEUE_ANNOUNCER_PLAYERONLY]   = sConfigMgr->GetBoolDefault("Battleground.QueueAnnouncer.PlayerOnly", false);
    m_int_configs[CONFIG_BATTLEGROUND_INVITATION_TYPE]               = sConfigMgr->GetIntDefault ("Battleground.InvitationType", 0);
    m_int_configs[CONFIG_BATTLEGROUND_PREMATURE_FINISH_TIMER]        = sConfigMgr->GetIntDefault ("Battleground.PrematureFinishTimer", 5 * MINUTE * IN_MILLISECONDS);
    m_int_configs[CONFIG_BATTLEGROUND_PREMADE_GROUP_WAIT_FOR_MATCH]  = sConfigMgr->GetIntDefault ("Battleground.PremadeGroupWaitForMatch", 5 * MINUTE * IN_MILLISECONDS);
    m_bool_configs[CONFIG_BG_XP_FOR_KILL]                            = sConfigMgr->GetBoolDefault("Battleground.GiveXPForKills", false);
    m_int_configs[CONFIG_ARENA_MAX_RATING_DIFFERENCE]                = sConfigMgr->GetIntDefault ("Arena.MaxRatingDifference", 150);
    m_int_configs[CONFIG_ARENA_RATING_DISCARD_TIMER]                 = sConfigMgr->GetIntDefault ("Arena.RatingDiscardTimer", 10 * MINUTE * IN_MILLISECONDS);
    m_int_configs[CONFIG_ARENA_RATED_UPDATE_TIMER]                   = sConfigMgr->GetIntDefault ("Arena.RatedUpdateTimer", 5 * IN_MILLISECONDS);
    m_bool_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_POINTS]              = sConfigMgr->GetBoolDefault("Arena.AutoDistributePoints", false);
    m_int_configs[CONFIG_ARENA_AUTO_DISTRIBUTE_INTERVAL_DAYS]        = sConfigMgr->GetIntDefault ("Arena.AutoDistributeInterval", 7);
    m_bool_configs[CONFIG_ARENA_QUEUE_ANNOUNCER_ENABLE]              = sConfigMgr->GetBoolDefault("Arena.QueueAnnouncer.Enable", false);
    m_bool_configs[CONFIG_ARENA_QUEUE_ANNOUNCER_PLAYERONLY]          = sConfigMgr->GetBoolDefault("Arena.QueueAnnouncer.PlayerOnly", false);
    m_int_configs[CONFIG_ARENA_SEASON_ID]                            = sConfigMgr->GetIntDefault ("Arena.ArenaSeason.ID", 1);
    m_int_configs[CONFIG_ARENA_START_RATING]                         = sConfigMgr->GetIntDefault ("Arena.ArenaStartRating", 0);
    m_int_configs[CONFIG_ARENA_START_PERSONAL_RATING]                = sConfigMgr->GetIntDefault ("Arena.ArenaStartPersonalRating", 1000);
    m_int_configs[CONFIG_ARENA_START_MATCHMAKER_RATING]              = sConfigMgr->GetIntDefault ("Arena.ArenaStartMatchmakerRating", 1500);
    m_bool_configs[CONFIG_ARENA_SEASON_IN_PROGRESS]                  = sConfigMgr->GetBoolDefault("Arena.ArenaSeason.InProgress", true);
    m_bool_configs[CONFIG_ARENA_LOG_EXTENDED_INFO]                   = sConfigMgr->GetBoolDefault("ArenaLog.ExtendedInfo", false);

    m_int_configs[CONFIG_PVP_ITEM_LEVEL_CUTOFF]                      = sConfigMgr->GetIntDefault("PvP.Item.Level.Cut.Off", 560);
    m_int_configs[CONFIG_PVP_ITEM_LEVEL_MIN]                         = sConfigMgr->GetIntDefault("PvP.Item.Level.Min", 650);
    m_int_configs[CONFIG_PVP_ITEM_LEVEL_MAX]                         = sConfigMgr->GetIntDefault("PvP.Item.Level.Max", 690);
    m_int_configs[CONFIG_CHALLENGE_MODE_ITEM_LEVEL_MAX]              = sConfigMgr->GetIntDefault("Challenge.Mode.Item.Level.Max", 630);

    m_int_configs[CONFIG_LAST_CLIENT_BUILD]                          = sConfigMgr->GetIntDefault("LastClientBuild", 20726);
    m_bool_configs[CONFIG_OFFHAND_CHECK_AT_SPELL_UNLEARN]            = sConfigMgr->GetBoolDefault("OffhandCheckAtSpellUnlearn", true);

    /// BattlePay configs
    m_int_configs[CONFIG_BATTLEPAY_MIN_SECURITY] = sConfigMgr->GetIntDefault("BattlePay.Security", 0);
    m_bool_configs[CONFIG_BATTLEPAY_ENABLE]      = sConfigMgr->GetBoolDefault("BattlePay.Enable", true);

    // Loyalty configs
    m_bool_configs[CONFIG_LOYALTY_EVENTS_ENABLE] = sConfigMgr->GetBoolDefault("Loyalty.EnableLoyaltyEvents", false);

    m_int_configs[CONFIG_CREATURE_PICKPOCKET_REFILL] = sConfigMgr->GetIntDefault("Creature.PickPocketRefillDelay", 10 * MINUTE);

    if (int32 clientCacheId = sConfigMgr->GetIntDefault("ClientCacheVersion", 0))
    {
        // overwrite DB/old value
        if (clientCacheId > 0)
        {
            m_int_configs[CONFIG_CLIENTCACHE_VERSION] = clientCacheId;
            TC_LOG_INFO("server.loading", "Client cache version set to: %u", clientCacheId);
        }
        else
            TC_LOG_ERROR("server.loading", "ClientCacheVersion can't be negative %d, ignored.", clientCacheId);
    }

    m_int_configs[CONFIG_INSTANT_LOGOUT] = sConfigMgr->GetIntDefault("InstantLogout", SEC_MODERATOR);

    m_int_configs[CONFIG_GUILD_EVENT_LOG_COUNT] = sConfigMgr->GetIntDefault("Guild.EventLogRecordsCount", GUILD_EVENTLOG_MAX_RECORDS);
    if (m_int_configs[CONFIG_GUILD_EVENT_LOG_COUNT] > GUILD_EVENTLOG_MAX_RECORDS)
        m_int_configs[CONFIG_GUILD_EVENT_LOG_COUNT] = GUILD_EVENTLOG_MAX_RECORDS;
    m_int_configs[CONFIG_GUILD_BANK_EVENT_LOG_COUNT] = sConfigMgr->GetIntDefault("Guild.BankEventLogRecordsCount", GUILD_BANKLOG_MAX_RECORDS);
    if (m_int_configs[CONFIG_GUILD_BANK_EVENT_LOG_COUNT] > GUILD_BANKLOG_MAX_RECORDS)
        m_int_configs[CONFIG_GUILD_BANK_EVENT_LOG_COUNT] = GUILD_BANKLOG_MAX_RECORDS;

    //visibility on continents
    m_MaxVisibleDistanceOnContinents = sConfigMgr->GetFloatDefault("Visibility.Distance.Continents", DEFAULT_VISIBILITY_DISTANCE);
    if (m_MaxVisibleDistanceOnContinents < 45*sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.Continents can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceOnContinents = 45*sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceOnContinents > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.Continents can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceOnContinents = MAX_VISIBILITY_DISTANCE;
    }

    Visibility_RelocationLowerLimit = sConfigMgr->GetFloatDefault("Visibility.RelocationLowerLimit", 20.0f);
    Visibility_AINotifyDelay = sConfigMgr->GetFloatDefault("Visibility.AINotifyDelay", 1000);

    //visibility in instances
    m_MaxVisibleDistanceInInstances = sConfigMgr->GetFloatDefault("Visibility.Distance.Instances", DEFAULT_VISIBILITY_INSTANCE);
    if (m_MaxVisibleDistanceInInstances < 45*sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.Instances can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInInstances = 45*sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInInstances > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.Instances can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInInstances = MAX_VISIBILITY_DISTANCE;
    }

    //visibility in BG
    m_MaxVisibleDistanceInBG = sConfigMgr->GetFloatDefault("Visibility.Distance.BG", DEFAULT_VISIBILITY_BGARENAS);
    if (m_MaxVisibleDistanceInBG < 45 * sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.BG can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInBG = 45 * sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInBG > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.BG can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInBG = MAX_VISIBILITY_DISTANCE;
    }

    //visibility in Arenas
    m_MaxVisibleDistanceInArenas = sConfigMgr->GetFloatDefault("Visibility.Distance.Arenas", DEFAULT_VISIBILITY_BGARENAS);
    if (m_MaxVisibleDistanceInArenas < 45 * sWorld->getRate(RATE_CREATURE_AGGRO))
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.Arenas can't be less max aggro radius %f", 45*sWorld->getRate(RATE_CREATURE_AGGRO));
        m_MaxVisibleDistanceInArenas = 45 * sWorld->getRate(RATE_CREATURE_AGGRO);
    }
    else if (m_MaxVisibleDistanceInArenas > MAX_VISIBILITY_DISTANCE)
    {
        TC_LOG_ERROR("server.loading", "Visibility.Distance.Arenas can't be greater %f", MAX_VISIBILITY_DISTANCE);
        m_MaxVisibleDistanceInArenas = MAX_VISIBILITY_DISTANCE;
    }

    m_visibility_notify_periodOnContinents = sConfigMgr->GetIntDefault("Visibility.Notify.Period.OnContinents", DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    m_visibility_notify_periodInInstances = sConfigMgr->GetIntDefault("Visibility.Notify.Period.InInstances",   DEFAULT_VISIBILITY_NOTIFY_PERIOD);
    m_visibility_notify_periodInBGArenas = sConfigMgr->GetIntDefault("Visibility.Notify.Period.InBGArenas",    DEFAULT_VISIBILITY_NOTIFY_PERIOD);

    ///- Load the CharDelete related config options
    m_int_configs[CONFIG_CHARDELETE_METHOD] = sConfigMgr->GetIntDefault("CharDelete.Method", 0);
    m_int_configs[CONFIG_CHARDELETE_MIN_LEVEL] = sConfigMgr->GetIntDefault("CharDelete.MinLevel", 0);
    m_int_configs[CONFIG_CHARDELETE_KEEP_DAYS] = sConfigMgr->GetIntDefault("CharDelete.KeepDays", 30);

    ///- Read the "Data" directory from the config file
    std::string dataPath = sConfigMgr->GetStringDefault("DataDir", "./");
    if (dataPath.at(dataPath.length()-1) != '/' && dataPath.at(dataPath.length()-1) != '\\')
        dataPath.push_back('/');

    if (reload)
    {
        if (dataPath != m_dataPath)
            TC_LOG_ERROR("server.loading", "DataDir option can't be changed at worldserver.conf reload, using current value (%s).", m_dataPath.c_str());
    }
    else
    {
        m_dataPath = dataPath;
        TC_LOG_INFO("server.loading", "Using DataDir %s", m_dataPath.c_str());
    }

    m_bool_configs[CONFIG_ENABLE_MMAPS] = sConfigMgr->GetBoolDefault("mmap.enablePathFinding", true);
    

    m_bool_configs[CONFIG_ENABLE_QUEST]              = sConfigMgr->GetBoolDefault("loading.quest", true);
    m_bool_configs[CONFIG_ENABLE_LOOTS]              = sConfigMgr->GetBoolDefault("loading.loot", true);
    m_bool_configs[CONFIG_ENABLE_GAMEOBJECTS]        = sConfigMgr->GetBoolDefault("loading.gameobject", true);
    m_bool_configs[CONFIG_ENABLE_LOCALES]            = sConfigMgr->GetBoolDefault("loading.locales", true);
    m_bool_configs[CONFIG_ENABLE_ONLY_SPECIFIC_MAP]  = sConfigMgr->GetBoolDefault("loading.onlyspecificmaps", false);
    m_bool_configs[CONFIG_ENABLE_RESEARCH_SITE_LOAD] = sConfigMgr->GetBoolDefault("loading.researchsite", true);
    m_bool_configs[CONFIG_ENABLE_ITEM_SPEC_LOAD]     = sConfigMgr->GetBoolDefault("loading.itemspecload", true);

    FillMapsToLoad();
    TC_LOG_ERROR("server.worldserver", "WORLD: MMap data directory is: %smmaps", m_dataPath.c_str());

    m_bool_configs[CONFIG_VMAP_INDOOR_CHECK] = sConfigMgr->GetBoolDefault("vmap.enableIndoorCheck", 0);
    bool enableIndoor = sConfigMgr->GetBoolDefault("vmap.enableIndoorCheck", true);
    bool enableLOS = sConfigMgr->GetBoolDefault("vmap.enableLOS", true);
    bool enableHeight = sConfigMgr->GetBoolDefault("vmap.enableHeight", true);
    std::string ignoreSpellIds = sConfigMgr->GetStringDefault("vmap.ignoreSpellIds", "");

    if (!enableHeight)
        TC_LOG_ERROR("server.loading", "VMap height checking disabled! Creatures movements and other various things WILL be broken! Expect no support.");

    VMAP::VMapFactory::createOrGetVMapManager()->setEnableLineOfSightCalc(enableLOS);
    VMAP::VMapFactory::createOrGetVMapManager()->setEnableHeightCalc(enableHeight);
    TC_LOG_INFO("server.loading", "VMap support included. LineOfSight:%i, getHeight:%i, indoorCheck:%i", enableLOS, enableHeight, enableIndoor);
    TC_LOG_INFO("server.loading", "VMap data directory is: %svmaps", m_dataPath.c_str());

    m_int_configs[CONFIG_MAX_WHO] = sConfigMgr->GetIntDefault("MaxWhoListReturns", 49);
    m_bool_configs[CONFIG_LIMIT_WHO_ONLINE] = sConfigMgr->GetBoolDefault("LimitWhoOnline", true);
    m_bool_configs[CONFIG_START_ALL_SPELLS] = sConfigMgr->GetBoolDefault("PlayerStart.AllSpells", false);
    if (m_bool_configs[CONFIG_START_ALL_SPELLS])
        TC_LOG_WARN("server.loading", "PlayerStart.AllSpells enabled - may not function as intended!");
    m_int_configs[CONFIG_HONOR_AFTER_DUEL] = sConfigMgr->GetIntDefault("HonorPointsAfterDuel", 0);
    m_bool_configs[CONFIG_START_ALL_EXPLORED] = sConfigMgr->GetBoolDefault("PlayerStart.MapsExplored", false);
    m_bool_configs[CONFIG_START_ALL_REP] = sConfigMgr->GetBoolDefault("PlayerStart.AllReputation", false);
    m_bool_configs[CONFIG_ALWAYS_MAXSKILL] = sConfigMgr->GetBoolDefault("AlwaysMaxWeaponSkill", false);
    m_bool_configs[CONFIG_PVP_TOKEN_ENABLE] = sConfigMgr->GetBoolDefault("PvPToken.Enable", false);
    m_int_configs[CONFIG_PVP_TOKEN_MAP_TYPE] = sConfigMgr->GetIntDefault("PvPToken.MapAllowType", 4);
    m_int_configs[CONFIG_PVP_TOKEN_ID] = sConfigMgr->GetIntDefault("PvPToken.ItemID", 29434);
    m_int_configs[CONFIG_PVP_TOKEN_COUNT] = sConfigMgr->GetIntDefault("PvPToken.ItemCount", 1);
    if (m_int_configs[CONFIG_PVP_TOKEN_COUNT] < 1)
        m_int_configs[CONFIG_PVP_TOKEN_COUNT] = 1;

    m_bool_configs[CONFIG_NO_RESET_TALENT_COST] = sConfigMgr->GetBoolDefault("NoResetTalentsCost", false);
    m_bool_configs[CONFIG_SHOW_KICK_IN_WORLD] = sConfigMgr->GetBoolDefault("ShowKickInWorld", false);
    m_int_configs[CONFIG_INTERVAL_LOG_UPDATE] = sConfigMgr->GetIntDefault("RecordUpdateTimeDiffInterval", 60000);
    m_int_configs[CONFIG_MIN_LOG_UPDATE] = sConfigMgr->GetIntDefault("MinRecordUpdateTimeDiff", 100);
    m_int_configs[CONFIG_NUMTHREADS] = sConfigMgr->GetIntDefault("MapUpdate.Threads", 1);
    m_int_configs[CONFIG_MAX_RESULTS_LOOKUP_COMMANDS] = sConfigMgr->GetIntDefault("Command.LookupMaxResults", 0);

    // chat logging
    m_bool_configs[CONFIG_CHATLOG_CHANNEL] = sConfigMgr->GetBoolDefault("ChatLogs.Channel", false);
    m_bool_configs[CONFIG_CHATLOG_WHISPER] = sConfigMgr->GetBoolDefault("ChatLogs.Whisper", false);
    m_bool_configs[CONFIG_CHATLOG_SYSCHAN] = sConfigMgr->GetBoolDefault("ChatLogs.SysChan", false);
    m_bool_configs[CONFIG_CHATLOG_PARTY] = sConfigMgr->GetBoolDefault("ChatLogs.Party", false);
    m_bool_configs[CONFIG_CHATLOG_RAID] = sConfigMgr->GetBoolDefault("ChatLogs.Raid", false);
    m_bool_configs[CONFIG_CHATLOG_GUILD] = sConfigMgr->GetBoolDefault("ChatLogs.Guild", false);
    m_bool_configs[CONFIG_CHATLOG_PUBLIC] = sConfigMgr->GetBoolDefault("ChatLogs.Public", false);
    m_bool_configs[CONFIG_CHATLOG_ADDON] = sConfigMgr->GetBoolDefault("ChatLogs.Addon", false);
    m_bool_configs[CONFIG_CHATLOG_BGROUND] = sConfigMgr->GetBoolDefault("ChatLogs.Battleground", false);

    // Warden
    m_bool_configs[CONFIG_WARDEN_ENABLED]              = sConfigMgr->GetBoolDefault("Warden.Enabled", false);
    m_int_configs[CONFIG_WARDEN_NUM_MEM_CHECKS]        = sConfigMgr->GetIntDefault("Warden.NumMemChecks", 3);
    m_int_configs[CONFIG_WARDEN_NUM_OTHER_CHECKS]      = sConfigMgr->GetIntDefault("Warden.NumOtherChecks", 7);
    m_int_configs[CONFIG_WARDEN_CLIENT_BAN_DURATION]   = sConfigMgr->GetIntDefault("Warden.BanDuration", 86400);
    m_int_configs[CONFIG_WARDEN_CLIENT_CHECK_HOLDOFF]  = sConfigMgr->GetIntDefault("Warden.ClientCheckHoldOff", 30);
    m_int_configs[CONFIG_WARDEN_CLIENT_FAIL_ACTION]    = sConfigMgr->GetIntDefault("Warden.ClientCheckFailAction", 0);
    m_int_configs[CONFIG_WARDEN_CLIENT_RESPONSE_DELAY] = sConfigMgr->GetIntDefault("Warden.ClientResponseDelay", 600);

    // Dungeon finder
    m_bool_configs[CONFIG_DUNGEON_FINDER_ENABLE] = sConfigMgr->GetBoolDefault("DungeonFinder.Enable", false);

    m_int_configs[CONFIG_LFG_OPTIONSMASK] = sConfigMgr->GetIntDefault("DungeonFinder.OptionsMask", 1);
    m_bool_configs[CONFIG_LFG_CASTDESERTER] = sConfigMgr->GetBoolDefault("DungeonFinder.CastDeserter", false);
    m_bool_configs[CONFIG_LFG_OVERRIDE_ROLES_REQUIRED] = sConfigMgr->GetBoolDefault("DungeonFinder.OverrideRolesRequired", false);
    m_bool_configs[CONFIG_LFG_MULTIQUEUE_ENABLED] = sConfigMgr->GetBoolDefault("DungeonFinder.MultiqueueEnabled", false);
    m_bool_configs[CONFIG_LFG_KEEP_QUEUES_IN_DUNGEON] = sConfigMgr->GetBoolDefault("DungeonFinder.KeepQueuesInDungeon", false);
    m_int_configs[CONFIG_LFG_TANKS_NEEDED] = sConfigMgr->GetIntDefault("DungeonFinder.TanksNeeded", 1);
    m_int_configs[CONFIG_LFG_HEALERS_NEEDED] = sConfigMgr->GetIntDefault("DungeonFinder.HealersNeeded", 1);
    m_int_configs[CONFIG_LFG_DPS_NEEDED] = sConfigMgr->GetIntDefault("DungeonFinder.DPSNeeded", 3);
    m_int_configs[CONFIG_LFG_SHORTAGE_CHECK_INTERVAL] = sConfigMgr->GetIntDefault("DungeonFinder.ShortageCheckInterval", 5);
    m_int_configs[CONFIG_LFG_SHORTAGE_PERCENT] = sConfigMgr->GetIntDefault("DungeonFinder.ShortagePercent", 50);
    m_int_configs[CONFIG_LFG_MAX_LFR_QUEUES] = sConfigMgr->GetIntDefault("DungeonFinder.MaxLfrQueues", 3);

    // DBC_ItemAttributes
    m_bool_configs[CONFIG_DBC_ENFORCE_ITEM_ATTRIBUTES] = sConfigMgr->GetBoolDefault("DBC.EnforceItemAttributes", true);

    // Max instances per hour
    m_int_configs[CONFIG_MAX_INSTANCES_PER_HOUR] = sConfigMgr->GetIntDefault("AccountInstancesPerHour", 5);

    m_int_configs[CONFIG_AUTO_SERVER_RESTART_HOUR] = sConfigMgr->GetIntDefault("Server.Auto.RestartHour", 4);
    if (m_int_configs[CONFIG_AUTO_SERVER_RESTART_HOUR] > 23)
    {
        m_int_configs[CONFIG_AUTO_SERVER_RESTART_HOUR] = 4;
    }

    // AutoBroadcast
    m_bool_configs[CONFIG_AUTOBROADCAST] = sConfigMgr->GetBoolDefault("AutoBroadcast.On", false);
    m_int_configs[CONFIG_AUTOBROADCAST_CENTER] = sConfigMgr->GetIntDefault("AutoBroadcast.Center", 0);
    m_int_configs[CONFIG_AUTOBROADCAST_INTERVAL] = sConfigMgr->GetIntDefault("AutoBroadcast.Timer", 60000);

    if (reload)
    {
        m_timers[WUPDATE_AUTOBROADCAST].SetInterval(m_int_configs[CONFIG_AUTOBROADCAST_INTERVAL]);
        m_timers[WUPDATE_AUTOBROADCAST].Reset();
    }

    // MySQL ping time interval
    m_int_configs[CONFIG_DB_PING_INTERVAL] = sConfigMgr->GetIntDefault("MaxPingTime", 30);

    //Reset Duel Cooldown
    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_ON_START] = sConfigMgr->GetBoolDefault("DuelReset.Cooldown.OnStart", false);
    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_ON_FINISH] = sConfigMgr->GetBoolDefault("DuelReset.Cooldown.OnFinish", false);
    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_ONLY_IN_ELWYNN_AND_DUROTAR] = sConfigMgr->GetBoolDefault("DuelReset.Cooldown.Only.in.Elwynn.and.Durotar", false);
    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_MAX_ENERGY_ON_START] = sConfigMgr->GetBoolDefault("DuelReset.Cooldown.Max.Energy.OnStart", false);
    m_bool_configs[CONFIG_DUEL_RESET_COOLDOWN_RESET_ENERGY_ON_START] = sConfigMgr->GetBoolDefault("DuelReset.Cooldown.Reset.Energy.OnStart", false);

    // Guild save interval
    m_bool_configs[CONFIG_GUILD_LEVELING_ENABLED] = sConfigMgr->GetBoolDefault("Guild.LevelingEnabled", true);
    m_int_configs[CONFIG_GUILD_SAVE_INTERVAL] = sConfigMgr->GetIntDefault("Guild.SaveInterval", 15);
    m_int_configs[CONFIG_GUILD_MAX_LEVEL] = sConfigMgr->GetIntDefault("Guild.MaxLevel", 25);
    m_int_configs[CONFIG_GUILD_UNDELETABLE_LEVEL] = sConfigMgr->GetIntDefault("Guild.UndeletableLevel", 4);
    rate_values[RATE_XP_GUILD_MODIFIER] = sConfigMgr->GetFloatDefault("Guild.XPModifier", 0.25f);
    m_int_configs[CONFIG_GUILD_DAILY_XP_CAP] = sConfigMgr->GetIntDefault("Guild.DailyXPCap", 7807500);
    m_int_configs[CONFIG_GUILD_WEEKLY_REP_CAP] = sConfigMgr->GetIntDefault("Guild.WeeklyReputationCap", 4375);

#ifndef CROSS
    // Blackmarket
    m_int_configs[CONFIG_BLACKMARKET_MAX_AUCTIONS] = sConfigMgr->GetIntDefault("BlackMarket.MaxAuctions", 10);
    m_int_configs[CONFIG_BLACKMARKET_AUCTION_DELAY] = sConfigMgr->GetIntDefault("BlackMarket.AuctionDelay", 120);
    m_int_configs[CONFIG_BLACKMARKET_AUCTION_DELAY_MOD] = sConfigMgr->GetIntDefault("BlackMarket.AuctionDelayMod", 60);

    // misc
    m_bool_configs[CONFIG_PDUMP_NO_PATHS] = sConfigMgr->GetBoolDefault("PlayerDump.DisallowPaths", true);
    m_bool_configs[CONFIG_PDUMP_NO_OVERWRITE] = sConfigMgr->GetBoolDefault("PlayerDump.DisallowOverwrite", true);
#endif

    // call ScriptMgr if we're reloading the configuration
    m_bool_configs[CONFIG_WINTERGRASP_ENABLE] = sConfigMgr->GetBoolDefault("Wintergrasp.Enable", false);
    m_int_configs[CONFIG_WINTERGRASP_PLR_MAX] = sConfigMgr->GetIntDefault("Wintergrasp.PlayerMax", 100);
    m_int_configs[CONFIG_WINTERGRASP_PLR_MIN] = sConfigMgr->GetIntDefault("Wintergrasp.PlayerMin", 0);
    m_int_configs[CONFIG_WINTERGRASP_PLR_MIN_LVL] = sConfigMgr->GetIntDefault("Wintergrasp.PlayerMinLvl", 77);
    m_int_configs[CONFIG_WINTERGRASP_BATTLETIME] = sConfigMgr->GetIntDefault("Wintergrasp.BattleTimer", 30);
    m_int_configs[CONFIG_WINTERGRASP_NOBATTLETIME] = sConfigMgr->GetIntDefault("Wintergrasp.NoBattleTimer", 150);
    m_int_configs[CONFIG_WINTERGRASP_RESTART_AFTER_CRASH] = sConfigMgr->GetIntDefault("Wintergrasp.CrashRestartTimer", 10);

    // Stats limits
    m_bool_configs[CONFIG_STATS_LIMITS_ENABLE] = sConfigMgr->GetBoolDefault("Stats.Limits.Enable", false);
    m_float_configs[CONFIG_STATS_LIMITS_DODGE] = sConfigMgr->GetFloatDefault("Stats.Limits.Dodge", 95.0f);
    m_float_configs[CONFIG_STATS_LIMITS_PARRY] = sConfigMgr->GetFloatDefault("Stats.Limits.Parry", 95.0f);
    m_float_configs[CONFIG_STATS_LIMITS_BLOCK] = sConfigMgr->GetFloatDefault("Stats.Limits.Block", 95.0f);
    m_float_configs[CONFIG_STATS_LIMITS_CRIT] = sConfigMgr->GetFloatDefault("Stats.Limits.Crit", 95.0f);

    // LFR
    m_float_configs[CONFIG_LFR_DROP_CHANCE] = sConfigMgr->GetFloatDefault("Lfr.DropChance", 25.0f);

    // Anticheat
    m_bool_configs[CONFIG_ANTICHEAT_ENABLE] = sConfigMgr->GetBoolDefault("Anticheat.Enable", true);
    m_int_configs[CONFIG_ANTICHEAT_REPORTS_INGAME_NOTIFICATION] = sConfigMgr->GetIntDefault("Anticheat.ReportsForIngameWarnings", 70);
    m_int_configs[CONFIG_ANTICHEAT_DETECTIONS_ENABLED] = sConfigMgr->GetIntDefault("Anticheat.DetectionsEnabled",31);
    m_int_configs[CONFIG_ANTICHEAT_MAX_REPORTS_FOR_DAILY_REPORT] = sConfigMgr->GetIntDefault("Anticheat.MaxReportsForDailyReport",70);
    m_int_configs[CONFIG_ANTICHEAT_MAX_REPORTS_BEFORE_BAN] = sConfigMgr->GetIntDefault("Anticheat.MaxReportsBeforeBan", 200);
    m_int_configs[CONFIG_ANTICHEAT_BAN_CHECK_TIME_RANGE] = sConfigMgr->GetIntDefault("Anticheat.BanCheckTimeRange", 120);

    // Announce server for a ban
    m_bool_configs[CONFIG_ANNOUNCE_BAN] = sConfigMgr->GetBoolDefault("AnnounceBan", false);
    m_bool_configs[CONFIG_ANNOUNCE_MUTE] = sConfigMgr->GetBoolDefault("AnnounceMute", false);
    m_bool_configs[CONFIG_SPELL_FORBIDDEN] = sConfigMgr->GetBoolDefault("SpellForbidden", false);

    m_bool_configs[CONFIG_DISABLE_RESTART] = sConfigMgr->GetBoolDefault("DisableRestart", false);

    // Vip Commands
    m_bool_configs[CONFIG_VIP_DEBUFF_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Debuff.Command", false);
    m_bool_configs[CONFIG_VIP_BANK_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Bank.Command", true);
    m_bool_configs[CONFIG_VIP_REPAIR_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Repair.Command", false);
    m_bool_configs[CONFIG_VIP_RESET_TALENTS_COMMAND] = sConfigMgr->GetBoolDefault("VIP.Reset.Talents.Command", false);
    m_bool_configs[CONFIG_VIP_TAXI_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Taxi.Command", false);
    m_bool_configs[CONFIG_VIP_HOME_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Home.Command", true);
    m_bool_configs[CONFIG_VIP_CAPITAL_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Capital.Command", false);
    m_bool_configs[CONFIG_VIP_EXCHANGE_ARENA_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Exchange.Arena.Command",false);
    m_bool_configs[CONFIG_VIP_EXCHANGE_FROST_COMMAND] = sConfigMgr->GetBoolDefault("Vip.Exchange.Frost.Command",false);
    m_int_configs[CONFIG_VIP_RATE_EXHANGE_HONOR_IN_ARENA]  = sConfigMgr->GetIntDefault("Vip.Rate.Exchange.Honor.In.Arena", 1);
    m_int_configs[CONFIG_VIP_RATE_EXHANGE_TRIUMPH_IN_FROST]  = sConfigMgr->GetIntDefault("Vip.Rate.Exchange.Triumph.In.Frost", 1);

    // Mail Antispam
    m_bool_configs[CONFIG_ANTISPAM_ENABLED] = sConfigMgr->GetBoolDefault("Antispam.Mail.Enabled", false);
    m_int_configs[CONFIG_ANTISPAM_MAIL_TIMER] = sConfigMgr->GetIntDefault("Antispam.Mail.Timer", 3600) * IN_MILLISECONDS;
    m_int_configs[CONFIG_ANTISPAM_MAIL_COUNT] = sConfigMgr->GetIntDefault("Antispam.Mail.Count", 10);

    m_bool_configs[CONFIG_TEMPLATES_ENABLED] = sConfigMgr->GetBoolDefault("Character.Templates.Enabled", false);

    m_bool_configs[CONFIG_AOE_LOOT_ENABLED] = sConfigMgr->GetBoolDefault("LootAoe.Enabled", true);

    // Lexics Cutter settings
    m_bool_configs[CONFIG_LEXICS_CUTTER_ENABLE] = sConfigMgr->GetBoolDefault("LexicsCutterEnable", true);

    m_bool_configs[CONFIG_ACHIEVEMENT_DISABLE] = sConfigMgr->GetBoolDefault("Achievement.disable", false);

    m_bool_configs[CONFIG_MOP_TRANSFER_ENABLE] = sConfigMgr->GetBoolDefault("MopTransfer.enable", false);

    m_bool_configs[CONFIG_FUN_ENABLE] = sConfigMgr->GetBoolDefault("Fun.Enable", false);
    m_bool_configs[CONFIG_LOG_PACKETS] = sConfigMgr->GetBoolDefault("LogPackets", true);
    m_bool_configs[CONFIG_WEB_DATABASE_ENABLE] = sConfigMgr->GetBoolDefault("WebDatabase.enable", false);


    m_bool_configs[CONFIG_LOG_PACKETS] = sConfigMgr->GetBoolDefault("LogPackets", true);

    // Specialization check at spell cast (as it may breaks spells)
    m_bool_configs[CONFIG_DISABLE_SPELL_SPECIALIZATION_CHECK] = sConfigMgr->GetBoolDefault("DisableSpellSpecializationCheck", false);

    m_int_configs[CONFIG_ACCOUNT_BIND_ALLOWED_GROUP_MASK] = sConfigMgr->GetIntDefault("AccountBind.AllowedGroupRealmMask", 0x7FFFFFFF);
    m_int_configs[CONFIG_ACCOUNT_BIND_GROUP_MASK] = sConfigMgr->GetIntDefault("AccountBind.GroupRealmMask", 1);
    m_int_configs[CONFIG_ACCOUNT_BIND_SHOP_GROUP_MASK] = sConfigMgr->GetIntDefault("AccountBind.ShopGroupMask", 0x7FFFFFFF);

    m_bool_configs[CONFIG_MUST_HAVE_AUTHENTICATOR_ACCESS] = sConfigMgr->GetBoolDefault("Security.MustHaveAuthenticatorAccess", true);

    std::string fn_analogsfile = sConfigMgr->GetStringDefault("LexicsCutterAnalogsFile", "letter_analogs.txt");
    std::string fn_wordsfile = sConfigMgr->GetStringDefault("LexicsCutterWordsFile", "innormative_words.txt");

    if (reload)
    {
        if (m_lexicsCutter)
        {
            delete m_lexicsCutter;
            m_lexicsCutter = nullptr;
        }
    }

    // Load Lexics Cutter
    m_lexicsCutter = new LexicsCutter();
    m_lexicsCutter->ReadLetterAnalogs(fn_analogsfile);
    m_lexicsCutter->ReadInnormativeWords(fn_wordsfile);
    m_lexicsCutter->MapInnormativeWords();

    // read additional parameters
    m_lexicsCutter->IgnoreLetterRepeat = sConfigMgr->GetBoolDefault("LexicsCutterIgnoreRepeats", true);
    m_lexicsCutter->IgnoreMiddleSpaces = sConfigMgr->GetBoolDefault("LexicsCutterIgnoreSpaces", true);
    m_lexicsCutter->CheckLetterContains = sConfigMgr->GetBoolDefault("LexicsCutterCheckContains", false);

#ifndef CROSS
    // InterRealm settings
    m_bool_configs[CONFIG_INTERREALM_ENABLE] = sConfigMgr->GetBoolDefault("InterRealm.Enabled", false);
#endif
	///< Spell queue state (on/off)
	m_bool_configs[CONFIG_SPELL_QUEUE_STATE] = sConfigMgr->GetBoolDefault("SpellQueueSystem.Enabled", true);
    m_int_configs[CONFIG_SPELLOG_FLAGS] = sConfigMgr->GetIntDefault("SpellLog.Flags", SPELLLOG_OUTPUT_FLAG_PLAYER);

    if (reload)
        sScriptMgr->OnConfigLoad(reload);
}

extern void LoadGameObjectModelList();

/// Initialize the World
void World::SetInitialWorldSettings()
{
    ///- Server startup begin
    uint32 startupBegin = getMSTime();

    ///- Initialize the random number generator
    srand((unsigned int)time(NULL));
    std::srand((unsigned int)time(NULL));

    ///- Initialize detour memory management
    dtAllocSetCustom(dtCustomAlloc, dtCustomFree);

    ///- Initialize VMapManager function pointers (to untangle game/collision circular deps)
    if (VMAP::VMapManager2* vmmgr2 = dynamic_cast<VMAP::VMapManager2*>(VMAP::VMapFactory::createOrGetVMapManager()))
    {
        vmmgr2->GetLiquidFlagsPtr = &GetLiquidFlags;
        vmmgr2->IsVMAPDisabledForPtr = &DisableMgr::IsVMAPDisabledFor;
    }

    ///- Initialize config settings
    LoadConfigSettings();

    ///- Load Motd from database
    LoadDBMotd();

    ///- Initialize Allowed Security Level
    LoadDBAllowedSecurityLevel();

    ///- Init highest guids before any table loading to prevent using not initialized guids in some code.
    sObjectMgr->SetHighestGuids();

    ///- Check the existence of the map files for all races' startup areas.
    if (!MapManager::ExistMapAndVMap(0, -6240.32f, 331.033f)
        || !MapManager::ExistMapAndVMap(0, -8949.95f, -132.493f)
        || !MapManager::ExistMapAndVMap(1, -618.518f, -4251.67f)
        || !MapManager::ExistMapAndVMap(0, 1676.35f, 1677.45f)
        || !MapManager::ExistMapAndVMap(1, 10311.3f, 832.463f)
        || !MapManager::ExistMapAndVMap(1, -2917.58f, -257.98f)
        || (m_int_configs[CONFIG_EXPANSION] && (
            !MapManager::ExistMapAndVMap(530, 10349.6f, -6357.29f) ||
            !MapManager::ExistMapAndVMap(530, -3961.64f, -13931.2f))))
    {
        TC_LOG_ERROR("server.loading", "Correct *.map files not found in path '%smaps' or *.vmtree/*.vmtile files in '%svmaps'. Please place *.map/*.vmtree/*.vmtile files in appropriate directories or correct the DataDir value in the worldserver.conf file.", m_dataPath.c_str(), m_dataPath.c_str());
        exit(1);
    }

    ///- Initialize pool manager
    sPoolMgr->Initialize();

    ///- Initialize game event manager
    sGameEventMgr->Initialize();

    //- Initialize TimeDiffMgr
    sTimeDiffMgr->Initialize();

    ///- Loading strings. Getting no records means core load has to be canceled because no error message can be output.

    TC_LOG_INFO("server.loading", "Loading Trinity strings...");
    if (!sObjectMgr->LoadTrinityStrings())
        exit(1);                                            // Error message displayed in function already

    ///- Update the realm entry in the database with the realm type from the config file
    //No SQL injection as values are treated as integers

#ifndef CROSS
    ///- Remove the bones (they should not exist in DB though) and old corpses after a restart
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_OLD_CORPSES);
    stmt->setUInt32(0, 3 * DAY);
    CharacterDatabase.Execute(stmt);
#endif

    ///- Load the DBC files
    TC_LOG_INFO("server.loading", "Initialize data stores...");
    LoadDBCStores(m_dataPath);
    LoadDB2Stores(m_dataPath);
    DetectDBCLang();

    TC_LOG_INFO("server.loading", "Initialize Spell Difficulty ...");
    sSpellMgr->InitializeSpellDifficulty();

    /// Load weighted graph on taxi nodes path
    sTaxiPathGraph.Initialize();

    TC_LOG_INFO("server.loading", "Loading SpellInfo store...");
    sSpellMgr->LoadSpellInfoStore();

    TC_LOG_INFO("server.loading", "Loading TalentSpellInfo store....");
    sSpellMgr->LoadTalentSpellInfo();

    TC_LOG_INFO("server.loading", "Loading SpellPowerInfo store....");
    sSpellMgr->LoadSpellPowerInfo();

    TC_LOG_INFO("server.loading", "Loading SkillLineAbilityMultiMap Data...");
    sSpellMgr->LoadSkillLineAbilityMap();

    TC_LOG_INFO("server.loading", "Loading Spell custom attributes...");
    sSpellMgr->LoadSpellCustomAttr();

    if (sWorld->getBoolConfig(CONFIG_ENABLE_RESEARCH_SITE_LOAD))
    {
        TC_LOG_INFO("server.loading", "Loading Research Site Zones...");
        sObjectMgr->LoadResearchSiteZones();

        TC_LOG_INFO("server.loading", "Loading Research Site Loot...");
        sObjectMgr->LoadResearchSiteLoot();
    }

    TC_LOG_INFO("server.loading", "Loading GameObject models...");
    LoadGameObjectModelList();

    TC_LOG_INFO("server.loading", "Loading Script Names...");
    sObjectMgr->LoadScriptNames();

    TC_LOG_INFO("server.loading", "Loading Instance Template...");
    sObjectMgr->LoadInstanceTemplate();

    // Must be called before `creature_respawn`/`gameobject_respawn` tables
    TC_LOG_INFO("server.loading", "Loading instances...");
    sInstanceSaveMgr->LoadInstances();

    uint32 oldMSTime = getMSTime();

    TC_LOG_INFO("server.loading", "Loading Creature Texts...");
    sCreatureTextMgr->LoadCreatureTexts();

    if (sWorld->getBoolConfig(CONFIG_ENABLE_LOCALES))
    {
        TC_LOG_INFO("server.loading", "Loading Localization strings...");
        sObjectMgr->LoadCreatureLocales();
        sObjectMgr->LoadGameObjectLocales();
        sObjectMgr->LoadQuestLocales();
        sObjectMgr->LoadNpcTextLocales();
        sObjectMgr->LoadPageTextLocales();
        sObjectMgr->LoadGossipMenuItemsLocales();
        sObjectMgr->LoadPointOfInterestLocales();

        TC_LOG_INFO("server.loading", "Loading Creature Text Locales...");
        sCreatureTextMgr->LoadCreatureTextLocales();
    }

    sObjectMgr->SetDBCLocaleIndex(GetDefaultDbcLocale());        // Get once for all the locale index of DBC language (console/broadcasts)
    TC_LOG_INFO("server.loading", ">> Localization strings loaded in %u ms", GetMSTimeDiffToNow(oldMSTime));

    TC_LOG_INFO("server.loading", "Loading Page Texts...");
    sObjectMgr->LoadPageTexts();

    TC_LOG_INFO("server.loading", "Loading Game Object Templates...");         // must be after LoadPageTexts
    sObjectMgr->LoadGameObjectTemplate();

    TC_LOG_INFO("server.loading", "Loading Garrison Plot Building Content...");
    sObjectMgr->LoadGarrisonPlotBuildingContent();

    TC_LOG_INFO("server.loading", "Loading Npc Recipes Conditions...");
    sObjectMgr->LoadNpcRecipesConditions();

    TC_LOG_INFO("server.loading", "Loading Transport templates...");
    sTransportMgr->LoadTransportTemplates();

    TC_LOG_INFO("server.loading", "Loading Spell Rank Data...");
    sSpellMgr->LoadSpellRanks();

    TC_LOG_INFO("server.loading", "Loading Spell Required Data...");
    sSpellMgr->LoadSpellRequired();

    TC_LOG_INFO("server.loading", "Loading Spell Group types...");
    sSpellMgr->LoadSpellGroups();

    TC_LOG_INFO("server.loading", "Loading Spell Learn Skills...");
    sSpellMgr->LoadSpellLearnSkills();                           // must be after LoadSpellRanks

    TC_LOG_INFO("server.loading", "Loading Spell Learn Spells...");
    sSpellMgr->LoadSpellLearnSpells();

    TC_LOG_INFO("server.loading", "Loading Spell Proc Event conditions...");
    sSpellMgr->LoadSpellProcEvents();

    TC_LOG_INFO("server.loading", "Loading Spell Proc conditions and data...");
    sSpellMgr->LoadSpellProcs();

    TC_LOG_INFO("server.loading", "Loading Spell Bonus Data...");
    sSpellMgr->LoadSpellBonusess();

    TC_LOG_INFO("server.loading", "Loading Aggro Spells Definitions...");
    sSpellMgr->LoadSpellThreats();

    TC_LOG_INFO("server.loading", "Loading Spell Group Stack Rules...");
    sSpellMgr->LoadSpellGroupStackRules();

    TC_LOG_INFO("server.loading", "Loading forbidden spells...");
    sSpellMgr->LoadForbiddenSpells();

    TC_LOG_INFO("server.loading", "Loading Spell Phase Dbc Info...");
    sObjectMgr->LoadSpellPhaseInfo();

    TC_LOG_INFO("server.loading", "Loading NPC Texts...");
    sObjectMgr->LoadGossipText();

    TC_LOG_INFO("server.loading", "Loading Enchant Spells Proc datas...");
    sSpellMgr->LoadSpellEnchantProcData();

    TC_LOG_INFO("server.loading", "Loading Item Random Enchantments Table...");
    LoadRandomEnchantmentsTable();

    TC_LOG_INFO("server.loading", "Loading Disables");
    DisableMgr::LoadDisables();                                                             // must be before loading quests and items

    TC_LOG_INFO("server.loading", "Loading Items...");                           ///< must be after LoadRandomEnchantmentsTable and LoadPageTexts
    sObjectMgr->LoadItemTemplates();
    sObjectMgr->LoadItemTemplateCorrections();

    TC_LOG_INFO("server.loading", "Loading Item set names...");                  ///< must be after LoadItemPrototypes
    sObjectMgr->LoadItemTemplateAddon();

    TC_LOG_INFO("misc", "Loading Item Scripts...");                           ///< must be after LoadItemPrototypes
    sObjectMgr->LoadItemScriptNames();

    TC_LOG_INFO("misc", "Loading Item Specs override...");                    ///< must be after LoadItemPrototypes
    sObjectMgr->LoadItemSpecsOverride();

    if (sWorld->getBoolConfig(CONFIG_ENABLE_ITEM_SPEC_LOAD))
    {
        TC_LOG_INFO("misc", "Loading Item Specs...");                   // must be after LoadItemPrototypes
        sObjectMgr->LoadItemSpecs();
    }

    TC_LOG_INFO("misc", "Loading Item Bonus Group...");                       ///< must be after LoadItemPrototypes
    sObjectMgr->LoadItemBonusGroup();

    TC_LOG_INFO("misc", "Loading Item Bonus Group Linked...");                ///< must be after LoadItemPrototypes
    sObjectMgr->LoadItemBonusGroupLinked();

    TC_LOG_INFO("server.loading", "Loading Creature Model Based Info Data...");
    sObjectMgr->LoadCreatureModelInfo();

    TC_LOG_INFO("server.loading", "Loading Creature templates...");
    sObjectMgr->LoadCreatureTemplates();

    TC_LOG_INFO("server.loading", "Loading Equipment templates...");             // Must be after LoadCreatureTemplate
    sObjectMgr->LoadEquipmentTemplates();

    TC_LOG_INFO("server.loading", "Loading Creature templates difficulties...");
    sObjectMgr->LoadCreatureTemplatesDifficulties();

    TC_LOG_INFO("server.loading", "Loading Creature template addons...");
    sObjectMgr->LoadCreatureTemplateAddons();

    TC_LOG_INFO("server.loading", "Loading Reputation Reward Rates...");
    sObjectMgr->LoadReputationRewardRate();

    TC_LOG_INFO("server.loading", "Loading Currency Loot Templates...");
    sObjectMgr->LoadCurrencyOnKill();

    TC_LOG_INFO("server.loading", "Loading Currency Loot Templates Personnal...");
    sObjectMgr->LoadPersonnalCurrencyOnKill();

    TC_LOG_INFO("server.loading", "Loading Creature Reputation OnKill Data...");
    sObjectMgr->LoadReputationOnKill();

    TC_LOG_INFO("server.loading", "Loading Reputation Spillover Data...");
    sObjectMgr->LoadReputationSpilloverTemplate();

    TC_LOG_INFO("server.loading", "Loading Points Of Interest Data...");
    sObjectMgr->LoadPointsOfInterest();

    TC_LOG_INFO("server.loading", "Loading Creature Base Stats...");
    sObjectMgr->LoadCreatureClassLevelStats();

    TC_LOG_INFO("server.loading", "Loading Creature Group Size Stats...");
    sObjectMgr->LoadCreatureGroupSizeStats();

    TC_LOG_INFO("server.loading", "Loading Creature Data...");
    sObjectMgr->LoadCreatures();

    TC_LOG_INFO("server.loading", "Loading Temporary Summon Data...");
    sObjectMgr->LoadTempSummons();                               // must be after LoadCreatureTemplates() and LoadGameObjectTemplates()

    TC_LOG_INFO("server.loading", "Loading pet levelup spells...");
    sSpellMgr->LoadPetLevelupSpellMap();

    TC_LOG_INFO("server.loading", "Loading pet default spells additional to levelup spells...");
    sSpellMgr->LoadPetDefaultSpells();

    TC_LOG_INFO("server.loading", "Loading Creature Addon Data...");
    sObjectMgr->LoadCreatureAddons();                            // must be after LoadCreatureTemplates() and LoadCreatures()

    if (sWorld->getBoolConfig(CONFIG_ENABLE_GAMEOBJECTS))
    {
        TC_LOG_INFO("server.loading", "Loading Gameobject Data...");
        sObjectMgr->LoadGameobjects();
    }

    if (sWorld->getBoolConfig(CONFIG_ENABLE_QUEST))
    {
        TC_LOG_INFO("server.loading", "Loading Creature Linked Respawn...");
        sObjectMgr->LoadLinkedRespawn();                             // must be after LoadCreatures(), LoadGameObjects()

        TC_LOG_INFO("server.loading", "Loading Weather Data...");
        WeatherMgr::LoadWeatherData();

        TC_LOG_INFO("server.loading", "Loading Quests...");
        sObjectMgr->LoadQuests();                                    // must be loaded after DBCs, creature_template, item_template, gameobject tables

        TC_LOG_INFO("server.loading", "Checking Quest Disables");
        DisableMgr::CheckQuestDisables();                           // must be after loading quests

        TC_LOG_INFO("server.loading", "Loading Quest Objectives...");
        sObjectMgr->LoadQuestObjectives();

        TC_LOG_INFO("server.loading", "Loading Quest Objective Locales...");
        sObjectMgr->LoadQuestObjectiveLocales();

        TC_LOG_INFO("server.loading", "Loading Quest POI");
        sObjectMgr->LoadQuestPOI();

        TC_LOG_INFO("server.loading", "Loading Quests Relations...");
        sObjectMgr->LoadQuestRelations();                            // must be after quest load
    }

    if (!sWorld->getBoolConfig(CONFIG_ENABLE_ONLY_SPECIFIC_MAP))
    {
        TC_LOG_INFO("server.loading", "Loading Objects Pooling Data...");
        sPoolMgr->LoadFromDB();

        TC_LOG_INFO("server.loading", "Loading Game Event Data...");               // must be after loading pools fully
        sGameEventMgr->LoadFromDB();
    }

    TC_LOG_INFO("server.loading", "Loading UNIT_NPC_FLAG_SPELLCLICK Data..."); // must be after LoadQuests
    sObjectMgr->LoadNPCSpellClickSpells();

    TC_LOG_INFO("server.loading", "Loading Vehicle Template Accessories...");
    sObjectMgr->LoadVehicleTemplateAccessories();                // must be after LoadCreatureTemplates() and LoadNPCSpellClickSpells()

    TC_LOG_INFO("server.loading", "Loading Vehicle Accessories...");
    sObjectMgr->LoadVehicleAccessories();                       // must be after LoadCreatureTemplates() and LoadNPCSpellClickSpells()

    TC_LOG_INFO("server.loading", "Loading Dungeon boss data...");
    sObjectMgr->LoadInstanceEncounters();

    TC_LOG_INFO("server.loading", "Loading LFG rewards...");
    sLFGMgr->LoadRewards();

    TC_LOG_INFO("server.loading", "Loading LFG entrance positions...");
    sLFGMgr->LoadLFGDungeons();

    TC_LOG_INFO("server.loading", "Loading SpellArea Data...");                // must be after quest load
    sSpellMgr->LoadSpellAreas();

    TC_LOG_INFO("server.loading", "Loading Spell Classes Info...");
    sSpellMgr->LoadSpellClassInfo();

    TC_LOG_INFO("server.loading", "Loading Talent Place Holder spell...");
    sSpellMgr->LoadSpellPlaceHolder();

    TC_LOG_INFO("server.loading", "Loading AreaTrigger definitions...");
    sObjectMgr->LoadAreaTriggerTeleports();

    TC_LOG_INFO("server.loading", "Loading Access Requirements...");
    sObjectMgr->LoadAccessRequirements();                        // must be after item template load

    TC_LOG_INFO("server.loading", "Loading LFR Access Requirements...");
    sObjectMgr->LoadLFRAccessRequirements();

    TC_LOG_INFO("server.loading", "Loading Quest Area Triggers...");
    sObjectMgr->LoadQuestAreaTriggers();                         // must be after LoadQuests

    TC_LOG_INFO("server.loading", "Loading Tavern Area Triggers...");
    sObjectMgr->LoadTavernAreaTriggers();

    TC_LOG_INFO("server.loading", "Loading AreaTrigger script names...");
    sObjectMgr->LoadAreaTriggerScripts();

    TC_LOG_INFO("server.loading", "Loading Graveyard-zone links...");
    sObjectMgr->LoadGraveyardZones();

    TC_LOG_INFO("server.loading", "Loading spell pet auras...");
    sSpellMgr->LoadSpellPetAuras();

	TC_LOG_INFO("server.loading", "Loading spell regulator values...");
	sSpellRegulator->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Spell target coordinates...");
    sSpellMgr->LoadSpellTargetPositions();

    TC_LOG_INFO("server.loading", "Loading enchant custom attributes...");
    sSpellMgr->LoadEnchantCustomAttr();

    TC_LOG_INFO("server.loading", "Loading linked spells...");
    sSpellMgr->LoadSpellLinked();

    TC_LOG_INFO("server.loading", "Loading spells upgrade item stage...");
    sSpellMgr->LoadSpellUpgradeItemStage();

    TC_LOG_INFO("server.loading", "Loading spells invalid...");
    sObjectMgr->LoadSpellInvalid();

    TC_LOG_INFO("server.loading", "Loading spells stolen...");
    sObjectMgr->LoadSpellStolen();

    TC_LOG_INFO("server.loading", "Loading disabled rankings...");
    sObjectMgr->LoadDisabledEncounters();

    TC_LOG_INFO("server.loading", "Loading conversation templates...");
    sObjectMgr->LoadConversationTemplates();

#ifndef CROSS
    /// It must be done before anything related to players
    LoadCharacterInfoStore();
#endif

    TC_LOG_INFO("server.loading", "Loading Player Create Data...");
    sObjectMgr->LoadPlayerInfo();

    TC_LOG_INFO("server.loading", "Loading Exploration BaseXP Data...");
    sObjectMgr->LoadExplorationBaseXP();

    TC_LOG_INFO("server.loading", "Loading Pet Name Parts...");
    sObjectMgr->LoadPetNames();

#ifndef CROSS
    CharacterDatabaseCleaner::CleanDatabase();
#endif

    TC_LOG_INFO("server.loading", "Loading the max pet number...");
    sObjectMgr->LoadPetNumber();

    TC_LOG_INFO("server.loading", "Loading pet stats...");
    sObjectMgr->LoadPetStatInfo();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Loading Player Corpses...");
    sObjectMgr->LoadCorpses();
#endif

    TC_LOG_INFO("server.loading", "Loading Player level dependent mail rewards...");
    sObjectMgr->LoadMailLevelRewards();


    if (sWorld->getBoolConfig(CONFIG_ENABLE_LOOTS))
    {
        // Loot tables
        LoadLootTables();
    }

    TC_LOG_INFO("server.loading", "Loading Skill Discovery Table...");
    LoadSkillDiscoveryTable();

    TC_LOG_INFO("server.loading", "Loading Skill Extra Item Table...");
    LoadSkillExtraItemTable();

    TC_LOG_INFO("server.loading", "Loading Skill Fishing base level requirements...");
    sObjectMgr->LoadFishingBaseSkillLevel();

    TC_LOG_INFO("server.loading", "Loading Achievements...");
    sAchievementMgr->LoadAchievementReferenceList();
    TC_LOG_INFO("server.loading", "Loading Achievement Criteria Lists...");
    sAchievementMgr->LoadAchievementCriteriaList();
    TC_LOG_INFO("server.loading", "Loading Achievement Criteria Data...");
    sAchievementMgr->LoadAchievementCriteriaData();
    TC_LOG_INFO("server.loading", "Loading Achievement Rewards...");
    sAchievementMgr->LoadRewards();
    TC_LOG_INFO("server.loading", "Loading Achievement Reward Locales...");
    sAchievementMgr->LoadRewardLocales();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Loading Completed Achievements...");
    sAchievementMgr->LoadCompletedAchievements();

    // Delete expired auctions before loading
    TC_LOG_INFO("server.loading", "Deleting expired auctions...");
    sAuctionMgr->DeleteExpiredAuctionsAtStartup();

    ///- Load dynamic data tables from the database
    TC_LOG_INFO("server.loading", "Loading Item Auctions...");
    sAuctionMgr->LoadAuctionItems();
    TC_LOG_INFO("server.loading", "Loading Auctions...");
    sAuctionMgr->LoadAuctions();

    TC_LOG_INFO("server.loading", "Loading Guild rewards...");
    sGuildMgr->LoadGuildRewards();

    sGuildMgr->LoadGuilds();

    sGuildFinderMgr->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Groups...");
    sGroupMgr->LoadGroups();

    TC_LOG_INFO("server.loading", "Loading ReservedNames...");
    sObjectMgr->LoadReservedPlayersNames();
#endif

    TC_LOG_INFO("server.loading", "Loading GameObjects for quests...");
    sObjectMgr->LoadGameObjectForQuests();

    TC_LOG_INFO("server.loading", "Loading BattleMasters...");
    sBattlegroundMgr->LoadBattleMastersEntry();

    TC_LOG_INFO("server.loading", "Loading GameTeleports...");
    sObjectMgr->LoadGameTele();

    TC_LOG_INFO("server.loading", "Loading Gossip menu...");
    sObjectMgr->LoadGossipMenu();

    TC_LOG_INFO("server.loading", "Loading Gossip menu options...");
    sObjectMgr->LoadGossipMenuItems();

    TC_LOG_INFO("server.loading", "Loading Vendors...");
    sObjectMgr->LoadVendors();                                   // must be after load CreatureTemplate and ItemTemplate

    TC_LOG_INFO("server.loading", "Loading Trainers...");
    sObjectMgr->LoadTrainerSpell();                              // must be after load CreatureTemplate

    TC_LOG_INFO("server.loading", "Loading Waypoints...");
    sWaypointMgr->Load();

    TC_LOG_INFO("server.loading", "Loading SmartAI Waypoints...");
    sSmartWaypointMgr->LoadFromDB();

    TC_LOG_INFO("server.loading", "Loading Creature Formations...");
    sFormationMgr->LoadCreatureFormations();

    TC_LOG_INFO("server.loading", "Loading World States...");              // must be loaded before battleground, outdoor PvP and conditions
    LoadWorldStates();

    TC_LOG_INFO("server.loading", "Loading Phase definitions...");
    sObjectMgr->LoadPhaseDefinitions();

    TC_LOG_INFO("server.loading", "Loading Conditions...");
    sConditionMgr->LoadConditions();

    TC_LOG_INFO("server.loading", "Loading faction change achievement pairs...");
    sObjectMgr->LoadFactionChangeAchievements();

    TC_LOG_INFO("server.loading", "Loading faction change spell pairs...");
    sObjectMgr->LoadFactionChangeSpells();

    TC_LOG_INFO("server.loading", "Loading faction change item pairs...");
    sObjectMgr->LoadFactionChangeItems();

    TC_LOG_INFO("server.loading", "Loading faction change reputation pairs...");
    sObjectMgr->LoadFactionChangeReputations();

    TC_LOG_INFO("server.loading", "Loading faction change title pairs...");
    sObjectMgr->LoadFactionChangeTitles();

    TC_LOG_INFO("server.loading", "Loading faction change title pairs...");
    sObjectMgr->LoadFactionChangeQuests();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Loading GM tickets...");
    sTicketMgr->LoadTickets();

    TC_LOG_INFO("server.loading", "Loading GM surveys...");
    sTicketMgr->LoadSurveys();
#endif

    TC_LOG_INFO("server.loading", "Loading client addons...");
    AddonMgr::LoadFromDB();

#ifndef CROSS
    ///- Handle outdated emails (delete/return)
    TC_LOG_INFO("server.loading", "Returning old mails...");
    sObjectMgr->ReturnOrDeleteOldMails(false);
#endif

    TC_LOG_INFO("server.loading", "Loading Autobroadcasts...");
    LoadAutobroadcasts();

    ///- Load and initialize scripts
    sObjectMgr->LoadQuestStartScripts();                         // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sObjectMgr->LoadQuestEndScripts();                           // must be after load Creature/Gameobject(Template/Data) and QuestTemplate
    sObjectMgr->LoadSpellScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadGameObjectScripts();                         // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadEventScripts();                              // must be after load Creature/Gameobject(Template/Data)
    sObjectMgr->LoadWaypointScripts();

    TC_LOG_INFO("server.loading", "Loading Scripts text locales...");      // must be after Load*Scripts calls
    sObjectMgr->LoadDbScriptStrings();

    TC_LOG_INFO("server.loading", "Loading spell script names...");
    sObjectMgr->LoadSpellScriptNames();

    TC_LOG_INFO("server.loading", "Initializing Scripts...");
    sScriptMgr->Initialize();
    sScriptMgr->OnConfigLoad(false);                                // must be done after the ScriptMgr has been properly initialized

    TC_LOG_INFO("server.loading", "Validating spell scripts...");
    sObjectMgr->ValidateSpellScripts();

    TC_LOG_INFO("server.loading", "Loading SmartAI scripts...");
    sSmartScriptMgr->LoadSmartAIFromDB();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Loading Calendar data...");
    sCalendarMgr->LoadFromDB();
#endif

    TC_LOG_INFO("server.loading", "Loading Cinematic path ...");
    sCinematicSequenceMgr->Load();

    TC_LOG_INFO("server.loading", "Loading AreaTrigger templates...");
    sObjectMgr->LoadAreaTriggerTemplates();

    TC_LOG_INFO("server.loading", "Loading AreaTrigger move splines...");
    sObjectMgr->LoadAreaTriggerMoveSplines();

    TC_LOG_INFO("server.loading", "Loading AreaTrigger move templates...");
    sObjectMgr->LoadAreaTriggerMoveTemplates();

    TC_LOG_INFO("server.loading", "Loading FollowerQuests...");
    sObjectMgr->LoadFollowerQuests();

    TC_LOG_INFO("server.loading", "Loading Bonus quest...");
    sObjectMgr->LoadBonusQuests();

    TC_LOG_INFO("server.loading", "Loading QuestForItem...");
    sObjectMgr->LoadQuestForItem();

    TC_LOG_INFO("server.loading", "Loading Spell Auras Not Save...");
    sSpellMgr->LoadSpellAurasNotSave();

    ///- Initialize game time and timers
    TC_LOG_INFO("server.loading", "Initialize game time and timers");
    m_gameTime = time(NULL);
    m_startTime = m_gameTime;

#ifndef CROSS
    LoginDatabase.PExecute("INSERT INTO uptime (realmid, starttime, uptime, revision) VALUES(%u, %u, 0, '%s')",
                            g_RealmID, uint32(m_startTime), GitRevision::GetFullVersion()); ///< One-time query
#endif

    m_timers[WUPDATE_WEATHERS].SetInterval(1*IN_MILLISECONDS);
    m_timers[WUPDATE_AUCTIONS].SetInterval(MINUTE*IN_MILLISECONDS);
    m_timers[WUPDATE_UPTIME].SetInterval(m_int_configs[CONFIG_UPTIME_UPDATE]*MINUTE*IN_MILLISECONDS);
                                                            //Update "uptime" table based on configuration entry in minutes.
    m_timers[WUPDATE_CORPSES].SetInterval(20 * MINUTE * IN_MILLISECONDS);
                                                            //erase corpses every 20 minutes
    m_timers[WUPDATE_CLEANDB].SetInterval(m_int_configs[CONFIG_LOGDB_CLEARINTERVAL]*MINUTE*IN_MILLISECONDS);
                                                            // clean logs table every 14 days by default
    m_timers[WUPDATE_AUTOBROADCAST].SetInterval(getIntConfig(CONFIG_AUTOBROADCAST_INTERVAL));
    m_timers[WUPDATE_DELETECHARS].SetInterval(DAY*IN_MILLISECONDS); // check for chars to delete every day

    m_timers[WUPDATE_PINGDB].SetInterval(getIntConfig(CONFIG_DB_PING_INTERVAL)*MINUTE*IN_MILLISECONDS);    // Mysql ping time in minutes

    m_timers[WUPDATE_GUILDSAVE].SetInterval(getIntConfig(CONFIG_GUILD_SAVE_INTERVAL) * MINUTE * IN_MILLISECONDS);

    m_timers[WUPDATE_REALM_STATS].SetInterval(MINUTE * IN_MILLISECONDS);

#ifndef CROSS
    m_timers[WUPDATE_BLACKMARKET].SetInterval(MINUTE * IN_MILLISECONDS);
    m_timers[WUPDATE_TRANSFER].SetInterval(10 * IN_MILLISECONDS);
    m_timers[WUPDATE_TRANSFER_EXP].SetInterval(13 * MINUTE * IN_MILLISECONDS);
#endif

    //to set mailtimer to return mails every day between 4 and 5 am
    //mailtimer is increased when updating auctions
    //one second is 1000 -(tested on win system)
    //TODO: Get rid of magic numbers
    tm localTm;
    localtime_r(&m_gameTime, &localTm);
    mail_timer = ((((localTm.tm_hour + 20) % 24) * HOUR * IN_MILLISECONDS) / m_timers[WUPDATE_AUCTIONS].GetInterval());
                                                            //1440
    mail_timer_expires = ((DAY * IN_MILLISECONDS) / (m_timers[WUPDATE_AUCTIONS].GetInterval()));
    TC_LOG_INFO("server.loading", "Mail timer set to: " UI64FMTD ", mail return is called every " UI64FMTD " minutes", uint64(mail_timer), uint64(mail_timer_expires));

    ///- Initilize static helper structures
    AIRegistry::Initialize();

    ///- Initialize MapManager
    TC_LOG_INFO("server.loading", "Starting Map System");
    sMapMgr->Initialize();

    ///- Initialize Battlegrounds
    TC_LOG_INFO("server.loading", "Starting Battleground System");
    sBattlegroundMgr->CreateInitialBattlegrounds();

    TC_LOG_INFO("server.loading", "Starting Game Event system...");
    uint32 nextGameEvent = sGameEventMgr->StartSystem();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);    //depend on next event

#ifndef CROSS
    // Delete all characters which have been deleted X days before
    Player::DeleteOldCharacters();
#endif

    // Delete all custom channels which haven't been used for PreserveCustomChannelDuration days.
    Channel::CleanOldChannelsInDB();

    TC_LOG_INFO("server.loading", "Starting Arena Season...");
    sGameEventMgr->StartArenaSeason();

#ifndef CROSS
    sTicketMgr->Initialize();
#endif

    //sBattlegroundMgr->InitAutomaticArenaPointDistribution();

    ///- Initialize outdoor pvp
    TC_LOG_INFO("server.loading", "Starting Outdoor PvP System");
    sOutdoorPvPMgr->InitOutdoorPvP();

    ///- Initialize Battlefield
    TC_LOG_INFO("server.loading", "Starting Battlefield System");
    sBattlefieldMgr->InitBattlefield();

    TC_LOG_INFO("server.loading", "Loading Transports...");
    sTransportMgr->SpawnContinentTransports();

    ///- Initialize Warden
    TC_LOG_INFO("server.loading", "Loading Warden Checks...");
    sWardenCheckMgr->LoadWardenChecks();

    TC_LOG_INFO("server.loading", "Loading Warden Action Overrides...");
    sWardenCheckMgr->LoadWardenOverrides();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Deleting expired bans...");
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE unbandate <= UNIX_TIMESTAMP() AND unbandate<>bandate");      // One-time query
#endif

    TC_LOG_INFO("server.loading", "Calculate next daily quest reset time...");
    InitDailyQuestResetTime();

    TC_LOG_INFO("server.loading", "Calculate next weekly quest reset time...");
    InitWeeklyQuestResetTime();

    TC_LOG_INFO("server.loading", "Calculate next monthly quest reset time...");
    InitMonthlyQuestResetTime();

    TC_LOG_INFO("server.loading", "Calculate random battleground reset time...");
    InitRandomBGResetTime();

    TC_LOG_INFO("server.loading", "Calculate next currency reset time...");
    InitCurrencyResetTime();

    TC_LOG_INFO("server.loading", "Calculate next daily loot reset time...");
    InitDailyLootResetTime();

    TC_LOG_INFO("server.loading", "Calculate next weekly guild challenges reset time...");
    InitGuildChallengesResetTime();

    TC_LOG_INFO("server.loading", "Calculate next weekly boss looted reset time...");
    InitBossLootedResetTime();

    TC_LOG_INFO("misc", "Initializing Opcodes...");
    InitOpcodes();

#ifdef CROSS
    TC_LOG_INFO("server.loading", "Loading InterRealm config...");
    sInterRealmMgr->LoadConfig();
#endif

    IRopcodeTable.Initialize();

    TC_LOG_INFO("misc", "Loading hotfix info...");
    sObjectMgr->LoadHotfixData();
    sObjectMgr->LoadHotfixTableHashs();

#ifndef CROSS
    TC_LOG_INFO("misc", "Loading guild challenge rewards...");
    sObjectMgr->LoadGuildChallengeRewardInfo();

    TC_LOG_INFO("misc", "Loading black market templates...");
    sBlackMarketMgr->LoadTemplates();

    TC_LOG_INFO("misc", "Loading black market auctions...");
    sBlackMarketMgr->LoadAuctions();
#endif

    TC_LOG_INFO("misc", "Loading realm name...");

    m_realmName = "Warlords of Draenor servers";

#ifndef CROSS
    QueryResult realmResult = LoginDatabase.PQuery("SELECT name FROM realmlist WHERE id = %u", g_RealmID);
    if (realmResult)
        m_realmName = (*realmResult)[0].GetString();
#endif

    TC_LOG_INFO("misc", "Loading area skip update...");
    sObjectMgr->LoadSkipUpdateZone();

    TC_LOG_INFO("server.loading", "Loading BattlePet template...");
    sObjectMgr->LoadBattlePetTemplate();

    TC_LOG_INFO("server.loading", "Loading BattlePet npc team member...");
    sObjectMgr->LoadBattlePetNpcTeamMember();
    ///sObjectMgr->ComputeBattlePetSpawns();

    TC_LOG_INFO("server.loading", "Loading Wild BattlePet pools...");
    sWildBattlePetMgr->Load();

    TC_LOG_INFO("server.loading", "Loading character template data...");
    sObjectMgr->LoadCharacterTemplateData();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Loading realm completed challenges...");
    sObjectMgr->LoadRealmCompletedChallenges();
#endif

    TC_LOG_INFO("server.loading", "Loading challenge mode rewards...");
    sObjectMgr->LoadChallengeRewards();

#ifndef CROSS
    TC_LOG_INFO("server.loading", "Init Garrison shipment manager...");
    sGarrisonShipmentManager->Init();

    PlayerDump::LoadColumnsName();

    sBattlepayMgr->LoadFromDatabase();

    m_timers[WUPDATE_SCAN_ACC_LOG_IP].SetInterval(5 * IN_MILLISECONDS);
#endif

    InitServerAutoRestartTime();

    uint32 startupDuration = GetMSTimeDiffToNow(startupBegin);

    QueryResult l_Result = LoginDatabase.PQuery("SELECT max(id) FROM account_log_ip");
    if (l_Result)
        m_LastAccountLogId = l_Result->Fetch()[0].GetUInt64();

    TC_LOG_INFO("server.worldserver", "World initialized in %u minutes %u seconds", (startupDuration / 60000), ((startupDuration % 60000) / 1000));

    if (uint32 realmId = sConfigMgr->GetIntDefault("RealmID", 0)) // 0 reserved for auth
        sLog->SetRealmId(realmId);
}

void World::DetectDBCLang()
{
    uint8 m_lang_confid = sConfigMgr->GetIntDefault("DBC.Locale", 0);

    if (m_lang_confid >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("server.loading", "Incorrect DBC.Locale! Must be >= 0 and < %d (set to 0)", TOTAL_LOCALES);
        m_lang_confid = LOCALE_enUS;
    }

    m_defaultDbcLocale = LocaleConstant(m_lang_confid);

    TC_LOG_INFO("server.loading", "Using %s DBC Locale", localeNames[m_defaultDbcLocale]);
}

void World::RecordTimeDiff(const char *text, ...)
{
    if (m_updateTimeCount != 1)
        return;
    if (!text)
    {
        m_currentTime = getMSTime();
        return;
    }

    uint32 thisTime = getMSTime();
    uint32 diff = getMSTimeDiff(m_currentTime, thisTime);

    if (diff > m_int_configs[CONFIG_MIN_LOG_UPDATE])
    {
        va_list ap;
        char str[256];
        va_start(ap, text);
        vsnprintf(str, 256, text, ap);
        va_end(ap);
        TC_LOG_INFO("misc", "Difftime %s: %u.", str, diff);
    }

    m_currentTime = thisTime;
}

void World::LoadAutobroadcasts()
{
    uint32 oldMSTime = getMSTime();

    m_Autobroadcasts.clear();

    std::string l_Query = "SELECT Text, TextFR, TextES, TextRU FROM autobroadcast WHERE Expension IN(-1, 5) AND RealmID IN(-1, " + std::to_string(g_RealmID) + ")";

    QueryResult result = LoginDatabase.Query(l_Query.c_str());

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 autobroadcasts definitions. DB table `autobroadcast` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {

        Field* fields = result->Fetch();

        AutoBroadcastText l_AutobrodCastText;
        l_AutobrodCastText.Text   = fields[0].GetString();
        l_AutobrodCastText.TextFR = fields[1].GetString();
        l_AutobrodCastText.TextES = fields[2].GetString();
        l_AutobrodCastText.TextRU = fields[3].GetString();

        m_Autobroadcasts.push_back(l_AutobrodCastText);

        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u autobroadcasts definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

/// Update the World !
void World::Update(uint32 diff)
{
    m_updateTime = diff;

#ifdef CROSS
    if (m_update_online_timer <= diff)
    {
        UpdateInterRealmStat();
        m_update_online_timer = 60000;
    }
    else
        m_update_online_timer -= diff;
#endif

    m_serverDelaySum += m_updateTime;
    ++m_serverUpdateCount;

    if (m_int_configs[CONFIG_INTERVAL_LOG_UPDATE] && diff > m_int_configs[CONFIG_MIN_LOG_UPDATE])
    {
        if (m_updateTimeSum > m_int_configs[CONFIG_INTERVAL_LOG_UPDATE])
        {
#ifndef CROSS
            LoginDatabase.PExecute("UPDATE realmlist set online=%u, queue=%u, lastupdate=%u where id=%u", GetActiveSessionCount(), GetQueuedSessionCount(), std::time(nullptr), g_RealmID);
#endif
            m_updateTimeSum = m_updateTime;
            m_updateTimeCount = 1;
        }
        else
        {
            m_updateTimeSum += m_updateTime;
            ++m_updateTimeCount;
        }
    }

    if (m_serverDelayTimer > m_int_configs[CONFIG_INTERVAL_LOG_UPDATE])
    {
        uint32 delay = m_serverUpdateCount ? (m_serverDelaySum / m_serverUpdateCount) : 1;

        m_serverDelaySum = 0;
        m_serverUpdateCount = 0;

#ifndef CROSS
        LoginDatabase.PExecute("UPDATE realmlist set delay=%u, lastupdate=%u where id=%u", delay, std::time(nullptr), g_RealmID);
#endif
        m_serverDelayTimer -= m_int_configs[CONFIG_INTERVAL_LOG_UPDATE];
    }
    else
        m_serverDelayTimer += diff;

    ///- Update the different timers
    for (int i = 0; i < WUPDATE_COUNT; ++i)
    {
        if (m_timers[i].GetCurrent() >= 0)
            m_timers[i].Update(diff);
        else
            m_timers[i].SetCurrent(0);
    }

    ///- Update the game time and check for shutdown time
    _UpdateGameTime();

#ifndef CROSS
    /// Handle daily quests reset time
    if (m_gameTime > m_NextDailyQuestReset)
    {
        ResetDailyQuests();
        ResetDailyGarrisonDatas();
        m_NextDailyQuestReset += DAY;
    }

    /// Handle weekly quests reset time
    if (m_gameTime > m_NextWeeklyQuestReset)
    {
        ResetWeeklyQuests();
        sGuildMgr->ResetReputationCaps();
    }

    /// Handle monthly quests reset time
    if (m_gameTime > m_NextMonthlyQuestReset)
        ResetMonthlyQuests();

    if (m_gameTime > m_NextRandomBGReset)
        ResetRandomBG();

    if (m_gameTime > m_NextCurrencyReset)
        ResetCurrencyWeekCap();

    if (m_gameTime >= m_NextDailyLootReset)
        ResetDailyLoots();

    if (m_gameTime >= m_NextGuildChallengesReset)
        ResetGuildChallenges();

    if (m_gameTime >= m_NextBossLootedReset)
        ResetBossLooted();

    /// <ul><li> Handle auctions when the timer has passed
    if (m_timers[WUPDATE_AUCTIONS].Passed())
    {
        m_timers[WUPDATE_AUCTIONS].Reset();

        ///- Update mails (return old mails with item, or delete them)
        //(tested... works on win)
        if (++mail_timer > mail_timer_expires)
        {
            mail_timer = 0;
            sObjectMgr->ReturnOrDeleteOldMails(true);
        }

        ///- Handle expired auctions
        sAuctionMgr->Update();
    }

    if (m_timers[WUPDATE_REALM_STATS].Passed())
    {
        m_timers[WUPDATE_REALM_STATS].Reset();
        LoginDatabase.PExecute("UPDATE realmlist SET online = %u, queue = %u where id = %u", GetActiveSessionCount(), GetQueuedSessionCount(), g_RealmID);
    }

    _updateTransfers();
#endif /* CROSS */

    if (m_gameTime > m_NextServerRestart && !m_bool_configs[CONFIG_DISABLE_RESTART])
        AutoRestartServer();

    uint32 diffTime = getMSTime();

#ifdef CROSS
    sInterRealmMgr->Update(diff);
#endif

    /// <li> Handle session updates when the timer has passed
    RecordTimeDiff(NULL);
    UpdateSessions(diff);

    SetRecordDiff(RECORD_DIFF_SESSION, getMSTime() - diffTime);
    diffTime = getMSTime();

    RecordTimeDiff("UpdateSessions");

    /// <li> Handle weather updates when the timer has passed
    if (m_timers[WUPDATE_WEATHERS].Passed())
    {
        m_timers[WUPDATE_WEATHERS].Reset();
        WeatherMgr::Update(uint32(m_timers[WUPDATE_WEATHERS].GetInterval()));
    }

    /// <li> Update uptime table
    if (m_timers[WUPDATE_UPTIME].Passed())
    {
        uint32 tmpDiff = uint32(m_gameTime - m_startTime);
        uint32 maxOnlinePlayers = GetMaxPlayerCount();

        m_timers[WUPDATE_UPTIME].Reset();

#ifndef CROSS
        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_UPTIME_PLAYERS);

        stmt->setUInt32(0, tmpDiff);
        stmt->setUInt16(1, uint16(maxOnlinePlayers));
        stmt->setUInt32(2, g_RealmID);
        stmt->setUInt32(3, uint32(m_startTime));

        LoginDatabase.Execute(stmt);
#endif
    }

    /// <li> Clean logs table
#ifndef CROSS
    if (sWorld->getIntConfig(CONFIG_LOGDB_CLEARTIME) > 0) // if not enabled, ignore the timer
    {
        if (m_timers[WUPDATE_CLEANDB].Passed())
        {
            m_timers[WUPDATE_CLEANDB].Reset();

            PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_OLD_LOGS);

            stmt->setUInt32(0, sWorld->getIntConfig(CONFIG_LOGDB_CLEARTIME));
            stmt->setUInt32(1, uint32(time(0)));

            LoginDatabase.Execute(stmt);
        }
    }
#endif

    /// <li> Handle all other objects
    ///- Update objects when the timer has passed (maps, transport, creatures, ...)
    RecordTimeDiff(NULL);
    sMapMgr->Update(diff);

    SetRecordDiff(RECORD_DIFF_MAP, getMSTime() - diffTime);
    diffTime = getMSTime();
    RecordTimeDiff("UpdateMapMgr");

    if (sWorld->getBoolConfig(CONFIG_AUTOBROADCAST))
    {
        if (m_timers[WUPDATE_AUTOBROADCAST].Passed())
        {
            m_timers[WUPDATE_AUTOBROADCAST].Reset();
            SendAutoBroadcast();
        }
    }

    sBattlegroundMgr->Update(diff);
    SetRecordDiff(RECORD_DIFF_BATTLEGROUND, getMSTime() - diffTime);
    diffTime = getMSTime();
    RecordTimeDiff("UpdateBattlegroundMgr");

    sOutdoorPvPMgr->Update(diff);
    SetRecordDiff(RECORD_DIFF_OUTDOORPVP, getMSTime() - diffTime);
    diffTime = getMSTime();
    RecordTimeDiff("UpdateOutdoorPvPMgr");

    sBattlefieldMgr->Update(diff);
    SetRecordDiff(RECORD_DIFF_BATTLEFIELD, getMSTime() - diffTime);
    diffTime = getMSTime();
    RecordTimeDiff("BattlefieldMgr");

#ifndef CROSS
    ///- Delete all characters which have been deleted X days before
    if (m_timers[WUPDATE_DELETECHARS].Passed())
    {
        m_timers[WUPDATE_DELETECHARS].Reset();
        Player::DeleteOldCharacters();
    }
#endif

    sPetBattleSystem->Update(diff);

    sLFGMgr->Update(diff);
    SetRecordDiff(RECORD_DIFF_LFG, getMSTime() - diffTime);
    diffTime = getMSTime();
    RecordTimeDiff("UpdateLFGMgr");

#ifndef CROSS
    if (InterRealmSession* tunnel = GetInterRealmSession())
        tunnel->Update(diff);

#endif /* not CROSS */
    // execute callbacks from sql queries that were queued recently
    ProcessQueryCallbacks();

    SetRecordDiff(RECORD_DIFF_CALLBACK, getMSTime() - diffTime);
    RecordTimeDiff("ProcessQueryCallbacks");

    sLFGListMgr->Update(diff);

    ///- Erase corpses once every 20 minutes
    if (m_timers[WUPDATE_CORPSES].Passed())
    {
        m_timers[WUPDATE_CORPSES].Reset();
        sObjectAccessor->RemoveOldCorpses();
    }

    ///- Process Game events when necessary
    if (m_timers[WUPDATE_EVENTS].Passed())
    {
        m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
        uint32 nextGameEvent = sGameEventMgr->Update();
        m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
        m_timers[WUPDATE_EVENTS].Reset();
    }

    ///- Ping to keep MySQL connections alive
    if (m_timers[WUPDATE_PINGDB].Passed())
    {
        m_timers[WUPDATE_PINGDB].Reset();
        TC_LOG_DEBUG("misc", "Ping MySQL to keep connection alive");
        CharacterDatabase.KeepAlive();
        LoginDatabase.KeepAlive();
        WorldDatabase.KeepAlive();
        HotfixDatabase.KeepAlive();

        if (sWorld->getBoolConfig(CONFIG_WEB_DATABASE_ENABLE))
            WebDatabase.KeepAlive();
    }

#ifndef CROSS
    if (m_timers[WUPDATE_GUILDSAVE].Passed())
    {
        m_timers[WUPDATE_GUILDSAVE].Reset();
        sGuildMgr->SaveGuilds();
    }

    // Update Blackmarket
    if (m_timers[WUPDATE_BLACKMARKET].Passed())
    {
        m_timers[WUPDATE_BLACKMARKET].Reset();
        sBlackMarketMgr->Update();
    }

    if (m_timers[WUPDATE_SCAN_ACC_LOG_IP].Passed())
    {
        m_timers[WUPDATE_SCAN_ACC_LOG_IP].Reset();

        PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_SCAN_LAST_ACCOUNT_LOG);
        l_Stmt->setUInt32(0, m_LastAccountLogId);
        m_AccountLogIpScanCallback = LoginDatabase.AsyncQuery(l_Stmt);
    }

    if (m_AccountLogIpScanCallback.ready())
    {
        PreparedQueryResult l_Result;
        m_AccountLogIpScanCallback.get(l_Result);

        if (l_Result)
        {
            do
            {
                Field* l_Fields = l_Result->Fetch();
                m_LastAccountLogId = l_Fields[0].GetUInt32();
                uint32 l_AccountId = l_Fields[1].GetUInt32();

                if (WorldSession* l_Session = FindSession(l_AccountId))
                {
                    auto l_Itr = m_NewSessions.find(l_AccountId);

                    if (l_Itr == m_NewSessions.end())
                        l_Session->KickPlayer();
                    else
                        m_NewSessions.erase(l_AccountId);
                }
            }
            while (l_Result->NextRow());
        }

        m_AccountLogIpScanCallback.cancel();
    }

#endif

    // update the instance reset times
    sInstanceSaveMgr->Update();

    // And last, but not least handle the issued cli commands
    ProcessCliCommands();

    sTimeDiffMgr->Update(diff);

    sScriptMgr->OnWorldUpdate(diff);
}

void World::ForceGameEventUpdate()
{
    m_timers[WUPDATE_EVENTS].Reset();                   // to give time for Update() to be processed
    uint32 nextGameEvent = sGameEventMgr->Update();
    m_timers[WUPDATE_EVENTS].SetInterval(nextGameEvent);
    m_timers[WUPDATE_EVENTS].Reset();
}

/// Send a packet to all players (except self if mentioned)
void World::SendGlobalMessage(WorldPacket* packet, WorldSession* self, uint32 team)
{
#ifdef CROSS
    PlayerMap::const_iterator itr;
    for (itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second &&
            itr->second &&
            itr->second->IsInWorld() &&
            itr->second->GetSession() != self &&
            (team == 0 || itr->second->GetTeam() == team))
        {
            itr->second->GetSession()->SendPacket(packet);
        }
    }
#else
    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
        }
    }
#endif
}

/// Send a packet to all GMs (except self if mentioned)
void World::SendGlobalGMMessage(WorldPacket* packet, WorldSession* self, uint32 team)
{
#ifdef CROSS
    PlayerMap::iterator itr;
    for (itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second &&
            itr->second->IsInWorld() &&
            itr->second->GetSession() != self &&
            !AccountMgr::IsPlayerAccount(itr->second->GetSession()->GetSecurity()) &&
            (team == 0 || itr->second->GetTeam() == team))
        {
            itr->second->GetSession()->SendPacket(packet);
        }
    }
#else
    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second != self &&
            !AccountMgr::IsPlayerAccount(itr->second->GetSecurity()) &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
        }
}
#endif
}

namespace Trinity
{
    class WorldWorldTextBuilder
    {
        public:
            typedef std::vector<WorldPacket*> WorldPacketList;
            explicit WorldWorldTextBuilder(int32 textId, va_list* args = NULL) : i_textId(textId), i_args(args) {}
            void operator()(WorldPacketList& data_list, LocaleConstant loc_idx)
            {
                char const* text = sObjectMgr->GetTrinityString(i_textId, loc_idx);

                if (i_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap, *i_args);

                    char str[2048];
                    vsnprintf(str, 2048, text, ap);
                    va_end(ap);

                    do_helper(data_list, &str[0]);
                }
                else
                    do_helper(data_list, (char*)text);
            }
        private:
            char* lineFromMessage(char*& pos) { char* start = strtok(pos, "\n"); pos = NULL; return start; }
            void do_helper(WorldPacketList& data_list, char* text)
            {
                char* pos = text;

                while (char* line = lineFromMessage(pos))
                {
                    WorldPacket* data = new WorldPacket();
                    ChatHandler::FillMessageData(data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, line, NULL);
                    data_list.push_back(data);
                }
            }

            int32 i_textId;
            va_list* i_args;
    };
}                                                           // namespace Trinity

/// Send a System Message to all players (except self if mentioned)
void World::SendWorldText(int32 string_id, ...)
{
    va_list ap;
    va_start(ap, string_id);

    Trinity::WorldWorldTextBuilder wt_builder(string_id, &ap);
    Trinity::LocalizedPacketListDo<Trinity::WorldWorldTextBuilder> wt_do(wt_builder);

#ifdef CROSS
    for (PlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (!itr->second || !itr->second || !itr->second->IsInWorld())
            continue;

        wt_do(itr->second);
    }
#else
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld())
            continue;

        wt_do(itr->second->GetPlayer());
    }
#endif

    va_end(ap);
}

/// Send a System Message to all GMs (except self if mentioned)
void World::SendGMText(int32 string_id, ...)
{
    va_list ap;
    va_start(ap, string_id);

#ifdef CROSS
    Trinity::WorldWorldTextBuilder wt_builder(string_id, &ap);
    Trinity::LocalizedPacketListDo<Trinity::WorldWorldTextBuilder> wt_do(wt_builder);
    for (PlayerMap::iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (!itr->second || !itr->second || !itr->second->IsInWorld())
            continue;

        if (AccountMgr::IsPlayerAccount(itr->second->GetSession()->GetSecurity()))
            continue;

        wt_do(itr->second);
    }
#else
    Trinity::WorldWorldTextBuilder wt_builder(string_id, &ap);
    Trinity::LocalizedPacketListDo<Trinity::WorldWorldTextBuilder> wt_do(wt_builder);
    for (SessionMap::iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (!itr->second || !itr->second->GetPlayer() || !itr->second->GetPlayer()->IsInWorld())
            continue;

        if (AccountMgr::IsPlayerAccount(itr->second->GetSecurity()))
            continue;

        wt_do(itr->second->GetPlayer());
    }
#endif

    va_end(ap);
}

/// DEPRECATED, only for debug purpose. Send a System Message to all players (except self if mentioned)
void World::SendGlobalText(const char* text, WorldSession* self)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = strdup(text);
    char* pos = buf;

    while (char* line = ChatHandler::LineFromMessage(pos))
    {
        ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, line, NULL);
        SendGlobalMessage(&data, self);
    }

    free(buf);
}

/// Send a packet to all players (or players selected team) in the zone (except self if mentioned)
void World::SendZoneMessage(uint32 zone, WorldPacket* packet, WorldSession* self, uint32 team)
{
#ifdef CROSS
    PlayerMap::const_iterator itr;
    for (itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        if (itr->second &&
            itr->second &&
            itr->second->IsInWorld() &&
            itr->second->GetZoneId() == zone &&
            itr->second != self->GetPlayer() &&
            (team == 0 || itr->second->GetTeam() == team))
        {
            itr->second->GetSession()->SendPacket(packet);
        }
    }
#else
    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld() &&
            itr->second->GetPlayer()->GetZoneId() == zone &&
            itr->second != self &&
            (team == 0 || itr->second->GetPlayer()->GetTeam() == team))
        {
            itr->second->SendPacket(packet);
        }
    }
#endif
}

/// Send a System Message to all players in the zone (except self if mentioned)
void World::SendZoneText(uint32 zone, const char* text, WorldSession* self, uint32 team)
{
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendZoneMessage(zone, &data, self, team);
}

/// Kick (and save) all players
void World::KickAll()
{
#ifndef CROSS
    m_QueuedPlayer.clear();                                 // prevent send queue update packet and login queued sessions

    // session not removed at kick and will removed in next update tick
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        itr->second->KickPlayer();
#endif
}

/// Kick (and save) all players with security level less `sec`
void World::KickAllLess(AccountTypes sec)
{
#ifndef CROSS
    // session not removed at kick and will removed in next update tick
    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetSecurity() < sec)
            itr->second->KickPlayer();
#endif
}

#ifndef CROSS
/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
BanReturn World::BanAccount(BanMode mode, std::string nameOrIP, std::string duration, std::string reason, std::string author)
{
    uint32 duration_secs = TimeStringToSecs(duration);
    PreparedQueryResult resultAccounts = PreparedQueryResult(NULL); //used for kicking
    PreparedStatement* stmt = NULL;
    PreparedStatement* stmtt = NULL;

    ///- Update the database with ban information
    switch (mode)
    {
        case BAN_IP:
            // No SQL injection with prepared statements
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_IP);
            stmt->setString(0, nameOrIP);
            resultAccounts = LoginDatabase.Query(stmt);
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_IP_BANNED);
            stmt->setString(0, nameOrIP);
            stmt->setUInt32(1, duration_secs);
            stmt->setString(2, author);
            stmt->setString(3, reason);
            LoginDatabase.Execute(stmt);
            break;
        case BAN_ACCOUNT:
            // No SQL injection with prepared statements
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME);
            stmt->setString(0, nameOrIP);
            resultAccounts = LoginDatabase.Query(stmt);
            break;
        case BAN_CHARACTER:
            // No SQL injection with prepared statements
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_ACCOUNT_BY_NAME);
            stmt->setString(0, nameOrIP);
            resultAccounts = CharacterDatabase.Query(stmt);
            break;
        default:
            return BAN_SYNTAX_ERROR;
    }

    if (!resultAccounts)
    {
        if (mode == BAN_IP)
            return BAN_SUCCESS;                             // ip correctly banned but nobody affected (yet)
        else
            return BAN_NOTFOUND;                            // Nobody to ban
    }

    ///- Disconnect all affected players (for IP it can be several)
    SQLTransaction trans = LoginDatabase.BeginTransaction();
    do
    {
        Field* fieldsAccount = resultAccounts->Fetch();
        uint32 account = fieldsAccount[0].GetUInt32();

        if (mode != BAN_IP)
        {
            // Check account already banned

            if (duration != "-1")
            {
                // temp banned
                stmtt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ALWAYS_BANNED);
                stmtt->setUInt32(0, account);
                PreparedQueryResult resultCheck = LoginDatabase.Query(stmtt);

                if (resultCheck)
                {
                    Field* fieldsCheck = resultCheck->Fetch();
                    uint32 timeRemaining = fieldsCheck[0].GetUInt32();
                    if (timeRemaining > duration_secs)
                    {
                         return BAN_TOO_SMALL;
                    }
                }

                //Permanent ban
                stmtt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BANNED_PERMANENT);
                stmtt->setUInt32(0, account);
                PreparedQueryResult resultCheckBan = LoginDatabase.Query(stmtt);

                if (resultCheckBan)
                {
                    return BAN_ALREADY_PERMANENT;
                }
            }

            // make sure there is only one active ban
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_NOT_BANNED);
            stmt->setUInt32(0, account);
            trans->Append(stmt);
            // No SQL injection with prepared statements
            stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_ACCOUNT_BANNED);
            stmt->setUInt32(0, account);
            stmt->setUInt32(1, duration_secs);
            stmt->setString(2, author);
            stmt->setString(3, reason);
            trans->Append(stmt);
        }

        if (WorldSession* sess = FindSession(account))
            if (std::string(sess->GetPlayerName()) != author)
                sess->KickPlayer();
    }
    while
        (resultAccounts->NextRow());

    LoginDatabase.CommitTransaction(trans);

    return BAN_SUCCESS;
}

/// Remove a ban from an account or IP address
bool World::RemoveBanAccount(BanMode mode, std::string nameOrIP)
{
    PreparedStatement* stmt = NULL;
    if (mode == BAN_IP)
    {
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_DEL_IP_NOT_BANNED);
        stmt->setString(0, nameOrIP);
        LoginDatabase.Execute(stmt);
    }
    else
    {
        uint32 account = 0;
        if (mode == BAN_ACCOUNT)
            account = AccountMgr::GetId(nameOrIP);
        else if (mode == BAN_CHARACTER)
            account = sObjectMgr->GetPlayerAccountIdByPlayerName(nameOrIP);

        if (!account)
            return false;

        //NO SQL injection as account is uint32
        stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_ACCOUNT_NOT_BANNED);
        stmt->setUInt32(0, account);
        LoginDatabase.Execute(stmt);
    }
    return true;
}

/// Ban an account or ban an IP address, duration will be parsed using TimeStringToSecs if it is positive, otherwise permban
BanReturn World::BanCharacter(std::string name, std::string duration, std::string reason, std::string author)
{
    Player* pBanned = sObjectAccessor->FindPlayerByName(name.c_str());
    uint32 guid = 0;

    uint32 duration_secs = TimeStringToSecs(duration);

    /// Pick a player to ban if not online
    if (!pBanned)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_BY_NAME);
        stmt->setString(0, name);
        PreparedQueryResult resultCharacter = CharacterDatabase.Query(stmt);

        if (!resultCharacter)
            return BAN_NOTFOUND;                                    // Nobody to ban

        guid = (*resultCharacter)[0].GetUInt32();
    }
    else
        guid = pBanned->GetGUIDLow();

    // Check if player is already banned
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_BANNED);
    stmt->setUInt32(0, guid);
    PreparedQueryResult bannedResult = CharacterDatabase.Query(stmt);
    if (bannedResult)
    {
        // If new duration > oldDuration, overwrite the ban, else, return
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_BANNED_DURATION);
        stmt->setUInt32(0, guid);
        PreparedQueryResult timeResult = CharacterDatabase.Query(stmt);
        if (timeResult)
        {
            Field* fields = timeResult->Fetch();
            uint32 oldTime = fields[0].GetUInt32();

            if ((time(NULL) + duration_secs) < oldTime)
                return BAN_SYNTAX_ERROR;
        }
    }

    // make sure there is only one active ban
    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_BAN);
    stmt->setUInt32(0, guid);
    CharacterDatabase.Execute(stmt);

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_BAN);
    stmt->setUInt32(0, guid);
    stmt->setUInt32(1, duration_secs);
    stmt->setString(2, author);
    stmt->setString(3, reason);
    CharacterDatabase.Execute(stmt);

    if (pBanned)
        pBanned->GetSession()->KickPlayer();

    return BAN_SUCCESS;
}

/// Remove a ban from a character
bool World::RemoveBanCharacter(std::string name)
{
    Player* pBanned = sObjectAccessor->FindPlayerByName(name.c_str());
    uint32 guid = 0;

    /// Pick a player to ban if not online
    if (!pBanned)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_GUID_BY_NAME);
        stmt->setString(0, name);
        PreparedQueryResult resultCharacter = CharacterDatabase.Query(stmt);

        if (!resultCharacter)
            return false;

        guid = (*resultCharacter)[0].GetUInt32();
    }
    else
        guid = pBanned->GetGUIDLow();

    if (!guid)
        return false;

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_BAN);
    stmt->setUInt32(0, guid);
    CharacterDatabase.Execute(stmt);
    return true;
}
#endif

/// Update the game time
void World::_UpdateGameTime()
{
    ///- update the time
    time_t thisTime = time(NULL);
    uint32 elapsed = uint32(thisTime - m_gameTime);
    m_gameTime = thisTime;

    ///- if there is a shutdown timer
    if (!IsStopped() && m_ShutdownTimer > 0 && elapsed > 0)
    {
        ///- ... and it is overdue, stop the world (set m_stopEvent)
        if (m_ShutdownTimer <= elapsed)
        {
#ifndef CROSS
            if (!(m_ShutdownMask & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount() == 0)
#else
            if (!(m_ShutdownMask & SHUTDOWN_MASK_IDLE))
#endif
                m_stopEvent = true;                         // exist code already set
            else
                m_ShutdownTimer = 1;                        // minimum timer value to wait idle state
        }
        ///- ... else decrease it and if necessary display a shutdown countdown to the users
        else
        {
            m_ShutdownTimer -= elapsed;

            ShutdownMsg();
        }
    }
}

/// Shutdown the server
void World::ShutdownServ(uint32 time, uint32 options, uint8 exitcode, const std::string& reason)
{
    // ignore if server shutdown at next tick
    if (IsStopped())
        return;

    m_ShutdownMask = options;
    m_ExitCode = exitcode;

    ///- If the shutdown time is 0, set m_stopEvent (except if shutdown is 'idle' with remaining sessions)
    if (time == 0)
    {
#ifndef CROSS
        if (!(options & SHUTDOWN_MASK_IDLE) || GetActiveAndQueuedSessionCount() == 0)
#else /* CROSS */
        if (!(options & SHUTDOWN_MASK_IDLE))
#endif
            m_stopEvent = true;                             // exist code already set
        else
            m_ShutdownTimer = 1;                            //So that the session count is re-evaluated at next world tick
    }
    ///- Else set the shutdown timer and warn users
    else
    {
        m_ShutdownTimer = time;
        ShutdownMsg(true, nullptr, reason);
    }

    sScriptMgr->OnShutdownInitiate(ShutdownExitCode(exitcode), ShutdownMask(options));
}

/// Display a shutdown message to the user(s)
void World::ShutdownMsg(bool show, Player* player, const std::string& reason)
{
    // not show messages for idle shutdown mode
    if (m_ShutdownMask & SHUTDOWN_MASK_IDLE)
        return;

    ///- Display a message every 12 hours, hours, 5 minutes, minute, 5 seconds and finally seconds
    if (show ||
        (m_ShutdownTimer < 5* MINUTE && (m_ShutdownTimer % 15) == 0) || // < 5 min; every 15 sec
        (m_ShutdownTimer < 15 * MINUTE && (m_ShutdownTimer % MINUTE) == 0) || // < 15 min ; every 1 min
        (m_ShutdownTimer < 30 * MINUTE && (m_ShutdownTimer % (5 * MINUTE)) == 0) || // < 30 min ; every 5 min
        (m_ShutdownTimer < 12 * HOUR && (m_ShutdownTimer % HOUR) == 0) || // < 12 h ; every 1 h
        (m_ShutdownTimer > 12 * HOUR && (m_ShutdownTimer % (12 * HOUR)) == 0)) // > 12 h ; every 12 h
    {
        std::string str = secsToTimeString(m_ShutdownTimer);
        if (!reason.empty())
            str += " - " + reason;

        ServerMessageType msgid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? SERVER_MSG_RESTART_TIME : SERVER_MSG_SHUTDOWN_TIME;

        SendServerMessage(msgid, str.c_str(), player);
        TC_LOG_DEBUG("misc", "Server is %s in %s", (m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"), str.c_str());
    }
}

/// Cancel a planned server shutdown
void World::ShutdownCancel()
{
    // nothing cancel or too later
    if (!m_ShutdownTimer || m_stopEvent)
        return;

    ServerMessageType msgid = (m_ShutdownMask & SHUTDOWN_MASK_RESTART) ? SERVER_MSG_RESTART_CANCELLED : SERVER_MSG_SHUTDOWN_CANCELLED;

    m_ShutdownMask = 0;
    m_ShutdownTimer = 0;
    m_ExitCode = SHUTDOWN_EXIT_CODE;                       // to default value
    SendServerMessage(msgid);

    TC_LOG_DEBUG("misc", "Server %s cancelled.", (m_ShutdownMask & SHUTDOWN_MASK_RESTART ? "restart" : "shuttingdown"));

    sScriptMgr->OnShutdownCancel();
}

/// Send a server message to the user(s)
void World::SendServerMessage(ServerMessageType type, const char *text, Player* player)
{
    std::string l_Message = "";

    if (type <= SERVER_MSG_STRING)
        l_Message = text;

    WorldPacket l_Data(SMSG_CHAT_SERVER_MESSAGE, 50);              // guess size
    l_Data << uint32(type);
    l_Data.WriteBits(l_Message.size(), 11);
    l_Data.FlushBits();

    l_Data.WriteString(l_Message);

    if (player)
        player->GetSession()->SendPacket(&l_Data);
    else
        SendGlobalMessage(&l_Data);
}

void World::UpdateSessions(uint32 diff)
{
#ifdef CROSS
    playersLock.lock();

    for (PlayerMap::iterator itr = m_players.begin(); itr != m_players.end();)
    {
        Player* player = itr->second;

        //CheckPlayerMaps(player);

        if (player->IsNeedRemove())
        {
            Map* map = player->FindMap();
            if (map && map->IsUpdating())
            {
                ++itr;
                continue;
            }

            player->RemovePlayer();
            delete player;
            itr = m_players.erase(itr);
        }
        else
        {
            WorldSession* pSession = itr->second->GetSession();
            WorldSessionFilter updater(pSession);

            pSession->Update(diff, updater);

            ++itr;
        }
    }

    playersLock.release();
#else
    ///- Add new sessions
    WorldSession* sess = NULL;
    while (addSessQueue.next(sess))
    {
        AddSession_(sess);
        AddNewSession(sess->GetAccountId());
    }

    ///- Then send an update signal to remaining ones
    for (SessionMap::iterator itr = m_sessions.begin(), next; itr != m_sessions.end(); itr = next)
    {
        next = itr;
        ++next;

        ///- and remove not active sessions from the list
        WorldSession* pSession = itr->second;
        WorldSessionFilter updater(pSession);

        if (!pSession->Update(diff, updater))    // As interval = 0
        {
            if (!RemoveQueuedPlayer(itr->second) && itr->second && getIntConfig(CONFIG_INTERVAL_DISCONNECT_TOLERANCE))
                m_disconnects[itr->second->GetAccountId()] = time(NULL);
            RemoveQueuedPlayer(pSession);
            m_sessions.erase(itr);
            delete pSession;

        }
    }
#endif
}

// This handles the issued and queued CLI commands
void World::ProcessCliCommands()
{
    CliCommandHolder::Print* zprint = NULL;
    void* callbackArg = NULL;
    CliCommandHolder* command = NULL;
    while (cliCmdQueue.next(command))
    {
        TC_LOG_INFO("misc", "CLI command under processing...");
        zprint = command->m_print;
        callbackArg = command->m_callbackArg;
        CliHandler handler(callbackArg, zprint);
        handler.ParseCommands(command->m_command);
        if (command->m_commandFinished)
            command->m_commandFinished(callbackArg, !handler.HasSentErrorMessage());
        delete command;
    }
}

void World::SendAutoBroadcast()
{
    if (m_Autobroadcasts.empty())
        return;

#ifndef CROSS
    AutoBroadcastText l_AutobroadcastText;
    l_AutobroadcastText = Trinity::Containers::SelectRandomContainerElement(m_Autobroadcasts);

    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
    {
        if (itr->second &&
            itr->second->GetPlayer() &&
            itr->second->GetPlayer()->IsInWorld())
        {
            std::string l_AutoBrodcast = "";

            switch (itr->second->GetSessionDbLocaleIndex())
            {
                case LocaleConstant::LOCALE_frFR:
                    l_AutoBrodcast = l_AutobroadcastText.TextFR;
                    break;
                case LocaleConstant::LOCALE_esMX:
                case LocaleConstant::LOCALE_esES:
                    l_AutoBrodcast = l_AutobroadcastText.TextES;
                    break;
                case LocaleConstant::LOCALE_ruRU:
                    l_AutoBrodcast = l_AutobroadcastText.TextRU;
                    break;
                default:
                    l_AutoBrodcast = l_AutobroadcastText.Text;
                    break;
            }

            if (l_AutoBrodcast.empty())
                continue;

            std::string l_AnnounceFormat = "|cffffff00[|c00077766Autobroadcast|cffffff00]: |cFFF222FF" + l_AutoBrodcast + "|r";

            WorldPacket l_Data;
            ChatHandler::FillMessageData(&l_Data, NULL, CHAT_MSG_SYSTEM, LANG_UNIVERSAL, NULL, 0, l_AnnounceFormat.c_str(), NULL);
            itr->second->SendPacket(&l_Data);
        }
    }
#endif
}

void World::LoadDBMotd()
{
    QueryResult l_Result = LoginDatabase.PQuery("SELECT Text, TextFR, TextES, TextRU FROM motd WHERE RealmID = '%d'", g_RealmID);
    if (l_Result)
    {
        Field* l_Fields = l_Result->Fetch();

        MotdText l_Motd;
        l_Motd.Text = l_Fields[0].GetString();
        l_Motd.TextFR = l_Fields[1].GetString();
        l_Motd.TextES = l_Fields[2].GetString();
        l_Motd.TextRU = l_Fields[3].GetString();

        SetDBMotd(l_Motd);
    }
}

void World::SetDBMotd(MotdText p_MotdText)
{
    m_Motd = p_MotdText;
}

void World::LoadDBAllowedSecurityLevel()
{
    QueryResult result = LoginDatabase.PQuery("SELECT allowedSecurityLevel from realmlist WHERE id = '%d'", g_RealmID);
    if (result)
        SetPlayerSecurityLimit(AccountTypes(result->Fetch()->GetUInt8()));
}

void World::SetPlayerSecurityLimit(AccountTypes _sec)
{
    AccountTypes sec = _sec < SEC_CONSOLE ? _sec : SEC_PLAYER;
    bool update = sec > m_allowedSecurityLevel;
    m_allowedSecurityLevel = sec;
    if (update)
        KickAllLess(m_allowedSecurityLevel);
}

void World::UpdateRealmCharCount(uint32 accountId)
{
#ifndef CROSS
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_COUNT);
    stmt->setUInt32(0, accountId);
    PreparedQueryResultFuture result = CharacterDatabase.AsyncQuery(stmt);
    m_realmCharCallbacks.push_back(std::move(result));
#endif
}

void World::_UpdateRealmCharCount(PreparedQueryResult resultCharCount)
{
#ifndef CROSS
    if (resultCharCount)
    {
        Field* fields = resultCharCount->Fetch();
        uint32 accountId = fields[0].GetUInt32();
        uint8 charCount = uint8(fields[1].GetUInt64());

        PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_REALM_CHARACTERS);
        stmt->setUInt8(0, charCount);
        stmt->setUInt32(1, accountId);
        stmt->setUInt32(2, g_RealmID);
        LoginDatabase.Execute(stmt);
    }
#endif
}

void World::InitWeeklyQuestResetTime()
{
    time_t wstime = uint64(sWorld->getWorldState(WS_WEEKLY_QUEST_RESET_TIME));
    time_t curtime = time(NULL);
    m_NextWeeklyQuestReset = wstime < curtime ? curtime : time_t(wstime);
}

void World::InitDailyQuestResetTime()
{
    time_t mostRecentQuestTime;

    QueryResult result = CharacterDatabase.Query("SELECT MAX(time) FROM character_queststatus_daily");
    if (result)
    {
        Field* fields = result->Fetch();
        mostRecentQuestTime = time_t(fields[0].GetUInt32());
    }
    else
        mostRecentQuestTime = 0;

    // client built-in time for reset is 6:00 AM
    // FIX ME: client not show day start time
    time_t curTime = time(NULL);
    tm localTm;
    localtime_r(&curTime, &localTm);
    localTm.tm_hour = 6;
    localTm.tm_min  = 0;
    localTm.tm_sec  = 0;

    // current day reset time
    time_t curDayResetTime = mktime(&localTm);

    // last reset time before current moment
    time_t resetTime = (curTime < curDayResetTime) ? curDayResetTime - DAY : curDayResetTime;

    // need reset (if we have quest time before last reset time (not processed by some reason)
    if (mostRecentQuestTime && mostRecentQuestTime <= resetTime)
        m_NextDailyQuestReset = mostRecentQuestTime;
    else // plan next reset time
        m_NextDailyQuestReset = (curTime >= curDayResetTime) ? curDayResetTime + DAY : curDayResetTime;
}

void World::InitMonthlyQuestResetTime()
{
    time_t wstime = uint64(sWorld->getWorldState(WS_MONTHLY_QUEST_RESET_TIME));
    time_t curtime = time(NULL);
    m_NextMonthlyQuestReset = wstime < curtime ? curtime : time_t(wstime);
}

void World::InitRandomBGResetTime()
{
    time_t bgtime = uint64(sWorld->getWorldState(WS_BG_DAILY_RESET_TIME));
    if (!bgtime)
        m_NextRandomBGReset = time_t(time(NULL));         // game time not yet init

    // generate time by config
    time_t curTime = time(NULL);
    tm localTm;
    localtime_r(&curTime, &localTm);
    localTm.tm_hour = getIntConfig(CONFIG_RANDOM_BG_RESET_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // current day reset time
    time_t nextDayResetTime = mktime(&localTm);

    // next reset time before current moment
    if (curTime >= nextDayResetTime)
        nextDayResetTime += DAY;

    // normalize reset time
    m_NextRandomBGReset = bgtime < curTime ? nextDayResetTime - DAY : nextDayResetTime;

    if (!bgtime)
        sWorld->setWorldState(WS_BG_DAILY_RESET_TIME, uint64(m_NextRandomBGReset));
}

void World::InitCurrencyResetTime()
{
    uint32 nextResetDay = sWorld->getWorldState(MS::Battlegrounds::WsCurrency::ResetTime);
    if (!nextResetDay)
    {
        uint32 baseDay = 16022; // mercredi 13 novembre 2013
        uint32 currentDay = (time(NULL) + 3600) / 86400;
        nextResetDay = baseDay;

        while (nextResetDay < currentDay)
            nextResetDay += 7;

        sWorld->setWorldState(MS::Battlegrounds::WsCurrency::ResetTime, nextResetDay);
    }

    m_NextCurrencyReset = nextResetDay * 86400 + 5 * 3600;
}

void World::InitDailyLootResetTime()
{
    uint32 l_NextResetDay = sWorld->getWorldState(WS_DAILY_LOOT_RESET_TIME);
    if (!l_NextResetDay)
    {
        uint32 l_CurrentDay = (time(NULL) + 3600) / 86400;
        l_NextResetDay = l_CurrentDay + 1;
        sWorld->setWorldState(WS_DAILY_LOOT_RESET_TIME, l_NextResetDay);
    }

    m_NextDailyLootReset = l_NextResetDay * 86400 + 5 * 3600;
}

void World::InitGuildChallengesResetTime()
{
    uint32 l_NextResetDay = sWorld->getWorldState(WS_WEEKLY_GUILD_CHALLENGES_RESET_TIME);
    if (!l_NextResetDay)
    {
        uint32 l_BaseDay = 16022; ///< Wednesday, November 13rd
        uint32 l_CurrentDay = (time(NULL) + 3600) / 86400;
        l_NextResetDay = l_BaseDay;

        while (l_NextResetDay < l_CurrentDay)
            l_NextResetDay += 7;

        l_NextResetDay = l_NextResetDay;
        sWorld->setWorldState(WS_WEEKLY_GUILD_CHALLENGES_RESET_TIME, l_NextResetDay);
    }

    m_NextGuildChallengesReset = l_NextResetDay * 86400 + 5 * 3600;
}

void World::InitBossLootedResetTime()
{
    uint32 l_NextResetDay = sWorld->getWorldState(WS_WEEKLY_BOSS_LOOTED_RESET_TIME);
    if (!l_NextResetDay)
    {
        uint32 l_BaseDay = 16022; ///< Wednesday, November 13rd
        uint32 l_CurrentDay = (time(NULL) + 3600) / 86400;
        l_NextResetDay = l_BaseDay;

        while (l_NextResetDay < l_CurrentDay)
            l_NextResetDay += 7;

        l_NextResetDay = l_NextResetDay;
        sWorld->setWorldState(WS_WEEKLY_BOSS_LOOTED_RESET_TIME, l_NextResetDay);
    }

    m_NextBossLootedReset = l_NextResetDay * 86400 + 5 * 3600;
}

#ifndef CROSS
void World::ResetDailyQuests()
{
    TC_LOG_INFO("misc", "Daily quests reset for all characters.");

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_DAILY);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->ResetDailyQuestStatus();

    // change available dailies
    sPoolMgr->ChangeDailyQuests();

    sAnticheatMgr->ResetDailyReportStates();
}

void World::ResetDailyGarrisonDatas()
{
    for (SessionMap::const_iterator l_Itr = m_sessions.begin(); l_Itr != m_sessions.end(); ++l_Itr)
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_GARRISON_DAILY_TAVERN_DATA);
        CharacterDatabase.Execute(stmt);

        Player* l_Player = l_Itr->second->GetPlayer();

        if (l_Player)
        {
            if (MS::Garrison::Manager* l_GarrisonMgr = l_Player->GetGarrison())
                l_GarrisonMgr->CleanGarrisonDailyTavernData();

            l_Player->ResetDailyGarrisonDatas();
        }
    }
}

void World::ResetWeeklyGarrisonDatas()
{
    for (SessionMap::const_iterator l_Itr = m_sessions.begin(); l_Itr != m_sessions.end(); ++l_Itr)
    {
        Player* l_Player = l_Itr->second->GetPlayer();

        if (l_Player != nullptr)
        {
            l_Player->ResetWeeklyGarrisonDatas();
            l_Player->SaveToDB();
        }
    }
}

void World::ResetCurrencyWeekCap()
{
    CharacterDatabase.Execute("UPDATE `character_currency` SET `week_count` = 0, `needResetCap` = 1");
    CharacterDatabase.Execute("UPDATE `character_arena_data` SET `prevWeekWins0` = `weekWins0`, `prevWeekWins1` = `weekWins1`, `prevWeekWins2` = `weekWins2`");
    CharacterDatabase.Execute("UPDATE `character_arena_data` SET `bestRatingOfWeek0` = 0, `weekWins0` = 0, `bestRatingOfWeek1` = 0, `weekWins1` = 0, `bestRatingOfWeek2` = 0, `weekWins2` = 0");
    CharacterDatabase.Execute("UPDATE `character_arena_data` SET `prevWeekGames0` = `weekGames0`, `prevWeekGames1` = `weekGames1`, `prevWeekGames2` = `weekGames2`, `prevWeekGames3` = `weekGames3`, `prevWeekGames4` = `weekGames4`, `prevWeekGames5` = `weekGames5`, `weekGames0` = 0, `weekGames1` = 0, `weekGames2` = 0, `weekGames3` = 0, `weekGames4` = 0, `weekGames5` = 0");

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->ResetCurrencyWeekCap();

    sWorld->setWorldState(MS::Battlegrounds::WsCurrency::ResetTime, getWorldState(MS::Battlegrounds::WsCurrency::ResetTime) + 7);

    InterRealmSession* l_Tunnel = sWorld->GetInterRealmSession();
    if (l_Tunnel && l_Tunnel->IsTunnelOpened())
    {
        WorldPacket l_Data(IR_CMSG_CURRENCY_CAP_RESET);
        l_Tunnel->SendPacket(&l_Data);
    }

    m_NextCurrencyReset = time_t(m_NextCurrencyReset + DAY * 7);
}

void World::ResetDailyLoots()
{
    /// Must clean this table every day
    CharacterDatabase.Execute("DELETE FROM `character_daily_loot_cooldown`");

    for (SessionMap::const_iterator l_Iter = m_sessions.begin(); l_Iter != m_sessions.end(); ++l_Iter)
    {
        if (l_Iter->second->GetPlayer())
            l_Iter->second->GetPlayer()->ResetDailyLoots();
    }

    m_NextDailyLootReset = time_t(m_NextDailyLootReset + DAY);
    sWorld->setWorldState(WS_DAILY_LOOT_RESET_TIME, getWorldState(WS_DAILY_LOOT_RESET_TIME) + 1);
}

void World::ResetGuildChallenges()
{
    CharacterDatabase.Execute("UPDATE `guild_challenges` SET ChallengeCount = 0");
    sWorld->setWorldState(WS_WEEKLY_GUILD_CHALLENGES_RESET_TIME, getWorldState(WS_WEEKLY_GUILD_CHALLENGES_RESET_TIME) + 7);
    m_NextGuildChallengesReset = time_t(m_NextGuildChallengesReset + DAY * 7);
}

void World::ResetBossLooted()
{
    CharacterDatabase.Execute("DELETE FROM `characters_boss_looted`");
    sWorld->setWorldState(WS_WEEKLY_BOSS_LOOTED_RESET_TIME, getWorldState(WS_WEEKLY_BOSS_LOOTED_RESET_TIME) + 7);
    m_NextBossLootedReset = time_t(m_NextBossLootedReset + DAY * 7);

    for (SessionMap::const_iterator l_Iter = m_sessions.begin(); l_Iter != m_sessions.end(); ++l_Iter)
    {
        if (l_Iter->second->GetPlayer())
            l_Iter->second->GetPlayer()->ResetBossLooted();
    }
}

void World::ResetWeeklyQuests()
{
    TC_LOG_INFO("misc", "Weekly quests reset for all characters.");
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_WEEKLY);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->ResetWeeklyQuestStatus();

    m_NextWeeklyQuestReset = time_t(m_NextWeeklyQuestReset + WEEK);
    sWorld->setWorldState(WS_WEEKLY_QUEST_RESET_TIME, uint64(m_NextWeeklyQuestReset));

    // change available weeklies
    sPoolMgr->ChangeWeeklyQuests();
}

void World::ResetMonthlyQuests()
{
    TC_LOG_INFO("misc", "Monthly quests reset for all characters.");

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_MONTHLY);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->ResetMonthlyQuestStatus();

    time_t mostRecentQuestTime = 0;

    // generate time
    time_t curTime = time(NULL);
    tm localTm;
    localtime_r(&curTime, &localTm);

    int month   = localTm.tm_mon;
    int year    = localTm.tm_year;

    ++month;

    // month 11 is december, next is january (0)
    if (month > 11)
    {
        month = 0;
        year += 1;
    }

    // reset time for next month
    localTm.tm_year     = year;
    localTm.tm_mon      = month;
    localTm.tm_mday     = 1;        // don't know if we really need config option for day / hour
    localTm.tm_hour     = 0;
    localTm.tm_min      = 0;
    localTm.tm_sec      = 0;

    time_t nextMonthResetTime = mktime(&localTm);

    // last reset time before current moment
    time_t resetTime = (curTime < nextMonthResetTime) ? nextMonthResetTime - MONTH : nextMonthResetTime;

    // need reset (if we have quest time before last reset time (not processed by some reason)
    if (mostRecentQuestTime && mostRecentQuestTime <= resetTime)
        m_NextMonthlyQuestReset = mostRecentQuestTime;
    else // plan next reset time
        m_NextMonthlyQuestReset = (curTime >= nextMonthResetTime) ? nextMonthResetTime + MONTH : nextMonthResetTime;

    sWorld->setWorldState(WS_MONTHLY_QUEST_RESET_TIME, uint64(m_NextMonthlyQuestReset));
}

void World::ResetEventSeasonalQuests(uint16 event_id)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_QUEST_STATUS_SEASONAL);
    stmt->setUInt16(0,event_id);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->ResetSeasonalQuestStatus(event_id);
}

void World::ResetRandomBG()
{
    TC_LOG_INFO("misc", "Random BG status reset for all characters.");

    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_BATTLEGROUND_RANDOM);
    CharacterDatabase.Execute(stmt);

    for (SessionMap::const_iterator itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second->GetPlayer())
            itr->second->GetPlayer()->SetRandomWinner(false);

    m_NextRandomBGReset = time_t(m_NextRandomBGReset + DAY);
    sWorld->setWorldState(WS_BG_DAILY_RESET_TIME, uint64(m_NextRandomBGReset));
}

void World::UpdateMaxSessionCounters()
{
    m_maxActiveSessionCount = std::max(m_maxActiveSessionCount, uint32(m_sessions.size()-m_QueuedPlayer.size()));
    m_maxQueuedSessionCount = std::max(m_maxQueuedSessionCount, uint32(m_QueuedPlayer.size()));
}

#endif /* not CROSS */
void World::LoadDBVersion()
{
    QueryResult result = WorldDatabase.Query("SELECT db_version, cache_id FROM version LIMIT 1");
    if (result)
    {
        Field* fields = result->Fetch();

        m_DBVersion = fields[0].GetString();
        // will be overwrite by config values if different and non-0
        m_int_configs[CONFIG_CLIENTCACHE_VERSION] = fields[1].GetUInt32();
    }

    if (m_DBVersion.empty())
        m_DBVersion = "Unknown world database.";
}

void World::ProcessStartEvent()
{
    isEventKillStart = true;
}

void World::ProcessStopEvent()
{
    isEventKillStart = false;
}

void World::UpdateAreaDependentAuras()
{
#ifdef CROSS
    PlayerMap::const_iterator itr;
    for (itr = m_players.begin(); itr != m_players.end(); ++itr)
        if (itr->second && itr->second && itr->second->IsInWorld())
        {
            itr->second->UpdateAreaDependentAuras(itr->second->GetAreaId());
            itr->second->UpdateZoneDependentAuras(itr->second->GetZoneId());
        }
#else
    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second && itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
        {
            itr->second->GetPlayer()->UpdateAreaDependentAuras(itr->second->GetPlayer()->GetAreaId());
            itr->second->GetPlayer()->UpdateZoneDependentAuras(itr->second->GetPlayer()->GetZoneId());
        }
#endif
}

void World::LoadWorldStates()
{
    uint32 oldMSTime = getMSTime();

    QueryResult result = CharacterDatabase.Query("SELECT entry, value FROM worldstates");

    if (!result)
    {
        TC_LOG_INFO("server.loading", ">> Loaded 0 world states. DB table `worldstates` is empty!");

        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        m_worldstates[fields[0].GetUInt32()] = fields[1].GetUInt32();
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", ">> Loaded %u world states in %u ms", count, GetMSTimeDiffToNow(oldMSTime));

}

void World::FillMapsToLoad()
{
    std::string options = sConfigMgr->GetStringDefault("loading.onlymaps", "");
    Tokenizer l_Tokens(options, ',');

    for (Tokenizer::const_iterator l_Iter = l_Tokens.begin(); l_Iter != l_Tokens.end(); ++l_Iter)
        m_MapsToLoad.push_back(uint32(atol(*l_Iter)));
}

bool World::CanBeSaveInLoginDatabase() const
{
    switch (m_int_configs[CONFIG_REALM_ZONE])
    {
        // REALM_ZONE_DEVELOPMENT is often used for international private servers
        // so allow saving to the acc DB for this realm type
        // case REALM_ZONE_DEVELOPMENT:
        case REALM_ZONE_TEST_SERVER:
        case REALM_ZONE_QA_SERVER:
            return false;
        default:
            return true;
    }
}

// Setting a worldstate will save it to DB
void World::setWorldState(uint32 index, uint64 value)
{
    WorldStatesMap::const_iterator it = m_worldstates.find(index);
    if (it != m_worldstates.end())
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_WORLDSTATE);

        stmt->setUInt32(0, uint32(value));
        stmt->setUInt32(1, index);

        CharacterDatabase.Execute(stmt);
    }
    else
    {
        PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_WORLDSTATE);

        stmt->setUInt32(0, index);
        stmt->setUInt32(1, uint32(value));

        CharacterDatabase.Execute(stmt);
    }
    m_worldstates[index] = value;
}

uint64 World::getWorldState(uint32 index) const
{
    WorldStatesMap::const_iterator it = m_worldstates.find(index);
    return it != m_worldstates.end() ? it->second : 0;
}

void World::ProcessQueryCallbacks()
{
    PreparedQueryResult result;

    for (std::deque<std::future<PreparedQueryResult>>::iterator itr = m_realmCharCallbacks.begin(); itr != m_realmCharCallbacks.end(); )
    {
        if ((*itr).wait_for(std::chrono::seconds(0)) != std::future_status::ready)
        {
            ++itr;
            continue;
        }

        result = (*itr).get();
        _UpdateRealmCharCount(result);
        itr = m_realmCharCallbacks.erase(itr);
    }

    /// - Update querys holders callbacks
    if (!m_QueryHolderCallbacks->empty())
    {
        m_QueryHolderCallbacks->remove_if([](QueryHolderCallback const& p_Callback) -> bool
        {
            if (p_Callback.m_QueryResultHolderFuture.ready())
            {
                SQLQueryHolder* l_QueryHolder;
                p_Callback.m_QueryResultHolderFuture.get(l_QueryHolder);

                p_Callback.m_Callback(l_QueryHolder);
                return true;
            }

            return false;
        });
    }

    /// - Add querys holders callbacks in buffer queue to real queue
    while (!m_QueryHolderCallbacksBuffer->empty())
    {
        m_QueryHolderCallbacks->push_front(m_QueryHolderCallbacksBuffer->front());
        m_QueryHolderCallbacksBuffer->pop_front();
    }

    /// - Update transactions callbacks
    if (!m_TransactionCallbacks->empty())
    {
        m_TransactionCallbacks->remove_if([](MS::Utilities::CallBackPtr const& l_Callback) -> bool
        {
            if (l_Callback->m_State == MS::Utilities::CallBackState::Waiting)
                return false;

            l_Callback->m_CallBack(l_Callback->m_State == MS::Utilities::CallBackState::Success);
            return true;
        });
    }

    /// - Add transactions callbacks in buffer queue to real queue
    while (!m_TransactionCallbacksBuffer->empty())
    {
        m_TransactionCallbacks->push_front(m_TransactionCallbacksBuffer->front());
        m_TransactionCallbacksBuffer->pop_front();
    }

    /// - Update prepared statements callbacks
    if (!m_PreparedStatementCallbacks->empty())
    {
        m_PreparedStatementCallbacks->remove_if([](PrepareStatementCallback const& p_Callback) -> bool
        {
            /// If the query result is avaiable ...
            if (p_Callback.second.ready())
            {
                /// Then get it
                PreparedQueryResult l_Result;
                p_Callback.second.get(l_Result);

                /// Give the result to the callback, and execute it
                p_Callback.first(l_Result);

                /// Delete the callback from the forward list
                return true;
            }

            /// We havn't the query result yet, we keep the callback and wait for the result!
            return false;
        });
    }

    /// - Add prepared statements in buffer queue to real queue
    while (!m_PreparedStatementCallbacksBuffer->empty())
    {
        m_PreparedStatementCallbacks->push_front(m_PreparedStatementCallbacksBuffer->front());
        m_PreparedStatementCallbacksBuffer->pop_front();
    }
}

void World::UpdatePhaseDefinitions()
{
#ifdef CROSS
    PlayerMap::const_iterator itr;
    for (itr = m_players.begin(); itr != m_players.end(); ++itr)
        if (itr->second && itr->second && itr->second->IsInWorld())
            itr->second->GetPhaseMgr().NotifyStoresReloaded();
#else
    SessionMap::const_iterator itr;
    for (itr = m_sessions.begin(); itr != m_sessions.end(); ++itr)
        if (itr->second && itr->second->GetPlayer() && itr->second->GetPlayer()->IsInWorld())
            itr->second->GetPlayer()->GetPhaseMgr().NotifyStoresReloaded();
#endif
}

bool World::ModerateMessage(std::string l_Text)
{
    if (!m_lexicsCutter)
        return false;

    return m_lexicsCutter->CheckLexics(l_Text);
}

#ifndef CROSS
std::string World::GetNormalizedRealmName() const
{
    std::string l_NormalizedName = "";
    for (uint32 l_I = 0; l_I < m_realmName.length(); l_I++)
    {
        if (m_realmName[l_I] == ' ')
            continue;

        l_NormalizedName += m_realmName[l_I];
    }

    return l_NormalizedName;
}
#endif

#ifdef CROSS
void World::AddPlayer(Player* player)
{
    ASSERT(player);

    playersLock.lock();

    m_players[player->GetGUID()] = player;

    playersLock.release();
}

bool World::HasPlayer(uint64 guid) const
{
    return (m_players.find(guid) != m_players.end());
}

Player* World::GetPlayer(uint64 guid)
{
    PlayerMap::iterator itr = m_players.find(guid);
    if (itr != m_players.end())
        return itr->second;

    return NULL;
}

const PlayerMap& World::GetAllPlayers()
{
    return m_players;
}

void World::UpdateInterRealmStat()
{
    ClientMap const& clients = sInterRealmMgr->GetClients();

    SQLTransaction trans = CharacterDatabase.BeginTransaction();
    trans->PAppend("DELETE FROM interrealmstat");
    uint32 count = 0;

    playersLock.lock();

    for (PlayerMap::const_iterator itr = m_players.begin(); itr != m_players.end(); ++itr)
    {
        Player* player = itr->second;
        if (player->GetSession()->isInInterRealmBG())
        {
            count++;
        }
    }

    playersLock.release();

    uint32 uptimeDiff = uint32(m_gameTime - m_startTime);

    trans->PAppend("REPLACE INTO interrealmstat VALUES (%u, %u, %u)", count, uint32(m_startTime), uptimeDiff);

    CharacterDatabase.CommitTransaction(trans);
}
#endif

#ifndef CROSS
void World::LoadCharacterInfoStore()
{
    TC_LOG_INFO("server.loading", "Loading character name data");

    QueryResult result = CharacterDatabase.Query("SELECT guid, name, account, race, gender, class, level FROM characters WHERE deleteDate IS NULL OR deleteDate = 0");
    if (!result)
    {
        TC_LOG_INFO("server.loading", "No character name data loaded, empty query");
        return;
    }

    uint32 count = 0;
    uint32 l_OldMSTime = getMSTime();

    do
    {
        Field* fields = result->Fetch();
        AddCharacterInfo(fields[0].GetUInt32(), fields[1].GetString(), fields[2].GetUInt32() /*accountid*/,
                         fields[4].GetUInt8() /*gender*/, fields[3].GetUInt8() /*race*/, fields[5].GetUInt8() /*class*/, fields[6].GetUInt8() /*level*/);
        ++count;
    }
    while (result->NextRow());

    TC_LOG_INFO("server.loading", "Loaded name data for %u characters in %u ms.", count, GetMSTimeDiffToNow(l_OldMSTime));
}

void World::AddCharacterInfo(uint32 guid, std::string const& name, uint32 accountId, uint8 gender, uint8 race, uint8 playerClass, uint8 level)
{
    CharacterInfo& data = _characterInfoStore[guid];
    data.Name       = name;
    data.AccountId  = accountId;
    data.Race       = race;
    data.Sex        = gender;
    data.Class      = playerClass;
    data.Level      = level;
}

void World::UpdateCharacterInfo(uint32 guid, std::string const& name, uint8 gender /*= GENDER_NONE*/, uint8 race /*= RACE_NONE*/)
{
    CharacterInfoContainer::iterator itr = _characterInfoStore.find(guid);
    if (itr == _characterInfoStore.end())
        return;

    itr->second.Name = name;

    if (gender != GENDER_NONE)
        itr->second.Sex = gender;

    if (race != RACE_NONE)
        itr->second.Race = race;
}

void World::UpdateCharacterInfoLevel(uint32 guid, uint8 level)
{
    CharacterInfoContainer::iterator itr = _characterInfoStore.find(guid);
    if (itr == _characterInfoStore.end())
        return;

    itr->second.Level = level;
}

CharacterInfo  const* World::GetCharacterInfo(uint32 guid) const
{
    CharacterInfoContainer::const_iterator itr = _characterInfoStore.find(guid);
    if (itr != _characterInfoStore.end())
        return &itr->second;
    else
        return NULL;
}

uint64 World::GetCharacterGuidByName(std::string const& p_Name)
{
    for (CharacterInfoContainer::iterator itr = _characterInfoStore.begin(); itr != _characterInfoStore.end(); ++itr)
    {
        if (itr->second.Name == p_Name)
            return MAKE_NEW_GUID(itr->first, 0, HIGHGUID_PLAYER);
    }

    return 0;
}

void World::_updateTransfers()
{
    /// Regular transfers (realm to realm)
    if (m_timers[WUPDATE_TRANSFER].Passed())
    {
        // Prepare transfer dump callback ...
        {
            PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_TRANSFERS_DUMP);
            l_Stmt->setUInt32(0, sLog->GetRealmID());

            m_transfersDumpCallbacks = LoginDatabase.AsyncQuery(l_Stmt);
        }

        // Prepare transfer load callback ...
        {
            PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_TRANSFERS_LOAD);
            l_Stmt->setUInt32(0, sLog->GetRealmID());

            m_transfersLoadCallbacks = LoginDatabase.AsyncQuery(l_Stmt);
        }

        m_timers[WUPDATE_TRANSFER].SetInterval(HOUR * IN_MILLISECONDS);
        m_timers[WUPDATE_TRANSFER].Reset();
    }

    if (m_transfersDumpCallbacks.ready() && m_transfersLoadCallbacks.ready())
    {
        PreparedQueryResult l_ToDump;
        PreparedQueryResult l_ToLoad;

        m_transfersDumpCallbacks.get(l_ToDump);
        m_transfersLoadCallbacks.get(l_ToLoad);

        if (l_ToDump)
        {
            do
            {
                Field* l_Field = l_ToDump->Fetch();
                uint32 l_Transaction = l_Field[0].GetUInt32();
                uint32 l_AccountID = l_Field[1].GetUInt32();
                uint32 l_CharGUID = l_Field[2].GetUInt32();

                /// Transfers aren't allowed with legacy account
                if (l_AccountID & 0x40000000)
                    continue;

                if (Player * l_Player = sObjectMgr->GetPlayerByLowGUID(l_CharGUID))
                    continue;

                bool l_Error = true;
                std::string l_Dump;

                if (PlayerDumpWriter().GetDump(l_CharGUID, l_AccountID, l_Dump, false))
                {
                    PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_UPD_TRANSFER_PDUMP);
                    stmt->setString(0, l_Dump);
                    stmt->setUInt32(1, l_Transaction);

                    if (LoginDatabase.DirectExecuteWithReturn(stmt))
                    {
                        l_Error = false;

                        CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid = '%u'", l_CharGUID);
                        CharacterDatabase.PExecute("DELETE FROM guild_member WHERE guid = '%u'", l_CharGUID);
                        CharacterDatabase.PExecute("UPDATE characters SET deleteInfos_Name=name, deleteInfos_Account=account, deleteDate='" UI64FMTD "', name='', account=0 WHERE guid=%u", uint64(time(NULL)), l_CharGUID);
                    }
                }

                if (l_Error)
                {
                    TC_LOG_TRACE("server.worldserver", "PlayerDump fail ! (guid %u)", l_CharGUID);
                    LoginDatabase.PExecute("UPDATE webshop_delivery_interrealm_transfer SET nb_attempt = nb_attempt + 1 WHERE id = %u", l_Transaction);
                    continue;
                }
            }
            while (l_ToDump->NextRow());
        }

        if (l_ToLoad)
        {
            uint32 l_Now = getMSTime();

            do
            {
                /// Prevent freeze
                if (GetMSTimeDiffToNow(l_Now) > 5000)
                    break;

                uint32 l_Timestamp     = getMSTime();
                Field* l_Field         = l_ToLoad->Fetch();
                uint32 l_Transaction   = l_Field[0].GetUInt32();
                uint32 l_AccountID     = l_Field[1].GetUInt32();
                uint32 l_CharGUID      = l_Field[2].GetUInt32();
                std::string l_CharDump = l_Field[3].GetString();

                /// Transfers aren't allowed with legacy account
                if (l_AccountID & 0x40000000)
                    continue;

                std::ostringstream l_Filename;
                l_Filename << "pdump/" << l_AccountID << "_" << l_CharGUID << "_" << l_Timestamp;

                FILE* l_File = fopen(l_Filename.str().c_str(), "w");
                if (!l_File)
                    continue;

                fprintf(l_File, "%s\n", l_CharDump.c_str());
                fclose(l_File);

                DumpReturn l_Error = PlayerDumpReader().LoadDump(l_Filename.str(), l_AccountID, "#Transfer", 0);
                remove(l_Filename.str().c_str());

                if (l_Error == DUMP_SUCCESS)
                {
                    LoginDatabase.PQuery("UPDATE webshop_delivery_interrealm_transfer SET state = 2 WHERE id = %u", l_Transaction);
                    continue;
                }

                LoginDatabase.PQuery("UPDATE webshop_delivery_interrealm_transfer SET error = %u, nb_attempt = nb_attempt + 1 WHERE id = %u", (uint32)l_Error, l_Transaction);
            }
            while (l_ToLoad->NextRow());
        }

        m_transfersDumpCallbacks.cancel();
        m_transfersLoadCallbacks.cancel();

        m_timers[WUPDATE_TRANSFER].SetInterval(15 * MINUTE * IN_MILLISECONDS);
        m_timers[WUPDATE_TRANSFER].Reset();
    }

    /// Inter exp transfers
    if (m_timers[WUPDATE_TRANSFER_EXP].Passed())
    {
        // Prepare transfer dump callback ...
        {
            PreparedStatement* l_Stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_TRANSFERS_EXP_LOAD);
            l_Stmt->setUInt32(0, sLog->GetRealmID());

            m_transfersExpLoadCallback = LoginDatabase.AsyncQuery(l_Stmt);
        }

        m_timers[WUPDATE_TRANSFER_EXP].SetInterval(HOUR * IN_MILLISECONDS);
        m_timers[WUPDATE_TRANSFER_EXP].Reset();
    }

    if (m_transfersExpLoadCallback.ready())
    {
        PreparedQueryResult l_ToLoad;
        m_transfersExpLoadCallback.get(l_ToLoad);

        if (l_ToLoad)
        {
            uint32 l_Now = getMSTime();

            do
            {
                /// Prevent freeze
                if (GetMSTimeDiffToNow(l_Now) > 5000)
                    break;

                uint32 l_Timestamp     = getMSTime();
                Field* l_Field         = l_ToLoad->Fetch();
                uint32 l_Transaction   = l_Field[0].GetUInt32();
                uint32 l_AccountID     = l_Field[1].GetUInt32();
                uint32 l_CharGUID      = l_Field[2].GetUInt32();
                std::string l_CharDump = l_Field[3].GetString();

                /// Transfers aren't allowed with legacy account
                if (l_AccountID & 0x40000000)
                    continue;

                std::ostringstream l_Filename;
                l_Filename << "pdump/" << l_AccountID << "_" << l_CharGUID << "_" << l_Timestamp;

                FILE* l_File = fopen(l_Filename.str().c_str(), "w");
                if (!l_File)
                    continue;

                fprintf(l_File, "%s\n", l_CharDump.c_str());
                fclose(l_File);

                DumpReturn l_Error = PlayerDumpReader().LoadDump(l_Filename.str(), l_AccountID, "#Transfer", 0, false, AtLoginFlags::AT_LOGIN_RENAME | AtLoginFlags::AT_LOGIN_DELETE_INVALID_SPELL);
                remove(l_Filename.str().c_str());

                if (l_Error == DUMP_SUCCESS)
                {
                    LoginDatabase.PQuery("UPDATE webshop_delivery_interexp_transfer SET state = 2 WHERE id = %u", l_Transaction);
                    continue;
                }

                //if (l_Error != DUMP_TOO_MANY_CHARS)
                //    TC_LOG_ERROR("server.world", "Inter Exp Transfer to realm [%u] on account [%u] failed. ErrorCode [%u]", g_RealmID, l_AccountID, l_Error);

                LoginDatabase.PQuery("UPDATE webshop_delivery_interexp_transfer SET error = %u, nb_attempt = nb_attempt + 1 WHERE id = %u", (uint32)l_Error, l_Transaction);
            }
            while (l_ToLoad->NextRow());
        }

        m_transfersExpLoadCallback.cancel();
        m_timers[WUPDATE_TRANSFER_EXP].SetInterval(20 * MINUTE * IN_MILLISECONDS);
        m_timers[WUPDATE_TRANSFER_EXP].Reset();
    }
}
#endif

void World::InitServerAutoRestartTime()
{
    time_t serverRestartTime = uint64(sWorld->getWorldState(WS_AUTO_SERVER_RESTART_TIME));
    if (!serverRestartTime)
        m_NextServerRestart = time_t(time(NULL));         // game time not yet init

    // generate time by config
    time_t curTime = time(NULL);
    tm localTm;
    localtime_r(&curTime, &localTm);
    localTm.tm_hour = getIntConfig(CONFIG_AUTO_SERVER_RESTART_HOUR);
    localTm.tm_min = 0;
    localTm.tm_sec = 0;

    // current day reset time
    time_t nextDayRestartTime = mktime(&localTm);

    // next reset time before current moment
    if (curTime >= nextDayRestartTime)
        nextDayRestartTime += DAY;

    // normalize reset time
    m_NextServerRestart = serverRestartTime < curTime ? nextDayRestartTime - DAY : nextDayRestartTime;

    if (!serverRestartTime)
        sWorld->setWorldState(WS_AUTO_SERVER_RESTART_TIME, uint64(m_NextServerRestart));

    if (m_bool_configs[CONFIG_DISABLE_RESTART])
        m_NextServerRestart += DAY * 1;
}

void World::AutoRestartServer()
{
    if (sWorld->getBoolConfig(CONFIG_DISABLE_RESTART))
        return;

    sWorld->ShutdownServ(300, SHUTDOWN_MASK_RESTART, RESTART_EXIT_CODE);

    m_NextServerRestart = time_t(m_NextServerRestart + DAY);
    sWorld->setWorldState(WS_AUTO_SERVER_RESTART_TIME, uint64(m_NextServerRestart));
}

void World::SendRaidQueueInfo(Player* player)
{
    using namespace lfg;

    std::function<std::string(uint32, uint32, uint32, uint32)> makeText = [](uint32 dungeon, uint32 tanks, uint32 healers, uint32 dps)
    {
        return std::to_string(dungeon) + std::string("=") + std::to_string(tanks) + std::string(",") + std::to_string(healers) +
            std::string(",") + std::to_string(dps) + std::string(";");
    };

    std::string text;
    for (auto&& manager : sLFGMgr->GetQueueManagers())
    {
        for (auto&& dungeonId : { 416, 417, 527, 528, 529, 530, 526, 610, 611, 612, 613, 716, 717, 724, 725 })
        {
            auto& q = manager.second.GetQueue(dungeonId);
            if (!q.GetBuckets().empty())
                text += makeText(dungeonId, q.GetTotalPlayers(PLAYER_ROLE_TANK), q.GetTotalPlayers(PLAYER_ROLE_HEALER), q.GetTotalPlayers(PLAYER_ROLE_DAMAGE));
            else
                text += makeText(dungeonId, 0, 0, 0);
        }
    }

    std::function<void(Player*)> sendInfo = [text](Player* plr)
    {
        WorldPacket data;
        //ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_ADDON, plr, plr, text, 0, "", DEFAULT_LOCALE, "RaidQueue");
        plr->GetSession()->SendPacket(&data);
    };

    if (player)
        sendInfo(player);
    else
    {
        for (auto&& itr : sWorld->GetAllSessions())
            if (Player* plr = itr.second->GetPlayer())
                sendInfo(plr);
    }
}

// Global World instance
World* sWorldInstance = nullptr;

World* World::instance()
{
    return sWorldInstance;
}