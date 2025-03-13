/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

CREATE TABLE IF NOT EXISTS `api_account_validate` (
  `AccountID` int(10) unsigned NOT NULL,
  `IsValidated` tinyint(3) unsigned NOT NULL,
  KEY `AccountID` (`AccountID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `api_points` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `AccountID` int(10) unsigned NOT NULL,
  `Points` int(10) unsigned NOT NULL,
  `TypePoints` int(10) unsigned NOT NULL,
  `Data` text NOT NULL,
  PRIMARY KEY (`id`),
  KEY `AccountID` (`AccountID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE IF NOT EXISTS `api_purchase` (
  `AccountID` int(10) unsigned NOT NULL,
  `Expansion` tinyint(3) unsigned NOT NULL,
  `Realm` int(11) NOT NULL,
  `CharacterGUID` int(10) unsigned NOT NULL,
  `ShopEntryData` text NOT NULL,
  `Count` int(10) unsigned NOT NULL,
  `TypePrice` int(10) unsigned NOT NULL,
  `Price` int(10) unsigned NOT NULL,
  `IP` text NOT NULL,
  KEY `AccountID` (`AccountID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;