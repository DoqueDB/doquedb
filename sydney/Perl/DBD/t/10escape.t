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
use Test::More tests => 24;
use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' =>0, 'debug' =>0});
ok(defined $dbh, "Connect to database");

ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

ok(my $sth = $dbh->prepare("INSERT INTO CompanyInfo VALUES(?,?,?)"), "Prepare for INSERT");
ok($sth->execute(1, 'company\nA', q{address''A}), q{Insert (1, 'company\nA', 'address''A') by prepare()});
ok($sth->execute(4, "company\nD", q{address'D}), q{Insert (4, 'company\nD', 'address'D') by prepare()});
ok($sth->finish(), "Close statement");

ok($dbh->do(q{INSERT INTO CompanyInfo VALUES(2,'company\nB', 'address''B')}), q{Insert (2, 'company\nB', 'address''B') by do()});

is($dbh->do(q{INSERT INTO CompanyInfo VALUES(3,'company C', 'address'C')}), undef, q{Insert (2, 'company C', 'address'C') by do() -> ERROR});


ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE ID = ?"), "Prepare for SELECT");

ok($sth->execute(1), "SELECT * FROM CompanyInfo WHERE ID = 1");
ok(my $record = $sth->fetch, "Fetch");
is($record->[1], 'company\nA', 'company\nA');
is($record->[2], q{address''A}, q{address''A});

ok($sth->execute(2), "SELECT * FROM CompanyInfo WHERE ID = 2");
ok(my $record = $sth->fetch, "Fetch");
is($record->[1], 'company\nB', 'company\nB');
is($record->[2], q{address'B}, q{address'B});

ok($sth->execute(4), "SELECT * FROM CompanyInfo WHERE ID = 4");
ok(my $record = $sth->fetch, "Fetch");
is($record->[1], "company\nD", 'company\nD');
is($record->[2], q{address'D}, q{address'D});

ok($sth->finish(), "Close statement");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), '"Discoonect from database');

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
