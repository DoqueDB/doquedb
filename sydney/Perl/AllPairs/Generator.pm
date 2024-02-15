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
# AllPairs::Generator -- Perl module for generating allpairs
######################################################################################################

package AllPairs::Generator;

######################################################################################################
# AllPairs::Generator is built from following free software
######################################################################################################

# ALLPAIRS, by James Bach, www.satisfice.com
# Version 1.21
# Copyright (C) 2001, 2023, James Bach (james@satisfice.com)

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#
# This program attempts to find the smallest number of test cases that
# include all pairings of each variable with each other variable.
#
# All permutations of each variable is easy, all pairs is much harder.
# The way this program works is that it makes a checklist of every pair of
# variables, and checks each pair off as it packs it into a test case.
# The program tries to pack as many un-checked-off pairs into each test case
# as possible. It's kind of like packing boxes: if you're smart you can find
# a combination of object for each box that will result in the minimum of wasted
# space. This program is not smart. It just packs the pairs into the test cases
# until every pair has been packed. It does not result in the smallest number
# of cases that could account for all the pairs, but the set will not be too large.
#
# All permutations of 10 variables of 10 values each results in 10 billion
# test cases. This program packs all pairs into a mere 177 cases. It's not
# optimal, but it's not bad, either.
#
# v1.01: Now compiles clean with strict and warnings turned on; added "don't care" tildes
# v1.1 : Status table now prints which cases go with with pairings.
# v1.2 : Now prints test case variables in the order provided.
#      : Improved error handling for bad tables.
#      : Added more usage information.
#      : Used "our" keyword instead of annoying package qualifiers.
# v1.21: Fixed an outrageous bug with the column order.
#
# TODO: 
#	- read REQUIRED table
#	- use slugs from REQUIRED table before doing the rest.
#	- implement PROHIBITED table

use strict;

my $usage = <<EOS;
Allpairs prepares test cases to cover all pairings of a set of variables.

EXE usage:    allpairs datafile
Script usage: perl allpairs.pl datafile

"datafile" is a tab delimited text file that consists of labelled columns
of variables. The first row of the table is treated as labels.
If you copy from Excel into a text file it will be the right format.

Example: allpairs test.txt

...where test.txt contains:

colors	cars	times
purple	Mazda	night
blue	Ford	day
silver		dawn

...will produce this output:


TEST CASES
case	colors	cars	times	pairings
1	purple	Mazda	night	3
2	purple	Ford	day	3
3	blue	Ford	night	3
4	blue	Mazda	day	3
5	silver	Mazda	dawn	3
6	silver	Ford	night	2
7	purple	Ford	dawn	2
8	blue	~Mazda	dawn	1
9	silver	~Mazda	day	1

PAIRING DETAILS
var1	var2	value1	value2	appearances	cases
colors	times	purple	night	1	1
colors	times	purple	day	1	2
colors	times	purple	dawn	1	7
colors	times	blue	night	1	3
colors	times	blue	day	1	4
colors	times	blue	dawn	1	8
colors	times	silver	night	1	6
colors	times	silver	day	1	9
colors	times	silver	dawn	1	5
colors	cars	purple	Mazda	1	1
colors	cars	purple	Ford	2	2, 7
colors	cars	blue	Mazda	2	4, 8
colors	cars	blue	Ford	1	3
colors	cars	silver	Mazda	2	5, 9
colors	cars	silver	Ford	1	6
times	cars	night	Mazda	1	1
times	cars	night	Ford	2	3, 6
times	cars	day	Mazda	2	4, 9
times	cars	day	Ford	1	2
times	cars	dawn	Mazda	2	5, 8
times	cars	dawn	Ford	1	7

EOS

# constructor
sub new
{
    my $this = {
	neededvalues => [],
	vars => [],
	labelshash => {},
	labels => [],
	listshash => {},
	listorderhash => {},
	listorder => [],
	pairshash => {},
	pairscaseshash => {},
	zeroes => ["","0","00","000"],
	slug => "",
    };
    bless $this;
    $this;
}

