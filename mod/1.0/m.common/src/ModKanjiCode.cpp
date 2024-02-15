// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModKanjiCode.cpp -- ModKanjiCode のメンバ定義
// 
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
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

// PARTIAL_SURROGATE_PAIR_SUPPORT
//   コンパイル時に-DPARTIAL_SURROGATE_PAIR_SUPPORTを指定すると、
//   基本多言語面(第0面, U+0000～U+FFFF)に含まれないUnicode文字
//   (第1面～第16面, U+10000～U+10FFFF)を、サロゲートペア相当の
//   2文字のModUnicodeChar(以下サロゲートペアと呼ぶ)として扱う。
//
// 仕様と制約事項
// ・UTF8->UCS2の変換では、4バイトのUTF8文字をサロゲートペアに
//   相当する2文字のModUnicodeCharに変換する。
//   UTF8の先頭バイトで残りのバイト数を判定しており、変換元に
//   対応するバイト列が揃っているかどうかはチェックしていない。
//   これは高速化のためと思われ、当初からの仕様であることもあり
//   変更していない。
// ・UCS2->UTF8の変換では、サロゲートペアに相当する2文字の
//   ModUnicodeCharを4バイトのUTF8文字に変換する。
//   サロゲートペアの片割れ(前半のみ, 後半のみ)が出現した場合、
//   REPLACEMENT CHARACTER(U+FFFD, 0xef 0xbf 0xbd)に変換する。
// ・UTF8またはUCS2からEUCやSJISへの変換では、サロゲートペアに
//   相当する2文字のModUnicodeCharを2文字のゲタに変換する。
//   サロゲートペアの片割れ(前半のみ, 後半のみ)が出現した場合、
//   その文字を1文字のゲタに変換する。
// ・サロゲートペアを構成する各ModUnicodeCharは、基本多言語面の
//   どの文字とも異なる値をもち、また互いにも異なる値をもつため、
//   文字列検索においてfalse matchを起こすことはない。
// ・サロゲートペアを構成する各ModUnicodeCharは、仕様上は通常の
//   ModUnicodeCharと区別されず、正規表現検索ではおのおのが'.'と
//   マッチし、また部分文字列の切り出しによりサロゲートペアを
//   構成する2文字が分離することがある。
//

#include "ModCommon.h"
#include "ModKanjiCode.h"
#include "ModTrait.h"
#include "ModMessage.h"
#include "ModAssert.h"
#include "ModError.h"

//
// CONST
// ModKanjiCode::literalCode -- 文字列リテラルの漢字コード
//
// NOTES
// この定数は文字列リテラルに用いられる漢字コードを表す。
//
const ModKanjiCode::KanjiCodeType   ModKanjiCode::literalCode =
#if MOD_CONF_LITERAL_CODE == 0
	ModKanjiCode::euc
#endif
#if MOD_CONF_LITERAL_CODE == 1
	ModKanjiCode::shiftJis
#endif
#if MOD_CONF_LITERAL_CODE == 2
	ModKanjiCode::utf8
#endif
;

//
// VARIABLE
// _ByteUTF8 -- utf8の先頭１バイトで何バイトになるか
//
// NOTES
// 0のところはUCS2の範囲のutf8の先頭バイトには現れない数字
// PARTIAL_SURROGATE_PAIR_SUPPORTが有効な場合、
// 先頭バイトが0xf0～0xf7のときはutf8が4バイトの長さをもつ
//
char _ByteUTF8[256] = {
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,

0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,	4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0
#else
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
#endif
};

//
// VARIABLE
// _MaskUTF8 -- UTF8の先頭バイトの文字コード領域。要素0は先頭バイト以外部分
//
unsigned char _MaskUTF8[4] = {
	0x3f, 0x7f, 0x1f, 0x0f
};

//
// VARIABLE
// _MarkUTF8 -- UTF8の先頭バイトに付加する値
//
unsigned char _MarkUTF8[4] = {
	0x00, 0x00, 0xc0, 0xe0
};

//
// VARIABLE
// _OffUTF8 -- UTF8->UCS2の時に、余計な数を削除するための数字
//
ModUnicodeChar _OffUTF8[] = {
	0x0000, 0x0000, 0x3080, 0x2080
};

//
// VARIABLE
// _euc2unicode -- EUC16 -> UCS2の変換テーブル
//
unsigned short _euc2unicode[] = {
#include "euc2unicode.tbl"
};

//
// VARIABLE
// _unicode2euc -- UCS2 -> EUC16の変換テーブル
//
unsigned short _unicode2euc[] = {
#include "unicode2euc.tbl"
};

//
// VARIABLE
// _SS2, _SS3, _ESC
//
unsigned char _ESC = 0x1b;
unsigned char _SS2 = 0x8e;
unsigned char _SS3 = 0x8f;

//
// VARIABLE
// _EUC16_GETA, _UCS2_GETA
//
unsigned short _EUC16_GETA = 0xa2ae;
unsigned short _UCS2_GETA  = 0x3013;

//
// VARIABLE
// _Kanji, _Hankana, _None
//
enum _JisMode { _Kanji, _Hankana, _None };

