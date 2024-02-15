// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// mkkeychr.cpp -- stdtbl.eucから文字列標準化処理のトリガとなる
//		文字のリストを作成する。
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#include <stdio.h>
#include <string.h>

#define STDTBL_MAX 1000
#define LOCAL_MAX_STR_POOL 100000

unsigned short KeyChrLstU[0x10000];
unsigned short KeyChrLstE[0x10000];

/* 標準化テーブル(部分)を格納する配列 */
unsigned char *StdTbl[STDTBL_MAX];
int StdTblCount;

/* 文字列本体の格納用プール */
unsigned char StrPool[LOCAL_MAX_STR_POOL];
int StrPoolCount;

/* 関数宣言 */
int InitMkKeyChar();
int ReadStdTblSrc();
int MkKeyCharSrc();
int GetMaxCode(int stdTblEnd);
int IsIncluding(int StdTblNo, int code);
int IsUnicode(char *str);

/* main routine */
int main()
{
  int rv;

  /* 初期化 */
  rv = InitMkKeyChar();
  if (rv<0){
    return rv;
  }

  /* stdtbl.eucを読み込む */
  rv = ReadStdTblSrc();
  if (rv<0){
    return rv;
  }

  /* keychar.euc を作成し、書き出す */
  rv = MkKeyCharSrc();
  if (rv<0){
    return rv;
  }

}

/* keychar.euc を作成し、書き出す */
int MkKeyCharSrc()
{
  int i;
  int j;
  int code;
  unsigned char *tmp;

  /* 標準化テーブルを見ながらカウントしていく */
  i = StdTblCount;
  while ( i>0){ /* 処理すべき標準化テーブルが残っている間 */

    /* 最多頻度の対象文字を得る */
    code = GetMaxCode(i);
    if ( code < 0){
      return code;
    }

    /* 出力 */
    if ( !(code & 0xffffff00) ){ // アスキーコード
      printf("%c\n", (char)code);
    }
    else if ( !(code & 0xffff0000)){ // EUCコード
      printf("%c%c\n", (char)(code/0x100),(char)(code%0x100));
    }
    else { // UNIコード
      printf("\\u%04x\n", (unsigned short)(code & 0x0000ffff));
    }

    /* 対象文字を含む標準化ルールを処理対象から除外する */
    for (j=0; j<i-1; ){
      if ( IsIncluding(j, code)){
	tmp = StdTbl[i-1];
	StdTbl[i-1] = StdTbl[j];
	StdTbl[j] = tmp;
	--i;
      }
      else{
	++j;
      }
    }
    if ( IsIncluding(j, code)){ // j == i-1
      --i;
    }
  }

  return 0;
}

/* 0maxまでのstdtblを調べ、最も多く使われているコードを見つける */
int GetMaxCode(int max)
{
  int i;
  int j;
  unsigned char *mchr;
  int mchrLen;
  int code;
  int maxCode;
  int maxCount;

  /* カウント配列を初期化する */
  for ( i = 0; i < 0x10000; ++i){
    KeyChrLstU[i] = 0;
    KeyChrLstE[i] = 0;
  }

  /* カウントする */
  for ( i = 0; i < max; ++i){
    for ( j = 0; StdTbl[i][j]!=0 ; ){
      mchr = StdTbl[i]+j;
      code = IsUnicode((char*)mchr);
      if ( code >=0){ // ユニコード
        mchrLen =6;
        KeyChrLstU[(code & 0x0000ffff)]++;
      }
      else if ( mchr[0]>=0xa0){ // EUC漢字
        code = mchr[0]*0x100 + mchr[1];
        KeyChrLstE[code]++;
        mchrLen = 2;
      }
      else {  // アスキー文字
        code = mchr[0];
        KeyChrLstE[code]++;
        mchrLen = 1;
      }

      j += mchrLen;
    }
  }

  /* 最大文字を探す */
  maxCode = -1;
  maxCount = 0;
  for ( i = 0; i < 0x10000; ++i){
    if ( KeyChrLstU[i]> maxCount){
      maxCode = 0x10000 + i;
      maxCount = KeyChrLstU[i];
    }
    if ( KeyChrLstE[i]> maxCount){
      maxCode = i;
      maxCount = KeyChrLstE[i];
    }
  }

  return maxCode;
  
}

/* \uXXXXと記述されたユニコード表現かどうか調べる */
int IsUnicode(char *mchr)
{
  int code;

  if ( mchr[0] !='\\') return -1;
  if ( mchr[1] !='u') return -1;
  if ( !strchr("0123456789ABCDEFabcdef",mchr[2]) ) return -1;
  if ( !strchr("0123456789ABCDEFabcdef",mchr[3]) ) return -1;
  if ( !strchr("0123456789ABCDEFabcdef",mchr[4]) ) return -1;
  if ( !strchr("0123456789ABCDEFabcdef",mchr[5]) ) return -1;
  if ( mchr[6] =='\\' || strchr("0123456789ABCDEFabcdef",mchr[6]) ) {
    sscanf((char*)(mchr+2),"%x",&code);
    code += 0x10000;
    return code;
  }

  return -1;
}

/* stdtbl.srcの該当行がチェック対象のコードを含むかどうか調べる */
int IsIncluding(int StdTblNo, int chkCode)
{
  int j;
  int mchrLen;
  unsigned char *mchr;
  int code;

  for ( j = 0; StdTbl[StdTblNo][j]!=0 ; ){
    mchr = StdTbl[StdTblNo]+j;
    if ( (code = IsUnicode((char*)mchr))>=0){ // ユニコード
      mchrLen =6;
    }
    else if ( mchr[0]>=0xa0){ // EUC漢字
      code = mchr[0]*0x100 + mchr[1];
      mchrLen =2;
    }
    else {  // アスキー文字
      code = mchr[0];
      mchrLen =1;
    }

    if ( chkCode == code){
      return 1;
    }
    j += mchrLen;
  }

  return 0;
}

/* stdtbl.src(euc)を読み込む */
int ReadStdTblSrc()
{
  unsigned char zzz[8192];
  unsigned char token1[4096];
  unsigned char token2[4096];
  int tn;
  
  /* 標準入力からデータを読み込み、配列にセットする */
  while (1){
    if (!fgets((char*)zzz, 8192, stdin)){
      break;
    }

    /* トークン２つの読み込み */
    tn = sscanf((char*)zzz,"%s %s", token1,token2);

    if ( tn < 1){ /* エラー */
      return -1;
    }

    /* 第１トークンの格納 */
    StdTbl[StdTblCount] = StrPool + StrPoolCount;
    memcpy( StdTbl[StdTblCount],token1,strlen((char*)token1)+1);
    StrPoolCount += (strlen((char*)token1) + 1);
    StdTblCount++;
  }

  return 0;
}

/* 初期化 */
int InitMkKeyChar()
{
  /* 標準化テーブルの格納数をリセット */
  StdTblCount = 0;
  StdTbl[0] = StrPool;

  /* 文字列本体の格納用プールをリセット */
  StrPoolCount = 0;

  return 0;
}
//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
