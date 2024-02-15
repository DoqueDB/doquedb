#!/usr/bin/perl
# 
# Copyright (c) 2000, 2014, 2023 Ricoh Company, Ltd.
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
#
# MkMoDic.pm - Make (UNA) Morphological Dictionary
#

package UNA::MkMoDic;

=head1 NAME

 UNA::MkMoDic - 形態素辞書ソースと品詞リストを作る

=head1 SYNOPSIS

 use UNA::MkMoDic;
 $mkdic = new UNA::MkMoDic;
 $mkdic->load('dic', $dic_file, $mpat);
 $mkdic->load('gobitbl', $gobitbl_file);
 $mkdic->load('hingrp', $hingrp_file);
 $mkdic->load('fealst', $fealst_file);
 $mkdic->load('hinmap', $hinmap_file);
 $mkdic->load('fixhin', $fixhin_file);
 $mkdic->load('hindomap', $hindomap_file);
 $mkdic->load('hantbl', $hantbl_file, $tmp_dir);
 $mkdic->load('unihinmap', $unihinmap_file);
 $mkdic->load('termtype', $termtype_file);
 $mkdic->load('termtypemap', $termtypemap_file);
 $mkdic->load('hinlst', $hinlst_file);
 $mkdic->load('hinhindotbl', $hinhindotbl_file);
 $mkdic->load('dichindotbl', $dichindotbl_file);
 $mkdic->make_morphdic($morphdic_file, $info_pat, \&arranger, $opt_s, $opt_F, $opt_N, $opt_j, $opt_B);
 $mkdic->make_hinlst($hinlst_file, $morph_h_file, $jptbl_file, $prefix, $opt_F, $opt_j, $opt_B, $dic_file);
 $mkdic->make_convtbl($convtbl_file, $prefix, $opt_B);
 $mkdic->make_termtypetbl($termtypetbl_file);
 $mkdic->make_hinhindotbl($hinhindotbl_file, $opt_N);
 $mkdic->{tmpfile1} = "$tmp_dir/\#MkMoDic.$$.1";
 $mkdic->{tmpfile2} = "$tmp_dir/\#MkMoDic.$$.2";

=head1 DESCRIPTION

 形態素辞書ソースと品詞リストを作る。
 C用ヘッダファイルを作ることもできる。
 各種データファイルをloadメソッドで読みこんだ後、
 make_morphdicメソッドやmake_hinlstメソッドで、ファイルを作る。
 $info_patにより、形態素辞書ソースの形式を指定できる。
 ファイルの文字コードは、$icodeや$ocodeで指定できる。
 中間ファイルは、出力デイレクトリのサブデイレクトリtmpに作成する。

 なお、素性は、複合語素性、前接素性、後接素性に分けられた後、ucs2bで
 ソートされ、品詞に連結される。
 また品詞リストと形態素辞書ソースはそれぞれ品詞および表記でucs2bのコード
 としてソートされる。

 "-j"が指定された場合は基本品詞により算出した品詞頻度を出力する。

=head1 HISTORY

 Ver.1.7  2000/10/18 ユニークなIDを付与
 Ver.1.38 2000/12/05 fixhin,hindomapを読めるようにして、再整理。
 Ver.1.43 2000/12/12 表記に縦棒がないと語構成情報をみないようにした。
 Ver.1.48 2001/08/16 NDEBUGではなくprefix_DEBUGを指定するようにした。
 Ver.5.00 2002/04/30 UNA-CmnToolsとしてRCS管理に
 Ver.5.01 2002/05/20 tmpディレクトリを指定出来るようにした。
 Ver.5.02 2002/07/04 半角記号追加登録機能を追加
 Ver.5.03 2002/07/25 レビューによる指摘事項を反映、コメントを追加
 Ver.5.04 2002/07/31 mkmodicレビューにより見つかったバグを修正
 Ver.5.05 2002/08/08 半角記号追加登録機能を全角記号を含む文字列エントリに
                     対しても処理を行なうようにした。
 Ver.5.06 2002/08/20 ツールの実行環境を/proj/nlp/tools/UNA-CmnTools/v1.2.dist
                     に変更
 Ver.5.07 2005/04/14 品詞頻度固定化機能を追加
 Ver.5.08 2013/05/13 Jconv.pmの仕様変更に追従
 Ver.5.09 2014/10/10 r1.2の修正をr1.3に反映。
 Ver.5.10 2014/12/05 unihinmap.src, termtype.lst, termtypemap.lstを読んで
                     JpTransTable[], convTable[], termType.utf8の内容を
		     出力するようにした

=head1 BUG

 素性に"前品詞＊後品詞"があっても、あくまで'*'行に書かれた品詞によって
 各活用形が展開される。

=head1 METHODS

=cut

$DEBUG = 0;

use lib "../tools/perl/lib";
use utf8;
use open IN  => ":utf8";
use open OUT => ":utf8";
binmode(STDOUT, ":utf8");
binmode(STDERR, ":utf8");

use UNA::Cmn;
use UNA::Cmn::Dic;
use UNA::Cmn::HinGrp;
use UNA::Cmn::GobiTbl;
use UNA::Cmn::FeaLst;
use UNA::HinMap;
use UNA::FixHin;
use UNA::HindoMap;
use UNA::HanTbl;
use UNA::HinhindoTbl;
use UNA::DichindoTbl;
use UNA::HinLst;
use UNA::UniHinMap;
use UNA::TermType;
use UNA::TermTypeMap;

=head2 new

 $mkdic = new UNA::MkMoDic;

 UNA::MkMoDicのオブジェクトを返す。

=cut

sub new {
	my($class) = @_;
	my $dic = new UNA::Cmn::Dic;
	my $hingrp = new UNA::Cmn::HinGrp;
	my $gobitbl = new UNA::Cmn::GobiTbl;
	my $fealst = new UNA::Cmn::FeaLst;
	my $hinmap = new UNA::HinMap;
	my $fixhin = new UNA::FixHin;
	my $hindomap = new UNA::HindoMap;
	my $hantbl = new UNA::HanTbl;
	my $hinhindotbl = new UNA::HinhindoTbl;
	my $dichindotbl = new UNA::DichindoTbl;
	my $hinlst = new UNA::HinLst;
	my $unihinmap = new UNA::UniHinMap;
	my $termtype = new UNA::TermType;
	my $termtypemap = new UNA::TermTypeMap;
	my $self =
	{
	 dic => $dic,					# コモン辞書
	 hingrp => $hingrp,				# 品詞グループ
	 gobitbl => $gobitbl,			# 語尾テーブル
	 fealst => $fealst,				# 素性リスト
	 hinmap => $hinmap,				# 品詞マップ
	 fixhin => $fixhin,				# 固定品詞リスト
	 hindomap => $hindomap,			# 頻度マップ
	 hantbl => $hantbl,				# 半角エントリ登録テーブル
	 hinhindotbl => $hinhindotbl,	# 基本品詞頻度テーブル
	 dichindotbl => $dichindotbl,	# 辞書品詞頻度テーブル
	 hinlst => $hinlst,				# 品詞リスト
	 unihinmap => $unihinmap,		# 統合品詞マップ
	 termtype => $termtype,			# 検索語タイプ
	 termtypemap => $termtypemap,	# 検索語タイプマップ
	 info_pat => '',				# 出力パターン
	 hindo => {},					# 品詞毎の頻度(単語頻度の推定値の和)
	 basehindo => {},				# 基本品詞毎の頻度(単語頻度の推定値の和)
	 real_hindo => {},				# 品詞毎の頻度(単語頻度の測定値の和)
	 real_basehindo => {},			# 品詞毎の頻度(単語頻度の測定値の和)
	 wnum => {},					# その品詞に属する単語数
	 hin_tbl => {},					# 全ての素性つき品詞
	 basehin_tbl => {},				# 全ての基本品詞
	 maefeahin_tbl => {},			# 全ての前接素性付き品詞
	 hin_code => {},				# 品詞コードを登録
	 hin_sstr => {},				# 品詞用ソート文字列
	 fea_sstr => {},				# 素性用ソート文字列
	 max_cost => -1,				# 出力したコストの最大値
	 min_cost => 99999,				# 出力したコストの最小値
	 uniqid => 0,					# 現在のユニーク番号
	 cino => 0,						# 異表記番号
	 wid2uniqid => {},				# 単語IDと活用番号からユニーク番号への変換表
	 wid2hyouki => {},				# 単語IDと活用番号から表記への変換表
	 hyouki2wid => {},				# 表記から単語IDへの変換表
	 hinhindo => {},				# 単語コスト算出過程の頻度
	 max_katn => 0,					# ひとつの品詞における活用の個数の最大値
	 tmpfile1 => "./tmp/\#MkMoDic.$$.1",
	 tmpfile2 => "./tmp/\#MkMoDic.$$.2",
	 sort_command => "sort",
	};
	bless $self, $class;
	$self;
}

