# process-aozora-data.pl - 青空文庫作品データ加工ツール
# 
# Copyright (c) 2023 Ricoh Company, Ltd.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the “Software”), to deal in
# the Software without restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
# Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# # COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# ■概要
# 青空文庫作品データをサンプルデータと同じ形式に加工してOUTPUTの下に出力する。
# 以下のデータが必要。
#   list_person_all_extended.csv：書誌情報ファイル
#   aozorabunko_text-master：作品データを格納したフォルダ
#   OUTPUT：出力先フォルダ(なければ作成される)
#
# ■書誌情報
# 書誌情報はCSVファイルであり、以下のデータを参照する。
#   第0列：作品ID(ゼロ埋め6桁)
#   第9列：文字遣い種別(新字新仮名, 旧字旧仮名)
#   第10列：作品著作権フラグ(あり, なし)
#   第14列：人物ID(ゼロ埋め6桁)
#   第45列：テキストファイルURL
#   第47列：テキストファイル符号化方式(ShiftJIS, UTF-8)
# テキストファイルURLの形式は以下のとおり。
#   https://www.aozora.gr.jp/cards/{人物ID(ゼロ埋め6桁)}/files/{作品識別名}.zip
# 以下の形に加工することでテキストデータを参照できる。
#   aozorabunko_text-master/cards/{人物ID(ゼロ埋め6桁)}/files/{作品識別名}/{作品識別名}.txt
# 
# ■ファイル構造
# 著作権の切れた作品のテキストデータは以下の構造をもつ。
#   作品名等の書誌情報(複数行)
#   1行開き
#   テキスト中に現れる記号についての情報(ないこともある)
#   1行開き(テキスト中に現れる記号についての情報がなければ省略)
#   本文
#   3行開き
#   底本情報(ない場合は［＃本文終わり］を置く)
# 
# ■テキストデータ中の特殊文字
# テキスト中に現れる以下の文字は特殊文字として扱う。
# 複数文字からなる特殊文字は行をまたがない。
#   ［＃…］：注記 (例：［＃ここから１字下げ］)
#   《…》：ルビ (例：惑《まど》ふ)
#   ｜：ルビ開始位置 (例：武州｜青梅《おうめ》の宿)
#   〔…〕：アクセント分解(例：〔e'tranger〕)
#   ※［＃…］：外字や特殊記号 (例：※［＃「將」の「爿」に代えて「将のへん」］)
#   ／＼：くの字点
#   ／″＼：濁点付きのくの字点
#   罫線素片：表組み、罫囲み
# 罫線素片としては以下のものが使われる。
#   ─│┌┐┘└├┬┤┴┼━┃┏┓┛┗┣┳┫┻╋┠┯┨┷┿┝┰┥┸╂
# 
# ■エスケープ
# テキスト中に特殊文字が出現する場合は以下の注記に置き換えられている。
#   《：※［＃始め二重山括弧、1-1-52］
#   》：※［＃終わり二重山括弧、1-1-53］
#   ［：※［＃始め角括弧、1-1-46］
#   ］：※［＃終わり角括弧、1-1-47］
#   〔：※［＃始めきっこう（亀甲）括弧、1-1-44］
#   〕：※［＃終わりきっこう（亀甲）括弧、1-1-45］
#   ｜：※［＃縦線、1-1-35］
#   ＃：※［＃井げた、1-1-84］
#   ※：※［＃米印、1-2-8］
# 
# ■作品データの加工
# 著作権の切れた作品のみ登録する。
# 登録するテキストは以下のように加工する。
# ・旧字旧仮名はそのままとする
# ・改行はLFまたはCR/LFとする(作業環境による)
# ・作品名等の書誌情報はそのまま出力する
# ・テキスト中に現れる記号についての情報は削除する
# ・底本情報とその前の3行開きは削除する
# ・注記、ルビ、ルビ開始位置は削除する
# ・アクセント分解はそのまま残す
# ・外字や特殊記号は「※」だけを残す
# ・くの字点、濁点付きのくの字点はそのまま残す
# ・エスケープされた特殊文字は元の文字に戻す
# ・罫線素片(およびその並び)は全角スペース1字に変換する
# ・上記加工後、連続する空行は1行にまとめる
# ・本文終了後の空行は削除する
# 加工したテキストは以下のファイルに置く。
#   {人物ID(ゼロ埋め6桁)}_{作品ID(ゼロ埋め6桁}.txt
#
# 最後に以下の情報を出力する。
#   作品数
#   総文字数
#   作品あたり平均文字数
#   文字数範囲ごとの作品数
#     ～4K文字, ～8K文字, ～16K文字, ～32K文字, ～64K文字, ～128K文字, それ以上
#   新字新仮名作品数
#   旧字旧仮名作品数
#
# その他以下の情報を出力する。
#   insert.csv: OUTPUTに出力された作品の書誌情報, 以下の加工あり
#     第0列：作品ID(二重引用符削除)
#     第55列：出力ファイル名(新規追加で二重引用符あり)
#     第56列：外部ファイル名(新規追加で二重引用符なし, 書式：FILE OUTPUT/<出力ファイル名>)
#   process-aozora-data.log: ログファイル
#
# Usage: perl process-aozora-data.pl
# Input:
#   list_person_all_extended.csv: bib information
#   aozorabunko_text-master: text data directory
# Output:
#   insert.csv: bib information for output files
#   OUTPUT/xxxxxx-yyyyyy.txt (xxxxxx:personID, yyyyyy:docID)
#   process-aozora-data.log: log
#
use utf8;
use strict;
use warnings;
use Text::ParseWords;
use Encode;
use IO::Handle;
use open IN  => ":encoding(cp932)";
use open OUT => ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";
STDERR->autoflush(1);