sub generate($$)
{
    my $obj = shift;
    my $file = shift || die $usage;
    my $result;

my $count = 1;
#my $file = shift @ARGV || die $usage;
$obj->maketables($file, "tables"); # read the datafile and fill the arrays with each variable.

$obj->populate(); # make the checklists for the pairs.

# This loop creates the "slug" which is the blank test case filled in by the recursive FINDNEXT routine.
#
for (my $c=0;$c<scalar(@{$obj->{vars}});$c++)
{
	$obj->{slug} .= "x\t";
}
chop $obj->{slug};

# print first line of the output table.
#
push(@$result, "TEST CASES\n");
my $line = "case\t";
@{$obj->{labels}} = $obj->gettable("tables","labels");
foreach (sort {$obj->{listorderhash}->{$a} <=> $obj->{listorderhash}->{$b}} @{$obj->{labels}})
{
    $line .= "$_\t";
}
$line .= "pairings\n";
push(@$result, $line);

# find each test case, then show the status of all the pairings
#
while($obj->more())
{
	@{$obj->{neededvalues}} = ();
	my $case = $obj->findnext($obj->{slug});

	push(@$result, $count."\t".$obj->readable($case).$obj->score($case)."\n");
	$obj->checkin($case, $count++);
}
push(@$result, $obj->status());

####################
# END OF MAIN CODE #
####################

$result;
}

# This routine counts the unique pairings in a test case.
#
sub score
{
    my $obj = shift;
	my $score = 0;
	my $case = $_[0];
	my @casevalues = split /\t/, $case;
	my ($c, $v) = 0;
	for ($c=0;$c<scalar(@{$obj->{vars}})-1;$c++)
	{
		for ($v=$c+1;$v<scalar(@{$obj->{vars}});$v++)
		{
			$score++ if (${$obj->{pairshash}->{"$c-$v"}}{$casevalues[$c]."-".$casevalues[$v]} == 0);
		}
	}
	return $score;
}

# This routine records all the new pairings in a test case in the checklists.
#
sub checkin
{
    my $obj = shift;
	my ($c, $v) = 0;
	my $case = $_[0];
	my $casenumber = $_[1];
	my @casevalues = split /\t/, $case;
	for ($c=0;$c<scalar(@{$obj->{vars}})-1;$c++)
	{
		for ($v=$c+1;$v<scalar(@{$obj->{vars}});$v++)
		{
			${$obj->{pairshash}->{"$c-$v"}}{$casevalues[$c]."-".$casevalues[$v]}++;
			push @{${$obj->{pairscaseshash}->{"$c-$v"}}{$casevalues[$c]."-".$casevalues[$v]}}, $casenumber;
		}
	}

}

# This routine creates the checklists of pairs.
#
sub populate
{
    my $obj = shift;
	my ($c, $v, $x, $y) = 0;
	for ($c=0;$c<scalar(@{$obj->{vars}})-1;$c++)
	{
		for ($v=$c+1;$v<scalar(@{$obj->{vars}});$v++)
		{
			for ($x=0;$x<$obj->{vars}->[$c];$x++)
			{
				for ($y=0;$y<$obj->{vars}->[$v];$y++)
				{
					${$obj->{pairshash}->{"$c-$v"}}{$x."-".$y} = 0;
				}
			}
		}
	}
}

