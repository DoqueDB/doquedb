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
# FakeErrorTest
# usage: bash FakeErrorTest.sh <scriptnumber> <countmin> <countstep> <countmax>

#default
fe_mn=""
cov=0
covstr=""

while getopts 'ce:' opt ; do
  case $opt in
    c) cov=1; covstr="-c" ;;
    e) fe_mn="-e $OPTARG"; outdir=$OPTARG ;;
    *) cat << _EOF_ 
Bad Argument.
_EOF_
       exit 1 ;;
  esac
done
shift `expr $OPTIND - 1`

for i in `seq $2 $3 $4`; do
   echo FakeErrorTest1.sh $covstr $fe_mn $1 $i
   bash FakeErrorTest1.sh $covstr $fe_mn $1 $i
done

if [ $cov -eq 1 ] ; then
  cp coverage/merged_result.cfy coverage/fake/$outdir/$1_$2_$3_$4.cfy
fi

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
