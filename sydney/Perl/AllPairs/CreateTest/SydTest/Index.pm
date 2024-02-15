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
# AllPairs::CreateTest::SydTest::Index -- index class
# member:
#	name		index name
#	type		index type
#	keys		key column list
#	hint		index hint
#	table		table
######################################################################################################

package AllPairs::CreateTest::SydTest::Index;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Column;
use AllPairs::CreateTest::SydTest::Condition;
use AllPairs::CreateTest::SydTest::Table;

#####################
# constructor
# args:
#	cls		class name
#	name		index name
#	type		index type
#	keys		key column list
#	table		table
#	hint		index hint
#	option		option

sub new ($$$$;$$$)
{
    my $cls = shift;
    my $name = shift;
    my $type = shift;
    my $keys = shift;
    my $table = shift;
    my $hint = shift;
    my $option = shift;

    my $this = {name=>$name, type=>$type, keys=>$keys, table=>$table, hint=>$hint, option=>$option};
    bless $this;

    map {
	die "Illegal key" unless ref($_) eq 'AllPairs::CreateTest::SydTest::Column';
	push(@{$_->{index}}, $this);
    } @$keys;

    $this;
}

#########################################################
# AllPairs::CreateTest::SydTest::Index::copy -- copy the index object
# args:
#	obj	index object
# return:
#	copied object
sub copy($)
{
    my $obj = shift;
    new AllPairs::CreateTest::SydTest::Index($obj->{name}, $obj->{type}, $obj->{keys}, $obj->{table},
					     $obj->{hint}, $obj->{option});
}

#########################################################
# AllPairs::CreateTest::SydTest::Index::getDefinition -- get index definition
# args:
#	obj	index object
# return:
#	character string of index definition
sub getDefinition($)
{
    my $obj = shift;
    my $result;

    $result = 'create ' . $obj->{type} . ' index ' . $obj->{name} . ' on ' . $obj->{table}->{name} . '(';
    $result .= join(', ', map {$_->{name}} @{$obj->{keys}});
    $result .= ')';
    $result .= ' ' . $_->{option} if $_->{option};
    $result .= " hint '" . join(' ', @{$obj->{hint}}) . "'" if $obj->{hint};

    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Index::createCondition -- create Condition object
# args:
#	obj		index object
#	type		creating type
#	option		[optional] additional option
# return:
#	condition object
sub createCondition($$;$)
{
    my $obj = shift;
    my $generatetype = shift;
    my $option = shift;
    my $conditions;
    my $result;
    my @keys = @{$obj->{keys}};

    map {push(@$conditions, $_->createCondition($generatetype, $option))} @keys;

    if ($#$conditions == 0) {
	$result = $conditions->[0];
    } else {
	$result = new AllPairs::CreateTest::SydTest::Condition('and', $conditions);
    }
    $result;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
