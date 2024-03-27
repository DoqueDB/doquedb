// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// 文字に関する情報クラス
// 
// Copyright (c) 2000, 2023, 2024 Ricoh Company, Ltd.
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

#include "UnicodeCharacterType.h"
#include "UnicodeDataFile.h"
#include "UnicodeDataRowUnicodeData.h"
#include "UnicodeDataRowBlocks.h"


#include <iostream>
#include <iomanip>
#include <assert.h>
#include <string.h>

using namespace std;

// 読み込む「設定ファイル(Unicode の定義ファイル名など)」のデフォルト名
const char g_defaultFilenameOfUnicodeData[] = "../src/UnicodeData-1.1.5.txt";

// 必要なブロックの最初の文字コードと最後の文字コードの
// デフォルト値 (Unicode 1.1.5に相当)

unsigned short g_HiraganaBlockFirst		= 0x3040;
unsigned short g_HiraganaBlockLast		= 0x309f;

unsigned short g_KatakanaBlockFirst		= 0x30a0;
unsigned short g_KatakanaBlockLast		= 0x30ff;

unsigned short g_GreekBlockFirst		= 0x0370;
unsigned short g_GreekBlockLast			= 0x03cf;

unsigned short g_GreekExtendedBlockFirst = 0x1f00;
unsigned short g_GreekExtendedBlockLast	= 0x1fff;

unsigned short g_CyrillicBlockFirst		= 0x0400;
unsigned short g_CyrillicBlockLast		= 0x04ff;

unsigned short g_BoxDrawingBlockFirst	= 0x2500;
unsigned short g_BoxDrawingBlockLast	= 0x257f;

// Unicode では定義されていない特殊な値（!!注!! ファイルから読み込めない情報）
const unsigned short g_AsciiLast		= 0x007F;	// ASCII コードの最大値
const unsigned short g_HankakukanaFirst	= 0xFF61;	// 半角カナ先頭文字
const unsigned short g_HankakukanaLast	= 0xFF9F;	// 半角カナ末尾文字

// 特殊な値のつづき (BasicLatin と全角大文字の A-F,a-f)
const unsigned short g_BasicLatinCapitalA		= 0x0041;
const unsigned short g_BasicLatinCapitalF		= 0x0046;
const unsigned short g_BasicLatinSmallA			= 0x0061;
const unsigned short g_BasicLatinSmallF			= 0x0066;
const unsigned short g_FullWidthLatinCapitalA	= 0xff21;
const unsigned short g_FullWidthLatinCapitalF	= 0xff26;
const unsigned short g_FullWidthLatinSmallA		= 0xff41;
const unsigned short g_FullWidthLatinSmallF		= 0xff46;

