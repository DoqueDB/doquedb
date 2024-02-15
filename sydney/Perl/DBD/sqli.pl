#!perl
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
#this is a Perl-version Sqli which acts the same role as Sqli.exe in TRMeister
use DBI;
use Getopt::Std;
use strict;

use constant PROMPT          => "\nSqli> ";
use constant WELCOME_MESSAGE => "Welcome to Sqli(Perl version) for TRMeister DBMS. Type 'q' for quit.\n";
use constant QUIT_MESSAGE    => "Bye\n";

my %option;
# read the options with getopts
getopts '?vh:p:c:u:w:', \%option;

my $database = shift;

show_version() if $option{v};
show_usage()   if $option{'?'} || ! defined $database;
$option{h} ||= 'localhost';
$option{p} ||= 54321;

my $dbh = eval {
	DBI->connect(
		"dbi:TRMeisterPP:database=$database;host=$option{h};port=$option{p}",$option{u},$option{w},
		{RaiseError => 0, PrintError => 0,dumplargeobject=>0,debug => 0,});
};
die $DBI::errstr if $@;

print WELCOME_MESSAGE;
print PROMPT;
my $query;
my $sth;
my $ret;
my $row;
while ($query = <>) {
	chomp $query;
	last if $query =~ /^q$/i;
	if ($query !~ /^\s*$/) {

			$sth = $dbh->prepare($query);
			if (!defined($sth))
			{
				print $dbh->errstr();
				print PROMPT;
				next;
			}
			my $ret = $sth->execute;
			if (!defined($ret))
			{
				print $sth->errstr();
				print PROMPT;
				next;
			}
			unless ($sth->{NUM_OF_FIELDS}) {
			    print "$ret rows affected.\n";
			} else {
			    my $names = $sth->FETCH('NAME');
			    print "\n{".(join ",",@{$names})."}\n" if defined($names);
			    while (my $row = $sth->fetch)
			    {
				print DBD::TRMeisterPP::Utility->get_string($row)."\n";
			    }
			    if ($sth->err)
			    {
				print $sth->errstr;
				print PROMPT;
				next;
			    }
			}
	}
	print PROMPT;
}

print QUIT_MESSAGE;
$dbh->disconnect;
exit;


sub show_usage
{
	die <<__USAGE__;
Usage: sqli.pl [-?v] -h HOSTNAME -u USERNAME -w PASSWORD DATABASE

  -?   Display this help and exit.
  -h   Connect to HOSTNAME, HOSTNAME can be IP address or Server machine's name
  -p   Port number
  -v   Output version information and exit.
  -u   User name
  -w   Password

  Example:
   (1)Connect default database "DefaultDB" located at localhost with port=54321,using following cmd:
	> perl sqli.pl -h localhost -p 54321 -u username -w password DefaultDB
   (2)Connect system database '\$\$SystemDB' located at localhost with port=54321,using following cmd:
	> perl sqli.pl -h localhost -p 54321 -u username -w password  \$\$SystemDB
   (3)Connect database "TestDB" located at server machine named "Research56" with port=54321,using following cmd:
	> perl sqli.pl -h Research56 -p 54321 -u username -w password  TestDB
   (4)Connect system database "TestDB" located at server machine with IP address is "123.34.56.78"with port=54321,using following cmd:
	> perl sqli.pl -h 123.34.56.78 -p 54321 -u username -w password  TestDB
__USAGE__
}

sub show_version
{
	die <<__VERSION__;
$0  Version 16.3
__VERSION__
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#

__END__
