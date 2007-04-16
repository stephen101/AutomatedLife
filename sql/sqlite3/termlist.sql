
--
-- Table structure for table 'termlist'
--

DROP TABLE IF EXISTS 'termlist';
CREATE TABLE 'termlist' (
  'term' text,
  'count' integer,
  'fk_collection' integer,
  PRIMARY KEY ('term')
);

--
-- Table structure for table `collection`
--

DROP TABLE IF EXISTS 'collection';
CREATE TABLE 'collection' (
  'id' integer,
  'name' text,
  PRIMARY KEY  ('id')
);

