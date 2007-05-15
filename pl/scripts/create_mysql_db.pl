#!/usr/bin/perl -w

use strict;
use DBI;
use File::Temp 'tempfile';
use Getopt::Long;

my $create = 0;
my $delete = 0;
my $result = GetOptions("create" => \$create,
						"delete" => \$delete );
						
$create = 1 if $delete == 0;


my $database = shift;
usage() unless $create + $delete == 1 and $database;


my $drh = DBI->install_driver("mysql");
my $rc;
my $user = "root";
my $pass = "";

# first try logging in as root with no password...
eval {
	$rc = $drh->func("dropdb", "$database", 'localhost', $user, '', 'admin');
};
eval {
	$rc = $drh->func("createdb", "$database", 'localhost', $user, '', 'admin');
};

if( $@){
	warn "Error connecting to MySQL driver: $@\n";
	exit(0);
}

# if there is a password for the mysql root user, make mysql prompt for it
if( !$rc ) {
	print "In order to create your database, you will need to enter
the MySQL root password\n";
	$pass = "-p ";
}



my ($fh,$filename) = tempfile();
print $fh "DROP database IF EXISTS `$database`;";

if( $create ){
	print $fh "CREATE database `$database`;";
	print $fh "USE `$database`;";
	print $fh join "\n", <DATA>;
	print $fh "GRANT ALL on `$database`.* to $ENV{USER}\@localhost;";
}
close $fh;



my $rv = system("mysql -u $user $pass $database < $filename");
die "couldn't create database: $!" if $rv;

if( $create ){
	warn "A new MySQL database, called '$database', was created.\n";
} else {
	warn "The MySQL database '$database' was successfully deleted.\n";
}


# usage information
sub usage {
	warn "Usage: ./$0 [--create|--delete] <database_name>\n";
	exit(0);
}


# this is just data taken from the ./sql/mysql5 directory

__DATA__


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

delimiter // ;

-- node triggers
--create trigger node_ai after insert on node for each row 
--	call update_node_count (NEW.fk_collection, NEW.type_major, 1); //
	
create trigger node_ad after delete on node for each row 
begin
	-- decrement node count for this collection/node type
	call update_node_count (OLD.fk_collection, OLD.type_major, -1);
	-- get rid of degree table entries
	delete from degree where fk_node = OLD.id;
	-- clear all edges working with this node
	delete from edge where fk_node_from = OLD.id or fk_node_to = OLD.id;
	-- clear all node meta values
	delete from node_meta where fk_node = OLD.id;
end; //

-- collection triggers
create trigger collection_ad after delete on collection for each row
begin
	-- clear all our nodes
	delete from node where fk_collection=OLD.id;
	-- and node counts
	delete from node_count where fk_collection=OLD.id;
	-- clear collection meta data
	delete from collection_meta where fk_collection=OLD.id;
end; //

-- edge triggers
create trigger edge_ad after delete on edge for each row
begin
	-- get rid of our entries in edge_query table
	delete from edge_query where id = OLD.id;
	-- now update all nodes' edges that were affected by this deletion
	call flag_edge_affected_nodes(OLD.fk_node_from, OLD.fk_node_to);
end; //

create trigger edge_ai after insert on edge for each row
begin
	-- after creating an edge between two existing nodes
	-- we must update any nodes that either node is connected to
	call flag_edge_affected_nodes(NEW.fk_node_from, NEW.fk_node_to);
end; //

create trigger edge_au after update on edge for each row
begin
	-- we must just update the edge_query table for this node, as the strength has changed
	if @batch_mode != 1 then
		insert ignore into dirty_node (fk_node) values (NEW.fk_node_from);
	end if;
end; //

delimiter ; //
delimiter // ;

-- ------------------------
-- these methods are used for the maintenance of graph data
-- ------------------------

