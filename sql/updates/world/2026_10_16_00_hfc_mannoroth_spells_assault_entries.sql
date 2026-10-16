-- Hellfire Citadel: Mannoroth spell_script_names (missing from 2026_03_25_02)
-- + Hellfire Assault real creature entries (93856/93846 were wrong templates)

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
	'spell_citadel_curse_of_the_legion','spell_citadel_mannoroth_gaze',
	'spell_citadel_mannoroth_gaze_damage','spell_citadel_spire_channel',
	'spell_citadel_spire_channel_perma','spell_citadel_spire_beam',
	'spell_citadel_summoner_beam'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(181275, 'spell_citadel_curse_of_the_legion'),
(181597, 'spell_citadel_mannoroth_gaze'),
(181617, 'spell_citadel_mannoroth_gaze_damage'),
(185256, 'spell_citadel_spire_channel'),
(187029, 'spell_citadel_spire_channel_perma'),
(182259, 'spell_citadel_spire_beam'),
(186648, 'spell_citadel_summoner_beam');

-- Clear ScriptName from wrong templates (Mayla / Mountainstrider Bull)
UPDATE `creature_template` SET `ScriptName`='' WHERE `entry` IN (93846, 93856)
	AND `ScriptName` IN ('npc_felfire_munitions_crate_hfa', 'npc_hellfire_engineer_hfa');

-- Felfire Demolisher drops munitions; Felfire Munitions is the pickup NPC
UPDATE `creature_template` SET `ScriptName`='npc_hellfire_engineer_hfa' WHERE `entry`=91103;
UPDATE `creature_template` SET `ScriptName`='npc_felfire_munitions_crate_hfa' WHERE `entry`=90610;

-- Hulking Berserker used as assault wave melee (keeps existing script binding)
UPDATE `creature_template` SET `ScriptName`='npc_hellfire_grunt_hfa' WHERE `entry`=93858;
