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
lang=_jp
btr=
# 引数より実行するディレクトリと内容を得る
while getopts '2e' opt ; do
    case $opt in
    e) lang=_euro ;;
    2) btr=_btr2 ;;
    *) exit 1 ;;
    esac
done
shift `expr $OPTIND - 1`
single=single$lang$btr
multi=multi$btr
recovery=recovery$btr

filter1()
{
    i=${3}.txt
    if [ -e $1/$2/expect/$3.log ] ; then
      perl makerecoverytest.pl < $1/$2/$i > $recovery/$2/$i
      perl makerecoveryanswer.pl < $1/$2/expect/$3.log > $recovery/$2/expect/$3.log
      cat recovery.log >> $recovery/$2/expect/$3.log
    fi
}

# ここから本番
if [ $1 -lt 5000 ] ; then # 5000未満なら
	sorm=$single
else 
	sorm=$multi
fi
if [ `expr $1 % 2` -eq 0 ] ; then # 偶数なら
	nore=normal
else 
	nore=except
fi
# $1は番号
filter1 $sorm $nore $1 

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
