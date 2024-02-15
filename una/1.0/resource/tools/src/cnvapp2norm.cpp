// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// UNA用ソース辞書のアプリ情報を異表記正規化済み表記情報に置き換える
// 
// Copyright (c) 2002, 2014, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string.h>
#include "unaconst.h"
#include "ModAutoPointer.h"
#include "ModOstrStream.h"
#include "ModNlpUnaJp/ModNormRule.h"
#include "ModNlpUnaJp/ModNormalizer.h"
using namespace std;

#define MAX_DAT_LEN 256

const ModUnicodeChar EXP_SEP = 0x0a; 	// \n
const ModUnicodeChar DELIM0 = 0x7b;     // {
const ModUnicodeChar DELIM1 = 0x2c;     // ,
const ModUnicodeChar DELIM2 = 0x7d;     // }
const ModUnicodeChar DELIM3 = 0x5c;     // \\

char  StrPool[MAX_STR_POOL];
long  PoolEnd;
char  *Unadic[MAX_REC_NO];
long  WordNum;

/* func definition */
int ReadDic( FILE *fpi);
int ConvertDic( FILE *fpo, char *rscPath, ModBoolean conversionFlag);
void PrintHelp();

/* maiun routine */
int main(int ac, char **av)
{
  FILE *fin;
  FILE *fout;
  char rscPath[256];
  int i;
  ModBoolean isStrictCnv = ModFalse;

  /* デフォルト設定 */
  fin = stdin;
  fout = stdout;
  strcpy(rscPath,".");

  /* 引数の処理 */
  for ( i = 1; i < ac; ++i){
    if ( strncmp(av[i],"-h",2) == 0){  // ヘルプ 
      PrintHelp();
      return 0;
    }
    else if ( strncmp(av[i],"-i",2) == 0){ // 入力辞書 
      fin  = fopen(av[++i],"r");
      if (fin == NULL) {
        cerr << "Can't open " << av[i] << endl;
	exit(1);
      }
    }
    else if ( strncmp(av[i],"-o",2) == 0){ // 出力辞書 
      fout  = fopen(av[++i],"w");
      if (fout == NULL) {
        cerr << "Can't open " << av[i] << endl;
	exit(1);
      }
    }
    else if ( strncmp(av[i],"-r",2) == 0){ // 異表記正規化データディレクトリ
      strcpy(rscPath,av[++i]);
    }
    else if ( strncmp(av[i],"-s",2) == 0){ // 異表記正規化の結果を厳格にトレース
      isStrictCnv = ModTrue;
    }
    else{
      PrintHelp();
      return 0;
    }
  }

  /* 辞書読み込み */
  ReadDic(fin);

  /* 結果をくっつけて、新しい辞書として出力する */
  ConvertDic(fout, rscPath, isStrictCnv);

  fclose(fout);
  fclose(fin);

  return 0;
}

