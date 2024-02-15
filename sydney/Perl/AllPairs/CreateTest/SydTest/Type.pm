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
# AllPairs::CreateTest::SydTest::Type -- type class
# member:
#	notation	type notation
#	precision	precision of the type (for numeric types)
#			maximum length (for string types)
#	scale		scale of the type (for numeric types)
#	cardinality	maximum cardinality (for array types)
######################################################################################################

package AllPairs::CreateTest::SydTest::Type;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Value;

#####################
# constructor
# args:
#	cls		class name
#	notation	type notation
#	precision	precision or length
#	scale		[optional] scale of the type
#	cardinality	[optional] cardinality of the type

sub new ($$$;$$)
{
    my $cls = shift;
    my $notation = shift;
    my $precision = shift;
    my $scale = shift;
    my $cardinality = shift;

    my $this = {notation=>$notation, precision=>$precision, scale=>$scale, cardinality=>$cardinality};
    bless $this;
    $this;
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::copy -- copy the type object
# args:
#	obj	type object
# return:
#	copied object
sub copy($)
{
    my $obj = shift;
    new AllPairs::CreateTest::SydTest::Type($obj->{notation}, $obj->{precision}, $obj->{scale}, $obj->{cardinality});
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::equal -- compare type objects
# args:
#	obj	type object
#	other	compared type object
# return:
#	1 if types are same
sub equal($$)
{
    my $obj = shift;
    my $other = shift;

    if (($obj->{notation} eq $other->{notation})
	&& ($obj->{cardinality} == $other->{cardinality})) {
	if ($obj->isNumeric && !$obj->isDecimal) {
	    return 1;
	} elsif ($obj->isDecimal) {
	    return ($obj->{precision} == $other->{precision})
		&& ($obj->{scale} == $other->{scale});
	} else {
	    return $obj->{precision} == $other->{precision};
	}
    }
    0;
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::createXXX -- create a type
# args:
#	<none>
# return:
#	created object
sub createFloat()
{
    new AllPairs::CreateTest::SydTest::Type('float', 15);
}
sub createInt()
{
    new AllPairs::CreateTest::SydTest::Type('int', 10);
}
sub createBigInt()
{
    new AllPairs::CreateTest::SydTest::Type('bigint', 19);
}
sub createDecimal($$)
{
    my $precision = shift;
    my $scale = shift;
    new AllPairs::CreateTest::SydTest::Type('decimal', $precision, $scale);
}
sub createChar($;$)
{
    my $notation = shift;
    my $length = shift;
    new AllPairs::CreateTest::SydTest::Type($notation, $length);
}

#####################################################
# AllPairs::CreateTest::SydTest::Type::createLiteralType -- get literal type
# args:
#	obj	base type object
#	literal	literal string
# return:
#	Type object corresponding to the literal
sub createLiteralType($$)
{
    my $obj = shift;
    my $literal = shift;

    my $result;

    die 'Array is not supported' if $obj->isArray;

    if ($literal eq 'null') {
	$result = $obj->copy;
    } elsif ($obj->isCharacterString) {
	my $length = length($literal);
	if ($literal =~ s/^\'(.*)\'$/$1/) {
	    # already it's literal
	    my $cnt = ($literal =~ s/\'\'/\'/g);
	    $length -= 2 + $cnt;
	}
	$result = createChar('nchar', $length);
    } else {
	# numeric value should be represented smallest type for the value
	if (my $number = AllPairs::CreateTest::SydTest::Value::analyzeNumericString($literal)) {
	    if ($number->{exp}) {
		# treat as float
		$result = Type::createFloat;
	    } elsif ($number->{fraction}) {
		# with comma
		my $mantissa = $number->{absinteger} . $number->{fractionnum};
		my $precision = length($mantissa);
		if ($obj->isFloat || $precision <= 15) {
		    # treat as float
		    $result = Type::createFloat;
		} else {
		    # treat as decimal
		    $result = createDecimal(length($number->{absinteger}) + length($number->{fraction}),
					    length($number->{fraction}));
		}
	    } else {
		# no comma
		my $absvalue = $number->{absinteger};
		if ($number->{integer} < 0) {
		    # negative value
		    if ($absvalue <= $AllPairs::CreateTest::INTMIN_abs) {
			$result = Type::createInt;
		    } elsif ($absvalue <= $AllPairs::CreateTest::BIGINTMIN_abs) {
			$result = Type::createBigInt;
		    } else {
			$result = Type::createDecimal(length($absvalue), 0);
		    }
		} else {
		    # positive value
		    if ($absvalue <= $AllPairs::CreateTest::INTMAX) {
			$result = Type::createInt;
		    } elsif ($absvalue <= $AllPairs::CreateTest::BIGINTMAX) {
			$result = Type::createBigInt;
		    } else {
			$result = Type::createDecimal(length($absvalue), 0);
		    }
		}
	    }
	} else {
	    $result = $obj->copy;
	    $result->{exception} = 'Data exception - invalid character value for cast.';
	}
    }

    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::setArray -- convert to array type
# args:
#	obj	type object
#	cardinality	max cardinality
# return:
#	none

sub setArray($$)
{
    my $obj = shift;
    my $cardinality = shift;

    $obj->{cardinality} = $cardinality;
    1;
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::isXXX -- check type category
# args:
#	obj	type object
# return:
#	1 when the type matches the category

sub isFloat($)
{
    my $obj = shift;
    $obj->{notation} eq 'float';
}
sub isInt($)
{
    my $obj = shift;
    $obj->{notation} eq 'int';
}
sub isBigInt($)
{
    my $obj = shift;
    $obj->{notation} eq 'bigint';
}
sub isDecimal($)
{
    my $obj = shift;
    $obj->{notation} =~ /dec|numeric/;
}
sub isExactNumeric($)
{
    my $obj = shift;
    $obj->{notation} =~ /int|dec|numeric/;
}
sub isNumeric($)
{
    my $obj = shift;
    $obj->{notation} =~ /int|dec|numeric|float/;
}
sub isCharacterString($)
{
    my $obj = shift;
    $obj->{notation} =~ /char|ntext|nclob/;
}
sub isFixedCharacterString($)
{
    my $obj = shift;
    $obj->{notation} =~ /^n?char$/;
}
sub isAsciiCharacterString($)
{
    my $obj = shift;
    $obj->{notation} =~ /^(var)?char$/;
}
sub isNationalCharacterString($)
{
    my $obj = shift;
    $obj->{notation} =~ /^n(var)?char$|ntext|nclob/;
}
sub isBinaryString($)
{
    my $obj = shift;
    $obj->{notation} =~ /image|binary/;
}
sub isArray($)
{
    my $obj = shift;
    $obj->{cardinality} > 0;
}
sub getElementType($)
{
    my $array = shift;
    my $result = $array->copy;
    $result->{cardinality} = 0;
    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::getDescription -- get type description for create table
# args:
#	obj	object
# return:
#	string description of the type

sub getDescription($)
{
    my $obj = shift;
    my $result = $obj->{notation};

    if ($obj->isDecimal) {
	$result .= '(' . $obj->{precision} . ',' . $obj->{scale} . ')';
    } elsif ($obj->isCharacterString) {
	if ($obj->{precision}) {
	    $result .= '(' . $obj->{precision} . ')';
	}
    }
    if ($obj->isArray) {
	$result .= ' ARRAY[' . $obj->{cardinality} . ']';
    }
    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Type::getMax/Min/OverMax/UnderMin -- get maximum/minimum values
# args:
#	obj	object
# return:
#	string description of the value

sub getMax($)
{
    my $obj = shift;
    my $result;

    if ($obj->isDecimal) {
	if ($obj->{precision} == $obj->{scale}) {
	    $result = '0';
	} else {
	    $result = '9' x ($obj->{precision} - $obj->{scale});
	}
	$result .= '.';
	$result .= '9' x $obj->{scale};
    } elsif ($obj->isInt) {
	$result = $AllPairs::CreateTest::INTMAX;
    } elsif ($obj->isBigInt) {
	$result = $AllPairs::CreateTest::BIGINTMAX;
    } elsif ($obj->isFloat) {
	$result = $AllPairs::CreateTest::DBLMAX_value . 'E' . $AllPairs::CreateTest::DBLMAX_exp;
    } else {
	die "Illegal calling of getMax: " . $obj->{notation};
    }
    $result;
}

sub getMin($)
{
    my $obj = shift;
    my $result;

    if ($obj->isDecimal) {
	$result = '-';
	if ($obj->{precision} == $obj->{scale}) {
	    $result = '0';
	} else {
	    $result .= '9' x ($obj->{precision} - $obj->{scale});
	}
	$result .= '.' if $obj->{scale};
	$result .= '9' x $obj->{scale};
    } elsif ($obj->isInt) {
	$result = $AllPairs::CreateTest::INTMIN;
    } elsif ($obj->isBigInt) {
	$result = $AllPairs::CreateTest::BIGINTMIN;
    } elsif ($obj->isFloat) {
	$result = $AllPairs::CreateTest::DBLMIN_value . 'E' . $AllPairs::CreateTest::DBLMIN_exp;
    } else {
	die "Illegal calling of getMin: " . $obj->{notation};
    }
    $result;
}

sub getOverMax($)
{
    my $obj = shift;
    my $result;

    if ($obj->isDecimal) {
	$result = '1';
	$result .= '0' x ($obj->{precision} - $obj->{scale});
	$result .= '.' if $obj->{scale};
	$result .= '0' x $obj->{scale};
    } elsif ($obj->isInt) {
	$result = $AllPairs::CreateTest::INTMAX;
	++$result;
    } elsif ($obj->isBigInt) {
	$result = $AllPairs::CreateTest::BIGINTMAX;
	++$result;
    } elsif ($obj->isFloat) {
	$result = $AllPairs::CreateTest::DBLMAX_value . 'E' . ($AllPairs::CreateTest::DBLMAX_exp + 1);
    } else {
	die "Illegal calling of getOverMax: " . $obj->{notation};
    }
    $result;
}

sub getUnderMin($)
{
    my $obj = shift;
    my $result;

    if ($obj->isDecimal) {
	$result = '-';
	$result = '1';
	$result .= '0' x ($obj->{precision} - $obj->{scale});
	$result .= '.' if $obj->{scale};
	$result .= '0' x $obj->{scale};
    } elsif ($obj->isInt) {
	$result = $AllPairs::CreateTest::INTMIN_abs;
	++$result;
	$result = '-' . $result;
    } elsif ($obj->isBigInt) {
	$result = $AllPairs::CreateTest::BIGINTMIN_abs;
	++$result;
	$result = '-' . $result;
    } elsif ($obj->isFloat) {
	$result = $AllPairs::CreateTest::DBLMIN_value . 'E' . ($AllPairs::CreateTest::DBLMIN_exp - 1);
    } else {
	die "Illegal calling of getUnderMin: " . $obj->{notation};
    }
    $result;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