#出力先フォルダを作る
if (! -d "OUTPUT") {
	mkdir "OUTPUT" || die "mkdir OUTPUT failed: $!\n";
}

#統計情報
my $doc_count = 0;			#作品数
my $char_count_min = 0;		#文字数最小の作品の文字数
my $out_file_min = "";		#文字数最小の作品の出力先ファイル名
my $char_count_max = 0;		#文字数最大の作品の文字数
my $out_file_max = "";		#文字数最大の作品の出力先ファイル名
my $char_count_total = 0;	#文字数の合計
my @docs_per_size = (0, 0, 0, 0, 0, 0, 0);	#サイズ範囲ごとの作品数
my $docs_new_char = 0;		#新字新仮名の作品数

#エスケープ復元用データ
my $escape = {
	"《" => "＃始め二重山括弧",
	"》" => "＃終わり二重山括弧",
	"［" => "＃始め角括弧",
	"］" => "＃終わり角括弧",
	"〔" => "＃始めきっこう",
	"〕" => "＃終わりきっこう",
	"｜" => "＃縦線",
	"＃" => "＃井げた",
	"※" => "＃米印"
};
my $count = 0;
my ($table1, $table2);
map {
	$table1->{$escape->{$_}} = ++$count;
	$table2->{$count} = $_;
} keys(%$escape);
my ($pattern1, $pattern2) = map {
	q[(?:] . 
	join(q(|), 
		map { quotemeta $_}
		sort { length($b) <=> length($a) }
		keys(%$_)) .
	q[)]
} ($table1, $table2);

my $log_file = "process-aozora-data.log";
open(LOG, ">$log_file") || die "can't open $log_file: $!\n";

