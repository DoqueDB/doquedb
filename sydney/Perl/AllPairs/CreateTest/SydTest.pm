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
# AllPairs::CreateTest::SydTest -- Perl module for creating test by SydTest
# member:
#	name		test name
# methods:
#	initialize	override CreateTest::initialize and output Initialize command.
#	terminate	override CreateTest::terminate and output Terminate command.
#	<following methods are inherited from CreateTest::Command class>
#	printExpect	output expected log
#	printXXX	output XXX command to test script
#			XXX is one of following;
#				InitializeSession
#				TerminateSession
#				CreatePreparedCommand
#				ErasePreparedCommand
#				PreparedCommand
#				Command
#				System
#				CreateUser
#				DropUser
#				ChangePassword
#				ChangeOwnPassword
################################################################################

package AllPairs::CreateTest::SydTest;
use base qw(AllPairs::CreateTest AllPairs::CreateTest::SydTest::Command);

use AllPairs::CreateTest::SydTest::Thread;

#####################
# constructor
# args:
#	cls		class
#	args		argument
sub new($$;$$$) {
    my $cls = shift;
    my $name = shift;
    my $multi = shift;
    my $outname = shift;
    my $expectname = shift;

    my $this = new AllPairs::CreateTest;
    $this->{name} = $name;
    $this->{multi} = $multi;
    $this->{out} = $outname;
    $this->{expect} = $expectname;
    bless $this;
    $this;
}

#####################
# initialize
# args:
#	obj		object
#	nodb		if 1, no database is created
sub initialize ($;$)
{
    my $obj = shift;
    my $nodb = shift;

    my $outname = $obj->{out};
    my $expectname = $obj->{expect};

    my $out;
    my $expect;
    if (defined $outname) {
	$out = "AllPairs::CreateTest::SydTest::OUT";
	open($out, ">" . $outname) || die $outname . ": $!";
	select($out);
	$obj->{outhandle} = $out;
    }
    if (defined $expectname) {
	$expect = "AllPairs::CreateTest::SydTest::EXPECT";
	open($expect, ">" . $expectname) || die $expectname . ": $!";
	$obj->{expecthandle} = $expect;
    }

    $obj->begin('Main', 0, $obj->{multi}, $out, $expect);
    $obj->{threads} = [];

    unless ($nodb) {
	$obj->printInitializeSession('TESTDB', $obj->{user}, $obj->{password});
	$obj->printCommand("create database TESTDB");
	$obj->printTerminateSession;
    }
    1;
}

#####################
# terminate
# args:
#	obj		object
#	nodb		if 1, no database is created
sub terminate ($;$)
{
    my $obj = shift;
    my $nodb = shift;

    unless ($nodb) {
	$obj->printInitializeSession('TESTDB', $obj->{user}, $obj->{password});
	$obj->printCommand("drop database TESTDB");
	$obj->printTerminateSession;
    }
    $obj->end;

    for $thread (@{$obj->{threads}}) {
	$thread->printBody;
    }

    if (defined $obj->{outhandle}) {
	close($obj->{outhandle});
	undef $obj->{outhandle};
	select(STDOUT);
    }
    if (defined $obj->{expecthandle}) {
	close($obj->{expecthandle});
	undef $obj->{expecthandle};
    }

    undef $obj->{command};
    1;
}

#####################
# createThread
# args:
#	obj		object
#	name		thread name
# return:
#	Thread object
sub createThread ($;$)
{
    my $obj = shift;
    my $name = shift;
    my $num = $#{$obj->{threads}} + 2;
    my $thread = new AllPairs::CreateTest::SydTest::Thread($name, $num, $obj->{multi},
							   $obj->{outhandle},
							   $obj->{expecthandle});
    push(@{$obj->{threads}}, $thread);
    $obj->printCreateThread($name);
    $thread;
}

#####################
# joinThread
# args:
#	obj		object
#	thread		thread object
sub joinThread ($;$)
{
    my $obj = shift;
    my $thread = shift;

    $obj->printJoinThread($thread->{name});
}

#####################
# createRecovery
# args:
#	obj		object
sub createRecovery ($)
{
    my $obj = shift;

    $obj->end(1);
    for $thread (@{$obj->{threads}}) {
	$thread->printBody;
    }
    if (defined $obj->{outhandle}) {
	close($obj->{outhandle});
	my $rname = $obj->{out};
	$rname =~ s/[.]([^.]+)$/r.\1/;
	die "illegal file name: $rname" if $rname eq $obj->{out};

	open($obj->{outhandle}, ">" . $rname) || die $rname . ": $!";
	select($obj->{outhandle});
    }

    $obj->begin('Main', 0, 0, $obj->{outhandle}, $obj->{expecthandle});
    $obj->{threads} = [];
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
