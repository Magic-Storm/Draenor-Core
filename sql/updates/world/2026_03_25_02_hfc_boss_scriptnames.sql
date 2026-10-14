-- Hellfire Citadel: ScriptName for all bosses and encounter adds
-- Map 1448 — https://www.wowhead.com/zone=7545/hellfire-citadel

-- ========== Bosses ==========
UPDATE `creature_template` SET `ScriptName`='boss_martak_hfc' WHERE `entry`=95068;
UPDATE `creature_template` SET `ScriptName`='boss_iron_reaver_hfc' WHERE `entry`=90284;
UPDATE `creature_template` SET `ScriptName`='boss_kormrok_hfc' WHERE `entry`=90776;
UPDATE `creature_template` SET `ScriptName`='boss_dia_darkwhisper' WHERE `entry`=92144;
UPDATE `creature_template` SET `ScriptName`='boss_gurtogg_bloodboil' WHERE `entry`=92146;
UPDATE `creature_template` SET `ScriptName`='boss_jubeithos' WHERE `entry`=92142;
UPDATE `creature_template` SET `ScriptName`='boss_kilrogg_deadeye' WHERE `entry`=90378;
UPDATE `creature_template` SET `ScriptName`='boss_gorefiend_hfc' WHERE `entry`=91809;
UPDATE `creature_template` SET `ScriptName`='boss_shadow_lord_iskar' WHERE `entry`=90316;
UPDATE `creature_template` SET `ScriptName`='boss_socrethar' WHERE `entry`=92330;
UPDATE `creature_template` SET `ScriptName`='boss_velhari_hfc' WHERE `entry`=93439;
UPDATE `creature_template` SET `ScriptName`='boss_zakuun_hfc' WHERE `entry`=89890;
UPDATE `creature_template` SET `ScriptName`='boss_xhulhorac_hfc' WHERE `entry`=93068;
UPDATE `creature_template` SET `ScriptName`='boss_mannoroth_hfc' WHERE `entry`=91349;
UPDATE `creature_template` SET `ScriptName`='boss_archimonde_hfc' WHERE `entry`=91331;

-- ========== Hellfire Assault adds ==========
UPDATE `creature_template` SET `ScriptName`='npc_hellfire_cannon_hfa' WHERE `entry`=90018;
UPDATE `creature_template` SET `ScriptName`='npc_hellfire_engineer_hfa' WHERE `entry`=93856;
UPDATE `creature_template` SET `ScriptName`='npc_felfire_munitions_crate_hfa' WHERE `entry`=93846;
UPDATE `creature_template` SET `ScriptName`='npc_hellfire_grunt_hfa' WHERE `entry`=93858;

-- ========== High Council ==========
UPDATE `creature_template` SET `ScriptName`='npc_jubeithos_mirror_image' WHERE `entry`=92143;

-- ========== Kilrogg ==========
UPDATE `creature_template` SET `ScriptName`='npc_drooling_bloodsucker' WHERE `entry`=93369;
UPDATE `creature_template` SET `ScriptName`='npc_massive_horror' WHERE `entry`=90523;
UPDATE `creature_template` SET `ScriptName`='npc_vision_of_death_demon' WHERE `entry` IN (90407, 90411, 90414);

-- ========== Gorefiend ==========
UPDATE `creature_template` SET `ScriptName`='npc_gorebound_spirit' WHERE `entry`=91812;
UPDATE `creature_template` SET `ScriptName`='npc_tortured_essence' WHERE `entry`=93145;

-- ========== Iskar ==========
UPDATE `creature_template` SET `ScriptName`='npc_fel_raven' WHERE `entry`=91263;
UPDATE `creature_template` SET `ScriptName`='npc_corrupted_felblade_raven' WHERE `entry`=91266;

-- ========== Socrethar ==========
UPDATE `creature_template` SET `ScriptName`='npc_soulbound_construct' WHERE `entry`=90296;
UPDATE `creature_template` SET `ScriptName`='npc_sargerei_spiritbinder' WHERE `entry`=92510;
UPDATE `creature_template` SET `ScriptName`='npc_sargerei_dominator' WHERE `entry`=92507;

