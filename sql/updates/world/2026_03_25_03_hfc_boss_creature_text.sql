-- Hellfire Citadel boss creature_text (remaining encounters)
-- Iron Reaver (90284) already has texts+sounds in base DB — do not overwrite.

-- Velhari 93439 — groups match eTalks in boss_velhari_hfc.cpp
DELETE FROM `creature_text` WHERE `entry`=93439;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(93439,0,0,'You will suffer for your insolence!',14,0,100,0,0,0,'Velhari - Aggro'),
(93439,1,0,'Pathetic.',14,0,100,0,0,0,'Velhari - Slay'),
(93439,1,1,'Another soul for the Legion.',14,0,100,0,0,0,'Velhari - Slay'),
(93439,2,0,'The... void... claims me...',14,0,100,0,0,0,'Velhari - Death'),
(93439,3,0,'Feel my malice!',14,0,100,0,0,0,'Velhari - Phase2'),
(93439,4,0,'There is no escape from oppression!',14,0,100,0,0,0,'Velhari - Phase3'),
(93439,5,0,'The weak perish. As it should be.',14,0,100,0,0,0,'Velhari - Wipe');

-- Zakuun 89890
DELETE FROM `creature_text` WHERE `entry`=89890;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(89890,0,0,'Your flesh will break!',14,0,100,0,0,0,'Zakuun - Aggro'),
(89890,1,0,'Broken!',14,0,100,0,0,0,'Zakuun - Slay'),
(89890,2,0,'I... am... broken...',14,0,100,0,0,0,'Zakuun - Death'),
(89890,3,0,'The axe hungers!',14,0,100,0,0,0,'Zakuun - Phase'),
(89890,4,0,'ENOUGH!',14,0,100,0,0,0,'Zakuun - Enrage'),
(89890,5,0,'You are not worthy.',14,0,100,0,0,0,'Zakuun - Wipe');

-- Xhul'horac 93068
DELETE FROM `creature_text` WHERE `entry`=93068;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(93068,0,0,'The realms merge... you die!',14,0,100,0,0,0,'Xhul - Aggro'),
(93068,1,0,'Consumed!',14,0,100,0,0,0,'Xhul - Slay'),
(93068,2,0,'The breach... closes...',14,0,100,0,0,0,'Xhul - Death'),
(93068,3,0,'Fel fire rains!',14,0,100,0,0,0,'Xhul - Fel phase'),
(93068,4,0,'Void claims all!',14,0,100,0,0,0,'Xhul - Void phase'),
(93068,5,0,'Fel and void as one!',14,0,100,0,0,0,'Xhul - Combined'),
(93068,6,0,'You cannot seal the breach.',14,0,100,0,0,0,'Xhul - Wipe');

-- Kilrogg 90378
DELETE FROM `creature_text` WHERE `entry`=90378;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(90378,0,0,'I have seen your death!',14,0,100,0,0,0,'Kilrogg - Aggro'),
(90378,1,0,'Just as I foresaw.',14,0,100,0,0,0,'Kilrogg - Slay'),
(90378,2,0,'My... destiny...',14,0,100,0,0,0,'Kilrogg - Death'),
(90378,3,0,'Gaze into your doom!',14,0,100,0,0,0,'Kilrogg - Vision'),
(90378,4,0,'Your fate is sealed.',14,0,100,0,0,0,'Kilrogg - Wipe');

-- Gorefiend 91809
DELETE FROM `creature_text` WHERE `entry`=91809;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(91809,0,0,'Your souls will feed me!',14,0,100,0,0,0,'Gorefiend - Aggro'),
(91809,1,0,'Delicious...',14,0,100,0,0,0,'Gorefiend - Slay'),
(91809,2,0,'No... more... hunger...',14,0,100,0,0,0,'Gorefiend - Death'),
(91809,3,0,'Enter my realm!',14,0,100,0,0,0,'Gorefiend - Shadow of Death'),
(91809,4,0,'I will feast again.',14,0,100,0,0,0,'Gorefiend - Wipe');

-- Iskar 90316
DELETE FROM `creature_text` WHERE `entry`=90316;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(90316,0,0,'The Eye of Anzu watches!',14,0,100,0,0,0,'Iskar - Aggro'),
(90316,1,0,'Fall from the sky!',14,0,100,0,0,0,'Iskar - Slay'),
(90316,2,0,'Sethe... claims me...',14,0,100,0,0,0,'Iskar - Death'),
(90316,3,0,'Phantoms of the wind!',14,0,100,0,0,0,'Iskar - Ability'),
(90316,4,0,'You cannot catch what you cannot see.',14,0,100,0,0,0,'Iskar - Wipe');