-- finishes an indexing process -- performs:
-- * update node counts table
-- * update degree table for dirty nodes
-- * update dirty nodes' edges in edge_query
drop procedure if exists indexing_cleanup //
create procedure indexing_cleanup (IN p_collection INT)
BEGIN
	declare d_collection INT;
	declare done INT default 0;
	declare cur1 cursor for select distinct fk_collection from dirty_node, node where fk_node = id and cleaning = 1;
	declare continue handler for not found set done = 1;
	call update_node_counts ();
	
	if @batch_mode != 1 then
		-- first set the cleaning flag on all the dirty nodes so that if new ones are
		-- added, we don't perform only some operations on them
		update dirty_node set cleaning = 1;
		
		-- now update the degree table
		call update_dirty_degrees ();
	
		open cur1;
		while done = 0 do
			fetch cur1 into d_collection;
			call update_dirty_edge_query (d_collection);
		end while;
		close cur1;

		-- remove our flagged nodes from the dirty_node table
		delete from dirty_node where cleaning = 1;
	else
		-- update all degrees
		call update_all_degrees ();
		
		-- update edge_query
		call update_edge_query (p_collection);
	end if;
		
END; //

--- update node_count table for all nodes
drop procedure if exists update_node_counts //
create procedure update_node_counts ()
BEGIN
	delete from node_count;
	insert into node_count
		select fk_collection, type_major, count(id)
			from node
			group by fk_collection, type_major;
END; //

--- update node_counts table
drop procedure if exists update_node_count //
create procedure update_node_count (IN id INT unsigned, IN type TINYINT, IN incr TINYINT) 
BEGIN
	insert into node_count (fk_collection, type_major, count) values (id, type, incr) on duplicate key update count = count+incr;
END; //

-- procedure to update all edge_query entries for all nodes
--drop procedure if exists update_edge_query_for_all_nodes //
--create procedure update_edge_query_for_all_nodes (IN cid INT unsigned)
--	begin
--		declare done int;
--		declare nid int unsigned;
--		declare cur1 cursor for select id from node where fk_collection = cid;
--		declare continue handler for not found set done = 1;
--		
--		set done = 0;
--		
--		open cur1;
--		while done = 0 do
--			fetch cur1 into nid;
--			call update_edge_query_for_node (nid);
--		end while;
--		close cur1;
--	end; //


-- procedure to update all edge_query entries for a node
drop procedure if exists update_edge_query //
create procedure update_edge_query (IN p_collection INT)
	begin
		declare min int;
		declare oneid int unsigned;
		declare factor decimal(6,5);
		
		-- see if we can't get the value of "min" from collection_meta
		set min = cast(get_collection_meta(p_collection, "min", "1") as unsigned);

		-- see if we can't get the max doc-count factor from collection_meta
		set factor = cast(get_collection_meta(p_collection, "max_factor", "0.1") as decimal);
		
		-- first clear all edge_query rows for this node
		delete from edge_query where fk_collection = p_collection;
		
		-- second re-populate
		insert into edge_query
			select
				e.id,
				n_from.fk_collection,
				e.fk_node_from,
				e.fk_node_to,
				e.strength,
				d_from.degree as degree_from,
				if(d_to.degree is null, 0, d_to.degree) as degree_to,
				n_to.type_major,
				n_to.type_minor
			from
				node n_from
					inner join
						edge e
							on
						e.fk_node_from = n_from.id
					inner join
						node n_to
							on
						e.fk_node_to = n_to.id
					inner join
						degree d_from
							on
						d_from.fk_node = e.fk_node_from
							and
						d_from.type_major = n_to.type_major
					left join
						degree d_to
							on
						d_to.fk_node = e.fk_node_to
							and
						d_to.type_major = n_from.type_major
					left join
						node_count nc
							on
						nc.fk_collection = n_from.fk_collection
							and
						nc.type_major = n_from.type_major
			where
				-- only for this collection
				n_from.fk_collection = p_collection
					and
				-- make sure the target node isn't too well connected (count * factor) or too disconnected (min)
				-- * only do this for edges from a document to a term node
				(
					(n_from.type_major = 2)
						or
					(d_to.degree >= min and d_to.degree < if(floor(nc.count * factor) < min+2, min+2, floor(nc.count * factor)))
				);
	end; //


