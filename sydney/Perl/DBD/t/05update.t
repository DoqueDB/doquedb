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
use Test::More tests => 14;

use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' =>0, 'debug' =>0});
ok(defined $dbh, "Connected to database");


ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

ok($dbh->do(qq{INSERT INTO CompanyInfo values(1, 'CompanyA', 'Street A')}), "Insert record A");

my $sth= $dbh->prepare("SELECT * FROM CompanyInfo WHERE id = 1");
ok($sth->execute(), "Select record A");
my $record = $sth->fetch();
ok($record, "Fetch");
is($record->[1], 'CompanyA', "Check column 2");
ok($sth->finish, "Finish statement handle");

ok($dbh->do("UPDATE CompanyInfo SET CompanyName = 'CompanyB' WHERE ID = 1"), "Update column 2 of record A");
my $sth= $dbh->prepare("SELECT * FROM CompanyInfo WHERE id = 1");
ok($sth->execute(), "Select record A");
my $record = $sth->fetch();
ok($record, "Fetch.");
is($record->[1], 'CompanyB', "Check column 2");
ok($sth->finish, "Finish statement handle");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