# デストラクタ
DESTROY {
	my($self) = @_;
	unlink $self->{tmpfile1}, $self->{tmpfile2} if !$DEBUG; # 一応消す。
}

=head2 load

 $mkdic->load('dic', $dic_file, $mpat);
 $mkdic->load('gobitbl', $gobitbl_file);
 $mkdic->load('hingrp', $hingrp_file);
 $mkdic->load('fealst', $fealst_file);
 $mkdic->load('hinmap', $hinmap_file);
 $mkdic->load('fixhin', $fixhin_file);
 $mkdic->load('hindomap', $hindomap_file);
 $mkdic->load('hantbl', $hantbl_file);
 $mkdic->load('hinhindotbl', $hinhindotbl_file);
 $mkdic->load('dichindotbl', $dichindotbl_file);
 $mkdic->load('hinlst', $hinlst_file);

 データファイルを読みこむ。

=cut

sub load {
	my($self, $type, $file, @args) = @_;
	if ($type !~ /^(dic|gobitbl|hingrp|fealst|hinmap|fixhin|hindomap|hantbl|hinhindotbl|dichindotbl|hinlst|unihinmap|termtype|termtypemap)$/){
		die "unknown file type: $type\n";
	}
	$self->{$type}->load($file, @args);
}

=head2 make_morphdic

 $mkdic->make_morphdic($morphdic_file, $info_pat, \&arranger, $opt_s, $opt_F, $opt_N, $opt_j, $opt_B);

 形態素辞書を作る。
 形態素辞書の一行がどのような形式になるかを $info_pat で指定する。
 arrangerを指定することで、加工もできる。
 $info_patでは、以下の変数を使用できる。

	<wid>       ... 単語ID
	<ino>       ... 異表記番号
	<uniqid>    ... ユニークなID
	<uniqids>   ... ユニークなID(複合語の場合、語構成を含む)
	<cost>      ... コスト
	<hyouki>    ... 表記(セパレータ付き)
	<hyouki0>   ... 代表表記(セパレータ付き)
	<shuu>      ... 終了形(セパレータ付き)
	<hin>       ... 品詞名(素性付き)
	<hin_code>  ... アプリ品詞コード(UNA品詞番号)
	<info:M>    ... common辞書のマークM行の補足事項
	<info0:M>   ... common辞書のマークM行の補足事項(異表記番号は00)

 また、これら変数の後に":MODIFIER"を書くことができる。これにより、
 変数の値を変えることができる。":MODIFIER"には以下のものがある。

	:unite      ... セパレータを取る
	:gokan      ... 語幹
	:gobi       ... 語尾
	:mae        ... 複合品詞における前品詞
	:ato        ... 複合品詞における後品詞
	:base       ... 基本品詞(素性を取った品詞)
	:pure       ... 純品詞(","以降を除いた基本品詞)
	:main       ... 品詞大分類
	:fea        ... 素性
	:NUMBER     ... NUMBER(>=0)番目の要素
	:-NUMBER    ... 後ろからNUMBER(N>=1)番目の要素を返す

 例えば、<hyouki0:gokan:unite> は代表表記の語幹(セパレータなし)を意味する。
 なお、<hin_code>を使う場合には、事前にmake_hinlstメソッドが実行されている
 必要がある。
 $opt_sがある場合、$hantbl_fileで指定される半角エントリを形態素辞書に追加
 登録する。
 $opt_Fがある場合、$hinhindotbl_fileで指定される品詞頻度を使用し、コストを算出する。
 $opt_Nがある場合、活用語尾の数から特殊な活用(口語や文語など)の数を除いてコストを算出する。
 $opt_jがある場合、基本品詞による品詞頻度を用いてコストを算出する。
 $opt_Bがある場合、$dichindotbl_fileで指定される頻度を使用し、コストを算出する。

=cut

sub make_morphdic {
	my($self, $outfile, $info_pat, $arranger, $opt_s, $opt_F, $opt_N, $opt_j, $opt_B) = @_;
	$self->{info_pat} = $info_pat;

	$self->{hingrp}->set_from_gobitbl($self->{gobitbl});
	$self->{fealst}->set_from_hingrp($self->{hingrp});
	$self->{hindo} = {};		# 品詞毎の頻度(単語頻度の推定値の和)
	$self->{basehindo} = {};	# 基本品詞毎の頻度(単語頻度の推定値の和)
	$self->{real_hindo} = {};	# 品詞毎の頻度(単語頻度の測定値の和)
	$self->{real_basehindo} = {};	# 品詞毎の頻度(単語頻度の測定値の和)
	$self->{wnum} = {};		# その品詞に属する単語数
	# 2014.01.28
	# アプリケーション情報パターンに<hin_code>が含まれる場合make_morphdic()は
	# 2回呼び出されるため、ここでuniqidとcinoをリセットしておく
	$self->{uniqid} = 0;	# 現在のユニーク番号
	$self->{cino} = 0;		# 異表記番号
	# make_hinlst前のmake_morphdicで格納されている値をクリアする。
	$self->{wid2uniqid} = {};
	$self->{wid2hyouki} = {};
	$self->{hyouki2wid} = {};
	my $tmpfile1 = $self->{tmpfile1};
	my $tmpfile2 = $self->{tmpfile2};
	open(TMP, ">$tmpfile1") || die "can't write $tmpfile1: $!\n";
	select TMP;
	for $wid (@{$self->{dic}{wids}}) {
		# 異表記番号を得る
		my $ihyoukin = $self->{dic}{ihyoukin}{$wid}; # 最大値を得る
		my @ino = ();
		for $i (0 .. $ihyoukin) {
			my $ino = sprintf("%02d", $i);
			if ($self->get_info('h', $wid, $ino)) { # 欠番でなければ
				push(@ino, $ino);
			}
		}
		# 表記毎に実行
		for $ino (@ino) {
			$self->make_1hyouki($wid, $ino, $opt_s, "", $opt_N, $opt_j);
		}
	}
	close TMP;
	select STDOUT;
	# ソートする
	system "$self->{sort_command} <$tmpfile1 >$tmpfile2";
	if ($?) {
		die "MkMoDic: $self->{sort_command}: exit($?)\n";
	}
	unlink $tmpfile1;

	# 登録した単語を出力
	if ($outfile) {
		open(OUT, ">$outfile") || die "can't write $outfile: $!\n";
		select OUT;
	}
	open(TMP, $tmpfile2) || die "can't read $tmpfile2: $!\n";
	while (<TMP>) {
		chomp;
		my @a = split / /;
		shift @a; # ソート用文字列
		shift @a; # unique番号
		my $line = $self->make_line(@a, $opt_F, $opt_j, $opt_B);
		$line = &$arranger($line) if $arranger;
		$line .= "\n";
		# 2022.11.16
		#$line = $self->{jconv}->conv($self->{output_code}, $line);
		print $line;
	}
	close TMP;
	unlink $tmpfile2;
	if ($outfile) {
		close OUT;
		select STDOUT;
	}
}

