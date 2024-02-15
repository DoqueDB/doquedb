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
#########################################
# AllPairs::CreateTest::SydTest::Command
# Utility to produce commands
# member:
#	expect		expected file's file handle
# methods:
#	printScript	output script
#	printExpect	output expected log
#	printXXX	output XXX command to test script
#			XXX is one of following;
#				Begin
#				End
#				Initialize
#				Terminate
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
#				CreateThread
#				JoinThread
#				Sync
#########################################
package AllPairs::CreateTest::SydTest::Command;

use Encode;
use Encode::Guess qw(euc-jp shiftjis utf8);

my $decoder = guess_encoding("文字");
ref($decoder) or die "Can't guess: $decoder";

#####################
# new
sub new
{
    my $cls = shift;
    bless {}, $cls;
}

#####################
# begin - start using
sub begin($$$$$)
{
    my $obj = shift;
    my $name = shift;
    my $num = shift;
    my $multi = shift;
    my $out = shift;
    my $expect = shift;

    $obj->{name} = $name;
    $obj->{num} = $num;
    $obj->{multi} = $multi;
    $obj->{outhandle} = $out;
    $obj->{expecthandle} = $expect;

    $obj->printBegin;
    $obj->printInitialize;
}

#####################
# end - end using
sub end($;$)
{
    my $obj = shift;
    my $recovery = shift;

    unless ($recovery) {
	$obj->printTerminate;
    }
    $obj->printEnd;
}

######################
# methods
######################

sub printScript($$)
{
    my $obj = shift;
    my $string = shift;

    print $string, "\n";
}

sub printExpect($$@)
{
    my $obj = shift;

    if (defined $obj->{expecthandle}) {
	my $expect = $obj->{expecthandle};
	my $string = $obj->createExpect(@_);
	print $expect $string, "\n" if $string ne '';
    }
}

sub printBegin($)
{
    my $obj = shift;
    $obj->printScript("Begin;");
    $obj->printExpect('INFO', 'Main Start.');
}

sub printEnd($)
{
    my $obj = shift;
    $obj->printScript("End;");
    $obj->printExpect('INFO', 'Main End.');
}

sub printInitialize($)
{
    my $obj = shift;
    $obj->printScript("Initialize;");
    $obj->printExpect('INFO', '[Main] Initialize');
}

sub printTerminate($)
{
    my $obj = shift;
    $obj->printScript("Terminate;");
    $obj->printExpect('INFO', '[Main] Terminate');
}

sub printInitializeSession($;$$$$)
{
    my $obj = shift;
    my $database = shift;
    my $user = shift;
    my $password = shift;
    my $except = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};

    $database = 'TESTDB' unless $database;
    my $sessionid = ($num == 0 ? "" : $num);
    my $outsessionid = ($num == 0 ? "" : "$num, ");
    if (defined $user) {
	$obj->printScript("InitializeSession $sessionid \"$database\" \"$user\" \"$password\";");
    } else {
	$obj->printScript("InitializeSession $sessionid \"$database\";");
    }
    if ($except) {
	$obj->printExpect('INFO',
			  "[$name] InitializeSession",
			  "[SydTest Option] $outsessionid$database");
	$obj->printExpect('ERR', $except);
    } else {
	$obj->printExpect('INFO',
			  "[$name] InitializeSession",
			  "[SydTest Option] $outsessionid$database",
			  'Session Initialize : ' . $num);
    }
}

sub printTerminateSession($)
{
    my $obj = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};

    if ($num == 0) {
	$obj->printScript("TerminateSession;");
	$obj->printExpect('INFO',
			  "[$name] TerminateSession",
			  'Session Terminate : ' . $num);
    } else {
	$obj->printScript("TerminateSession $num;");
	$obj->printExpect('INFO',
			  "[$name] TerminateSession",
			  '[SydTest Option] ' . $num,
			  'Session Terminate : ' . $num);
    }
}

sub printCreatePreparedCommand($$$)
{
    my $obj = shift;
    my $label = shift;
    my $sql = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    my $outsql = $sql;
    $outsql =~ s/\\(.)/$1/g;

    if ($num == 0) {
	$obj->printScript("CreatePreparedCommand \"$label\" \"$sql\";");
    } else {
	$obj->printScript("CreatePreparedCommand $num \"$label\" \"$sql\";");
    }
    $obj->printExpect('INFO',
		      "[$name] CreatePreparedCommand",
		      $sessionnum . "[[Label]] $label",
		      $sessionnum . "[[SQL Query]] $outsql");
}
sub printErasePreparedCommand($$)
{
    my $obj = shift;
    my $label = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    if ($num == 0) {
	$obj->printScript("ErasePreparedCommand \"$label\";");
    } else {
	$obj->printScript("ErasePreparedCommand $num \"$label\";");
    }

    $obj->printExpect('INFO',
		      "[$name] ErasePreparedCommand",
		      $sessionnum . "[[Label]] $label");
}

