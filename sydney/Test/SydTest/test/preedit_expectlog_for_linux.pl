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
# 正解ログに書かれているシステムコマンド出力の削除に使う
# SydTest.sh から呼び出す。
# 空行の処理はSydTest.shのdiffのBオプションにまかせる。
# Linux のみ使う。(040823) 
$user=`whoami`;
chomp($user);
$mytop="/proj/sydney/work/$user";

# スコアを丸めるために使う。
$use_libTerm=0;

while (<>) {
    next if /\d+ 個のファイルを移動しました。/;
    next if /\d+ 個のファイルをコピーしました/;

    #
    s/d:\\Mssql7\\Data/d:\/Mssql7\/Data/;

    # recoveryの4500.log等への対応(040611)
    # 2860は大文字だった (040728)
    next if /d:\\proj\\sydney\\vX.X\\[tT]est\\[sS]yd[tT]est\\test\\(single|recovery)\\normal>echo off/;

    s!c:/Program Files/Ricoh/TRMeister/db/(data|system)/(TESTDB\d|TESTDB)!d:\\dm\\$1\\$2!g;
    s!area (a modify|a) 'c:/Program Files/Ricoh/TRMeister/db/(\w+)!area $1 'd:\\dm\\$2!;

    # "mkdir" is normalized to "md". ex:2736 (040628)
    s!\[System Parameter] mkdir![System Parameter] md!;

    # スコア違い対応
    # freetext, wordlistを使っているSQL文はlibTermを使っている。
    # weightもそうかもしれない。(050224)
    # 1368のfreetextがif文を通過するのでFROM T AS TO を追加
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
    # diskfull
    next if /^FullText2::FullTextFile: \[INFO\] FullTextFile::(insert|update|expunge)$/;
    next if /^FullText2::InvertedSection: \[INFO\] InvertedSection::(insert|expunge)LocationList$/;

    # Execution_OverflowNull
    if (/Execution_OverflowNull/){
        $use_libTerm=3 if ( -e "$mytop/opt/RICOHtrm/lib/libimf.so" );
    }
    if ($use_libTerm == 3 ) {
        s/(2.22507\d+E-308)\,(-|)(4.45)(\d+|)(E-308)\,(-|)(2.22\d+E-308)/$1\,$2$3$4$5\,${6}0E0/;
    }
    s!Admin,..\\manager.cpp!Admin,..\\Manager.cpp!;
    print;
}
#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
