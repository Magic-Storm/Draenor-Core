////////////////////////////////////////////////////////////////////////////////
//
// Project-Hellscream https://hellscream.org
// Copyright (C) 2018-2020 Project-Hellscream-6.2
// Discord https://discord.gg/CWCF3C9
//
////////////////////////////////////////////////////////////////////////////////

#include "ScriptPCH.h"
#include "SpellMgr.h"
#include "WorldSession.h"

/// Zone outposts. The drafting table opens the client player-choice window
/// (SMSG_DISPLAY_PLAYER_CHOICE). The chosen spell is the zone ability.

enum OutpostSpells
{
    SpellMechashredder      = 164050, ///< Gorgrond lumber yard
    SpellChampionsHonor     = 164221, ///< Gorgrond sparring arena
    SpellArtilleryStrike    = 162075, ///< Talador arsenal
    SpellGuardianOrb        = 161768, ///< Talador arcane sanctum
    SpellSmugglingRun       = 170097, ///< Spires of Arak trading post
    SpellHomeAwayFromHome   = 168487, ///< Spires of Arak brewery
    SpellTelaariTalbuk      = 165803, ///< Nagrand corral, Alliance
    SpellFrostwolfWarWolf   = 164222, ///< Nagrand corral, Horde
    SpellWorvarDemolisher   = 160241  ///< Nagrand tankworks, Horde
};

enum OutpostQuests
{
    QuestWeNeedAnOutpost    = 35063, ///< Alliance Gorgrond
    QuestYourBaseYourChoice = 35151, ///< Horde Gorgrond
    QuestAtYourCommandA     = 34558, ///< Alliance Talador
    QuestAtYourCommandH     = 34566, ///< Horde Talador
    QuestOrdersCommanderH   = 35277, ///< Horde Spires of Arak
    QuestOrdersCommanderA   = 35286, ///< Alliance Spires of Arak
    QuestAChoiceToMakeA     = 34769, ///< Alliance Nagrand
    QuestAChoiceToMakeH     = 34899  ///< Horde Nagrand
};

struct OutpostSite
{
    uint32 Quest;
    uint32 Credit;
    uint32 Table;
    uint32 FirstSpell;
    uint32 SecondSpell;
    char const* FirstText;
    char const* FirstDescription;
    char const* SecondText;
    char const* SecondDescription;
    bool SecondIsSiegeEngine;
    uint64 SwapCost;
};

static OutpostSite const g_Sites[] =
{
    { QuestWeNeedAnOutpost, 81119, 0, SpellMechashredder, SpellChampionsHonor,
      "Highpass Logging Camp", "Operations in the southern overgrowth. Grants Summon Mechashredder 5000 in Gorgrond.",
      "Highpass Sparring Ring", "Operations in the northern wastelands. Grants Champion's Honor in Gorgrond.", false, 5000 * GOLD },
    { QuestYourBaseYourChoice, 74594, 0, SpellMechashredder, SpellChampionsHonor,
      "Lumber Mill", "Grants Summon Mechashredder 5000 in Gorgrond.",
      "Sparring Arena", "Grants Champion's Honor in Gorgrond.", false, 5000 * GOLD },
    { QuestAtYourCommandA, 79807, 230665, SpellArtilleryStrike, SpellGuardianOrb,
      "Wrynn Artillery Tower", "Three artillery volleys at the targeted location. Grants Artillery Strike in Talador.",
      "Arcane Sanctum", "Summons a Guardian Orb that electrocutes nearby enemies. Grants Guardian Orb in Talador.", false, 5000 * GOLD },
    { QuestAtYourCommandH, 79807, 231231, SpellArtilleryStrike, SpellGuardianOrb,
      "Arsenal", "Three artillery volleys at the targeted location. Grants Artillery Strike in Talador.",
      "Arcane Sanctum", "Summons a Guardian Orb that electrocutes nearby enemies. Grants Guardian Orb in Talador.", false, 5000 * GOLD },
    { QuestOrdersCommanderH, 81918, 232062, SpellHomeAwayFromHome, SpellSmugglingRun,
      "Brewery", "Teleport to your inn in Spires of Arak. Experience gains there are increased.",
      "Smuggling Run", "Summon a rare goods vendor. Grants Smuggling Run! in Spires of Arak.", false, 5000 * GOLD },
    { QuestOrdersCommanderA, 81919, 232089, SpellHomeAwayFromHome, SpellSmugglingRun,
      "Brewery", "Teleport to your inn in Spires of Arak. Experience gains there are increased.",
      "Smuggling Run", "Summon a rare goods vendor. Grants Smuggling Run! in Spires of Arak.", false, 5000 * GOLD },
    { QuestAChoiceToMakeA, 79679, 0, SpellTelaariTalbuk, 0,
      "Corral", "A Telaari Talbuk that can be ridden while in combat.",
      "Tankworks", "Summons a Telaari Siege Engine.", true, 500 * GOLD },
    { QuestAChoiceToMakeH, 80143, 0, SpellFrostwolfWarWolf, SpellWorvarDemolisher,
      "Corral", "A Frostwolf War Wolf that can be ridden while in combat.",
      "Tankworks", "Summons a Wor'var Demolisher.", false, 500 * GOLD }
};

