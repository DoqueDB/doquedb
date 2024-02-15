# テストスクリプトに書かれているパス名の変換に使う
# SydTest.sh から呼び出す。
# linux用。(040610,040616) 
# 060203修正

$user=`whoami`;
chomp($user);
$mytop="/proj/sydney/work/$user";
$dbtop="/proj/sydney/data/SydTest/solaris_db/SydTest";
$paddingc = 0;
$Output=1;
$result=1;

while (<>) {
    # 2721.txt で create database AreaTest に対して、
    # Exists "d:\\dm\\data\\areatest" で統一されていない
    # 大文字小文字を統一する。
    # diff は iオプションを付けて実行しているので、
    # preedit_resultout_for_linux.pl で元に戻す必要はない
    s!areatest!AreaTest!g;

    s!..\\\\..\\\\restore.bat!sh ../../restore.sh!;

    # パス名の変更
    s![dD]:\\\\dm\\\\data\\\\Cabinet1\\\\Sydney!$mytop/opt/RICOHtrm/db/data/Cabinet1/Sydney!g;
    s![dD]:\\\\dm\\\\(data|area|alterarea|system)\\\\!$mytop/opt/RICOHtrm/db/\1/!g;
    s![dD]:\\\\dm\\\\(data|area|alterarea|system|alter_data_TESTDB|alter_system_TESTDB)!$mytop/opt/RICOHtrm/db/\1!g;
    s!mkdir \\"d:\\\\dm\\\\backup\\\\DBDatabaseTest!mkdir -p \\"d:\\\\dm\\\\backup\\\\DBDatabaseTest!;
    s![dD]:(\\\\|/)dm!$mytop/opt/RICOHtrm/db!g;
    s!X:/SydTest!$dbtop!g;
    s!x:\\\\benchmark!/proj/sydney/data/SydTest/linux!g;
    s!d:\\\\benchmark!/proj/sydney/data/SydTest/linux!g;

    s!h:\\\\diskfulltest\\\\(data|log|system)!/diskfull/diskfulltest/\1!g;
    s!(\W)h:\\\\!$1/diskfull/!g;
    s!(\W)h:!$1/diskfull!g;
    s!perl ..\\\\..\\\\makepadding.pl!perl ../../makepadding.pl!;
    s!\\"c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\user.pswd\\"!$mytop/opt/RICOHtrm/etc/user.pswd!g;
    s!\\"c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\test.pswd\\"!$mytop/opt/RICOHtrm/etc/test.pswd!g;
    s!c:\\\\Program Files\\\\Ricoh\\\\TRMeister\\\\test.pswd!$mytop/opt/RICOHtrm/etc/test.pswd!;
    s!c:/Program Files/Ricoh/TRMeister/db/(data|system)/(\w+)!$mytop/opt/RICOHtrm/db/\1/\2!g;
    s!area (a modify|a) 'c:/Program Files/Ricoh/TRMeister/db/(\w+)!area $1 '$mytop/opt/RICOHtrm/db/$2!;

    # SydTest.exe に /R オプションをつけない
    $result = 0 if ( /(ESCAPE|escape) (\'\\\\\'|\(\'\\\\\'\))/ ) ;
    $result = 0 if ( /FieldSeparator=\\"\\\\t\\"/ ) ;
    $result = 0 if ( /RecordSeparator=\\"\\\\n===\\\\n\\"/ ) ;
    $result = 0 if ( /RecordSeparator=\\":\\\\n==\\\\n:\\"/ ) ;
    $result = 0 if ( /select f from TBL where g like \'\%\\\\\%\'/ ) ;
    $result = 0 if ( /Bug report 1324/ ) ;
    
    #ランダム
    if (/rand_max_(\d+)/) {
        $T = int(rand($1))+1;
        s!rand_max_\d+!$T!;
    }

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
        $Output=2 if (!( /select\[/ ) && /select N\, sectionized\((C|C\d)\)/ && !( /PreparedCommand/ ));
        if ( /select N\, sectionized\((C|C\d)\)/ ) {
            s/sectionized\((C|C\d)\) //;
        }
        $Output=2 if ( /select (C|C\d), sectionized\(C\d\)/ || /SELECT sectionized\(\(T0\./ || /select SN\, sectionized\(Content/ || /select sectionized\(ContentText\)  from/ );
    }
    if ( $sectionized == 1 ) {
        next if /AssureCount /;
        #$Output=0 if ( /PreparedCommand / && /sel/ );
    }

    #diskfullテストのpaddingサイズを変える
    if ( /makepadding\.pl \d+ > / ){ 
        if ( /Solaris/ ) {
            s/# Solaris //;
            $solarisdisk = 1;
        }
        elsif ( $solarisdisk == 1 ) {
            next;
        }
    }
    print if ( $Output == 1 );
    if ( $Output == 0 ) {
        $Output=1 if ( /\;/ );
    }
    if ( $Output == 2 && /\;|verify/ ) {
       $Output=1;
       printf "Command \"\"\;\n";
    }
}
exit $result;
