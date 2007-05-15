package Semantic::API;
{
use 5.006001;
use strict;
use warnings;
use Carp;

require Exporter;

our @ISA = qw(Exporter);

our @EXPORT_OK = qw( have_sqlite have_mysql );

our $VERSION = '2.04';

require XSLoader;
XSLoader::load('Semantic::API', $VERSION);

    sub stoplist {
        my $location = Semantic::API::stoplist_location();
        open FH, $location or die "Stoplist not found at: $location\n";
		my @stoplist = <FH>;
		my $blacklist;
		close FH;
		foreach my $line ( @stoplist ){
			chomp $line;
			push @$blacklist, $line;
		}
		return $blacklist;
    }

}

package Semantic::API::Search;
{
    use base 'Semantic::API';
    use warnings;
    use strict;
    use Carp;
    
    sub new {
        my ( $class, %params ) = @_;
        my $self;
        my @required = qw/database collection/;
        foreach( @required ){ 
            croak "Required class attribute not supplied: $_\n" unless defined $params{$_}; 
        }
        if( $params{'storage'} eq 'mysql'){
            delete $params{'storage'};
            $params{'username'} = $ENV{'USER'} unless defined $params{'username'};
            $self = Semantic::API::MySQLSearch->new(%params);
        
        } elsif ( $params{'storage'} eq 'sqlite'){
            delete $params{'storage'};
            $self = Semantic::API::SQLiteSearch->new(%params);
        
        } elsif ( defined $params{'storage'}) {
            croak "Unrecognized storage policy: ".$params{'storage'}."\n";
        } else {
            croak "Undefined storage policy!\n";
        }
        
        return $self;
    }

    sub better_search {
        my ($self, $query) = @_;
    
        my $ref = $self->_better_semantic_search($query); # C++/XS function
        if( wantarray ){ 
            return ( $ref->[0], $ref->[1] );
        } else {
            return $ref->[0];
        }
    }
    

    sub semantic_search {
        my ($self, $query) = @_;
    
        my $ref = $self->_semantic_search($query); # C++/XS function
        if( wantarray ){ 
            return ( $ref->[0], $ref->[1] );
        } else {
            return $ref->[0];
        }
    }

    sub keyword_search {
        my ($self, $query) = @_;
    
        my $ref = $self->_keyword_search($query); # C++/XS function
        if( wantarray ){
            return ( $ref->[0], $ref->[1] );
        } else {
            return $ref->[0];
        }
    }

    sub find_similar {
        my ($self, @ids) = @_;
    
        my $ref = $self->_find_similar(\@ids); # C++/XS function
        if( wantarray ){
            return ( $ref->[0], $ref->[1] );
        } else {
            return $ref->[0];
        }
    }

    sub summarize {
        my ( $self, @doc_ids ) = @_;
        my $ref = $self->_summarize(\@doc_ids);
        if( wantarray ){
            return $$ref;
        } else {
            return $ref->[0];
        }
    }

    sub paginate {
        my ( $self, $url, $start, $total, $results_per_page ) = @_;
        my $page;
        $url =~ s/\s+/\+/g;
        if( $total > $results_per_page ){
            my $pages = sprintf "%d", $total / $results_per_page;
            $pages++ if $total   % $results_per_page;
            my $prev = $start - $results_per_page;
            if( $prev >= 0 ){ 
                $page .= "<a href=\"$url;start=$prev\">&laquo; previous</a> ";
            } else {            $page .= "&laquo; previous ";
            }       
            my $s;  

            if ($start > (5 * $results_per_page)){
                $s = $start / $results_per_page - 4;
            } else {
                $s = 0; 
            }       
            for( my $i = $s; $i < $pages; $i++ ){ 
                last if $i >= 10 + $s;
                if( $i == $start / $results_per_page ){
                    $page .= "<b>". ($i + 1 ). "</b> ";
                } else {
                    $page .= "<a href=\"$url;start=".
                         ($i * $results_per_page ) .
                         "\">". ($i + 1) ."</a> ";
                }       
            }       
            my $next = $start + $results_per_page;
            if( $next > $total ){
                $page .= "next &raquo; ";
            } else {
                $page .= "<a href=\"$url;start=$next\">next &raquo;</a> ";
            }       
        }
        return $page;
    }       
}

