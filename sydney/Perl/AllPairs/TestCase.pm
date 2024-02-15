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
######################################################################################################
# AllPairs::TestCase -- Perl module for representing testcases for all-pair method
# member:
#	name		parameter name
#	pattern		array reference of parameter values
#	func		array reference of action for each parameter value
# methods:
#	new		constructor
#	addPattern	add one choice for parameter
######################################################################################################

package AllPairs::TestCase;

#####################
# constructor(TestCase)
# args:
#	cls		class name
#	name		parameter name
sub new ($$)
{
    my ($cls, $name) = @_;

    my $this = {'name'=>$name, 'pattern'=>[], 'func'=>{}};
    bless $this;

    $this;
}


#####################################################
# AllPairs::TestCase::addPattern -- add one parameter value and corresponding action
# args:
#	obj	object
#	pattern	parameter value
#	func	action for the parameter value
sub addPattern($$;$)
{
    my ($obj, $pattern, $func) = @_;

    push(@{$obj->{'pattern'}}, $pattern);
    if (defined $func) {
	$obj->{'func'}->{$pattern} = $func;
    }
    1;
}

###########################################################################
# AllPairs::TestCase::addSimplePattern -- addPattern according to single parameters
# args:
#	case	object
#	args	parameter values for any test cases(ARRAY reference)
sub addSimplePattern($$)
{
    my $case = shift;
    my $args = shift;

    # define a function returning no-name function
    my $addFunc = sub($$$)
    {
	my $value = shift;
	return sub($$) {
	    my $allcase = shift;
	    my $obj = shift;
	    $obj->{$case->name()} = $value;
	    1;
	}
    };

    # add patterns according to patterns
    for $arg (@{$args}) {
	$case->addPattern($arg, &$addFunc($arg));
    }

    1;
}

###########################################################################
# AllPairs::TestCase::callFunc -- call function
# args:
#	case	object
#	args	parameter values for any test cases(ARRAY reference)
sub callFunc($$$$;$)
{
    my ($obj, $case, $allcase, $test, $num) = @_;

    if (defined $obj->func($case)) {
	&{$obj->func($case)}($allcase, $test, $num)
    }
}

###########################################################################
# AllPairs::TestCase::name/pattern/func -- accessor
# args:
#	obj	object
sub name ($)
{
    my ($obj) = @_;

    $obj->{'name'};
}

sub pattern ($)
{
    my ($obj) = @_;

    $obj->{'pattern'};
}

sub func ($$)
{
    my ($obj, $pattern) = @_;

    $obj->{'func'}->{$pattern};
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