# 活用する前の一表記分の処理
sub make_1hyouki {
	my($self, $wid, $ino, $opt_s, $opt_F, $opt_N, $opt_j) = @_;
	# 語の情報を得る
	my $hyouki = $self->get_info('h', $wid, $ino);
	my $hin = $self->{dic}{hin}{$wid};
	my($maehin, $atohin) = &fuku2maeato($hin);
	if ($maehin) {
		# 代表表記に＊が認められないのであれば#を外してもよいが、現在は規定していない
		#	warn "old hinshi format of fukugougo: $hin: *? $wid$ino\n";
	}
	my @features = $self->get_features($wid, $ino);
	my $fukufea = &get_fukufea(@features);
	if ($fukufea) {
		($maehin, $atohin) = &fuku2maeato($fukufea);
		if (!$maehin) {
			$maehin = (&hin2basefea($hin))[0]; # 基本品詞で代用
		}
		if (!$atohin) {
			$atohin = (&hin2basefea($hin))[0]; # 基本品詞で代用
		}
	}
	# 活用するかどうかで分けて処理
	if (&is_katsuyou($hyouki)) {
		my($gokan, $gobi) = &hyouki2gokangobi($hyouki);
		if ($maehin =~ /^動詞\./ && $atohin =~ /^動詞/) {
			$maehin = $atohin;	# 例:「歩き｜疲れ＾る」
		}
		if (!$self->{gobitbl}{brothers}{$atohin}{$gobi}) {
			die "ERROR: katsuyou not found: $wid: $atohin: $gobi in $hyouki\n";
		}
		# 活用形毎に処理
		my $shuu;
		my @hin_and_gobi = @{$self->{gobitbl}{brothers}{$atohin}{$gobi}};

		# 素性の妥当性チェック
		for $feature (@features) {
			$done = 0;
			for $hin_and_gobi (@hin_and_gobi) {
				if ($self->check_feature(${$hin_and_gobi}[0], $feature)) {
					$done = 1;
					last;
				}
			}
			if (!$done) {
				print STDERR "WARNING: $wid $hyouki $hin isn't the target of $feature\n";
			}
		}
		my $katn = scalar(@hin_and_gobi);

		# 品詞名が口語、文語、動詞.終止連体,ザ変一字.ジンは活用数から除く
		if($opt_N) {
			for $hin_and_gobi (@hin_and_gobi) {
				if(${$hin_and_gobi}[0] =~/口語|文語|動詞\.終止連体,ザ変一字\.ジン/){
					$katn--;
				}
			}
		}

		# 活用の最大値を覚えておく
		$self->{max_katn} = $katn if $katn > $self->{max_katn};
		my $kati = 1; # 活用の番号
		for $hin_and_gobi (@hin_and_gobi) { # 品詞と語尾の展開
			my($newhin, $newgobi) = @{$hin_and_gobi};
			my @feas = $self->valid_features($newhin, @features);
			if ($fukufea) {
				my $thin = $newhin; # 一時的に保存
				$newhin = $hin;
				$newhin .= "\.".&get_katsuyou($thin); # みかけ品詞も活用
				$newhin .= ";$maehin＊$thin";
				@feas = grep {$_ ne $fukufea} @feas;
			}
			else {
				$newhin = "$maehin＊$newhin" if $maehin;
			}
			$newhin = &add_features($newhin, @feas);
			my $newhyouki = $newgobi ? "$gokan＾$newgobi" : $gokan;
			$shuu ||= $newhyouki;
			# 一旦登録
			$self->regist_word($wid, $ino, $newhyouki, $shuu, $newhin, $katn, $kati, $opt_s, $opt_F, $opt_j);
			$kati++;
		}
	}
	else {
		# 素性の妥当性チェック
		for $feature (@features) {
			if (!$self->check_feature($atohin, $feature)) {
				print STDERR "WARNING: $wid $hyouki $hin isn't the target of $feature\n";
			}
		}

		# 非活用語の登録
		my @feas = $self->valid_features($atohin, @features);
		$hin = &add_features($hin, @feas);
		# 一旦登録
		$self->regist_word($wid, $ino, $hyouki, $hyouki, $hin, 1, 0, $opt_s, $opt_F, $opt_j);
	}
}

# 素性を得る
sub get_features {
	my($self, $wid, $ino) = @_;
	my @features = ();
	my $features = $self->get_info('m', $wid, $ino);
	if ($features) {
		# 重複をチェック
		my @a = split /\s+/, $features;
		my %mark;
		my $fuku_n = 0;
		for (@a) {
			if ($mark{$_}) {
				die "ERROR: features re-specified: $wid$ino: $_\n";
			}
			if (&is_fukufea($_)) {
				$fuku_n++;
			}
		}
		if ($fuku_n > 1) {	# 複合語素性はひとつまで
			die "ERROR: two more fukukougo feature: $wid$ino: $features\n";
		}
		# ソートする
		@features = sort {$self->fea2sstr($a) cmp $self->fea2sstr($b)} @a;
	}
	@features;
}

# その品詞につく素性だけを返す
sub valid_features {
	my($self, $hin, @features) = @_;
	my @a = ();
	for $feature (@features) {
		if (&is_fukufea($feature)) { # 複合語素性ならO.K.
			push(@a, $feature);
		}
		elsif ($self->{fealst}{conn_flag}{$feature}{$hin}) {
			push(@a, $feature);
		}
	}
	@a;
}

# 素性がその品詞につくかどうかチェックする
# つくなら1、つかないなら0が返る
sub check_feature {
	my($self, $hin, $feature) = @_;
	if (&is_fukufea($feature)) { # 複合語素性ならO.K.
		return 1;
	}
	if ($self->{fealst}{conn_flag}{$feature}{$hin}) {
		return 1;
	}
	return 0;
}

# 複合語素性を得る
sub get_fukufea {
	my(@features) = @_;
	my $fukufea = '';
	for (@features) {
		if (&is_fukufea($_)) {
			$fukufea = $_;
			last;
		}
	}
	$fukufea;
}

# 複合語素性を置き換える
sub set_fukufea {
	my($features, $fukufea) = @_;
	for $i (0 .. $#$features) {
		if (&is_fukufea($features->[$i])) {
			$features->[$i] = $fukufea;
		}
	}
}

# 活用形を得る
sub get_katsuyou {
	my($hin) = @_;
	my $true_hin = (split(/,/, $hin))[0]; # 真の品詞
	my @path = split(/\./, $true_hin);    # 品詞の階層
	$path[-1];			          # 最後の要素を返す
}

# 単語を登録する
sub regist_word {
	my($self, $wid, $ino, $hyouki, $shuu, $hin, $katn, $kati, $opt_s, $opt_F, $opt_j) = @_;
	# 頻度計算
	my $hindo = $self->get_info('t', $wid, $ino) || 0;
	my $real_hindo = 0;
	if ($hindo) {
		my @hindos;
		for $h (split /\s+/, $hindo) { # いくつあってもOK
			if ($h =~ /^\-+$/) {
				$h = 0;
			}
			elsif ($h < 0) {
				die "ERROR: bad hindo: t? $wid$ino: $h\n";
			}
			push(@hindos, $h);
			$real_hindo += $h;
		}
		$hindo = $self->{hindomap}->estimate(@hindos); # 推定値に変更
	}
	$hindo = 1 if $hindo == 0;
	$hindo /= $katn;		## 活用形の頻度をいいかげんに得る
	$real_hindo /= $katn;

	my $maefeahin = &hin2maefeahin($hin);
	# 基本品詞の頻度を登録
	# 以下の値は実行辞書作成と基本辞書品詞頻度テーブルの出力で使用する
	if($opt_j || $opt_F){
		my $mae_basehin = (&hin2basefea($maefeahin))[0];
		$self->{basehindo}{$mae_basehin} += $hindo;
		$self->{real_basehindo}{$mae_basehin} += $real_hindo;
	}
	# 前接素性つき品詞の頻度を登録
	$self->{hindo}{$maefeahin} += $hindo;
	$self->{real_hindo}{$maefeahin} += $real_hindo;
	$self->{wnum}{$maefeahin}++;

	# 品詞を登録
	$self->{maefeahin_tbl}{$maefeahin}++;

	if(!$opt_F){
		$self->{hin_tbl}{$hin}++;
		my $mae_basehin = (&hin2basefea($maefeahin))[0];
		my $atofeahin = &hin2atofeahin($hin);
		my $ato_basehin = (&hin2basefea($atofeahin))[0];
		$self->{basehin_tbl}{$mae_basehin}++;
		$self->{basehin_tbl}{$ato_basehin}++;
		my $uhyouki = &hyouki_unite($hyouki);
		$self->{wid2hyouki}{"$wid$ino-$kati"} = $uhyouki;
		$self->{hyouki2wid}{"$uhyouki"} = "$wid$ino";

		# ソート用文字列をつけて単語を一旦出力
		my $sstr = $self->hyouki2sstr($hyouki);
		# 半角記号の追加登録
		if($opt_s){
			$self->add_word($sstr, $hyouki, $wid, $ino, $shuu, $hin, $maefeahin, $hindo, $kati, $opt_s);  
		}
		else{
			# ユニーク番号の付与
			my $uniqid = ++$self->{uniqid};
			$self->{wid2uniqid}{"$wid$ino-$kati"} = $uniqid;
			my $idstr = sprintf("%09d", $uniqid); # 順序性を残すため
			print "$sstr $idstr $hyouki $wid $ino $shuu $hin $maefeahin $hindo $kati\n";
		}
	}
}

