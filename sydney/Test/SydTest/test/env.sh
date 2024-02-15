#!/bin/sh
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
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

# 定数の定義
. conf.sh
pwd=`pwd`
major=`expr $pwd : '.*v\([0-9]*\)\.[0-9]*/[Tt]est/[Ss]yd[Tt]est/test'`
minor=`expr $pwd : '.*v[0-9]*\.\([0-9]*\)/[Tt]est/[Ss]yd[Tt]est/test'`
testtype=`expr $pwd : '.*/\([a-z]*\)/sydney/v[0-9.]*/[Tt]est/[Ss]yd[Tt]est/test'`
sydversion=v$major.$minor

timeout=10800


# dm はデータベースの置き場所

if [ "$OS" = "Windows_NT" ]; then
    isWindows=1
    isLinux=0
    isSolaris=0

    if [ "$PROCESSOR_ARCHITECTURE" = "AMD64" -o "$PROCESSOR_ARCHITEW6432" = "AMD64" ]; then
        osname="windows_x64"
        arc=x64
    else
        osname=windows
        arc=x86
    fi
    progamdir="Program Files"

    if [ `env | grep -i 'LIB' | grep -i 'amd64' | wc -w` -ge 1 ] ; then
        if [ `env | grep ProgramW6432 | wc -w` -ge 1 ] ; then
            PROGRAMFILESDIR=$ProgramW6432
        else
            PROGRAMFILESDIR=$PROGRAMW6432
        fi
        # 64bit版のregeditとregをコピーしてリネームして使用
        osname=windows
        REGCMD=$pwd/REG64
        REGEDITCMD=$pwd/REGEDIT64
    else
        PROGRAMFILESDIR=$PROGRAMFILES
        REGCMD=REG
        REGEDITCMD=regedit
    fi

    mytop=`expr $pwd : '/cygdrive/\([a-z]\)/.*'`:
    hdrive=h:
    xdrive=x:
    sydtop=$mytop/`expr $pwd : '/cygdrive/[a-z]/\(.*\)/[Tt]est/[Ss]yd[Tt]est/[Tt]est'`
    # tools/setup/package/conf.bat に合わせる。
    dm="$PROGRAMFILESDIR/Ricoh/TRMeister/db/sydtest"
    save="${dm}/save"
    dumpdst=C:\\dumpdm

    # レジストリのパス これを読み込むときに\を一つ消費する。
    regpath_sydney=HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister
    # modのレジストリをいじることはない？
    # registry_mod=HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\ModCommonLibrary
    exe=bat

    # PROGRAMFILESは空白を含むので「"」で囲む
    # SydTestオプション設定
    sydtest="$PROGRAMFILESDIR/ricoh/TRMeister/SydTest.exe"
    sydtestlog="$PROGRAMFILESDIR/ricoh/TRMeister/log/syslog.csv"

else
    isWindows=0
    mytop=${pwd%%/TRMeister/*}
    sydtop=${pwd%%/Test*}
    # h:\は、今のところwhaleのみ/diskfullが対応している。(040615)
    hdrive=/diskfull
    # x:\は、x:\SydTestとx:\benchmarkで対応するディレクトリが異なる。
    # このxdriveはx:\SydTest\opttestがに対応するディレクトリの存在調査に使う。
    xdrive=/proj/sydney/data/SydTest
    dm=$databasepath
    save=${installpath}/save
    systemconffile=${installpath}/etc/sydtest.conf
    # sydtestはパス名変換と環境変数設定してからSydTestを起動する。
    sydtest=${installpath}/bin/sydtest
    sydtestlog="${installpath}/log/trmeister.log"
    dumpdst=${installpath}/dumpdm
    exe=sh

    if [ `uname` = Linux ]; then
        isLinux=1
        isSolaris=0

        # linux 環境のcoredumpの設定が0KBになっている時の対応
        # linux 環境のperl warningメッセージを消す
        ulimit -c unlimited
        PERL_BADLANG=0; export PERL_BADLANG

        osname=linux
    fi

    if [ `uname` = SunOS ]; then
        isLinux=0
        isSolaris=1
        osname=solaris
    fi
fi

setup=$sydtop/tools/setup
testroot=$sydtop/Test # 大文字注意！
testtop=$testroot/SydTest/test # 大文字注意！


# c.g/sydney_v15.0/dump.shで${dm}以下は${save}にコピーされる。
# ${save}はrestore_sydtest.shで使う。
# ${dm}はSydTest.shのdumpdmでも使う。

#ディレクトリの関係
#/proj/sydney/work/horibe [mytop]
#			/proj
#			    /sydney/v14.0 [sydtop]
#					/tools/setup [setup]
#					/Test [testroot]
#					    /SydTest
#						   /test [testtop]
#						       /??? [dir]
#							  /normal [testdir]
#							  /except [testdir]
#						   /VolumeX [xdrive]
#			/opt/sydney [ddrive]
#				  /db (= C:\Sydney)
#				  /save (= C:\Sydney\save) [save]
#				  /dm_bak
#				  /dumpdm (= ???\dumpdm)[dumpdst]
#/diskfull [hdrive]

#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
