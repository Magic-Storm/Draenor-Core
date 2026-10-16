-- Alliance Menagerie: Lio the Lioness was missing daily quest starters/enders.
-- Horde Serr'ah (79858) already has Scrappin' (36662) + Mastering the Menagerie (37645).
-- Wowhead: L1–2 daily = Battle Pet Roundup (36483); L3 daily = Mastering the Menagerie (37644).

DELETE FROM `creature_queststarter` WHERE `id` = 85418 AND `quest` IN (36483, 37644);
INSERT INTO `creature_queststarter` (`id`, `quest`) VALUES
(85418, 36483),
(85418, 37644);

DELETE FROM `creature_questender` WHERE `id` = 85418 AND `quest` = 37644;
INSERT INTO `creature_questender` (`id`, `quest`) VALUES
(85418, 37644);
