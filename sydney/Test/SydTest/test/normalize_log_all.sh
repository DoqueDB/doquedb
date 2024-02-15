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
pushd $1/$2/result
  for i in [0-9][0-9][0-9][0-9].out
  do
    f=${i%.out}
    if [ $f -lt 5000 ] ; then
      cat ${f}.out | perl ../../../log_normalizer.pl > ../except/${f}.log
    else
      cat ${f}.out | bash ../../../multi_log_normalizer.sh > ../except/${f}.log
    fi
  done
popd
#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
