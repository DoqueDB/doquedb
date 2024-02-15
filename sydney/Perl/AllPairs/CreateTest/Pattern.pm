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
# AllPairs::CreateTest::Pattern -- Perl module for creating pattern
# members:
#	target	array reference which hold results
################################################################################

package AllPairs::CreateTest::Pattern;
use base qw(AllPairs::CreateTest);

#####################
# constructor
# args:
#	cls		class
#	name		name
#	delimiter	delimiter
#	argsspec	array reference describing arguments
#	target		target
sub new($$) {
    my $cls = shift;
    my $name = shift;
    my $delimiter = shift;
    my $argsspec = shift;
    my $target = shift;

    die 'Illegal argument' unless ref($argsspec) eq 'ARRAY';
    die 'Illegal argument' unless ref($target) eq 'ARRAY';

    my $this = new AllPairs::CreateTest;
    $this->{name} = $name;
    $this->{delimiter} = $delimiter;
    $this->{argsspec} = $argsspec;
    $this->{target} = $target;
    bless $this;
    $this;
}

#####################################################
# AllPairs::CreateTest::generate -- called for each test patterns
# args:
#	obj	object
#	num	sequence number of test patterns
sub generate ($$)
{
    my $obj = shift;
    my $num = shift;

    my $data = [$obj->{name}];
    my $maxidx = $obj->{maxidx};
    for (my $i = 0; $i < $maxidx; ++$i) {
	push(@$data, $obj->{$i});
    }
    push(@{$obj->{target}}, join($obj->{delimiter}, @$data));
    1;
}

#####################################################
# AllPairs::CreateTest::create --
# args:
#	obj	object
sub create ($)
{
    my $obj = shift;
    my $case;
    my @testcases;
    my $idx = 0;

    for $spec (@{$obj->{argsspec}}) {
	die 'Illegal spec' unless ref($spec) eq 'ARRAY';
	$case = new AllPairs::TestCase($idx);
	$case->addSimplePattern($spec);
	push(@testcases, $case);
	++$idx;
    }
    $obj->{maxidx} = $idx;
    $obj->AllPairs::CreateTest::create(@testcases);
}


1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
