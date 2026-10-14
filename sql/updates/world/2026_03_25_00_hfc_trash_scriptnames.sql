-- Hellfire Citadel trash ScriptNames
-- Encounters / trash: https://www.wowhead.com/zone=7545/hellfire-citadel
-- Guides: Kormrok + High Council + Zakuun strategy (Wowhead)

-- Antechamber / Passage
UPDATE `creature_template` SET `ScriptName`='npc_hfc_shambling_hulk' WHERE `entry`=94804;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_fel_hellweaver' WHERE `entry`=94806;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_hellfire_guardian' WHERE `entry` IN (94808, 94985);
UPDATE `creature_template` SET `ScriptName`='npc_hfc_gorebound_assassin' WHERE `entry`=94832;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_fel_touched_seer' WHERE `entry`=94916;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_iron_peon' WHERE `entry`=94924;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_fel_starved_trainee' WHERE `entry`=94947;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_graggra' WHERE `entry`=94995;

-- Pits of Mannoroth
UPDATE `creature_template` SET `ScriptName`='npc_hfc_pit_magnaron' WHERE `entry` IN (94777, 94779, 94816);
UPDATE `creature_template` SET `ScriptName`='npc_hfc_fel_extractor' WHERE `entry`=94285;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_shadow_infuser' WHERE `entry`=94283;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_fiery_enkindler' WHERE `entry`=94284;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_armored_skullsmasher' WHERE `entry`=94859;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_grim_ambusher' WHERE `entry`=94861;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_keen_eyed_gronnstalker' WHERE `entry`=94894;

-- Bastion of Shadows
UPDATE `creature_template` SET `ScriptName`='npc_hfc_delusional_zealot' WHERE `entry`=93955;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_corrupted_talonpriest' WHERE `entry`=93985;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_shadowfel_warden' WHERE `entry`=93968;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_korvos' WHERE `entry`=95642;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_adjunct_kuroh' WHERE `entry`=91520;

-- Destructor's Rise approach
UPDATE `creature_template` SET `ScriptName`='npc_hfc_mekbarash' WHERE `entry`=92554;
UPDATE `creature_template` SET `ScriptName`='npc_hfc_named_felguard' WHERE `entry` IN (92526, 92523, 92514);
UPDATE `creature_template` SET `ScriptName`='npc_hfc_succubus_temptress' WHERE `entry` IN (94617, 94618, 94724);
