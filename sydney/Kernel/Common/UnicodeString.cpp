// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Unicodestring.cpp -- UNICODE 関連の関数定義
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

namespace {
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#ifdef OBSOLETE
#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Message.h"
#include "Common/UnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {

// 
// STRUCT
// Stringkey -- 文字のキー値
// 
// NOTES
// 比較対象となる各文字のメインキーとサブキーを保持する
//
struct StringKey {
	unsigned short usMainKey;
	unsigned short usSubKey;
};

// 
// STRUCT
// Charkey -- 文字のコード、キー値
//
// NOTES
// キーをセットした文字について、文字コードとセットしたキーを保持する
//
struct CharKey {
	ModUnicodeChar usCode;
	unsigned short usMainKey;
	unsigned short usSubKey;
};

//
// CONST
// UCS-2文字コードによる特定の文字のコードを示す定数群
//
const ModUnicodeChar usFullDaku       = 0x309bU; // 「゛」
const ModUnicodeChar usHalfDaku       = 0xff9eU; //  半角「゛」
const ModUnicodeChar usFullHandaku    = 0x309cU; // 「゜」
const ModUnicodeChar usHalfHandaku    = 0xff9fU; //  半角「゜」
const ModUnicodeChar usFullKataU      = 0x30a6U; // 「ウ」
const ModUnicodeChar usHalfKataU      = 0xff73U; //  半角「ウ」
const ModUnicodeChar usHiraRepeat     = 0x309dU; // 「ゝ」
const ModUnicodeChar usHiraDakuRepeat = 0x309eU; // 「ゞ」
const ModUnicodeChar usKataRepeat     = 0x30fdU; // 「ヽ」
const ModUnicodeChar usKataDakuRepeat = 0x30feU; // 「ヾ」
const ModUnicodeChar usFullLong       = 0x30fcU; // 「ー」
const ModUnicodeChar usHalfLong       = 0xff70U; //  半角「ー」
const ModUnicodeChar usFullHiraA      = 0x3042U; // 「あ」
const ModUnicodeChar usFullHiraN      = 0x3093U; // 「ん」

// 全てのUCS-2文字に対する順序と照合属性のテーブル
const StringKey cBaseKey[] = {
#include "Common/UnicodeCharJapaneseComparisonTable.h"
};

// 長音記号の置き換えのためのテーブル
const ModUnicodeChar usKanaLongKey[48] = {
	0x0118U, 0x0119U, 0x11aU, 0x011bU, 0x011cU, 0x0118U, 0x0119U, 0x011aU,
	0x011bU, 0x011cU, 0x118U, 0x0119U, 0x011aU, 0x011bU, 0x011cU, 0x0118U,
	0x0119U, 0x011aU, 0x11bU, 0x011cU, 0x0118U, 0x0119U, 0x011aU, 0x011bU,
	0x011cU, 0x0118U, 0x119U, 0x011aU, 0x011bU, 0x011cU, 0x0118U, 0x0119U,
	0x011aU, 0x011bU, 0x11cU, 0x0118U, 0x011aU, 0x011cU, 0x0118U, 0x0119U,
	0x011aU, 0x011bU, 0x11cU, 0x0118U, 0x0119U, 0x011bU, 0x011cU, 0x0147U
};

// その他の定数
const ModUnicodeString cstrDaku
	("かきくけこさしすせそたちつてとはひふへほ", 0, LiteralCode);
const ModUnicodeString cstrHandaku("はひふへほ", 0, LiteralCode);
const ModUnicodeChar usAKey = cBaseKey[usFullHiraA].usMainKey;
const ModUnicodeChar usNKey = cBaseKey[usFullHiraN].usMainKey;	 

//
// FUNCTION
// $$$::setKey -- 文字の基幹文字のキーをセットする
//
// NOTES
// 文字が長音、繰り返し記号だった場合、直前の文字によって基幹文字への
// 置き換えを行い、メインキーとサブキーを取得する
// それ以外の場合はその文字自身のメインキーとサブキーを取得する
// 比較の対象にならない文字の場合は比較対象の文字が現れるまで読み込む
//
// ARGUMENTS
// const ModUnicodeChar*&	pString_
//		キーをセットする文字へのポインタ
//		(戻り値) キーをセットした文字の次の文字へのポインタ
// CharKey&					cKey_
//		現在の文字の直前文字のコード、キー
//		(戻り値) セットした文字コードとキー
//				 文字列が終端まで行った時はコード、キーともに0
//
// RETURN
//	なし
//
void
setKey(const ModUnicodeChar*& pString_, CharKey& cKey_)
{
	ModUnicodeChar usAddKey = 0;
	int iFlag = 0;
	ModUnicodeChar usRepflag = 0;
	// 現在の文字についてキーがセットされるまで文字列を読み進む
	while (iFlag == 0) {
		usAddKey = 0;
		switch (*pString_) {
		// 文字が長音記号
		case usFullLong :
			usAddKey = 0x01;
		case usHalfLong :
			usAddKey += 0x04;
			cKey_.usCode = *pString_;
			// 直前文字が仮名の場合はその列によってメインキーの置き換えを行う
			if (cKey_.usMainKey >= usAKey && cKey_.usMainKey <= usNKey) {
				cKey_.usMainKey = usKanaLongKey[cKey_.usMainKey - usAKey];
				cKey_.usSubKey = usAddKey;
			}
			// 直前文字が仮名以外の場合は置き換えを行わない
			else {
				cKey_.usMainKey = cBaseKey[cKey_.usCode].usMainKey;
				cKey_.usSubKey = cBaseKey[cKey_.usCode].usSubKey;
			}
			iFlag = 1;
			break;

		// 繰り返し記号  ひらがな、カタカナの区別はしない
		case usHiraDakuRepeat :
		case usKataDakuRepeat :
			usAddKey = 0x40;
			usRepflag = 0x6f;
			cKey_.usCode = *pString_;
			// 直前の文字が仮名であればそのメインキーに置き換える
			if (cKey_.usMainKey >= usAKey && cKey_.usMainKey <= usNKey) {
				cKey_.usSubKey |= usAddKey;
				cKey_.usSubKey &= usRepflag;
			}
			// 直前文字が仮名以外であれば置き換えを行わない
			else {
				cKey_.usMainKey = cBaseKey[cKey_.usCode].usMainKey;
				cKey_.usSubKey = cBaseKey[cKey_.usCode].usSubKey;
			}
			iFlag = 1;
			break;

		case usHiraRepeat :
		case usKataRepeat :
			usRepflag = 0x2f;
			cKey_.usCode = *pString_;
			// 直前の文字が仮名であればそのメインキーに置き換える
			if (cKey_.usMainKey >= usAKey && cKey_.usMainKey <= usNKey) {
				cKey_.usSubKey &= usRepflag;
			}
			// 直前文字が仮名以外であれば置き換えを行わない
			else {
				cKey_.usMainKey = cBaseKey[cKey_.usCode].usMainKey;
				cKey_.usSubKey = cBaseKey[cKey_.usCode].usSubKey;
			}
			iFlag = 1;
			break;

		// 文字列が終端まで行った場合はキーをすべて0にして戻る
		case 0 :
			cKey_.usCode = 0;
			cKey_.usMainKey = 0;
			cKey_.usSubKey = 0;
			return;
			break;

		// 対象となる文字で置き換えが必要ない場合テーブルのキー値をそのままセット
		default :
			if (cBaseKey[*pString_].usMainKey != 0) {	
				cKey_.usCode = *pString_;
				cKey_.usMainKey = cBaseKey[*pString_].usMainKey;
				cKey_.usSubKey = cBaseKey[*pString_].usSubKey;
				iFlag = 1;
			}
			break;
		}
		// キー値がセットされなかった場合次の文字へ進める
		pString_++;
	}
}

//
// FUNCTION
// $$$::changeMark -- 単体の濁点、半濁点の処理
//
// NOTES
// 単体の濁点、半濁点が出現した場合、直前の文字を見てキー値の設定をする
// 直前文字が濁音、半濁音になりうる場合は直前文字の濁音キーをセットするが、
// それ以外の場合は濁点、半濁点は無視する(比較に影響しない)
//
// ARGUMENTS
// const ModUnicodeChar*&	pString_
//		濁音かどうか判断する文字へのポインタ
//		(戻り値) 濁音であれば次の文字へのポインタ
// CharKey&					pKey_
//		直前文字のコード、キー
//		(戻り値) セットした文字コードとキー
//
// RETURN
//	1	濁点、半濁点の処理終了
//	0	濁点、半濁点の処理をする前に文字列の終端まで進んだ
//
int
changeMark (const ModUnicodeChar*& pString_, CharKey& pKey_)
{
	int iFlag = 0;
	int i;

	while (iFlag == 0) {
		switch (*pString_) {
		// 濁点
		case usFullDaku :
		case usHalfDaku :
			// 直前文字がカタカナ"ウ"の場合は濁音セット
			if ((pKey_.usSubKey & 0xe0) == 0x20  
				&& (pKey_.usCode == usFullKataU || pKey_.usCode == usHalfKataU)) {
				pKey_.usSubKey += 0x40;
			}
			else {
				// 直前文字が濁音のつきうる仮名であれば濁音セット
				for (i = 0; cstrDaku[i] != '\0'; i++) {
					if (pKey_.usMainKey == cBaseKey[cstrDaku[i]].usMainKey && (pKey_.usSubKey & 0xe0) == 0x20) {
						pKey_.usSubKey += 0x40;
						break;
					}
				}
			}
			//- iFlag = 1; これがあると「ガ」<「カ゛」になってしまう
			pString_++;
			break;

		case usFullHandaku :
		case usHalfHandaku :
			// 直前文字が半濁音のつきうる仮名であれば半濁音セット
			for (i = 0; cstrHandaku[i] != '\0'; i++) {
				if (pKey_.usMainKey == cBaseKey[cstrHandaku[i]].usMainKey && (pKey_.usSubKey & 0xe0) == 0x20) {
					pKey_.usSubKey += 0x80;
					break;
				}
			}
			//- iFlag = 1; これがあると「ぱ」<「は゜」になってしまう
			pString_++;
			break;

		// サブキーをセットする前に文字列が終端まで行った場合は呼び出し元へ戻る
		case 0 :
			return(0);
			break;

		default :
			// 今の文字が濁点でも半濁点でもなかったら何もせずにそのままかえる
			if (cBaseKey[*pString_].usMainKey != 0) 
				iFlag = 1;
			else 
				pString_++;
			break;
		}
	}
	return(1);
}

}

