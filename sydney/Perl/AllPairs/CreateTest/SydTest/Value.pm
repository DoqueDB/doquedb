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
# AllPairs::CreateTest::SydTest::Value -- value class
# member:
#	type		the type of the value
#	value		string representation of value
#	elements	array reference of elements(if isArray)
######################################################################################################

package AllPairs::CreateTest::SydTest::Value;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Type;
use AllPairs::CreateTest::SydTest::Command;

#####################
# constructor
# args:
#	cls		class name
#	type		type object
#	value		[optional] value
#	elements	[optional] array reference of elements
sub new ($$;$$)
{
    my $cls = shift;
    my $type = shift;
    my $value = shift;
    my $elements = shift;

    my $this = {type=>$type, value=>$value, elements=>$elements};
    bless $this;

    $this;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::copy -- copy the value
# args:
#	obj	value object
#	type	[optional] type object
# return:
#	copied object
sub copy($;$)
{
    my $obj = shift;
    my $result;
    if (my $type = shift) {
	$result = new AllPairs::CreateTest::SydTest::Value($type, $obj->{value}, $obj->{elements});
    } else {
	$result = new AllPairs::CreateTest::SydTest::Value($obj->{type}->copy, $obj->{value}, $obj->{elements});
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::isNegative -- check whether the value is negative numeric
# args:
#	obj	value object
# return:
#	1 if the value is negative numeric
sub isNegative($)
{
    my $obj = shift;
    $obj->{type}->isNumeric && $obj->{value} =~ /^-/;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::isNull -- check whether the value is null
# args:
#	obj	value object
# return:
#	1 if the value is null
sub isNull($)
{
    my $obj = shift;
    $obj->{value} eq 'null';
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::getLiteralOutput -- get string representation for literal
# args:
#	obj	value object
# return:
#	string representation for literal
sub getLiteralOutput($)
{
    my $obj = shift;
    my $result;

    my $value = $obj->{value};

    if (($value eq 'null') || ($value eq '?')) {
	$result = $value;

    } elsif ($obj->{type}->isArray) {
	$result = 'ARRAY [';
	$result .= join(',', map {$_->getLiteralOutput} @{$obj->{elements}});
	$result .= ']';

    } elsif ($obj->{type}->isCharacterString) {
	if ($value =~ /^\'(.*)\'$/) {
	    # already it's literal
	    $result = $value;
	} else {
	    # escape quotes and add heading/trailing quotes
	    $value =~ s/\'/\'\'/g;
	    $result = "'" . $value . "'";
	}
    } else {
	# numeric value should be represented smallest type for the value
	if (my $number = AllPairs::CreateTest::SydTest::Value::analyzeNumericString($value)) {
	    if ($number->{exp}) {
		# treat as float
		$result = 0.0 + ($number->{integer} . $number->{fraction} . $number->{exp});
	    } elsif ($number->{fraction}) {
		# with comma
		my $mantissa = $number->{absinteger} . $number->{fractionnum};
		my $precision = length($mantissa);
		if ($obj->{type}->isFloat || $precision <= 15) {
		    # treat as float
		    $result = 0.0 + ($number->{integer} . $number->{fraction});
		} else {
		    # treat as decimal
		    $result = $number->{integer} . $number->{fraction};
		}
	    } else {
		# no comma
		$result = $number->{integer};
	    }
	} else {
	    # never occur
	    die "Can't analyze numeric string: $value";
	}
    }
    $result;
}

#########################################################
# AllPairs::CreateTest::SydTest::Value::getSQL -- get SQL representation of the value
# args:
#	obj	value object
# return:
#	character string of condition description
sub getSQL($;$)
{
    my $obj = shift;
    my $includeTableName = shift;

    $obj->getLiteralOutput;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::castData -- cast a value into another type
# args:
#	obj	Value object of the source data
#	type	Type object of the target
# return:
#	casted Value object

sub castData($$)
{
    my $obj = shift;
    my $type = shift;
    my $result;

    if ($obj->isNull()) {
	$result = $obj->copy($type);

    } elsif ($type->isArray) {
	my $elementtype = $type->getElementType;
	$result = new AllPairs::CreateTest::SydTest::Value($type);
	if ($obj->{type}->isArray) {
	    @{$result->{elements}} = map {$_->castData($elementtype)} @{$obj->{elements}};
	} else {
	    push(@{$result->{elements}}, $obj->castData($elementtype));
	}
    
    } elsif ($type->isCharacterString) {
	my $value = $obj->getOutputFormat($type);
	$result = new AllPairs::CreateTest::SydTest::Value($type, $value);

    } elsif ($type->isFloat) {
	if ($obj->{type}->isCharacterString) {
	    # character string
	    $result = $obj->castNumericString($type);
	} elsif ($obj->{type}->isFloat) {
	    #float
	    $result = $obj->copy($type);
	} else {
	    $obj->{type}->isExactNumeric or die "illegal source type for cast";
	    # integer/decimal
	    $result = $obj->castDecimalToFloat($type);
	}
    } else {
	$type->isExactNumeric or die "illegal target type for cast";
	if ($obj->{type}->isCharacterString) {
	    # character string
	    $result = $obj->castNumericString($type);
	} elsif ($obj->{type}->isFloat) {
	    # float
	    $result = $obj->castFloatToDecimal($type);
	} else {
	    $obj->{type}->isExactNumeric or die "illegal source type for cast";
	    # integer/decimal
	    $result = $obj->castDecimalToDecimal($type);
	}
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::getOutputFormat -- get string representation of the data
# args:
#	obj		Value object
#	type		target Type object
# return:
#	string representation of the value
sub getOutputFormat($$)
{
    my $obj = shift;
    my $type = shift;

    my $result;
    if ($obj->isNull) {
	$result = '(null)';
    } elsif ($type->isArray) {
	my $elementtype = $type->getElementType;
	$result = '{';
	$result .= join(',', map {$_->getOutputFormat($elementtype)} @{$obj->{elements}});
	$result .= '}';
    } else {
	my $source = $obj->{value};
	if ($type->isExactNumeric) {
	    $result = getDecimalOutput($source, $type);

	} elsif ($type->isFloat) {
	    $result = getFloatOutput($source, $type);

	} else { # char/nclob
	    $type->isCharacterString or die "unknown type: " . $type->{notation};

	    if ($source =~ s/^\'(.*)\'$/$1/) {
		$source =~ s/\'\'/\'/g;
	    }
	    if ($obj->{type}->isFloat) {
		$result = getFloatOutput($source, $obj);
	    } else {
		$result = $source;
	    }
	    my $l = length(AllPairs::CreateTest::SydTest::Command::getUnicodeString($result)) / 2;
	    if ($type->{precision} ne 'no limit' && $type->{precision} ne '') {
		if ($type->isFixedCharacterString && $l < $type->{precision}) {
		    $result .= ' ' x ($type->{precision} - $l);
		} elsif ($l > $type->{precision}) {
		    $result = AllPairs::CreateTest::SydTest::Command::cutString($result, $type->{precision} * 2);
		}
	    }
	}
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::getParameterFormat -- get string representation for parameter
# args:
#	obj		Value object
# return:
#	string representation of the value for parameter
sub getParameterFormat($)
{
    my $obj = shift;
    my $result;
    if ($obj->isNull) {
	$result = 'null';
    } elsif ($obj->{type}->isArray) {
	$result = '[';
	$result .= join(',', map {$_->getParameterFormat} @{$obj->{elements}});
	$result .= ']';
    } else {
	my $source = $obj->{value};
	if ($obj->{type}->isInt) {
	    $result = $source;
	} else {
	    $result = '"' . $source . '"';
	}
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::analyzeNumericString -- divide numeric string into parts
# args:
#	value		numeric string
# return:
#	reference to hash map denoting divided numeric string
sub analyzeNumericString($)
{
    my $value = shift;
    my $result;

    # clean up the string
    $value =~ s/^\'(.*)\'$/$1/;
    $value =~ s/^ +//;
    $value =~ s/ +$//;

    my @part = ($value =~ /^([+-]?)([0-9]+)(\.([0-9]+))?([Ee]([+-]?[0-9]+))?/);
    if ($#part >= 0) {
	my $sign = $part[0];
	my $integer = int($part[1]);
	$integer =~ s/^0+([1-9][0-9]*|0)$/$1/;
	my $fraction = $part[3];
	$fraction =~ s/0+$//;
	my $expnum = int($part[5]);

	$result = {
	    absinteger	=>	$integer,
	    integer	=>	$sign . $integer,
	    fractionnum	=>	$fraction,
	    fraction	=>	($fraction ? '.' . $fraction : ''),
	    expnum	=>	$expnum,
	    exp		=>	($expnum ? 'E' . $expnum : '')
	}
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::castNumericString -- convert string to numeric data
# args:
#	obj		Value obuject
#	type		target type
# return:
#	casted Value object
sub castNumericString($$)
{
    my $obj = shift;
    my $type = shift;
    my $result;

    $type->isNumeric or die "Invalid type for castNumericString: " . $type->{notation};

    if (my $number = analyzeNumericString($obj->{value})) {
	my $literal = createLiteral($type, $number->{integer} . $number->{fraction} . $number->{exp});
	$literal->{type}->isNumeric or die "Unexpected non-numeric type: " . $literal->{type}->{notation};
	if ($literal->{type}->equal($type)) {
	    $result = $literal;
	} else {
	    $result = castData($literal, $type);
	}
    } else {
	# invalid numeric string
	$result = new AllPairs::CreateTest::SydTest::Value($type, 'null');
	$result->{exception} = 'Data exception - invalid character value for cast.';
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::castDecimalToFloat -- convert decimal to float
# args:
#	obj		Value obuject
#	type		target type
# return:
#	casted Value object
sub castDecimalToFloat($$)
{
    my $obj = shift;
    my $type = shift;

    $type->isFloat or die "Invalid type for castDecimalToFloat: " . $type->{notation};

    new AllPairs::CreateTest::SydTest::Value($type, 0.0 + $obj->{value});
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::castFloatToDecimal -- convert float to decimal
# args:
#	obj		Value obuject
#	type		target type
# return:
#	casted Value object
sub castFloatToDecimal($$)
{
    my $obj = shift;
    my $type = shift;
    my $result;

    $type->isExactNumeric or die "Invalid type for castFloatToDecimal: " . $type->{notation};

    if (my $number = analyzeNumericString($obj->{value})) {
	my $e = $number->{expnum};
	my $l = length($number->{absinteger});
	my $value;
	my $precision;
	my $scale;
	if ($e == 0) {
	    # no exp or exp==0
	    $value = $number->{integer} . $number->{fraction};
	    $scale = length($number->{fractionnum});
	    $precision = length($number->{absinteger}) + $scale;
	} elsif ($e < 1 - $l) {
	    # negative exp part
	    $value = $number->{sign} . '0.' . ('0' x int(-$l-$e))
		. $number->{absinteger} . $number->{fractionnum};
	    $scale = (-$l-$e) + length($number->{absinteger}) + length($number->{fractionnum});
	    $precision = $scale;
	} else {
	    # positive exp part
	    $value = $number->{absinteger} . $number->{fractionnum};
	    if (length($number->{fractionnum}) < $e) {
		$value = $number->{sign} . $value . ('0' x ($e - length($number->{fractionnum})));
		$scale = 0;
		$precision = length($number->{absinteger}) + $e;
	    } elsif (length($number->{fractionnum}) > $e) {
		$value = $number->{sign} . substr($value, 0, $e + $l) . '.' . substr($value, $e + $l);
		$precision = length($number->{absinteger}) + length($number->{fractionnum});
		$scale = $precision - ($e + $l);
	    }
	}
	my $decimalresult = new AllPairs::CreateTest::SydTest::Value(Type::createDecimal($precision, $scale),
							   $value);
	$result = $decimalresult->castDecimalToDecimal($type);

    } else {
	die "Unexpected numeric string failure: " . $obj->{value};
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::castDecimalToDecimal -- convert decimal to decimal
# args:
#	obj		Value obuject
#	type		target type
# return:
#	casted Value object
sub castDecimalToDecimal($$)
{
    my $obj = shift;
    my $type = shift;
    my $result;

    $type->isExactNumeric or die "Invalid type for castDecimalToDecimal: " . $type->{notation};

    if ($obj->{type}->equal($type)) {
	$result = $obj->copy($type);
    } else {
	my $precision;
	my $scale;
	my $value = $obj->{value};
	if ($obj->{type}->isDecimal) {
	    # precision and scale should match the value
	    $precision = $obj->{type}->{precision};
	    $scale = $obj->{type}->{scale};
	} else {
	    # int or bigint have scale 0
	    $precision = length($value);
	    --$precision if $value =~ /^-/;
	    $scale = 0;
	}
	# check compatibility
	if ($precision - $scale > $type->{precision} - $type->{scale}) {
	    # can't cast
	    $result = new AllPairs::CreateTest::SydTest::Value($type, 'null');
	    $result->{exception} = 'Data exception - numeric value out of range.';
	} else {
	    $value = getDecimalOutput($value, $type);
	    $result = new AllPairs::CreateTest::SydTest::Value($type, $value);
	}
    }
    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::getDecimalOutput -- get string representation of the decimal data
# args:
#	source		source string
#	type		target Type object
# return:
#	string representation of the value

sub getDecimalOutput($$)
{
    my $source = shift;
    my $type = shift;

    $type->isExactNumeric or die "target type have to be a decimal";

    my $result = $source;
    $result =~ s/^([-+]?)0+([0-9])/$1$2/;
    my $i = index($source, '.');
    my $s = ($i < 0) ? 0 : (length($source) - $i - 1);
    if ($s < $type->{scale}) {
	if ($s == 0) {
	    $result .= '.';
	}
	$result .= '0' x ($type->{scale} - $s);

    } elsif ($s > $type->{scale}) {
	if ($type->{scale} == 0) {
	    if ($i >= 0) {
		substr($result, $i) = '';
	    }
	} else {
	    # $i must be greater than 0
	    substr($result, $i + $type->{scale} + 1) = '';
	}
    }
    $result =~ s/^-(0(\.0+)?)$/$1/;

    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::getFloatOutput -- get string representation of the float data
# args:
#	source		source string
#	type		target Type object
# return:
#	string representation of the value

sub getFloatOutput($$)
{
    my $source = shift;
    my $type = shift;

    my $result = $source;
    $result = sprintf('%.14E', $result) unless $result =~ /^(-?[0-9](\.[0-9]+)?)E/;
    $result =~ s/\+//;
    $result =~ s/\.?0*E/E/;
    $result =~ s/E(-)?0*/E$1/;
    $result =~ s/E$/E0/;
    # rouding error
    $result =~ s/9\.9{14}E-309/1E-308/;
    # log_normalizer eliminate 13th and 14th digits
    $result =~ s/^([0-9]+\.[0-9]{12}).*E/$1E/;

    $result;
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::compare -- compare for sorting
# args:
#	obj		value object
#	obj2		value object
# return:
#	compare result

sub compare($$)
{
    my $obj = shift;
    my $obj2 = shift;

    if ($obj->{type}->isNumeric) {
	return $obj->{value} <=> $obj2->{value};
    } else {
	return $obj->{value} cmp $obj2->{value};
    }
}

#####################################################
# AllPairs::CreateTest::SydTest::Value::assign -- assign parameter value
# args:
#	obj	parameter object
#	values	parameter values
# return:
#	corresponding value
sub assign($$)
{
    my $obj = shift;
    my $values = shift;

    return $obj;
}
1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
