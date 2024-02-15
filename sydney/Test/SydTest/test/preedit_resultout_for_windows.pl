# 実行結果のDeleteTest編集に使う。
# SydTest.sh から呼び出す。

$xcopycommnd = 0;
while (<>) {
    # DB
    s!X:\\SydTest\\(DeleteTest|optimizer_distinct_test) \"c:\\Program Files/Ricoh/TRMeister/db/sydtest/(DeleteTest|optimizer_distinct_test)\"!X:/SydTest/$1 d:/dm!;
    # v17.1 で追加された分散テスト用 (220000 台) (20130318)
    s!X:\\SydTest\\DistFullText2DB\\(OptNewTestDB_[a-zA-Z0-9_\*]+) \"c:\\Program Files/Ricoh/TRMeister/db/sydtest/(OptNewTestDB_[a-zA-Z0-9_\*]+)\"!X:/SydTest/DistFullText2DB/$1 d:/dm!;
    # 分散テスト 210000 - 210120 (20121017)
    s!X:\\SydTest\\RipwayDistDB\\([a-zA-Z0-9_]+)\\(RipwayDB_[a-zA-Z0-9_\*]+) \"c:\\Program Files/Ricoh/TRMeister/db/sydtest/(RipwayDB_[a-zA-Z0-9_\*]+)\"!X:/SydTest/RipwayDistDB/$1/$2 d:/dm!;
    s!X:\\SydTest\\TrecDistDB\\(TrecDB_[a-zA-Z0-9_\*]+) \"c:\\Program Files/Ricoh/TRMeister/db/sydtest/(TrecDB_[a-zA-Z0-9_\*]+)\"!X:/SydTest/TrecDistDB/$1 d:/dm!;
    # 分散テスト 200425 (20121017)
    s!X:\\SydTest\\DistTestDB_v1702\\(DistTestDB_[a-zA-Z0-9_\*]+) \"c:\\Program Files/Ricoh/TRMeister/db/sydtest/(DistTestDB_[a-zA-Z0-9_\*]+)\"!X:/SydTest/DistTestDB_v1702/$1 d:/dm!;

    s!\"c:\\Program Files\\Ricoh\\TRMeister\\db(\\data2|\\system2|\\data|\\system|\\alter_data_TESTDB|\\alter_system_TESTDB|\\optimizer_distinct_test|\\RipwayDB_[a-zA-Z0-9_\*]+|\\TrecDB_[a-zA-Z0-9_\*]+|\\OptNewTestDB_[a-zA-Z0-9_\*]+|\\alterarea|)(\\TESTDB_bak|\\TESTDB\d|\\TESTDB|\\\d\w|\\\d|)\"!d:\\dm$1$2!g;
    s!\"c:\\Program Files\\Ricoh\\TRMeister\\db\\(alterarea|area)\\(\d\w|\d)\\(\w\d)(\\FTS_I1_2|)\"!d:\\dm\\$1\\$2\\$3$4!g;
    s!\"c:(\\|/)Program Files(\\|/)Ricoh(\\|/)TRMeister(\\|/)db(\\|/)(data2|system2|data|system|alter_data_TESTDB|alter_system_TESTDB|alterarea|optimizer_distinct_test|area|RipwayDB_[a-zA-Z0-9_\*]+|TrecDB_[a-zA-Z0-9_\*]+|OptNewTestDB_[a-zA-Z0-9_\*]+|)(/|\\|)(DefaultDB|TESTDB\d|TESTDB|\d\w|\d|)\"!d:$1dm$5$6$7$8!g;
    s!c:\\Program Files/Ricoh/TRMeister/db/sydtest!d:/dm!g;
    s!c:(\\|/)Program Files\\Ricoh\\TRMeister\\db\\sydtest!d:\\dm!g;
    s!c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\db\\\\sydtest\\\\data\\\\databasetest!d:\\dm\\data\\databasetest!;
    s!(\"|)c:/Program Files/Ricoh/TRMeister/db/sydtest/(data|system)/(limedio_load|limedio)(\"|)!d:\\dm\\$2\\$3!g;

    # 分散テストの子サーバ用 (20121012)
    s!(\{)c:\\Program Files/Ricoh/TRMeister2/db/(data|system)/TESTDB(\})!\1d:\\trmchild2\\\2\\TESTDB\3!;
    s!c:(\\|/)Program Files(\\|/)Ricoh(\\|/)TRMeister(\d+)(\\|/)db!d:\\trmchild\4!g;
    s!c:(\\\\|/)Program Files(\\\\|/)Ricoh(\\\\|/)TRMeister(\d+)(\\\\|/)db!d:\\trmchild\4!;
    s!mv -f 'c:\\Program Files/Ricoh/TRMeister(\d|)/log/syslog.csv'!move /Y d:\\trchild\1/log/trmeister.log!;
    s!'c:\\Program Files/Ricoh/TRMeister(\d|)/log/syslog.csv'!d:\\trchild\1/log/trmeister.log!;

    s!X:\\SydTest\\IndexTestDB\\(btree2|bitmap)_v3_(nvar|var|n|)char(_nontruncate|)(_onedata_array|_onedata|_array|)\\\*!X:/SydTest/IndexTestDB/$1_v3_$2char$3$4/*!g;
    s!X:\\SydTest\\IndexTestDB\\array_int_array\\\*!X:/SydTest/IndexTestDB/array_int_array/*!g;
    s!X:\\SydTest\\(FullText2DB\\|)VerifyFtsTESTDB\\data\\VerifyFtsTESTDB\\(T\d) \"d:\\dm\\data\\VerifyFtsTESTDB\\(T\d)\"!X:/SydTest/$1VerifyFtsTESTDB\\data\\VerifyFtsTESTDB\\$2 d:\\dm\\data\\VerifyFtsTESTDB!;
    s!X:\\SydTest\\OptimizerDB\\(.+DB)\\!X:/SydTest/OptimizerDB/$1/!;
    s!X:\\SydTest(\\TRMeisterDB|\\FullText2DB|)\\(.+DB|.+_ext)\\\*!X:/SydTest/$2/*!;
    s!X:/SydTest/FullText2DB\\VerifyFtsTESTDB!X:/SydTest/FullText2DB/VerifyFtsTESTDB!;
    s!xcopy /s /q /e /k /i X:/SydTest/FullText2DB\\!cp -Rf X:/SydTest/FullText2DB/!;

    # SydTest.exeを実行する際にSydTest.batで変更した部分を戻す。
    # c:\Sydneyの場合もある。(ex: 4500.txt) (040625)
    s!\"d:(\\|/)dm(/data|/system|/DeleteTest|/optimizer_distinct_test|/RipwayDB_[a-zA-Z0-9_\*]+|/TrecDB_[a-zA-Z0-9_\*]+|/OptNewTestDB_[a-zA-Z0-9_\*]+|)\"!d:$1dm$2!g;
    s!\"d:\\dm\\(\w+|)(area)(\\1|\\2|)(T1|)(FTS_I1_2)\"!d:\\dm\\$1$2$3$4$5!g;
    s!\"d:(\\|/)dm(\\|/)(.+)\"( of=|  | )\"d:(\\|/)dm(\\|/)(.+)\"!d:$1dm$2$3$4d:$5dm$6$7!;
    s!\"d:(\\|/)dm(\\|/)(.+)\"!d:$1dm$2$3!;

    # 作業フォルダの変更(ex: 1154) (040705)
    s!d:\\CVS-Root\\proj!d:\\proj!ig;

    # 作業フォルダの変更(ex: 1154) (カバレージ用) (050105)
    s!d:\\CVS-Root\\[Cc]overage!d:\\proj!ig;
    next if/Starting Purify'd application/;
    next if/Purify'd application exited with code 0/;

    # RdmCab1フォルダの変更(ex: 1350) (040714)
    s!opttest_c_sydney!opttest!;

    # RdmCab1フォルダの変更(ex: 1364) (051013)
    s!opt_newtest_c_sydney!opt_newtest!;

    s!\{-0E0\}!\{0E0\}!;

    # xcopy を使ったときの修正
    if ( $xcopycommnd == 1 ){
        $xcopycommnd = 0;
        next if ( /^\d+/ );
    }

    # mkdir を使ったときの修正(ex. 210000) (120810)
    s!mkdir(.*) > nul 2>&1!mkdir $1!;

    # OS 64bit環境でSydney 64bitの時 または OS 32bit環境でSydney 32bit
    if (~0 != 4294967295) {
        if (/\[<System>\] SydTest VmHWM=(\d+)/) {
            if ( $1 < 160 ) {
                s!\d+!xxx!;
            }
        }
    }
    else {
        if (/\[<System>\] SydTest VmHWM=\d{2}/) {
            s!\d{2}!xxx!;
        }
    }

    if ( /xcopy (\/\w)+/ ){
        s!xcopy /[sS] /[qQ] /[eE] /[kK] (/x /i |/i /y |/i |)!cp -Rf !g;
        $xcopycommnd = 1;
    }

    print;
}