-- procedure to update all edge_query entries for a node
drop procedure if exists update_dirty_edge_query //
create procedure update_dirty_edge_query (IN p_collection INT)
	begin
		declare min int;
		declare oneid int unsigned;
		declare factor decimal;
		
		-- see if we can't get the value of "min" from collection_meta
		set min = cast(get_collection_meta(p_collection, "min", "3") as unsigned);

		-- see if we can't get the max doc-count factor from collection_meta
		set factor = cast(get_collection_meta(p_collection, "max_factor", "0.1") as decimal);
		
		-- first clear all edge_query rows for this node
		delete from edge_query where fk_node_from in (select fk_node from dirty_node, node where id = fk_node and fk_collection = p_collection and cleaning = 1);
		
		-- second re-populate
		insert into edge_query
			select
				e.id,
				n_from.fk_collection,
				e.fk_node_from,
				e.fk_node_to,
				e.strength,
				d_from.degree as degree_from,
				if(d_to.degree is null, 0, d_to.degree) as degree_to,
				n_to.type_major,
				n_to.type_minor
			from
				node n_from
					inner join
						edge e
							on
						e.fk_node_from = n_from.id
					inner join
						node n_to
							on
						e.fk_node_to = n_to.id
					inner join
						degree d_from
							on
						d_from.fk_node = e.fk_node_from
							and
						d_from.type_major = n_to.type_major
					left join
						degree d_to
							on
						d_to.fk_node = e.fk_node_to
							and
						d_to.type_major = n_from.type_major
					left join
						node_count nc
							on
						nc.fk_collection = n_from.fk_collection
							and
						nc.type_major = n_from.type_major
			where
				-- only for dirty nodes
				e.fk_node_from in (select fk_node from dirty_node, node where id = fk_node and fk_collection = p_collection and cleaning = 1)
					and
				-- make sure the target node isn't too well connected (count * factor) or too disconnected (min)
				(d_to.degree > min and d_to.degree < if(floor(nc.count * factor) < min+2, min+2, floor(nc.count * factor)));
	end; //

-- maintenance procedure	
--create procedure perform_maintenance () 
--	begin
--		alter table edge_query order by fk_node_from;
--		alter table collection_meta order by fk_collection;
--		alter table edge_term_form_map order by fk_edge;
--		alter table node_count order by fk_collection;
--		alter table term_form order by fk_term;
--		optimize table edge_query, edge, node, term, doc, sentence, collection, node_count, edge_term_form_map, term_form;
--		analyze table edge_query, edge, node, term, doc, sentence, collection_meta, node_count, edge_term_form_map, term_form;
--	end; //

-- updates the degree table for all dirty nodes
drop procedure if exists update_dirty_degrees //
create procedure update_dirty_degrees ()
	begin
		delete from degree where fk_node in (select fk_node from dirty_node where cleaning = 1);
		insert into degree select 
			fk_node_from as fk_node, 
			type_major, 
			count(fk_node_to) as degree 
		from edge, node 
		where 
			edge.fk_node_to = node.id 
				and
			edge.fk_node_from in (select fk_node from dirty_node where cleaning = 1)
		group by fk_node, type_major;
	end; //

-- updates the degree table for all nodes
drop procedure if exists update_all_degrees //
create procedure update_all_degrees ()
	begin
		delete from degree;
		insert into degree select 
			fk_node_from as fk_node, 
			type_major, 
			count(fk_node_to) as degree 
		from edge, node 
		where 
			edge.fk_node_to = node.id 
		group by fk_node, type_major;
	end; //

		
-- procedure to get all effected edges (incoming to node1 or node2) & update them
drop procedure if exists flag_edge_affected_nodes //
create procedure flag_edge_affected_nodes (IN p_node1 INT unsigned, IN p_node2 INT unsigned)
	begin
		if @batch_mode != 1 then
			insert ignore into dirty_node select fk_node_to, 0 from edge where fk_node_from = p_node1 or fk_node_from = p_node2;
			insert ignore into dirty_node (fk_node) values (p_node1);
		end if;
	end; //
	
-- ------------------------
-- these methods are used for the creation of graph data
-- ------------------------
drop function if exists get_collection_id //
create function get_collection_id (p_name VARCHAR(100))
	returns int unsigned
	deterministic modifies sql data
	begin
		-- insert into 
		insert ignore into collection (name) values (p_name);
		
		return (select id from collection where name=p_name);
	end; //
	
