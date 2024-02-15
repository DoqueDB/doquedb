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
use Test::More tests => 40;

use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' =>0, 'debug' =>0});
ok(defined $dbh, "Connected to database");

ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

my $rows = 0;
ok($dbh->begin_work("start transaction read write"), "Start transaction read write");
ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;
ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;

my $sth;
my $record;

ok($sth = $dbh->prepare("SELECT COUNT(*) from CompanyInfo where id = 1"), "Prepare query of select");
ok($sth->execute(), "Execute");
ok($record = $sth->fetch(), "Fetch");
is($record->[0], $rows, "If counts == $rows");
ok($sth->finish(), "Close statement");

ok($dbh->rollback(), "Rollback");
$rows--;
$rows--;

ok($sth = $dbh->prepare("SELECT COUNT(*) from CompanyInfo where id = 1"), "Prepare query of select");
ok($sth->execute(), "Execute");
ok($record = $sth->fetch(), "Fetch");
is($record->[0], $rows, "If counts == $rows");
ok($sth->finish(), "Close statement");

ok($dbh->begin_work("start transaction read write"), "Start transaction read write");
ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;
ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;

ok($dbh->disconnect(), "Discoonected from database without commit");
$rows--;
$rows--;

$dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 1, 'debug' =>0});
ok(defined $dbh, "Connected to database again");

ok($sth = $dbh->prepare("SELECT COUNT(*) from CompanyInfo where id = 1"), "Prepare query of select");
ok($sth->execute(), "Execute");
ok($record = $sth->fetch(), "Fetch");
is($record->[0], $rows, "If counts == $rows");
ok($sth->finish(), "Close statement");

ok($dbh->begin_work("start transaction read write"), "Start transaction read write");
ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;
ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;
ok($dbh->commit(), "Commit");

ok($dbh->do("INSERT INTO CompanyInfo VALUES (1, 'CompanyA', 'StreetA')"), "INSERT record A");
$rows++;

ok($dbh->disconnect(), "Discoonected from database with commit");

$dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 1, 'debug' =>0});
ok(defined $dbh, "Connected to database");

ok($sth = $dbh->prepare("SELECT COUNT(*) from CompanyInfo where id = 1"), "Prepare query of select");
ok($sth->execute(), "Execute");
ok($record = $sth->fetch(), "Fetch");
is($record->[0], $rows, "If counts == $rows");
ok($sth->finish(), "Close statement");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
