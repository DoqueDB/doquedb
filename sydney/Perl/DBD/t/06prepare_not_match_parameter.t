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
use Test::More tests => 18;
use DBI;
use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError'=> 0, 'debug' =>0});
ok(defined $dbh, "Connected to database");


ok($dbh->do(qq{CREATE TABLE CompanyInfo (ID INT, CompanyName VARCHAR(64), Address VARCHAR(100))}), "Create table");

my $sth = undef;
my $record = undef;

#test for parameter_count < placeholder_count,it must failed
print "\n******************************************
test for parameter_count < placeholder_count,it must FAIL
********************************\n";
ok($sth = $dbh->prepare("INSERT INTO CompanyInfo VALUES (?, ?, ?)"), "Test prepare for query of INSERT");
is($sth->execute(1, "CompanyA"), undef, "Insert one tuple with just 2 parameters");
ok($sth->finish(), "Test closing of statement handle");

#there should be no tuples in table CompanyInfo
ok($sth = $dbh->prepare("select count(*) from CompanyInfo"));
ok($sth->execute());
ok($record = $sth->fetch(), "Fetch the tuple count of CompanyInfo");
is($record->[0], 0,"There is no tuple here");

#test for parameter_count > placeholder_count,it ignore redundant parameter and only insert matched parameters
print "\n******************************************
test for parameter_count > placeholder_count,
it ignore redundant parameter and only insert matched parameters.
And it is successful.
********************************\n";
ok($sth = $dbh->prepare("INSERT INTO CompanyInfo VALUES (?, ?, ?)"), "Test prepare for query of INSERT");
ok($sth->execute(1, "CompanyA","AddressA","OtherInfo"), "Insert one tuple with 4 parameters");
ok($sth->finish(), "Test closing of statement handle");
#there should be one tuple in table CompanyInfo
ok($sth = $dbh->prepare("select count(*) from CompanyInfo"));
ok($sth->execute());
ok($record = $sth->fetch(), "Fetch the tuple count of CompanyInfo");
is($record->[0], 1,"There is only one tuple here");

ok($dbh->do(qq{DROP TABLE CompanyInfo}), "Drop table");
ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