#define	PRIVATE_HANDLING_OF_UNICODE_DATA
//
// 以前取得したUnicodeData-1.1.5.txtでは4E00～FAFFまでのデータは
// 以下のようになっていた
// UnicodeData-1.1.5.txt (旧):
//   4DFE;HANGUL SYLLABLE MIEUM WEO RIEUL-SIOS;Lo;0;L;1106 116F 11B3;;;;N;;;;;
//   4DFF;HANGUL SYLLABLE MIEUM WEO RIEUL-THIEUTH;Lo;0;L;1106 116F 11B4;;;;N;;;;;
//   4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
//   9FFF;<CJK Ideograph, Last>;Lo;0;L;;;;;N;;;;;
//   E000;<Private Use, First>;Co;0;L;;;;;N;;;;;
//   F8FF;<Private Use, Last>;Co;0;L;;;;;N;;;;;
//   F900;<CJK Compatibility Ideograph, First>;Lo;0;L;;;;;N;;;;;
//   FAFF;<CJK Compatibility Ideograph, Last>;Lo;0;L;;;;;N;;;;;
//   FB00;LATIN SMALL LIGATURE FF;Ll;0;L;0066 0066;;;;N;;;;;
//   FB01;LATIN SMALL LIGATURE FI;Ll;0;L;0066 0069;;;;N;;;;;
//
// しかし、現在同じファイルを取得すると以下のようになっている
// UnicodeData-1.1.5.txt (新):
//   4DFE;HANGUL SYLLABLE MIEUM WEO RIEUL-SIOS;Lo;0;L;1106 116F 11B3;;;;N;;;;;
//   4DFF;HANGUL SYLLABLE MIEUM WEO RIEUL-THIEUTH;Lo;0;L;1106 116F 11B4;;;;N;;;;;
//   4E00;<CJK IDEOGRAPH REPRESENTATIVE>;Lo;0;L;;;;;N;;;;;
//   FB00;LATIN SMALL LIGATURE FF;Ll;0;L;0066 0066;;;;N;;;;;
//   FB01;LATIN SMALL LIGATURE FI;Ll;0;L;0066 0069;;;;N;;;;;
//
// この問題に対応するため、4E00～FAFFのコード範囲については
// 従来と同じModUnicodeCharTrait.tblが生成されるように独自処理を行う
//
// なお、現在でもUnicodeData.txtのより新しいバージョンでは
// First, Lastが復活している
// 文字種別が増えているため、ModUnicodeCharTrait.tblはいずれ見直して
// 修正する必要がある
// UnicodeData-2.0.14.txt:
//   33FD;IDEOGRAPHIC TELEGRAPH SYMBOL FOR DAY THIRTY;So;0;L;<compat> 0033 0030 65E5;;;;N;;;;;
//   33FE;IDEOGRAPHIC TELEGRAPH SYMBOL FOR DAY THIRTY-ONE;So;0;L;<compat> 0033 0031 65E5;;;;N;;;;;
//   4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
//   9FA5;<CJK Ideograph, Last>;Lo;0;L;;;;;N;;;;;
//   AC00;<Hangul Syllable, First>;Lo;0;L;;;;;N;;;;;
//   D7A3;<Hangul Syllable, Last>;Lo;0;L;;;;;N;;;;;
//   D800;<Unassigned High Surrogate, First>;Cs;0;L;;;;;N;;;;;
//   DB7F;<Unassigned High Surrogate, Last>;Cs;0;L;;;;;N;;;;;
//   DB80;<Private Use High Surrogate, First>;Cs;0;L;;;;;N;;;;;
//   DBFF;<Private Use High Surrogate, Last>;Cs;0;L;;;;;N;;;;;
//   DC00;<Low Surrogate, First>;Cs;0;L;;;;;N;;;;;
//   DFFF;<Low Surrogate, Last>;Cs;0;L;;;;;N;;;;;
//   E000;<Private Use, First>;Co;0;L;;;;;N;;;;;
//   F8FF;<Private Use, Last>;Co;0;L;;;;;N;;;;;
//   F900;<CJK Compatibility Ideograph, First>;Lo;0;L;;;;;N;;;;;
//   FA2D;<CJK Compatibility Ideograph, Last>;Lo;0;L;;;;;N;;;;;
//   FB00;LATIN SMALL LIGATURE FF;Ll;0;L;<compat> 0066 0066;;;;N;;;;;
//   FB01;LATIN SMALL LIGATURE FI;Ll;0;L;<compat> 0066 0069;;;;N;;;;;
//
#ifdef PRIVATE_HANDLING_OF_UNICODE_DATA
int do_private = 0; // 独自処理を行うコード範囲内
#endif

//
// 動作確認のための関数
//
void
mainTest()
{
	UnicodeDataFile	datafile(g_defaultFilenameOfUnicodeData,
							 UnicodeDataRowTypes::UnicodeData);

	for (unsigned short i = 0; i < 0xffff; ++i) {
		UnicodeDataRowUnicodeData* row
			= (UnicodeDataRowUnicodeData*)datafile.getNextRow();
		if (row == 0) {
			// 全ての行を読み終った
			; assert(datafile.isEmpty());
			break;
		}
		cout << "codeValue = " << hex << row->getCodeValue() << endl;
		cout << "name = " << row->getName() << endl;
		cout << "char type = " << row->getCharacterType() << endl;
		cout << "upper code = " << hex << row->getUpperCaseCodeValue() << endl;
		cout << "lower code = " << hex << row->getLowerCaseCodeValue() << endl;
		cout << "title code = " << hex << row->getTitleCaseCodeValue() << endl;
		cout << endl;

		delete row;
		row = 0;
	}
}

