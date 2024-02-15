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

my %attr = (
PrintError => 0,
RaiseError => 0,
);
my $dbh = DBI->connect("DBI:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], \%attr);
ok(defined $dbh, "Connected to database");


is($dbh->do(qq{CREATE TABLE CompanyInfo (ID IsNT, CompanyName VARCHAR(64), Address VARCHAR(100))}), undef, "CREATE TABLE CompanyInfo (ID IsNT, CompanyName VARCHAR(64), Address VARCHAR(100)");
is($dbh->err, 196609, "Check error number");
#print $dbh->errstr."\n";


my $sth;

ok($sth = $dbh->prepare("INSERT INTO ? VALUES (8, null, null)"), "INSERT INTO ? VALUES (8, null, null)");
is($sth->execute("companyinfo"), undef, "Execute unsuccessful");
is($dbh->err(), 196609, "Check error number");
is($sth->err(), 196609, "Check error number");



is($dbh->do(qq{DROP TABLE CompanyInfo}), undef, "DROP TABLE CompanyInfo");
is($dbh->err, 262147, "Check error number");
#print $dbh->errstr."\n";

ok($dbh->disconnect(), "Discoonected from database");

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
