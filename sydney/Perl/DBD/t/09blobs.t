# 
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
use Test::More tests => 16;
use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]",  $ARGV[3], $ARGV[4],
	{'RaiseError' => 0, 'PrintError'=>0,'debug' =>0, 'dumpLargeObject' => 0,});
ok(defined $dbh, "Connected to database, dumpLargeObject = 0");

ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100), Description BLOB)}), "Create table with BLOB field");

my $sth;
ok($dbh->do(q{INSERT INTO CompanyInfo VALUES(1, 'CompanyA', 'BC', 'DJSIDJOAS')}), "Insert a record with BLOB");

ok($sth = $dbh->prepare("SELECT * from CompanyInfo where id = 1"), "Prepare");
ok($sth->execute, "Execute");
ok(my $record = $sth->fetch, "Fetch");
is($record->[3], 'size=18', "Print size=18");

ok($dbh->disconnect(), "Discoonected from database");

$dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4],
	{'RaiseError' => 1, 'debug' =>0, 'dumpLargeObject' => 1,});
ok(defined $dbh, "Connected to database, dumpLargeObject = 1");

ok($sth = $dbh->prepare("SELECT * from CompanyInfo where id = 1"), "Prepare");
ok($sth->execute, "Execute");
ok(my $record = $sth->fetch, "Fetch");
is($record->[3], "D\0J\0S\0I\0D\0J\0O\0A\0S\0", "Dump BLOB");
ok($sth->finish(), "Close statement");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
