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
###########################################################
#このスクリプトはeucでcheckoutして編集してcommitすること！#
###########################################################

#このスクリプトにはeucでは表示できない?文字がある。
#Windowsでcheckoutして編集してcommitすると、
#今度はその文字が化けてしまってperlを実行できなくなる。(20050706)

# 実行結果の編集に使う。
# SydTest.sh から呼び出す。
# 空行の処理はSydTest.shのdiffのBオプションにまかせる。
# Linux のみ使う。(040823) 

# linux用に変更したパス名を元に戻すのに使う
$userName=`whoami`;
chomp($userName);

# スコアを丸めるために使う。
$use_libTerm=0;

$installpath=$ENV{INSTALL_PATH};
$mytoppath=$ENV{MYTOP_PATH};

while (<>) {

    # 自動リカバリ前のプロセス強制終了時のメッセージを抑制
    s!$installpath/bin/sydtest: line \d+: \d+ 強制終了.*!!;

    # linux用に変更したパス名を元に戻す
    # ディレクトリの区切り文字も\に変換する。
    if (m!$mytoppath! || m!$installpath! || m!/diskfull!) {
    s!$installpath/db!d:\\dm!g;
    s!$mytoppath/TRMeister/sydney/Test/SydTest/test!X:\\SydTest!g;
    s!/proj/sydney/data/SydTest/linux/!x:\\benchmark!g;
    # 分散テストの子サーバ用
    s!$installpath(\d+)/db!d:\\trmchild$1!g;
    # TR log
    s!$installpath(\d+|\*|)/log!d:\\trchild$1/log!g;
    # diskfullにgオプションを使うと、
    # "/diskfull/diskfulltest"を"h:h:test"にしてしまう。
    # 3701.txt 等 (040615)
    s!(/dev/sd[ab]\d+|)\s+\d+\s+\d+\s+\d+\s+\d+% /diskfull!\t1 個のディレクトリ\t1 バイトの空き領域!;
    s!/diskfull!h:!;

    #
    s!cp -f $installpath/etc/user.pswd!cp -f \"c:\\Program Files\\Ricoh\\TRMeister\\user.pswd\"!g;
    s!Server_PasswordFilePath, $installpath/etc/test.pswd!Server_PasswordFilePath, c:\\Program Files\\Ricoh\\TRMeister\\test.pswd!;
    s!$installpath/etc/test.pswd!\"c:\\Program Files\\Ricoh\\TRMeister\\test.pswd\"!g;
    s!$installpath/([.\w]+)/([.\w]+)/([.\w]+)!c:\\Program Files\\Ricoh\\TRMeister\\$1\\$2\\$3!g;
    s! > /dev/null 2>&1!!;

    s!/!\\!g;
    s!cp -Rf \\proj\\sydney\\data\\SydTest!cp -Rf X:\\SydTest!g;

    # 戻し過ぎた\を/に戻す。1350.txt 等
    s!^(Dist)(FullText2|Optimizer)DB\\!$2DB/!;
    #s!(FullText2|Optimizer)DB\\!$1DB/!;

    s!rm -rf d:\\dm\\data d:\\dm\\system!rm -rf d:/dm/data d:/dm/system!g;
    s!cp -Rf X:\\SydTest\\(TRMeisterDB\\|FullText2DB/|FullText2DB\\)(OptNew|Opt|Ripway)TestDB\\\* d:\\dm!cp -Rf X:/SydTest/\2TestDB/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\TRMeisterDB\\(Sys0_Sydney_ext)\\\* d:\\dm!cp -Rf X:/SydTest/\1/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\(JoinDt|JoinExists|Olive)TestDB\\\* d:\\dm!cp -Rf X:/SydTest/\1TestDB/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\(DefaultDB|ColumnDB|TrecDB)\\\* d:\\dm!cp -Rf X:/SydTest/$1/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\Unique_old_TESTDB\\\* d:\\dm!cp -Rf X:/SydTest/Unique_old_TESTDB/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\(FullText2DB|OptimizerDB|)(/|\\)(DefaultDB|ColumnDB|VerifyFtsTESTDB)\\\* d:\\dm!cp -Rf X:/SydTest/$1/$3/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\(DeleteTest|optimizer_distinct_test|DefaultDB) d:\\dm!cp -Rf X:/SydTest/\1 d:/dm!;
    s!cp -Rf X:\\SydTest\\IndexTestDB\\(\w+)\\\* d:\\dm!cp -Rf X:/SydTest/IndexTestDB/$1/* d:/dm!;
    s!cp -Rf X:\\SydTest\\(mountdb_test|limedio_150821DB)\\\* d:\\dm!cp -Rf X:/SydTest/$1/* d:/dm!;
    # 220*** v17.1 で追加された分散テスト用
    s!cp -Rf X:\\SydTest\\DistFullText2DB\\(OptNewTestDB_[a-zA-Z0-9_\*]+) d:\\dm!cp -Rf X:/SydTest/DistFullText2DB/\1 d:/dm!;
    # 200700.txt - 20716.txt (20121210)
    s!cp -Rf X:\\SydTest\\RipwayDistDB\\([a-zA-Z0-9_]+)\\(RipwayDB_[a-zA-Z0-9_\*]+)\\\* d:\\dm!cp -Rf X:/SydTest/RipwayDistDB/\1/\2/\* d:/dm!;
    s!cp -Rf X:\\SydTest\\TrecDistDB\\(TrecDB_[a-zA-Z0-9_\*]+)\\\* d:\\dm!cp -Rf X:/SydTest/TrecDistDB/\1/\* d:/dm!;
    # 210000 - 210120 (20121017)
    s!cp -Rf X:\\SydTest\\RipwayDistDB\\([a-zA-Z0-9_]+)\\(RipwayDB_[a-zA-Z0-9_\*]+) d:\\dm!cp -Rf X:/SydTest/RipwayDistDB/\1/\2 d:/dm!;
    s!cp -Rf X:\\SydTest\\TrecDistDB\\(TrecDB_[a-zA-Z0-9_\*]+) d:\\dm!cp -Rf X:/SydTest/TrecDistDB/\1 d:/dm!;
    # 200425 (20121017)
    s!cp -Rf X:\\SydTest\\DistTestDB_v1702\\(DistTestDB_[a-zA-Z0-9\*]+) d:\\dm!cp -Rf X:/SydTest/DistTestDB_v1702/\1 d:/dm!;
    s!cp -Rf X:\\SydTest\\opttest_c_sydney_ext\\\* d:\\dm!cp -Rf X:/SydTest/opttest_ext/\* d:/dm!;
    s!rm -rf d:\\dm\\data\\TESTDB\\T!rm -rf d:/dm/data/TESTDB/T!;
    s!rm -rf d:\\dm\\data\\TESTDB\\users\\IDENTITY!rm -rf d:/dm/data/TESTDB/users/IDENTITY!;
    s!rm -rf d:\\dm\\(JoinDtTest|DeleteTest|optimizer_distinct_test|RipwayDB_[a-zA-Z0-9_\*]+|OptNewTestDB_[a-zA-Z0-9_\*]+|DistTestDB_[a-zA-Z0-9_\*]+|TrecDB_[a-zA-Z0-9_\*]+)!rm -rf d:/dm/\1!;
    s!mkdir( -p|) d:\\dm$!mkdir\1 d:/dm!;

    #
    s!trmeister:\\\\(root:doqadmin@|)localhost:5432(\d)\\TESTDB!trmeister://\1localhost:5432\2/TESTDB!;
    s!inprocess:\\\\(\w+):(\w+@)\\(\w+)!inprocess://\1:\2/\3!;
    s!(\"|\')d:\\trmchild(\d)\\(data|system)\\TESTDB(\d|_\d+|)(\"|\')!\1d:\\trmchild\2/\3/TESTDB\4\5!g;
    s!d:\\trchild\*\\log\\!d:\\trchild/log/! if( /grep -i 'Connection ran out.'/ );
    s!d:\\trchild(\d|\*|)\\log\\!d:\\trchild\1/log/!g;

    #
    s!\"d:(\\|/)dm(/data|/system|/DeleteTest|/optimizer_distinct_test|/RipwayDB_[a-zA-Z0-9_\*]+|/OptNewTestDB_[a-zA-Z0-9_\*]+|/DistTestDB_[a-zA-Z0-9_\*]+|/TrecDB_[a-zA-Z0-9_\*]+|)\"!d:$1dm$2!g;
    s!\"d:\\dm\\(\w+|)(area)(\\1|\\2|)(T1|)(FTS_I1_2)\"!d:\\dm\\$1$2$3$4$5!g;
    s!\"d:(\\|/)dm(\\|/)(.+)\"( of=|  | )\"d:(\\|/)dm(\\|/)(.+)\"!d:$1dm$2$3$4d:$5dm$6$7!;
    s!\"d:(\\|/)dm(\\|/)(.+)\"!d:$1dm$2$3!;

    # 24050.txt 24052.txt
    if (/MASTER_temp\.SYD/ || /VERSION_temp\.SYD/ || /VerifyFtsTESTDB/) {
        #s!cp -Rf \"\\proj\\sydney\\data\\SydTest\\!cp -Rf \"X:/SydTest/!g;
        #s!d:\\dm\\(data|system)\\!d:\\dm\\\1\\!g;
        s!if=\\dev\\zero!if=/dev/zero!g;
    }
    s!cp -Rf X:\\SydTest\\(FullText2DB/|)VerifyFtsTESTDB\\data\\VerifyFtsTESTDB\\(T\d) d:\\dm\\data\\VerifyFtsTESTDB\\!cp -Rf X:/SydTest/$1VerifyFtsTESTDB\\data\\VerifyFtsTESTDB\\$2 d:\\dm\\data\\VerifyFtsTESTDB!;
    }
    # 24050.txt
    if (/(Tbtr\.Ibtr)/ || /(Tbmp\.Ibmp)/ || /(Tary\.Iary)/ || /(Kernel).+(PhysicalFile).+(Initialize\sof\sverification\shas\sbeen\sfailed)/) {
        s!/!\\!g;
    }
    # 上記のパス名以外の区切り文字を個別に変換する。
    # (ex:4550) (040714)
    s!Admin,../Manager.cpp!Admin,..\\Manager.cpp!;
    # v15用？に追加 (040803)
    s!Admin,../../Admin/Manager.cpp!Admin,..\\..\\Admin\\Manager.cpp!;
    s!Admin,../../../Kernel/Admin/Manager.cpp!Admin,..\\Manager.cpp!;
    s!Schema,../../../Kernel/Schema/(Index|Table|Sequence).cpp!Schema,..\\..\\..\\Kernel\\Schema\\$1.cpp!;
    # ex: 2860 (040728)
    s!../../restore.sh!..\\..\\restore.bat!;

    # 2736
    s!mv: cannot stat \`d:\\dm\\data\\databasetest!mv: cannot stat \'d:\\dm\\data\\databasetest!;
    s!mv: cannot stat \`d:\\dm\\area\\2b\': (No such file or directory|そのようなファイルやディレクトリはありません)!指定されたファイルが見つかりません。!;
    s!: そのようなファイルやディレクトリはありません!: No such file or directory!;

    # linux用に変更したコマンド名を元に戻す
    # s/A/B/;でAの中の「]」に「\」はいらないが、エディタが間違えるので追加。
    s!\[System Parameter\] mv -f![System Parameter] move /Y!;
    s!\[System Parameter\] mkdir( -p|)![System Parameter] md!;
    s!\[System Parameter\] cp -rp![System Parameter] ..\\..\\switchcopy.bat!;

    # 20643
    s!Function 'DBGetScoreCalculator' not found in 'imf' library!Library 'imf' not found!;

    # preedit_txt_for_linux.plで変更したコマンド名を元に戻す
    # 本来は "df /diskfull" だが、上で/diskfullをh:に変換されている
    # 3701.txt等 (040615)
    s!df h:!dir h:!;

    # Linux専用の正解LOG
    s!Uncategorized Os Error: -(2|5) occured \(getaddrinfo\)!Uncategorized Os Error: xx occured \(getaddrinfo\)!;

    # pure virtual method called
    s!\(コアダンプ\)!\(core dumped\)!;
    $pvmc=1, next if (/pure virtual method called/);
    if ($pvmc==1) {
        next if (/active exception/);
        if (/\(core dumped\)/) {
            $coreno=$1 if (/line\s+\d+:\s+(\d+)/);
            unlink "core.$coreno";
            $pvmc=0;
            next;
        }
    }

    # ModMessage
    $modm=1 if (/hint 'kwic/);
    if ($modm==1) {
        $modm=0 if (/drop table/);
        next if (/ModInvertedTokenizer.+\[ERR\] create failed: Bad argument/);
        next if (/ModInvertedQueryParser.+\[ERR\] no termString/);
    }

    # Peak Memory 
    s!grep '\^Vm.+' /proc/`pgrep SydTest`/status!cscript.exe ../../ps.js \| grep 'SydTest'!;
    if (/\[<System>\] SydTest VmHWM=(\d+)/) {
        if ( $1 < 160 ) {
            s!\d+!xxx!;
        }
    }

    #Windows 以外では 0に- がつく
    s!\{-0E0\}!\{0E0\}!;

    # bad_allocのFakeエラーのテスト
    s!Worker(.+) St9bad_alloc!worker$1 bad allocation!;
    s!Worker(.+) std::bad_alloc!Worker$1 bad allocation!;
    s!Manager(.+) St9bad_alloc!Manager$1 bad allocation!;
    s!Manager(.+) std::bad_alloc!Manager$1 bad allocation!;
    s!OpenMP(.+) St9bad_alloc!OpenMP$1 bad allocation!;
    s!OpenMP(.+) std::bad_alloc!OpenMP$1 bad allocation!;

    # dir と df の出力フォームの違いを吸収する。
    # diff を取る時、wオプションで空白を無視しているのでタブでも構わない。
    # Use '\\', because 'h:' is included.
    # 3701.txt等 (040615)
    # 様々なパーティションに対応 (050610)
    #s!\\dev\\sd[ab]\d\s+\d+\s+\d+\s+\d+\s+\d+% h:!\t1 個のディレクトリ\t1 バイトの空き領域!;
    #s!\\dev\\sd[ab]\d\d\s+\d+\s+\d+\s+\d+\s+\d+% h:!\t1 個のディレクトリ\t1 バイトの空き領域!;

    # スコア違い対応
    # freetext, wordlistを使っているSQL文はlibTermを使っている。
    # weightもそうかもしれない。(050224)
    # 1368のfreetextのif文で通過するためFROM T AS TO を追加
    if (/\[\[SQL Query\]\]/) {
        $use_libTerm=2 if (/score/ && !( /sectionized/ ));
        $use_libTerm=1 if (/(freetext|wordlist|weight|FROM T AS T0 WHERE)/);
    }
    if ($use_libTerm == 2 ) {
        s/([,\{]\d\.\d{2})\d{8,12}(E[-0-9])/\1\2/g;
    }
    if ($use_libTerm == 1) {
        # WindowsとlibTermの使い方が異なる？5桁以降を切捨て。
        # ex:1378,1380,1384,1390 (050224)
        # s/A/B/でAの中の「{」に「\」はいらないが、エディタが間違えるので追加。
        s/([,\{]\d\.\d{3})\d{8,11}(E[-0-9])/\1\2/g;
    }
    if ($use_libTerm == 0) {
        # Windowsとdoubleの数字の丸め方が異なる？13,14桁を切捨て。
        # ex:4222,4260,4262 (040913)
        s/([,\{]\d\.\d{12})\d{1,2}(E[-0-9])/\1\2/g;
    }

    # cascade
    next if /bnormal exit status to force-terminate cascade \d/;

    #debug版での対応
    next if /bit length overflow/;
    next if /code \d bits \d->\d/;
    next if /code 1\d bits \d->\d/;
    next if /ModInvertedNgramTokenizer/;
    next if /ModInvertedDualTokenizer/;
    next if /ModCommonInitialize/;
    next if /ModParameterSolaris/;
    next if /ModParameter/;
    next if /FullText2::FullTextFile.+ \[INFO\] FullTextFile::(insert|update|expunge)/;
    next if /FullText2::InvertedSection.+ \[INFO\] InvertedSection::(insert|expunge)LocationList/;
    print;
}
#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
