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
###########################################################
# AllPairs::CreateTest::SydTest::Thread -- thread in script
#

package AllPairs::CreateTest::SydTest::Thread;
use base qw(AllPairs::CreateTest::SydTest::Command);

#####################
# new
sub new($$$$)
{
    my $cls = shift;
    my $name = shift;
    my $num = shift;
    my $multi = shift;
    my $out = shift;
    my $expect = shift;
    bless {name=>$name,
	   num=>$num,
	   multi=>$multi,
	   outhandle=>$out,
	   expecthandle=>$expect}, $cls;
}

#####################
# begin - start using
sub begin
{
    my $obj = shift;

    $obj->printBegin;
    $obj->printInitialize;
}

#####################
# end - end using
sub end($)
{
    my $obj = shift;

    $obj->printTerminate;
    $obj->printEnd;
}

sub printBody($)
{
    my $obj = shift;

    $obj->SUPER::printScript($obj->{script});
    if (defined $obj->{expecthandle}) {
	my $expect = $obj->{expecthandle};
	print $expect $obj->{expectlog};
    }
}

###################
# commands

sub printScript($$)
{
    my $obj = shift;
    my $string = shift;

    $obj->{script} .= $string . "\n";
}

sub printExpect($$@)
{
    my $obj = shift;
    my $string = $obj->createExpect(@_);

    $obj->{expectlog} .= $string . "\n" if $string ne '';
}

sub printBegin
{
    my $obj = shift;
    my $name = $obj->{name};
    $obj->printScript("$name\n{");
    $obj->printExpect('INFO',
		      "$name begin.")
}

sub printEnd
{
    my $obj = shift;
    $obj->printScript("}");
    my $name = $obj->{name};
    $obj->printExpect('INFO',
		      "$name end.")
}

sub printInitialize($)
{
    ;
}

sub printTerminate($)
{
    ;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
