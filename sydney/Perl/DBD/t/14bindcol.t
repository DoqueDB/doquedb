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

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' => 0, 'debug' =>0});
ok(defined $dbh, "Connected to database");

ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

ok($dbh->do(qq{INSERT INTO CompanyInfo values(1, 'CompanyA', 'Street A')}), "Insert record A");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(2, 'CompanyB', 'Street B')}), "Insert record B");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(3, 'CompanyC', 'Street C')}), "Insert record C");

my $sth= $dbh->prepare("SELECT * FROM CompanyInfo order by ID", "Prepare for select");
ok($sth->execute(), "Select records");
my $value;
ok($sth->bind_col(2, \$value), "Bind_col");
ok($sth->bind_col(1, \$value), "Bind_col");
$sth->fetch;
is($value, 'CompanyA');
my $value1;
my $value2;
ok($sth->bind_columns(\$value1, \$value2, \$value), "Bind_Columns");
$sth->fetch;
is($value1, 2, "Check");
is($value2, 'CompanyB', "Check");
is($value, 'Street B', "Check");
$sth->fetch;
is($value1, 3, "Check");


ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
