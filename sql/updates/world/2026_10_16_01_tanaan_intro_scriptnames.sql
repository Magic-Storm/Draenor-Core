-- Tanaan Intro (Dark Portal / Assault on the Dark Portal)
-- Clear npc_generic_tanaan_guardian from non-intro creatures (wrong base-DB bindings)

UPDATE `creature_template` SET `ScriptName`='' WHERE `entry` IN (12922, 42874)
	AND `ScriptName`='npc_generic_tanaan_guardian';

-- Ensure stair vanguards / wave NPCs keep the guardian AI
UPDATE `creature_template` SET `ScriptName`='npc_generic_tanaan_guardian' WHERE `entry` IN (81990, 82188, 82781);
UPDATE `creature_template` SET `ScriptName`='npc_iron_grunt' WHERE `entry`=78883;
UPDATE `creature_template` SET `ScriptName`='npc_archmage_khadgar' WHERE `entry` IN (78558, 78559);
