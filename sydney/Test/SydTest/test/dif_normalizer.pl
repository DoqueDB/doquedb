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
# dif_normalizer.pl

@OrgArray = (@OrgArray, [@_]); # オリジナルのdif情報
@ResArray = (@ResArray, [@_]); # normalizeした結果, difが存在する場合の情報
@SerArray = (@SerArray, [@_]); # 作業用
@array1=();                       # SerArray内の情報を比較の為に1-D arrayにコピー
@array2=();                       # SerArray内の情報を比較の為に1-D arrayにコピー
@OrgdatArray=();                  # オリジナルの中間出力情報
@OrglogArray=();                  # オリジナルの正解出力情報
@OrderByArray=();                 # オリジナルのdifファイルからorder by結果の情報
@DifNo=();                        # Difファイルの行番号
@ComNo=();                        # Commandの出力範囲(行番号)
@ComOrderByNo=();                 # Order By の sql Noを取得
@OrderBySQL=();                   # Order by の sqlを取得
$orderbyinNo="";                  # コマンド Order By 句の始め
$orderbyoutNo="";                 # コマンド Order By 句の終わり
$OrderBy=0;                       # difファイルの内容にOrder by出力が含まれているかどうか
$OrderByT=0;                      # Order By 句を使った出力でのdiff情報
#$depth=0;                        # SerArray[row]の深さ
$explain=0;                       # Plan出力が含まれるかどうか
$success="";

sub InitArray
{
    (@myarray) = @_;
    while (@myarray > 0) { pop (@myarray); }
    return @myarray;
}

sub PrintDifResult
{
    my (@Two_D_Array) = @_;

    for $i ( 0 .. $#Two_D_Array ) {
        for $j ( 0 .. $#{$Two_D_Array[$i]} ) {
            print "$Two_D_Array[$i][$j]";
        }
    }
}

