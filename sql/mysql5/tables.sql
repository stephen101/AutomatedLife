-- MySQL dump 10.9
--
-- Host: localhost    Database: semantic
-- ------------------------------------------------------
-- Server version	5.1.3-alpha

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

--
-- Table structure for table `collection`
--

DROP TABLE IF EXISTS `collection`;
CREATE TABLE `collection` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `name` varchar(50) NOT NULL,
  `last_update` timestamp NOT NULL default CURRENT_TIMESTAMP,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `collection_meta`
--

DROP TABLE IF EXISTS `collection_meta`;
CREATE TABLE `collection_meta` (
  `fk_collection` tinyint(4) unsigned NOT NULL,
  `key` varchar(100) NOT NULL,
  `value` mediumtext NOT NULL,
  UNIQUE KEY `fk_collection` (`fk_collection`,`key`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `degree`
--

DROP TABLE IF EXISTS `degree`;
CREATE TABLE `degree` (
  `fk_node` int(10) unsigned NOT NULL default '0',
  `type_major` tinyint(3) unsigned NOT NULL default '0',
  `degree` smallint(5) unsigned default NULL,
  PRIMARY KEY  (`fk_node`,`type_major`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `dirty_node`
--

DROP TABLE IF EXISTS `dirty_node`;
CREATE TABLE `dirty_node` (
  `fk_node` int(10) unsigned NOT NULL default '0',
  `cleaning` tinyint(3) unsigned NOT NULL default '0',
  PRIMARY KEY (fk_node)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `content`
--

DROP TABLE IF EXISTS `content`;
CREATE TABLE `content` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `content` varchar(255) NOT NULL,
  PRIMARY KEY  (`id`),
  UNIQUE KEY `content` (`content`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `edge`
--

DROP TABLE IF EXISTS `edge`;
CREATE TABLE `edge` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `fk_node_from` int(10) unsigned NOT NULL,
  `fk_node_to` int(10) unsigned NOT NULL,
  `strength` decimal(10,0) unsigned NOT NULL default '0',
  PRIMARY KEY  (`id`),
  UNIQUE KEY `edge` (`fk_node_from`,`fk_node_to`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `edge_query`
--

DROP TABLE IF EXISTS `edge_query`;
CREATE TABLE `edge_query` (
  `id` int(10) unsigned NOT NULL,
  `fk_collection` tinyint(4) unsigned NOT NULL,
  `fk_node_from` int(10) unsigned NOT NULL,
  `fk_node_to` int(10) unsigned NOT NULL,
  `strength` smallint(5) unsigned NOT NULL,
  `degree_from` smallint(5) unsigned NOT NULL,
  `degree_to` smallint(5) unsigned NOT NULL,
  `type_major` tinyint(3) unsigned NOT NULL,
  `type_minor` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `fk_node_from` (`fk_node_from`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `node`
--

DROP TABLE IF EXISTS `node`;
CREATE TABLE `node` (
  `id` int(10) unsigned NOT NULL auto_increment,
  `fk_collection` tinyint(3) unsigned NOT NULL,
  `type_major` tinyint(3) unsigned NOT NULL,
  `type_minor` tinyint(3) unsigned NOT NULL default '0',
  `fk_content` int(10) unsigned NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `node_count`
--

DROP TABLE IF EXISTS `node_count`;
CREATE TABLE `node_count` (
  `fk_collection` tinyint(3) unsigned NOT NULL,
  `type_major` tinyint(3) unsigned NOT NULL,
  `count` bigint(8) unsigned NOT NULL,
  PRIMARY KEY  (`fk_collection`,`type_major`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `node_meta`
--

DROP TABLE IF EXISTS `node_meta`;
CREATE TABLE `node_meta` (
  `fk_node` int(10) unsigned NOT NULL,
  `key` varchar(255) NOT NULL,
  `value` text NOT NULL,
  PRIMARY KEY (`fk_node`,`key`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

