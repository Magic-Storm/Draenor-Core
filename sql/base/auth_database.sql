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

-- Battlenet account system
CREATE TABLE IF NOT EXISTS `battlenet_accounts` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `email` varchar(320) NOT NULL,
  `sha_pass_hash` varchar(64) NOT NULL,
  `joindate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `last_ip` varchar(15) NOT NULL DEFAULT '127.0.0.1',
  `failed_logins` int(10) unsigned NOT NULL DEFAULT '0',
  `locked` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `lock_country` varchar(2) NOT NULL DEFAULT '00',
  `last_login` timestamp NULL DEFAULT NULL,
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `locale` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `os` varchar(4) NOT NULL DEFAULT 'Win',
  PRIMARY KEY (`id`),
  UNIQUE KEY `email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Battlenet accounts';

-- Game accounts linked to battlenet accounts
CREATE TABLE IF NOT EXISTS `battlenet_account_gameaccounts` (
  `battlenetAccountId` int(10) unsigned NOT NULL,
  `gameAccountId` int(10) unsigned NOT NULL,
  PRIMARY KEY (`battlenetAccountId`, `gameAccountId`),
  KEY `fk_battlenet_account_gameaccounts_gameaccount` (`gameAccountId`),
  CONSTRAINT `fk_battlenet_account_gameaccounts_battlenet` FOREIGN KEY (`battlenetAccountId`) REFERENCES `battlenet_accounts` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_battlenet_account_gameaccounts_gameaccount` FOREIGN KEY (`gameAccountId`) REFERENCES `account` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Battlenet account game account links';

-- Account last played character
CREATE TABLE IF NOT EXISTS `account_last_played_character` (
  `accountId` int(10) unsigned NOT NULL,
  `region` tinyint(3) unsigned NOT NULL,
  `battlegroup` tinyint(3) unsigned NOT NULL,
  `realmId` int(10) unsigned,
  `characterName` varchar(12),
  `characterGUID` bigint(20) unsigned,
  `lastPlayedTime` int(10) unsigned,
  PRIMARY KEY(`accountId`, `region`, `battlegroup`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Update realmlist for 6.2.4
UPDATE `realmlist` SET `gamebuild`=21355 WHERE `gamebuild`=20726;
