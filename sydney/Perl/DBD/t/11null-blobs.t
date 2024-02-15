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
use Test::More tests => 20;
use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' => 0, 'debug' =>0, 'dumpLargeObject' => 1});
ok(defined $dbh, "Connect to database");


ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyDescription BLOB, Address VARCHAR(100))}), "Create table with BLOB");

ok($dbh->do("INSERT INTO CompanyInfo VALUES(NULL, NULL, 'NULL')"), "Insert (null, null, 'NULL')");
ok(my $sth = $dbh->prepare("INSERT INTO CompanyInfo VALUES(?,?,?)"), "Prepare for INSERT");
ok($sth->execute(undef, undef, 'UNDEF'), "Insert (undef, undef, 'UNDEF')");
ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE Address like 'NULL'"), "SELECT * FROM CompanyInfo WHERE Address like 'NULL'");
ok($sth->execute());
ok(my $record = $sth->fetch());
is($record->[0], undef, "Compare data");
is($record->[1], undef, "Compare data");
ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE Address like 'UNDEF'"), "SELECT * FROM CompanyInfo WHERE Address like 'UNDEF'");
ok($sth->execute);
ok(my $record = $sth->fetch());
is($record->[0], undef, "Compare data");
is($record->[1], undef, "Compare data");
ok($sth->finish(), "Close statement");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), '"Discoonect from database');

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