struct OutpostLink
{
    uint32 Entry;
    uint32 Quest;
    bool GameObject;
};

static OutpostLink const g_Links[] =
{
    { 81119,  QuestWeNeedAnOutpost,    false },
    { 75136,  QuestWeNeedAnOutpost,    false },
    { 81588,  QuestWeNeedAnOutpost,    false }, ///< Thaelin Darkanvil, beside the Highpass table
    { 74594,  QuestYourBaseYourChoice, false },
    { 79133,  QuestAtYourCommandA,     false },
    { 79176,  QuestAtYourCommandH,     false },
    { 81920,  QuestOrdersCommanderH,   false },
    { 81929,  QuestOrdersCommanderA,   false },
    { 79674,  QuestAChoiceToMakeA,     false },
    { 79576,  QuestAChoiceToMakeA,     false },
    { 80003,  QuestAChoiceToMakeH,     false },
    { 79807,  QuestAtYourCommandA,     false },
    { 81918,  QuestOrdersCommanderH,   false },
    { 81919,  QuestOrdersCommanderA,   false },
    { 79679,  QuestAChoiceToMakeA,     false },
    { 80143,  QuestAChoiceToMakeH,     false },
    { 230665, QuestAtYourCommandA,     true  },
    { 231231, QuestAtYourCommandH,     true  },
    { 232062, QuestOrdersCommanderH,   true  },
    { 232089, QuestOrdersCommanderA,   true  }
};

static OutpostSite const* SiteForQuest(uint32 p_Quest)
{
    for (OutpostSite const& l_Site : g_Sites)
        if (l_Site.Quest == p_Quest)
            return &l_Site;

    return nullptr;
}

static OutpostSite const* SiteFor(uint32 p_Entry, bool p_GameObject)
{
    for (OutpostLink const& l_Link : g_Links)
    {
        if (l_Link.Entry == p_Entry && l_Link.GameObject == p_GameObject)
            return SiteForQuest(l_Link.Quest);
    }

    return nullptr;
}

/// Alliance tankworks summon. The horde twin is 160241; the alliance id is
/// resolved from the client spell name so a guessed id is never taught.
static uint32 TelaariSiegeEngineSpell()
{
    static int32 l_Cached = -1;
    if (l_Cached >= 0)
        return uint32(l_Cached);

    l_Cached = 0;
    for (uint32 l_Id = 160000; l_Id <= 172000; ++l_Id)
    {
        SpellInfo const* l_Info = sSpellMgr->GetSpellInfo(l_Id);
        if (!l_Info || !l_Info->SpellName || strcmp(l_Info->SpellName, "Telaari Siege Engine") != 0)
            continue;

        if (!l_Cached)
            l_Cached = int32(l_Id);

        if (l_Info->GetEffectByType(SPELL_EFFECT_SUMMON))
        {
            l_Cached = int32(l_Id);
            break;
        }
    }

    return uint32(l_Cached);
}

static uint32 SiteSpell(OutpostSite const* p_Site, bool p_Second)
{
    if (!p_Second)
        return p_Site->FirstSpell;

    if (p_Site->SecondIsSiegeEngine)
        return TelaariSiegeEngineSpell();

    return p_Site->SecondSpell;
}

static void ApplyOutpostChoice(Player* p_Player, OutpostSite const* p_Site, bool p_Second)
{
    uint32 l_Spell = SiteSpell(p_Site, p_Second);
    uint32 l_Other = SiteSpell(p_Site, !p_Second);

    if (l_Other && p_Player->HasSpell(l_Other))
        p_Player->removeSpell(l_Other);

    if (l_Spell && !p_Player->HasSpell(l_Spell))
        p_Player->learnSpell(l_Spell, false);

    p_Player->KilledMonsterCredit(p_Site->Credit);

    if (p_Site->Table)
        p_Player->QuestObjectiveSatisfy(p_Site->Table, 1, QUEST_OBJECTIVE_TYPE_GO);
}

static bool AlreadyChose(Player* p_Player, OutpostSite const* p_Site)
{
    uint32 l_First = SiteSpell(p_Site, false);
    uint32 l_Second = SiteSpell(p_Site, true);
    return (l_First && p_Player->HasSpell(l_First)) || (l_Second && p_Player->HasSpell(l_Second));
}

