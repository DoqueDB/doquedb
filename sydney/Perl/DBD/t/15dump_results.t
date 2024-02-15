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
use Test::More tests => 11;
use DBI;

use strict;

my $dbh = DBI->connect("dbi:TRMeisterPP:database=$ARGV[2];host=$ARGV[0];port=$ARGV[1];crypt=$ARGV[5]", $ARGV[3], $ARGV[4], {'RaiseError' => 0,'PrintError' =>1, 'debug' =>0});
ok(defined $dbh, "Connected to database");
ok($dbh->do("CREATE TABLE CompanyInfo (ID INT ARRAY[4],Comment varchar(40) ARRAY[no limit], CompanyName VARCHAR(64), Address VARCHAR(100),Rate float,R2 decimal(10,5),ch char(3)) "), "Create table");


ok($dbh->do(qq{INSERT INTO CompanyInfo values(array[1,2,3,4], array['aaaaa','vvvvv','eeeee'],'CompanyA', 'Street A',12.3,222.333,'aaa')}), "Insert record A");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(array[2,3,4,5],array['aaaaa','vvvvv','eeeee'], 'CompanyB', 'Street B',23.4,0.44,'baa')}), "Insert record B");
ok($dbh->do(qq{INSERT INTO CompanyInfo values(array[112,113,14,15],array['aaaa1a','vvvv1v','e1eeee'], 'Company1B', 'Street 1B',23.4,0.44,'baa')}), "Insert record C");

my $sth;
ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo order by ID"), "Prepare for select and dump by dump_results");
ok($sth->execute(), "Select records");
#test dump_results interface
$sth->dump_results(30, "\n", ",");

ok($sth = $dbh->prepare("SELECT * FROM CompanyInfo order by ID"), "Prepare for select and dump by dump_tuple_to_string");
ok($sth->execute(), "Select records");
#test DBD::TRMeisterPP::Utility->dump_tuple_to_string interface
my $tuple;
while($tuple = $sth->fetch)
{
	#print dump_tuple_to_string($tuple)."\n";
	print DBD::TRMeisterPP::Utility->get_string($tuple)."\n";
} 
ok($dbh->do(qq{drop table CompanyInfo}), "Drop table");
ok($dbh->disconnect(),"Disconnect from database");

#this function acts same role as DBD::TRMeisterPP::Utility->get_string
sub dump_tuple_to_string
{
	my $tuple = shift;
	return "" if !defined($tuple);
	my $element;
	my @data_array;
	foreach  $element (@{$tuple})
	{
		if(ref($element) eq "ARRAY")
		{
			my $subelement;
			my @temp_array;
			foreach $subelement (@{$element})
			{
				push @temp_array,DBI::neat($subelement); 	
			}
			push @data_array, "{".(join ",",@temp_array)."}";
		}
		else
		{
			push @data_array, DBI::neat($element);
		}
	}
	return "{".(join "," ,@data_array)."}";
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
