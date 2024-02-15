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
# AllPairs::CreateTest::SydTest::Parameter -- parameter class
# member:
#	type		the type of the value
#	seqno		sequence number of parameter
######################################################################################################

package AllPairs::CreateTest::SydTest::Parameter;
use base AllPairs::CreateTest::SydTest::Value;

use AllPairs::CreateTest;
use AllPairs::CreateTest::SydTest;
use AllPairs::CreateTest::SydTest::Type;
use AllPairs::CreateTest::SydTest::Command;

#####################
# constructor
# args:
#	cls		class name
#	type		type object
#	seqno		sequence number of parameter
sub new ($$$)
{
    my $cls = shift;
    my $type = shift;
    my $seqno = shift;

    my $this = new AllPairs::CreateTest::SydTest::Value($type, '?');
    $this->{seqno} = $seqno;
    bless $this;

    $this;
}

#####################################################
# AllPairs::CreateTest::SydTest::Parameter::assign -- assign parameter value
# args:
#	obj	parameter object
#	values	parameter values
# return:
#	corresponding value
sub assign($$)
{
    my $obj = shift;
    my $values = shift;

    if ($obj->{seqno} >= 0 && $obj->{seqno} <= $#{$values}) {
	return $values->[$obj->{seqno}];
    }
    return $obj;
}
1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
