# テストスクリプトに書かれているパス名の変換に使う
# SydTest.sh から呼び出す。
# windows用。(20100305) 
$result=0;

while (<>) {
    # DB
    s!X:/SydTest/(OptTest|OptNewTest|RipwayTest)DB/!X:/SydTest\\\\TRMeisterDB\\\\$1DB\\\\!;
    s!X:/SydTest/FullText2DB/(OptTest|OptNewTest|RipwayTest|VerifyFtsTEST)DB/!X:/SydTest\\\\FullText2DB\\\\$1DB\\\\!;
    s!X:/SydTest/OptimizerDB/(Default|IndexTest)DB/!X:/SydTest\\\\OptimizerDB\\\\$1DB\\\\!;
    s!X:/SydTest/(Sys0_Sydney_ext)/!X:/SydTest\\\\TRMeisterDB\\\\$1\\\\!;
    s!X:/SydTest/IndexTestDB/(.+)/\*!X:/SydTest\\\\IndexTestDB\\\\$1\\\\\*!;
    s!X:/SydTest(/FullText2DB)(/|\\\\)VerifyFtsTESTDB\\\\data\\\\VerifyFtsTESTDB\\\\(T\d) d:\\\\dm\\\\data\\\\VerifyFtsTESTDB\\\\!X:/SydTest$1\\\\VerifyFtsTESTDB\\\\data\\\\VerifyFtsTESTDB\\\\$3 d:\\\\dm\\\\data\\\\VerifyFtsTESTDB\\\\$3!;
    s!X:/SydTest/(.+DB|.+_ext|RINOA3)/\*!X:/SydTest\\\\$1\\\\\*!;
    s!X:/SydTest/(optimizer_distinct_test|DeleteTest) d:/dm!X:/SydTest\\\\$1 d:/dm/$1!;
    # v17.1 で追加された分散テスト用
    s!X:/SydTest/DistFullText2DB/(OptNewTestDB_[a-zA-Z0-9_\*]+) d:/dm!X:/SydTest\\\\DistFullText2DB\\\\$1 d:/dm/$1!;
    # 分散テスト 210000 - 210120 (20121017)
    s!X:/SydTest/RipwayDistDB/([a-zA-Z0-9_]+)/(RipwayDB_[a-zA-Z0-9_\*]+) d:/dm!X:/SydTest\\\\RipwayDistDB\\\\$1\\\\$2 d:/dm/$2!;
    # 分散テスト 200425 (2012101)
    s!X:/SydTest/DistTestDB_v1702/(DistTestDB_[a-zA-Z0-9_\*]+) d:/dm!X:/SydTest\\\\DistTestDB_v1702\\\\$1 d:/dm/$1!;
    # 分散テスト Trecデータ用 (20131002)
    s!X:/SydTest/TrecDistDB/(TrecDB_[a-zA-Z0-9_\*]+) d:/dm!X:/SydTest\\\\TrecDistDB\\\\$1 d:/dm/$1!;

    s!cp -Rf X:/SydTest/FullText2DB(/|\\\\)!xcopy /s /q /e /k /i /y X:\\\\SydTest\\\\FullText2DB\\\\!;
    s!cp -Rf X:/SydTest\\\\!xcopy /s /q /e /k /i X:\\\\SydTest\\\\!;

    s!\'d:\\\\dm\\\\data\\\\Sydney\'!\'c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\db\\\\sydtest\\\\data\\\\Sydney\'!g;
    s! [dD]:(\\\\|/)dm(\\\\|/)(alter_data_TEST|alter_system_TEST|optimizer_distinct_test|DeleteTest|BitmapDB|RipwayDB_[a-zA-Z0-9_\*]+|TrecDB_[a-zA-Z0-9_\*]+|OptNewTestDB_[a-zA-Z0-9_\*]+|DistTestDB_[a-zA-Z0-9\*]+)(DB|)! \\"c:\\\\Program Files$1Ricoh$1TRMeister$1db$1sydtest$2$3$4\\"!g;
    s! [dD]:(\\\\|/)dm(\\\\|/)(alterarea|area)(\\\\1|\\\\2|)(a|b|)(\\\\T1|)(\\\\FTS_I1_2|)! \\"c:\\\\Program Files$1Ricoh$1TRMeister$1db$1sydtest$2$3$4$5$6$7\\"!g;

    s! [dD]:(\\\\|/)dm(\\\\|/|)(data|system|)(2|)(\\\\TESTDB|\\\\DefaultDB|\\\\VerifyFtsTESTDB\\\\|\\\\TEST|TRMeister|)(T\d|\ds|\d|_bak|)(\\\\LOGICALLOG.SYD|\\\\LOGICALLOGDIR|)! \\"c:\\\\Program Files$1Ricoh$1TRMeister$1db$1sydtest$2$3$4$5$6$7\\"!g;
    s![dD]:(\\\\|/)dm(\\\\|/|)(data|system|)!c:\\\\Program Files$1Ricoh$1TRMeister$1db$1sydtest$2$3!g;
    s!c:/Program Files/Ricoh/TRMeister/db/(data|system)/(limedio_load|limedio)!c:/Program Files/Ricoh/TRMeister/db/sydtest/$1/$2!g;

    # 分散テストの子サーバ用 (20121012)
    s!cp -Rf \\"[dD]:(\\\\|/)trmchild(\d+)(\\\\|/)(data|system|)(\\\\|/)!xcopy /s /q /e /k /i \\"d:$1trmchild$2$3$4$5!;
    s![dD]:(\\\\|/)trmchild(\d+)(\\\\|/)(data|system|)(\\\\|/)!c:\\\\Program Files$1Ricoh$1TRMeister$2$1db$3$4$3!g;
    if (/Connection ran out/) {
        s![dD]:(\\\\|/)trchild(\d+|\*|)(\\\\|/)log(\\\\|/)sydtestlog!'c:\\\\Program Files$1Ricoh$1TRMeister$3log$4syslog.csv'!g;
    } else {
        s![dD]:(\\\\|/)trchild(\d+|\*|)(\\\\|/)log(\\\\|/)sydtestlog!'c:\\\\Program Files$1Ricoh$1TRMeister$2$3log$4syslog.csv'!g;
        s!mainsydtestlog!'c:\\\\Program Files/Ricoh/TRMeister/log/syslog.csv'!g;
    }

    # 
    s![dD]:\\\\[pP]roj!d:\\\\CVS-Root\\\\proj!;
    s!d:\\\\benchmark!x:\\\\benchmark!g;

    #ランダム
    if (/rand_max_(\d+)/) {
        $T = int(rand($1))+1;
        s!rand_max_\d+!$T!;
    }

    # バッチコマンドの mkdir では -p オプションがないため削除 (20120810)
    s!mkdir( -p|)(.*)";$!mkdir $2 > nul 2>&1";!;
    # 標準エラー出力削除
    s! ( > /dev/null 2>&1)(\"\;$)! > nul 2>&1$2!;

    # B+木索引の列数 7列などテストできないテストをスキップ
    $result=8 if/Skip BoundsChecker/;
    $result=7 if/Skip Purify/;
    $result=6 if/Skip Purify BoundsChecker/;

    print;
}
exit $result;