#書誌情報ファイルを見て対象作品を加工する
my $bib_in  = "list_person_all_extended.csv";
my $bib_out = "insert.csv";
open(BIBIN,  "<$bib_in")  || die "can't open $bib_in $!\n";
open(BIBOUT, ">$bib_out") || die "can't open $bib_out $!\n";
my $header = <BIBIN>;	#最初の1行はカラム見出し
$header =~ s/\r?\n$//;
#print BIBOUT "$header,出力ファイル名\n";
$count = 0;	#処理件数
my %processed = ();	# 処理済みの作品
while (<BIBIN>) {
	s/\r?\n$//;

	#何件めを処理しているか出力する
	print STDERR "\r$count " if ++$count % 10 == 0;

	#CSVの行をカラムに分割する(改行を含むカラムはない)
	my $line = $_;
	my @data = &parse_line(',', undef, $line);
	my $doc_id      = $data[ 0];	#作品ID(ゼロ埋め6桁)
	my $char_type   = $data[ 9];	#文字遣い種別(新字新仮名, 旧字旧仮名)
	my $copyr_flag	= $data[10];	#作品著作権フラグ(あり, なし)
	my $person_id   = $data[14];	#人物ID(ゼロ埋め6桁)
	my $text_url    = $data[45];	#テキストファイルURL

	#重複する作品はスキップ
	next if exists $processed{$doc_id};
	$processed{$doc_id} = 1;

	#著作権の切れていない作品はスキップ
	next if $copyr_flag eq "あり";

	#該当作品を加工して出力する
	my $out_file = "${person_id}_${doc_id}.txt";
	my $char_count = &process_document($doc_id, $text_url, "OUTPUT/$out_file");
	if ($char_count < 0) {
		#該当作品のファイルがなかった；単にスキップする
		#ファイルがなかった原因はprocess_document()でログに書かれている
		next;
	}

	#書誌情報と統計情報を記録する
	$line =~ s/^"(\d{6})"/$1/;
	print BIBOUT "$line,\"$out_file\",FILE OUTPUT/$out_file\n";
	&update_stats($char_count, $out_file, $char_type);
}
close(BIBIN);
close(BIBOUT);
print STDERR "\r$doc_count \n";

#統計情報を出力する
&print_stats;
close(LOG);
exit;

#1件の作品を加工して出力する
sub process_document
{
	my ($doc_id, $text_url, $out_path) = @_;

	#テキストファイルURLをフォルダ内のファイルパスに変換する
	if (!$text_url) {
		#テキストファイルURLがない
		#メッセージをログに書いて処理をスキップする
		print LOG "$doc_id: テキストURLが空\n";
		return -1;
	}
	if ($text_url !~ /^https:\/\/www.aozora.gr.jp\/cards\//) {
		#テキストファイルURLが青空文庫の外部を指していた
		#メッセージをログに書いて処理をスキップする
		print LOG "$doc_id: テキストが青空文庫の外にある ($text_url)\n";
		return -1;
	}
	$text_url =~ /^https:\/\/www.aozora.gr.jp\/(.+)\/([^\/]+)\.zip$/;
	my $in_path = "aozorabunko_text-master/$1/$2/$2.txt";

	#作品ファイルを読んで加工する
	if (!open(IN, "<$in_path")) {
		#作品ファイルが開けなかった
		#(書誌情報とデータの実体に不整合がある)
		#メッセージをログに書いて処理をスキップする
		print LOG "$doc_id: 作品ファイルがない ($in_path)\n";
		return -1;
	}
	if (!open(OUT, ">$out_path")) {
		#出力先がオープンできなかった
		print LOG "$doc_id: 出力先がオープンできない ($out_path)\n";
		return -1;
	}
	my $inside_bib = 1;		#冒頭の書誌情報を処理中(1:処理中, 2:処理中でない)
	my $inside_note = 0;	#テキスト中に現れる記号についての情報を処理中(1:処理中, 2:処理中でない)
	my $blank_line = 0;		#空行の有無(1:出力していない空行がある, 0:〃がない)
	my $char_count = 0;		#出力文字数の情報
	while (<IN>) {
		s/\r?\n$//;
		my $line = $_;

		#連続する空行はまとめておく
		#冒頭の書誌情報は空行が来たら終わり
		if ($line =~ /^$/) {
			$blank_line = 1;
			if ($inside_bib) {
				$inside_bib = 0;
			}
			next;
		}

		#テキスト中に現れる記号についての情報は
		#全体を出力から除外する(扱いとしては空行)
		if (!$inside_note && $line =~ /^------------------------------/) {
			$inside_note = 1;
			$blank_line = 1;
			next;
		}
		if ($inside_note) {
			if ($line =~ /^------------------------------/) {
				$inside_note = 0;
			}
			next;
		}

		#冒頭の書誌情報出力中なら入力をそのまま出力する
		if ($inside_bib) {
			print OUT "$line\n";
			$char_count += length($line) + 1;
			next;
		}

		#末尾の底本情報が来た
		if ($line =~ /^(底本：|底本:|［＃本文終わり］)/) {
			#作品ファイルの読み込みを終了する
			#出力していない空行は出力しないで終わる
			last;
		}

		#ここまで来たら本文
		#エスケープ文字をローカル表現に変換する
		$line =~ s/※［($pattern1)[^］]*］/\x{fff0}$table1->{$1}\x{fff1}/go;

		#注記とルビを削除する
		#外字や特殊記号を※に変換する(注記部分のみ削除する)
		#罫線素片の並びを全角スペースに変換する
		$line =~ s/［＃[^］]*］//g;
		$line =~ s/《[^》]+》//g;
		$line =~ s/｜//g;
		$line =~ s/[─│┌┐┘└├┬┤┴┼━┃┏┓┛┗┣┳┫┻╋┠┯┨┷┿┝┰┥┸╂]+/  /g;

		#ローカル表現を元の文字に戻す
		$line =~ s/\x{fff0}($pattern2)\x{fff1}/$table2->{$1}/go;

		if ($line) {
			#加工後も文字が残っていたので出力する
			#出力していない空行があったら本文の前に空行を1行出力する
			if ($blank_line) {
				print OUT "\n";
				$char_count++;
				$blank_line = 0;
			}

			#加工した本文を出力する
			print OUT "$line\n";
			$char_count += length($line) + 1;
		} else {
			#加工後に文字がなくなったので空行とする
			$blank_line = 1;
		}
	}
	close(IN);
	close(OUT);
	return $char_count;
}