# 単語を出力する
sub make_line {
	my($self, $hyouki, $wid, $ino, $shuu, $hin, $maefeahin, $hindo, $kati, $opt_F, $opt_j, $opt_B) = @_;

	my $hinhindo;
	my $mae_basehin = (&hin2basefea($maefeahin))[0] if($opt_j);
 	my $cost = $hindo;
	my $hin_code = $self->{hin_code}{$hin} || 0;
	$hin_code = sprintf("%04x", $hin_code);
	my $uniqid = sprintf("%09d", $self->{wid2uniqid}{"$wid$ino-$kati"});
	my $uniqids = $self->make_uniqids($wid, $ino, $hyouki, $hin, $kati);
	my $line = $self->{info_pat};
	$line =~ s/<wid>/$wid/g;
	$line =~ s/<uniqid>/$uniqid/g;
	$line =~ s/<uniqids>/$uniqids/g;
	$line =~ s/<ino>/$ino/g;
	$line =~ s/<cost>/$cost/ge;
	$line =~ s/<hyouki(:.+?|)>/&eval_modifiers($hyouki, 'hyouki', $1)/ge;
	$line =~ s/<hyouki0(:.+?|)>/&eval_modifiers($self->{dic}{hyouki0}{$wid},
	'hyouki', $1)/ge;
	$line =~ s/<shuu(:.+?|)>/&eval_modifiers($shuu, 'hyouki', $1)/ge;
	$line =~ s/<hin(:.+?|)>/&eval_modifiers($hin, 'hin', $1)/ge;
	$line =~ s/<hin_code>/$hin_code/g;
	$line =~ s/<info:(.)(:.+?|)>/&eval_modifiers($self->get_info($1, $wid, $ino), 'info', $2)/ge;
	$line =~ s/<info0:(.)(:.+?|)>/&eval_modifiers(
	$self->{dic}->get_info($1, $wid, '00'), 'info', $2)/ge;
	$line;
}

