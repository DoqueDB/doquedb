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
# AllPairs::CreateTest::SydTest::Condition -- condition class
# member:
#	operator	predicate operator
#	operands	operands
#	option		additional description
######################################################################################################

package AllPairs::CreateTest::SydTest::Condition;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest::Column;
use AllPairs::CreateTest::SydTest::Type;
use AllPairs::CreateTest::SydTest::Value;

#####################
# constructor
# args:
#	cls		class name
#	operator	predicate operator
#	operands	operands
#	option		additional description
#	result		hash holding expected result data

sub new ($$$;$$)
{
    my $cls = shift;
    my $operator = shift;
    my $operands = shift;
    my $option = shift;
    my $result = shift;

    my $this = {operator=>$operator, operands=>$operands, option=>$option, result=>$result, generatetype=>$operator};

    $this->{operator} =~ s/like.*/like/;
    bless $this;
    $this;
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::copy -- copy the condition object
# args:
#	obj	condition object
# return:
#	copied object
sub copy($)
{
    my $obj = shift;
    new AllPairs::CreateTest::SydTest::Condition($obj->{operator}, $obj->{operands}, $obj->{option}, $obj->{result});
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::getSQL -- get SQL representation of the condition
# args:
#	obj	condition object
# return:
#	character string of condition description
sub getSQL($;$)
{
    my $obj = shift;
    my $includeTableName = shift;
    my $result;

    my $n = $#{$obj->{operands}};
    if ($obj->{operator} eq 'freetext') {
	$result .= getOperandSQL($obj->{operands}->[0], $includeTableName);
	$result .= ' contains freetext(';
	$result .= getOperandSQL($obj->{operands}->[1], $includeTableName);
	$result .= ')';
    } elsif ($obj->{operator} eq 'wordlist') {
	$result .= getOperandSQL($obj->{operands}->[0], $includeTableName);
	$result .= ' contains wordlist(';
	my @operands = @{$obj->{operands}};
	shift(@operands);
	$result .= join(',', map {getOperandSQL($_, $includeTableName)} @operands);
	$result .= ')';
    } elsif ($obj->{operator} eq 'not') {
	$result = $obj->{operator} . '(' . getOperandSQL($obj->{operands}->[0], $includeTableName) . ')';
    } elsif ($obj->{operator} eq 'between') {
	$result = getOperandSQL($obj->{operands}->[0], $includeTableName);
	$result .= ' between ';
	$result .= getOperandSQL($obj->{operands}->[1], $includeTableName);
	$result .= ' and ';
	$result .= getOperandSQL($obj->{operands}->[2], $includeTableName);
    } elsif ($obj->{operator} eq 'in') {
	$result = getOperandSQL($obj->{operands}->[0], $includeTableName);
	$result .= ' in (';
	my @rest = @{$obj->{operands}};
	shift(@rest);
	$result .= join(',', map {getOperandSQL($_, $includeTableName)} @rest);
	$result .= ')';
    } elsif ($obj->{operator} eq 'exists') {
	$result = 'exists (' . getOperandSQL($obj->{operands}->[0], $includeTableName);
	$result .= ' where ' . $obj->{operands}->[1]->getSQL(1) . ')';
    } elsif ($obj->{operator} eq 'and') {
	    $result = join(' ' . $obj->getOperatorSQL . ' ',
			   map {'(' . getOperandSQL($_, $includeTableName) . ')'} @{$obj->{operands}});
    } else {
	if ($n == 0) {
	    $result .= getOperandSQL($obj->{operands}->[0], $includeTableName);
	    $result .= ' ';
	    $result .= $obj->getOperatorSQL;
	} else {
	    $result = join(' ' . $obj->getOperatorSQL . ' ',
			   map {getOperandSQL($_, $includeTableName)} @{$obj->{operands}});
	}
    }

    if ($obj->{option}) {
	$result .= ' ';
	$result .= $obj->{option};
    }

    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::getOperandSQL -- get SQL representation of an operand
# args:
#	operand
# return:
#	character string of operand
sub getOperandSQL($;$)
{
    my $operand = shift;
    my $includeTableName = shift;

    if (ref($operand) eq 'AllPairs::CreateTest::SydTest::Column'
	||
	ref($operand) eq 'AllPairs::CreateTest::SydTest::Condition'
	||
	ref($operand) eq 'AllPairs::CreateTest::SydTest::Value'
	||
	ref($operand) eq 'AllPairs::CreateTest::SydTest::Parameter'
	||
	ref($operand) eq 'AllPairs::CreateTest::SydTest::Subquery') {
	return $operand->getSQL($includeTableName);
    } elsif (ref($operand) eq 'ARRAY') {
	if (ref($operand->[0]) eq 'AllPairs::CreateTest::SydTest::Table') {
	    return 'select * from ' . join(',', map {$_->{name}} @$operand);
	} elsif (ref($operand->[0]) eq 'AllPairs::CreateTest::SydTest::Column') {
	    return '(' . join(',', map {$_->getSQL($includeTableName)} @${operand}) . ')';
	} else {
	    die 'Invalid type of array operand: ' . ref($operand->[0]);
	}
    } else {
	die 'Invalid type of operand: ' . ref($operand);
    }
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::getOperatorSQL -- get SQL representation of an operand
# args:
#	obj	condition object
# return:
#	character string of operator
sub getOperatorSQL($)
{
    my $obj = shift;

    if ($obj->{reverse}) {
	my $operator = $obj->{operator};
	$operator =~ tr/<>/></;
	return $operator;
    } else {
	return $obj->{operator};
    }
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::setResult -- set expected result data
# args:
#	obj	condition object
#	data	hash holding expected result data
# return:
#	none
sub setResult($$)
{
    my $obj = shift;
    my $data = shift;

    die "Illegal result data: " . ref($data) unless ref($data) eq "HASH";

    $obj->{result} = $data;
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::getResultOrder -- order data with condition's index
# args:
#	obj	condition object
#	data	array reference holding result data
#	parameter array reference holding parameter values
#	result	array reference for return value
sub getResultOrder($$$$)
{
    my $obj = shift;
    my $data = shift;
    my $parameter = shift;
    my $result = shift;

    if ($obj->{operator} eq 'in') {
	# Result of 'in' is ordered in in-value list
	my $columnposition = $obj->{operands}->[0]->{position};
	defined($columnposition) or die "Illegal condition (" . $obj->getSQL . ")";

	my @inoperand = @{$obj->{operands}};
	shift(@inoperand);
	map {$_ = $_->assign($parameter);} @inoperand;

	@$result =
	    sort {
		my $data_a = $a->[$columnposition]->{value};
		my $data_b = $b->[$columnposition]->{value};
		my $idx_a;
		my $idx_b;
		my $i = 0;
		map {
		    if (!(defined $idx_a) && ($_->{value} eq $data_a)) {$idx_a = $i;}
		    if (!(defined $idx_b) && ($_->{value} eq $data_b)) {$idx_b = $i;}
		    ++$i;
		} @inoperand;

		$idx_a <=> $idx_b;
	    } @$data;
    } else {
	my $index = $obj->usedIndex('');
	if ($#$index >= 0) {
	    my $keypos = $index->[0]->{keys}->[0]->{position};
	    @$result =
		sort {
		    $a->[$keypos]->compare($b->[$keypos]);
		} @$data;
	} else {
	    @$result = @$data;
	}
    }
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::evaluate -- evaluate condition
# args:
#	obj	condition object
#	columns	array reference to columns corresponds to tuple
#	tuple	array reference to tuple data
# return:
#	1 if condition match the tuple
sub evaluate($$$)
{
    my $obj = shift;
    my $columns = shift;
    my $tuple = shift;

    if ($obj->{operator} eq 'not') {
	return ($obj->{operands}->[0]->evaluate($columns, $tuple) == 1) ? 0 : 1;
    } elsif ($obj->{operator} eq 'and') {
	for my $operand (@{$obj->{operands}}) {
	    return 0 unless $operand->evaluate($columns, $tuple);
	}
	return 1;
    } elsif ($obj->{operator} eq 'or') {
	for my $operand (@{$obj->{operands}}) {
	    return 1 if $operand->evaluate($columns, $tuple);
	}
	return 0;
    } elsif ($obj->{result}) {
	my $columnposition = $obj->{operands}->[($obj->{reverse} ? 1 : 0)]->{position};
	defined($columnposition) or die "Illegal condition (" . $obj->getSQL . ")";
	my $columndata = $tuple->[$columnposition]->{value};
	if ($obj->{result}->{$columndata}) {
	    return 1;
	}
    }
    return 0;
}

#########################################################
# AllPairs::CreateTest::SydTest::Condition::usedIndex -- evaluated by index?
# args:
#	obj	condition object
#	type	index type
#	not	under not node?
# return:
#	array reference whose elements are candidate index for the condition
sub usedIndex($$;$)
{
    my $obj = shift;
    my $type = shift;
    my $not = shift;

    if ($obj->{operator} eq 'not') {
	return $obj->{operands}->[0]->usedIndex($type, 1 - $not);
    } elsif ($obj->{operator} =~ /^(and|or)$/) {
	my $result = [];
	for my $operand (@{$obj->{operands}}) {
	    # take intersection
	    my $tmp = $operand->usedIndex($type, $not);
	    if ($#$tmp >= 0) {
		if ($#$result < 0) {
		    @$result = @$tmp;
		} else {
		    my %flag;
		    map {$flag{$_} = 1} @$tmp;
		    @$result = grep {$flag{$_} == 1} @$result;
		    last if $#$result < 0;
		}
	    } else {
		$result = [];
		last;
	    }
	}
	return $result;
    } else {
	if ($not == 0 || $obj->{operator} =~ /[<>]|between/) {
	    if ((($obj->{generatetype} =~ /[<>=]|between|likehead/) && ($type eq ''))
		||
		(($obj->{generatetype} =~ /contains|freetext|wordlist|like/) && ($type eq 'fulltext'))) {
		my $column = $obj->{operands}->[($obj->{reverse} ? 1 : 0)];
		defined($column) or die "Illegal condition (" . $obj->getSQL . ")";
		@$result = grep {$_->{type} eq $type} @{$column->{index}};
		return $result;
	    }
	}
    }
    return [];
}
1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