//
// FUNCTION
// Common::UnicodeString::compare -- UCS-2 日本語文字列の比較
//
// NOTES
// JIS X 4061規格に基づいて日本語文字列の比較を行う
// 対象とするのはUCS-2文字列
// 規格に含まれない部分として、文字の全角、半角でも比較を行う
//
// ARGUMENTS
// const ModUnicodeChar*	pusString1_
// const ModUnicodeChar*	pusString2_
//		比較を行う文字列
//
// RETURN
// int
//		pusString1_の方が大きければ1、小さければ-1、二つの文字列が
//		等しければ0を返す
//
int 
UnicodeString::compare(const ModUnicodeChar* pusString1_, 
	const ModUnicodeChar* pusString2_)
{
	CharKey usChar1 = {0, 0, 0};
	CharKey usChar2 = {0, 0, 0};

	int iSubresult1 = 0;	// 第1照合属性
	int iSubresult2 = 0;	// 第2照合属性
	int iSubresult3 = 0;	// 第3照合属性
	int iSubresult4 = 0;	// 第4照合属性(全角半角)

	//- 初期値は当初0だったが、それだと片方が""のときに誤るので
	//- 1に改めた。
	int iPoint1 = 1;		// サブキーセットフラグ
	int iPoint2 = 1;		

	while (1) {
		// 現在の文字のキーを設定する
		setKey(pusString1_, usChar1);
		setKey(pusString2_, usChar2);
		
#if 0 //けっこううっとうしい
		SydDebugMessage << ModHex
			<< static_cast<int>(usChar1.usMainKey) << " vs "
			<< static_cast<int>(usChar2.usMainKey) << ", "
			<< ModDec << iPoint1 << " vs "
			          << iPoint2 << ModEndl;
#endif

		// メインのキーのみで大小が判断つけばその値を返す
		// どちらかの文字列でキーの設定が文字列の終端まで終わった場合は
		// ループを抜ける
		if (usChar1.usMainKey == 0 || usChar2.usMainKey == 0) break;
		if      (usChar1.usMainKey > usChar2.usMainKey)  return (1);
		else if (usChar1.usMainKey < usChar2.usMainKey)  return(-1);
		
		// 二つの文字のメインキーが等しく、
		// かつここまでにサブキーで大小が判断つかない場合、
		// サブキーを評価する
		// 現在の文字の次の文字を見て、現在のサブキーを求める

		if (iSubresult1 == 0) {
			iPoint1 = changeMark(pusString1_, usChar1);
			iPoint2 = changeMark(pusString2_, usChar2);

#if 0
			SydDebugMessage << ModHex
				<< "Sub1 : " << static_cast<int>(usChar1.usSubKey) << ", "
				<< "Sub2 : " << static_cast<int>(usChar2.usSubKey) << ", "
				<< ModDec << "iPoint1 : " << iPoint1 << ", " 
				          << "iPoint2 : " << iPoint2 << ModEndl;
#endif

			// サブキーの判定は四段階によって行われる
			// 文字列全体を通して段階が上の比較が優先
			if      ((usChar1.usSubKey >> 6) > (usChar2.usSubKey >> 6) )
				iSubresult1 = 1;
			else if ((usChar1.usSubKey >> 6) < (usChar2.usSubKey >> 6))
				iSubresult1 = -1;

			else if (iSubresult2 == 0 && ((usChar1.usSubKey >> 4) > (usChar2.usSubKey >> 4)))
				iSubresult2 = 1;
			else if (iSubresult2 == 0 && ((usChar1.usSubKey >> 4) < (usChar2.usSubKey >> 4)))
				iSubresult2 = -1;

			else if (iSubresult3 == 0 && (usChar1.usSubKey >> 3 > usChar2.usSubKey >> 3))
				iSubresult3 = 1;
			else if (iSubresult3 == 0 && (usChar1.usSubKey >> 3 < usChar2.usSubKey >> 3)) 
				iSubresult3 = -1;

			else if (iSubresult4 == 0 && (usChar1.usSubKey > usChar2.usSubKey))
				iSubresult4 = 1;
			else if (iSubresult4 == 0 && (usChar1.usSubKey < usChar2.usSubKey))
				iSubresult4 = -1;
			// どちらかの文字列が終端まで行った場合はループを抜ける
			if (iPoint1 == 0 || iPoint2 == 0)
				break;
		}
	}

	// for文を抜けた場合、どちらかの文字列が終端に達したか、
	// 二つの文字列のメインキーが全く同じ
	// メインキーが同じ場合はサブキーの値を返す
	if ((usChar1.usMainKey == 0 && usChar2.usMainKey == 0)
		|| (iPoint1 == 0 && iPoint2 == 0)) {

#if 0
		SydDebugMessage << iSubresult1 << " " << iSubresult2 << " " 
			            << iSubresult3 << " " << iSubresult4 << ModEndl;
#endif

		if      (iSubresult1 != 0) 
			return(iSubresult1);
		else if (iSubresult2 != 0)
			return(iSubresult2);
		else if (iSubresult3 != 0)
			return(iSubresult3);
		else
			return (iSubresult4);
	}
	else if (usChar1.usMainKey == 0 || iPoint1 == 0) 
		return(-1);
	else if (usChar2.usMainKey == 0 || iPoint2 == 0) 
		return (1);
	else
		return (0);
}
#endif

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
