-- Kilrogg Deadeye adds.
-- Live encounter-frame IDs (BigWigs KilroggDeadeye.lua) and Wowhead:
--   90521 Salivating Bloodthirster — walks to the fel pool
--   93369 Hulking Terror — the add it becomes (Savage Strikes, Rending Howl, Fel Rupture)
-- 93369 was previously bound as the walker, and 90523 as the terror. That swapped the two roles.

UPDATE `creature_template` SET `ScriptName` = 'npc_drooling_bloodsucker' WHERE `entry` = 90521;
UPDATE `creature_template` SET `ScriptName` = 'npc_massive_horror' WHERE `entry` IN (93369, 90523);