drop function if exists create_node //
create function create_node (p_collection_id TINYINT, p_type_major TINYINT, p_type_minor TINYINT, p_fk_content INT unsigned)
	returns int unsigned
	deterministic
	modifies sql data
	begin
		declare the_id, none INT unsigned;
		declare cur1 cursor for select id from node where fk_collection = p_collection_id and type_major = p_type_major and type_minor = p_type_minor and fk_content = p_fk_content;
		declare continue handler for not found set none = 1;
		if @batch_mode = 1 then
			set none = 1;
		else
			set none = 0;
			open cur1;
			fetch cur1 into the_id;
			close cur1;
		end if;
		
		if none = 1 then
			-- we have to create the node
			insert into node (fk_collection, type_major, type_minor, fk_content) values (p_collection_id, p_type_major, p_type_minor, p_fk_content);
			set the_id = last_insert_id();
		end if;
		
		return the_id;
	end; //
	
-- creates or returns the node entry for the passed doc in collection
drop function if exists create_node_with_content //
create function create_node_with_content (p_collection TINYINT unsigned, p_type_major TINYINT, p_type_minor TINYINT, p_content VARCHAR(255))
	returns int unsigned
	deterministic
	modifies sql data
	begin
		return create_node(p_collection, p_type_major, p_type_minor, create_content_row(p_content));
	end; //
	
-- returns the id (or creates it) of the content row in the `content` table
drop function if exists create_content_row //
create function create_content_row (p_content VARCHAR(255))
	returns int unsigned
	deterministic
	modifies sql data
	begin
		insert ignore into content (content) values (p_content);
		
		return (select id from content where content=p_content);
	end; //

drop procedure if exists set_collection_meta //
create procedure set_collection_meta (IN p_name VARCHAR(100), IN p_key VARCHAR(100), IN p_value VARCHAR(1024))
	begin
		insert into 
			collection_meta (`fk_collection`, `key`, `value`) 
				values (get_collection_id(p_name), p_key, p_value) 
			on duplicate key update value=VALUES(value);
	end; //
		
drop procedure if exists create_edge //
create procedure create_edge (p_node_from INT unsigned, p_node_to INT unsigned, p_strength DECIMAL, p_additive BOOL)
	begin
		-- insert our new row, or update the existing one if we need to.
		if p_additive then
			insert into
				edge (fk_node_from, fk_node_to, strength)
					values (p_node_from, p_node_to, p_strength)
				on duplicate key 
					update strength = strength + p_strength;
		else
			insert into
				edge (fk_node_from, fk_node_to, strength)
					values (p_node_from, p_node_to, p_strength)
				on duplicate key 
					update strength = p_strength;
		end if;
	end; //


-- ---------------------
-- these methods are used for the extraction of graph data
-- ---------------------

drop function if exists get_collection_meta //
create function get_collection_meta (p_collection INT, p_key VARCHAR(100), p_value_default VARCHAR(1024))
	returns VARCHAR(1024)
	deterministic
	reads sql data
	begin
		declare none INT default 0;
		declare thevalue VARCHAR(1024);
		declare cur1 cursor for select value from collection_meta where fk_collection = p_collection and `key` = p_key;
		declare continue handler for not found set none = 1;
		
		open cur1;
		fetch cur1 into thevalue;
		close cur1;
		
		if none = 1 then
			set thevalue = p_value_default;
		end if;
		
		return thevalue;
	end; //
	
drop function if exists get_collection_meta_by_node //
create function get_collection_meta_by_node (p_node INT, p_key VARCHAR(100), p_value_default VARCHAR(1024))
	returns VARCHAR(1024)
	deterministic
	reads sql data
	begin
		declare none INT default 0;
		declare thevalue VARCHAR(1024);
		declare cur1 cursor for select value from collection_meta m, node n where m.fk_collection = n.fk_collection and n.id = p_node and `key` = p_key;
		declare continue handler for not found set none = 1;

		open cur1;
		fetch cur1 into thevalue;
		close cur1;

		if none = 1 then
			set thevalue = p_value_default;
		end if;

		return thevalue;
	end; //
	
drop function if exists get_node_collection //
create function get_node_collection (p_node INT)
	returns tinyint
	deterministic
	reads sql data
	begin
		declare none INT default 0;
		declare theid INT;
		declare cur1 cursor for select fk_collection from node where id = p_node;
		declare continue handler for not found set none = 1;
		
		open cur1;
		fetch cur1 into theid;
		close cur1;
		
		if none = 1 then
			set theid = NULL;
		end if;
		
		return theid;
	end; //


delimiter ; //
