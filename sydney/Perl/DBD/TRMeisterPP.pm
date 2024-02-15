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
package DBD::TRMeisterPP;

use strict;

use DBI;
use Carp;
use vars qw($VERSION $err $errstr $state $drh);
use Data::Dumper;
$VERSION = '16.5.0';
$err = 0;
$errstr = '';
$state = undef;
$drh = undef;

#####################################################
# DBD::TRMeisterPP::driver -- Get driver handle
# args:
#	none
# return:
#	Driver handle
#	Die		Error
sub driver
{
    return $drh if $drh;

    my $class = shift;
    $class .= '::dr';

    $drh = DBI::_new_drh($class, {
	Name        => 'TRMeisterPP',
	Version     => $VERSION,
	Err         => \$DBD::TRMeisterPP::err,
	Errstr      => \$DBD::TRMeisterPP::errstr,
	State       => \$DBD::TRMeisterPP::state,
	Attribution => 'DBD::TRMeisterPP by Ricoh',
			 }, {}) or die"Can not create driver\n";
}

#####################################################
# DBD::TRMeisterPP::_parse_dsn -- Parse DSN (parse database hostname port only)
# args:
#	$dsn		dsn string
# return:
#	Refference of the hash containing imformation in DSN
#	undef		Fail
sub _parse_dsn
{
    my $class = shift;
    my $dsn = shift;
    my($dsnhash,$pair);
    return undef if ! defined $dsn;
    my @list = split /[:;]/,$dsn;
    foreach $pair (@list)
    {
        $pair =~ /(database|host|port)=(.*)/;
        $dsnhash->{$1} = $2;
    }
    return $dsnhash;
}


package DBD::TRMeisterPP::dr;
$DBD::TRMeisterPP::dr::imp_data_size = 0;

use Net::TRMeister;
use strict;

#####################################################
# DBD::TRMeisterPP::dr::connect -- Connect to database
# args:
#	$dsn		DSN string
#	$attrhash	Hash of attribute:
#				dumpLargeObject: 1=>dump the binary data   0=> just print "size = **" (default)
#				largeObjectBufferSize: If dump the binary data, how much memory is allocated for object.
#				truncOverBufferSize: 1=>trunc data if it exceed buffer size   0=> not trunc
#				debug: 1=> output debug info   0=> no debug info
# return:
#	Database handle
#	undef		Fail
sub connect
{
    my $drh = shift;

    my ($dsn, $user, $password,$attrhash) = @_;

    return undef if !defined($dsn);

    my $data_source_info = DBD::TRMeisterPP->_parse_dsn($dsn);
    if(defined($attrhash->{AutoCommit}) && $attrhash->{AutoCommit} == 0)
    {
        die "Set AutoCommit is not Supported\n" ;
    }
    my $dbh = DBI::_new_dbh($drh, {
        Name         => $dsn,
        BegunWork    => 0,
        USER => $user,
        CURRENT_USER => $user,
    }, {});

    my $TRMeister = Net::TRMeister->new(
        hostname => $data_source_info->{host},
        port     => $data_source_info->{port},
        database => $data_source_info->{database},
        protocolVersion => undef,
        user => $user,
        password => $password,
        #dumpLargeObject =>1:dump the binary data 0: just print "size = **"
        dumpLargeObject => $attrhash->{dumpLargeObject}||0,
        #largeObjectBufferSize => if dump the binary data,how manay memory is allocated for object
        largeObjectBufferSize => $attrhash->{largeObjectBufferSize}||1024,
        #longtruncOk => if the size of binary data exceeds the largeObjectBufferSize,should we trunc it (1)or not(0)
        truncOverBufferSize => $attrhash->{truncOverBufferSize}||1,
        debug    => $attrhash->{debug},
        #user 's encoding
        userEncode => $attrhash->{userEncode},
    );
    $dbh->STORE(TRMeisterpp_connection => $TRMeister);

    if (defined $TRMeister->{errnum})
    {
        return $dbh->set_err($TRMeister->{errnum}, $TRMeister->{errstr});
    }
    return $dbh;
}

#####################################################
# DBD::TRMeisterPP::dr::data_sources -- Return all datasources
# args:
#	none
# return:
#	"dbi:TRMeisterPP:"
sub data_sources
{
    return ("dbi:TRMeisterPP:");
}

#####################################################
# DBD::TRMeisterPP::dr::disconnect_all -- Disconnect all
# args:
#	none
# return:
#	none
sub disconnect_all
{}

