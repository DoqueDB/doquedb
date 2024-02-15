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
# FakeErrorTest1
# usage: bash FakeErrorTest.sh [-c] [] <scriptnumber> <count>

check_and_mkdir()
{
  if [ ! -d $1 ]; then
    mkdir $1
  fi
}

#default
fe_mn=diskfull 
cov=0
covstr=""

while getopts 'ce:' opt ; do
  case $opt in
    c) cov=1; covstr="c" ;;
    e) fe_mn=$OPTARG ;;
    *) cat << _EOF_ 
Bad Argument.
_EOF_
	exit 1 ;;
  esac
done
shift `expr $OPTIND - 1`

case $fe_mn in
  diskfull)   fe_name=Os::File::write_DiskFull ;;
  memoryfull) fe_name=Os::Memory::allocate_NotEnoughMemory ;;
  mapfull)    fe_name=Os::Memory::map_NotEnoughMemory ;;
    *) cat << _EOF_ 
Non-supported FakeError.
_EOF_
	exit 1 ;;
esac

if [ $1 -lt 5000 ] ; then # 5000未満なら
	sorm=single
else 
	sorm=multi
fi
if [ `expr $1 % 2` -eq 0 ] ; then # 偶数なら
	nore=n
	nore2=normal
else 
	nore=e
	nore2=except
fi

check_and_mkdir fake
check_and_mkdir fake/$fe_mn

if [ $cov -eq 1 ] ; then
  check_and_mkdir coverage/fake
  check_and_mkdir coverage/fake/$fe_mn
fi

bash setreg.sh Exception_FakeError "$fe_name count=$2"
bash SydTest_sh.txt -s $1 -l$covstr$nore $sorm
cp $sorm/$nore2/result/$1.out fake/$fe_mn/$1_$2.out
cp $sorm/$nore2/result/$1.dif fake/$fe_mn/$1_$2.dif
if [ $cov -eq 1 ] ; then
  mv coverage/$nore2/$1.cfy.gz coverage/fake/$fe_mn/$1_$2.cfy.gz 
fi
bash delreg.sh Exception_FakeError

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