void
initializeBlockInformation(const char* const filenameOfBlocks)
{
	// ファイルのオープンに失敗すると例外発生
	UnicodeDataFile	datafile(filenameOfBlocks, UnicodeDataRowTypes::Blocks);

	UnicodeDataRowBlocks*		row = 0;

	while ((row = (UnicodeDataRowBlocks*)datafile.getNextRow()) != 0) {

		if (!strcmp(row->getName(), "Greek")) {
			g_GreekBlockFirst	= row->getFirstCodeValue();
			g_GreekBlockLast	= row->getLastCodeValue();

			cerr << hex
				 << g_GreekBlockFirst << " ;"
				 << g_GreekBlockLast << "; g_Greek" << endl;

		} else if (!strcmp(row->getName(), "Greek Extended")) {
			g_GreekExtendedBlockFirst	= row->getFirstCodeValue();
			g_GreekExtendedBlockLast	= row->getLastCodeValue();

			cerr << hex
				 << g_GreekExtendedBlockFirst << " ;"
				 << g_GreekExtendedBlockLast << "; g_GreekExtended" << endl;

		} else if (!strcmp(row->getName(), "Cyrillic")) {
			g_CyrillicBlockFirst		= row->getFirstCodeValue();
			g_CyrillicBlockLast			= row->getLastCodeValue();

			cerr << hex
				 << g_CyrillicBlockFirst	<< " ;"
				 << g_CyrillicBlockLast	<< "; g_Cyrillic" << endl;

		} else if (!strcmp(row->getName(), "Hiragana")) {
			g_HiraganaBlockFirst	= row->getFirstCodeValue();
			g_HiraganaBlockLast		= row->getLastCodeValue();

			cerr << hex
				 << g_HiraganaBlockFirst << " ;"
				 << g_HiraganaBlockLast << "; g_Hiragana" << endl;

		} else if (!strcmp(row->getName(), "Katakana")) {
			g_KatakanaBlockFirst	= row->getFirstCodeValue();
			g_KatakanaBlockLast		= row->getLastCodeValue();

			cerr << hex
				 << g_KatakanaBlockFirst << " ;"
				 << g_KatakanaBlockLast << "; g_Katakana" << endl;

		} else if (!strcmp(row->getName(), "Box Drawing")) {
			g_BoxDrawingBlockFirst	= row->getFirstCodeValue();
			g_BoxDrawingBlockLast	= row->getLastCodeValue();

			cerr << hex
				 << g_BoxDrawingBlockFirst << " ;"
				 << g_BoxDrawingBlockLast << "; g_BoxDrawing" << endl;

		}
	}
}

//
// 定義ファイルの第三フィールド(カテゴリ名)を使って分類を行ない、
// 求められた文字種を返す
//
UnicodeCharacterType
getCharacterTypeByCategoly(const UnicodeDataRowUnicodeData* row)
{
	UnicodeCharacterType	charType;

	// Mark
	if (row->getCharacterType()[0] == 'M') {
		// Mark is a SYMBOL
		charType.addSymbol();
	}

	// Number
	else if (!strcmp(row->getCharacterType(), "Nd")) {
		charType.addDigit();
	} else if (!strcmp(row->getCharacterType(), "Nl")) {
		charType.addSymbol();
	} else if (!strcmp(row->getCharacterType(), "No")) {
		charType.addSymbol();
	}

	// Separetor
	else if (!strcmp(row->getCharacterType(), "Zs")) {
		charType.addSpace();
	} else if (!strcmp(row->getCharacterType(), "Zl")) {
		charType.addSymbol();
	} else if (!strcmp(row->getCharacterType(), "Zp")) {
		charType.addSymbol();
	}

	// Control
	else if (!strcmp(row->getCharacterType(), "Cc")) {
		charType.addControl();
	} else if (!strcmp(row->getCharacterType(), "Cf")) {
		charType.addFormat();
	} else if (!strcmp(row->getCharacterType(), "Cs")) {
		charType.addSurrogate();
	} else if (!strcmp(row->getCharacterType(), "Co")) {
		charType.addGaiji();
	} else if (!strcmp(row->getCharacterType(), "Cn")) {
		// 現在、Cn に該当する文字は Unicode に存在せず、どのような文字種に
		// するべきかわかっていない
		cerr << "Define File is including unexpected character type("
			 << row->getCharacterType() << ")."
			 << endl;
		; assert(0);
		throw 1;
	}

	// Letter
	else if (!strcmp(row->getCharacterType(), "Lu")) {
		charType.addAlphabet();
		charType.addUpper();
	} else if (!strcmp(row->getCharacterType(), "Ll")) {
		charType.addAlphabet();
		charType.addLower();
	} else if (!strcmp(row->getCharacterType(), "Lt")) {
		charType.addAlphabet();
	} else if (!strcmp(row->getCharacterType(), "Lm")) {
		charType.addSymbol();
	} else if (!strcmp(row->getCharacterType(), "Lo")) {
		charType.addLetterOther();
	}

	// Punction
	else if (row->getCharacterType()[0] == 'P') {
		// Punction is a SYMBOL
		charType.addSymbol();
	}

	// Symbol
	else if (row->getCharacterType()[0] == 'S') {
		// Symbol is a SYMBOL
		charType.addSymbol();
	}

	// その他
	else {
		// 読み込んだ定義ファイル(Unicode.dat)に未知の文字種類
		// が含まれていた！！！
		cerr << "Dafine File is including unexpected character type("
			 << row->getCharacterType() << ")."
			 << endl;
		; assert(0);
		throw 1;
	}

	return charType;
}

