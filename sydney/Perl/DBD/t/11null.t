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
use Test::More tests => 47;
use DBI;
use strict;

my $dbh = DBI->connect("DBI:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError'=>0, 'debug' =>0});
ok(defined $dbh, "Connect to database");


ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

ok($dbh->do("INSERT INTO CompanyInfo VALUES(NULL, NULL, 'NULL')"), "Insert (null, null, 'NULL')");
ok(my $sth = $dbh->prepare("INSERT INTO CompanyInfo VALUES(?,?,?)"), "Prepare for INSERT");
ok($sth->execute(undef, undef, 'UNDEF'), "Insert (undef, undef, 'UNDEF')");
ok($sth->bind_param(1, undef), "Bind param 1 undef");
ok($sth->bind_param(2, undef), "Bind param 2 undef");
ok($sth->bind_param(3, 'UNDEF-ANOTHER'), "Bind param 3 undef-another");
ok($sth->execute(), "Execute");

ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE Address like 'NULL'"), "SELECT * FROM CompanyInfo WHERE Address like 'NULL'");
ok($sth->execute());
ok(my $record = $sth->fetch());
is($record->[0], undef, "Compare data");
is($record->[1], undef, "Compare data");
is($record->[2], 'NULL', "Compare data");
ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE Address like 'UNDEF'"), "SELECT * FROM CompanyInfo WHERE Address like 'UNDEF'");
ok($sth->execute);
ok(my $record = $sth->fetch());
is($record->[0], undef, "Compare data");
is($record->[1], undef, "Compare data");
is($record->[2], 'UNDEF', "Compare data");
ok($sth->finish(), "Close statement");


ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE Address like 'UNDEF-ANOTHER'"), "SELECT * FROM CompanyInfo WHERE Address like 'UNDEF'");
ok($sth->execute);
ok(my $record = $sth->fetch());
is($record->[0], undef, "Compare data");
is($record->[1], undef, "Compare data");
is($record->[2], 'UNDEF-ANOTHER', "Compare data");
ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo where ID = ?"), "SELECT * FROM CompanyInfo WHERE ID is ?");
$sth->bind_param(1, undef);
ok($sth->execute());
#ok($record = $sth->fetch());
#print $record->[2];
ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo where ID is NULL"), "SELECT * FROM CompanyInfo WHERE ID is NULL");
ok($sth->execute());
my $n = 0;
while($sth->fetch())
{
	$n++;
}
is($n, 3);
ok($sth->finish(), "Close statement");

ok($sth = $dbh->prepare("UPDATE CompanyInfo SET ADDRESS = ? WHERE ADDRESS like 'UNDEF'"), "UPDATE CompanyInfo SET ADDRESS = ? WHERE ADDRESS like 'UNDEF'");
ok($sth->execute(undef), "? => undef");
ok($sth->finish, "Close statement");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo WHERE ID = 0"), "SELECT * FROM CompanyInfo WHERE ID = 0");
ok($sth->execute());
is(my $record = $sth->fetch(),undef,"empty result");
ok($sth->finish(), "Close statement");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), '"Discoonect from database');

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
