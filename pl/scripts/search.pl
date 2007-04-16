#!/usr/bin/perl -w


######################################
#
#	A simple search engine program
#
######################################


use strict;
use Semantic::API;
use DBI;
use CGI;


#############################################################
my $COLLECTION = 'test';
my ( @TERMS, @RESULTS );
my ( $RESULTS_TO_DISPLAY, $TERMS_TO_DISPLAY ) = ( 10, 10 );
#############################################################


###############################
# CGI Variables
###############################
my $cgi = new CGI;
my $start = $cgi->param( 'start' ) || 0;
my $query = $cgi->param( 'query' ) || '';




##############################
#	Start the HTML output
##############################
print "Content-type: text/html\n\n";
print qq|<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
        "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
  <title>Search Engine</title>
</head>
<body>
  <form method="get" action="">
    <p>
      <input type="text" name="query" value="$query" />
      <input type="submit" />
    </p>\n|;



##########################
#	Do the actual search
##########################
if( $query ){

    # Create collection-based objects	
    my $semantic = Semantic::API::Search->new( storage => 'mysql',
					                           database => 'semantic',
                                               user => 'semantic',
                                               collection => $COLLECTION );



    my ($results, $terms) = $semantic->semantic_search( $query );
	
	
	##################################
	#	TERM BASED CALCULATIONS
	##################################
	my @sorted_terms = sort { $terms->{$b} <=> $terms->{$a} } keys %$terms; 
	my @top_terms = splice( @sorted_terms, $start, $TERMS_TO_DISPLAY );
	
	print "<p>Related Terms: ". ( join ", ", @top_terms ) ."</p>\n";
	print "<hr />\n";


	##################################
	#	DOCUMENT BASED CALCULATIONS
	##################################
    
	print "<p>Result Count: ".(scalar keys %$results)."</p>\n";
	
    my @sorted_results = sort { $results->{$b} <=> $results->{$a} } keys %$results; 
	my @display_results = splice( @sorted_results, $start, $RESULTS_TO_DISPLAY );
	
	
	
	##################################
	#	Access the storage engine to
	# 	retrieve the title and text
	##################################
	my $i = 1 + $start;
	print $semantic->paginate( "?query=$query", $start, scalar keys %$results, $RESULTS_TO_DISPLAY);
	foreach my $id ( @display_results ){
        print "<p>$i. $id</p>\n";
        print "<p>";
		print $semantic->summarize($id);
		print "</p>\n";
        $i++;
	}

}	


print "</body>\n</html>\n";