package Semantic::API::Index;
{
    use base 'Semantic::API';
    use Carp;
    use warnings;
    use strict;
    
    sub new {
        my ( $class, %params ) = @_;
        my $self;
        my @required = qw/database collection/;
        foreach( @required ){ 
            croak "Required class attribute not supplied: $_\n" unless defined $params{$_}; 
        }
        if( $params{'storage'} eq 'mysql'){
            $params{'username'} = $ENV{USER} unless defined $params{'username'};
            delete $params{'storage'};
            $self = Semantic::API::MySQLIndex->new(%params);
            
        } elsif ( $params{'storage'} eq 'sqlite'){
            delete $params{'storage'};
            $self = Semantic::API::SQLiteIndex->new(%params);
        
        }
        $self->set_default_encoding( $params{'default_encoding'}) if defined $params{'default_encoding'};
        $self->set_parsing_method( $params{'parsing_method'}) if defined $params{'parsing_method'};
        return $self;
    }

    sub index_file{
        my ($self, $filename, $w) = @_;
        my $weight = 1;
        if( defined $w and $w =~ m/^\d+$/){
            $weight = $w;
        }
        if( -f $filename ){
            return $self->_index_file( $filename, $weight );
        } else {
            croak "File does not exist: $filename\n";
        }
    }

	sub finish {
		my ( $self, $min ) = @_;
		$min = 1 unless $min;
		$self->_finish($min);
		return 1;
	}



    sub index{
        my ($self, $id, $text, $w ) = @_;
        my $weight = 1;
        if( defined $w and $w =~ m/^\d+$/ ) {
            $weight = $w;
        } 
        $self->_index($id,$text,$weight);
        return 1;       
    }

    sub reindex{
        my ( $self, $id, $text, $w ) = @_;
        my $old = $self->get_document_text($id);
        if( $old and $old ne $text ){
            $self->unindex($id);
            $self->index($id,$text,$w)
        }
        return 1;
    }
    
    sub reindex_file{
        my ( $self, $filename, $weight ) = @_;
        if( -f $filename ){
            $self->unindex($filename);
            $self->index_file($filename,$weight);
        } else {
            croak "Invalide filename: $filename\n";
        }
        return 1;
    }
    

    
}

package Semantic::API::MySQLSearch;
{
    use base 'Semantic::API::Search';
}
package Semantic::API::SQLiteSearch;
{
    use base 'Semantic::API::Search';
}

package Semantic::API::MySQLIndex;
{
    use base 'Semantic::API::Index';
}
package Semantic::API::SQLiteIndex;
{
    use base 'Semantic::API::Index';
}



1;
__END__


=head1 NAME

Semantic::API - Perl extension for a graph-based search

=head1 DESCRIPTION

The Semantic Engine has emerged from an attempt to improve on standard keyword
searches. By analyzing the statistical patterns in natural language, 
concept-based relationships can be established between distinct texts that may
not share particular key words. By representing a text collection as a
graph-theoretic network, similarities and relationships can easily be found in
otherwise unstructured data.

=head1 SYNOPSIS - Indexing

  use Semantic::API;

  my $semantic = Semantic::API::Index->new( 
                            collection => 'my_collection',
                            storage => 'mysql',
                            database => 'my_database',
                            host => 'localhost',
                            username => 'my_user',
                            password => '***',
                            min_term_frequency => 2,
                            max_document_frequency => '0.2' );
  -- OR --
  my $semantic = Semantic::API::Index->new( 
                            collection => 'my_collection',
                            storage => 'sqlite',
                            database => 'my_database' );

  $semantic->add_word_filters( 
                            too_many_numbers     => 10,
                            minimum_length       => 3,
                            maximum_word_length  => 15,
                            maximum_phrase_length=> 3,
                            blacklist            => \@blacklist,
                            whitelist            => \@whitelist);

  $semantic->index_file( $filename ); # read this file and index it!
  ...
  $semantic->set_default_encoding( "utf8"); # use this encoding for any incoming text
  $semantic->index( $id, $text );
  ...
  $semantic->index( $id, $text, $weight ); # give this item a different weight

  $semantic->finish(); # commit everything to the database


