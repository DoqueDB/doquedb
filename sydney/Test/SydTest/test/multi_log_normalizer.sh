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
#for s in `seq 0 50`
for s in `seq 0 9` 100
do
	grep "<<$s>>" $1
done | perl ../../../log_normalizer.pl 
if [ -e recovery_$1 ]
then
	perl ../../../log_normalizer.pl recovery_$1
fi
#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
