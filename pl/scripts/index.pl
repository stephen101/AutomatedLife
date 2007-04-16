#!/usr/bin/perl -w

use strict;
use Semantic::API;
use File::Find;

###################################
#   Get a list of files to index
###################################
my @files;
my $directory = shift;
die "Usage: ./index.pl <directory list>\n" unless $directory;
find(\&wanted, $directory);


##################################
#   Create indexing object
##################################
my $indexer = Semantic::API::Index->new( storage => 'mysql',
                                         database => 'semantic',
                                         collection => 'my_collection' );


$indexer->add_word_filters( minimum_length => 3,
                            too_many_numbers => 10,
                            maximum_word_length => 15 );

# use this encoding for any incoming text
$indexer->set_default_encoding( "utf8"); 


# read each file and add to the index
foreach my $filename ( @files ){
   # warn "Indexing $filename\n";
    $indexer->index_file( $filename ); 
}


# commit everything to the database
print STDERR "\nNow adding everything to the database...";
$indexer->finish(); 
print STDERR "done!\n"; 

# Valid extensions are: txt, htm, html, rtf, doc, pdf
sub wanted {
    push @files, $File::Find::name if m/\.(?:pdf|doc|rtf|txt|html?)$/;
}
