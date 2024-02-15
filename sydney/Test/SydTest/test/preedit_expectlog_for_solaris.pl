# 正解ログに書かれているシステムコマンド出力の削除に使う
# SydTest.sh から呼び出す。
# 空行の処理はSydTest.shのdiffのBオプションにまかせる。
# Linux のみ使う。(040823) 

# スコアを丸めるために使う。
$use_libTerm=0;

# Solarisの対応していないテストを除くために使う
$Output=1;

while (<>) {
    next if s/^\s*$//g;
    next if /\d+ 個のファイルを移動しました。/;
    next if /\d+ 個のファイルをコピーしました/;

    #
    s/d:\\Mssql7\\Data/d:\/Mssql7\/Data/;

    # recoveryの4500.log等への対応(040611)
    # 2860は大文字だった (040728)
    next if /d:\\proj\\sydney\\vX.X\\[tT]est\\[sS]yd[tT]est\\test\\(single|recovery)\\normal>echo off/;
    # "mkdir" is normalized to "md". ex:2736 (040628)
    s!\[System Parameter] mkdir![System Parameter] md!;

    s!c:/Program Files/Ricoh/TRMeister/db/(data|system)/(TESTDB\d|TESTDB)!d:\\dm\\$1\\$2!g;
    s!area (a modify|a) 'c:/Program Files/Ricoh/TRMeister/db/(\w+)!area $1 'd:\\dm\\$2!;

    # hint 'sectionized' がsolarisで使えない
    if ( /sectionized/ ) {
        $sectionized=1;
	s/ hint \'sectionized\'//;
        if ( / hint \'(\w+|sectionized)\, (sectionized|inverted|delayed)/ ) {
            s/sectionized\, //;
        }
        if ( / hint\(\'delayed\'\,\'sectionized\'\)/ ) {
            s/\,\'sectionized\'//;
        }
	$Output=2 if ( /select N\, sectionized\((C|C\d)\) / && $Output != 3 && !( /PreparedCommand/ ));
        $Output=2 if (!( /select\[/ ) && /select N\, sectionized\((C|C\d)\)/ && !( /PreparedCommand/ ));
	if ( /select N\, sectionized\((C|C\d)\)/ ) {
	    s/sectionized\((C|C\d)\) //;
        }
        $Output=2 if ( /select (C|C\d), sectionized\(C\d\)/ || /SELECT sectionized\(\(T0\./ || /select SN\, sectionized\(Content/ || /select sectionized\(ContentText\)  from/ ); 
    }
    if ( $sectionized == 1 ) {
        next if /AssureCount/;
        #$Output=3 if ( /PreparedCommand/ );
    }

    # スコア違い対応
    # freetext, wordlistを使っているSQL文はlibTermを使っている。
    # weightもそうかもしれない。(050224)
    # 1368のfreetextがif文を通過するのでFROM T AS TO を追加
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
    if ($use_libTerm == 1 ) {
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
    
    # bad_allocのFakeエラーのテスト

    if ( $Output == 0 ) {
        $Output=1 if ( /(Success.|Command|TerminateSession)/ );
    }
    print if ( $Output == 1 );
    if ( $Output == 2 ) {
        $Output=0;
        split /(SELECT|select)/;
        print @_[0], "\n";
    } 
    if ( $Output == 3 ) {
	$Output=1 if ( /Success/ );
        $Output=0 if ( /ErasePreparedCommand/ ); 
    }
}