# This recursive routine walks through all the values of all the variables, trying to construct
# a test case with the highest number of unique pairings.
#
sub findnext
{
    my $obj = shift;
	my ($c, $v, $x, $y) = 0;
	my $case = shift;
	my @casevalues = split /\t/, $case;
	my @scores = ();
	my @scorestrings = ();
	my $best = "x";

	# find the unfinished part of the test case.
	for ($c=0;$c<scalar(@{$obj->{vars}});$c++)
	{
		last if ($casevalues[$c] eq "x");
	}

	# but if no part is unfinished, then we're done.
	return $case if ($c == scalar(@{$obj->{vars}}));

	# let's walk through the values for the particular variable we have to choose.
	for ($x=0;$x<$obj->{vars}->[$c];$x++)
	{
		@scores = ();

		# let's check the current variable paired against the all the other values.
		for ($v=0;$v<scalar(@{$obj->{vars}});$v++)
		{
			# but don't check it against itself.
			if ($v == $c) {$scores[$v] = 0; next}

			# for any variable we've already chosen, we already know the pairing status
			# and we just add that to the score.
			if ($v < $c)
			{
				$scores[$v] = ${$obj->{pairshash}->{"$v-$c"}}{$casevalues[$v]."-".$x};
			}

			# for the variables we haven't yet chosen, we walk through those values and see what the best pairing score will be. 
			else
			{
				$best = "x";
				for ($y=0;$y<$obj->{vars}->[$v];$y++)
				{
					$best = ${$obj->{pairshash}->{"$c-$v"}}{$x."-".$y} if ($best eq "x" || ${$obj->{pairshash}->{"$c-$v"}}{$x."-".$y} < $best)
				}
				$scores[$v] = $best+0;
			} 
		}

		# now create a sorted string of scores for the value ($x) of current variable ($c) vs. values ($y) of each other variable ($v)
		#foreach (@scores) {print "value:$_\n"}
		foreach (sort @scores)
		{
			$scorestrings[$x] .= $obj->{zeroes}->[4-length($_)].$_."\t";
		}
		chop $scorestrings[$x];
		$scorestrings[$x] .= ":".$obj->{zeroes}->[4-length($x)].$x;
	}

	# the scores for each choice are now in a set of strings of the form {score}:{choice}.
	# the next step is to sort the scorestrings, pick the best, and record that choice...
	$casevalues[$c]  = (split /:/,(sort @scorestrings)[0])[1]+0;

	# this monstrousity of a line of code records whether the best choice is a needed value or not. If the best choice
	# results in no unique pairings, then we call it "N" meaning it's the best choice, but really doesn't matter.
	$obj->{neededvalues}->[$c] = ((sort(split /\t/,(split /:/,(sort @scorestrings)[0])[0]))[1] == 0) ? "Y" : "N";

	# now construct the test case string and call findnext again. 
	$case = "";
	foreach (@casevalues)
	{
		$case .= $_."\t";
	}
	chop $case;

	# after the recursion bottoms out, it will unwind via this return statement.
	return $obj->findnext($case);
}

# This routine displays the status of the pairing checklists.
#
sub status
{
    my $obj = shift;
	my ($c, $v, $x, $y) = 0;
	my @result;
	push(@result, "");
	push(@result, "PAIRING DETAILS\n");
	push(@result, "var1\tvar2\tvalue1\tvalue2\tappearances\tcases\n");
	for ($c=0;$c<scalar(@{$obj->{vars}})-1;$c++)
	{
		for ($v=$c+1;$v<scalar(@{$obj->{vars}});$v++)
		{
			for ($x=0;$x<$obj->{vars}->[$c];$x++)
			{
				for ($y=0;$y<$obj->{vars}->[$v];$y++)
				{
				    my $line = $obj->{labels}->[$c]."\t".
					      $obj->{labels}->[$v]."\t".
					      ($obj->gettable("tables",$c))[$x]."\t".
					      ($obj->gettable("tables",$v))[$y]."\t".
					      ${$obj->{pairshash}->{"$c-$v"}}{$x."-".$y}."\t";
					my $comma = "";
					foreach (@{$obj->{pairscaseshash}->{"$c-$v"}{$x."-".$y}})
					{
						$line .= $comma."$_";
						$comma = ", ";
					}
					$line .= "\n";
				    push(@result, $line);
				}
			}
		}
	}
	@result;
}

# This routine returns true if there are any unpaired variables left to pack into a test case.
#
sub more
{
    my $obj = shift;
	my ($c, $v, $x, $y) = 0;
	for ($c=0;$c<scalar(@{$obj->{vars}})-1;$c++)
	{
		for ($v=$c+1;$v<scalar(@{$obj->{vars}});$v++)
		{
			for ($x=0;$x<$obj->{vars}->[$c];$x++)
			{
				for ($y=0;$y<$obj->{vars}->[$v];$y++)
				{
					if (${$obj->{pairshash}->{"$c-$v"}}{$x."-".$y} == 0) {return 1};
				}
			}
		}
	}
	return 0;
}

