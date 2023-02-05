-- --------------------------------------------------------
-- Host:                         logon.hellscream.org
-- Server version:               5.5.9-log - MySQL Community Server (GPL)
-- Server OS:                    Win32
-- HeidiSQL Version:             10.3.0.5771
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Dumping structure for table worldpvp.spell_proc_event
CREATE TABLE IF NOT EXISTS `spell_proc_event` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SchoolMask` tinyint(4) NOT NULL DEFAULT '0',
  `SpellFamilyName` smallint(5) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask0` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask1` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask2` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellFamilyMask3` int(10) unsigned NOT NULL DEFAULT '0',
  `procFlags` int(10) unsigned NOT NULL DEFAULT '0',
  `procEx` int(10) unsigned NOT NULL DEFAULT '0',
  `ppmRate` float NOT NULL DEFAULT '0',
  `CustomChance` float NOT NULL DEFAULT '0',
  `Cooldown` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- Dumping data for table worldpvp.spell_proc_event: 570 rows
DELETE FROM `spell_proc_event`;
/*!40000 ALTER TABLE `spell_proc_event` DISABLE KEYS */;
INSERT INTO `spell_proc_event` (`entry`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`, `SpellFamilyMask3`, `procFlags`, `procEx`, `ppmRate`, `CustomChance`, `Cooldown`) VALUES
	(324, 0, 0, 0, 0, 0, 0, 139944, 0, 0, 100, 3),
	(974, 0, 0, 0, 0, 0, 0, 139944, 0, 0, 0, 3),
	(1463, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0),
	(6346, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0),
	(7383, 1, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0),
	(7434, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(8178, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(9782, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(9784, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(12169, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(13877, 0, 0, 0, 0, 0, 0, 12653588, 0, 0, 0, 0),
	(14190, 0, 8, 1082131720, 6, 0, 0, 0, 2, 0, 0, 1),
	(15088, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(15128, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(15277, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0),
	(15286, 32, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(15346, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0),
	(15600, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),
	(16196, 0, 0, 0, 0, 0, 0, 17408, 2, 0, 0, 0),
	(16282, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(16550, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(16620, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(16624, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(16864, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 6, 10),
	(155166, 0, 0, 0, 0, 0, 0, 327680, 0, 0, 0, 0),
	(17495, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(17619, 0, 13, 0, 0, 0, 0, 32768, 0, 0, 0, 0),
	(18820, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(19387, 0, 9, 16, 8192, 0, 0, 0, 0, 0, 0, 0),
	(19573, 0, 9, 8388608, 0, 0, 0, 16384, 0, 0, 0, 0),
	(20165, 0, 10, 0, 0, 0, 0, 20, 0, 20, 0, 0),
	(75475, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(20705, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(21185, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(21882, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(21890, 0, 4, 712396527, 876, 0, 0, 0, 0, 0, 0, 0),
	(22007, 0, 3, 2097185, 0, 0, 0, 0, 65536, 0, 0, 0),
	(22618, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(22648, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(23547, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0),
	(23548, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(23551, 0, 11, 192, 0, 0, 0, 0, 0, 0, 0, 0),
	(23552, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(23572, 0, 11, 192, 0, 0, 0, 0, 0, 0, 0, 0),
	(23578, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0),
	(23581, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0),
	(71519, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105),
	(23686, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0),
	(23688, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(23689, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0),
	(23721, 0, 9, 2048, 0, 0, 0, 0, 0, 0, 0, 0),
	(23920, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(24353, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(24389, 0, 3, 12582935, 64, 0, 0, 0, 0, 0, 0, 0),
	(24658, 0, 0, 0, 0, 0, 0, 82192, 0, 0, 0, 0),
	(24905, 0, 0, 0, 0, 0, 0, 0, 2, 15, 0, 0),
	(25050, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(25669, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),
	(25988, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(26016, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0),
	(26107, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0),
	(26119, 0, 10, 2416967683, 0, 0, 0, 0, 65536, 0, 0, 0),
	(26128, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0),
	(26135, 0, 10, 8388608, 0, 0, 0, 0, 65536, 0, 0, 0),
	(26480, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(26605, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(27419, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(27498, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(27521, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 15),
	(27656, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(27774, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(27787, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(28716, 0, 7, 16, 0, 0, 0, 262144, 0, 0, 0, 0),
	(28719, 0, 7, 32, 0, 0, 0, 0, 2, 0, 0, 0),
	(28744, 0, 7, 64, 0, 0, 0, 278528, 0, 0, 0, 0),
	(28752, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(28789, 0, 10, 3221225472, 0, 0, 0, 0, 0, 0, 0, 0),
	(28802, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(28809, 0, 6, 4096, 0, 0, 0, 0, 2, 0, 0, 0),
	(28812, 0, 8, 33554438, 0, 0, 0, 0, 2, 0, 0, 0),
	(28816, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(28823, 0, 11, 192, 0, 0, 0, 0, 0, 0, 0, 0),
	(29150, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29385, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0),
	(29455, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(29501, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29624, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29625, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29626, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29632, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29633, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29634, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29635, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29636, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29637, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(29838, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(29977, 0, 3, 12582935, 64, 0, 0, 0, 0, 0, 0, 0),
	(30003, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(30823, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0),
	(30884, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(30937, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(31394, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(31794, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(31904, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(32409, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(32587, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(32642, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(32734, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(32776, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(32777, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(32837, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 45),
	(32844, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0),
	(32885, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(33076, 0, 0, 0, 0, 0, 0, 664232, 0, 0, 0, 0),
	(33089, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(33127, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0),
	(60524, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(33297, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(33299, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(33510, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0),
	(33648, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(33719, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(33746, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(33759, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(33953, 0, 0, 0, 0, 0, 0, 278528, 0, 0, 0, 45),
	(34080, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0),
	(34320, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(34355, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(34584, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(34586, 0, 0, 0, 0, 0, 0, 0, 0, 1.5, 0, 0),
	(34598, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(34749, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0),
	(34774, 0, 0, 0, 0, 0, 0, 0, 0, 1.5, 0, 20),
	(34783, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(34827, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(34954, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(35077, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(35080, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 60),
	(35083, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(35086, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(35121, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(35551, 0, 0, 0, 0, 0, 0, 8388608, 0, 0, 100, 0),
	(36032, 0, 3, 4096, 32768, 0, 0, 0, 0, 0, 0, 0),
	(36096, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(36111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(36541, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(37165, 0, 8, 2098176, 0, 0, 0, 0, 0, 0, 0, 0),
	(37168, 0, 8, 4063232, 9, 0, 0, 0, 0, 0, 0, 0),
	(37170, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),
	(37173, 0, 8, 750519704, 262, 0, 0, 0, 0, 0, 0, 30),
	(37189, 0, 10, 3221225472, 0, 0, 0, 0, 2, 0, 0, 60),
	(37195, 0, 10, 8388608, 0, 0, 0, 0, 262144, 0, 0, 0),
	(37197, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 45),
	(37213, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(37214, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(37227, 0, 11, 448, 0, 0, 0, 0, 2, 0, 0, 60),
	(37237, 0, 11, 1, 0, 0, 0, 0, 2, 0, 0, 0),
	(37247, 8, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 45),
	(37377, 32, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(37379, 32, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(37384, 0, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0),
	(37443, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(37514, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0),
	(37516, 0, 4, 1024, 0, 0, 0, 0, 0, 0, 0, 0),
	(37519, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0),
	(37523, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(37528, 0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0),
	(37536, 0, 4, 65536, 0, 0, 0, 0, 0, 0, 0, 0),
	(37568, 0, 6, 2048, 0, 0, 0, 0, 0, 0, 0, 0),
	(37594, 0, 6, 4096, 0, 0, 0, 0, 0, 0, 0, 0),
	(37600, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(37601, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(37603, 0, 6, 32768, 0, 0, 0, 0, 0, 0, 0, 0),
	(37655, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(37657, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 3),
	(38026, 1, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0),
	(38031, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(38290, 0, 0, 0, 0, 0, 0, 0, 0, 1.6, 0, 0),
	(38299, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15),
	(38326, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(38327, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(38334, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(38347, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(38350, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(38394, 0, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0),
	(38857, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(39027, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(39372, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(39437, 4, 5, 4964, 192, 0, 0, 0, 65536, 0, 0, 0),
	(39442, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0),
	(39443, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(39530, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(39958, 0, 0, 0, 0, 0, 0, 0, 0, 0.7, 0, 40),
	(40407, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0),
	(40438, 0, 6, 32832, 0, 0, 0, 0, 0, 0, 0, 0),
	(40442, 0, 7, 20, 1088, 0, 0, 0, 0, 0, 0, 0),
	(40444, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(40458, 0, 4, 33554432, 1537, 0, 0, 0, 0, 0, 0, 0),
	(40463, 0, 11, 129, 16, 0, 0, 0, 0, 0, 0, 0),
	(40470, 0, 10, 3229614080, 0, 0, 0, 0, 262144, 0, 0, 0),
	(40475, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0),
	(40478, 0, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0),
	(40482, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(40485, 0, 9, 0, 1, 0, 0, 0, 0, 0, 0, 0),
	(40899, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(41034, 0, 0, 0, 0, 0, 0, 0, 1024, 0, 0, 0),
	(41260, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(41262, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(41381, 0, 0, 0, 0, 0, 0, 0, 256, 0, 0, 0),
	(41393, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0),
	(41434, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 45),
	(41469, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0),
	(41635, 0, 0, 0, 0, 0, 0, 2148180648, 0, 0, 0, 0),
	(41989, 0, 0, 0, 0, 0, 0, 0, 0, 0.5, 0, 0),
	(42083, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(42135, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90),
	(42136, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 90),
	(42770, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0),
	(43443, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(5227, 0, 0, 0, 0, 0, 0, 2429268, 0, 0, 20, 10),
	(104428, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 1),
	(44549, 0, 3, 736, 4096, 0, 0, 69632, 16384, 0, 15, 0),
	(44835, 0, 7, 0, 128, 0, 0, 16, 0, 0, 0, 0),
	(45054, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15),
	(45057, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(45354, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(45481, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(45482, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(45483, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(45484, 0, 0, 0, 0, 0, 0, 16384, 0, 0, 0, 45),
	(46025, 32, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(46569, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(46662, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20),
	(46832, 0, 0, 0, 0, 0, 0, 1712808, 0, 0, 100, 0),
	(46867, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(46915, 0, 4, 64, 1028, 0, 0, 0, 262144, 0, 0, 0),
	(20784, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(47515, 0, 0, 0, 0, 0, 0, 2376704, 0, 0, 0, 0),
	(47517, 0, 6, 6144, 65536, 0, 0, 0, 0, 0, 0, 0),
	(48484, 0, 7, 34816, 1088, 0, 0, 0, 0, 0, 0, 0),
	(48500, 0, 7, 96, 33554434, 0, 0, 0, 2, 0, 0, 0),
	(49222, 0, 0, 0, 0, 0, 0, 139944, 0, 0, 0, 2),
	(49530, 0, 15, 20971520, 0, 0, 0, 0, 0, 0, 0, 0),
	(49622, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(50781, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 6),
	(51128, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0),
	(51346, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(51349, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(51352, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(51359, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(51414, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(67209, 1, 8, 1048576, 0, 0, 0, 327680, 0, 0, 0, 0),
	(33757, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 4),
	(51530, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 55, 0),
	(51564, 0, 11, 256, 0, 16, 0, 0, 0, 0, 0, 0),
	(51626, 0, 8, 268476416, 0, 0, 0, 0, 0, 0, 0, 0),
	(51667, 0, 8, 131072, 8, 0, 0, 0, 0, 0, 0, 0),
	(51701, 0, 0, 0, 0, 0, 0, 2445588, 2, 0, 100, 2),
	(51915, 0, 0, 0, 0, 0, 0, 16777216, 0, 0, 100, 600),
	(52127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(52420, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(52423, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0),
	(52798, 0, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0),
	(52898, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(66192, 0, 15, 4194321, 537001988, 0, 0, 16, 0, 0, 100, 0),
	(53260, 0, 9, 2048, 8388609, 0, 0, 0, 2, 0, 0, 0),
	(53376, 0, 10, 0, 8192, 0, 0, 1024, 0, 0, 0, 6),
	(53397, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(53576, 0, 10, 1075838976, 65536, 0, 0, 0, 0, 0, 0, 0),
	(32216, 0, 4, 0, 256, 0, 0, 16, 0, 0, 0, 0),
	(63251, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(54695, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(54707, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(54738, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(54808, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(54838, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(54841, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 3),
	(69739, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(55380, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(55381, 0, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 15),
	(55440, 0, 11, 64, 0, 0, 0, 0, 0, 0, 0, 0),
	(55610, 0, 15, 0, 67108864, 0, 0, 4096, 0, 0, 0, 0),
	(55747, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(56218, 0, 5, 2, 0, 0, 0, 0, 0, 0, 0, 0),
	(56249, 0, 5, 0, 0, 1024, 0, 0, 0, 0, 0, 0),
	(56355, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0),
	(56364, 0, 3, 0, 16777216, 0, 0, 0, 0, 0, 0, 0),
	(56451, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3),
	(56611, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(56835, 0, 15, 4194304, 65536, 0, 0, 0, 0, 0, 0, 0),
	(57345, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(57352, 0, 0, 0, 0, 0, 0, 332116, 0, 0, 0, 45),
	(58372, 0, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0),
	(58386, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0),
	(58442, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15),
	(58444, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5),
	(58647, 0, 15, 0, 4, 0, 0, 0, 0, 0, 0, 0),
	(58677, 0, 15, 8192, 0, 0, 0, 0, 0, 0, 0, 0),
	(58901, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(59176, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(59327, 0, 15, 134217728, 0, 0, 0, 0, 0, 0, 0, 0),
	(59345, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(59630, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 35),
	(60061, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60063, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60132, 0, 15, 16, 134348800, 0, 0, 0, 0, 0, 0, 0),
	(60170, 0, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0),
	(60172, 0, 5, 262144, 0, 0, 0, 0, 0, 0, 0, 0),
	(60176, 0, 4, 32, 16, 0, 0, 0, 0, 0, 0, 0),
	(60221, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60301, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60306, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60317, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60436, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60442, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60473, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60482, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15),
	(60490, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60493, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60519, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60529, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60537, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(105574, 0, 0, 0, 0, 0, 0, 19223892, 0, 0, 35, 55),
	(61356, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 90),
	(61618, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(62147, 0, 15, 2, 0, 0, 0, 0, 0, 0, 0, 0),
	(62459, 0, 15, 4, 0, 0, 0, 0, 0, 0, 0, 0),
	(64914, 0, 8, 65536, 0, 0, 0, 0, 0, 0, 0, 0),
	(64938, 0, 4, 2097216, 0, 0, 0, 0, 2, 0, 0, 0),
	(65002, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(65005, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(64999, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0),
	(65007, 0, 0, 0, 0, 0, 0, 81920, 0, 0, 0, 0),
	(65013, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(65020, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(65025, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(64415, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(60066, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(62115, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(62114, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(166020, 0, 15, 33554432, 0, 0, 0, 0, 0, 0, 0, 0),
	(69755, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(63335, 0, 15, 0, 2, 0, 0, 0, 0, 0, 0, 0),
	(63733, 0, 6, 2048, 4, 0, 0, 0, 0, 0, 0, 0),
	(64571, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10),
	(64440, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 20),
	(64714, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(64738, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(64742, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(64752, 0, 7, 8392704, 256, 2097152, 0, 0, 0, 0, 0, 0),
	(64786, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15),
	(64792, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 45),
	(64824, 0, 7, 2097152, 0, 0, 0, 0, 0, 0, 0, 0),
	(64928, 0, 11, 1, 0, 0, 0, 0, 2, 0, 0, 0),
	(64860, 0, 9, 0, 1, 0, 0, 0, 0, 0, 0, 0),
	(64867, 0, 3, 536870945, 4096, 0, 0, 0, 0, 0, 0, 0),
	(64882, 0, 10, 0, 1048576, 0, 0, 0, 0, 0, 0, 0),
	(64890, 0, 10, 0, 65536, 0, 0, 0, 2, 0, 0, 0),
	(64908, 0, 6, 8192, 0, 0, 0, 0, 0, 0, 0, 0),
	(64912, 0, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0),
	(59057, 0, 15, 0, 131072, 0, 0, 0, 0, 0, 0, 0),
	(53234, 0, 9, 131072, 1, 1, 0, 0, 2, 0, 0, 0),
	(56375, 0, 3, 16777216, 0, 0, 0, 65536, 0, 0, 0, 0),
	(57989, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0),
	(50034, 0, 15, 16, 131072, 0, 0, 0, 0, 0, 0, 0),
	(63374, 0, 11, 2147483648, 0, 0, 0, 65536, 0, 0, 0, 0),
	(54821, 0, 7, 4096, 0, 0, 0, 16, 0, 0, 0, 0),
	(54845, 0, 7, 4, 0, 0, 0, 65536, 0, 0, 0, 0),
	(56800, 0, 8, 8388612, 0, 0, 0, 16, 0, 0, 0, 0),
	(58375, 0, 4, 0, 512, 0, 0, 16, 0, 0, 0, 0),
	(67771, 1, 0, 0, 0, 0, 0, 0, 262147, 0, 35, 45),
	(67702, 1, 0, 0, 0, 0, 0, 0, 262147, 0, 35, 45),
	(63320, 0, 5, 2147745792, 0, 32768, 0, 1024, 0, 0, 0, 0),
	(71562, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105),
	(57870, 0, 9, 8388608, 0, 0, 0, 262144, 0, 0, 0, 0),
	(70807, 0, 11, 0, 0, 16, 0, 0, 0, 0, 100, 0),
	(67667, 0, 0, 0, 0, 0, 0, 16384, 0, 0, 0, 45),
	(67672, 0, 0, 0, 0, 0, 0, 8388948, 0, 0, 0, 45),
	(67670, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0, 45),
	(67653, 0, 0, 0, 0, 0, 0, 4194344, 0, 0, 0, 45),
	(70761, 0, 10, 0, 2147500032, 1, 0, 1024, 0, 0, 0, 0),
	(67758, 0, 0, 0, 0, 0, 0, 69632, 2, 0, 0, 2),
	(124974, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(71642, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(71611, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(71640, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(71634, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 30),
	(71645, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0, 45),
	(71602, 0, 0, 0, 0, 0, 0, 65536, 0, 0, 0, 45),
	(71606, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100),
	(71637, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100),
	(71540, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(71402, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(72417, 0, 0, 0, 0, 0, 0, 327680, 0, 0, 0, 60),
	(72413, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(72419, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60),
	(70748, 0, 3, 0, 2097152, 0, 0, 1024, 0, 0, 0, 0),
	(70830, 0, 11, 0, 131072, 0, 0, 0, 0, 0, 0, 0),
	(70727, 0, 9, 0, 0, 0, 0, 64, 0, 0, 0, 0),
	(70730, 0, 9, 16384, 4096, 0, 0, 262144, 0, 0, 0, 0),
	(70803, 0, 8, 4063232, 8, 0, 0, 0, 0, 0, 0, 0),
	(70805, 0, 8, 0, 131072, 0, 0, 0, 0, 0, 0, 0),
	(70841, 0, 5, 4, 256, 0, 0, 262144, 0, 0, 0, 0),
	(108505, 0, 0, 0, 0, 0, 0, 1048576, 0, 0, 0, 0),
	(108281, 0, 0, 0, 0, 0, 0, 87312, 0, 0, 0, 0),
	(108283, 0, 0, 0, 0, 0, 0, 87312, 0, 0, 0, 4),
	(118455, 0, 0, 0, 0, 0, 0, 12582932, 0, 0, 0, 0),
	(117952, 0, 0, 0, 0, 0, 0, 12582932, 0, 0, 0, 0),
	(128991, 0, 0, 0, 0, 0, 0, 16384, 0, 20, 0, 45),
	(128990, 0, 0, 0, 0, 0, 0, 65536, 0, 15, 0, 50),
	(128989, 0, 0, 0, 0, 0, 0, 20, 0, 20, 0, 45),
	(128445, 0, 0, 0, 0, 0, 0, 340, 2, 20, 0, 55),
	(126708, 0, 0, 0, 0, 0, 0, 1048576, 0, 15, 0, 45),
	(126706, 0, 0, 0, 0, 0, 0, 81920, 0, 15, 0, 55),
	(126702, 0, 0, 0, 0, 0, 0, 1048576, 0, 15, 0, 55),
	(126482, 0, 0, 0, 0, 0, 0, 20, 0, 15, 0, 65),
	(126473, 0, 0, 0, 0, 0, 0, 65536, 0, 15, 0, 105),
	(122277, 0, 0, 0, 0, 0, 0, 340, 0, 20, 0, 50),
	(122276, 0, 0, 0, 0, 0, 0, 20, 0, 20, 0, 50),
	(70854, 0, 4, 0, 16, 0, 0, 0, 0, 0, 0, 0),
	(71404, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 50),
	(70652, 0, 15, 8, 0, 0, 0, 0, 0, 0, 0, 0),
	(70756, 0, 10, 2097152, 65536, 0, 0, 0, 0, 0, 0, 0),
	(122275, 0, 0, 0, 0, 0, 0, 340, 0, 20, 0, 50),
	(50421, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(71564, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0),
	(75474, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(75465, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(75457, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(75455, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(71545, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 0),
	(71406, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 0),
	(122274, 0, 0, 0, 0, 0, 0, 1048576, 0, 20, 0, 50),
	(52437, 1, 4, 536870912, 0, 0, 0, 16, 0, 0, 0, 0),
	(71903, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0),
	(122273, 0, 0, 0, 0, 0, 0, 81920, 0, 20, 0, 50),
	(118766, 0, 0, 0, 0, 0, 0, 340, 0, 20, 0, 50),
	(118765, 0, 0, 0, 0, 0, 0, 20, 0, 20, 0, 50),
	(70672, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(118764, 0, 0, 0, 0, 0, 0, 340, 0, 20, 0, 50),
	(118763, 0, 0, 0, 0, 0, 0, 1048576, 0, 20, 0, 50),
	(71585, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(70664, 0, 7, 16, 0, 0, 0, 0, 0, 0, 0, 0),
	(118762, 0, 0, 0, 0, 0, 0, 81920, 0, 20, 0, 50),
	(71880, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),
	(71892, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0),
	(105138, 0, 0, 0, 0, 0, 0, 81920, 0, 25, 0, 45),
	(105136, 0, 0, 0, 0, 0, 0, 1048576, 0, 15, 0, 45),
	(102440, 0, 0, 0, 0, 0, 0, 1048576, 0, 15, 0, 45),
	(102436, 0, 0, 0, 0, 0, 0, 81920, 0, 15, 0, 55),
	(70844, 0, 4, 256, 0, 0, 0, 0, 0, 0, 0, 0),
	(67115, 0, 15, 20971520, 0, 0, 0, 0, 0, 0, 0, 45),
	(67712, 0, 0, 0, 0, 0, 0, 69632, 2, 0, 0, 2),
	(102433, 0, 0, 0, 0, 0, 0, 1048576, 0, 15, 0, 45),
	(95871, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 120),
	(49509, 0, 0, 0, 0, 0, 0, 0, 51, 0, 0, 0),
	(75481, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(70817, 0, 11, 0, 4096, 0, 0, 65536, 0, 0, 0, 0),
	(5301, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0),
	(58642, 0, 15, 0, 134217728, 0, 0, 16, 0, 0, 0, 0),
	(67151, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(45355, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 45),
	(115174, 0, 53, 128, 0, 0, 0, 0, 0, 0, 0, 0),
	(109078, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 90),
	(114052, 0, 0, 0, 0, 0, 0, 87312, 0, 0, 0, 0),
	(127722, 0, 0, 0, 0, 0, 0, 12582912, 0, 0, 0, 0),
	(44448, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(109306, 0, 0, 0, 0, 0, 0, 4368, 0, 0, 0, 0),
	(119030, 0, 0, 0, 0, 0, 0, 1712808, 0, 0, 0, 0),
	(108373, 0, 0, 0, 0, 0, 0, 86036, 0, 0, 0, 0),
	(63254, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0),
	(105764, 0, 11, 536870912, 0, 0, 0, 0, 0, 0, 0, 0),
	(128938, 0, 0, 0, 0, 0, 0, 12932436, 2, 0, 0, 0),
	(114028, 0, 0, 0, 0, 0, 0, 0, 2048, 0, 0, 0),
	(85043, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0),
	(86703, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 100, 0),
	(86701, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 100, 0),
	(78203, 0, 6, 32768, 0, 0, 0, 0, 1027, 0, 100, 0),
	(81333, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(77756, 0, 11, 268435456, 0, 0, 0, 0, 0, 0, 0, 0),
	(86172, 0, 10, 0, 0, 0, 0, 87376, 1027, 0, 25, 0),
	(54939, 0, 10, 32768, 0, 0, 0, 17408, 0, 0, 100, 0),
	(51522, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(114250, 0, 10, 1073741824, 0, 0, 0, 16384, 0, 0, 100, 0),
	(105900, 0, 0, 0, 0, 0, 0, 278528, 0, 0, 0, 0),
	(88736, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 5),
	(69762, 0, 0, 0, 0, 0, 0, 81920, 0, 0, 0, 1),
	(99074, 0, 10, 0, 1048576, 0, 0, 0, 1027, 0, 0, 0),
	(99093, 0, 10, 0, 32768, 0, 0, 0, 1027, 0, 0, 0),
	(99067, 0, 10, 0, 65536, 0, 0, 0, 1027, 0, 0, 0),
	(98970, 0, 15, 0, 1073741824, 0, 0, 0, 0, 0, 0, 0),
	(98996, 0, 15, 0, 134348800, 0, 0, 0, 1027, 0, 0, 0),
	(99001, 0, 7, 34816, 1088, 0, 0, 0, 1027, 0, 0, 0),
	(99009, 0, 7, 8388608, 128, 200000, 0, 16, 1027, 0, 100, 0),
	(99019, 0, 7, 5, 0, 0, 0, 0, 1027, 0, 20, 45),
	(99013, 0, 7, 0, 16, 0, 0, 0, 1027, 0, 0, 0),
	(99015, 0, 7, 0, 2, 0, 0, 0, 1027, 0, 0, 0),
	(99057, 0, 9, 0, 1, 4194304, 0, 0, 1027, 0, 0, 0),
	(99061, 0, 3, 536870945, 4096, 8, 0, 0, 1027, 0, 0, 45),
	(99134, 0, 6, 7168, 0, 0, 0, 0, 1027, 0, 0, 0),
	(99174, 0, 0, 0, 0, 0, 0, 0, 1026, 0, 0, 0),
	(99213, 0, 11, 0, 16777217, 0, 0, 16, 1027, 0, 100, 0),
	(99204, 0, 11, 1, 0, 0, 0, 65536, 1027, 0, 0, 0),
	(99190, 0, 11, 0, 0, 16, 0, 0, 1027, 0, 0, 0),
	(99229, 0, 5, 16385, 192, 0, 0, 0, 1027, 0, 0, 0),
	(99239, 0, 4, 33554432, 67117056, 0, 0, 16, 1027, 0, 0, 0),
	(99234, 0, 4, 65536, 128, 0, 0, 17408, 1027, 0, 0, 0),
	(134563, 0, 0, 0, 0, 0, 0, 1747624, 0, 0, 0, 0),
	(140023, 0, 0, 0, 0, 0, 0, 12915028, 0, 0, 0, 0),
	(138349, 0, 0, 0, 0, 0, 0, 8, 3, 0, 0, 0),
	(139218, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0),
	(136709, 0, 0, 0, 0, 0, 0, 4, 3, 0, 0, 5),
	(136512, 0, 0, 0, 0, 0, 0, 87380, 0, 0, 50, 2),
	(138701, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 85),
	(138757, 0, 0, 0, 0, 0, 0, 0, 0, 1.21, 0, 0),
	(146313, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 115),
	(146219, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 115),
	(146295, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 115),
	(138704, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 55),
	(138700, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 115),
	(124081, 0, 0, 0, 0, 0, 0, 664232, 0, 0, 0, 0),
	(157804, 0, 11, 268435456, 0, 0, 0, 0, 0, 0, 0, 0),
	(157784, 0, 11, 0, 0, 69469184, 0, 0, 0, 0, 0, 0),
	(166096, 0, 11, 0, 0, 0, 64, 0, 0, 0, 0, 0),
	(157765, 0, 11, 2, 4, 0, 0, 65536, 0, 0, 100, 0),
	(88766, 0, 11, 0, 0, 0, 0, 65536, 1027, 0, 0, 0),
	(155159, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0),
	(35110, 0, 0, 4096, 0, 0, 0, 4416, 2, 0, 0, 0),
	(152261, 0, 0, 0, 0, 0, 0, 141992, 64, 0, 100, 0),
	(157724, 0, 0, 131072, 0, 0, 0, 256, 2, 0, 0, 0),
	(152151, 0, 0, 0, 0, 0, 0, 69904, 1027, 0, 0, 0),
	(114015, 0, 0, 0, 0, 0, 0, 4368, 0, 0, 0, 0),
	(81206, 0, 0, 0, 0, 0, 0, 16384, 0, 0, 0, 0),
	(159362, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0),
	(93499, 0, 0, 0, 0, 0, 0, 2359296, 0, 0, 0, 0),
	(5118, 0, 0, 0, 0, 0, 0, 139944, 0, 0, 0, 0),
	(178819, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0),
	(93399, 0, 0, 0, 0, 0, 0, 2359296, 0, 0, 0, 0),
	(50041, 0, 0, 0, 0, 0, 0, 69648, 0, 0, 0, 0),
	(162908, 0, 0, 0, 0, 0, 0, 20, 0, 0, 33, 0),
	(170896, 0, 0, 0, 0, 0, 0, 16777216, 0, 0, 100, 0),
	(166005, 0, 9, 8, 0, 0, 0, 2097152, 0, 0, 0, 0),
	(166021, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	(176930, 0, 0, 0, 0, 0, 0, 0, 0, 0.92, 0, 11),
	(13046, 0, 0, 0, 0, 0, 0, 0, 1026, 0, 0, 0),
	(73681, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0),
	(162288, 0, 0, 0, 0, 0, 0, 2148182696, 0, 0, 0, 0),
	(162892, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0, 0),
	(159636, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0),
	(85804, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(174116, 0, 0, 0, 0, 0, 0, 2148182696, 0, 0, 0, 0),
	(160373, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(167740, 0, 0, 0, 0, 0, 0, 8872, 64, 0, 0, 0),
	(165459, 0, 0, 0, 0, 0, 0, 17408, 0, 0, 0, 0),
	(165476, 0, 0, 0, 0, 0, 0, 16384, 0, 0, 0, 0),
	(170523, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 4),
	(165353, 0, 0, 0, 0, 0, 0, 139944, 16, 0, 0, 0),
	(167718, 0, 0, 0, 0, 0, 0, 17408, 268435456, 0, 0, 0),
	(167655, 0, 0, 0, 0, 0, 0, 69648, 0, 0, 0, 0),
	(165431, 0, 0, 0, 0, 0, 0, 262144, 0, 0, 0, 0),
	(166639, 0, 0, 0, 0, 0, 0, 331792, 0, 0, 0, 0),
	(167714, 0, 0, 0, 0, 0, 0, 2446356, 0, 0, 0, 0),
	(165516, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0),
	(165482, 0, 0, 0, 0, 0, 0, 17408, 0, 0, 0, 0),
	(76672, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(76613, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(170374, 0, 0, 0, 0, 0, 0, 0, 1027, 0, 0, 0),
	(159239, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0),
	(158298, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0);
/*!40000 ALTER TABLE `spell_proc_event` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;