#統計情報を記録する
sub update_stats
{
	my ($char_count, $out_file, $char_type) = @_;

	$doc_count++;
	$char_count_total += $char_count;
	if ($char_count_min == 0 || $char_count < $char_count_min) {
		$char_count_min = $char_count;
		$out_file_min = $out_file;
	}
	if ($char_count_max == 0 || $char_count > $char_count_max) {
		$char_count_max = $char_count;
		$out_file_max = $out_file;
	}
	if ($char_count <= 4096) {
		$docs_per_size[0]++;
	} elsif ($char_count <= 8192) {
		$docs_per_size[1]++;
	} elsif ($char_count <= 16384) {
		$docs_per_size[2]++;
	} elsif ($char_count <= 32768) {
		$docs_per_size[3]++;
	} elsif ($char_count <= 65536) {
		$docs_per_size[4]++;
	} elsif ($char_count <= 131072) {
		$docs_per_size[5]++;
	} else {
		$docs_per_size[6]++;
	}
	if ($char_type eq "新字新仮名") {
		$docs_new_char++;
	}
}

#統計情報を出力する
sub print_stats
{
	print "作品数：$doc_count\n";
	print "総文字数：$char_count_total\n";
	print "作品あたり平均文字数：" . int($char_count_total / $doc_count + 0.5) . "\n";
	print "文字数最小の作品とその文字数：$char_count_min ($out_file_min)\n";
	print "文字数最大の作品とその文字数：$char_count_max ($out_file_max)\n";
	print "文字数範囲ごとの作品数：\n";
	print "  ～4K文字：$docs_per_size[0]\n";
	print "  ～8K文字：$docs_per_size[1]\n";
	print "  ～16K文字：$docs_per_size[2]\n";
	print "  ～32K文字：$docs_per_size[3]\n";
	print "  ～64K文字：$docs_per_size[4]\n";
	print "  ～128K文字：$docs_per_size[5]\n";
	print "  それ以上：$docs_per_size[6]\n";
	print "新字新仮名作品数：$docs_new_char\n";
	print "旧字旧仮名作品数：" . ($doc_count - $docs_new_char) . "\n";
}