-- Socrethar 92330
DELETE FROM `creature_text` WHERE `entry`=92330;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(92330,0,0,'The construct will be your end!',14,0,100,0,0,0,'Socrethar - Aggro'),
(92330,1,0,'Another for the Eternal!',14,0,100,0,0,0,'Socrethar - Slay'),
(92330,2,0,'I... endure...',14,0,100,0,0,0,'Socrethar - Death'),
(92330,3,0,'Apocalypse!',14,0,100,0,0,0,'Socrethar - Ability'),
(92330,4,0,'The Sargerei prevail.',14,0,100,0,0,0,'Socrethar - Wipe');

-- Mannoroth 91349 — groups match eTalks in boss_mannoroth_hfc (TalkPhase2=0..TalkPhase4b=3, TalkSlay=14, TalkDeath=15, TalkWipe=16)
DELETE FROM `creature_text` WHERE `entry`=91349;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(91349,0,0,'The Legion rises!',14,0,100,0,0,0,'Mannoroth - Phase2'),
(91349,1,0,'Feel my wrath!',14,0,100,0,0,0,'Mannoroth - Phase3'),
(91349,2,0,'More blood!',14,0,100,0,0,0,'Mannoroth - Phase4'),
(91349,3,0,'I am unbound!',14,0,100,0,0,0,'Mannoroth - Phase4b'),
(91349,14,0,'Crush!',14,0,100,0,0,0,'Mannoroth - Slay'),
(91349,15,0,'Impossible...',14,0,100,0,0,0,'Mannoroth - Death'),
(91349,16,0,'You cannot stop the Legion.',14,0,100,0,0,0,'Mannoroth - Wipe');

-- Archimonde 91331 — groups aligned to eTalks order in boss_archimonde_hfc.cpp
DELETE FROM `creature_text` WHERE `entry`=91331;
INSERT INTO `creature_text` (`entry`,`groupid`,`id`,`text`,`type`,`language`,`probability`,`emote`,`duration`,`sound`,`comment`) VALUES
(91331,0,0,'The time has come.',14,0,100,0,0,0,'Archimonde - Intro0'),
(91331,1,0,'Gul''dan... you have failed me for the last time.',14,0,100,0,0,0,'Archimonde - Intro2'),
(91331,2,0,'I am the end of all things!',14,0,100,0,0,0,'Archimonde - Aggro'),
(91331,3,0,'Doomfire!',14,0,100,0,0,0,'Archimonde - Doomfire'),
(91331,4,0,'Marked for death!',14,0,100,0,0,0,'Archimonde - Death Brand'),
(91331,5,0,'Shadow and fel!',14,0,100,0,0,0,'Archimonde - Shadowfel Burst'),
(91331,6,0,'Allure of flames!',14,0,100,0,0,0,'Archimonde - Allure'),
(91331,7,0,'You are nothing!',14,0,100,0,0,0,'Archimonde - 85%'),
(91331,8,0,'Burn!',14,0,100,0,0,0,'Archimonde - 75%'),
(91331,9,0,'The Legion arrives!',14,0,100,0,0,0,'Archimonde - Phase2'),
(91331,10,0,'Shackled!',14,0,100,0,0,0,'Archimonde - Shackled Torment'),
(91331,11,0,'Chaos unbound!',14,0,100,0,0,0,'Archimonde - Wrought Chaos'),
(91331,12,0,'Fall before me!',14,0,100,0,0,0,'Archimonde - 55%'),
(91331,13,0,'The Twisting Nether opens!',14,0,100,0,0,0,'Archimonde - Phase3'),
(91331,14,0,'Kneel!',14,0,100,0,0,0,'Archimonde - 22%'),
(91331,15,0,'Banished!',14,0,100,0,0,0,'Archimonde - Nether Banish'),
(91331,16,0,'Perish!',14,0,100,0,0,0,'Archimonde - Slay'),
(91331,17,0,'Rain of Chaos!',14,0,100,0,0,0,'Archimonde - Rain of Chaos'),
(91331,18,0,'Corruption spreads!',14,0,100,0,0,0,'Archimonde - Seething'),
(91331,19,0,'Grommash... pathetic.',14,0,100,0,0,0,'Archimonde - Grommash'),
(91331,20,0,'Marked by the Legion!',14,0,100,0,0,0,'Archimonde - Mark'),
(91331,21,0,'Source of Chaos!',14,0,100,0,0,0,'Archimonde - Source'),
(91331,22,0,'I ascend!',14,0,100,0,0,0,'Archimonde - Nether Ascension'),
(91331,23,0,'As I foresaw.',14,0,100,0,0,0,'Archimonde - Wipe'),
(91331,24,0,'Draenor will burn.',14,0,100,0,0,0,'Archimonde - Intro1');
