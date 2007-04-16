# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl Semantic-Engine.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More;
BEGIN { use_ok('Semantic::API') };
use strict;


#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.


my $doc1 = "Glacial ice often appears blue.";
my $doc2 = "Glaciers are made up of fallen snow.";
my $doc3 = "Firn is an intermediate state between snow and glacial ice.";
my $doc4 = "Ice shelves occur when ice sheets extend over the sea.";
my $doc5 = "Glaciers and ice sheets calve icebergs into the sea.";
my $doc6 = "Firn is half as dense as sea water.";
my $doc7 = "Icebergs are chunks of glacial ice under water.";
my @blacklist = qw/an and are as between into is of often over the under up when/;
my $val;
my $summary;
my $docs;
my $terms;
my $obj;

if( -f "t/.skip.mysql.tests"){
	unlink "t/.skip.mysql.tests";
	plan skip_all => "MySQL tests disabled";
}

if( Semantic::API::have_mysql() ){
	plan tests => 27;
} else {
	plan skip_all => "MySQL support not enabled";
}
	
ok( $obj = Semantic::API::Index->new( storage => 'mysql',
									  database => 'semantic_test', 
									  collection => 'test',
									  lexicon => '../share/lexicon.txt'), "Creating MySQL Indexer");

ok( $obj->add_word_filters( minimum_length 			=> 3,
							maximum_word_length 	=> 15,
							maximum_phrase_length	=> 1,
							blacklist 				=> \@blacklist), "Adding Word Filters");

$obj->set_default_encoding("utf8");

ok( $val = $obj->index( 'doc1', $doc1 ), "Indexing document 1");
ok( $val = $obj->index( 'doc2', $doc2 ), "Indexing document 2");
ok( $val = $obj->index( 'doc3', $doc3 ), "Indexing document 3");
ok( $val = $obj->index( 'doc4', $doc4 ), "Indexing document 4");
ok( $val = $obj->index( 'doc5', $doc5 ), "Indexing document 5");
ok( $val = $obj->index( 'doc6', $doc6 ), "Indexing document 6");
ok( $val = $obj->index( 'doc7', $doc7 ), "Indexing document 7");
ok( $obj->finish(), "Adding to database" );



ok( $obj = Semantic::API::Search->new(  storage => 'mysql',
										database => 'semantic_test', 
										collection => 'test', 
										keep_top_edges => 1), "Conducting Search");
ok( ($docs,$terms) = $obj->keyword_search('ice'), "Keyword Search -- ice" );
is( scalar keys %$docs, 5, "Checking results");
ok( ($docs,$terms) = $obj->semantic_search('ice'), "Semantic Search -- ice" );
is( scalar keys %$docs, 7, "Checking results");
ok( ($docs,$terms) = $obj->keyword_search('firn'), "Keyword Search -- firn" );
is( scalar keys %$docs, 2, "Checking results");
ok( ($docs,$terms) = $obj->semantic_search('firn'), "Semantic Search -- firn" );
is( scalar keys %$docs, 4, "Checking results");
ok( ($docs,$terms) = $obj->keyword_search('glacier'), "Keyword Search -- glacier" );
is( scalar keys %$docs, 2, "Checking results");
ok( ($docs,$terms) = $obj->semantic_search('glacier'), "Semantic Search -- glacier" );
is( scalar keys %$docs, 5, "Checking results");


ok( ($docs,$terms) = $obj->find_similar( 'doc3'), "Finding similar");
is( scalar keys %$terms, 5, "Checking results");

ok( $summary = $obj->summarize('doc3'), "Summarize");
ok( $summary eq $doc3, "Checking summary");
#warn "SUMMARY: ".$summary."\n";


