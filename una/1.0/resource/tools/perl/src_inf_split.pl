#!/usr/bin/perl
# 
# Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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
################################################################################
#
# src_inf_split.pl: 屈折情報の分離、屈折語形の生成
# ・source.datの屈折情報から屈折語形を生成する
# ・出力形式: 品詞毎の屈折タイプ数＝フィールド数（FSは"#"）、異表記区切りは"#"
#   名詞: 2フィールド(NS,ND)
#   動詞: 5フィールド(VI,V3,VD,VN,VG)
#   形容詞、副詞: 3フィールド(AB,AC,AS)
#   その他: 1フィールド(X)
#
################################################################################
use utf8;
use open IN  => ":utf8";
binmode(STDIN,  ":utf8");
binmode(STDOUT, ":utf8");

$FS = ' ';		# set field separator
$, = ' ';		# set output field separator
$\ = "\n";		# set output record separator

$FS = $, = '#';

while (<>) {
    chomp;	# strip record separator
    @Fld = split($FS, $_, 9999);

    # 屈折情報あり
    if ($Fld[$#Fld] ne '') {
	$N_INF = (@INF = split(/\|/, $Fld[$#Fld], 9999));

	### 名詞
	if ($N_INF == 1) {
	    ##### 小品詞の数情報が複数 → 複数形エントリとする
	    if ($Fld[1] =~ /p$/) {
		$NS = '';
		$NP = $Fld[0] . '|NP|@';
	    }

	    else {
		####### 単数形
		$NS = $Fld[0] . '|NS|@';

		####### 複数形
		$NP = '';
		$N_VAR = (@VAR = split(/~/, $Fld[$#Fld], 9999));
		for ($i = 0; $i < $N_VAR; $i++) {
		    if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {
			########### 規則屈折
			if ($VAR[$i] =~ /^\-./) {
			    ############# 外来屈折は不規則扱い
			    if ($VAR[$i] eq '-a' || $VAR[$i] eq '-i' ||

			      $VAR[$i] eq '-y' || $VAR[$i] eq '-er' ||

			      $VAR[$i] eq '-en' || $VAR[$i] eq '-im' ||

			      $VAR[$i] eq '-ren' || $VAR[$i] eq '-ta' ||

			      $VAR[$i] eq '-x') {
				$SUF = '*';
				$VAR[$i] =~ s/^\-//;
				$VAR[$i] =~ s/\-$//;
				$NP = $NP . ';' . $Fld[0] . $VAR[$i] . '|NP|'

				  . $SUF;
			    }
			    ############# 規則屈折
			    else {
				$SUF = $VAR[$i];
				$SUF =~ s/^\-//;
				$SUF =~ s/\-$//;
				$NP = $NP . ';' . $Fld[0] . $SUF . '|NP|@+' . $SUF;
			    }
			}
			########### 同形
			elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			    $NP = $NP . ';' . $Fld[0] . '|NP|@';
			}
			########### その他
			else {
			    $NP = $NP . ';' . $VAR[$i] . '|NP|*';
			}
		    }
		}
		$NP =~ s/^;//;
	    }
	    print $NS, $NP;
	}

	### 形容詞、副詞
	elsif ($N_INF == 2) {
	    ##### 原級
	    if ($Fld[1] =~ /^A/) {
		$AB = $Fld[0] . '|AB|@';
	    }
	    else {
		$AB = $Fld[0] . '|DB|@';
	    }
            ##### 比較級
	    $AC = '';
	    $N_VAR = (@VAR = split(/~/, $INF[0], 9999));
	    for ($i = 0; $i < $N_VAR; $i++) {
		if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {	#???
		    ######### 規則屈折
		    if ($VAR[$i] =~ /^\-./) {
			$SUF = $VAR[$i];
			$SUF =~ s/^\-//;
			$SUF =~ s/\-$//;

			if ($Fld[1] =~ /^A/) {
			    $AC = $AC . ';' . $Fld[0] . $SUF . '|AC|@+' . $SUF;
			}
			else {
			    $AC = $AC . ';' . $Fld[0] . $SUF . '|DC|@+' . $SUF;
			}
		    }
		    ######### 同形
		    elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			if ($Fld[1] =~ /^A/) {
			    $AC = $AC . ';' . $Fld[0] . '|AC|@';
			}
			else {
			    $AC = $AC . ';' . $Fld[0] . '|DC|@';
			}
		    }
		    ######### その他
		    else {
			if ($Fld[1] =~ /^A/) {
			    $AC = $AC . ';' . $VAR[$i] . '|AC|*';
			}
			else {
			    $AC = $AC . ';' . $VAR[$i] . '|DC|*';
			}
		    }
		}
	    }
	    $AC =~ s/^;//;

	    ##### 最上級
	    $AS = '';
	    $N_VAR = (@VAR = split(/~/, $INF[1], 9999));
	    for ($i = 0; $i < $N_VAR; $i++) {
		if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {	#???
		    ######### 規則屈折
		    if ($VAR[$i] =~ /^\-./) {
			$SUF = $VAR[$i];
			$SUF =~ s/^\-//;
			$SUF =~ s/\-$//;

			if ($Fld[1] =~ /^A/) {
			    $AS = $AS . ';' . $Fld[0] . $SUF . '|AS|@+' . $SUF;
			}
			else {
			    $AS = $AS . ';' . $Fld[0] . $SUF . '|DS|@+' . $SUF;
			}
		    }
		    ######### 同形
		    elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			if ($Fld[1] =~ /^A/) {
			    $AS = $AS . ';' . $Fld[0] . '|AS|@';
			}
			else {
			    $AS = $AS . ';' . $Fld[0] . '|DS|@';
			}
		    }
		    ######### その他
		    else {
			if ($Fld[1] =~ /^A/) {
			    $AS = $AS . ';' . $VAR[$i] . '|AS|*';
			}
			else {
			    $AS = $AS . ';' . $VAR[$i] . '|DS|*';
			}
		    }
		}
	    }
	    $AS =~ s/^;//;
	    print $AB, $AC, $AS;
	}

	### 動詞
	elsif ($N_INF == 4) {
	    ##### 不定形
	    $VI = $Fld[0] . '|VI|@';
	    ##### 現在形
	    $VR = $Fld[0] . '|VR|@';

	    ##### ３単現
	    $V3 = '';
	    $N_VAR = (@VAR = split(/~/, $INF[2], 9999));
	    for ($i = 0; $i < $N_VAR; $i++) {
		if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {
		    ######### 規則屈折
		    if ($VAR[$i] =~ /^\-./) {
			$SUF = $VAR[$i];
			$SUF =~ s/^\-//;
			$SUF =~ s/\-$//;
			$V3 = $V3 . ';' . $Fld[0] . $SUF . '|V3|@+' . $SUF;
		    }
		    ######### 同形
		    elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			$V3 = $V3 . ';' . $Fld[0] . '|V3|@';
		    }
		    ######### その他
		    else {
			$V3 = $V3 . ';' . $VAR[$i] . '|V3|*';
		    }
		}
	    }
	    $V3 =~ s/^;//;

	    ##### 過去形
	    $VD = '';
	    $N_VAR = (@VAR = split(/~/, $INF[0], 9999));
	    for ($i = 0; $i < $N_VAR; $i++) {
		if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {
		    ######### 規則屈折
		    if ($VAR[$i] =~ /^\-./) {
			if ($VAR[$i] eq '-t') {
			    $SUF = '*';
			    $VAR[$i] =~ s/^\-//;
			    $VAR[$i] =~ s/\-$//;
			    $VD = $VD . ';' . $Fld[0] . $VAR[$i] . '|VD|' . $SUF;
			}
			else {
			    $SUF = $VAR[$i];
			    $SUF =~ s/^\-//;
			    $SUF =~ s/\-$//;
			    $VD = $VD . ';' . $Fld[0] . $SUF . '|VD|@+' . $SUF;
			}
		    }
		    ######### 同形
		    elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			$VD = $VD . ';' . $Fld[0] . '|VD|@';
		    }
		    ######### その他
		    else {
			$VD = $VD . ';' . $VAR[$i] . '|VD|*';
		    }
		}
	    }
	    $VD =~ s/^;//;

	    ##### 過去分詞
	    $VN = '';
	    $N_VAR = (@VAR = split(/~/, $INF[1], 9999));
	    for ($i = 0; $i < $N_VAR; $i++) {
		if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {
		    ######### 規則屈折 ("-ed"以外は不規則扱い)
		    if ($VAR[$i] =~ /^\-./) {
			if ($VAR[$i] eq '-den' || $VAR[$i] eq '-en' ||
			  $VAR[$i] eq '-n' || $VAR[$i] eq '-ne' ||
			  $VAR[$i] eq '-t') {
			    $SUF = '*';
			    $VAR[$i] =~ s/^\-//;
			    $VAR[$i] =~ s/\-$//;
			    $VN = $VN . ';' . $Fld[0] . $VAR[$i] . '|VN|' . $SUF;
			}
			else {
			    $SUF = $VAR[$i];
			    $SUF =~ s/^\-//;
			    $SUF =~ s/\-$//;
			    $VN = $VN . ';' . $Fld[0] . $SUF . '|VN|@+' . $SUF;
			}
		    }
		    ######### 同形
		    elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			$VN = $VN . ';' . $Fld[0] . '|VN|@';
		    }
		    ######### その他
		    else {
			$VN = $VN . ';' . $VAR[$i] . '|VN|*';
		    }
		}
	    }
	    $VN =~ s/^;//;

	    ##### 現在分詞
	    $VG = '';
	    $N_VAR = (@VAR = split(/~/, $INF[3], 9999));
	    for ($i = 0; $i < $N_VAR; $i++) {
		if ($i == 0 || ($i > 0 && $VAR[$i] ne $VAR[$i - 1])) {
		    ######### 規則屈折
		    if ($VAR[$i] =~ /^\-./) {
			$SUF = $VAR[$i];
			$SUF =~ s/^\-//;
			$SUF =~ s/\-$//;
			$VG = $VG . ';' . $Fld[0] . $SUF . '|VG|@+' . $SUF;
		    }
		    ######### 同形
		    elsif ($Fld[0] eq $VAR[$i] || $VAR[$i] eq '-') {
			$VG = $VG . ';' . $Fld[0] . '|VG|@';
		    }
		    ######### その他
		    else {
			$VG = $VG . ';' . $VAR[$i] . '|VG|*';
		    }
		}
	    }
	    $VG =~ s/^;//;
	    print $VI, $V3, $VD, $VN, $VG;
	}
    }

    # 屈折情報なし
    else {
	### 名詞
	if ($Fld[1] =~ /^N/) {
	    if ($Fld[1] =~ /p$/) {
		$NS = '';
		$NP = $Fld[0] . '|NP|@';
	    }
	    else {
		$NS = $Fld[0] . '|NS|@';
		$NP = '';
	    }
	    print $NS, $NP;
	}

	### 形容詞
	elsif ($Fld[1] =~ /^A/) {
	    $AB = $Fld[0] . '|AB|@';
	    $AC = '';
	    $AS = '';
	    print $AB, $AC, $AS;
	}

	### 副詞
	elsif ($Fld[1] =~ /^D/) {
	    $AB = $Fld[0] . '|DB|@';
	    $AC = '';
	    $AS = '';
	    print $AB, $AC, $AS;
	}

	else {
	    print $Fld[0] . '|X|@';
	}
    }
}
