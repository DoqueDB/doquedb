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
use Config;

#test if bigint can be used in sql retrieval statement
if($Config{use64bitint} eq 'define' || $Config{longsize} >= 8)
{
	print "[INFO]\nYour perl support 64bit integer\n"; 
}
else
{
	print "[WARNING]Your perl DO NOT support 64bit integer,so BIGINT data type in TRMeister can NOT be used in Sql retrieval statement\n";
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
