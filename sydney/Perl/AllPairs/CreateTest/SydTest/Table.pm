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
# AllPairs::CreateTest::SydTest::Table -- table class
# member:
#	name		table name
#	columns		column list
#	primarykey	primary key list
######################################################################################################

package AllPairs::CreateTest::SydTest::Table;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Column;
use AllPairs::CreateTest::SydTest::Value;

#####################
# constructor
# args:
#	cls		class name
#	name		table name
#	columns		column list

sub new ($$;$)
{
    my $cls = shift;
    my $name = shift;
    my $columns = shift;

    my $this = {name=>$name, columns=>$columns};
    bless $this;
    $this;
}

#########################################################
# AllPairs::CreateTest::SydTest::Table::copy -- copy the table object
# args:
#	obj	table object
# return:
#	copied object
sub copy($)
{
    my $obj = shift;
    new AllPairs::CreateTest::SydTest::Table($obj->{name}, $obj->{columns});
}

#########################################################
# AllPairs::CreateTest::SydTest::Table::addColumn -- add column definition
# args:
#	obj	table object
#	column	column object to be added
#	primarykey 1 if the column is primary key
# return:
#	<none>
sub addColumn($$;$)
{
    my $obj = shift;
    my $column = shift;
    my $primarykey = shift;

    $column->{table} = $obj;
    push(@{$obj->{columns}}, $column);
    push(@{$obj->{primarykey}}, $column) if $primarykey;

    $column->{position} = $#{$obj->{columns}};
}

#########################################################
# AllPairs::CreateTest::SydTest::Table::getDefinition -- get table definition
# args:
#	<none>
# return:
#	character string of table definition
sub getDefinition($)
{
    my $obj = shift;
    my $result;

    my @tableelements = map {$_->getDefinition} @{$obj->{columns}};

    if ($#{$obj->{primarykey}} >= 0) {
	push(@tableelements, 'primary key(' . join(',', map {$_->{name}} @{$obj->{primarykey}}));
    }

    $result = 'create table ' . $obj->{name} . '(';
    $result .= join(', ', @tableelements);
    $result .= ')';

    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Table::getInsert -- get insert statement
# args:
#	obj		table object
#	data		insert data (array reference to array reference)
# return:
#	(character string of insert statement, insert data)
sub getInsert($$)
{
    my $obj = shift;
    my $data = shift;
    my $result;
    my $param;

    $result .= 'insert ' . $obj->{name} . ' values ';
    my $first = 1;
    for my $tuple (@$data) {
	my $oneparam;
	$result .= ',' unless $first;
	$first = 0;
	$result .= '(';
	$result .= join(',', map {'?'} @$tuple);
	$result .= ')';
	push(@$param, join(',', map {$_->getParameterFormat} @$tuple));
    }

    ($result, join(',', @$param));
}

#########################################################
# AllPairs::CreateTest::SydTest::Table::createData -- create data inserted to the table
# args:
#	obj		table object
#	count		number of tuples to be created
#	generatetype	type for generating data
# return:
#	array reference of inserted data, which is also array reference
sub createData($$;$)
{
    my $obj = shift;
    my $count = shift;
    my $generatetype = shift;
    my $result;

    $generatetype = 'auto' unless $generatetype;

    my $garray;
    if (ref($generatetype) eq 'ARRAY') {
	$garray = $generatetype;
	die 'Illegal generate type' unless $#garray != $#{$obj->{columns}};
    } else {
	@$garray = ($generatetype) x ($#{$obj->{columns}} + 1);
    }

    for (my $i = 0; $i < $count; ++$i) {
	my $tuple;
	my $j = 0;
	for my $column (@{$obj->{columns}}) {
	    my $columndata = $column->createData($garray->[$j++], $count);
	    my $data;

	    my $n = $#$columndata + 1;
	    my $checksum = unpack("%32C*", $i . $column->{name} . $column->{table}->{name});

	    if ($column->{type}->isArray) {
		$data = new AllPairs::CreateTest::SydTest::Value($column->{type});
		my $elementtype = $column->{type}->getElementType;

		# add element data
		my $maxelement = $column->{type}->{cardinality};
		$maxelement = 10 if ($maxelement > 10);
		my $elementcount = ($checksum % $maxelement) + 1; # number of elements changes over 1 ~ max
		for (my $j = 0; $j < $elementcount; ++$j) {
		    $checksum = unpack("%32C*", $j . $checksum);
		    push(@{$data->{elements}},
			 new AllPairs::CreateTest::SydTest::Value($elementtype, $columndata->[$checksum % $n]));
		}
	    } else {
		$data = new AllPairs::CreateTest::SydTest::Value($column->{type}, $columndata->[$checksum % $n]);
	    }
	    push(@$tuple, $data);
	}
	push(@$result, $tuple);
    }
    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Table::getResult -- get result data by condition
# args:
#	obj		table object
#	condition	condition object
#	data		all the tuple data
#	parameter array reference holding parameter values
# return:
#	array reference of result data, which is output format of each tuple
sub getResult($$$$)
{
    my $obj = shift;
    my $condition = shift;
    my $data = shift;
    my $parameter = shift;
    my $result = [];

    my @data;
    $condition->getResultOrder($data, $parameter, \@data);

    for my $tuple (@data) {
	if ($condition->evaluate($obj->{columns}, $tuple)) {
	    my @output;
	    for my $i (0..$#$tuple) {
		$obj->{columns}->[$i] or die "Illegal column($i)";
		push(@output, $tuple->[$i]->getOutputFormat($obj->{columns}->[$i]->{type}));
	    }
	    push(@$result, join(',', @output));
	}
    }
    $result;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