=head1 SYNOPSIS - Searching

  use Semantic::API;

  my $semantic = Semantic::API::Search->new( collection => 'my_collection',
                                             storage => 'mysql',
                                             database => 'my_database',
                                             username => 'my_user',
                                             password => '***',
                                             host => 'localhost' );

  -- OR --
  my $semantic = Semantic::API::Search->new( collection => 'my_collection',
                                             storage => 'sqlite',
                                             database => 'my_database' );


  my ($results, $terms ) = $semantic->semantic_search( 'query' );
  -- OR --
  my ($results, $terms ) = $semantic->keyword_search( 'query' );

  my @term_list   = sort { $terms->{$b}   <=> $terms->{$a}   } keys %$terms;
  my @result_list = sort { $results->{$b} <=> $results->{$a} } keys %$results;

  foreach( @result_list ){
      ...
  }


=head1 METHODS

=over

=item new( %PARAMETERS )

Parameters: takes a named parameter list (see Synopsis above) specifying the 
storage policy, a collection name, and some database parameters:

    storage    => 'mysql' or 'sqlite'
    collection => 'collection name'
    database   => 'database name'
    username   => 'mysql username' (optional)
    password   => 'mysql password' (optional)
    host       => 'mysql host' (optional)
    min_term_frequency => 'minimum occurrence of a term' (optional)
    max_document_frequency => 'maximum percent of collection in
                               which a term occurs' (optional)

Additional parameters are listed below.

=back

=head2 Indexing

Additional optional parameters: 

    lexicon => 'path/to/lexicon.gz'
    default_encoding => 'iso-8859-1'
    parsing_method => 'nouns'

=over

=item add_word_filters( %FILTERS ) 

Various filters can be added to trim the list of words that are indexed. 
All nouns are, by default, added to the index, but some other words will
sometimes slip through. Filters available for use include:

    minimum_length        => $num  # omit words with fewer characters than $num
    maximum_word_length   => $num  # omit words with more characters than $num
    maximum_phrase_length => $num  # omit phrases with more words than $num
    too_many_numbers      => $num  # omit words containing more numbers than $num
    blacklist             => \@array # omit words in this array
    whitelist             => \@array # keep only words in this array

=item set_parsing_method( $METHOD )

This controls what classes of words are extracted from a document. 
Default is 'nouns'; other values include: 'proper_nouns', 'noun_phrases',
'adjectives', 'verbs'

=item set_default_encoding( $ENCODING )

This sets the encoding to use when indexing text. The default encoding
is set to ISO-8859-1 (latin1). Everything will be converted to utf8.

=item index( $ID, $TEXT, [$WEIGHT=1] )

This method will read the text, extract nouns, apply any filters and add
the data to the semantic index.

=item index_file( $FILENAME, [$WEIGHT=1] )

See `index` above

=item reindex( $ID, $TEXT, [$WEIGHT=1] )

If the text for this document has changed, the old one will be removed
and the new document will be added to the index.

=item reindex_file( $FILENAME, [$WEIGHT=1] )

See `reindex` above

=item unindex( $ID )

Remove this document (or term) from the index

=item finish()

VERY IMPORTANT! This will save the entire index to the storage medium. 
If you do not call this function, nothing will be saved.

=item merge( $FIRST => $SECOND )

Merge the two documents or terms. The $FIRST item will be merged into 
the $SECOND. (All reference to the $FIRST item will be removed.)

=item 

=back

=head2 Searching

Additional optional parameters (with default values):

    depth          => 4    # depth of graph traversal
    trials         => 100  # number of trials for random walk
    keep_top_edges => 0.3  # percent of edges kept before traversal
                           # set this to `1' to do no pruning

=over

=item semantic_search( $QUERY )

Parameters: query is the raw search query from a user.

=item keyword_search( $QUERY )

Parameters: same as above, however the search results are returned using a 
simple keyword search, versus a Semantic search.

=item find_similar( @DOCUMENT_IDS )

Parameters: same as above, however the search begins on the given document 
node(s) rather than a term node.

=item summarize( @DOCUMENT_IDS )

Returns a summary of the given document(s). If more than one document
is provided, it will find the best summary for the document set.

=item get_document_text( $DOCUMENT_ID )

Returns the text of the given document



=back

=head2 Utilities

These are exported by Semantic::API by request only

=over

=item Semantic::API::have_sqlite()

Returns true if SQLite support is enabled

=item Semantic::API::have_mysql()

Returns true if MySQL support is enabled

=back

=head1 SEE ALSO

For more information, please visit http://www.knowledgesearch.org

=head1 AUTHORS

    Aaron Coburn, <acoburn@middlebury.edu>
    Gabe Schine, <gschine@middlebury.edu>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2006 by Aaron Coburn and Gabe Schine

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.6 or,
at your option, any later version of Perl 5 you may have available.


=cut