sub ReadDifFile
{
    my $i=0, $j=0, $s=0, $t=0, $no=0, $st=0, $sqlt="";

    do {
        $line=<IN_dif>;

        while (($line =~ /^[0123456789]/) or ($line != EOF)) {
            $OrgArray[$i][$j] = $line; # [i, 0]に行番号を入れる。

            if ( $OrgArray[$i][$j] =~ /(\d+)(\,|[a-zA-Z])/){
                $DifNo[$s]=$1;
                $s++;
                $OrderByT=0;
            }
            # order by 句のdifであるか確認
            foreach $value ( @ComOrderByNo ) {
                if (( $ComNo[$value] < $DifNo[$s-1] ) && ( $ComNo[($value+1)] > $DifNo[$s-1] )) {
                    $orderbyinNo[$t] = $ComNo[$value], $orderbyoutNo[$t] = $ComNo[$value+1]-1;
                    $OrderBy=1;
                    $OrderByT=1;
                    $sqlt = $OrderBySQL[$value];
                    $sqlt =~ s/^.+\] (\[\[.+)/$1/;
                    $t++;
                }
            }

            if (( $s == 1 ) && ( $no == 0 )) {
                for ( $count=0; $DifNo[$s-1] > $no; $count++ ){ $no = $ComNo[$count]; }
                if ( $OrderByT == 1 ) {
                    $OrderByArray[$i][$j] = $line;
                    $j++;
                    if ( $explain == 0 ) {
                        $OrderByArray[$i][$j] = $sqlt;
                    } else {
                        $OrderByArray[$i][$j] = "[[SQL Query]] \n";
                    }
                }
                $st=1;
            }
            if ( $no < $DifNo[$s-1] ) {
                for ( $count=0; $DifNo[$s-1] > $no; $count++ ){ $no = $ComNo[$count]; }
                $OrgArray[$i][$j] = "";
                $j =0, $i++;
                $OrgArray[$i][$j] = $line;
                if ( $OrderByT == 1 ) {
                    $OrderByArray[$i][$j] = $line;
                    $j++;
                    if ( $explain == 0 ) {
                        $OrderByArray[$i][$j] = $sqlt;
                    } else {
                        $OrderByArray[$i][$j] = "[[SQL Query]] \n";
                    }
                }
            } elsif (($line =~ /^[0123456789]/) && ($st == 0)) {
                $OrgArray[$i][0] =~ s/(\x0D\x0A|\x0D|\x0A)$//;
                $OrgArray[$i][0] = "$OrgArray[$i][0] ; $line";
                if ( $OrderByT == 1 ) {
                    $OrderByArray[$i][0] =~ s/(\x0D\x0A|\x0D|\x0A)$//;
                    $OrderByArray[$i][0] = "$OrderByArray[$i][0] ; $line";
                }
            }
            $st=0;
            $j++;
            $line=<IN_dif>;

            #ここで数字(行)以降から数字まで一つのcolに保存
            while ($line =~ /^[^0-9]/) {
                $OrgArray[$i][$j] = $line;
                if ( $OrderByT == 1 ){
                    #$line="" if ($line =~ /^---/);
                    $OrderByArray[$i][$j] = $line;
                }
                $j++;
                $line=<IN_dif>;
            } #while
        } #while
    } until ($line== EOF);
}

sub ReadDatFile
{
    my $i=0, $ob=0, $no=1, $check=0;

    while (<IN_dat>) {
        # odry byのあるsql no取得
        s/(\x0D\x0A|\x0D|\x0A)$/\n/;
        $OrgdatArray[$no]=$_;
        if ($_ =~ /order by\s+([^ \t]+)/i ) {
            $ComOrderByNo[$ob] = $i - 1;
            $OrderBySQL[$i-1] = $OrgdatArray[$no];
            $ob++;
        }

        # Plan出力を含んだ出力
        if ( $_ =~ /start explain execute/ ) {
            $explain=1;
        }

        # CreatePreparedCommand で order by を使用しているときは並べ替えをしない
        if ( $_ =~ /CreatePreparedCommand/ ) {
            $check=1;
        }
        if ( $_ =~ /ErasePreparedCommand/ ) {
            $check=0;
        }
        if (( $check == 1 ) && ($_ =~ /order by \S*( desc| asc|)( limit \d|)(" \[.+\]\;|)$/i )) {
            $check=0;
            $success=9;
        }

        if (($_ =~ /\[Main\] Command/ ) || ($_ =~ /\[Main\] PreparedCommand/)){
            $ComNo[$i] = $no;
            $i++;
        }
        if ($_ =~ /Main End/ ) { $ComNo[$i] = $no; }
        if ($_ == EOF ) { $ComNo[$i] = $no; }
        $no++;
    }
}

sub ReadLogFile
{
    my $no=1;
    while (<IN_log>) {
        s/(\x0D\x0A|\x0D|\x0A)$/\n/;
        $OrglogArray[$no]=$_;
        $no++;
    }
}

# stringに付いている矢印を返す。
sub getArrow
{
    my ($str) = @_;
    if ( ($str =~ (/^>/)) || ($str =~ (/^</)) ) { return $&; }
    else                                        { return -1; }
}

# array内二つをチェックして同じならpop、そうでないならエラーなので終了
sub CompareArray
{
    my $result = 0; # Normalizer()の@array1と@array2が同じなら返り値0そうでないなら、-1

    my $len1 = @array1;
    my $len2 = @array2;
    if (len1 != len2){ $result = -1; }
    else {
        my @sorted1 = sort(@array1);
        my @sorted2 = sort(@array2);
        for $i (0..$len1) {

            #$sorted1[$i] =~ s/length \= \d+/length \= xxx/g;
            #$sorted2[$i] =~ s/length \= \d+/length \= xxx/g;

            #$sorted1[$i] =~ s/size\=\d+/size\=xxx/g;
            #$sorted2[$i] =~ s/size\=\d+/size\=xxx/g;

            if ( $sorted1[$i] ne $sorted2[$i] ) {
                $result = -1;
                last;
            }
        }
    }
    return $result;
} # end CompareArray

sub PushIntoOneArray
{
    # The temporary line
    my $string;
    # The temporary character which stores an arrow character.
    my $arrow;
    # difの異なる行同士を表す組の、各行数が等しいかどうか
    my $precheck_success;

    for my $i( 0 .. $#OrgArray ){
        for $j ( 0...$#{$OrgArray[$i]} ){
            $string = $OrgArray[$i][$j];
            $string =~ s/\x0D\x0A|\x0D|\x0A/\n/g;

            $arrow = getArrow($string);
            if    ($arrow eq "<") { $precheck_success++; }
            elsif ($arrow eq ">") { $precheck_success--; }

            if ($string =~ /^[<]/) {
                $string =~ s/[<]\s//;
                push (@array1, $string);
            }
            elsif ($string =~ /^[>]/) {
                $string =~ s/[>]\s//;
                push (@array2, $string);
            }
        }
        $success = CompareArray;
        last if ( $success != 0 );
    }
    return $precheck_success;
}

# 中間出力と正解LOGを比較する
sub PushIntoOneArray2
{
    my $no=1, $i=0;
    @array1=();
    @array2=();

    # The temporary line
    my $string;
    # The temporary character which stores an arrow character.
    my $arrow;
    # difの異なる行同士を表す組の、各行数が等しいかどうか
    my $precheck_success;

    for my $i( 0 .. $#OrgArray ){
        for $j ( 0...$#{$OrgArray[$i]} ){
            $string = $OrgArray[$i][$j];
            $string =~ s/\x0D\x0A|\x0D|\x0A/\n/g;

            $arrow = getArrow($string);
            if    ($arrow eq "<") { $precheck_success++; }
            elsif ($arrow eq ">") { $precheck_success--; }
        }
    }

    foreach $value ( @DifNo ) {
        foreach $value1 ( @ComNo ) {
            if (( $no < $value ) && ( $value1 > $value ) && ($i < $no)) {
                for ($i=$no; $i < $value1; $i++) {
                    push (@array1, $OrgdatArray[$i]);
                    push (@array2, $OrglogArray[$i]);
                }
                $i=$no;
            }
            $no=$value1;
            $success = CompareArray;
            last if ( $success != 0 );
        }
    }
    return $precheck_success;
}

# メインループ
sub Normalizer()
{
    my $equal_arrow_num="", $name=${args[0]};

    # Set SerArray to array1 and array2.
    if ( $success != 9 ) {
        $equal_arrow_num = PushIntoOneArray;
    }

    # single系のみ
    if (($success != 0) && ($name %10000 < 5000) && ($success != 9)) {
        ReadLogFile;
        $equal_arrow_num = PushIntoOneArray2;
    }

    if (($equal_arrow_num == 0) && ($success == 0)) {
        # Success
        @array1 = InitArray(@array1);
        @array2 = InitArray(@array2);
        @OrgArray = InitArray(@OrgArray);
     } else {
        # Fail
        if ($OrderBy == 1) {
            if (@OrderByArray > 0) {
                for my $i( 0 .. $#OrderByArray ){
                   for $j ( 0...$#{$OrderByArray[$i]} ){
                       if (($j == 0) && ($OrgArray[$i][$j] =~ /\d/) && ( $success != 9 )){
                           $OrderByArray[$i][$j] =~ s/(\x0D\x0A|\x0D|\x0A)$/ : ***** \n/;
                       }
                       $OrgArray[$i][$j] = $OrderByArray[$i][$j];
                   }
                }
            }
            @OrderByArray=();
        }
        do {
            # Copy SerArray to ResArray.
            push(@ResArray, shift(@OrgArray) );
        } while (@OrgArray > 0);
        @array1 = InitArray(@array1);
        @array2 = InitArray(@array2);
    }

    if ($OrderBy == 1) {
        if (@OrderByArray > 0) {
            for my $i( 0 .. $#OrderByArray ){
               for $j ( 0...$#{$OrderByArray[$i]} ){
                   $OrgArray[$i][$j] = $OrderByArray[$i][$j];
               }
            }
        }
        @OrderByArray=();
    }

    # outファイル読み込みは終了したが、まだOrgArrayに情報が残っている場合もある。
    if (@OrgArray > 0) {
        do {
            push(@ResArray, shift(@OrgArray) );
        }
        while (@OrgArray > 0);
    }
}

####################################### main #######################################
$args[0] = <@ARGV>;

open(IN_dat, "./result/${args[0]}.dat") or die;
open(IN_dif, "./result/${args[0]}.dif") or die;
open(IN_log, "./tmpexpectlog.log") or die;

ReadDatFile;                      # order by
ReadDifFile;                      # read dif file and push info into array

close(IN_dat);                    # Close dat file
close(IN_dif);                    # Close input file

Normalizer();
PrintDifResult(@ResArray);
close(IN_log);

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
