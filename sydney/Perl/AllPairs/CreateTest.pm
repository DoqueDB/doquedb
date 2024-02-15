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
################################################################################
# AllPairs::CreateTest -- Perl module for creating test cases using allpair.pl
# methods:
#	new		constructor
#	create		create all the test cases according to array of TestCase
#       << following methods should be overridden by subclasses >>
#	initialize	called once at the beginning of 'create' method
#	terminate	called once at the end of 'create' method
#	generate	called for each test patterns
################################################################################

package AllPairs::CreateTest;

use AllPairs::Generator;
use AllPairs::TestCase;

###################
# constructor
sub new
{
    my $cls = shift;

    my $this = bless {}, $cls;
    $this;
}

#####################################################
# AllPairs::CreateTest::initialize -- called once at the beginning of 'create' method
# notes:
#	this method should be overridden by subclasses
# args:
#	obj	object
sub initialize ($)
{
    1;
}

#####################################################
# AllPairs::CreateTest::terminate -- called once at the end of 'create' method
# notes:
#	this method should be overridden by subclasses
# args:
#	obj	object
sub terminate ($)
{
    1;
}

#####################################################
# AllPairs::CreateTest::generate -- called for each test patterns
# notes:
#	this method should be overridden by subclasses
# args:
#	obj	object
#	num	sequence number of test patterns
sub generate ($$)
{
    1;
}

#####################################################
# AllPairs::CreateTest::create -- create all the test cases using allpair.pl
# args:
#	obj		object
#	testcases	array of TestCase objects
sub create ($@)
{
    my ($obj, @testcases) = @_;
    my $num;
    my $tmp = "TMP$obj";
    my $filename = "temp$obj.txt";
    $filename =~ s/:/_/g;

    $obj->initialize();

    open ($tmp, ">$filename") || die "Can't open $filename: $!";
    print $tmp join("\t", map ($_->name(), @testcases)), "\n";

    $num = 0;
    while (1)
    {
	my @patterns = ();
	my $found = 0;
	foreach $entry (@testcases)
	{
	    if (defined $entry->pattern()->[$num]) {
		$found = 1;
		push(@patterns, $entry->pattern()->[$num]);
	    } else {
		push(@patterns, '');
	    }
	}
	last if ($found == 0);

	print $tmp join("\t", @patterns), "\n";
	++$num;
    }
    close $tmp;

    my $allpairobj = new AllPairs::Generator;
    my $allpair = $allpairobj->generate($filename);

    #while (<$allpair>)
    for (@$allpair)
    {
	last if /PAIRING DETAILS/;
	if (/^(\d+)\t(.*)$/)
	{
	    my $num = $1;
	    my @cases = split(/\t/, $2);

	    pop @cases;
	    map (s/^~//, @cases);

	    my $index = 0;
	    my $allcase = {};
	    map ($allcase->{$testcases[$index++]->name()} = $_, @cases);

	    $index = 0;
	    map ($testcases[$index++]->callFunc($_, $allcase, $obj, $num), @cases);

	    $obj->generate($num);
	}
    }
    #close $allpair;

    $obj->terminate();

    unlink($filename);

    1;
}

#######################################
# Constant values
#######################################
$INTMAX = '2147483647';
$INTMIN_abs = '2147483648';
$INTMIN = '-2147483648';
$BIGINTMAX = '9223372036854775807';
$BIGINTMIN_abs = '9223372036854775808';
$BIGINTMIN = '-9223372036854775808';
$DBLMAX_value = '1.79769313486231';
$DBLMAX_exp = '308';
$DBLMIN_value = '2.22507385850721';
$DBLMIN_exp = '-308';

#######################################
# Utility methods
#######################################

###########################################################################
# AllPairs::CreateTest::addSimplePattern -- addPattern according to single parameters
# args:
#	obj	object
#	args	parameter values for any test cases(ARRAY reference)
#	case	TestCase object(return value)
sub addSimplePattern($$$)
{
    my $obj = shift;
    my $args = shift;
    my $case = shift;

    # define a function returning no-name function
    my $addFunc = sub($$$)
    {
	my $key = shift;
	my $value = shift;
	return sub($$) {
	    my $allcase = shift;
	    my $obj = shift;
	    $obj->{$key} = $value;
	    push(@{$obj->{selection}}, "$key -> $value");
	    1;
	}
    };

    # add patterns according to patterns
    $obj->{selection} = [];
    for $arg (@{$args}) {
	$case->addPattern($arg, &$addFunc($case->name(), $arg));
    }

    1;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
