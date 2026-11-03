-- Draenor zone outposts. The drafting table opens the player-choice window
-- and the selected card teaches the matching zone ability.

UPDATE `gameobject_template` SET `ScriptName` = 'go_draenor_outpost_drafting_table' WHERE `entry` IN (230665, 231231, 232062, 232089);

UPDATE `creature_template` SET `ScriptName` = 'npc_draenor_outpost_choice'
WHERE `entry` IN (81119, 75136, 81588, 74594, 79133, 79176, 79807, 81918, 81919, 81920, 81929, 79674, 79576, 79679, 80003, 80143)
  AND (`ScriptName` = '' OR `ScriptName` IS NULL);

UPDATE `creature_template` SET `npcflag` = `npcflag` | 1, `unit_flags` = `unit_flags` & ~33554432
WHERE `entry` IN (81119, 79807, 81918, 81919, 79679, 80143);
