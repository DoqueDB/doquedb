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
# テストスクリプトに書かれているパス名の変換に使う SydTest.sh から呼び出す。
# linux用。(040610,040616) 060203修正
$user=`whoami`;
chomp($user);
$cwd=`pwd`;
chomp($cwd);
$testnum=$ARGV[0];
$testnum =~ s!\.[^.]+$!!;
$installpath=$ENV{INSTALL_PATH};
$dbtop="/proj/sydney/data/SydTest";
$paddingc = 0;
$result=1;
while (<>) {
    # 2721.txt で create database AreaTest に対して、 Exists
    # "d:\\dm\\data\\areatest" で統一されていない 大文字小文字を統一する。 diff は
    # iオプションを付けて実行しているので、 preedit_resultout_for_linux.pl
    # で元に戻す必要はない
    s!areatest!AreaTest!g;
    s!mainsydtestlog!$installpath/log/doquedb.log!g;
    s!\$\(TestDir\)!$cwd!g;
    s!\$\(TestNum\)!$testnum!g;

    # パス名の変更
    s![dD]:\\\\dm\\\\data\\\\Cabinet1\\\\Sydney!$installpath/db/data/Cabinet1/Sydney!g;
    s![dD]:\\\\dm\\\\(data|area|alterarea|system)\\\\!$installpath/db/\1/!g;
    s![dD]:\\\\dm\\\\(data|area|alterarea|system)!$installpath/db/\1!g;
    # 分散テストの子サーバ用 (20121012 kurumida)
    s![dD]:(\\\\|/)trmchild(\d+)(\\\\|/)(data|area|alterarea|system)!$installpath$2/db/\4!g;
    s![dD]:(\\\\|/)trchild(\d+|\*)/log/sydtestlog!$installpath$2/log/doquedb.log!g;

    s!mkdir \\"d:\\\\dm\\\\backup\\\\DBDatabaseTest!mkdir -p \\"d:\\\\dm\\\\backup\\\\DBDatabaseTest!;
    s![dD]:(\\\\|/)dm!$installpath/db!g;
    #s!X:/SydTest/(Opt|OptNew|Ripway)TestDB!$dbtop/TRMeisterDB/${1}TestDB!g;
    #s!X:/SydTest/Sys0_Sydney_ext!$dbtop/TRMeisterDB/Sys0_Sydney_ext!;
    #s!X:/SydTest!$dbtop!g;
    #s![dx]:\\\\benchmark\\\\patent\\\\src.V2\\\\1000\\\\!/proj/sydney/data/SydTest/linux/patent/src.V2/1000_utf8/!g;
    #s![dx]:\\\\benchmark\\\\patent\\\\src.V2\\\\0\\\\!/proj/sydney/data/SydTest/linux/patent/src.V2/0_utf8/!g;
    #s![dx]:\\\\benchmark!/proj/sydney/data/SydTest/linux!g;
    s!(\W)h:!$1/diskfull!g;
    s!\\"c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\user.pswd\\"!$installpath/etc/user.pswd!g;
    s!\\"c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\test.pswd\\"!$installpath/etc/test.pswd!g;
    s!c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\test.pswd!$installpath/etc/test.pswd!;
    s!c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\([.\w]+)\\\\([.\w]+)\\\\([.\w]+)!$installpath/$1/$2/$3!g;
    s!c:/Program Files/Ricoh/TRMeister/db/(data|system)/(\w+)!$installpath/db/$1/$2!g;
    s!area (a modify|a) 'c:/Program Files/Ricoh/TRMeister/db/(\w+)!area $1 '$installpath/db/$2!;
    s!taskkill /F /IM SydTest\*!kill `ps -a | grep SydTest | awk '{print $1}'`!;
    if (~0 == 4294967295) {
        s!cscript.exe ../../ps.js \| grep 'SydTest'!grep '^VmSize:' /proc/`pgrep SydTest`/status!;
    }
    if (~0 == 18446744073709551615) {
        s!cscript.exe ../../ps.js \| grep 'SydTest'!grep '^VmHWM:' /proc/`pgrep SydTest`/status!;
    }
    if ( /System \"tasklist | grep 'SydTest/ ){
        s!tasklist!ps ux!;
        s!\$5!\$4!;
    }
    # SydTest.exe に /R オプションをつけない
    $result = 0 if ( /(ESCAPE|escape) (\'\\\\\'|\(\'\\\\\'\))/ ) ;
    $result = 0 if ( /FieldSeparator=\\"\\\\t\\"/ ) ;
    $result = 0 if ( /RecordSeparator=\\":\\\\n==\\\\n:\\"/ ) ;
    $result = 0 if ( /RecordSeparator=\\"\\\\n===\\\\n\\"/ ) ;
    $result = 0 if ( /select f from TBL where g like \'\%\\\\\%\'/ ) ;
    $result = 0 if ( /Bug report 1324/ ) ;
    $result = 0 if ( /SydTest MEM=%s\\\\n\\"/ );
    $result = 0 if ( /SydTest VmHWM=%s\\\\n\\"/ );
    $result = 0 if ( /freetext\(\'\.\\\\\'\)/ );
    #diskfullテストのpaddingサイズを変える
    if ( /makepadding\.pl \d+ > / ){
        if ( /Linux/ ) {
            s/# Linux //;
            $linuxdisk = 1;
        }
        elsif ( $linuxdisk == 1 ) {
            next;
        }
    }
    #ランダム
    if (/rand_max_(\d+)/) {
        $T = int(rand($1))+1;
        s!rand_max_\d+!$T!;
    }
    print;
}
exit $result;
#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
