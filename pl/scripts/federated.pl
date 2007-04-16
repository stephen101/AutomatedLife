#!/usr/bin/perl -w


##############################################
#
#	A simple multi-collection search engine
#
##############################################


use strict;
use Semantic::API;
use DBI;
use CGI;


#############################################################
my @COLLECTIONS = qw/first second third/;
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
  <title>Federated Search Engine</title>
</head>
<body>
  <form method="get" action="$0">
    <p>
      <input type="text" name="query" value="$query" />
      <input type="submit" />
    </p>\n|;



##########################
#	Do the actual search
##########################
if( $query ){

	
	foreach my $C ( @COLLECTIONS ){
	
		# Create collection-based objects	
		my $semantic = Semantic::API::Search->new( database => 'semantic',
											  user => 'semantic',
											  collection => $C );
	
		my ($results, $terms ) = $semantic->search( $query );
	
		push @RESULTS, map { 
							  { id => $_,
							 	r  => $results->{$_},
							 	c  => $C } 
						    } keys %$results;
		push @TERMS, map {  
							{ term => $_,
						      r    => $terms->{$_},
						      c    => $C } 
						 } keys %$terms;
							 
		
	}
	
	
	print "<p>Result Count: ".scalar @RESULTS."</p>\n";
	
	
	
	###############################
	#	TERM BASED CALCULATIONS
	###############################
	my @sorted_terms = sort { $b->{r} <=> $a->{r} } @TERMS; 
	my @top_terms;
	{ 
		my %term_index;
		foreach my $ref ( @sorted_terms ){
			my $t = $ref->{'term'};
			push @top_terms, $t unless defined $term_index{ $t };
			$term_index{$t}  = 1;
			last if scalar @top_terms >= $TERMS_TO_DISPLAY;
		}
	}

	###############################
	#	FIX THE STEMMING
	###############################
	#	if stemming was turned on during the indexing
	# 	you will probably want to map these words back
	#	to their original forms using some sort of
	#	file-based binary search
	
	
	print "<p>Terms: ".( join ", ", @top_terms )."</p>\n";
	print "<hr />\n";



	#################################
	#	DOCUMENT BASED CALCULATIONS
	#################################
	my @sorted_results = sort { $b->{r} <=> $a->{r} } @RESULTS; 
	my @display_results = splice( @sorted_results, $start, $RESULTS_TO_DISPLAY );
	
	
	my %TEXT;
	my $ORGANIZED;
	foreach my $ref ( @display_results ){
		push @{ $ORGANIZED->{ $ref->{c} } }, $ref;
	}


	
	###########################################
	#	Here I assume that each collection
	#	is stored in a standard format --
	#	in this case, a mysql database
	###########################################
	foreach my $collection ( keys %$ORGANIZED ){
		my $dbh = DBI->connect( "dbi:mysql:".$collection."_docs", $collection, "", {RaiseError=>1} );
		my $sth = $dbh->prepare( "SELECT title, text FROM docs WHERE doc_id = ?" );
		foreach my $ref ( @{ $ORGANIZED->{ $collection } } ){
			my $id = $ref->{id};
			$sth->execute( $id );
			my ( $title, $text ) = $sth->fetchrow_array;
			$TEXT{ $collection.$id } = [ $title, $text ];	
		}
		$sth->finish;
		$dbh->disconnect;
	}
	


	##################################
	# Print out the title and text
	##################################
	foreach my $ref ( @display_results ){
		my $id = $ref->{id};
		my $c = $ref->{c};
		my ( $title, $text ) = @{ $TEXT{ $c.$id } };
		print "<p>".$c.": $title</p>\n";
		foreach my $term ( @words ){
			$title =~ s/\b($term[^\s]*)\b/<b>$1<\/b>/ig;
			$text =~ s/\b($term[^\s]*)\b/<b>$1<\/b>/ig;
            $text =~ s/\n{2,}/<br \/><br \/>/g;
		}
		print "<p>$text</p>\n";
        print "<hr />\n";
		
	}
}	


print "</body>\n</html>\n";




	
	
	

										   
										   
