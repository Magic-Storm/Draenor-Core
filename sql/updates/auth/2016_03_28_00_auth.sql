-- Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
-- Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by the
-- Free Software Foundation; either version 2 of the License, or (at your
-- option) any later version.
--
-- This program is distributed in the hope that it will be useful, but WITHOUT
-- ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
-- FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
-- more details.
--
-- You should have received a copy of the GNU General Public License along
-- with this program. If not, see <http://www.gnu.org/licenses/>.

-- Remove old battlenet tables
DROP TABLE IF EXISTS `battlenet_components`;
DROP TABLE IF EXISTS `battlenet_modules`;

-- Create account last played character table
DROP TABLE IF EXISTS `account_last_played_character`;
CREATE TABLE `account_last_played_character` (
  `accountId` int(10) unsigned NOT NULL,
  `region` tinyint(3) unsigned NOT NULL,
  `battlegroup` tinyint(3) unsigned NOT NULL,
  `realmId` int(10) unsigned,
  `characterName` varchar(12),
  `characterGUID` bigint(20) unsigned,
  `lastPlayedTime` int(10) unsigned,
  PRIMARY KEY(`accountId`, `region`, `battlegroup`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Update battlenet_accounts table
ALTER TABLE `battlenet_accounts`
  DROP `s`,
  DROP `v`,
  DROP `sessionKey`;

-- Update realmlist for 6.2.4
UPDATE `realmlist` SET `gamebuild`=21355 WHERE `gamebuild`=20726;
