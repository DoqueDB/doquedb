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
sngdif() {
  for i in [0-4]???.out; do
    echo $i
    perl ../../../log_normalizer.pl $i >norm
    perl ../../../log_normalizer.pl recovery_$i >>norm
    diff -wi norm ../expect/${i%.out}.log >${i%.out}.dif
  done
  rm norm
}

muldif() {
  for i in [5-9]???.out; do
    echo $i
    #9350は要注意
    ../../../multi_log_normalizer.sh $i >norm
    diff -wi norm ../expect/${i%.out}.log >${i%.out}.dif
  done
  rm norm
}

dirdif() {
  echo $1
  cd $1
  if [ -e normal/result_a ] ; then
    for suffix in a b c d; do
      pushd normal/result_$suffix
      sngdif
      muldif
      popd
    done
  else
    pushd normal/result
   sngdif
    muldif
    popd
  fi
  pushd except/result
  sngdif
  muldif
  popd
  cd ..
}

dirdif single
dirdif multi
dirdif recovery
echo "DONE"

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
