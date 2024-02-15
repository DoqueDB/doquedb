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
#diffの前処理としてlogを単純化する。

# 以前ここでしていたパス名の変換などの処理は、
# preedit_resultout_for_windows.pl と preedit_resultout_for_linux.pl に移行
# (040609,040617)

# Planなどの出力の余分な箇所を除くのに使う
$Output=1;

# checkpoint の出力変換
$checkpoint=0;

# FakeError の出力
$FakeError=0;

$terminated = 0;
$value = 0;

# Plan
$Plan_index = 0;

# Recovery
$Recovery=0;

while(<>) {
    s/\r+//;

    # 行頭以外のスレッド番号も削除
    # 長い出力は分割されるが、改行が入らないことがあるため
    # (ex: 1114) (040712)
    # スレッド番号の前にプロセス番号が入ることがある (050901)
    s!\((\d+:)?-?\d+\) SydTest::[eE]xecutor\.cpp!SydTest::Executor.cpp!;

    s/((\w+::)?\w+)\.(cpp|h|lemon) \d+/$1/g;
    s/\.cpp,\d+,/.cpp,XXX,/g;
    s/\[(\w+):\d+\]/[$1]/g;
    s/(TimeSpan: )[0-9.]+s/$1/;
    s/makepadding\.pl \d+/makepadding.pl XXX/;
    # tokenは'$'の場合もあるので追加(040824)
    s/near token [a-zA-Z0-9_\$]+/near token XXX/g;
    s/個のディレクトリ+\s+/個のディレクトリ/;
    s/[\d,]+ バイト/          XXX バイト/;
    # スレッド番号の前にプロセス番号が入ることがある (050901)
    s/^\((\d+:)?-?\d+\) //; # Win98ではスレッドIDが負になりうる
    s/[Rr]eorganize([Ii]ndex|[Tt]able)/Reorganize/;
    s/^(\w+)::(\w+): \[ERROR\] /$2: [ERR] ($1) /;
    s/Worker \[\d+\]/Worker []/;
    s/executor/Executor/;
    s/v\d+\.\d+.\d+.\d+/vX.X.X.X/;
    s/v\d+\.\d+/vX.X/;
    # 分散テストのホスト名とポート番号を削除(120730)
    s/((create|alter) cascade .+) (on|to) .+ \d+/$1/;
    # ディレクトリ名の変更に対応(031119)
    s/(single|multi|recovery)_\w+/$1/;
    # Ignore the differences of path-names,
    # which is happened in diskfulltest (ex: 3701.txt).
    # (040617)
    s/(Not enough space on the disk) : '.+'/$1/;
    # moveの出力で、WindowsXPはディレクトリでもファイルと表示するので正規化。
    # (041019)
    s/個のディレクトリを移動しました。/個のファイルを移動しました。/;

    # 2週目のチェックポイントログをなくす(130903)
    $checkpoint=1 if (/DropUser/);
    $checkpoint=0 if (( $checkpoint==1) && ( /Command/ ));
    next if (( $checkpoint==1) && ( /\[ERR\] .* The checkpoint processing is executing/ ));
    s/^Executor: \[ERR\] \(SydTest\)( <<\d>>|) .* The checkpoint processing is executing./SydTest::Executor: \[INFO\]$1 Success./;
    # ↓あまりにうざいので追加(030312)
    next if /\[ERR\] No such file or directory/;
    next if /ModThread: \[ERR] \(message not implemented\)/;
    # ↓あまりにうざいので追加(030703)
    next if /ModNormRule: \[ERR] Bad argument/;
    next if /ModNLP: \[ERR] Bad argument/;
    # さらに追加(040803)
    next if /ModNLP: \[ERR] ModNlpAnalyzer:/;
    # デバッグ環境だと出力が多い
    # ex:single_b1/0411等 (040825)
    next if /file\w*: \[ERR] \(Btree\) /;
    # diskfull (040910)
    next if /(IndexFile|LogicalInterface): \[ERR] \(FullText\) Recovery failed./;
    s/\(Schema::Database\) Database 'DISKFULLTEST' (corrupted|is not available)./\(Schema::Database\) Database 'DISKFULLTEST' may be corrupted./;
    # ERROR 112は十分なDISK容量がないようなエラー? SERVER2008
    s/System Call \'GetOverlappedResult\' error 112 occurred./Not enough space on the disk/;
    next if /OpenMP: .* Not enough space on the disk/;
    # OpenMP ThreadID
    s/^(Utility::OpenMP: \[INFO\] Exception occurred in ThreadID:) \d+/$1 xxxx/;


    # v15では一部のExceptionの出力結果が変更されている(040629)
    s/Bad argument\./Insufficient arguments to function\./g;
    s/Feature not supported\./Not supported./g;
    s/Read-only SQL-transaction/Transaction access is denied/g;
    s/Active SQL-transaction/Already begin transaction/g;

    # v15ではエラーを出力する場所が異なる?
    # ex: recovery/4550 (040729)
    s!Admin,\.\.\\\.\.\\\.\.\\Kernel\\Admin\\Manager\.cpp!Admin,..\\Manager.cpp!i;
    #s!Admin,..\\..\\Admin\\Manager.cpp!Admin,..\\Manager.cpp!;
    # カバレージ環境(c.g含む)では絶対パス表示されるのでこの上の正規化ではhitしない (2005/01/26 shishido)
    s!Admin,d:\\proj\\sydney\\vX\.X\\kernel\\admin!Admin,..!i;

    # not found エラーを正規化する。
    # ex: 4355, multi_b2/5381 (0400823)

    s/Object No=(0x)?[0-9a-zA-Z]+/Object No=xxx/; # Object Noが16進数になったので修正

    s/Database '\w+' is not found\./xxx not found./;
    s/\(Schema::\w+\) (Area|Index|Table|Column) '\w+' not found( in (database|table) '\w+')?\./(Schema::Reorganize) xxx not found./;
    # verifyでは「Schema::***」が頭に付くわけではない
    s/(Area|Index|Table|Column) '\w+' not found( in (database|table) '\w+')?\./xxx not found./;

    # Schema Error を正規化する
    # ex:2011,2017,2021,2851 (040810)
    s/(\(Schema::\w+\)) \w+ '\w+'\s?\w* already defined\s?\w*\s?\w*\s?'?\w*'?\./$1 Insufficient arguments to function./;

    #'Can't alter database TESTDB. Bad path specification.'を削除する(ex:5821)(2005/04/11 khonma)
    s!\(Schema::Reorganize\) Reorganize failed 'Can't alter database TESTDB\. Bad path specification\.'!(Schema::Reorganize) Reorganize failed!;

    #v15ではSyntaxErrorを出す場所が異なる (040803)
    # 一部修正 (040817)
    s!\(Analysis::\w+\) SQL syntax error 'Item Reference: no column is found with the specified name\(\w+\)\.'!(Opt::GraphConversion) xxx not found!;
    s!\(Analysis::\w+\) SQL syntax error '((Insert|Delete|Update) Statement|Table Primary): (table|column) '\w+' does not exist in the (database|table) '\w+'\.'!(Opt::GraphConversion) xxx not found!;


    #v15ではFullTextのファイル構成も変更されている
    # 一部修正 (040909)
    s!\(FullText::[Ll]ogical[Ii]nterface\) Insufficient arguments to function!(FullText::LogicalFormat) Insufficient arguments to function!g;

    # SQL syntax error のtokenを削除する
    # ex: 4713,1300 (040816)
    s/(SQL syntax error):\w+/$1/;
    s/(SQL syntax error ' near token XXX) "\w+"/$1/;

    # NOT NULL constraint violation 以降の出力を削る (2004/08/31)
    # single_b(1,2)/except 0411, 0415, 他多数
    s/NOT NULL constraint violation on the column\s?'?\w*\d*'?\./NOT NULL constraint violation./;

    # Workerが、エラーを起こしたSQL文とObject文とを出力する(しない)場合があるので、出力する場合は削除
    # ex: 1360 (041210)->やっぱりいらない(041217)->やっぱりいる(05/03/01)
    next if /[Ww]orker: \[ERR] \(\w+\) \[\w*] SQL=/;
    unless ( /Connection ran out./ ) {
        next if /[Ww]orker: \[ERR] \(\w+\) Object No=xxx \(/;
    }

    # 実装の都合で変わることがあるので、Object No=xxx (モジュール名) エラー名をObject No=xxx (XXX::XXX)に変換 (05/01/25)
    s/Object No=xxx \(\w+::\w+\)/Object No=xxx \(XXX::XXX::XXX\)/;
    s/Object No=xxx \(\w+::\w+::\w+\)/Object No=xxx \(XXX::XXX::XXX\)/;

    # カバレージ環境でのテストの場合で, cvs-root\coverageと出力される部分をcvs-root\projと変換 (2004/12/06)
    s/coverage/proj/i;

    # recovery 開始時のチェックを消す
    $Recovery=1 if /Main End/;
    $Recovery=0 if /Main Start/;
    next if ($Recovery==1 && /INSTALLDIR.+SydTest/);
    next if /RECOVERYSTART/;

    # 実行環境によりdoubleの数字の丸め方が異なる？13,14桁を切捨て。
    # ex:4222,4260,4262 (040913)
    s/([,\{]\d\.\d{12})\d{1,2}(E[-0-9])/$1$2/g;

    # Terminate の直後は以下の出力を無視する
    next if ($terminated && /Database synchronizer wakeup failed/);

    # Terminate が実行されていたらフラグを立てる
    $terminated = /SydTest.*Terminate$/;

    # 出力の修正
    s/^(mv: cannot stat) `/$1 '/;

    #time   minの十の桁まで
    $value = 1 if ( /CURRENT_TIMESTAMP/ ) ;
    $value = 1 if ( /system_session/ ) ;
    $value = 1 if ( /current_timestamp/ ) ;
    if ( $value == 1 ) {
        ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time);
        $year = $year + 1900 ;
        $mon = $mon + 1 ;
        $sec = "", $mday = "", $wday = "", $yday = "", $isdst ="";
        $hour = "0$hour" if ( $hour < 10 ) ;
        $min = "0$min" if ( $min < 10 ) ;
        $min_ten=int( $min/10 );
        s/$hour:$min_ten\d:\d+\.\d+,[0|2]/$hour:$min_ten :xx.xxx,xx/g;
        s/$hour:$min_ten\d:\d+\.\d+/$hour:$min_ten :xx.xxx/g;
    }

    # 2周目 multi/normal/9434
    s!(:xx.xxx\,QueryExpression\,)ReadOnly!$1VersionUse!;

    # ddコマンド single/normal/24050
    s!(\d+ bytes \(\d+ (k|)B\) copied).+!$1!;

    # single_dist/normal/211000
    s/\{(A|B)-9\d{7}\}/\{$1-9xxxxxxx\}/;

    # single_dist/normal/214302
    $FakeError=1 if ( /Exception_FakeError/ );
    if ( $FakeError == 1 )
    {
        s/\{JP199\d{7}\}/\{JP199xxxxxxx\}/;
    }

    # Server_PrintTime 1
    s!\[DEBUG\] (Executor|Optimizer)(\(prepare\)|) Time: 00:00:00.\d\d\d!\[DEBUG\] $1 Time: 00:00:00.xxx!;

    next if/\[DEBUG\]( Erase | )PREPARE-ID: \d/;  # /Pオプション
    next if/\[DEBUG\] Worker Time: 00:00:0\d.\d\d\d/; # /Pオプション
    next if/\[DEBUG\] Optimizer\(generate\) Time: 00:00:0\d.\d\d\d/; # /Pオプション

    # Plan 出力
    next if( /\[INFO\] \[TEST\]/ && /(SQL|Parameter)=/ );
    next if(/\{(BEGIN|END) PLAN\}/);
    $Output=3 if(/^Opt::Generator/);
    $Output=8 if(/explain execute/);
    $Output=9 if(/start explain/);
    if($Output > 2) {
        s/\d+:(\D)/xx:$1/;
        s/\#\d+/\#xx/g;
        s/\[lock\?\](\}|)//g;
        $Plan_index=0 if /End Of Data/;
        $Plan_index=0 if /execute/;
        $Plan_index=1 if /index (scan|fetch) on/;
        if ($Plan_index == 1) {
            $Plan_index=9 if($Output == 9);
            s/ to data:\{#xx,#xx\}/ to data:\{#xx\}/;
        }
        if ($Plan_index == 9) {
            s/ to data:\{(#xx,#xx|#xx)\}(\}|)//;
            s/(\{index scan on) (.+)$/$1 $2\}/;
            s/^SydTest::Executor: \[INFO \d*\](\s)(\s+<--|.+\[get by bitset\])/$1$2/;
            s/^(\s+)(<-- .+)(\[l)$/$1            $2/;
            s/^(\s+)(<-- .+\[get by b)$/$1            $2itset\]/;
            next unless ((/SQL Query/) || (/\s\s\w+:/) || (/<--/) || (/\{/) || (/get by bitset/));
        }
        if ($Output == 9) {
            s/ for//;
            unless ((/select/) || (/INFO \d/)){ s/(TBL|T|JP)\.(.+)/$2/; }
            if((/sequential scan on/) || (/index (fetch|scan) on/)){ s/(TBL|T)\d\.(.+)/$2/;}
            next if((/SydTest::Executor: \[INFO \d*\]/) || (/^\s\s\s+group by/) || (/lock tuple/) || (/^\s\s\s+T\d\.\D+ = T\d.\w/));
        }
        next if((/Opt::Generator: \[DEBUG 1\].+/) || (/output to <connection:/) || (/(unlock tuple|lock table|start up)/) || (/xx:check cancel/));

        s/xx:(cast|case).+/xx:xxxx/;
        s/xx:ca$/xx:xxxx/;
        s/\[locked\]//; # /Pオプション
        s/do until line \d+ unless /do until line xx unless /;
    }

    $Output=1 if((/SydTest::/) && (/End Of Data/) && ($Output != 9 ));
    $Output=1 if(/end explain/);
    print if ( $Output >= 1 );
}

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