//
// 定義ファイルの第三フィールド(カテゴリ名)以外の情報(他のフィールドや
// その他の特殊な定数など)を使って分類を行ない、求められた文字種を返す
//
UnicodeCharacterType
getCharacterTypeByOther(const unsigned short				code,
						const UnicodeDataRowUnicodeData*	row,
						const UnicodeCharacterType&			constCharType)
{
	UnicodeCharacterType	charType;

	//
	// 定義ファイルの第三フィールド以外の情報も使って分類を行なう
	//
		
	// 数字(16進)
	if (constCharType.isDigit()) {
		// 0 から 9 なら数字(16進)
		charType.addXdigit();
	} else {
		// 2000年 03/23 に仕様変更により追加
		if ((code >= g_BasicLatinCapitalA && code <= g_BasicLatinCapitalF)
			|| (code >= g_BasicLatinSmallA && code <= g_BasicLatinSmallF)
			|| (code >= g_FullWidthLatinCapitalA && code <= g_FullWidthLatinCapitalF)
			|| (code >= g_FullWidthLatinSmallA && code <= g_FullWidthLatinSmallF))
		{
			// BasicLatin と全角大文字の A-F, a-f ならば16進
			charType.addXdigit();
		}

#if 0	// 2000年 03/23 に仕様変更により削除

		// "LEETER A" から "LETTER F" なら数字(16進)
		char* ptr = strstr(row->getName(), "LETTER");
		if (ptr != 0) {
			const char letter = *(ptr + 7);
			const char lastLetter = *(ptr + 8);
			if (letter >= 'A' && letter <= 'F'&& lastLetter == '\0') {
				charType.addXdigit();
			}
		}
#endif
	}

	//
	// ここ以降は主に文字コードにもとづいて MOD 固有の分類を行なう
	//

	// ASCII
	if (code <= g_AsciiLast) {
		charType.addAscii();
	}

	// 半角カタカナ
	if (code >= g_HankakukanaFirst && code <= g_HankakukanaLast) {
		charType.addHankakuKana();
	}

	// ひらがな
	if (code >= g_HiraganaBlockFirst && code <= g_HiraganaBlockLast) {
		charType.addHiragana();
	}

	// カタカナ
	if (code >= g_KatakanaBlockFirst && code <= g_KatakanaBlockLast) {
		charType.addKatakana();
	}

	// ギリシャ文字
	if ((code >= g_GreekBlockFirst && code <= g_GreekBlockLast)
		|| (code >= g_GreekExtendedBlockFirst && code <= g_GreekExtendedBlockLast)) {
		charType.addGreek();
	}

	// ロシア文字
	if (code >= g_CyrillicBlockFirst && code <= g_CyrillicBlockLast) {
		charType.addCyrillic();
	}

	// 罫線
	if (code >= g_BoxDrawingBlockFirst && code <= g_BoxDrawingBlockLast) {
		charType.addLine();
	}

	return charType;
}