package DBD::TRMeisterPP::db;

$DBD::TRMeisterPP::db::imp_data_size = 0;
use strict;


#####################################################
# DBD::TRMeisterPP::db::_count_param -- Count number of params
# args:
#	$statement	Statement.
# return:
#	Number of params
sub _get_param_count
{
    my @statement = split //, shift; # select from --> "select" "from"
    my $num = 0;

    while (defined(my $c = shift @statement))
    {
	if ($c eq "'")
	{
	    my $end = $c;
	    while (defined(my $c = shift @statement))
	    {
		last if $c eq $end;
	    }
	}
	elsif ($c eq '?')
	{
	    $num++;
	}
    }
    return $num;
}

#####################################################
# DBD::TRMeisterPP::db::prepare -- Prepare statement
# args:
#	$statement	Statement
#	@attribs	Attributes, now it is none.
# return:
#	Statement handle
sub prepare
{
    my $dbh = shift;
    my ($statement, @attribs) = @_;
    my $TRMeister = $dbh->FETCH('TRMeisterpp_connection');

    my $sth = DBI::_new_sth($dbh, {
	Statement => $statement,
	PrepareID => undef,
	ErrorNum => undef,
	ErrorMessage => undef,
			    })or die"Can not create statement\n";

    $sth->STORE(TRMeisterpp_handle => $TRMeister);
    $sth->STORE(TRMeisterpp_params => []);
    $sth->STORE(NUM_OF_PARAMS => _get_param_count($statement));
    $sth;
}



#####################################################
# DBD::TRMeisterPP::db::do -- Prepare and execute
# args:
#	$statement	Statement
#	$attr		Attribute. Now it is undef
#	@params		List of binded params
# return:
#	Affected rows
#	"0E0"		Success but affected rows = 0
#	undef		Fail
sub do
{
    my($dbh, $statement, $attr, @params) = @_;
    my $TRMeister = $dbh->FETCH('TRMeisterpp_connection');
    my $sth;

    return $dbh->set_err($TRMeister->{errnum},$TRMeister->{errstr}) if !defined ($sth = $dbh->prepare($statement, $attr));

    my $result = $sth->execute(@params);
    if ($sth->{NUM_OF_FIELDS}) {
	# need fetch
	my $rows = 0;
	while(defined($sth->fetch))
	{
	    $rows++;
	}
	return $dbh->set_err($TRMeister->{errnum},$TRMeister->{errstr}) if $sth->err;
	return ($rows == 0) ? "0E0" : $rows;
    }
    return $result;
}

#####################################################
# DBD::TRMeisterPP::db::begin_work -- Start transaction
# args:
#	$start_transaction	Statement to start transaction
# return:
#	1	success
#	undef	Fail
sub begin_work
{
    my $dbh = shift;
    my $start_transaction = shift;

    if  ($dbh->FETCH('BegunWork') eq 1)
    {
	return $dbh->set_err(1, "Active SQL-transaction");
    }
    else
    {

	my $sth = undef;
	return undef if !defined($sth = $dbh->prepare($start_transaction));
	return undef if !defined($sth->execute());
	$dbh->STORE('BegunWork',  1); # trigger post commit/rollback action
    }
    return 1;
}

#####################################################
# DBD::TRMeisterPP::db::commit -- Commit
# args:
#	none
# return:
#	1	Success
#	undef	Fail
sub commit
{
    my $dbh = shift;

    if (($dbh->FETCH('BegunWork') eq 0))
    {
	return $dbh->set_err(1, "Not in transaction");
    }
    else
    {
	my $sth = undef;
	return undef if !defined($sth = $dbh->prepare('COMMIT'));
	return undef if !defined($sth->execute());
	$dbh->STORE('BegunWork',0);
    }

    return 1;
}

#####################################################
# DBD::TRMeisterPP::db::rollback -- Rollback
# args:
#	none
# return:
#	1	Success
#	undef	Fail
sub rollback
{
    my $dbh = shift;

    if ($dbh->FETCH('BegunWork') eq 0)
    {
	return $dbh->set_err(1, "Not in transaction");
    }
    else
    {
	my $sth = undef;
	return undef if !defined($sth = $dbh->prepare('ROLLBACK'));
	return undef if !defined($sth->execute());
	$dbh->STORE('BegunWork',0);
    }
    return 1;
}

