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
# テストスクリプトの正解ファイルを障害回復用に変換

while(<>)
{	   
    if (/^SydTest::[Ee]xecutor: \[INFO\] \[Main\] Initialize\s*$/) {
	print <<_EOB_;
SydTest::Executor: [INFO] [Main] BeginTimeSpan
SydTest::Executor: [INFO] [Time Option] DefaultTimeSpan
SydTest::Executor: [INFO] [Main] Initialize
SydTest::Executor: [INFO] [Main] EndTimeSpan
SydTest::Executor: [INFO] [Time Option] DefaultTimeSpan
SydTest::Executor: [INFO] [TIME] TimeSpan: 
_EOB_
    } elsif (/^SydTest::[Ee]xecutor: \[INFO\] \[Main\] Terminate\s*$/) {
	next;
    } else {
	s/\r//;
	print;
    }
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