//--------------------------------------------------------------------------
// MODULE:    ReadDic
//
// ABSTRACT:  UNA用ソース辞書(euc)を読み込む
//
// FUNCTION:  UNA用ソース辞書を読み込んでメモリ上に蓄積する
//            WordNum/UnaDic/StrPool/PoolEndが変化する
//
// RETURN:
//    0  正常終了
//   -1  エラー
//
// NOTE:
//
int ReadDic(
  FILE *fin   /* UNA用ソース辞書のためのファイルポインタ */
)
{
  char lineBuffer[MAX_LINE_LEN*3]; /* １行分のバッファ */
  char id[256];          /* 単語ID文字列 */
  char hyoki[256];       /* 表記文字列 */
  char hinsi[256];       /* 品詞文字列 */
  char cost[256];        /* コスト */

  /* 文字列プールの終わり位置と、単語数を初期化する */
  PoolEnd = 0;
  WordNum = 0;

  /* ひたすら読み込み、メモリに蓄積 */
  while (1){

    /* break if eof */
    if ( !fgets(lineBuffer,MAX_LINE_LEN*3,fin) )
      break;
 
    /* get infos */
    sscanf(lineBuffer,"%s %s %s %s", id,hyoki,hinsi,cost);

    /* simple check */
    if ( (cost[0] < '0') || (cost[0] > '9')){
      fprintf(stderr, " Error ! : format error %s\n", lineBuffer);
      return -1;
    }

    /* set data formatted */
    lineBuffer[0] = 0;
    sprintf(lineBuffer,"%s %s %s %s", id, hyoki, hinsi, cost);
    Unadic[WordNum] = (StrPool+PoolEnd);
    strcpy(Unadic[WordNum],lineBuffer);
    PoolEnd = PoolEnd + strlen(lineBuffer)+1;
    WordNum++;
  }

  return 0;
}
//--------------------------------------------------------------------------
// MODULE:    ConvertDic
//
// ABSTRACT:  メモリ中の内容を正規化して出力する
//
// FUNCTION:  指定されたファイルに正規化後に出力
//
// RETURN:
//    0  正常終了
//   -1  エラー
//
// NOTE:
//
int ConvertDic(
  FILE *fout,    /* 出力先 */
  char *rscPath,  /* 正規化に用いるnomalizerのリソースパス */
  ModBoolean conversionFlag /* unicode変換に起因する変換漏れを回避する/しない */
)
{
  ModOs::Process::setEncodingType(ModKanjiCode::utf8);
  ModUnicodeChar d0(DELIM0);
  ModUnicodeChar d1(DELIM1);
  ModUnicodeChar d2(DELIM2);
  ModUnicodeChar d3(DELIM3);
  ModUnicodeChar sep(EXP_SEP);
  ModSize  len(100000);
  ModSize  mem_size(ModSizeMax >> 10);
  ModSize i;
  ModKanjiCode::KanjiCodeType io_code = ModKanjiCode::utf8;
  char buf[MAX_DAT_LEN];
  char id[MAX_DAT_LEN];
  ModCharString data_dir("");
  data_dir = rscPath;
  data_dir += "/";

  try {
    /* メモリ関連の初期化 */
    ModMemoryPool::initialize(mem_size);

    /* normalizerの初期化 */
    ModAutoPointer<UNA::UNAJP::ModNormRule> normRule;
    normRule = new UNA::UNAJP::ModNormRule(data_dir);
    UNA::UNAJP::ModNormalizer norm(normRule);

    for (i =0;i < WordNum; ++i){
      sscanf(Unadic[i],"%s %s", id, buf);
      ModUnicodeString line(buf, io_code);

      // bufを一個ずつチェックして、\uXXXXの表記をユニコード番号に変換
      int qq;  // 参照位置
      int pp;  // 格納位置
      char tnum[5];
      pp = 0;
      for ( qq = 0; line[qq]!=0; ){
	/* 英数系文字の場合はユニコード番号=ASC番号 */
	if ( line[qq] == '\\' &&  line[qq+1]=='u' &&
	     ((line[qq+2] >='0' && line[qq+2]<='9')|| 
	       (line[qq+2]>='a' && line[qq+2]<='f'))  && 
	     ((line[qq+3] >='0' && line[qq+3]<='9')|| 
	       (line[qq+3]>='a' && line[qq+3]<='f'))  && 
	     ((line[qq+4] >='0' && line[qq+4]<='9')|| 
	       (line[qq+4]>='a' && line[qq+4]<='f'))  && 
	     ((line[qq+5] >='0' && line[qq+5]<='9')|| 
	       (line[qq+5]>='a' && line[qq+5]<='f')) ){
           // asci の16進数字が得られる
           tnum[0]=line[qq+2];
           tnum[1]=line[qq+3];
           tnum[2]=line[qq+4];
           tnum[3]=line[qq+5];
           tnum[4]=0;
	   int y;
	   sscanf(tnum,"%x",&y); // pp は格納先の配列番号
	   line[pp] = y;
           pp++;
	   qq+=6;
	}
	else {
	   line[pp] = line[qq];
	   pp++;
	   qq++;
	}
      }
      line[pp] = 0;
      line.truncate( &(line[pp]));
      ModUnicodeString result;
      result = norm.normalizeBuf(line, 0, 0,
		UNA::UNAJP::ModNormalized, d0, d1, d2, d3, len);

      fputs(Unadic[i], fout);
      fputc(0x20, fout); // space

      int i;
      ModBoolean conversionFailed = ModFalse;
      ModUnicodeString s(result.getString(io_code), io_code);
      for ( i = 0; i < result.getLength(); ++i){
        if ( result[i] != s[i] ){
          conversionFailed = ModTrue;
          break;
        }
      }
      if ( ModTrue == conversionFlag && ModTrue == conversionFailed){
        for ( i = 0; i < result.getLength(); ++i){
          fprintf(fout,"\\u%04x", result[i]);
        }
      }
      else{
      	fputs(result.getString(io_code), fout);
      }
      fputc(0x0a, fout); // return
    }  
  } catch (ModException& e) {
    cout << "exception!: " << e.getMessageBuffer() << endl;
    exit(1);
  } catch (...) { 
    cout << "Unexpected exception!" << endl;
    exit(1);
  }
  return 0;
}
//--------------------------------------------------------------------------
// MODULE:    PrintHelp
//
// ABSTRACT:  ヘルプ表示
//
// FUNCTION:  
//
// RETURN:
//
// NOTE:
//
void PrintHelp() {
  cerr << "Usage: "
     << "[-h] "
     << "[-r DIR] "
     << "[-i FILE] "
     << "[-o FILE] " << endl
     << "\t-h       : このメッセージを表示" << endl
     << "\t-r DIR   : 辞書ディレクトリ(デフォルトは.)" << endl
     << "\t-i FILE  : 入力UTF-8ファイル(デフォルトは標準入力)" << endl
     << "\t-o FILE  : 出力UTF-8ファイル(デフォルトは標準出力)" << endl;
  return;
}