# 語構成を含むユニークな番号を作成する
sub make_uniqids {
	my($self, $wid, $ino, $hyouki, $hin, $kati) = @_;
	my @r = ();
	my $gokousei = $self->get_info('g', $wid, $ino);
	my @hyouki = &hyouki_split($hyouki);
	if ($gokousei && scalar(@hyouki) > 1) { # 語構成があるなら
		my @go = split /\s+/, $gokousei;
		shift @go; # 見かけの品詞は無視
		if ($#hyouki != $#go) {	# 語構成の数が一致するかどうかチェック
			my($hn, $gn) = ($#hyouki + 1, $#go + 1);
			die "ERROR: bad gokousei count: $wid: hyouki=$hn <=> gokousei=$gn\n";
		}

		# 半角表記の複合語の場合
		if($hyouki =~/\\u/) {
			my $cnv; 
			my @hgo = ();
			for $i(0 .. $#hyouki) {
				if($hyouki[$i] =~/\\u/) {
					if($self->{hyouki2wid}{"$hyouki[$i]"}) {
						push(@hgo,$self->{hyouki2wid}{"$hyouki[$i]"});
					} else {
						die "ERROR: no registered entry: $hyouki[$i]\n";
					}
				} else {
					push(@hgo,$go[$i]);
				}
			}
			@go = @hgo;
		}
		for $i (0 .. $#go) {
			my $cid = $go[$i];
			if ($cid !~ /^(\d\d\d\d\d\d\d\d)(\d\d)$/) {
				die "ERROR: bad gokousei id: $cid: g? $wid$ino $gokousei\n";
			}
			my($cwid, $cino) = ($1, $2);
			my $chyouki = $self->get_info('h', $cwid, $cino);
			my $uniqid;
			if (&is_katsuyou($chyouki)) { # 子が活用語なら
				if ($self->{wid2uniqid}{"$cid-0"}) { # 本当に活用語かチェック
					die "ERROR: not katsuyou: $cid: g? $wid$ino $gokousei\n";
				}
				if ($i == $#go && $kati > 0) { # 親が活用しているのなら
					# 品詞が同じかどうかチェックする
					my $phin = $self->get_atohin($wid, $ino);
					my $chin = $self->get_atohin($cwid, $cino);
					if ($phin ne $chin) {
						die "ERROR: mismatch hinshi $chin($cid) <=> $phin($wid): g? $wid$ino $gokousei\n";
					}
					# 表記が同じかチェックする
					my $phyouki = &hyouki_unite($hyouki[$i]);
					my $thyouki = $self->{wid2hyouki}{"$cid-$kati"};
					if (!$thyouki) {
						die "ERROR: no such hyouki: $phyouki: $cid: g? $wid$ino $gokousei\n";
					}
					if ($thyouki ne $phyouki) {
						warn "mismatch hyouki: $thyouki($cid) <=> $phyouki($wid$ino) : g? $wid$ino-$kati $gokousei\n";
						@r = ();
						last;
					}
					# 活用番号が同じ物を使う
					$uniqid = $self->{wid2uniqid}{"$cid-$kati"};
				}
				else { # 親が活用していないのなら
					# 表記が同じ物を捜す
					my @a;
					for $j (1 .. $self->{max_katn}) {
						my $thyouki = $self->{wid2hyouki}{"$cid-$j"};
						if ($thyouki) {
							if ($thyouki eq &hyouki_unite($hyouki[$i])) {
								push(@a, $j);
						    }
						}
					}
					if (!@a) {
						warn "no such child's hyouki: $chyouki: g? $wid$ino $gokousei\n";
						@r = ();
						last;
					}
					if (scalar(@a) >= 2) { # 2つ以上ある
						die "ERROR: two more child candidates: $chyouki($hyouki[$i]): g? $wid$ino $gokousei\n";
					}
					# 子のユニーク番号を付ける
					$uniqid = $self->{wid2uniqid}{"$cid-$a[0]"};
				}
			}
			else {
				# 本当に活用語ではないかチェックする
				if ($self->{wid2uniqid}{"$cid-1"}) {
					die "ERROR: is katsuyou: $cid: g? $wid$ino $gokousei\n";
				}
				# 表記が同じかチェックする
				my $phyouki = &hyouki_unite($hyouki[$i]);
				my $thyouki = $self->{wid2hyouki}{"$cid-0"};
				if (!$thyouki) {
					die "ERROR: no such hyouki: $phyouki: $cid: g? $wid$ino $gokousei\n";
				}
				if ($thyouki ne $phyouki) {
					warn "mismatch hyouki: $thyouki($cid) <=> $phyouki($wid$ino) : g? $wid$ino-$kati $gokousei\n";
					@r = ();
					last;
				}
				# 子のユニーク番号を付ける
				$uniqid = $self->{wid2uniqid}{"$cid-0"};
			}
			if (!$uniqid) {
				die "ERROR: unknown gokousei id: $chyouki($cid): g? $wid$ino $gokousei\n";
			}
		    push(@r, sprintf("%09d", $uniqid));
		}
	}
	unshift(@r, sprintf("%09d", $self->{wid2uniqid}{"$wid$ino-$kati"}));
	join(':', @r); # コロンでつないで出力
}

# 単語IDと異表記番号から、活用前の、素性無し後品詞を得る。
sub get_atohin {
	my($self, $wid, $ino) = @_;
	my @features = $self->get_features($wid, $ino);
	my $fukufea = &get_fukufea(@features);
	my $atohin;
	if ($fukufea) {
		$atohin = (&fuku2maeato($fukufea))[1];
	}
	if (!$atohin) {
		my $hin = $self->{dic}{hin}{$wid};
		$atohin = (&hin2basefea($hin))[0]; # 基本品詞で代用
		$atohin = (&fuku2maeato($atohin))[1]; # old format
	}
	$atohin;
}

# modifierを評価
sub eval_modifiers {
	my($str, $type, $modis) = @_;
	$str ||= '';
	for $modi (split /:/, $modis) {
		next if !$modi;
		if ($modi =~ /^\-?[0-9]+$/) {
			if ($type eq 'hyouki') {
				$str = &hyouki_element($str, $modi);
			}
			elsif ($type eq 'fea') {
				$str = &features_element($str, $modi);
			}
			else {
				$str = &info_element($str, $modi);
			}
		}
		elsif ($modi eq 'unite') {
			if ($type eq 'hyouki') {
				$str = &hyouki_unite($str);
			}
			elsif ($type eq 'fea') {
				$str = &features_unite($str);
			}
			else {
				$str = &info_unite($str);
			}
		}
		elsif ($modi eq 'gokan') {
			$str = (&hyouki2gokangobi($str))[0];
			$type = 'hyouki';
		}
		elsif ($modi eq 'gobi') {
			$str = (&hyouki2gokangobi($str))[1];
			$type = 'hyouki';
		}
		elsif ($modi eq 'mae') {
			$str = (&fuku2maeato($str))[0];
			$type = 'hin';
		}
		elsif ($modi eq 'ato') {
			$str = (&fuku2maeato($str))[1];
			$type = 'hin';
		}
		elsif ($modi eq 'base') {
			$str = (&hin2basefea($str))[0];
			$type = 'hin';
		}
		elsif ($modi eq 'fea') {
			$str = (&hin2basefea($str))[1];
			$type = 'fea';
		}
		elsif ($modi eq 'pure') {
			$str = &hin2purehin($str);
		}
		elsif ($modi eq 'main') {
			$str = &hin2mainhin($str);
		}
		else {
			die "ERROR: unknown modifier: $str: $modi (in $modis)\n";
		}
    }
    $str;
}

#表記からソート用文字列を得る
sub hyouki2sstr {
	my ($self, $hyouki) = @_;
	# 表記中の＾と｜を取り除く
	$hyouki = &hyouki_unite($hyouki);

	# "\uxxxx"をUCS2文字に変換する
	$hyouki =~ s/\\u([0-9A-Za-z]{4})/chr(eval "0x".$1)/eg;

	# 各文字をUCS2BEとして16進表記する
	$hyouki =~ s/(.)/sprintf('%04x', ord($1))/eg;
	return($hyouki);
}

#品詞名からソート用文字列を得る
# 内部コードUnicode化により品詞そのものをソート用文字列とする
# 「〜」が漢字や「！」より先にくるようにする
#
sub hin2sstr {
	my($self, $hin) = @_;
	# 「～」のソート順を変更する
	my $hinx = $hin;
	$hinx =~ s/\x{ff5e}/\x{301c}/g;
	$self->{hin_sstr}{$hin} || ($self->{hin_sstr}{$hin} = $hinx);
}

#素性名からソート用文字列を作る
# 内部コードUnicode化により素性名そのものをソート用文字列とする
# 「〜」が漢字や「！」より先にくるようにする
#
sub fea2sstr {
	my($self, $fea) = @_;
	my $feax = $fea;
	$feax =~ s/\x{ff5e}/\x{301c}/g;
	my $type = '2'; # 前接素性
	if (&is_fukufea($fea)) {
		$type = '1'; # 複合語素性
	}
	elsif (&is_atofea($fea)) {
		$type = '3'; # 後接素性
	}
	$fea = $type.$fea;
	$self->{fea_sstr}{$fea} || ($self->{fea_sstr}{$fea} = $type.$feax);
}

=head2 make_hinlst

 $mkdic->make_hinlst($hinlst_file, $morph_h_file, $jptbl_file, $prefix, $opt_F, $opt_j, $opt_B, $dic_file);

 品詞リストを作る。
 $morph_h_fileが指定されている場合は、unamorph.hで使われるUNA_HIN_*
 マクロ定数の定義リスト(UNA_HIN_RENGOを除く)をそのファイルに出力する。
 $prefixが指定されていれば、出力されるマクロ定数名の前に$prefixがつけられる。
 $jptbl_fileが指定されている場合は、UnaJpDicSet::load() (ModNlpResourceUnaJp.cpp)で
 読まれ、getConcept()で参照されるjpTrans.tblの内容をそのファイルに出力する。
 出力されるファイルのコードは、$ocodeになる。
 $opt_Fがある場合、$hinhindotbl_fileで指定される頻度を使用し、品詞頻度を算出する。
 $opt_jがある場合、基本品詞による頻度を用いて品詞頻度を算出する。
 $opt_Bがある場合、$dichindotbl_fileで指定される頻度を使用し、品詞頻度を算出する。

 事前にmake_morphdicメソッドが実行されている必要がある。

=cut

sub make_hinlst {
	my($self, $outfile, $morph_h_file, $jptbl_file, $prefix, $opt_F, $opt_j, $opt_B, $dic_file) = @_;
	$prefix ||= '';
	# 出力すべき品詞を調べ、@hinsに格納する

	my(@hins, %fixed, %hinlst, %used, %mohin_no);
	for $hin (@{$self->{fixhin}{hin}}) { # 固定品詞を出力
		push(@hins, $hin);
		$fixed{$hin}++;
	}
	for $hin (@{$self->{hinlst}{hins}}) { # 前に作成したUNA品詞リストの品詞を出力
		next if $fixed{$hin};
		push(@hins, $hin);
		$hinlst{$hin}++;
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{hin_tbl}}) { # 辞書で使用している品詞
		$used{$hin}++;
		next if $fixed{$hin} || $hinlst{$hin};
		push(@hins, $hin);
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{basehin_tbl}}) { # 基本品詞
		next if $fixed{$hin} || $hinlst{$hin} || $used{$hin};
		push(@hins, $hin);
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{maefeahin_tbl}}) { # 前接素性付き品詞
		next if $fixed{$hin} || $hinlst{$hin} || $used{$hin} || $self->{basehin_tbl}{$hin};
		push(@hins, $hin);
	}
	# dichindo.tblが存在する場合
	if(defined($opt_B)){
		for $hin (@{$self->{dichindotbl}{hins}}) { # 過去に作成した実行用データに出現する品詞
			next if $fixed{$hin} || $hinlst{$hin} || $used{$hin} || $self->{basehin_tbl}{$hin} || $self->{maefeahin_tbl};
			push(@hins, $hin);
		}
	}

	# 形態素品詞番号の割り当て
	my $mohin_no = 1;
	for $hin (@hins) {
		$mohin_no{$hin} = $mohin_no++;
	}
	# 品詞リストを出力
	my $macro_prefix = uc ($prefix);
	if ($outfile) {
		open(OUT, ">$outfile") || die "can't write $outfile: $!\n";
		select OUT;
	}
	# データ作成で指定された辞書ソース名の出力
	my $line;
	if(defined($self->{hinlst}{comment})){
		$line = $self->{hinlst}{comment};
	}
	print $line;

	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)} @hins) {
		my $maefeahin = &hin2maefeahin($hin);
		my $hin_code = $self->get_hin_code($hin);
		my $hin_no = 0;
		$hin_no = eval($hin_code) if $hin_code; # eval is here
		$self->{hin_code}{$hin} = $hin_no;
		$hin_no = sprintf("%04x", $hin_no);
		my $mohin_no = $mohin_no{$hin};
		my $hin_wnum;
		my $hin_hindo;
		my $hindo;
		my $mae_basehin = (&hin2basefea($maefeahin))[0] if($opt_j);

		# $hindoと$hin_hindoの取得
		# 辞書頻度テーブルが指定され、テーブルに品詞がある場合
		# 辞書頻度テーブルの頻度を使用する
		if(defined($opt_B) && grep {$_ eq $hin} @{$self->{hinlst}{hins}}){
			# 辞書頻度テーブルの品詞内単語数に実行用データ作成対象単語数を累積する
			$hin_wnum = $self->{hinlst}->{hin_wnum}{$hin} + $self->{wnum}{$maefeahin};
			$hindo = $self->get_dichindotbl_hindo($hin);
			$hin_hindo = $self->get_dichindotbl_hinhindo($hin);
		}

		# 基本辞書品詞頻度テーブルが指定され、テーブルに品詞がある場合
		# 基本辞書品詞頻度テーブルの頻度を使用する
		if($opt_F && $self->{hinhindotbl}{hinhindo}{hin}{$maefeahin}){
			# 辞書頻度テーブルの品詞内単語数に実行用データ作成対象単語数を累積する
			$hin_wnum = $self->{hinlst}->{hin_wnum}{$hin} + $self->{wnum}{$maefeahin};
			if($opt_j){
				$hindo = $self->get_basedic_hindo($maefeahin, $opt_j);
				$hin_hindo = $self->get_basedic_realhindo($maefeahin, $opt_j) || 0;
			} else {
				$hindo = $self->get_basedic_hindo($maefeahin);
				$hin_hindo = $self->get_basedic_realhindo($maefeahin) || 0;
			}
		}

		# 辞書頻度テーブル、基本辞書品詞頻度テーブルで設定されなかった時
		# UNA品詞リストの頻度が取得できる場合は辞書の頻度に加算する
		if(!defined($hin_wnum)){
			$hin_wnum = $self->{hinlst}->{hin_wnum}{$hin} + $self->{wnum}{$maefeahin} || 0;
			if($opt_j){
				$hindo = $self->{hinlst}->{hindo}{$hin} + $self->{basehindo}{$mae_basehin};
				$hin_hindo = $self->{hinlst}->{hin_hindo}{$hin} + $self->{real_basehindo}{$mae_basehin} || 0;
			} else {
				$hindo = $self->{hinlst}->{hindo}{$hin} + $self->{hindo}{$maefeahin};
				$hin_hindo = $self->{hinlst}->{hin_hindo}{$hin} + $self->{real_hindo}{$maefeahin} || 0;
			}
		}
		# 近似的に推定する(本当は各単語頻度測定値の和を推定値に変えてから、平均をとりたいところ)
		$hin_hindo = $self->{hindomap}->estimate($hin_hindo);
		$hin_hindo = sprintf("%f", $hin_hindo);

		# 単語コスト算出のための$hindoも出力する
		my $line = "$hin $mohin_no $hin_no $hin_wnum $hin_hindo $hindo \# ";
		$line .= "fixed:" if $fixed{$hin};
		$line .= "$hin_code:";
		# 複数辞書化によりunusedは出力しないことにした
		#$line .= "unused:" if !$used{$hin};
		$line .= "\n";
		print $line;
	}
	if ($outfile) {
		close OUT;
		select STDOUT;
	}

	if ($morph_h_file) {
		# unamorph.h用データを出力
		open(MORPHH, ">$morph_h_file") || die "can't write $morph_h_file: $!\n";
		my $line = "// UNA::MkMoDicにより自動生成されたファイル\n" .
		           "// unamorph.hの固定品詞名マクロ(UNA_HIN_*)の元データ\n" .
		           "\n" .
		           "#define ${macro_prefix}_NOTHING			0 /* 単語無し(どの品詞でもない) */\n";
		print MORPHH $line;
		for $hin (@hins) {
			my $mohin_no = $mohin_no{$hin};
			if ($fixed{$hin}) {
				$line = sprintf("#define %s_%-25s %4d /* %s */\n",
					$macro_prefix, $self->{fixhin}{label}[$mohin_no-1], $mohin_no, $hin);
				print MORPHH $line;
			}
		}
		close MORPHH;
	}
	if ($jptbl_file) {
		# jpTrans.tbl用情報を出力
		open(JPTBL, ">$jptbl_file") || die "can't write $jptbl_file: $!\n";
		my $line = "# jpTrans.tbl: UNA::MkMoDicにより自動生成されたファイル\n" .
		           "#\n" .
		           "#コード                 詳細品詞名\n" .
		           "0                       ダミー\n";
		#print JPTBL $self->{jconv}->conv('utf8', $line);
		print JPTBL $line;
		for $hin (@hins) {
			my $hin_code = $self->get_hin_code($hin);
			$hin_code = sprintf("%-24s", $hin_code);
			$line = $hin_code . $hin . "\n";
			print JPTBL $line;
		}
		close JPTBL;
	}
}

