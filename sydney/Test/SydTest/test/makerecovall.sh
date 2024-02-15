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
sub() {
  for i in $1/$2/????.txt ; do
     n=${i%.txt}
     n=${n#$1/$2/}
     echo $n
     bash makerecov1.sh $n
  done
}

sub single normal
sub single except
sub multi normal
sub multi except

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
