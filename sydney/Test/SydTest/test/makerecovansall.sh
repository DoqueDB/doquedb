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
filter()
{
  pushd $1/$2
    files=`echo [0-9]???.txt`
  popd
  for i in $files; do
    f=${i%.txt}
#    if [ -e $1/$2/expect/${f}.log ] ; then
    if [ -e $1/$2/$i ] ; then
      perl makerecoveryanswer.pl < $1/$2/expect/${f}.log > $3/$2/expect/${f}.log
	cat recovery.log >> $3/$2/expect/${f}.log
    fi
  done
}

filter_dir()
{
  filter $1 normal $2
  filter $1 except $2
}

clean()
{
  rm -f recovery/normal/expect/????.log
  rm -f recovery/except/expect/????.log
}

clean
filter_dir single recovery
filter_dir multi  recovery

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
