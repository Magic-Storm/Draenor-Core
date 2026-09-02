-- Client 6.2.4.21742 Battle.net / dual-socket login. Existing accounts are preserved.

ALTER TABLE `account`
  ADD COLUMN `last_attempt_ip` varchar(15) NOT NULL DEFAULT '127.0.0.1' AFTER `last_ip`,
  ADD COLUMN `battlenet_account` int(10) unsigned DEFAULT NULL AFTER `recruiter`,
  ADD COLUMN `battlenet_index` tinyint(3) unsigned DEFAULT NULL AFTER `battlenet_account`;

ALTER TABLE `account`
  MODIFY `sessionkey` varchar(80) NOT NULL DEFAULT '';

ALTER TABLE `battlenet_accounts`
  ADD COLUMN `LastCharacterUndelete` int(10) unsigned NOT NULL DEFAULT '0';

CREATE TABLE IF NOT EXISTS `battlenet_account_bans` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'Account identifier',
  `bandate` int(10) unsigned NOT NULL DEFAULT '0',
  `unbandate` int(10) unsigned NOT NULL DEFAULT '0',
  `bannedby` varchar(50) NOT NULL,
  `banreason` varchar(255) NOT NULL,
  PRIMARY KEY (`id`,`bandate`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Ban List';

ALTER TABLE `realmlist`
  ADD COLUMN `localAddress` varchar(255) NOT NULL DEFAULT '127.0.0.1' AFTER `address`,
  ADD COLUMN `localSubnetMask` varchar(255) NOT NULL DEFAULT '255.255.255.0' AFTER `localAddress`,
  ADD COLUMN `Region` tinyint(3) unsigned NOT NULL DEFAULT '2',
  ADD COLUMN `Battlegroup` tinyint(3) unsigned NOT NULL DEFAULT '1';

UPDATE `realmlist` SET `gamebuild`=21742;