sub printPreparedCommand($$;$$$)
{
    my $obj = shift;
    my $label = shift;
    my $param = shift;
    my $except = shift;
    my $output = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    if ($num == 0) {
	$obj->printScript("PreparedCommand \"$label\" [$param];");
    } else {
	$obj->printScript("PreparedCommand $num \"$label\" [$param];");
    }

    my $outparam = $param;
    $outparam =~ s/\"//g;
    $outparam =~ s/null/(null)/g;
    $outparam = checkThreshold('{' . $outparam . '}',
			       ': ', ']] ',
			       ']] ');

    $obj->printExpect('INFO',
		      "[$name] PreparedCommand",
		      $sessionnum . "[[Label]] $label",
		      $sessionnum . "[[SQL Parameter" . $outparam);

    if (defined $except && $except ne '') {
	$obj->printExpect('ERR', $except);
    } else {
	if (defined $output) {
	    $obj->printExpect('INFO',
			      (map {$sessionnum . checkThreshold('{' . $_ . '}', '<', '>')} @$output),
			      $sessionnum . "End Of Data.");
	}
	$obj->printExpect('INFO', $sessionnum . 'Success.');
    }
}

sub printCommand($$;$$$)
{
    my $obj = shift;
    my $sql = shift;
    my $param = shift;
    my $except = shift;
    my $output = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    my $command = ($num == 0 ? "Command \"$sql\"" : "Command $num \"$sql\"");
    if (defined $param && $param ne '') {
	$command .= " [$param]";
    }
    $command .=";";
    $obj->printScript($command);

    my $outsql = $sql;
    $outsql =~ s/\\([\"\\])/$1/g;
    $obj->printExpect('INFO',
		      "[$name] Command",
		      $sessionnum . "[[SQL Query]] $outsql");
    if (defined $param && $param ne '') {
	my $outparam = $param;
	$outparam =~ s/\"//g;
	$outparam =~ s/null/(null)/g;
	$outparam =~ s/\[/{/g;
	$outparam =~ s/\]/}/g;
	$obj->printExpect('INFO',
			  $sessionnum . "[[SQL Parameter"
			  . checkThreshold('{' . $outparam . '}',
					   ': ', ']] ',
					   ']] '));
    }
    if (defined $except && $except ne '') {
	if ($sql =~ /^\s*verify/ && defined $output) {
	    $obj->printExpect('INFO',
			      (map {$sessionnum . checkThreshold('{' . $_ . '}', '<', '>')} @$output),
			      $sessionnum . 'End Of Data.');
	}
	$obj->printExpect('ERR', $except);
    } else {
	if (defined $output) {
	    $obj->printExpect('INFO',
			      (map {$sessionnum . checkThreshold('{' . $_ . '}', '<', '>')} @$output),
			      $sessionnum . 'End Of Data.');
	}
	$obj->printExpect('INFO', $sessionnum . 'Success.');
    }
}

sub printSystem($$;$)
{
    my $obj = shift;
    my $command = shift;
    my $output = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};

    $obj->printScript("System \"$command\";");

    $obj->printExpect('INFO',
		      "[$name] System",
		      '[System Parameter] ' . $command);
    if (defined $output) {
	$obj->printExpect('', @$output);
    }
}

sub printCreateUser($$$;$$)
{
    my $obj = shift;
    my $user = shift;
    my $password = shift;
    my $id = shift;
    my $except = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    my $escapeduser = $user;
    my $escapedpassword = $password;
    $escapeduser =~ s/([\"\\])/\\$1/g;
    $escapedpassword =~ s/([\"\\])/\\$1/g;

    my $command = (($num == 0) ?  "CreateUser" : "CreateUser $num");
    if (defined $id) {
	$obj->printScript("$command \"$escapeduser\" \"$escapedpassword\" $id;");
	$obj->printExpect('INFO',
			  "[$name] CreateUser",
			  $sessionnum . "[SydTest Option] $user $password $id");
    } else {
	$obj->printScript("$command \"$escapeduser\" \"$escapedpassword\";");
	$obj->printExpect('INFO',
			  "[$name] CreateUser",
			  $sessionnum . "[SydTest Option] $user $password");
    }

    if (defined $except && $except ne '') {
	$obj->printExpect('ERR', $except);
    }
}

sub printDropUser($$;$$)
{
    my $obj = shift;
    my $user = shift;
    my $behavior = shift;
    my $except = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    my $escapeduser = $user;
    $escapeduser =~ s/([\"\\])/\\$1/g;

    my $command = (($num == 0) ?  "DropUser" : "DropUser $num");
    if (defined $behavior) {
	$obj->printScript("$command \"$escapeduser\" $behavior;");

	$obj->printExpect('INFO',
			  "[$name] DropUser",
			  $sessionnum . "[SydTest Option] $user $behavior");
    } else {
	$obj->printScript("$command \"$escapeduser\";");

	$obj->printExpect('INFO',
			  "[$name] DropUser",
			  $sessionnum . "[SydTest Option] $user");
    }

    if (defined $except && $except ne '') {
	$obj->printExpect('ERR', $except);
    }
}

sub printChangePassword($$$;$)
{
    my $obj = shift;
    my $user = shift;
    my $password = shift;
    my $except = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    my $escapeduser = $user;
    my $escapedpassword = $password;
    $escapeduser =~ s/([\"\\])/\\$1/g;
    $escapedpassword =~ s/([\"\\])/\\$1/g;

    my $command = (($num == 0) ?  "ChangePassword" : "ChangePassword $num");
    $obj->printScript("$command \"$escapeduser\" \"$escapedpassword\";");

    $obj->printExpect('INFO',
		      "[$name] ChangePassword",
		      $sessionnum . "[SydTest Option] $user $password");
    if (defined $except && $except ne '') {
	$obj->printExpect('ERR', $except);
    }
}

sub printChangeOwnPassword($$;$)
{
    my $obj = shift;
    my $password = shift;
    my $except = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};
    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");

    my $escapedpassword = $password;
    $escapedpassword =~ s/([\"\\])/\\$1/g;

    my $command = (($num == 0) ?  "ChangeOwnPassword" : "ChangeOwnPassword $num");
    $obj->printScript("$command \"$escapedpassword\";");

    $obj->printExpect('INFO',
		      "[$name] ChangeOwnPassword",
		      $sessionnum . "[SydTest Option] $password");
    if (defined $except && $except ne '') {
	$obj->printExpect('ERR', $except);
    }
}

sub printCreateThread($$)
{
    my $obj = shift;
    my $threadname = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};

    $obj->printScript("CreateThread \"$threadname\";");
    $obj->printExpect('INFO',
		      "[$name] CreateThread",
		      "[SydTest Option] $threadname");
}

sub printJoinThread($$)
{
    my $obj = shift;
    my $threadname = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};

    $obj->printScript("JoinThread \"$threadname\";");
    $obj->printExpect('INFO',
		      "[$name] JoinThread",
		      "[SydTest Option] $threadname");
}

sub printSync($$;$)
{
    my $obj = shift;
    my $label = shift;
    my $count = shift;

    my $name = $obj->{name};
    my $num = $obj->{num};

    if (defined $count) {
	$obj->printScript("Sync \"$label\" $count;");
    } else {
	$obj->printScript("Sync \"$label\";");
    }
    $obj->printExpect('INFO',
		      "[$name] Sync",
		      "[SydTest Option] $label");
}

sub getUnicodeString($)
{
    my $source = shift;

    encode("UCS-2BE", decode($decoder->name, $source));
}

sub getNativeString($)
{
    my $ucs2 = shift;

    encode($decoder->name, decode("UCS-2BE", $ucs2));
}

sub cutString($$)
{
    my $string = shift;
    my $length = shift;

    my $ucs2 = getUnicodeString($string);
    my $tmp = substr($ucs2, 0, $length);
    getNativeString($tmp);
}

# INFO output can be long, in such case, output is divided into several parts
sub getInfoOutput($)
{
    my $source = shift;
    my $length = 2046;

    my $result = "SydTest::Executor: [INFO] ";
    if (length($source) > $length) {
	# convert source into ucs-2
	
	my $i = 1;
	while (length($source) > $length) {
	    my $ucs2 = getUnicodeString($source);
	    my $tmp = substr($ucs2, 0, $length);
	    substr($ucs2, 0, $length) = "";
	    my $tmpsource = getNativeString($tmp);

	    while (length($tmpsource) < $length) {
		my $c = substr($ucs2, 0, 2);
		my $csource = getNativeString($c);
		if (length($tmpsource) + length($csource) > $length) {
		    last;
		}
		substr($ucs2, 0, 2) = "";
		$tmpsource .= $csource;
	    }

	    $result .= $tmpsource . "\n";
	    $result .= "SydTest::Executor: [INFO $i] ";
	    ++$i;
	    substr($source, 0, length($tmpsource)) = "";
	    $length = 2046;
	}
    }
    $result .= $source;
    $result;
}

sub checkThreshold($$$;$$)
{
    my $source = shift;
    my $prefix = shift;
    my $postfix = shift;
    my $prefix_succeed = shift;
    my $postfix_succeed = shift;

    my $ucs2 = getUnicodeString($source);

    length($ucs2) > 500 * 2 ? $prefix . 'length = ' . (length($ucs2) / 2) . $postfix
	: $prefix_succeed . $source . $postfix_succeed;
}

sub createExpect
{
    my $obj = shift;
    my $category = shift;
    my @string = @_;
    my $string;

    my $num = $obj->{num};

    if ($category eq 'ERR') {
	if ($obj->{multi}) {
	    my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");
	    $string = join("\n",
			   map {"Executor: [ERR] (SydTest) $sessionnum" . "Object No=xxx (XXX::XXX) " . $_}
			   @string);
	} else {
	    $string = join("\n",
			   map {"Executor: [ERR] (SydTest) Object No=xxx (XXX::XXX) " . $_}
			   @string);
	}
    } elsif ($category eq 'INFO') {
	if ($obj->{multi}) {
	    $string = join("\n", map {getInfoOutput($_)} (grep {/<<[0-9]+>>/} @string));
	} else {
	    $string = join("\n", map {getInfoOutput($_)} @string);
	}
    } else {
	my $sessionnum = ($obj->{multi} ? "<<$num>> " : "");
	$string = join("\n", map {$sessionnum . $_} @string);
    }
    $string;
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
