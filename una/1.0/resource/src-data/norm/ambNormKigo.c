/*
 * ambNormKigo --- 記号の曖昧性を除去する
 * 
 * Copyright (c) 1993, 2023 Ricoh Company, Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */ 

#define  AMB_NORM_KIGOU_C
#include <stdio.h>
#include "limeuniv.h"
#include "ambStd.h"				/* 検索システム共通 */
#include "amb.h"				/* 曖昧処理 */

static char *amb_version = ambVersion;

/*--------------------------------------------------------------*/
/*      削除記号テーブル                                        */
/*--------------------------------------------------------------*/
static const ushort ambKigoDelTbl[] = {
        0xa1a1,         /*    space             */
        0xa1a2,         /* 、 touten            */
        0xa1a3,         /* 。 kuten             */
        0xa1a4,         /* ， comma             */
        0xa1a5,         /* ． period            */
        0xa1a6,         /* ・ nakaguro          */
        0xa1a7,         /* ： colon             */
        0xa1a8,         /* ； semicolon         */
        0xa1a9,         /* ？                   */
        0xa1aa,         /* ！                   */
        0xa1ab,         /* ゛ dakuten           */
        0xa1ac,         /* ゜ han-dakuten       */
        0xa1ad,         /* ´ accentegue        */
        0xa1ae,         /* ｀ accentegure       */
        0xa1af,         /* ¨ umlaut            */
        0xa1b0,         /* ＾ accensil conflex  */
        0xa1b1,         /* ￣ upper line        */
        0xa1b2,         /* ＿ under line        */
        0xa1bc,         /* ー chouon kigou      */
        0xa1bd,         /* ― dash              */
        0xa1be,         /* ‐ hyphen            */
        0xa1bf,         /* ／                   */
        0xa1c0,         /* ＼                   */
        0xa1c1,         /* 〜                   */
        0xa1c2,         /* ‖                   */
        0xa1c3,         /* ｜                   */
        0xa1c4,         /* …                   */
        0xa1c5,         /* ‥                   */
        0xa1dd,         /* − minus             */
        0xa1eb,         /* ° degree            */
        0xa1ec,         /* ′ minute            */
        0xa1ed,         /* ″ second            */
};

/*--------------------------------------------------------------*/
/*      標準化記号テーブル                                      */
/*--------------------------------------------------------------*/
typedef struct {
        ushort  midasi;         /*　見出し文字  */
        ushort  norm;           /*　変換後文字  */
} ambKigoCnv;
static const ambKigoCnv ambKigoCnvTbl[] = {
        {0xa1c6, 0xa1ca},       /* ‘ --> （    */
        {0xa1c7, 0xa1cb},       /* ’ --> ）    */
        {0xa1c8, 0xa1ca},       /* “ --> （    */
        {0xa1c9, 0xa1cb},       /* ” --> ）    */
        {0xa1ca, 0xa1ca},       /* （ --> （    */
        {0xa1cb, 0xa1cb},       /* ） --> ）    */
        {0xa1cc, 0xa1ca},       /* 〔 --> （    */
        {0xa1cd, 0xa1cb},       /* 〕 --> ）    */
        {0xa1ce, 0xa1ca},       /* ［ --> （    */
        {0xa1cf, 0xa1cb},       /* ］ --> ）    */
        {0xa1d0, 0xa1ca},       /* ｛ --> （    */
        {0xa1d1, 0xa1cb},       /* ｝ --> ）    */
        {0xa1d2, 0xa1ca},       /* 〈 --> （    */
        {0xa1d3, 0xa1cb},       /* 〉 --> ）    */
        {0xa1d4, 0xa1ca},       /* 《 --> （    */
        {0xa1d5, 0xa1cb},       /* 》 --> ）    */
        {0xa1d6, 0xa1ca},       /* 「 --> （    */
        {0xa1d7, 0xa1cb},       /* 」 --> ）    */
        {0xa1d8, 0xa1ca},       /* 『 --> （    */
        {0xa1d9, 0xa1cb},       /* 』 --> ）    */
        {0xa1da, 0xa1ca},       /* 【 --> （    */
        {0xa1db, 0xa1cb},       /* 】 --> ）    */
        {0xa1e3, 0xa1ca},       /* ＜ --> （    */
        {0xa1e4, 0xa1cb},       /* ＞ --> ）    */
        {0xa2e3, 0xa1ca},       /* ≪ --> （    */
        {0xa2e4, 0xa1cb},       /* ≫ --> ）    */
};

/*--------------------------------------------------------------*/
/*      プロトタイプ宣言                                        */
/*--------------------------------------------------------------*/
ushort  seAmbNormKigou();       /* 記号揺れ適用　　　           */

/****************************************************************/
/*                                                              */
/*  Name   ... seAmbNormKigo                                    */
/*             記号揺れ適用　　　　                             */
/*  Input  ... ushort inp_moji     : 記号コード (JIS-X0208)     */
/*  Output ... ushort  rtn         : 変換後の文字コード         */
/*                                    0x0000 : 文字が無くなった */
/*                                    else   : 変換後の文字　   */
/*                                                              */
/*  Author ... W.Hiraishi                                       */
/*  Date   ... 93-11-05                                         */
/*                                                              */
/****************************************************************/
ushort
#ifdef LIME_C_ANSI
seAmbNormKigo( ushort inp_moji )
#else
seAmbNormKigo(inp_moji)
reg1 ushort     inp_moji;
#endif
{
        int     low;
        int     high;
        int     mid;

        /*------------------------------------------------------*/
        /*      削除記号テーブルのサーチ　　                    */
        /*------------------------------------------------------*/
        low = 0;
        high = (sizeof ambKigoDelTbl / sizeof(ushort)) - 1;
        while (low <= high) {
                mid = (low+high) /2;
                if (inp_moji < ambKigoDelTbl[mid])
                        high = mid -1;
                else if (inp_moji > ambKigoDelTbl[mid])
                        low = mid + 1;
                else {  /* 検出 */
                        return (ushort)0x0000; 
                }
        }

        /*------------------------------------------------------*/
        /*      標準化記号テーブルのサーチ                      */
        /*------------------------------------------------------*/
        low = 0;
        high = (sizeof ambKigoCnvTbl / sizeof(ambKigoCnv)) - 1;
        while (low <= high) {
                mid = (low+high) /2;
                if (inp_moji < ambKigoCnvTbl[mid].midasi)
                        high = mid -1;
                else if (inp_moji > ambKigoCnvTbl[mid].midasi)
                        low = mid + 1;
                else {  /* 検出 */
                        return ambKigoCnvTbl[mid].norm;
                }
        }

        return inp_moji;
}