//
// 分類を行なう文字のコード値とデータ行(NULL の可能性あり)を受けとり、
// 与えられた文字コードに対応した文字種を返す。
//
// code	: この文字について分析する
// row	: 普通は code に対応するデータ行が渡されるが、対応するものがなければ
//		   code の次のデータ行が渡される。次の行も無ければ NULL が渡される。
//
UnicodeCharacterType
createCharacterType(const unsigned short				code,
					const UnicodeDataRowUnicodeData*	row)
{
	UnicodeCharacterType	charType;

#ifdef PRIVATE_HANDLING_OF_UNICODE_DATA
	// コード4E00の行が以下の形式で＊ない＊場合、
	//   4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
	// コード範囲4E00～FAFFについては独自にcharTypeを設定する
	if (!do_private && code == 0x4e00 && row != 0 && 
		strcmp(row->getName(), "<CJK Ideograph, First>") != 0) {
		do_private = 1;
	}
	if (do_private) {
		//   4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
		//   9FFF;<CJK Ideograph, Last>;Lo;0;L;;;;;N;;;;;
		//   E000;<Private Use, First>;Co;0;L;;;;;N;;;;;
		//   F8FF;<Private Use, Last>;Co;0;L;;;;;N;;;;;
		//   F900;<CJK Compatibility Ideograph, First>;Lo;0;L;;;;;N;;;;;
		//   FAFF;<CJK Compatibility Ideograph, Last>;Lo;0;L;;;;;N;;;;;
		charType.clear();
		if (code >= 0x4e00 && code <= 0x9fff) {
			charType.addKanji();
			charType.addLetterOther();
		} else if (code >= 0xa000 && code <= 0xdfff) {
			charType.addNotused();
		} else if (code >= 0xe000 && code <= 0xf8ff) {
			charType.addGaiji();
		} else if (code >= 0xf900 && code <= 0xfaff) {
			charType.addLetterOther();
			if (code == 0xfaff) {
				do_private = 0;
			}
		}
		return charType;
	}
#endif

	//
	// 文字名フィールドに「ブロックの先頭、末尾」という特殊な情報が
	// 書いてある場合は、ここで処理してしまう。
	//

	//
	// (注意)
	// ブロックの間にある「notused」を漢字などに誤って分類しないこと。
	// "<..., First>" を読む時は注意しよう。


	if (row != 0) {
		// 特殊な定義方法(ブロック内部の全文字を2行で表現している)の場合は
		// ここで処理する

		// 漢字
		if ((!strcmp(row->getName(), "<CJK Ideograph, First>")
			 && code == row->getCodeValue())
			|| !strcmp(row->getName(), "<CJK Ideograph, Last>")) {

			charType = getCharacterTypeByCategoly(row);
			charType.addKanji();	// 漢字

			return charType;	// !! 処理終了 !!
		}

		// 漢字以外のブロック
		//
		// - Hangul Syllable
		// - Unassigned High Surrogate
		// - Private Use High Surrogate
		// - Low Surrogate
		// - Private Use
		//
		else if ((strstr(row->getName(), ", First>")
				  && code == row->getCodeValue())
				 || strstr(row->getName(), ", Last>")) {
			//
			charType = getCharacterTypeByCategoly(row);

			return charType;	// !! 処理終了 !!
		}
	}

	//
	// これ以降の判定は「row の文字名フィールドには本当の文字名が
	// 入っている」という前提で処理を行なう
	//

	if (row != 0 && code == row->getCodeValue()) {
		//* 分類したい文字(code)に関する情報が定義ファイルに存在する場合の処理

		UnicodeCharacterType charTypeByCategory
			= getCharacterTypeByCategoly(row);
		UnicodeCharacterType charTypeByOther
			= getCharacterTypeByOther(code, row, charTypeByCategory);
		charType = charTypeByCategory | charTypeByOther;
		
	} else {
		//* 分類したい文字(code)の情報がファイルにない場合の処理
		; assert(!charType.isNotNone());
		
		// Unicode で使われていない文字コード
		charType.addNotused();
	}

	return charType;
}

// ModUnicodeCharTrait のテーブル作成用のフォーマットで文字種を出力
//
// code :
//	これから文字種を出力する文字のコード値
// mappedCode :
//	大文字/小文字変換した先のコード値変換できない時は code と同じ値が渡される
// charType :
//	出力する文字種
//
void printModUnicodeCharTraitTable(const unsigned short				code,
								   const UnicodeCharacterType		charType,
								   const UnicodeDataRowUnicodeData*	row)
{

	// 出力のフォーマットは次のようになる
	//
	// /* コード値 */ {文字種、大文字/小文字への変換先コード値},
	//

	cout.setf(ios::right, ios::adjustfield);
	cout.fill('0');

	// コード値を先頭に出力
	cout << "/* 0x" << setw(4) << hex << code << " */ ";

	// 文字種を出力
	cout << "{0x" << setw(8) << hex << charType.getValue();

	// 変換先コード値を出力
	{
		unsigned short mappedCode = code;
		if (charType.isUpper()) {
			mappedCode = row->getLowerCaseCodeValue();
		}
		if (charType.isLower()) {
			; assert(mappedCode == code);
			mappedCode = row->getUpperCaseCodeValue();
		}
		
		cout << ", 0x" << setw(4) << hex << mappedCode;
	}

	// 終了
	if (code == 0xffff) {
		// 安全のために、最後はカンマをつけないそして、改行コードも入れる
		cout << "}" << endl << endl;
	} else {
		cout << "}," << endl;
	}
}


