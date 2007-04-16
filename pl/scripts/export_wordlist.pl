#!/usr/bin/perl -w

use strict;
use DBI;
use Getopt::Long 'GetOptions';

my ($db_name1,$db_name2) = ('')x2;
my ($use_mysql,$mysql,$sqlite) = (0)x3;
my ($dbh, $select);
my ($db_user, $db_pass) = ($ENV{USER},"");

my $result = GetOptions( "from=s"  => \$db_name1,
						 "to=s" => \$db_name2,
						 "mysql" => \$mysql,
						 "sqlite" => $sqlite,
						);
my $collection_name = shift;
$use_mysql = 1 if (!$mysql and !$sqlite) or ($mysql and $sqlite) or ($mysql and !$sqlite);

usage("You must provide a collection name!" ) unless $collection_name;
usage("You must identify a database from which to import data!") unless $db_name1;
usage("You must identify a database into which the data will be exported!") unless $db_name2;

 


### MySQL ##############
if( $use_mysql ){
	$dbh = DBI->connect("dbi:mysql:$db_name1",$db_user,$db_pass,{RaiseError=>1}) or die $!;
	$select = "SELECT m.value, c.id FROM collection_meta m, collection c WHERE c.id=m.fk_collection AND c.name=? AND `key`=?";

### SQLite #############
} else {
	$dbh = DBI->connect( "dbi:SQLite:dbname=$db_name1","","") or die $!;
	$select = "SELECT m.value, c.id FROM collection_meta m, collection c WHERE c.id=m.fk_collection AND c.name=? AND 'key'=?";
}
########################



### Fetch the wordlist ############
my $sth = $dbh->prepare($select);
$sth->execute($collection_name, "wordlist");
my ($wordlist, $id) = $sth->fetchrow_array();
$sth->finish;
$dbh->disconnect;
die "Couldn't extract wordlist!\n" unless length $wordlist;
###################################



### MySQL ##############
if( $use_mysql){
	$dbh = DBI->connect("dbi:mysql:$db_name2",$db_user,$db_pass,{RaiseError=>1}) or die $!;

### SQLite #############
} else {
	$dbh = DBI->connect( "dbi:SQLite:dbname=$db_name2","","") or die $!;
}
########################



### Clear the deck! ###################
$sth = $dbh->prepare("DELETE FROM collection WHERE id=?");
$sth->execute($id);
$sth->finish;
$sth = $dbh->prepare("DELETE FROM termlist WHERE fk_collection=?");
$sth->execute($id);
$sth->finish;
#######################################



### Insert data into the new collection ########
$sth = $dbh->prepare("INSERT INTO collection (id,name) VALUES (?,?)");
$sth->execute($id,$collection_name);
$sth->finish;

$sth = $dbh->prepare("INSERT INTO termlist (term,count,fk_collection) VALUES (?,?,?)");
while( $wordlist =~ m/[^,:;]+,([^,:;]+):([^,:;]+);/g ){
	my $term = $1;
	next if $term =~ m/\W$/;
	my $count = $2;
	$sth->execute($term,$count,$id);
}
$sth->finish;
$dbh->disconnect;
################################################


sub usage {
	my $warning = shift @_;
	warn "$warning\n\n" if $warning;
	warn "Usage: ./export_wordlist [--mysql or --sqlite] --from=db_name --to=db_name collection_name\n";
	exit(0);
}