# This routine turns a tab delimited table of variables into a set of hashes containing arrays.
#
sub maketables
{
    my $obj = shift;
	# populates array LABELS and hash LISTS indexed by table name. Multiple tables can be processed, that way.
	my ($file, $tablename) = @_;
	my $index = 0;
	my $count = 0;
	my $numoftabs = 0;

	open (DATA, $file) || die "Can't open $file";
	my @data = <DATA>;
	close DATA;
	
	my $label = shift @data;
	if ($label !~ /\t/) {die "Error: The first line of the file must be a tab-delimited list of labels with more than one label in it, and no blank labels.\n"}
	if ($label =~ /\t\t/) {die "Error: Missing column label or extraneous tabs in the first line of the file. The first line of the file must be a tab-delimited list of labels with more than one label in it, and no blank labels.\n"}
	foreach(split /\t/, $label)
	{
		chomp $_;
		if (exists($obj->{listorderhash}->{$_})) {die "Each column must have a unique label. Column \"$_\" is not unique.\n"}
		else 
		{
			$obj->{listorderhash}->{$_} = $count++;
		}
		push @{$obj->{labelshash}->{$tablename}}, $_;
	}
	
	foreach (@data)
	{
		$index = 0;
		$numoftabs = s/\t/\t/g;
		$numoftabs++;
		if ($numoftabs != $count){die "Error in the table. This row:\n\n$_\nhas $numoftabs columns instead of $count.\n\nThe data table should be tab delimited. Each row of the table must have the same number of columns as the first row (the label row). Check for extra tabs or spurious lines in the table.\n"}
		foreach(split /\t/)
		{
			chomp $_;
			if ($_ ne "") {push @{$obj->{listshash}->{${$obj->{labelshash}->{$tablename}}[$index]}}, $_;}
			$index++;
		}
	}

	# reorder the variable lists by size, because the allpairs algorithm works better that way.
	@{$obj->{labelshash}->{$tablename}} = sort {scalar(@{$obj->{listshash}->{$b}}) <=> scalar(@{$obj->{listshash}->{$a}})} @{$obj->{labelshash}->{$tablename}};
	$index = 0;
	foreach (@{$obj->{labelshash}->{$tablename}})
	{
		$obj->{listorder}->[$obj->{listorderhash}->{$_}] = $index++;
	}
	for ($index=0;$index<scalar(@{$obj->{labelshash}->{$tablename}});$index++)
	{
		$obj->{vars}->[$index] = scalar(@{$obj->{listshash}->{${$obj->{labelshash}->{$tablename}}[$index]}});
	}
	#print "\n";
		
}

# To make the code easier to work with, this routine extracts a list from the hash of lists.
# That way, we can do what we want to do by saying $mylist[0] instead of ${$obj->{listshash}->{${$obj->{labelshash}->{$tablename}}[$index]}}[0]
#
sub gettable
{
    my $obj = shift;
	my ($tablename,$index) = @_;
	if ($index eq "labels")
	{ return @{$obj->{labelshash}->{$tablename}} }
	return @{$obj->{listshash}->{${$obj->{labelshash}->{$tablename}}[$index]}};
}

# This routine translates the variable value indexes into their readable labels.
#
sub readable
{
    my $obj = shift;
	my $case = shift;
	my $newcase = "";
	my $t = 0;
	my @list = split /\t/, $case;
	for ($t=0;$t<scalar(@list);$t++)
	{
		if ($obj->{neededvalues}->[$obj->{listorder}->[$t]] eq "N") {$newcase .= "~"} # "~" is the don't care symbol.
		$newcase .= ($obj->gettable("tables",$obj->{listorder}->[$t]))[$list[$obj->{listorder}->[$t]]]."\t";
	}
	return $newcase;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
