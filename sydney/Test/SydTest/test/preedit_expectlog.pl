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
# 正解ログの normalize に使う
# SydTest.sh から呼び出す。(040617) 

# ホスト名の取得
chomp($host = `hostname`);

while (<>) {
    # スレッド番号の削除
    # log_normalizer.plで行頭条件を使って削除しているが、
    # 長い出力行を分割する時、改行が入らないことがある
    # (ex: 1114.log) (040712)
    s!\(-?\d+\) SydTest::Executor!SydTest::Executor!;

    # エラー表記の書式変換がrecovery_b1/6201で未適用
    # (040823)
    s/^(\w+)::(\w+): \[ERROR\] /\2: [ERR] (\1) /;

    # Ignore the differences of path-names,
    # which is happened in diskfulltest (ex: 3701.txt).
    s/(Not enough space on the disk) : '.+'/$1/;

    # not found エラーを正規化する。
    # ex:4711, multi (040726)
    
    s/Object No=\d+/Object No=xxx/;
    # Object Noが16進数のもあるので修正
    s/Object No=(0x)?[0-9a-zA-Z]+/Object No=xxx/;

    s/Database '\w+' is not found./xxx not found./;
    s/\(Schema::\w+\) (Area|Index|Table|Column) '\w+' not found in (database|table) '\w+'./(Schema::Reorganize) xxx not found./;
    s/(Area|Index|Table|Column) '\w+' not found in (database|table) '\w+'./xxx not found./;

    # SQLParserLが、SQL syntax error行を出力しなくなった。
    # ex:1300 (041210)->やっぱりいらない(041217)
    #next if/SQLParserL: \[ERR] \(Statement\) SQL syntax error/;
    # SQL syntax error のtokenを削除する
    # ex: 4713 (040810)
    s/(SQL syntax error):\w+/$1/;
    s/(SQL syntax error ' near token XXX) "\w+"/$1/;
    # XXXに変換されずに$のまま作られたのがある 4710 (040824)
    s/(SQL syntax error ' near token) \$/$1 XXX/;

    # デバッグ環境で作られたため出力が多い
    # ex:4153,b1/0411等 (040825)
    next if /ModNLP: \[ERR] ModNlpAnalyzer:/;
    next if /file\w*: \[ERR] \(Btree\) /;

    # error出力の仕様変更？
    # 正解ログの出力が多い
    # ミススペルを利用して余分な行を削除
    # ex: 4201-4221 (040816)
    next if /occored/;

    # Workerが、エラーを起こしたSQL文とObject文とを出力する(しない)場合があるので、出力する場合は削除 
    # ex: 1360 (041210)->やっぱりいらない(041217)->やっぱりいる(05/03/01)
    next if /[Ww]orker: \[ERR] \(\w+\) \[\w*] SQL=/;
    next if /[Ww]orker: \[ERR] \(\w+\) Object No=xxx \(/;

    # Key?が、エラーを出力しなくなった。
    # 2701のみ？ (041210)->やっぱりいらない(041217)
    #next if /Key: \[ERR] \(Schema\) Illegal key definition: invalid column /;

    # 実装の都合で変わることがあるので、Object No=xxx (モジュール名) エラー名をObject No=xxx (XXX::XXX)に変換 (05/01/25)
    s/Object No=xxx \(\w+::\w+\)/Object No=xxx \(XXX::XXX::XXX\)/;
    s/Object No=xxx \(\w+::\w+::\w+\)/Object No=xxx \(XXX::XXX::XXX\)/;

    # 空白を比較するようにしたことでの対応(050623)
    s/個のディレクトリ+\s+/個のディレクトリ/;
    s/XXX バイトの空き領域/          XXX バイトの空き領域/;

    # 実行環境によりdoubleの数字の丸め方が異なる？13,14桁を切捨て。
    # ex:4222,4260,4262 (040913)
    s/([,\{]\d\.\d{12})\d{1,2}(E[-0-9])/\1\2/g;

    # 
    s!move /Y d:\\trchild(\d|)\\log\\doquedb.log!mv -f 'c:\\Program Files/Ricoh/TRMeister$1/log/syslog.csv'!;
    s!d:\\trchild(\d|)\\log\\doquedb.log!'c:\\Program Files/Ricoh/TRMeister$1/log/syslog.csv'!g;
    if ( /Connection ran out./ ) {
        s!d:\\trchild\*/log/doquedb.log!d:\\trchild/log/doquedb.log!;
    }

    #time   minの十の桁まで
    if ( /20YY-MM-DD hh:mm:ss.xxx/) {
        ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time);
        $y = $year+1900;
        $m = $mon+1;
        $m = "0$m" if ( $m < 10 ) ;
        $mday = "0$mday" if ( $mday < 10 ) ;
        $hour = "0$hour" if ( $hour < 10 ) ;
        $min = "0$min" if ( $min < 10 ) ;
        $min_ten=int( $min/10 );
        s/20YY-MM-DD hh:mm:ss/$y-$m-$mday $hour:$min_ten :xx/g;
    }

    s/\$hostname/$host/;

    print;
}
#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
