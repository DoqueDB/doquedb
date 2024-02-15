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

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' =>0, 'debug' =>0});
ok(defined $dbh, "Connected to database");


ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

my $sth;
my $rows = 0;
ok($sth = $dbh->prepare("INSERT INTO CompanyInfo VALUES (?, ?, ?)"), "Test prepare for query of INSERT");

ok($sth->execute(1, 'CompanyA', 'Address A'));
$rows++;

ok($sth->execute('3', 'CompanyC', 'Address C'));
$rows++;

my $id = 2;
my $companyname = 'CompanyB';
my $address = 'Address B';
ok($sth->execute($id, $companyname, $address));
$rows++;

ok($sth->bind_param(1, 4), "Bind");
ok($sth->bind_param(2, "Company \n D"), "Bind");
ok($sth->bind_param(3, "Address D"), "Bind");
ok($sth->execute());
$rows++;

ok($sth->finish, "Test closing of statement handle");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo"), "Test prepare for query of SELECT");
ok($sth->execute());
while(my $record = $sth->fetch())
{
	print '   '.$record->[0].' , '.$record->[1].' , '.$record->[2]."\n";
}

ok($sth->finish, "Test closing of statement handle");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
