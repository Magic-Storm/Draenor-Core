-- Ancient Trading Mechanism: ScriptName already set; SmartAI fights C++ script (auction quests 36948 / 37014).
UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (86806, 87206) AND `ScriptName` = 'npc_AncientTradingMechanism_Garr';