//
// FUNCTION pubblic
// ModKanjiCode::getCharacterSize -- １文字のバイト数
//
// NOTES
// 各種漢字コード(特にマルチバイト文字)の1文字のバイト数を求める。
// 引数 character には文字の先頭 1 バイトを渡さなければいけない。
// 先頭以外を渡した場合の動作は未定義。
//
// （注意）
// このメソッドで求めたバイト数と jj が実際に必要としたバイト数が
// 異なっていた場合でも、ほとんどの場合はエラーが検出できないであろう。
//
// ARGUMENTS
// const unsigned char	c
//		文字の先頭バイト
// const KanjiCodeType	codeType
//		漢字コード
// 
// RETURN
// 各種漢字コードの１文字のバイト数(このメソッドに渡された先頭バイトも含む)
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		渡された漢字コードには対応していない
//
ModSize
ModKanjiCode::getCharacterSize(const unsigned char		c,
							   const KanjiCodeType		codeType)
{
	// !! 既存のソースコードには「0x8e」「0x8f」などの値が直接埋め込まれて
	// !! いる。このメソッドも習慣に従って即値を埋め込む。
	ModSize characterSize = 0;	// 1文字のバイト数(渡された1バイトを含む)

	switch(codeType) {
	case euc:
	case jis:
	{
		if (!(c & 0x80)) { // 英数字
			characterSize = 1;
		} else if (c == 0x8e) { // SS2
			characterSize = 2;
		} else if (c == 0x8f) { // SS3
			characterSize = 3;
		} else { // SS1
			characterSize = 2;
		}
	}
	break;
	case shiftJis:
	{
		if (c >= 161 && c <= 223) { // 半角カタカナ
			characterSize = 1;
		} else if (c & 0x80) { // 漢字
			characterSize = 2;
		} else { // 英数字
			characterSize = 1;
		}
	}
	break;
	case utf8:
		// テーブルを引いて調べる
		characterSize = _ByteUTF8[c];
	break;
	case ucs2:
		// UCS-2 は固定長なので、渡された character を調べていない
		characterSize = sizeof(ModUnicodeChar);
		break;
	default:
		// サポートしていない漢字コードが渡された
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	// 1文字のバイト数は1以上(バイト数には渡された１バイトも含まれているから)
	; ModAssert(characterSize > 0);
	return characterSize;
}

//
// FUNCTION private
// ModKanjiCode::getChar -- char* を一文字分ModWideCharに変換する
//
// NOTES
// この関数はchar*で与えられた特定の漢字コードによる文字列の先頭文字を
// ModWideChar に変換するのに用いる。
//
// ARGUMENTS
// const char*& source
//		変換する文字を指すポインタ。
//		変換が成功したら次の文字を指すように変更される。
// ModKanjiCode::KanjiCodeType inCode
//		sourceの漢字コード
// int& mode
//		処理中の状態を表す変数。static にしたいところを MT 環境に配慮して
//		引数にした。
//
// RETURN
// ModWideCharに変換したものを返す。
// 変換に失敗した場合は 0 を返す。
//
// EXCEPTIONS
// なし
//
inline ModWideChar
ModKanjiCode::getChar(const char*& source, KanjiCodeType inCode, int& mode)
{
	if (source == 0 || *source == ModCharTrait::null()) {
		return ModWideCharTrait::null();
	}

	ModWideChar wideChar;
	int c1;
	int c2;
	char string[4];

	switch (inCode) {
	case euc:
		wideChar = ModWideCharTrait::makeWideChar(source, inCode);
		source += ModWideCharTrait::byteSize(source, inCode);
		return wideChar;
	case jis:
	case shiftJis:
		while (*source != 0) {
			c1 = (int)((unsigned char)(*source));
			source++;
			switch (c1) {
			case 0x0e:					// JIS7 半角カナ SO
				mode |= kana;
				continue;
			case 0x0f:					// JIS7 半角カナ SI
				mode &= ~kana;
				continue;
			case 0x1b:					// エスケープ
				c2 = (int)((unsigned char)(*source));
				source++;
				if (c2 == 0) {
					string[0] = (char)0xa2;
					string[1] = (char)0xae;
					string[2] = 0;
					return ModWideCharTrait::makeWideChar(string,
														  ModKanjiCode::euc);
				}
				if (c2 == '$') {		// KanjiIN
					// 続く一文字を読み飛ばす
					source++;
					mode |= kanji;		// KanjiIN フラグセット
					mode &= ~kana;		// カナモードリセット
					continue;
				}
				if (c2 == '(') {		// KanjiOUT
					if (*source == 'I') {
						// 半角カナ
						mode |= (kana | kanji);
						source++;
						continue;
					}
					// 続く一文字を読み飛ばす
					source++;
					mode &= ~kanji;		// KanjiIN フラグリセット
					continue;
				}
				ModMessage << "Unknown escape sequence" << ModEndl;
				continue;
			default:
				break;
			}
			if (c1 >= 0x80) {
				if (c1 >= 0xa0 && c1 < 0xe0) {
					// JIS8 半角カナ
					string[0] = (char)0x8e;
					string[1] = (char)c1;
					string[2] = 0;
					return ModWideCharTrait::makeWideChar(string,
														  ModKanjiCode::euc);
				}
				c2 = (int)((unsigned char)(*source));
				source++;
				if (c2 == 0) {
					ModMessage << "Incomplete 2byte code" << ModEndl;
					string[0] = (char)0xa2;
					string[1] = (char)0xae;
					string[2] = 0;
					return ModWideCharTrait::makeWideChar(string,
														  ModKanjiCode::euc);
				}
				if (c2 < 0x40 || c2 == 0x7f || c2 > 0xfc) {
					ModMessage << "Bad shift jis" << ModEndl;
					string[0] = (char)0xa2;
					string[1] = (char)0xae;
					string[2] = 0;
					return ModWideCharTrait::makeWideChar(string,
														  ModKanjiCode::euc);
				}
				if (c1 >= 0xf0) {
					ModMessage << "Gaiji" << ModEndl;
					string[0] = (char)0xa2;
					string[1] = (char)0xae;
					string[2] = 0;
					return ModWideCharTrait::makeWideChar(string,
														  ModKanjiCode::euc);
				}
				if (c1 > 0x9f) {
					c1 -= 0x40;
				}
				c1 += c1;
				if (c2 <= 0x9e) {
					c1 -= 0xe1;
					if (c2 >= 0x80) {
						c2--;
					}
					c2 -= 0x1f;
				} else {
					c1 -= 0xe0;
					c2 -= 0x7e;
				}
				string[0] = (char)(c1 | 0x80);
				string[1] = (char)(c2 | 0x80);
				string[2] = 0;
				return ModWideCharTrait::makeWideChar(string,
													  ModKanjiCode::euc);
			}
			if (mode == 0 || c1 <= 0x20 || c1 == 0x7f) {
				return (ModWideChar)c1;
			}
			if ((mode & kana) != 0) {
				// JIS7 半角カナ
				if (c1 >= 0x60) {
					ModMessage << "Bad kana" << ModEndl;
					string[0] = (char)0xa2;
					string[1] = (char)0xae;
					string[2] = 0;
					return ModWideCharTrait::makeWideChar(string,
														  ModKanjiCode::euc);
				}
				string[0] = (char)0x8e;
				string[1] = (char)(c1 | 0x80);
				string[2] = 0;
				return ModWideCharTrait::makeWideChar(string,
													  ModKanjiCode::euc);
			}
			c2 = (int)((unsigned char)(*source));
			source++;
			if (c2 == 0) {
				ModMessage << "Incomplete 2byte code" << ModEndl;
				string[0] = (char)0xa2;
				string[1] = (char)0xae;
				string[2] = 0;
				return ModWideCharTrait::makeWideChar(string,
													  ModKanjiCode::euc);
			}
			if (c2 <= 0x20 || c2 >= 0x7f) {
				ModMessage << "Bad kanji" << ModEndl;
				string[0] = (char)0xa2;
				string[1] = (char)0xae;
				string[2] = 0;
				return ModWideCharTrait::makeWideChar(string,
													  ModKanjiCode::euc);
			}
			string[0] = (char)(c1 | 0x80);
			string[1] = (char)(c2 | 0x80);
			string[2] = 0;
			return ModWideCharTrait::makeWideChar(string, ModKanjiCode::euc);
		}
	default:
		break;
	}
	return 0;
}

//
// FUNCTION private
// ModKanjiCode::putChar -- ModWideChar を char* に変換する
//
// NOTES
// この関数は ModWideChar で表される文字を char* の配列に特定の
// 漢字コードで格納するために用いる。
//
// ARGUMENTS
// char*& destination
//		変換した結果を入れるアドレスへの参照。
//		格納が成功した場合、値が次の文字を入れるべき位置に変更される。
// ModWideChar wideChar
//		変換するワイド文字
// const char* last
//		領域の最後を指すポインタ
// KanjiCodeType outCode
//		出力に使う漢字コード
// int& mode
//		処理中の状態を表す変数。static にしたいところを MT 環境に配慮して
//		引数にした。
//
// RETURN
// 正しく格納できた場合は ModTrue を返し、
// 領域が終了、またはwideCharが0であった場合は ModFalseを返す。
//
// EXCEPTIONS
// なし
//
inline ModBoolean
ModKanjiCode::putChar(char*& destination, ModWideChar wideChar,
					  const char* last, KanjiCodeType outCode, int& mode)
{
	//
	// null が渡されたら無条件に ModFalse を返す
	//
	if (wideChar == ModWideCharTrait::null()) {
		*destination = 0;
		return ModFalse;
	}

	char string[4];
	int length;

	if (outCode == euc || outCode == shiftJis) {
		// EUC/ShiftJis なら直接そのコードの文字列に変換する
		length = ModWideCharTrait::convertToString(string, wideChar, outCode);
		string[length] = 0;
	} else {
		ModAssert(outCode == jis)
		// Jis ならまず EUC に変換してから変換する
		length = ModWideCharTrait::convertToString(string, wideChar, euc);
		string[length] = 0;

		if ((unsigned char)string[0] == 0x8f) {	// SS3 -> 〓にする
			string[0] = (char)0xa2;
			string[1] = (char)0xae;
			string[2] = 0;
			length = 2;
		}
		if ((unsigned char)string[0] < 0x80) { // ASCII
			if (mode != 0) {
				// KanjiOut
				*destination++ = '\033';
				*destination++ = '(';
				*destination++ = 'B';
				mode = 0;
			}
		} else if ((unsigned char)string[0] == 0x8e) {	// 半角カナ
			if (mode != kana) {
				// KanjiIn、KanjiOut の分が残っているか
				if (destination + 3 * 2 + 1 >= last) {
					// 十分な領域が残っていないのであきらめる
					if (mode == kanji) {
						*destination++ = '\033';
						*destination++ = '(';
						*destination++ = 'B';
						mode = 0;
					}
					*destination = 0;
					return ModFalse;
				}
				*destination++ = '\033';
				*destination++ = '(';
				*destination++ = 'I';
				mode = kana;
			}
			string[0] = ((unsigned char)string[1]) & 0x7f;
			string[1] = 0;
			length = 1;
		} else {
			if (mode != kanji) {		// 2バイトコード
				// KanjiIn、KanjiOut の分が残っているか
				if (destination + 3 * 2 + 1 >= last) {
					// 十分な領域が残っていないのであきらめる
					if (mode == kana) {
						// カナモードのときはKanjiOutしておく
						*destination++ = '\033';
						*destination++ = '(';
						*destination++ = 'B';
						mode = 0;
					}
					*destination = 0;
					return ModFalse;
				}
				// KanjiIn
				*destination++ = '\033';
				*destination++ = '$';
				*destination++ = 'B';
				mode = kanji;
			}
			string[0] &= 0x7f;
			string[1] &= 0x7f;
		}
	}

	if ((mode & kanji) != 0) {
		// KanjiOutの分が残ってるか
		if (destination + length + 3 >= last) {
			*destination++ = '\033';
			*destination++ = '(';
			*destination++ = 'B';
			*destination = 0;
			mode = 0;
			return ModFalse;
		}
	}
	if (destination + length >= last) {
		// 超えてしまうので終りにする
		*destination = 0;
		return ModFalse;
	}

	ModOsDriver::Memory::copy(destination, string, length);
	destination += length;

	return ModTrue;
}

//
// FUNCTION private
// ModKanjiCode::transfer -- 漢字コードの変換
//
// NOTES
// この関数はchar*で与えられた文字列の漢字コードを変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
// ModKanjiCode::KanjiCodeType inCode
//		入力文字列の漢字コード
// ModKanjiCode::KanjiCodeType outCode
//		出力文字列の漢字コード
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		inCode に渡された漢字コード種類には対応していない
//
const char*
ModKanjiCode::transfer(char* destination, const char* source,
					   ModSize sizeOfDestination,
					   KanjiCodeType inCode, KanjiCodeType outCode)
{
	if (inCode != euc && inCode != jis && inCode != shiftJis) {
		// サポートしていない文字コードが渡されたので例外送出
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	int inMode = 0;
	int outMode = 0;
	char* last = destination + sizeOfDestination - 1;
	ModBoolean result = ModTrue;

	//
	// putChar が失敗するまでgetCharとputCharを繰り返す。
	// source、destinationはgetChar、putCharで変更される。
	// destination が last に達するか、0 をputCharすると result が
	// ModFalse になる。
	// 
	const char* returnValue;
	while (result == ModTrue) {
		returnValue = source;
		ModWideChar wideChar = ModKanjiCode::getChar(source, inCode, inMode);
		result = ModKanjiCode::putChar(destination, wideChar, last, outCode,
									   outMode);
	}
	if (outCode == jis && outMode != 0) {
		*destination++ = '\033';
		*destination++ = '(';
		*destination++ = 'B';
		*destination = 0;
	}
	return returnValue;
}

//
// FUNCTION private
// ModKanjiCode::transfer -- ワイド文字からの漢字コードの変換
//
// NOTES
// この関数はModWideChar*で与えられた文字列をchar*に特定の漢字コードで
// 変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const ModWideChar* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
// ModKanjiCode::KanjiCodeType outCode
//		出力文字列の漢字コード
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		outCode に渡された漢字コード種類には対応していない
//
const ModWideChar*
ModKanjiCode::transfer(char* destination, const ModWideChar* source,
					   ModSize sizeOfDestination,
					   KanjiCodeType outCode)
{
	if (outCode != euc && outCode != jis && outCode != shiftJis) {
		// サポートしていない文字コードが渡されたので例外送出
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	int outMode = 0;
	char* last = destination + sizeOfDestination - 1;
	ModBoolean result = ModTrue;

	//
	// putChar が失敗するまでputCharを繰り返す。
	// destinationはputCharで変更される。
	// destination が last に達するか、0 をputCharすると result が
	// ModFalse になる。
	// 
	const ModWideChar* returnValue;
	const ModWideChar* wp = source;
	while (result == ModTrue) {
		returnValue = wp;
		result = ModKanjiCode::putChar(destination, *wp, last, outCode,
									   outMode);
		wp++;
	}
	if (outCode == jis && outMode != 0) {
		*destination++ = '\033';
		*destination++ = '(';
		*destination++ = 'B';
		*destination = 0;
	}
	return returnValue;
}

//
// FUNCTION private
// ModKanjiCode::transfer -- ワイド文字列への変換
//
// NOTES
// この関数はchar*で与えられた特定の漢字コードによる文字列を
// ワイド文字列に変換するのに用いる。
//
// ARGUMENTS
// ModWideChar* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
// ModKanjiCode::KanjiCodeType inCode
//		入力文字列の漢字コード
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		inCode に渡された漢字コード種類には対応していない
//
const char*
ModKanjiCode::transfer(ModWideChar* destination, const char* source,
					   ModSize sizeOfDestination,
					   KanjiCodeType inCode)
{
	if (inCode != euc && inCode != jis && inCode != shiftJis) {
		// サポートしていない文字コードが渡されたので例外送出
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	int inMode = 0;
	ModWideChar* last = destination + sizeOfDestination - 1;
	ModBoolean result = ModTrue;

	//
	// getChar が 0 を返すか、wp が last に到達するまで getChar を繰り返す。
	// getChar の返り値を *wp に代入していく
	// 
	ModWideChar* wp = destination;
	const char* returnValue = source;
	while (wp < last) {
		ModWideChar wideChar = ModKanjiCode::getChar(source, inCode, inMode);
		if (wideChar == ModWideCharTrait::null()) {
			break;
		}
		*wp++ = wideChar;
		returnValue = source;
	}
	// null-terminate させる
	*wp = ModWideCharTrait::null();

	return returnValue;
}

////////// 内部クラス JJTransfer ///////////

//
// FUNCTION public
// ModKanjiCode::JJTransfer::transfer -- jj ライブラリを使ってコード変換
//
// NOTES
// MT-safe な jj ライブラリを使ってコード変換する。
//
// inString の末尾以外の位置に nul 文字が現れた場合、その nul 以降の
// 変換を行なうか否かは未定義(jj の動作に依存)。
//
// (注意)
// 引数の前半は「変換先」に関する情報で、後半は「変換元」に関する情報である。
// 引数を渡す時に間違えないように注意が必要。
//
// ARGUMENTS
// char*				outString
//		変換先文字列の領域へのポインタ。領域は呼出側で確保すること。
//		char* 以外の型の文字列を渡す時はキャストが必要。
// const int			outBytes
//		outString に渡した領域のバイト数
// const KanjiCodeType	outType
//		変換先文字列の文字コード
// const char*			inString
//		変換元文字列(終端文字で終っていること)。
//		char* 以外の型の文字列を渡す時はキャストが必要。
// const KanjiCodeType	inType
//		変換元文字列の文字コード
// 
// RETURN
// outString を返す
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		inType, outType に渡されたコードが未対応なものであった
//
char*
ModKanjiCode::JJTransfer::transfer(char*				outString,
								   const int			outBytes,
								   const KanjiCodeType	outType,
								   const char*			inString,
								   const KanjiCodeType	inType)
{
	// 引数検査
	if (outString == 0 || inString == 0) {
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);
	}

	if ((inType == ucs2 && outType == utf8)
		|| (inType == utf8 && outType == ucs2))
	{
		// UNICODE間の変換は高速化のため自作関数で行う
		return (char*)transferForUnicode(
			(unsigned char*)outString,
			outBytes,
			outType,
			(const unsigned char*)inString,
			inType);
	}

	// 前はG-BASEのjjライブラリを利用していたが、64ビット化できないので
	// 今はここも自作になっている

	const unsigned char* s = (const unsigned char*)inString;
	unsigned char* d = (unsigned char*)outString;
	unsigned short c;

	int outCount = 0;

	_JisMode inMode = _None;
	_JisMode outMode = _None;

	for (;;) {
		
		c = 0;
		
		switch (inType) {
		case euc:
			if (*s == _SS2) {
				// 半角カナ
				++s;
				c = *s++;
			} else if (*s == _SS3) {
				// 外字
				++s;
				c = (*s++ << 8);
				c |= (*s++ & 0x7f);
			} else if (*s & 0x80) {
				// 漢字
				c = (*s++ << 8);
				c |= *s++;
			} else {
				// ASCII
				c = *s++;
			}
			break;

		case jis:
			while (*s == _ESC) {
				if (*(s+1) == '$') {
					if (*(s+2) == 'B') {
						// 漢字モード
						inMode = _Kanji;
						s += 3;
					} else {
						break;
					}
				} else if (*(s+1) == '(') {
					if (*(s+2) == 'B') {
						// モード解除
						inMode = _None;
						s += 3;
					} else if (*(s+2) == 'I') {
						// 半角カナモード
						inMode = _Hankana;
						s += 3;
					} else {
						break;
					}
				} else {
					break;
				}
			}
			
			switch (inMode) {
			case _Kanji:
				c = (*s++ << 8);
				c |= *s++;
				c |= 0x8080;
				break;
			case _Hankana:
				c = (*s++ | 0x80);
				break;
			case _None:
				c = *s++;
				break;
			}
			break;
			
		case shiftJis:
			if (*s >= 0x81 && *s <= 0x9f || *s >= 0xe0 && *s <= 0xfc) {
				// 漢字
				unsigned char c1 = *s++;
				unsigned char c2 = *s++;
				if (c1 > 0x9f)
					c1 -= 0x40;
				c1 += c1;
				if (c2 <= 0x9e) {
					c1 -= 0xe1;
					if (c2 >= 0x80)
						c2 -= 1;
					c2 -= 0x1f;
				} else {
					c1 -= 0xe0;
					c2 -= 0x7e;
				}
				c = ((c1 << 8) | c2 | 0x8080);
			} else if (*s & 0x80) {
				// 半角カナ
				c = *s++;
			} else {
				// ASCII
				c = *s++;
			}
			break;
			
		case utf8:
			{
				int size = _ByteUTF8[*s];
				c = *s++;
				switch (size)
				{
				case 3: c <<= 6; c += *s++;
				case 2: c <<= 6; c += *s++;
				default:
					break;
				}
				c -= _OffUTF8[size];
			}
			break;
			
		case ucs2:
			ModOsDriver::Memory::copy(&c, s, sizeof(unsigned short));
			s += sizeof(unsigned short);
			break;
			
		}

		if (c != 0) {
			
			if ((inType == euc || inType == jis || inType == shiftJis) &&
				(outType == utf8 || outType == ucs2)) {

				// EUC16 -> UCS2 の変換を行う
				c = _euc2unicode[c];
				if (c == 0) c = _UCS2_GETA;

			} else if ((inType == utf8 || inType == ucs2) &&
					   (outType == euc || outType == jis ||
						outType == shiftJis)) {

				// UCS2 -> EUC16 の変換を行う
				// サロゲートペアはゲタ2文字になる
				// サロゲートペアの片割れだけが出現した場合はゲタ1文字になる
				c = _unicode2euc[c];
				if (c == 0) c = _EUC16_GETA;
			
			}
		}

		switch (outType) {
		case euc:
			switch (c & 0x8080) {
			case 0x0000:
				// ASCII
				outCount++;
				*d++ = (unsigned char)c;
				break;
				
			case 0x0080:
				// 半角カナ
				outCount += 2;
				if (outCount > outBytes) break;
				*d++ = _SS2;
				*d++ = (unsigned char)c;
				break;
				
			case 0x8000:
				// 外字
				outCount += 3;
				if (outCount > outBytes) break;
				*d++ = _SS3;
				*d++ = (unsigned char)(c >> 8);
				*d++ = (unsigned char)(c & 0xff | 0x80);
				break;
				
			case 0x8080:
				// 漢字
				outCount += 2;
				if (outCount > outBytes) break;
				*d++ = (unsigned char)(c >> 8);
				*d++ = (unsigned char)(c & 0xff);
				break;
				
			}
			break;
			
		case jis:
			switch (c & 0x8080) {
			case 0x0000:
				// ASCII
				switch (outMode) {
				case _Kanji:
				case _Hankana:
					outCount += 4;
					if (outCount > outBytes) break;
					*d++ = (unsigned char)_ESC;
					*d++ = (unsigned char)'(';
					*d++ = (unsigned char)'B';
					*d++ = (unsigned char)c;
					break;
				case _None:
				default:
					outCount++;
					*d++ = (unsigned char)c;
					break;
				}
				outMode = _None;
				break;
				
			case 0x0080:
				// 半角カナ
				switch (outMode) {
				case _Kanji:
				case _None:
				default:
					outCount += 4;
					if ((outCount + 3) > outBytes) break;
					*d++ = (unsigned char)_ESC;
					*d++ = (unsigned char)'(';
					*d++ = (unsigned char)'I';
					*d++ = (unsigned char)(c & 0x7f);
					break;
				case _Hankana:
					outCount++;
					if ((outCount + 3) > outBytes) break;
					*d++ = (unsigned char)(c & 0x7f);
					break;
				}
				outMode = _Hankana;
				break;
				
			case 0x8000:
			case 0x8080:
				// 漢字と外字(EUC外字も漢字にマップする)
				switch (outMode) {
				case _Kanji:
					outCount += 2;
					if ((outCount + 3) > outBytes) break;
					*d++ = (unsigned char)((c >> 8) & 0x7f);
					*d++ = (unsigned char)(c & 0x7f);
					break;
				case _Hankana:
				case _None:
				default:
					outCount += 5;
					if ((outCount + 3) > outBytes) break;
					*d++ = (unsigned char)_ESC;
					*d++ = (unsigned char)'$';
					*d++ = (unsigned char)'B';
					*d++ = (unsigned char)((c >> 8) & 0x7f);
					*d++ = (unsigned char)(c & 0x7f);
					break;
				}
				outMode = _Kanji;
				break;
				
			}
			break;
			
		case shiftJis:
			switch (c & 0x8080) {
			case 0x0000:
				// ASCII
				outCount++;
				*d++ = (unsigned char)c;
				break;
				
			case 0x0080:
				// 半角カナ
				outCount++;
				*d++ = (unsigned char)c;
				break;
				
			case 0x8000:
			case 0x8080:
				// 漢字と外字(EUC外字も漢字にマップする)
				{
					outCount += 2;
					if (outCount > outBytes) break;
					unsigned short c2 = c & 0x7f7f;
					unsigned short c1 = ((c2 >> 8) + 0xe1) >> 1;
					if (c1 > 0x9f)
						c1 += 0x40;
					if ((c2 & 0x100) == 0) {
						c2 &= 0xff;
						c2 += 0x7e;
					} else {
						c2 &= 0xff;
						c2 += 0x1f;
						if (c2 > 0x7e)
							c2 += 1;
					}
					*d++ = (unsigned char)c1;
					*d++ = (unsigned char)c2;
				}
				break;
			}
			break;
			
		case utf8:
			{
				int size;
				if (c < 0x80)		size = 1;
				else if (c < 0x800) size = 2;
				else				size = 3;
				outCount += size;
				if (outCount > outBytes) break;
				d += size;
				switch (size) {
				case 3: *--d = ((c & 0x3f) | 0x80); c >>= 6;
				case 2: *--d = ((c & 0x3f) | 0x80); c >>= 6;
				case 1: *--d = (c | _MarkUTF8[size]);
				}
				d += size;
			}
			break;
				
		case ucs2:
			outCount += 2;
			if (outCount > outBytes) break;
			ModOsDriver::Memory::copy(d, &c, sizeof(unsigned short));
			d += 2;
			break;
			
		}

		if (c == 0 || outCount >= outBytes) break;
	}

	if (outType == jis) {

		switch (outMode) {
		case _Kanji:
		case _Hankana:
			*d++ = (unsigned char)_ESC;
			*d++ = (unsigned char)'(';
			*d++ = (unsigned char)'B';
			break;
		case _None:
		default:
			break;
		}
		
	}
	
	return outString;
}

//
// FUNCTION public
// ModKanjiCode::JJTransfer::getTransferredSize -- jj ライブラリを使って各種漢字コードの変換後のバイト数を取得
//
// NOTES
// MT-safe な jj ライブラリの jjStrnlen を呼び出しているだけである。
// inString は終端文字で終る文字列でなければならない。
//
// (注意)
// 引数の前半は「変換先」に関する情報で、後半は「変換元」に関する情報である。
// 引数を渡す時に間違えないように注意が必要。
//
// ARGUMENTS
// const KanjiCodeType	outType
//		変換後の文字列の漢字コード種類
// const char*			inString
//		変換元文字列。
//		char* 以外の型の文字列を渡す時はキャストが必要。
// const KanjiCodeType	inType
//		変換元文字列の文字コード
//	ModSize upper
//		変換元文字列のうち、最大でも指定されたバイト数しか変換しない
// 
// RETURN
// 各種漢字コードの変換後のバイト数(終端文字は含まない)
// inString に空文字列や NULL ポインターを渡した場合は 0 を返す。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		inType,outType に渡されたコードには対応していない
//
ModSize
ModKanjiCode::JJTransfer::getTransferredSize(const KanjiCodeType	outType,
											 const char*			inString,
											 const KanjiCodeType	inType,
											 ModSize upper)
{
	if (inString == 0) {
		// NULLポインタを渡された場合はゼロを返す
		return 0;
	}

	if ((inType == ucs2 && outType == utf8)
		|| (inType == utf8 && outType == ucs2))
	{
		// UNICODE間の変換は高速化のため自作関数で行う
		return getTransferredSizeForUnicode(
			outType,
			(const unsigned char*)inString,
			inType,
			upper);
	}

	// 前はG-BASEのjjライブラリを利用していたが、64ビット化できないので
	// 今はここも自作になっている

	const unsigned char* b = (const unsigned char*)inString;
	const unsigned char* s = b;
	unsigned short c;
	int u = (int)upper;
	if (u < 0)
		u = ModInt32Max;

	ModSize outCount = 0;

	_JisMode inMode = _None;
	_JisMode outMode = _None;

	for (;;) {
		
		c = 0;
		
		switch (inType) {
		case euc:
			if (*s == _SS2) {
				// 半角カナ
				++s;
				if ((s-b) >= u || *s == 0) break;
				c = *s++;
			} else if (*s == _SS3) {
				// 外字
				++s;
				if ((s+1-b) >= u || *s == 0 || *(s+1) == 0) break;
				c = (*s++ << 8);
				c |= (*s++ & 0x7f);
			} else if (*s & 0x80) {
				// 漢字
				if ((s+1-b) >= u || *s == 0 || *(s+1) == 0) break;
				c = (*s++ << 8);
				c |= *s++;
			} else {
				// ASCII
				c = *s++;
			}
			break;

		case jis:
			while (*s == _ESC) {
				s++;
				if (*s == '$') {
					s++;
					if ((s-b) >= u) break;
					if (*s == 'B') {
						// 漢字モード
						inMode = _Kanji;
						s++;
					} else {
						s -= 2;
						break;
					}
				} else if (*s == '(') {
					s++;
					if ((s-b) >= u) break;
					if (*s == 'B') {
						// モード解除
						inMode = _None;
						s++;
					} else if (*s == 'I') {
						// 半角カナモード
						inMode = _Hankana;
						s++;
					} else {
						s -= 2;
						break;
					}
				} else {
					s -= 1;
					break;
				}
			}
			
			if ((s-b) >= u) break;
			
			switch (inMode) {
			case _Kanji:
				if ((s+1-b) >= u || *(s+1) == 0) break;
				c = (*s++ << 8);
				c |= *s++;
				c |= 0x8080;
				break;
			case _Hankana:
				c = *s++ | 0x80;
				break;
			case _None:
				c = *s++;
				break;
			}
			break;
			
		case shiftJis:
			if (*s >= 0x81 && *s <= 0x9f || *s >= 0xe0 && *s <= 0xfc) {
				// 漢字
				if ((s+1-b) >= u) break;
				unsigned char c1 = *s++;
				if (*s == 0) break;
				unsigned char c2 = *s++;
				if (c1 > 0x9f)
					c1 -= 0x40;
				c1 += c1;
				if (c2 <= 0x9e) {
					c1 -= 0xe1;
					if (c2 >= 0x80)
						c2 -= 1;
					c2 -= 0x1f;
				} else {
					c1 -= 0xe0;
					c2 -= 0x7e;
				}
				c = (c1 << 8) | c2 | 0x8080;
			} else if (*s & 0x80) {
				// 半角カナ
				c = *s++;
			} else {
				// ASCII
				c = *s++;
			}
			break;
			
		case utf8:
			{
				int size = _ByteUTF8[*s];
				if ((s+size-b) >= u) break;
				c = *s++;
				switch (size)
				{
				case 3: if (*s == 0) { c = 0; size = 0; break; }
					c <<= 6; c += *s++;
				case 2: if (*s == 0) { c = 0; size = 0; break; }
					c <<= 6; c += *s++;
				default:
					break;
				}
				c -= _OffUTF8[size];
			}
			break;
			
		case ucs2:
			if ((s+1-b) >= u) { c = 0; break; }
			ModOsDriver::Memory::copy(&c, s, sizeof(unsigned short));
			s += sizeof(unsigned short);
			break;
			
		}

		if (c != 0) {
			
			if ((inType == euc || inType == jis || inType == shiftJis) &&
				(outType == utf8 || outType == ucs2)) {

				// EUC16 -> UCS2 の変換を行う
				c = _euc2unicode[c];
				if (c == 0) c = _UCS2_GETA;

			} else if ((inType == utf8 || inType == ucs2) &&
					   (outType == euc || outType == jis ||
						outType == shiftJis)) {

				// UCS2 -> EUC16 の変換を行う
				// サロゲートペアはゲタ2文字になる
				// サロゲートペアの片割れだけが出現した場合はゲタ1文字になる
				c = _unicode2euc[c];
				if (c == 0) c = _EUC16_GETA;
			
			}
		}

		if (c == 0) break;

		switch (outType) {
		case euc:
			switch (c & 0x8080) {
			case 0x0000:
				// ASCII
				outCount++;
				break;
				
			case 0x0080:
				// 半角カナ
				outCount += 2;
				break;
				
			case 0x8000:
				// 外字
				outCount += 3;
				break;
				
			case 0x8080:
				// 漢字
				outCount += 2;
				break;
				
			}
			break;
			
		case jis:
			switch (c & 0x8080) {
			case 0x0000:
				// ASCII
				switch (outMode) {
				case _Kanji:
				case _Hankana:
					outCount += 4;
					break;
				case _None:
				default:
					outCount++;
					break;
				}
				outMode = _None;
				break;
				
			case 0x0080:
				// 半角カナ
				switch (outMode) {
				case _Kanji:
				case _None:
				default:
					outCount += 4;
					break;
				case _Hankana:
					outCount++;
					break;
				}
				outMode = _Hankana;
				break;
				
			case 0x8000:
			case 0x8080:
				// 漢字と外字(EUC外字も漢字にマップする)
				switch (outMode) {
				case _Kanji:
					outCount += 2;
					break;
				case _Hankana:
				case _None:
				default:
					outCount += 5;
					break;
				}
				outMode = _Kanji;
				break;
				
			}
			break;
			
		case shiftJis:
			switch (c & 0x8080) {
			case 0x0000:
				// ASCII
				outCount++;
				break;
				
			case 0x0080:
				// 半角カナ
				outCount++;
				break;
				
			case 0x8000:
			case 0x8080:
				// 漢字と外字(EUC外字も漢字にマップする)
				outCount += 2;
				break;
			}
			break;
			
		case utf8:
			{
				int size;
				if (c < 0x80)		size = 1;
				else if (c < 0x800) size = 2;
				else				size = 3;
				outCount += size;
			}
			break;
				
		case ucs2:
			outCount += 2;
			break;
			
		}

		if ((s-b) >= u) break;
	}

	if (outType == jis) {

		switch (outMode) {
		case _Kanji:
		case _Hankana:
			outCount += 3;
			break;
		case _None:
		default:
			break;
		}
		
	}
	
	return outCount;
}

//
// FUNCTION private
// ModKanjiCode::JJTransfer::transferForUnicode -- UTF8<->UCS2の変換
//
// NOTES
// jjStrcpyは遅いのでここで実装する
//
// ARGUMENTS
// unsigned char*				outString
//		変換先文字列の領域へのポインタ。領域は呼出側で確保すること。
//		unsigned char* 以外の型の文字列を渡す時はキャストが必要。
// const int			outBytes
//		outString に渡した領域のバイト数
// const KanjiCodeType	outType
//		変換先文字列の文字コード
// const unsigned char*			inString
//		変換元文字列(終端文字で終っていること)。
//		char* 以外の型の文字列を渡す時はキャストが必要。
// const KanjiCodeType	inType
//		変換元文字列の文字コード
// 
// RETURN
// outString を返す
//
// EXCEPTIONS
//
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
unsigned char*
ModKanjiCode::JJTransfer::transferForUnicode(unsigned char* outString,
											 const int outBytes,
											 const KanjiCodeType outType,
											 const unsigned char* inString,
											 const KanjiCodeType inType)
{
	unsigned char* ret = outString;
	int size = 0;
	if (outType == utf8 && inType == ucs2)
	{
		// UCS2 -> UTF8への変換

		while (size < outBytes)
		{
			int s;
			int incomplete_sp = 0;
			ModUnicodeChar c, c2;
			ModOsDriver::Memory::copy(&c, inString, sizeof(ModUnicodeChar));
			ModUnicodeChar c_org = c;
			inString += sizeof(ModUnicodeChar);
			if (c < 0x80)		 s = 1;
			else if (c < 0x800)  s = 2;
			else if (c < 0xd800
				  || c > 0xdfff) s = 3;
			else if (c < 0xdc00) {
				// 0xd800-0xdbff：サロゲートペア前半
				ModOsDriver::Memory::copy(&c2, inString, sizeof(ModUnicodeChar));
				if (c2 < 0xdc00 || c2 >= 0xe000) {
					// 後続文字がサロゲートペア後半でない
					incomplete_sp = 1;
					s = 3;
				} else {
					s = 4;
				}
			} else {
				// 0xdc00-0xdfff：サロゲートペア後半が単体で出現した
				incomplete_sp = 1;
				s = 3;
			}
			if ((size + s) > outBytes) break;
			if (incomplete_sp) {
				// サロゲートペア前半または後半が単体で出現した
				// REPLACEMENT CHARACTER(U+FFFD, 0xef 0xbf 0xbd)に変換する
				outString[0] = 0xef;
				outString[1] = 0xbf;
				outString[2] = 0xbd;
			} else if (s == 4) {
				// サロゲートペア前半が出現した
				// サロゲートペア前半後半を合わせて4バイトのUTF8表現に変換する
				// UCS2 [11011xxx xxyyyyyy] [110111yy zzzzzzzz]
				//    + [00000000 01000000]
				// 					↓
				//      [11011XXX XXyyyyyy] [110111yy zzzzzzzz]
				// 					↓
				// UTF8 [11110XXX] [10XXyyyy] [10yyyyzz] [10zzzzzz]
				inString += sizeof(ModUnicodeChar);
				outString[0] = ((c + 0x0040 & 0x0300) >> 8) | 0xf0;
				outString[1] = ((c + 0x0040 & 0x00fc) >> 2) | 0x80;
				outString[2] = ((c & 0x0003) << 4) | ((c2 & 0x03c0) >> 6) | 0x80;
				outString[3] = (c2 & 0x003f) | 0x80;
			} else {
				// サロゲートペア以外のUCS2文字を1～3バイトのUTF8表現に変換する
				outString += s;
				switch (s)
				{
				case 3: *--outString = (c & 0x3f) | 0x80; c >>= 6;
				case 2: *--outString = (c & 0x3f) | 0x80; c >>= 6;
				case 1: *--outString = (c | _MarkUTF8[s]);
				}
			}
			outString += s;
			size += s;
			if (c_org == 0) break;
		}
	}
	else if (outType == ucs2 && inType == utf8)
	{
		// UTF8 -> UCS2への変換
		
		while (size < outBytes)
		{
			if ((size + sizeof(ModUnicodeChar)) > outBytes) break;
			int s = _ByteUTF8[*inString];
			if (s == 4) {
				// 4バイトのUTF8表現をサロゲートペアに変換する
				// UTF8 [11110xxx] [10xxyyyy] [10yyyyzz] [10zzzzzz]
				// 					↓
				//      [11011xxx xxyyyyyy] [110111yy zzzzzzzz]
				//    - [00000000 01000000]
				// 					↓
				// UCS2 [11011XXX XXyyyyyy] [110111yy zzzzzzzz]
				if ((size + sizeof(ModUnicodeChar) * 2) > outBytes) {
					// サロゲートペア前半後半が格納できない
					if ((size + sizeof(ModUnicodeChar)) <= outBytes) {
						// ModUnicodeChar 1文字を格納するスペースは
						// あるので終端文字を格納する
						ModUnicodeChar c = 0;
						ModOsDriver::Memory::copy(outString, &c, sizeof(ModUnicodeChar));
						outString += sizeof(ModUnicodeChar);
						size += sizeof(ModUnicodeChar);
					}
					// ここで終了する
					break;
				}
				ModUnicodeChar c1 =
					((inString[0] & 0x07) << 8) +
					((inString[1] & 0x3f) << 2) +
					((inString[2] & 0x30) >> 4) + 0xd800 - 0x0040;
				ModUnicodeChar c2 =
					((inString[2] & 0x0f) << 6) +
					 (inString[3] & 0x3f)       + 0xdc00;
				ModOsDriver::Memory::copy(outString, &c1, sizeof(ModUnicodeChar));
				outString += sizeof(ModUnicodeChar);
				ModOsDriver::Memory::copy(outString, &c2, sizeof(ModUnicodeChar));
				outString += sizeof(ModUnicodeChar);
				inString += 4;
				size += sizeof(ModUnicodeChar) * 2;
				if (c1 == 0 || c2 == 0) break;
			} else {
				// 1～3バイトのUTF8表現を1文字のUCS2文字に変換する
				ModUnicodeChar c = *inString++;
				switch (s)
				{
				case 3:	c <<= 6; c += *inString++;
				case 2: c <<= 6; c += *inString++;
				default:
					break;
				}
				c -= _OffUTF8[s];
				ModOsDriver::Memory::copy(outString, &c, sizeof(ModUnicodeChar));
				outString += sizeof(ModUnicodeChar);
				size += sizeof(ModUnicodeChar);
				if (c == 0) break;
			}
		}
	}
	return ret;
}
#else
unsigned char*
ModKanjiCode::JJTransfer::transferForUnicode(unsigned char* outString,
											 const int outBytes,
											 const KanjiCodeType outType,
											 const unsigned char* inString,
											 const KanjiCodeType inType)
{
	unsigned char* ret = outString;
	int size = 0;
	if (outType == utf8 && inType == ucs2)
	{
		// UCS2 -> UTF8への変換

		while (size < outBytes)
		{
			int s;
			ModUnicodeChar c;
			ModOsDriver::Memory::copy(&c, inString, sizeof(ModUnicodeChar));
			ModUnicodeChar c_org = c;
			inString += sizeof(ModUnicodeChar);
			if (c < 0x80)		s = 1;
			else if (c < 0x800)	s = 2;
			else				s = 3;
			if ((size + s) > outBytes) break;
			outString += s;
			switch (s)
			{
			case 3: *--outString = (c & 0x3f) | 0x80; c >>= 6;
			case 2: *--outString = (c & 0x3f) | 0x80; c >>= 6;
			case 1: *--outString = (c | _MarkUTF8[s]);
			}
			outString += s;
			size += s;
			if (c_org == 0) break;
		}
	}
	else if (outType == ucs2 && inType == utf8)
	{
		// UTF8 -> UCS2への変換
		
		while (size < outBytes)
		{
			if ((size + sizeof(ModUnicodeChar)) > outBytes) break;
			int s = _ByteUTF8[*inString];
			ModUnicodeChar c = *inString++;
			switch (s)
			{
			case 3:	c <<= 6; c += *inString++;
			case 2: c <<= 6; c += *inString++;
			default:
				break;
			}
			c -= _OffUTF8[s];
			ModOsDriver::Memory::copy(outString, &c, sizeof(ModUnicodeChar));
			outString += sizeof(ModUnicodeChar);
			size += sizeof(ModUnicodeChar);
			if (c == 0) break;
		}
	}
	return ret;
}
#endif

//
// FUNCTION public
// ModKanjiCode::JJTransfer::getTransferredSizeForUnicode
//	-- UTF8<->UCS2変換時のバイト数を得る
//
// NOTES
// jjStrlenは遅いのでここで実装する
//
// ARGUMENTS
// const KanjiCodeType	outType
//		変換後の文字列の漢字コード種類
// const unsigned char*			inString
//		変換元文字列。
//		char* 以外の型の文字列を渡す時はキャストが必要。
// const KanjiCodeType	inType
//		変換元文字列の文字コード
//	ModSize upper
//		変換元文字列のうち、最大でも指定されたバイト数しか変換しない
// 
// RETURN
// 各種漢字コードの変換後のバイト数(終端文字は含まない)
// inString に空文字列や NULL ポインターを渡した場合は 0 を返す。
//
// EXCEPTIONS
//
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
ModSize
ModKanjiCode::JJTransfer::getTransferredSizeForUnicode(
	const KanjiCodeType outType,
	const unsigned char* inString,
	const KanjiCodeType inType,
	ModSize upper)
{
	ModSize size = 0;
	ModSize inSize = 0;
	if (outType == utf8 && inType == ucs2)
	{
		// UCS2 -> UTF8への変換

		while (inSize < upper)
		{
			int s;
			ModUnicodeChar c;
			ModOsDriver::Memory::copy(&c, inString, sizeof(ModUnicodeChar));
			if (c == 0) break;
			inString += sizeof(ModUnicodeChar);
			inSize += sizeof(ModUnicodeChar);
			if (c < 0x80)		 s = 1;
			else if (c < 0x800)	 s = 2;
			else if (c < 0xd800
				  || c > 0xdfff) s = 3;
			else if (c < 0xdc00) {
				// サロゲートペア前半
				ModUnicodeChar c2;
				ModOsDriver::Memory::copy(&c2, inString, sizeof(ModUnicodeChar));
				if (c2 < 0xdc00 || c2 >= 0xe000) {
					// 後続文字がサロゲートペア後半でない
					// サロゲートペア前半単体をREPLACEMENT CHARACTERに読み替える
					s = 3;
				} else {
					// サロゲートペアはUTF8で4バイトになる
					s = 4;
					inString += sizeof(ModUnicodeChar);
					inSize += sizeof(ModUnicodeChar);
				}
			}
			else {
				// サロゲートペア後半
				// 単体で出現したらREPLACEMENT CHARACTERに読み替える
				s = 3;
			}
			size += s;
		}
	}
	else if (outType == ucs2 && inType == utf8)
	{
		// UTF8 -> UCS2への変換
		
		while (inSize < upper)
		{
			if (*inString == 0) break;
			int s = _ByteUTF8[*inString];
			; ModAssert(s >= 0 && s <= 4);
			if (s == 0) s = 1;
			// 途中のバイトが0だったらそこで打ち切る
			switch (s) {
			case 4: if (*++inString == 0) goto endWhile;
			case 3: if (*++inString == 0) goto endWhile;
			case 2: if (*++inString == 0) goto endWhile;
			case 1:
			default:
				++inString;
			}
			inSize += s;
			size += sizeof(ModUnicodeChar);
			if (s == 4) {
				// 4バイトのUTF8はサロゲートペア相当として
				// ModUnicodeChar 2文字にカウントする
				size += sizeof(ModUnicodeChar);
			}
		}
	endWhile:
		;
	}
	return size;
}
#else
ModSize
ModKanjiCode::JJTransfer::getTransferredSizeForUnicode(
	const KanjiCodeType outType,
	const unsigned char* inString,
	const KanjiCodeType inType,
	ModSize upper)
{
	ModSize size = 0;
	ModSize inSize = 0;
	if (outType == utf8 && inType == ucs2)
	{
		// UCS2 -> UTF8への変換

		while (inSize < upper)
		{
			int s;
			ModUnicodeChar c;
			ModOsDriver::Memory::copy(&c, inString, sizeof(ModUnicodeChar));
			if (c == 0) break;
			inString += sizeof(ModUnicodeChar);
			inSize += sizeof(ModUnicodeChar);
			if (c < 0x80)		s = 1;
			else if (c < 0x800)	s = 2;
			else				s = 3;
			size += s;
		}
	}
	else if (outType == ucs2 && inType == utf8)
	{
		// UTF8 -> UCS2への変換
		
		while (inSize < upper)
		{
			if (*inString == 0) break;
			int s = _ByteUTF8[*inString];
			; ModAssert(s >= 0 && s <= 3);
			if (s == 0) s = 1;
			// 途中のバイトが0だったらそこで打ち切る
			switch (s) {
			case 3: if (*++inString == 0) goto endWhile;
			case 2: if (*++inString == 0) goto endWhile;
			case 1:
			default:
				++inString;
			}
			inSize += s;
			size += sizeof(ModUnicodeChar);
		}
	endWhile:
		;
	}
	return size;
}
#endif

//
// Copyright (c) 1997, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