# その品詞の品詞コードを得る
sub get_hin_code {
    my($self, $hin) = @_;
	my $basehin = (&hin2basefea($hin))[0]; # 本当は意味素性は残したい
	my $hin_code = '';
	my $prev_level = -1;
	my %past; # そのレベルで見つかったことがあると真
	my %prev; # 直前でマッチしてたら真
	if ($self->{hinmap}) {
		for $hinmap_def (@{$self->{hinmap}{defs}}) {
			my($rexpr, $str, $level) = @{$hinmap_def};
			my $found;
			$past{$level} = 0 if $level > $prev_level; # レベルがあがったら
			if (!$past{$level}) { # まだ見つかっていない
				if ($level == 0 || $prev{$level - 1} == 1) { # 親が直前で真
					if ($basehin =~ /$rexpr/) {	# 正規表現にマッチ
						$found++;
					}
				}
			}
			if ($found) {
				$past{$level} = $prev{$level} = 1;
				if ($str =~ /^[\&\|]/) {
					$hin_code .= $str;
				}
				elsif ($str ne '') {
					$hin_code = $str;
				}
			}
			else {
				$prev{$level} = 0;
			}
			$past{$level} = 0 if $rexpr eq '.';	# 再度マッチさせるため
			$prev_level = $level;
		}
	}
	$hin_code;
}

# 辞書の情報を得る
sub get_info {
	my($self, $mark, $wid, $ino) = @_;
	my $info;
	if ($mark eq 'h') {
		if ($ino eq '00') {	# 異表記00番の代わりに代表表記を返す
			$info = $self->{dic}{hyouki0}{$wid};
		}
		else {
			$info = $self->{dic}->get_info($mark, $wid, $ino);
		}
	}
	else {
		$info = $self->{dic}->get_info($mark, $wid, $ino);
		if ($ino ne '00' && !defined $info) {
			$info = $self->{dic}->get_info($mark, $wid, '00');
		}
	}
	$info;
}

# 基本辞書品詞頻度テーブルから前接素性付き品詞の頻度を得る
sub get_basedic_hindo{
	my($self, $maefeahin, $opt_j) = @_;
	if($opt_j){
		$self->{hinhindotbl}{hinhindo}{basehindo}{$maefeahin} || 0;
	} else {
		$self->{hinhindotbl}{hinhindo}{hindo}{$maefeahin} || 0;
	}
}

# 基本辞書品詞頻度テーブルから前接素性付き品詞の頻度を得る
sub get_basedic_realhindo{
	my($self, $maefeahin, $opt_j) = @_;
	if($opt_j){
		$self->{hinhindotbl}{hinhindo}{real_basehindo}{$maefeahin};
	} else {
		$self->{hinhindotbl}{hinhindo}{real_hindo}{$maefeahin};
	}
}

# 辞書頻度テーブルから品詞頻度を得る
sub get_dichindotbl_hindo{
	my($self, $hin) = @_;
	$self->{dichindotbl}{hindo}{$hin};
}

# 辞書頻度テーブルから品詞頻度を得る
sub get_dichindotbl_hinhindo{
	my($self, $hin) = @_;
	$self->{dichindotbl}{hin_hindo}{$hin};
}