#####################################################
# DBD::TRMeisterPP::db::rollback -- Rollback
# args:
#	none
# return:
#	1	Success
#	undef	Fail
sub disconnect
{
    my $dbh = shift;

    if ($dbh->FETCH('BegunWork') eq 1)
    {
	return undef if !defined $dbh->rollback();
    }
    my $TRMeister = $dbh->FETCH('TRMeisterpp_connection');
    return undef if !defined $TRMeister;
    return undef if !defined $TRMeister->cancel_prev_access_result;
    return undef if !defined $TRMeister->close;
    return 1;
}

#####################################################
# DBD::TRMeisterPP::db::FETCH -- Fetch key
# args:
#	$key	Key name
# return:
#	Key value
sub FETCH
{
    my $dbh = shift;
    my $key = shift;
    return $dbh->{AutoCommit} if $key =~ /^AutoCommit$/;
    return $dbh->{$key} if $key =~ /^(?:TRMeisterpp_.*)$/;
    return $dbh->SUPER::FETCH($key);
}

#####################################################
# DBD::TRMeisterPP::db::STORE -- Store key
# args:
#	$key	Key name
#	$new	Key value
# return:
#	defined	Success
#	undef	Fail
sub STORE
{
    my $dbh = shift;
    my ($key, $new) = @_;
    if ($key =~ /^AutoCommit$/)
    {
	$dbh->{$key} = $new;
	return 1;
    }
    if ($key =~ /^BegunWork$/)
    {
	$dbh->{$key} = $new;
	return 1;
    }
    if ($key =~ /^TRMeisterpp_/) {
	$dbh->{$key} = $new;
	return 1;
    }

    return $dbh->SUPER::STORE($key, $new);
}

#####################################################
# DBD::TRMeisterPP::db::DESTROY -- Destructor
# args:
#	none
# return:
#	none
sub DESTROY
{
    my $dbh = shift;
    $dbh->disconnect();
}

package DBD::TRMeisterPP::st;

$DBD::TRMeisterPP::st::imp_data_size = 0;
use strict;

#####################################################
# DBD::TRMeisterPP::st::bind_param -- Bind param
# args:
#	$index	Index of param, start from 1.
#	$value	Value
#	$attr	Attribute, now it is undef.
# return:
#	1	Success
sub bind_param
{
    my $sth = shift;
    my ($index, $value, $attr) = @_;
    #my $type = (ref $attr) ? $attr->{TYPE} : $attr;
    #if ($type)
    #{
    #	my $dbh = $sth->{Database};
    #	$value = $dbh->quote($sth, $type);
    #}
    my $params = $sth->FETCH('TRMeisterpp_params');
    $params->[$index - 1] = $value;
    1;
}

#####################################################
# DBD::TRMeisterPP::st::execute -- Execute statement
# args:
#	Binded params
# return:
#	1	Success
#	undef	Fail
sub execute
{
    my $sth = shift;
    my @bind_values = @_;
    my $params = (@bind_values) ?
	\@bind_values : $sth->FETCH('TRMeisterpp_params');

    my $num_param = $sth->FETCH('NUM_OF_PARAMS');

    my $statement = $sth->{Statement};
    my $TRMeister = $sth->FETCH('TRMeisterpp_handle');

    $TRMeister->cancel_prev_access_result;
    if($num_param != 0)
    {
	if(!defined $sth->{PrepareID})
	{
	    my $prepareid = $TRMeister->prepare_statement($statement);

	    return $sth->set_err($TRMeister->{errnum},$TRMeister->{errstr}) if !defined $prepareid;

	    $sth->{PrepareID} = $prepareid;
	}
	return $sth->set_err($TRMeister->{errnum},$TRMeister->{errstr}) if !defined $TRMeister->execute_preparestatement($sth->{PrepareID},$params);
    }
    else
    {
	return $sth->set_err($TRMeister->{errnum},$TRMeister->{errstr}) if !defined $TRMeister->execute_nonpreparestatement($statement);
    }



    $sth->{TRMeisterpp_resultset} = undef;
    my $resultset = $TRMeister->create_resultset;#-->Net::TRMeister::ResultSet
    $sth->{TRMeisterpp_resultset} = $resultset;

    if($resultset->is_error)
    {
	return $sth->set_err($TRMeister->{errnum},$TRMeister->{errstr});
    }

    $statement =~ s/^\s*--.*$//m; # remove -- comment line
    if ($statement =~ /^\s*(?:select|values)/i) {

	if ($resultset->has_result && !defined $sth->FETCH('NAME') )
	{
	    #may be no column name but has result.e.g. back up command
	    $sth->STORE(NUM_OF_FIELDS => $resultset->get_column_num );
	    $sth->STORE(NAME => $resultset->get_column_names);
	}

	return 1;

    } else {

	my $rows = 0;
	while (defined($resultset->each))
	{
	    $rows++;
	}
	return $sth->set_err($TRMeister->{errnum},$TRMeister->{errstr}) if $resultset->is_error;
	return ($rows == 0) ? "0E0" : $rows;
    }
}

