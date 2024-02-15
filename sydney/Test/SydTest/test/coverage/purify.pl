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
# Purifyテストで検出されたERROR, WARNING等を正規化して出力
#
# カレントフォルダ内のdirnames_/testdir_で定義したフォルダ内全部の*.txtをサーチ
#
# for each single(multi, recovery)/normal, excpet
# 1) Purifyテストで作成された*.txtを探す。
# 2) 定義された値を正規化
# 3) ファイルにエラー部分のファイル名を出力する

# エラー定義
# 1) [E]が存在する
# 2) [W] UMR
# 3) メモリリークが1000バイト以上

@dirnames_ = qw(single    single_utf8
                single_eu single_eu_utf8
                single_ja single_ja_utf8
                single_zh single_zh_utf8
                single_dist recovery_dist
                multi
                recovery );

@testdir_ = qw(normal except);

@error;
@warning;
@memory;
@memory2;
$puriout=0;
    
####################################### func #######################################

# エラー定義で引っかかった内容(array内)を出力
sub print_array
{
  my ($type, @array) = @_;
  
  print "$type...\n";
  print "name\tdescription\n";
  print "---------------------------------------------------------------------------------------------------------------------\n";

  #foreach $item (@_)
  while (@array) {
    $item = shift(@array);
    print "$item\n";
  }
  print "\n";

  #return @array;
} # end sub


# 結果を出力
sub print_res
{
  # エラー内容が重複している場合があるので単一にする
  @error   = delete_dupulicate(@error);
  @warning = delete_dupulicate(@warning);
  @memory = delete_dupulicate(@memory);
  @memory2 = delete_dupulicate(@memory2);

  print "#########################\t$dirnames/$testdir\t#########################\n\n";

  if ( (!@error) && (!@warning) && (!@memory) && (!@memory2) && (!@linuxlog) )  { print ("No Error, Memory Leak Error, and Waring found\n\n"); }

  else {
   if (!@error)   { print ("No Error found \n\n");              }
   if (!@warning) { print ("No Waring found \n\n");             }
   if (!@memory)  { print ("No Memory Leak Error found \n\n"); }
   if (!@memory2)  { print ("No Memory Leak Error found \n\n"); }
  }
  
  if (@error)   { @error   = print_array("Error...", @error);              }  
  if (@warning) { @warning = print_array("Warning...", @warning);          }
  if (@memory)  { @memory  = print_array("Memory Leak Error...", @memory); } 
  if (@memory2)  { @memory2  = print_array("Memory Leak Error...", @memory2); }

  print "\n";
  
} # end print_res


# 同ファイルにまったく同じエラーが存在する場合は削除
sub delete_dupulicate
{
    %ct;
    return @_ = grep(!$ct{$_}++, @_);
} # end sub

# メインループ
sub normalize
{
  my $num;
  
  while ( @_ ) {
    $text = shift(@_);
    $text =~ /\d+/;
    $num = $&;
    
    open( SRC, "< $text" ) or die;

    while( <SRC> )
    {

      # 同じ内容でも回数が違うと重複されて出力されるので削除
      s/{\d+ 回発生}//i;

      # 想定内エラーが存在する場合は削除(下記は参考用）
      s/ModOsDriver::Memory::alloc\(DWORD,ModBoolean\)に割り当てた \d+ ブロックから \d+ バイト \[libCommon04.dll\]//i;
      s/freeに割り当てた \d+ ブロックから \d+ バイト \[MSVCR71.dll\]//i;
      s/LdrLoadDllに割り当てた \d+ ブロックから \d+ バイト \[NTDLL.dll\]//i;
      s/\d+ ブロックから \d+ バイト \[kernel32.dll\]//i;

      # 想定内エラーが存在する場合は削除（想定内エラーを下記に記載する
      s/C:\\WINDOWS\\system32\\WLDAP32.dllに割り当てた \d+ ブロックから \d+ バイト//i;
      next if/\{0 バイト, 0 ブロック\}/;
      #next if/TRMeister::SydTest::Executor/;
      next if/^\[I\] /;

      # hint heap 'compressed' でのpurify誤検地
      next if/ModUnicodeCharTrait::length/;
      next if/ModPureStrStreamBuffer<WORD>/;
      next if/ModUnicodeCharTrait::compare/;
      next if/A0xafc3b0fb::_CRC32::generate/;
     
      # [E]:
      if ($_ =~ /^\[E\]/) {
        chomp($line = "$num\t$_");
        push (@error, $line);
      }

      # [W] UMR
      if ($_ =~ /^\[W\] UMR/) {
        chomp($line = "$num\t$_");
        push (@warning, $line);
      }

      # メモリリークが1000バイト以上
      if ( $_ =~ /すべてのメモリ\D+\{(\d+)/ ) {
        #if ($1 >= 1000) {
          chomp($line = "$num\t$_");
          push (@memory, $line);
        #}
      }

      # 想定外メモリリーク
      if ( $_ =~ /メモリ/ ) {
         if ( $_ =~ /\D+\ (\d+)/ ) {
            if (\D > 1) {
              chomp($line = "$num\t$_");
              push (@memory2, $line);
            }
         }
      }

      if ( $_ =~ /^\w{3}: .+:/) {
          chomp($line = "$num\t$_");
          #push (@linuxlog, $line);
          $tmpout=$line;
          $puriout=1;
      }

      if ( $puriout==1 ) {
          if ( $_ =~ /\[rtlib.o\]/ ) {
              $puriout=0;
          } elsif ( $_ =~ /\[lib\w.so.\d/ ) {
              $puriout=0;
          } elsif ( $_ =~ /\[.+\]/ ) {
              if ( $tmpout =~ /U\w{2}:/ ) {
                  push (@warning, $tmpout);
                  chomp($line = "$num\t$_");
                  push (@warning, $line);
              } elsif ( $tmpout =~ /M\w{2}:/ ) {
                  push (@memory, $tmpout);
                  chomp($line = "$num\t$_");
                  push (@memory, $line);
              } else {
                  push (@memory2, $tmpout);
                  chomp($line = "$num\t$_");
                  push (@memory2, $line);
              }
              $puriout=0;
          }
      }
      
      $puriout=0 if ( $_ =~ /\*\*\*\*/ ) ;
    }
    close(SRC);
  } # while
  print_res();
  
} # end normalize

####################################### main #######################################


# 指定されたdirから全部のテキスト形式ファイル名を探す

# purify不要なテスト
system 'rm -f recovery/normal/25520.txt';   # 強制的に切断したテスト
system 'rm -f recovery/normal/25522.txt';   # 強制的に切断したテスト
system 'rm -f recovery/except/25521.txt';   # 強制的に切断したテスト

# single, recover, multi, etc系
foreach $dirnames (@dirnames_) {
  # normal/except系
  foreach $testdir (@testdir_) {
    @files_ = <./$dirnames/$testdir/*.txt>;
    normalize(@files_) if (@files_);
  }
}

#debug;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