# 半角記号の追加登録
sub add_word{
	my($self, $sstr, $hyouki, $wid, $ino, $shuu, $hin, $maefeahin, $hindo, $kati, $opt_s) = @_;

    # ユニーク番号の付与
	my $uniqid;
	my $cino;			  # 異表記番号
	if($ino eq '00'){	# 異表記の最大値でリセット
		$self->{cino} = $self->{dic}{ihyoukin}{$wid};
	}

	my $sstrs = '';       # 変換後のソート用文字列群 
	my $hyoukis = '';     # 変換後の表記群  

	my $p;				  # '｜'の位置
	if($hyouki=~/｜/){
		$p = index($hyouki, '｜');
	}

	# 一表記毎に文字コード列を分割
	my @sstr = split /([0-9a-fA-F]{4})/,$sstr;
	my $count = 0;
	for (@sstr){
		my $sepstr = $_;
		next if $sepstr eq '';
		$count++;

		# 元表記
		my $p_hyouki = chr(eval("0x".$sepstr));
		# ソート用のコードポイントから元のコードポイントに戻す
		# ～：301C → FF5E
		# ∥：2016 → 2225
		# -：2212 → FF0D
		my $tr_hyouki = $p_hyouki;
		if($sstrs eq ''){
			$sstrs .= $sepstr;
			$hyoukis .= $tr_hyouki;
		} elsif($sepstr ne '') {
			$sstrs .= ",$sepstr";
			$hyoukis .= ",$tr_hyouki";
		}

		my $hancode = $self->{hantbl}{code}{$sepstr};
		if($hancode){ # han.tblに変換ルールあり
			$sstrs .= ":$hancode";
			if($p == $count){
				$hyoukis .= "｜:\\u$hancode｜";
			} else {
				$hyoukis .= ":\\u$hancode";
			}
		}

		# '｜'がある場合
		if($p == $count && !$hancode){
			$hyoukis .= "｜";
		}
	}
	$sstrs =~ tr/A-Z/a-z/;

	# 組合せを展開し文字列パターンを生成する
	# sstrs		30a4,30c8,2212:002d,30e8,2212:002d,30ab,30c9,2212:002d
	# hyoukis	イ,ト,−:\u002d,ヨ,−:\u002d,カ,ド,−:\u002d
	my @esstr = ();
	my @ehyouki = ();

	if($sstrs eq ''){
		die "ERROR: sstrs is null\n";
	}

	if($sstrs=~/:/){ # 半角表記あり
		@esstr = $self->exp_hankaku_array($sstrs);
		@ehyouki = $self->exp_hankaku_array($hyoukis);

		# tmpfile1に出力
		for($i = 0; $i <= $#esstr; $i++) {
			$uniqid = ++$self->{uniqid};
			$uniqid = sprintf("%09d", $uniqid);

			# 半角表記の異表記番号の採番
			if($i == 0){
				$self->{wid2uniqid}{"$wid$ino-$kati"} = $uniqid;
				$cino = $ino;
			} else {
				$cino = ++$self->{cino};
				$cino = sprintf("%02d", $cino);
				$self->{wid2uniqid}{"$wid$cino-$kati"} = $uniqid;

				# 生成された半角表記の語を単語IDから表記への変換表へ登録
				# common.dicの語はregist_word()で登録済み
				# 二重定義のチェック
				if (defined $self->{wid2hyouki}{"$wid$cino-$kati"}) {
					my $wid2hyouki = $self->{wid2hyouki}{"$wid$cino-$kati"};
					die "ERROR: redifinition: $wid$cino-$kati $wid2hyouki <=> $ehyouki[$i]\n";
				} else {
					# 登録
					my $uhyouki = &hyouki_unite($ehyouki[$i]);
					$self->{wid2hyouki}{"$wid$cino-$kati"} = $uhyouki;
					$self->{hyouki2wid}{"$uhyouki"} = "$wid$cino";
				}
			}
			print "$esstr[$i] $uniqid $ehyouki[$i] $wid $cino $shuu $hin $maefeahin $hindo $kati\n";
		}
	} else {
		# tmpfile1に出力
		$uniqid = ++$self->{uniqid};
		$uniqid = sprintf("%09d", $uniqid);
		$ino = sprintf("%02d", $ino);
		$self->{wid2uniqid}{"$wid$ino-$kati"} = $uniqid;
		print "$sstr $uniqid $hyouki $wid $ino $shuu $hin $maefeahin $hindo $kati\n";
	}
}

# 半角表記などの文字列の展開
sub exp_hankaku_array{
	my($self, $line) = @_;
  	my @eline = ();
	my @cline = split /,/,$line;
	for ($i = 0; $i <= $#cline; $i++){
		# ソート表記
		if($cline[$i]=~/:/) {
			my @dline = split /:/,$cline[$i];

			if($#eline < 0){
				for ($j = 0; $j <= $#dline; $j++){
					push(@eline, $dline[$j]);
				}
			} else {
				my $elineMax = $#eline;
				for ($k = 0; $k <= $elineMax; $k++){
					my $pre_eline = $eline[$k];
					$eline[$k] = "$pre_eline$dline[0]";
					push(@eline, "$pre_eline$dline[1]");
				}
			}
		} else {
			if($#eline < 0){
				push(@eline, $cline[$i]);
			} else {
				for ($k = 0; $k <= $#eline; $k++){
					$eline[$k] .= $cline[$i];
				}
			}
		}
	}
	@eline;
}

=head2 make_convtbl

 $mkdic->make_convtbl($convtbl_file, $prefix, $opt_B);

 統合品詞番号変換テーブルを作る。
 ModNlpResourceUnaJp.cppのconvTable[]のデータを生成し、$convtbl_fileに出力する。
 出力されるファイルのコードは、$ocodeになる。

 事前にmake_morphdicメソッドが実行されている必要がある。

=cut

sub make_convtbl {
	my($self, $convtbl_file, $prefix, $opt_B) = @_;
	$prefix ||= '';
	# 出力すべき品詞を調べ、@hinsに格納する
	my(@hins, %fixed, %hinlst, %used, %mohin_no);
	for $hin (@{$self->{fixhin}{hin}}) {	# 固定品詞を出力
		push(@hins, $hin);
		$fixed{$hin}++;
	}
	for $hin (@{$self->{hinlst}{hins}}) { # 前に作成したUNA品詞リストの品詞を出力
		next if $fixed{$hin};
		push(@hins, $hin);
		$hinlst{$hin}++;
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{hin_tbl}}) { # 辞書で使用している品詞
		$used{$hin}++;
		next if $fixed{$hin} || $hinlst{$hin};
		push(@hins, $hin);
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{basehin_tbl}}) { # 基本品詞
		next if $fixed{$hin} || $hinlst{$hin} || $used{$hin};
		push(@hins, $hin);
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{maefeahin_tbl}}) { # 前接素性付き品詞
		next if $fixed{$hin} || $hinlst{$hin} || $used{$hin} || $self->{basehin_tbl}{$hin};
		push(@hins, $hin);
	}
	# dichindo.tblが存在する場合
	if(defined($opt_B)){
		for $hin (@{$self->{dichindotbl}{hins}}) { # 過去に作成した実行用データに出現する品詞
			next if $fixed{$hin} || $hinlst{$hin} || $used{$hin} || $self->{basehin_tbl}{$hin} || $self->{maefeahin_tbl};
			push(@hins, $hin);
		}
	}
	# 形態素品詞番号の割り当て
	my $mohin_no = 1;
	for $hin (@hins) {
		$mohin_no{$hin} = $mohin_no++;
	}
	# 統合品詞番号変換テーブルを出力
	my $macro_prefix = uc ($prefix);
	open(CONVTBL, ">$convtbl_file") || die "can't write $convtbl_file: $!\n";
	my $line = "// UNA::MkMoDicにより自動生成されたファイル\n\n" .
	           "\t// UNA品詞番号からUNA統合品詞番号に変換するテーブル\n" .
	           "\tconst int convTable[] =\n" .
			   "\t{\n" .
			   "\t\t UNA_OTHER                          //    0 UNA_HIN_NOTHING\n";
	print CONVTBL $line;
	for $hin (@hins) {
		my $uni_hin_code = $self->get_uni_hin_code($hin);
		my $mohin_no = $mohin_no{$hin};
		$line = sprintf("\t\t,%-35s// %4d", $uni_hin_code, $mohin_no);
		if ($fixed{$hin}) {
			$line .= " ${macro_prefix}_$self->{fixhin}{label}[$mohin_no-1]";
		}
		$line .= "\n";
		print CONVTBL $line;
	}
	$line = "\t};\n";
	print CONVTBL $line;
	close CONVTBL;
}