// 内容確認のためのフォーマットで出力
void printAllInformations(const unsigned short				code,
						  const UnicodeCharacterType		charType,
						  const UnicodeDataRowUnicodeData*	row)
{

	cout.setf(ios::right, ios::adjustfield);
	cout.fill('0');
	cout << "0x" << setw(4) << hex << code;
	if (row != 0 && row->getCodeValue() == code) {
		cout << " : " << row->getName() << endl;
	} else {
		cout << " : /* This position shall not be used */" << endl;
	}

	cout << "	isNotused : ";
	if (charType.isNotused()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isAlphabet : ";
	if (charType.isAlphabet()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isUpper : ";
	if (charType.isUpper()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isLower : ";
	if (charType.isLower()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isDigit : ";
	if (charType.isDigit()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	is XDigit: ";
	if (charType.isXdigit()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isSymbol : ";
	if (charType.isSymbol()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isSpace : ";
	if (charType.isSpace()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isAscii : ";
	if (charType.isAscii()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isHankakuKana : ";
	if (charType.isHankakuKana()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}
	
	cout << "	isHiragana : ";
	if (charType.isHiragana()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}
	cout << "	isKatakana : ";
	if (charType.isKatakana()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isGreek : ";
	if (charType.isGreek()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isCyrillic : ";
	if (charType.isCyrillic()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isLine : ";
	if (charType.isLine()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}

	cout << "	isKanji : ";
	if (charType.isKanji()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}
	
	cout << "	isNotNone : ";
	if (charType.isNotNone()) {
		cout << "true" << endl;
	} else {
		cout << "false" << endl;
	}
}

int
main(int argc, char* argv[])
{
	const char* filenameOfUnicodeData	= 0;
	const char* filenameOfBlocks		= 0;

	if (argc == 1) {
		// 引数省略時はデフォルト値を適用
		filenameOfUnicodeData	= g_defaultFilenameOfUnicodeData;
		filenameOfBlocks		= 0;

	} else if (argc == 2) {
		// Blocks を表す引数省略時はデフォルト値を適用
		filenameOfUnicodeData	= argv[1];
		filenameOfBlocks		= 0;
		
	} else if (argc == 3) {
		filenameOfUnicodeData	= argv[1];
		filenameOfBlocks		= argv[2];
	} else {
		cerr << "bad argument. usage : % "
			 <<	argv[0] << " [UnicodeData.txt [Blocks.txt]]"
			 << endl;
		return 1;
	}

	try {
		// ブロックに関する情報を初期化
		if (filenameOfBlocks != 0) {
			initializeBlockInformation(filenameOfBlocks);
		}

		// ファイルのオープンに失敗すると例外発生
		UnicodeDataFile	datafile(filenameOfUnicodeData,
								 UnicodeDataRowTypes::UnicodeData);

		unsigned short				cur = 0x0000;
		const unsigned short		end = 0xffff;
		UnicodeDataRowUnicodeData*	row = 0;

		for ( ; ; ++cur) {

			if (row == 0 && !datafile.isEmpty()) {
				// 定義ファイルから次のデータ行を読み込む
				row = (UnicodeDataRowUnicodeData*)datafile.getNextRow();
			}

			UnicodeCharacterType	charType;

			// データ行(null の可能性あり)にもとづいて文字種を作成		
			charType = createCharacterType(cur, row);

			//* 分類結果を適切な方法で出力
//			int formatType = 0;
			int formatType = 1;
//			int formatType = 2;

			switch(formatType) {

			case 0: // 何もしない
				// do nothing 
				break;

			case 1:	// テーブル作成のためのフォーマットで出力
				printModUnicodeCharTraitTable(cur, charType, row);
				break;

			case 2:	// 内容確認のためのフォーマットで出力
				printAllInformations(cur, charType, row);
				break;

			default:
				cerr << "unknown format type";
				; assert(0);
				throw 1;
			}

			// 使い終った情報を始末する
			if (row != 0 && row->getCodeValue() <= cur) {
				; assert(row->getCodeValue() == cur);
				delete row;
				row = 0;
			}

			if (cur == end) {
				// テーブル作成終了
				break;
			}
		}
	} catch (int errCode) {
		cerr << "cought exception !" << endl;
		cerr << "what happen ?" << endl;
		return 1;
	}

	return 0;
}
