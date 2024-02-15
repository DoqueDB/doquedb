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
use Test::More tests => 41;
use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError'=>0, 'debug' =>0});
ok(defined $dbh, "Connected to database");

ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

ok($dbh->do(qq{INSERT INTO CompanyInfo values(1, 'CompanyA', 'Street A')}), "Insert record A");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(2, 'CompanyB', 'Street B')}), "Insert record B");


my $sth= $dbh->prepare("SELECT * FROM CompanyInfo order by ID", "Prepare for select");
ok($sth->execute(), "Select records");
my $record = $sth->fetch();
ok($record, "Fetch");
is($record->[0], 1, "Check column 1");
is($record->[1], 'CompanyA', "Check column 2");
is($record->[2], 'Street A', "Check column 3");
ok(my @recordlist = $sth->fetchrow_array(), "Fetch twice");
is($recordlist[0], 2, "Check column 1");
is($recordlist[1], 'CompanyB', "Check column 2");
is($recordlist[2], 'Street B', "Check column 3");
ok($sth->finish, "Finish statement handle");

$sth= $dbh->prepare("SELECT * FROM CompanyInfo order by ID", "Prepare for select");
ok($sth->execute(), "Select records");
$record = $sth->fetchrow_arrayref();
ok($record, "Fetch once");
is($record->[0], 1, "Check column 1");
is($record->[1], 'CompanyA', "Check column 2");
is($record->[2], 'Street A', "Check column 3");
ok($sth->finish, "Finish statement handle");

$sth= $dbh->prepare("SELECT * FROM CompanyInfo order by ID", "Prepare for select");
ok($sth->execute(), "Select records");
is(my $dumpedrows = $sth->dump_results(undef, undef, ";"), 2, "Dump results");
ok($sth->finish, "Finish statement handle");

ok(my @records = $dbh->selectrow_array("SELECT * FROM CompanyInfo order by ID"), 'Select by selectrow_array()');
is($records[1], 'CompanyA', "Check");

ok(@records = $dbh->selectrow_array("SELECT * FROM CompanyInfo WHERE id = ?", undef, 2), 'Select by selectrow_array with bound params');
is($records[1], 'CompanyB', "Check");

my $recordref;
ok($recordref = $dbh->selectall_arrayref("SELECT * FROM CompanyInfo order by ID"), "Select by selectall_arrayref");
is($recordref->[1][1], 'CompanyB', "Check");

ok($recordref = $dbh->selectall_arrayref("SELECT * FROM CompanyInfo WHERE id = ?", undef, 2), "Select by selectall_arrayref");
is($recordref->[0][1], 'CompanyB', "Check");

ok($record = $dbh->selectcol_arrayref("SELECT * FROM CompanyInfo order by ID"), 'Select by selectcol_arrayref');
is($record->[0], 1, "Check");
is($record->[1], 2, "Check");

ok($record = $dbh->selectcol_arrayref("SELECT * FROM CompanyInfo WHERE ID = ?", undef, 2), 'Select by selectcol_arrayref with bound params');
is($record->[0], 2, "Check");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE ID = ?"), "Prepare");
ok($record = $dbh->selectcol_arrayref($sth, undef, 2), 'Select by selectcol_arrayref with bound params and prepared sth');
is($record->[0], 2, "Check");


ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
