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
#
# 特定のスクリプトを容易に複数回反復できるようにするスクリプト。
#

userecov=0
dump=0
lang=_jp
btr=

while getopts 'cpDr2e' opt ; do
    case $opt in
    c) cp="c$cp" ;;
    p) cp="p$cp" ;;
    D) cp="D$cp"; dump=1 ;;
    r) userecov=1 ;;
    e) lang=_euro ;;
    2) btr=_btr2 ;;
    *) cat << _EOF_ 
usage: bash repeat.sh [-cpr] スクリプト番号 反復回数
  -c : coverage版を使う
  -p : purify版を使う
  -e : data_euroでテストする(指定しないときはdata_jp)
  -2 : SyDrvBtr2.dllでテストする(指定しないときはSyDrvBtr.dll)
  -D : テストごとにダンプを行う。-rをセットしたときのみ有意義
  -r : recovery版スクリプトを実行する
exit 1 ;;
_EOF_
    esac
done

single=single$lang$btr
multi=multi$btr
recovery=recovery$btr

if [ "$cp" == "cp" -o "$cp" == "pc" ]; then
  exit 1;
fi

shift `expr $OPTIND - 1`

if [ $1 -lt 5000 ] ; then # 5000未満なら
	sorm=$single
else 
	sorm=$multi
fi
if [ $userecov -eq 1 ] ; then
	sorm=$recovery
fi

if [ `expr $1 % 2` -eq 0 ] ; then # 偶数なら
	nore=n
	nore2=normal
else 
	nore=e
	nore2=except
fi

for i in `seq 1 $2`; do
  echo Trial $i
  bash SydTest.sh -s $1 -l$cp$nore $sorm
  mv $sorm/$nore2/result/$1.out $sorm/$nore2/result/$1_$i.out
  mv $sorm/$nore2/result/$1.dif $sorm/$nore2/result/$1_$i.dif
  if [ $userecov -eq 1 -a $dump -eq 1 ]; then
    mv d:/dumpdm/${sorm}_${nore2}_$1 d:/dumpdm/${nore2}_$1_$i
  fi
done

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
