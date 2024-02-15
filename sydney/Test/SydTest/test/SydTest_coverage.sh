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
# 定数の設定
testtype=coverage
. env.sh

coveragedir=$testtype
sydtest="$PROGRAMFILES/Rational/Coverage/cache/SydTest\$Coverage_D_${coveragedir}_Sydney_${rationaltools_sydversion}_Tools_setup_sydney.exe"
normal=normal
except=except

cvr_result=$testtop/coverage
sydtestoptions="/c /p /b /s"

if [ ! -e "$sydtest" ] ; then # なければinstrumentする
  "$PROGRAMFILES/Rational/Coverage/Coverage.exe" /run=no $setup/sydney/sydtest.exe
fi	

dotest()
{
    cd $testtop/$dir >/dev/null 2>&1
    if [ $? -ne 0 ] ; then
	return 1
    fi
    cd $testdir
    if [ $? -ne 0 ] ; then
	echo "Cannot go to $dir/$testdir"
    else
	echo "start testing $dir/$testdir"
	for file in $cond; do
		name=`basename $file .txt`
		if [ ! -d './result' ]; then 
		    mkdir ./result
		fi
  		echo coverage $file
		"$sydtest" $sydtestoptions $file >./result/${name}.out

		if [ $dorestore -eq 1 ] ; then
		  bash $testtop/restore_sydtest.sh
		fi
		  #数秒おきにpallingを実行し、coveragew.exeがなくなっていたら先に進むようにする
	    if [ -e ${WINDIR}/system32/tasklist.exe ] ; then
		  while [ `tasklist|gawk 'BEGIN{a=0} /coveragew/{a++} END{print a}'` -gt 0 ] ; do
		    sleep 10
		    echo -n .
		  done
	    else
		  while [ `tlist|gawk 'BEGIN{a=0} /coveragew/{a++} END{print a}'` -gt 0 ] ; do
		    sleep 10
		    echo -n .
		  done
	    fi
		  echo
		    pushd $cvr_result >/dev/null 2>&1
#			for sfx in cfy; do
#			    mv result.$sfx $testdir/${name}.$sfx
#			    rm -f ${name}.${sfx}.gz
#			    gzip -f $testdir/${name}.$sfx
#			done
		    popd >/dev/null 2>&1
	done
    fi
    echo "end testing $dir/$testdir"
	
}

##### main

# パラメータの初期値
list=""
option=0
cond="."
dorestore=1;

# 引数より実行するディレクトリと内容を得る
while getopts 'Naeno:s:' opt ; do
    case $opt in
    N) dorestore=0 ;;
    n) option=1 ;;
    e) option=2 ;;
    a) option=0 ;;
    s) cond=$OPTARG ;;
    o) sydtestoptions=$OPTARG ;;
    *) cat << _EOF_ 
Bad Argument.
_EOF_
exit 1 ;;
    esac
done

if [ "$cond" = "" ]; then
    echo "Please select condition as '-s <cond>'."
    exit 1
fi

shift `expr $OPTIND - 1`
# 指定ディレクトリをリストにする
for arg in $*; do
    list="$list $arg"
done

# $listが空だったらトップdir以下全てのdirectoryが対象
if [ "$list" = "" ]; then
    list=`echo ./*`
fi

for name in $list; do
   # ディレクトリ名についた余計な部分を削除
   dir=`basename $name`

   # 正常系テスト
   case $option in
   0|1) testdir=$normal; dotest ;;
   esac

   # 異常系テスト 
   case $option in
   0|2) testdir=$except; dotest ;;
   esac
done

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
