
--
-- Table structure for table 'collection'
--

DROP TABLE IF EXISTS 'collection';
CREATE TABLE 'collection' (
  'id' integer primary key,
  'name' text unique,
  'last_update' integer default CURRENT_TIMESTAMP
);

--
-- Table structure for table 'collection_meta'
--

DROP TABLE IF EXISTS 'collection_meta';
CREATE TABLE 'collection_meta' (
  'fk_collection' integer,
  'key' text,
  'value' text,
  UNIQUE ('fk_collection','key')
);

--
-- Table structure for table 'degree'
--

DROP TABLE IF EXISTS 'degree';
CREATE TABLE 'degree' (
  'fk_node' integer,
  'type_major' integer,
  'degree' integer,
  PRIMARY KEY ('fk_node','type_major')
);

--
-- Table structure for table 'content'
--

DROP TABLE IF EXISTS 'content';
CREATE TABLE 'content' (
  'id' integer primary key,
  'content' text unique
);

--
-- Table structure for table 'edge'
--

DROP TABLE IF EXISTS 'edge';
CREATE TABLE 'edge' (
  'id' integer primary key,
  'fk_node_from' integer,
  'fk_node_to' integer,
  'strength' real,
  UNIQUE ('fk_node_from','fk_node_to')
);

--
-- Table structure for table 'edge_query'
--

DROP TABLE IF EXISTS 'edge_query';
CREATE TABLE 'edge_query' (
  'id' integer primary key,
  'fk_collection' integer,
  'fk_node_from' integer,
  'fk_node_to' integer,
  'strength' integer,
  'degree_from' integer,
  'degree_to' integer,
  'type_major' integer,
  'type_minor' integer,
  UNIQUE ('fk_node_from', 'fk_node_to')
);

--
-- Table structure for table 'node'
--

DROP TABLE IF EXISTS 'node';
CREATE TABLE 'node' (
  'id' integer primary key,
  'fk_collection' integer,
  'type_major' integer,
  'type_minor' integer,
  'fk_content' integer
);

--
-- Table structure for table 'node_count'
--

DROP TABLE IF EXISTS 'node_count';
CREATE TABLE 'node_count' (
  'fk_collection' integer,
  'type_major' integer,
  'count' integer,
  PRIMARY KEY ('fk_collection','type_major')
);

-- 
-- Table structure for table 'node_meta'
--

DROP TABLE IF EXISTS 'node_meta';
CREATE TABLE 'node_meta' ( 
   'fk_node' integer, 
   'key' text, 
   'value' text, 
   UNIQUE('fk_node','key') 
);
