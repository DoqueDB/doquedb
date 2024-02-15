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
use Test::More tests => 10;

use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0, 'PrintError' => 0,'debug' =>0});
ok(defined $dbh, "Connected to database");

ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");
is($dbh->begin_work("start transaction read write"),1,"one transaction is start");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(1, 'ComsfsspanyA', 'Stresfsfet A')}), "Insert record 1");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(2, 'ComsfsspanyB', 'Stresfsfet B')}), "Insert record 2");

is($dbh->do('select * from CompanyInfo'),2,"Two tuple should be here");
#ok($dbh->disconnect(), "Disconnected from database");
print "first connection is not disconnect\n";
print "second connection is begin\n";
$dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 1, 'debug' =>0});
ok(defined $dbh, "Connected to database");
is($dbh->do('select * from CompanyInfo'),'0E0',"no tuple should be here");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");
ok($dbh->disconnect(), "Disconnected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
