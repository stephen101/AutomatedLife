# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl Semantic-Engine.t'

#########################

# change 'tests => 1' to 'tests => last_test_to_print';

use Test::More tests => 3;
use strict;

BEGIN { use_ok('Semantic::API') };

#########################

# Insert your test code below, the Test::More module is use()ed here so read
# its man page ( perldoc Test::More ) for help writing this test script.

ok( ( Semantic::API::have_sqlite() == 0 or Semantic::API::have_sqlite() == 1 ), "testing sqlite");
ok( ( Semantic::API::have_mysql() == 0 or Semantic::API::have_mysql() == 1 ), "testing mysql");