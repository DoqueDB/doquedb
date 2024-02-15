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
#test.sh
#batch test sript for Linux 
#for each *.t file under current directory,execute it with the arguments  (1)hostname (2)port number (3) Database (4)username (5)password
#modify above arguments if necessary
for file in *.t
do
perl $file localhost 54321 defaultDB root doqadmin
#perl $file localhost 54321 defaultDB root doqadmin true
done

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
