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
# AllPairs::CreateTest::SydTest::Subquery -- subquery class
# member:
#	from		from tables
#	select		select list
#	where		where condition
#	extra		extra parts(limit, order by etc.)
######################################################################################################

package AllPairs::CreateTest::SydTest::Subquery;
use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Table;
use AllPairs::CreateTest::SydTest::Column;
use AllPairs::CreateTest::SydTest::Condition;
use AllPairs::CreateTest::SydTest::Value;

#####################
# constructor
# args:
#	cls		class name
#	from		from tables
#	select		select list
#	where		where condition
#	extra		extra parts(limit, order by etc.)

sub new ($$$$;$)
{
    my $cls = shift;
    my $from = shift;
    my $select = shift;
    my $where = shift;
    my $extra = shift;

    my $this = {from=>$from, select=>$select, where=>$where, extra=>$extra};
    bless $this;
    $this;
}

#########################################################
# AllPairs::CreateTest::SydTest::Subquery::copy -- copy the subquery object
# args:
#	obj	subquery object
# return:
#	copied object
sub copy($)
{
    my $obj = shift;
    new AllPairs::CreateTest::SydTest::Subquery($obj->{from}, $obj->{select}, $obj->{where}, $obj->{extra});
}

#########################################################
# AllPairs::CreateTest::SydTest::Subquery::getSQL -- get SQL representation of the subquery
# args:
#	obj	subquery object
# return:
#	character string of condition description
sub getSQL($;$)
{
    my $obj = shift;
    my $includeTableName = shift;

    '(' . 'select ' . join(',', map {$_->getSQL($includeTableName)} @{$obj->{select}})
	. ' from ' . join(',', map {$_->{name}} @{$obj->{from}})
	. ($obj->{where} ? ' where ' . $obj->{where}->getSQL($includeTableName) : '')
	. $obj->{extra}
	. ')';
}

1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
