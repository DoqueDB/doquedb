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

while (<>) {

    # linux用に変更したパス名を元に戻す
    # ディレクトリの区切り文字も\に変換する。
	next if s/^\s*$//g;
    if (m!/proj/sydney/work/($userName)! || m!/diskfull!) {
	s!/proj/sydney/work/($userName)/opt/RICOHtrm/db!d:\\dm!g;
	s!/proj/sydney/work/($userName)/proj/sydney/v\d+\.\d+/Test/SydTest/test!X:\\SydTest!g;
	s!/proj/sydney/data/SydTest/linux/!x:\\benchmark!g;
	# diskfullにgオプションを使うと、
	# "/diskfull/diskfulltest"を"h:h:test"にしてしまう。
	# 3701.txt 等 (040615)
        s!/diskfull\s+\(/dev/dsk/emcpower0a\):\s+\d+\sブロック\s+\d+\sファイル!\t1 個のディレクトリ\t1 バイトの空き領域!;	
	s!/diskfull!h:!;

	#
	s!cp -f /proj/sydney/work/($userName)/opt/RICOHtrm/etc/user.pswd!cp -f \"c:\\Program Files\\Ricoh\\TRMeister\\user.pswd\"!g;
	s!Server_PasswordFilePath, /proj/sydney/work/($userName)/opt/RICOHtrm/etc/test.pswd!Server_PasswordFilePath, c:\\Program Files\\Ricoh\\TRMeister\\test.pswd!;
	s!/proj/sydney/work/($userName)/opt/RICOHtrm/etc/test.pswd!\"c:\\Program Files\\Ricoh\\TRMeister\\test.pswd\"!g;

	s!/!\\!g;
        s!cp -Rf \\proj\\sydney\\data\\SydTest\\solaris_db\\SydTest!cp -Rf X:\\SydTest!g;

        # 戻し過ぎた\を/に戻す。1350.txt 等
	s!rm -rf d:\\dm\\data d:\\dm\\system!rm -rf d:/dm/data d:/dm/system!g;
        s!cp -Rf X:\\SydTest\\(Opt|OptNew|Ripway|JoinDt|JoinExists|Olive)TestDB\\\* d:\\dm!cp -Rf X:/SydTest/\1TestDB/\* d:/dm!;
        s!cp -Rf X:\\SydTest\\(Sys0_Sydney_ext)\\\* d:\\dm!cp -Rf X:/SydTest/\1/\* d:/dm!;
	s!cp -Rf X:\\SydTest\\(DefaultDB|ColumnDB)\\\* d:\\dm!cp -Rf X:/SydTest/\1/\* d:/dm!;
	s!cp -Rf X:\\SydTest\\(DeleteTest|optimizer_distinct_test|DefaultDB) d:\\dm!cp -Rf X:/SydTest/\1 d:/dm!;
	s!cp -Rf X:\\SydTest\\opttest_c_sydney_ext\\\* d:\\dm!cp -Rf X:/SydTest/opttest_ext/\* d:/dm!;
        s!cp -Rf X:\\SydTest\\IndexTestDB\\(b\w+_v3_\w+)\\\* d:\\dm!cp -Rf X:/SydTest/IndexTestDB/$1/\* d:/dm!;
        s!cp -Rf X:\\SydTest\\IndexTestDB\\(\w+)\\!cp -Rf X:/SydTest/IndexTestDB/$1/!;	
	s!rm -rf d:\\dm\\data\\TESTDB\\T!rm -rf d:/dm/data/TESTDB/T!;
	s!rm -rf d:\\dm\\(JoinDtTest|DeleteTest|optimizer_distinct_test)!rm -rf d:/dm/\1!;

        #
        s!\"d:(\\|/)dm(/data|/system|/DeleteTest|/optimizer_distinct_test|)\"!d:$1dm$2!g;
        s!\"d:\\dm\\(\w+|)(area)(\\1|\\2|)(T1|)(FTS_I1_2)\"!d:\\dm\\$1$2$3$4$5!g;
        s!\"d:(\\|/)dm(\\|/)(.+)\"( of=|  | )\"d:(\\|/)dm(\\|/)(.+)\"!d:$1dm$2$3$4d:$5dm$6$7!;
        s!\"d:(\\|/)dm(\\|/)(.+)\"!d:$1dm$2$3!;

	# 24050.txt 24052.txt
	if (/MASTER_temp\.SYD/ || /VERSION_temp\.SYD/ || /VerifyFtsTESTDB/) {
	 #s!cp -rf \"\\proj\\sydney\\data\\SydTest\\solaris_db\\SydTest\\!cp -rf \"X:/SydTest/!g;
	 #s!\"d:\\dm\\(data|system)\\\"!d:\\dm\\\1\\!g;
	 s!if=\\dev\\zero!if=/dev/zero!g;
	}
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
    # ex: 2860 (040728)
    s!sh ../../restore.sh!..\\..\\restore.bat!;

    # linux用に変更したコマンド名を元に戻す
    # s/A/B/;でAの中の「]」に「\」はいらないが、エディタが間違えるので追加。
    s!\[System Parameter\] mv -f![System Parameter] move /Y!;
    s!\[System Parameter\] mkdir( -p|)![System Parameter] md!;
    s!\[System Parameter\] cp -rp![System Parameter] ..\\..\\switchcopy.bat!;

    # ex 2736
    s!mv: d:\\dm\\area\\2b を使用できません!指定されたファイルが見つかりません!;

    # preedit_txt_for_linux.plで変更したコマンド名を元に戻す
    # 本来は "df /diskfull" だが、上で/diskfullをh:に変換されている
    # 3701.txt等 (040615)
    s!df h:!dir h:!;

    #
    $modm=1 if (/hint 'kwic/);
    if ($modm==1) {
        $modm=0 if (/drop table/);
        next if (/ModInvertedTokenizer.+\[ERR\] create failed: Bad argument/);
        next if (/ModInvertedQueryParser.+\[ERR\] no termString/);
    }

    #Windows 以外では 0に- がつく
    s!\{-0E0\}!\{0E0\}!;

    # bad_allocのFakeエラーのテスト
    s!Manager(.+) Out of Memory!Manager$1 bad allocation!;
    s!Worker(.+) Out of Memory!Worker$1 bad allocation!;

    # dir と df の出力フォームの違いを吸収する。
    # diff を取る時、wオプションで空白を無視しているのでタブでも構わない。
    # Use '\\', because 'h:' is included.
    # 3701.txt等 (040615)
    # 様々なパーティションに対応 (050610)
    #s!\(/dev/dsk/emcpower0a\):\s+\d+\s+\d+\s+\d+\s+\d+%\s+!\t1 個のディレクトリ\t1 バイトの空き領域!;

    # スコア違い対応
    # freetext, wordlistを使っているSQL文はlibTermを使っている。
    # weightもそうかもしれない。(050224)
    # 1368のfreetextのif文で通過するためFROM T AS TO を追加
    if (/\[\[SQL Query\]\]/) {
        $use_libTerm=3 if (/avg/);
        $use_libTerm=2 if (/score/ && !( /sectionized/ ));
	$use_libTerm=1 if (/(freetext|wordlist|weight|FROM T AS T0 WHERE)/);
    }
    if ($use_libTerm == 3 ) {
        s/([{]\d\.\d{4})\d{2}(E[0-9][}])/\1\2/g;
    }
    if ($use_libTerm == 2 ) {
        s/([,\{]\d\.\d{2})\d{8,12}(E[-0-9])/\1\2/g;
    }
    if ($use_libTerm == 1) {
	# WindowsとlibTermの使い方が異なる？5桁以降を切捨て。
	# ex:1378,1380,1384,1390 (050224)
	# s/A/B/でAの中の「{」に「\」はいらないが、エディタが間違えるので追加。
	s/([,\{]\d\.\d{4})\d{7,10}(E[-0-9])/\1\2/g;
    }
    if ($use_libTerm == 0) {
	# Windowsとdoubleの数字の丸め方が異なる？13,14桁を切捨て。
	# ex:4222,4260,4262 (040913)
	s/([,\{]\d\.\d{12})\d{1,2}(E[-0-9])/\1\2/g;
    }
    #debug版での対応
    next if /bit length overflow/;
    next if /code \d bits \d->\d/;
    next if /code 1\d bits \d->\d/;
    next if /ModInvertedNgramTokenizer/;
    next if /ModInvertedDualTokenizer/;
    next if /ModCommonInitialize/;
    next if /ModParameterSolaris/;
    next if /ModParameter/;

    print;
}