#####################################################
# DBD::TRMeisterPP::st::rows -- Number of rows of result
# args:
#	none
# return:
#	-1	Not support
sub rows
{
    my $sth = shift;

    return -1;
}

#####################################################
# DBD::TRMeisterPP::st::fetch -- Fetch next result
# args:
#	none
# return:
#	Refference to the list containing the values of next row in resultset
#	undef	Fail
sub fetch
{
    my $sth = shift;
    my $TRMeister = $sth->FETCH('TRMeisterpp_handle');

    my $resultset = $sth->FETCH('TRMeisterpp_resultset');
    my $row = $resultset->each;
    return $sth->set_err($TRMeister->{errnum}, $TRMeister->{errstr}) if $resultset->is_error;
    return undef unless $row;

    return $sth->_set_fbav($row);
}
*fetchrow_arrayref = \&fetch;
#####################################################
# DBD::TRMeisterPP::st::finish -- Clear statement envrionment
# args:
#	none
# return:
#	1	Success
#	undef	Fail
sub finish
{
    my $sth = shift;
    my $TRMeister = $sth->FETCH('TRMeisterpp_handle');

    return undef if !defined($TRMeister->cancel_prev_access_result);
    if (defined $sth->{PrepareID})
    {
	return undef if !defined($TRMeister->clean_prepareid_for_preparestatement($sth->{PrepareID}));
    }

    return 1;
}
#####################################################
# DBD::TRMeisterPP::st::FETCH -- Fetch key
# args:
#	$key	Key name
# return:
#	Key value
sub FETCH
{
    my $sth = shift;
    my $key = shift;

    return $sth->{NAME} if $key eq 'NAME';
    return $sth->{$key} if $key =~ /^TRMeisterpp_/;
    return $sth->SUPER::FETCH($key);
}

#####################################################
# DBD::TRMeisterPP::st::STORE -- Store key
# args:
#	$key	Key name
#	$new	Key value
# return:
#	defined	Success
#	undef	Fail
sub STORE
{
    my $sth = shift;
    my ($key, $value) = @_;

    if ($key eq 'NAME')
    {
	$sth->{NAME} = $value;
	return 1;
    }
    elsif ($key =~ /^TRMeisterpp_/)
    {
	$sth->{$key} = $value;
	return 1;
    }
    return $sth->SUPER::STORE($key, $value);
}

#####################################################
# DBD::TRMeisterPP::st::DESTROY -- Destructor
# args:
#	none
# return:
#	none
sub DESTROY
{
    my $sth = shift;
}
package DBD::TRMeisterPP::Utility;
use DBI;
#####################################################
# DBD::TRMeisterPP::Utility::get_string -- return the string format of a tuple
# args:
#	$tuple	the tuple to be dumped
# return:
#	string format of the tuple
sub get_one_string
{
    my $self = shift;
    my $element = shift;

    if(ref($element) eq "ARRAY")
    {
	my $subelement;
	my @subarray;
	foreach $subelement (@{$element})
	{
	    push @subarray, $self->get_one_string($subelement);
	}
	return "{".(join ",",@subarray)."}";
    }
    elsif(ref($element) eq "HASH")
    {
	my @temp;
	foreach (sort keys %$element)
	{
	    push @temp, ($_ . "=>" . $self->get_one_string($element->{$_}));
	}
	return "{" . (join ",", @temp) . "}";
    }
    else
    {
	return defined($element)? $element : '(null)';
    }
}

sub get_string
{
    my $self = shift;
    my $tuple = shift;
    return "" if !defined($tuple);
    my $element;
    my @data_array;
    foreach  $element (@{$tuple})
    {
	push @data_array, $self->get_one_string($element);
    }
    return "{".(join "," ,@data_array)."}";
}
1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