static void WriteChoiceResponse(WorldPacket& p_Data, int32 p_ResponseId, char const* p_Answer, char const* p_Description)
{
    p_Data << int32(p_ResponseId);
    p_Data << int32(0);
    p_Data.WriteBits(strlen(p_Answer), 9);
    p_Data.WriteBits(strlen(p_Description), 11);
    p_Data.WriteBit(0);
    p_Data.FlushBits();
    p_Data.WriteString(p_Answer);
    p_Data.WriteString(p_Description);
}

static void SendOutpostChoice(Player* p_Player, OutpostSite const* p_Site)
{
    std::string l_Question = "What outpost structure should we construct?";

    WorldPacket l_Data(SMSG_DISPLAY_PLAYER_CHOICE, 512);
    l_Data << int32(p_Site->Quest);
    l_Data.WriteBits(l_Question.size(), 8);
    l_Data.FlushBits();
    l_Data.WriteString(l_Question);
    l_Data << uint32(2);
    WriteChoiceResponse(l_Data, 1, p_Site->FirstText, p_Site->FirstDescription);
    WriteChoiceResponse(l_Data, 2, p_Site->SecondText, p_Site->SecondDescription);
    p_Player->GetSession()->SendPacket(&l_Data);
}

static bool OfferOutpostChoice(Player* p_Player, OutpostSite const* p_Site)
{
    if (!p_Site)
        return false;

    if (p_Player->GetQuestStatus(p_Site->Quest) == QUEST_STATUS_INCOMPLETE)
    {
        if (AlreadyChose(p_Player, p_Site))
        {
            ApplyOutpostChoice(p_Player, p_Site, p_Player->HasSpell(SiteSpell(p_Site, true)));
            return false;
        }

        SendOutpostChoice(p_Player, p_Site);
        return true;
    }

    /// After 16 December 2014 the foreman swaps the building at level 100.
    if (p_Player->getLevel() >= 100 && AlreadyChose(p_Player, p_Site))
    {
        SendOutpostChoice(p_Player, p_Site);
        return true;
    }

    return false;
}

void WorldSession::HandleChoiceResponseOpcode(WorldPacket& p_RecvData)
{
    if (!m_Player)
        return;

    int32 l_ChoiceID = 0;
    int32 l_ResponseID = 0;
    p_RecvData >> l_ChoiceID;
    p_RecvData >> l_ResponseID;

    if (l_ResponseID != 1 && l_ResponseID != 2)
        return;

    OutpostSite const* l_Site = SiteForQuest(uint32(l_ChoiceID));
    if (!l_Site)
        return;

    bool l_Second = l_ResponseID == 2;
    bool l_FirstChoice = m_Player->GetQuestStatus(l_Site->Quest) == QUEST_STATUS_INCOMPLETE;

    if (!l_FirstChoice)
    {
        if (m_Player->getLevel() < 100 || !AlreadyChose(m_Player, l_Site))
            return;

        uint32 l_Spell = SiteSpell(l_Site, l_Second);
        if (l_Spell && m_Player->HasSpell(l_Spell))
            return;

        if (!m_Player->HasEnoughMoney(l_Site->SwapCost))
        {
            m_Player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, nullptr, 0, 0);
            return;
        }

        m_Player->ModifyMoney(-int64(l_Site->SwapCost));
    }

    ApplyOutpostChoice(m_Player, l_Site, l_Second);
}

class go_draenor_outpost_drafting_table : public GameObjectScript
{
    public:
        go_draenor_outpost_drafting_table() : GameObjectScript("go_draenor_outpost_drafting_table") { }

        bool OnGossipHello(Player* p_Player, GameObject* p_GameObject) override
        {
            OutpostSite const* l_Site = SiteFor(p_GameObject->GetEntry(), true);
            return OfferOutpostChoice(p_Player, l_Site);
        }
};

class npc_draenor_outpost_choice : public CreatureScript
{
    public:
        npc_draenor_outpost_choice() : CreatureScript("npc_draenor_outpost_choice") { }

        bool OnGossipHello(Player* p_Player, Creature* p_Creature) override
        {
            OutpostSite const* l_Site = SiteFor(p_Creature->GetEntry(), false);

            /// Talador's shared credit serves both factions. Pick the quest the player actually has.
            if (p_Creature->GetEntry() == 79807)
            {
                if (p_Player->GetQuestStatus(QuestAtYourCommandA) == QUEST_STATUS_INCOMPLETE)
                    l_Site = SiteForQuest(QuestAtYourCommandA);
                else if (p_Player->GetQuestStatus(QuestAtYourCommandH) == QUEST_STATUS_INCOMPLETE)
                    l_Site = SiteForQuest(QuestAtYourCommandH);
            }

            return OfferOutpostChoice(p_Player, l_Site);
        }
};

#ifndef __clang_analyzer__
void AddSC_draenor_outposts()
{
    new go_draenor_outpost_drafting_table();
    new npc_draenor_outpost_choice();
}
#endif
