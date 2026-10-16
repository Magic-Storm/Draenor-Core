-- Vaultkeeper Sarniv (Alliance Storehouse L3): SmartAI was wrong for a void-storage NPC.
-- Align with Horde Vaultkeeper Maeelthas (85288) — npcflags/IconName already set voidstorage.
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = '' WHERE `entry` = 85290;
