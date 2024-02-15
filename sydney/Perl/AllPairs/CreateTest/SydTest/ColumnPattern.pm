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
# AllPairs::CreateTest::SydTest::ColumnPattern -- Perl module for creating column pattern
################################################################################

package AllPairs::CreateTest::SydTest::ColumnPattern;
use base qw(AllPairs::CreateTest);
use AllPairs::CreateTest::SydTest::Column;
use AllPairs::CreateTest::SydTest::Index;
use AllPairs::CreateTest::SydTest::Table;
use AllPairs::CreateTest::SydTest::Type;

#####################
# constructor
# args:
#	cls		class
#	typeargs	array reference holding all possible type
#	indexargs	array reference holding all possible index
#	generateargs	array reference holding all possible generate type for producing data
sub new($$;$$$) {
    my $cls = shift;
    my $typeargs = shift;
    my $indexargs = shift;
    my $generateargs = shift;

    my $this = new AllPairs::CreateTest;
    $this->{typeargs} = $typeargs;
    $this->{indexargs} = $indexargs;
    $this->{generateargs} = $generateargs;
    bless $this;

    $this->setup;
    $this;
}

#####################
# initialize
# args:
#	obj		object
sub initialize ($)
{
    my $obj = shift;

    $obj->{columntypes} = [];
    $obj->{indextypes} = [];
    $obj->{generatetypes} = [];
    1;
}

#####################
# terminate
# args:
#	obj		object
sub terminate ($)
{
    my $obj = shift;

    undef $obj->{type};
    undef $obj->{index};
    undef $obj->{generatetype};
    1;
}

#####################
# generate
# args:
#	obj		object
#	num		sequence number of test patterns
sub generate ($$)
{
    my $obj = shift;
    my $num = shift;

    my $type;

    if ($obj->{type} eq 'int') {
	$type = AllPairs::CreateTest::SydTest::Type::createInt;
    } elsif ($obj->{type} eq 'float') {
	$type = AllPairs::CreateTest::SydTest::Type::createFloat;
    } elsif ($obj->{type} eq 'bigint') {
	$type = AllPairs::CreateTest::SydTest::Type::createBigInt;
    } elsif ($obj->{type} =~ /decimal_([0-9]+)_([0-9]+)/) {
	$type = AllPairs::CreateTest::SydTest::Type::createDecimal($1, $2);
    } elsif ($obj->{type} =~ /([a-z]+)(_([0-9]+))?/) {
	$type = AllPairs::CreateTest::SydTest::Type::createChar($1, $3);
    } else {
	die "Unknown type notation: " . $obj->{type};
    }

    push(@{$obj->{columntypes}}, $type);
    push(@{$obj->{indextypes}}, $obj->{index});
    push(@{$obj->{generatetypes}}, $obj->{generatetype});

    1;
}

#####################
# setup
# args:
#	obj		object
sub setup ($)
{
    my $obj = shift;

    #add patterns according to the type
    my @testcases = ();

    my $case = new AllPairs::TestCase('type');
    $case->addSimplePattern($obj->{typeargs});
    push(@testcases, $case);

    $case = new AllPairs::TestCase('index');
    $case->addSimplePattern($obj->{indexargs});
    push(@testcases, $case);

    $case = new AllPairs::TestCase('generatetype');
    $case->addSimplePattern($obj->{generateargs});
    push(@testcases, $case);

    $obj->create(@testcases);

    1;
}

#####################
# createTable
# args:
#	obj		object
#	tableName	table name (for single) or table name prefix (for multi)
#	mode		single ... create one table having whole columns
#			multi  ... create one table for each column
#	count		the number of records in each table
# return:
#	Hash reference holding results
sub createTable ($$$$)
{
    my $obj = shift;
    my $tableName = shift;
    my $mode = shift;
    my $count = shift;

    my $result;

    if ($mode eq 'single') {
	my $table = new AllPairs::CreateTest::SydTest::Table($tableName);
	$result->{table} = $table;

	my $columnidx = 1;
	for my $columntype (@{$obj->{columntypes}}) {
	    my $column = new AllPairs::CreateTest::SydTest::Column("f$columnidx", $columntype);
	    $column->{name} .= '_' . $column->getTypeDescription();
	    $table->addColumn($column);

	    my $indextype = $obj->{indextypes}->[$columnidx - 1];
	    if ($indextype ne 'none') {
		$column->{name} .= '_' . $indextype;
		my $index = new AllPairs::CreateTest::SydTest::Index("I_" . $tableName . "_" . $column->{name},
								     ($indextype eq 'btree' ? '' : $indextype),
								     [$column], $table);
		push(@{$result->{index}->{$indextype}}, $index);

		if ($indextype eq 'unique') {
		    $column->{unique} = 1;
		}
	    }
	    ++$columnidx;
	}
	$result->{data} = $table->createData($count, $obj->{generatetypes})
	
    } elsif ($mode eq 'multi') {
	my $tables = [];
	my $tableidx = 1;
	for my $columntype (@{$obj->{columntypes}}) {
	    my $table = new AllPairs::CreateTest::SydTest::Table("$tableName$tableidx");
	    my $column = new AllPairs::CreateTest::SydTest::Column("f", $columntype);
	    $table->addColumn($column);

	    my $indextype = $obj->{indextypes}->[$columnidx - 1];
	    if ($indextype ne 'none') {
		my $index = new AllPairs::CreateTest::SydTest::Index("I_$tableName$tableidx\_f",
								     ($indextype eq 'btree' ? '' : $indextype),
								     [$column], $table);
		push(@{$result->{index}->{$indextype}}, $index);
	    }
	    $result->{data} = $table->createData($count, $obj->{generatetypes}->[$tableidx - 1]);
	    push(@$tables, $table);
	    ++$tableidx;
	}
	$result->{tables} = $tables;
    }

    $result;
}
1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
