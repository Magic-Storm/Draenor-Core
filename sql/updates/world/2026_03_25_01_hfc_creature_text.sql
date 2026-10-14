-- Hellfire Citadel creature_text (boss + trash)
-- Quotes: https://warcraft.wiki.gg/wiki/Hellfire_Assault
--          https://warcraft.wiki.gg/wiki/Kormrok
--          https://warcraft.wiki.gg/wiki/Hellfire_High_Council
-- type: 12 = SAY, 14 = YELL

-- ========== Siegemaster Mar'tak 95068 ==========
-- group: 0 Aggro, 1 Wave, 2 Slay, 3 Wipe, 4 Death/Flee
DELETE FROM `creature_text` WHERE `entry` = 95068;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(95068, 0, 0, 'So, you wish to test the might of the Fel Horde? Come, warriors, show them how we treat unwanted guests.', 14, 0, 100, 0, 0, 0, 'Mar''tak - Aggro'),
(95068, 1, 0, 'Fire in the hole!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 1, 1, 'Comin'' in hot!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 1, 2, 'Run them down!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 1, 3, 'Burn them to the ground.', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 1, 4, 'Send in the crusher!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 1, 5, 'Cannons on the way!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 1, 6, 'Strike them down!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wave'),
(95068, 2, 0, 'Hah-ha! Trample their corpses!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Slay'),
(95068, 2, 1, 'For Gul''dan!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Slay'),
(95068, 3, 0, 'We are victorious!', 14, 0, 100, 0, 0, 0, 'Mar''tak - Wipe'),
(95068, 4, 0, 'I''ll be back...', 14, 0, 100, 0, 0, 0, 'Mar''tak - Flee/Death');

-- ========== Kormrok 90776 ==========
-- group: 0 Aggro, 1 Leap, 2 Slay, 3 Wipe, 4 Death
DELETE FROM `creature_text` WHERE `entry` = 90776;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(90776, 0, 0, 'Kill!', 14, 0, 100, 0, 0, 0, 'Kormrok - Aggro'),
(90776, 1, 0, 'Destroy!', 14, 0, 100, 0, 0, 0, 'Kormrok - Leap'),
(90776, 1, 1, 'None escape!', 14, 0, 100, 0, 0, 0, 'Kormrok - Leap'),
(90776, 2, 0, 'Fragile!', 14, 0, 100, 0, 0, 0, 'Kormrok - Slay'),
(90776, 2, 1, 'Broken!', 14, 0, 100, 0, 0, 0, 'Kormrok - Slay'),
(90776, 3, 0, 'Weak!', 14, 0, 100, 0, 0, 0, 'Kormrok - Wipe'),
(90776, 4, 0, 'I... am... unmade...', 14, 0, 100, 0, 0, 0, 'Kormrok - Death');

-- ========== Dia Darkwhisper 92144 ==========
-- group: 0 Aggro, 1 Slay, 2 Death, 3 Ability
DELETE FROM `creature_text` WHERE `entry` = 92144;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(92144, 0, 0, 'Gurtogg! Dispatch our enemies...', 14, 0, 100, 0, 0, 0, 'Dia - Aggro'),
(92144, 1, 0, 'How it struggles when death is near!', 14, 0, 100, 0, 0, 0, 'Dia - Slay'),
(92144, 1, 1, 'It has fallen. It is mine now.', 14, 0, 100, 0, 0, 0, 'Dia - Slay'),
(92144, 2, 0, 'The sweet mercy... of death.', 14, 0, 100, 0, 0, 0, 'Dia - Death'),
(92144, 3, 0, 'They have been marked.', 14, 0, 100, 0, 0, 0, 'Dia - Mark/Ability'),
(92144, 3, 1, 'The void, it hungers!', 14, 0, 100, 0, 0, 0, 'Dia - Mark/Ability'),
(92144, 3, 2, 'And then the dead shall rise once more.', 14, 0, 100, 0, 0, 0, 'Dia - Wailing Horror');

-- ========== Gurtogg Bloodboil 92146 ==========
-- group: 0 Aggro, 1 Slay, 2 Death, 3 Fel Rage, 4 Ability
DELETE FROM `creature_text` WHERE `entry` = 92146;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(92146, 0, 0, 'Yes, Dark Lady.', 14, 0, 100, 0, 0, 0, 'Gurtogg - Aggro'),
(92146, 1, 0, 'Time to feast!', 14, 0, 100, 0, 0, 0, 'Gurtogg - Slay'),
(92146, 1, 1, 'Crush you!', 14, 0, 100, 0, 0, 0, 'Gurtogg - Slay'),
(92146, 2, 0, 'ArRRgghhHhh...', 14, 0, 100, 0, 0, 0, 'Gurtogg - Death'),
(92146, 3, 0, 'Drink your blood, eat your flesh!', 14, 0, 100, 0, 0, 0, 'Gurtogg - Fel Rage'),
(92146, 3, 1, 'I''ll rip the meat from your bones!', 14, 0, 100, 0, 0, 0, 'Gurtogg - Fel Rage'),
(92146, 4, 0, 'Iron Horde will CRUSH you!', 14, 0, 100, 0, 0, 0, 'Gurtogg - Leap/Ability'),
(92146, 4, 1, 'More! I want MORE!', 14, 0, 100, 0, 0, 0, 'Gurtogg - Tainted Blood');

-- ========== Blademaster Jubei'thos 92142 ==========
-- group: 0 Mirror/Felstorm, 1 Slay, 2 Death
DELETE FROM `creature_text` WHERE `entry` = 92142;
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(92142, 0, 0, 'Burn... BURN!', 14, 0, 100, 0, 0, 0, 'Jubei''thos - Mirror/Felstorm'),
(92142, 1, 0, 'To ash...', 14, 0, 100, 0, 0, 0, 'Jubei''thos - Slay'),
(92142, 1, 1, 'To cinders!', 14, 0, 100, 0, 0, 0, 'Jubei''thos - Slay'),
(92142, 2, 0, 'I am everburning!', 14, 0, 100, 0, 0, 0, 'Jubei''thos - Death');

-- ========== Antechamber trash ==========
DELETE FROM `creature_text` WHERE `entry` IN (94806, 94808, 94985, 94832, 94916, 94924, 94804, 94947, 94995);
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
-- Fel Hellweaver 94806: 0 Aggro, 1 Death
(94806, 0, 0, 'Burn in fel fire!', 14, 0, 100, 0, 0, 0, 'Fel Hellweaver - Aggro'),
(94806, 1, 0, 'The Legion... claims me...', 14, 0, 100, 0, 0, 0, 'Fel Hellweaver - Death'),
-- Hellfire Guardian 94808 / 94985
(94808, 0, 0, 'None shall pass!', 14, 0, 100, 0, 0, 0, 'Hellfire Guardian - Aggro'),
(94808, 1, 0, 'For the Fel Horde!', 14, 0, 100, 0, 0, 0, 'Hellfire Guardian - Death'),
(94985, 0, 0, 'None shall pass!', 14, 0, 100, 0, 0, 0, 'Hellfire Guardian - Aggro'),
(94985, 1, 0, 'For the Fel Horde!', 14, 0, 100, 0, 0, 0, 'Hellfire Guardian - Death'),
-- Gorebound Assassin 94832
(94832, 0, 0, 'Your blood is mine!', 12, 0, 100, 0, 0, 0, 'Gorebound Assassin - Aggro'),
(94832, 1, 0, '...silence.', 12, 0, 100, 0, 0, 0, 'Gorebound Assassin - Death'),
-- Fel Touched Seer 94916
(94916, 0, 0, 'The fel mends all wounds!', 14, 0, 100, 0, 0, 0, 'Fel Touched Seer - Aggro'),
(94916, 1, 0, 'My vision... fades...', 14, 0, 100, 0, 0, 0, 'Fel Touched Seer - Death'),
-- Iron Peon 94924
(94924, 0, 0, 'Work harder! Fight harder!', 12, 0, 100, 0, 0, 0, 'Iron Peon - Aggro'),
(94924, 1, 0, 'Ow!', 12, 0, 100, 0, 0, 0, 'Iron Peon - Death'),
-- Shambling Hulk 94804
(94804, 0, 0, 'RRAAAAGH!', 14, 0, 100, 0, 0, 0, 'Shambling Hulk - Aggro'),
(94804, 1, 0, '...', 14, 0, 100, 0, 0, 0, 'Shambling Hulk - Death'),
-- Fel-Starved Trainee 94947
(94947, 0, 0, 'For blood and glory!', 14, 0, 100, 0, 0, 0, 'Fel-Starved Trainee - Aggro'),
(94947, 1, 0, 'I... failed...', 14, 0, 100, 0, 0, 0, 'Fel-Starved Trainee - Death'),
-- Graggra 94995
(94995, 0, 0, 'GRAAAGH!', 14, 0, 100, 0, 0, 0, 'Graggra - Aggro'),
(94995, 1, 0, 'Smash!', 14, 0, 100, 0, 0, 0, 'Graggra - Smash'),
(94995, 2, 0, '...', 14, 0, 100, 0, 0, 0, 'Graggra - Death');

-- ========== Pits of Mannoroth named trash ==========
DELETE FROM `creature_text` WHERE `entry` IN (94816, 94779, 94777, 94285, 94283, 94284);
INSERT INTO `creature_text` (`entry`, `groupid`, `id`, `text`, `type`, `language`, `probability`, `emote`, `duration`, `sound`, `comment`) VALUES
(94816, 0, 0, 'Crush!', 14, 0, 100, 0, 0, 0, 'Togdrov - Aggro'),
(94816, 1, 0, 'Broken...', 14, 0, 100, 0, 0, 0, 'Togdrov - Death'),
(94779, 0, 0, 'Drown!', 14, 0, 100, 0, 0, 0, 'Sovokk - Aggro'),
(94779, 1, 0, 'Unmade...', 14, 0, 100, 0, 0, 0, 'Sovokk - Death'),
(94777, 0, 0, 'Burn!', 14, 0, 100, 0, 0, 0, 'Morkronn - Aggro'),
(94777, 1, 0, 'Ashes...', 14, 0, 100, 0, 0, 0, 'Morkronn - Death'),
(94285, 0, 0, 'Your essence is mine!', 14, 0, 100, 0, 0, 0, 'Fel Extractor - Aggro'),
(94283, 0, 0, 'Shadows claim you!', 14, 0, 100, 0, 0, 0, 'Shadow Infuser - Aggro'),
(94284, 0, 0, 'Feel the flame!', 14, 0, 100, 0, 0, 0, 'Fiery Enkindler - Aggro');