-- ========== Velhari ==========
UPDATE `creature_template` SET `ScriptName`='npc_velhari_anima_fountain' WHERE `entry`=93569;
UPDATE `creature_template` SET `ScriptName`='npc_velhari_sanguine_font' WHERE `entry`=93570;

-- ========== Zakuun ==========
UPDATE `creature_template` SET `ScriptName`='npc_zakuun_residual_energy' WHERE `entry`=90294;
UPDATE `creature_template` SET `ScriptName`='npc_zakuun_fel_crystal' WHERE `entry`=90305;

-- ========== Xhul'horac ==========
UPDATE `creature_template` SET `ScriptName`='npc_xhulhorac_omnus' WHERE `entry`=93344;
UPDATE `creature_template` SET `ScriptName`='npc_xhulhorac_unstable_voidfiend' WHERE `entry`=93071;

-- ========== Mannoroth ==========
UPDATE `creature_template` SET `ScriptName`='boss_fel_iron_summoner' WHERE `entry`=91305;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_guldan_mann' WHERE `entry`=91326;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_fel_red_spire_mann' WHERE `entry`=95851;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_doom_lord_mann' WHERE `entry`=91241;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_fel_imp_mann' WHERE `entry`=91259;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_dread_infernal_mann' WHERE `entry`=91270;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_khadgar_mann' WHERE `entry`=95659;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_khadgar_portal_mann' WHERE `entry`=95699;

-- ========== Archimonde ==========
UPDATE `creature_template` SET `ScriptName`='npc_citadel_the_black_gate' WHERE `entry`=98232;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_guldan' WHERE `entry`=93926;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_khadgar_fight' WHERE `entry`=94142;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_yrel_fight' WHERE `entry`=92769;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_grommash_fight' WHERE `entry`=93618;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_doomfire_spirit' WHERE `entry`=92208;
UPDATE `creature_template` SET `ScriptName`='npc_citadel_doomfire_stalker' WHERE `entry`=92442;

-- ========== Spell scripts ==========
DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
	'spell_kilrogg_vision_of_death','spell_kilrogg_heartseeker','spell_kilrogg_corruption_sickness',
	'spell_kilrogg_immortal_resolve','spell_kilrogg_immortal_salvation','spell_kilrogg_cleansing_aura',
	'spell_eye_of_anzu','spell_velhari_seal_of_decay','spell_velhari_edict_of_condemnation',
	'spell_velhari_ancient_corruption','spell_zakuun_soul_cleave','spell_zakuun_seed_of_destruction',
	'spell_xhulhorac_chains_of_fel','spell_citadel_curse_of_the_legion','spell_citadel_mannoroth_gaze',
	'spell_citadel_mannoroth_gaze_damage','spell_citadel_spire_channel','spell_citadel_spire_channel_perma',
	'spell_citadel_spire_beam','spell_citadel_summoner_beam'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(182428, 'spell_kilrogg_vision_of_death'),
(180372, 'spell_kilrogg_heartseeker'),
(182159, 'spell_kilrogg_corruption_sickness'),
(180718, 'spell_kilrogg_immortal_resolve'),
(185563, 'spell_kilrogg_immortal_salvation'),
(187089, 'spell_kilrogg_cleansing_aura'),
(179202, 'spell_eye_of_anzu'),
(180353, 'spell_velhari_seal_of_decay'),
(182884, 'spell_velhari_edict_of_condemnation'),
(180351, 'spell_velhari_ancient_corruption'),
(179406, 'spell_zakuun_soul_cleave'),
(181508, 'spell_zakuun_seed_of_destruction'),
(186500, 'spell_xhulhorac_chains_of_fel');

-- GameObject
UPDATE `gameobject_template` SET `ScriptName`='go_citadel_mannoroth_platform_fix' WHERE `entry`=700000;