# その品詞の統合品詞コードを得る
# get_hin_code()と異なり、素性つき品詞のときは
# $basehinの末尾に';'を付与した文字列を結果とする
sub get_uni_hin_code {
	my($self, $hin) = @_;
	my ($basehin, $fea) = &hin2basefea($hin);
	if ($fea ne '') {
		$basehin .= ';';
	}
	my $uni_hin_code = '';
	my $prev_level = -1;
	my %past; # そのレベルで見つかったことがあると真
	my %prev; # 直前でマッチしてたら真
	if ($self->{unihinmap}) {
		for $unihinmap_def (@{$self->{unihinmap}{defs}}) {
			my($rexpr, $str, $level) = @{$unihinmap_def};
			my $found;
			$past{$level} = 0 if $level > $prev_level; # レベルがあがったら
			if (!$past{$level}) { # まだ見つかっていない
				if ($level == 0 || $prev{$level - 1} == 1) { # 親が直前で真
					if ($basehin =~ /$rexpr/) {	# 正規表現にマッチ
						$found++;
					}
				}
			}
			if ($found) {
				$past{$level} = $prev{$level} = 1;
				if ($str =~ /^[\&\|]/) {
					$uni_hin_code .= $str;
				}
				elsif ($str ne '') {
					$uni_hin_code = $str;
				}
			}
			else {
				$prev{$level} = 0;
			}
			$past{$level} = 0 if $rexpr eq '.';	# 再度マッチさせるため
			$prev_level = $level;
		}
	}
	$uni_hin_code;
}

=head2 make_termtypetbl

 #$mkdic->{output_code} = $ocode; # euc,utf8,sjis,ucs2l,ucs2b
 $mkdic->make_termtypetbl($termtypetbl_file, $opt_B);

 検索語タイプテーブル(np-ja/termType.utf8)を生成し、$termtypebtl_fileに出力する。
 出力されるファイルのコードはeuc、改行コードはLFになる。(日本語を含まない。)

 事前にmake_morphdicメソッドが実行されている必要がある。

=cut

sub make_termtypetbl {
	my($self, $termtypetbl_file, $opt_B) = @_;
	# 出力すべき品詞を調べ、@hinsに格納する
	my(@hins, %fixed, %hinlst, %used, %mohin_no);
	for $hin (@{$self->{fixhin}{hin}}) {	# 固定品詞を出力
		push(@hins, $hin);
		$fixed{$hin}++;
	}
	for $hin (@{$self->{hinlst}{hins}}) { # 前に作成したUNA品詞リストの品詞を出力
		next if $fixed{$hin};
		push(@hins, $hin);
		$hinlst{$hin}++;
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{hin_tbl}}) { # 辞書で使用している品詞
		$used{$hin}++;
		next if $fixed{$hin} || $hinlst{$hin};
		push(@hins, $hin);
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{basehin_tbl}}) { # 基本品詞
		next if $fixed{$hin} || $hinlst{$hin} || $used{$hin};
		push(@hins, $hin);
	}
	for $hin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{maefeahin_tbl}}) { # 前接素性付き品詞
		next if $fixed{$hin} || $hinlst{$hin} || $used{$hin} || $self->{basehin_tbl}{$hin};
		push(@hins, $hin);
	}
	# dichindo.tblが存在する場合
	if(defined($opt_B)){
		for $hin (@{$self->{dichindotbl}{hins}}) { # 過去に作成した実行用データに出現する品詞
			next if $fixed{$hin} || $hinlst{$hin} || $used{$hin} || $self->{basehin_tbl}{$hin} || $self->{maefeahin_tbl};
			push(@hins, $hin);
		}
	}
	# 検索語タイプテーブルを出力
	open(TTTBL, ">$termtypetbl_file") || die "can't write $termtypetbl_file: $!\n";
	my $line = $self->{termtype}{val}{"TERM_STOP"} . "\n"; # UNA_HIN_NOTHINGに対応する行
	print TTTBL $line;
	my $num = $#{$self->{termtypemap}{hin}};
	for $hin (@hins) {
		$line = "";
		for (my $i = 0; $i <= $num; $i++) {
			my $pat = $self->{termtypemap}{hin}[$i];
			if ($hin =~ /^$pat/) {
				my $term = $self->{termtypemap}{term}[$i];
				$line = $self->{termtype}{val}{$term};
				last;
			}
		}
		$line .= "\n";
		print TTTBL $line;
	}
	close TTTBL;
}

=head2 >make_hinhindotbl

 $mkdic->>make_hinhindotbl($hinhindotbl_file, $opt_N);

 基本品詞頻度テーブルを生成し、$hinhindotbl_fileに出力する。
 出力されるファイルのコードはeuc、改行コードはLFになる。

=cut

sub make_hinhindotbl {
	my($self, $hinhindotbl_file, $opt_N) = @_;

	$self->{hingrp}->set_from_gobitbl($self->{gobitbl});
	$self->{fealst}->set_from_hingrp($self->{hingrp});
	$self->{hindo} = {};			# 品詞毎の頻度(単語頻度の推定値の和)
	$self->{basehindo} = {};		# 基本品詞毎の頻度(単語頻度の推定値の和)
	$self->{real_hindo} = {};		# 品詞毎の頻度(単語頻度の測定値の和)
	$self->{real_basehindo} = {};	# 基本品詞毎の頻度(単語頻度の測定値の和)
	$self->{wnum} = {};				# その品詞に属する単語数

	# 基本品詞頻度テーブルを出力
	open(HHDTBL, ">$hinhindotbl_file") || die "can't write $hinhindotbl_file: $!\n";
	select HHDTBL;
	my $line = <<"END";
#
# 基本辞書品詞頻度テーブル(hinhindo.tbl)
#
# Description:
#  前接素性付き品詞の頻度を記載する。
#  記述形式は以下の通りである。
#    前接素性つき品詞, 前接素性つき品詞hindo, 基本品詞hindo, 前接素性つき品詞real_hindo, 基本品詞real_hindo
#  mkmodic実行コマンドで"-F"オプションが指定された場合、本ファイルを読み込み、
#  前接素性付きもしくは基本品詞の頻度を用いて単語コストや品詞頻度を算出する。
#
END
	print $line;

	for $wid (@{$self->{dic}{wids}}) {
		# 異表記番号を得る
		my $ihyoukin = $self->{dic}{ihyoukin}{$wid}; # 最大値を得る
		my @ino = ();
		for $i (0 .. $ihyoukin) {
			my $ino = sprintf("%02d", $i);
			if ($self->get_info('h', $wid, $ino)) { # 欠番でなければ
				push(@ino, $ino);
		    }
		}
		# 表記毎に実行
		for $ino (@ino) {
			# $hinhindotbl_fileがある場合はhinhindo.tbl作成用に動作を制御する
			$self->make_1hyouki($wid, $ino, "", $hinhindotbl_file, $opt_N);
		}
	}

	# 元々頻度は前接素性品詞で算出していたため基本辞書品詞頻度テーブルには前接素性品詞に対する
    # 品詞と頻度を出力する
	# 頻度の取得時は前接素性品詞でアクセスする
	for $maefeahin (sort {$self->hin2sstr($a) cmp $self->hin2sstr($b)}
			keys %{$self->{maefeahin_tbl}}) { # 前接素性付き品詞
		my $mae_basehin = (&hin2basefea($maefeahin))[0];
		my $hindo = $self->{hindo}{$maefeahin};
		my $basehindo = $self->{basehindo}{$mae_basehin};
		my $real_hindo = $self->{real_hindo}{$maefeahin};
		my $real_basehindo = $self->{real_basehindo}{$mae_basehin};
		print "$maefeahin, $hindo, $basehindo, $real_hindo, $real_basehindo\n";
	}
	
	close HHDTBL;
}

=head1 COPYRIGHT

 Copyright 2000, 2014, 2023 Ricoh Company, Ltd. All rights reserved.

=head1 SEE ALSO

L<UNA::Cmn>, L<UNA::Cmn::Dic>, L<UNA::Cmn::HinGrp>, L<UNA::Cmn::GobiTbl>,
L<UNA::Cmn::FeaLst>, L<UNA::HinMap>, L<UNA::HanTbl>,
L<UNA::UniHinMap>, L<UNA::TermType>, L<UNA::TermTypeMap>

=cut

1;
