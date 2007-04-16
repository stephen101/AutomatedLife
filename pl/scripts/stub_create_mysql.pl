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


