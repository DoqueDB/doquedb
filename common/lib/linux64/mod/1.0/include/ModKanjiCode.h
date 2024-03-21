// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModKanjiCode.h -- ModKanjiCode のクラス定義
// 
// Copyright (c) 1997, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModKanjiCode_H__
#define __ModKanjiCode_H__

#include "ModTypes.h"
#include "ModCommonDLL.h"
#include "ModWideChar.h"
#include "ModUnicodeChar.h"

// ModMessage.h をインクルードしたいが相互インクルードになってしまう。
class ModMessage;

//
// CLASS
// ModKanjiCode -- 漢字コード変換のためのクラス定義
//
// NOTES
// char や ModWideChar で表された文字列のコード変換を行なう関数を
// まとめたクラスである。
// このクラスはインスタンスは作られない。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModKanjiCode
{
public:
	friend class ModMessageStream;

	//
	// KanjiCodeを表す定数
	//
	// 本クラスのメソッドに文字コード種類 unknown を渡すと
	// 例外 ModCommonErrorBadArgument を送出する。
	//
	enum KanjiCodeType {
		euc				= 0,
		jis				= 1,
		shiftJis		= 2,
		utf8			= 3,
		ucs2			= 4,	// ModUnicodeChar 型の文字は UCS-2 の相当する
		processLocale	= 5,	// プロセスロケール
		
		unknown			= -1	// 未知の文字コード種類
	};

	// MOD 関係者にこれらのメソッドを使っていないことを確認したので
	// 削除する。だが、用心のためにコメントアウトするだけにしておく
#ifdef OBSOLETE
	// char* に対するコード変換
	//（ここにあるメソッドより、なるべく jj を利用したメソッドを使うべき）
	static const char* eucToJis(char* destination, const char* source,
								ModSize sizeOfDestination);
	static const char* jisToEuc(char* destination, const char* source,
								ModSize sizeOfDestination);
	static const char* eucToShiftJis(char* destination, const char* source,
									 ModSize sizeOfDestination);
	static const char* shiftJisToEuc(char* destination, const char* source,
									 ModSize sizeOfDestination);
	static const char* jisToShiftJis(char* destination, const char* source,
									 ModSize sizeOfDestination);
	static const char* shiftJisToJis(char* destination, const char* source,
									 ModSize sizeOfDestination);

	// ModWideChar* に対するコード変換 出力先の型に注意
	//（ここにあるメソッドより、なるべく jj を利用したメソッドを使うべき）
	static const char* eucToWide(ModWideChar* destination, const char* source,
								 ModSize sizeOfDestination);
	static const char* jisToWide(ModWideChar* destination, const char* source,
								 ModSize sizeOfDestination);
	static const ModWideChar* wideToShiftJis(char* destination,
											 const ModWideChar* source,
											 ModSize sizeOfDestination);
	static const char* shiftJisToWide(ModWideChar* destination,
									  const char* source,
									  ModSize sizeOfDestination);
#endif

	// EUCへの変換(入力の漢字コードを引数で与える)
	// (inType には euc,jis,shiftjis しか指定できない。
	//  他の種類を渡すと例外 ModCommonErrorBadArgument が返る)
	static const char* toEuc(char* destination, const char* source,
							 ModSize sizeOfDestination, KanjiCodeType inType);

	// ModWideChar*への変換(入力の漢字コードを引数で与える)
	// (inType には euc,jis,shiftjis しか指定できない。
	//  他の種類を渡すと例外 ModCommonErrorBadArgument が返る)
	static const char* toWide(ModWideChar* destination, const char* source,
							  ModSize sizeOfDestination, KanjiCodeType inType);

	static const ModWideChar* wideToJis(char* destination,
										const ModWideChar* source,
										ModSize sizeOfDestination);
	static const ModWideChar* wideToEuc(char* destination,
										const ModWideChar* source,
										ModSize sizeOfDestination);

	// jj を使った文字変換(変換先のバイト数を指定できる)
	static char* jjTransfer(char*					outString,
							const int				outBytes,
							const KanjiCodeType		outType,
							const char*				inString,
							const KanjiCodeType		inType);
	
	// 各種漢字コードの変換後のバイト数を取得
	// - 文字列は終端文字で終っていること
	// - 得られるバイト数に終端文字は含まない
	// - char* ではない文字列(ucs2 など)は char* にキャストして渡すこと
    static ModSize jjGetTransferredSize(const char*				source,
										const KanjiCodeType		sourceType,
										const KanjiCodeType		outType);
    static ModSize jjGetTransferredSize(const char*				source,
										const KanjiCodeType		sourceType,
										ModSize					upper,
										const KanjiCodeType		outType);

	// 各種漢字コード(特にマルチバイト文字)の１文字のバイト数を取得
	// (ucs2 文字は固定長なので sizeof(ModUncicodeChar) でバイト数を求める
	//  ことをお勧めします。もし、codeType に ucs2 を渡した場合は c を
	//  調べずに sizeof(ModUncicodeChar) を返します)
	ModCommonDLL
    static ModSize getCharacterSize(const unsigned char		c,
									const KanjiCodeType		codeType);

	// 文字列リテラルに用いられる漢字コード
	ModCommonDLL
	static const KanjiCodeType literalCode;

private:
	//
	// 処理中の状態を示す定数
	//
	enum ModKanjiCodeMode {
		kana = 1,						// 半角カナ
		kanji = 2						// kanji IN
	};

	// jj ライブラリをカプセルしたコード変換クラス

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

	class JJTransfer {
	public:
		// 文字コードを変換する。
		// - 変換元文字バッファ、変換先文字バッファは char* にキャストすること
 		// - jj の jjSetStr, jjGetc を読んでいるだけ。
		// - 戻り値は変換したバイト数。
		ModCommonDLL
		static char* transfer(char*						outString,
							  const int					outBytes,
							  const KanjiCodeType		outType,
							  const char*				inString,
							  const KanjiCodeType		inType);

		// 各種漢字コードの変換後のバイト数を取得
		// - jj の jjStrlen を読んでいるだけ。
		ModCommonDLL
		static ModSize getTransferredSize(const KanjiCodeType	outType,
										  const char*			inString,
										  const KanjiCodeType	inType,
										  ModSize upper);

	private:
		// UTF8 <-> UCS2の変換
		static unsigned char* transferForUnicode(unsigned char* outString,
												 const int outBytes,
												 const KanjiCodeType outType,
												 const unsigned char* inString,
												 const KanjiCodeType inType);
		
		// UTF8 <-> UCS2のサイズ取得
		static ModSize getTransferredSizeForUnicode(
			const KanjiCodeType	outType,
			const unsigned char* inString,
			const KanjiCodeType inType,
			ModSize upper);
	};


	// インスタンスは作られないのでコンストラクタとデストラクタを
	// ここで宣言して定義はしない。
	ModKanjiCode();
	~ModKanjiCode();

	// 実際の変換を行なう関数
	ModCommonDLL
	static const char* transfer(char* destination, const char* source,
								ModSize sizeOfDestination,
								KanjiCodeType inCode, KanjiCodeType outCode);
	ModCommonDLL
	static const ModWideChar* transfer(char* destination,
									   const ModWideChar* source,
									   ModSize sizeOfDestination,
									   KanjiCodeType outCode);
	ModCommonDLL
	static const char* transfer(ModWideChar* destination, const char* source,
								ModSize sizeOfDestination,
								KanjiCodeType inCode);
	// transferの下請け
	static ModWideChar getChar(const char*& source, KanjiCodeType inCode,
							   int& mode);
	static ModBoolean putChar(char*& destination, ModWideChar wideChar,
							  const char* last, KanjiCodeType outCode,
							  int& mode);
};

// MOD 関係者にこれらのメソッドを使っていないことを確認したので
// 削除する。だが、用心のためにコメントアウトするだけにしておく

#ifdef OBSOLETE
//
// FUNCTION
// ModKanjiCode::eucToJis -- EUCからJISへの変換
//
// NOTES
// この関数はchar*で与えられた文字列をEUCからJISに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::eucToJis(char* destination, const char* source,
					   ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::euc, ModKanjiCode::jis);
}

//
// FUNCTION
// ModKanjiCode::jisToEuc -- JISからEUCへの変換
//
// NOTES
// この関数はchar*で与えられた文字列をJISからEUCに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::jisToEuc(char* destination, const char* source,
					   ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::jis, ModKanjiCode::euc);
}

//
// FUNCTION
// ModKanjiCode::eucToShiftJis -- EUCからSHIFTJISへの変換
//
// NOTES
// この関数はchar*で与えられた文字列をEUCからSHIFTJISに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::eucToShiftJis(char* destination, const char* source,
							ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::euc, ModKanjiCode::shiftJis);
}

//
// FUNCTION
// ModKanjiCode::shiftJisToEuc -- SHIFTJISからEUCへの変換
//
// NOTES
// この関数はchar*で与えられた文字列をSHIFTJISからEUCに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::shiftJisToEuc(char* destination, const char* source,
							ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::shiftJis, ModKanjiCode::euc);
}

//
// FUNCTION
// ModKanjiCode::jisToShiftJis -- JISからSHIFTJISへの変換
//
// NOTES
// この関数はchar*で与えられた文字列をJISからSHIFTJISに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::jisToShiftJis(char* destination, const char* source,
							ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::jis, ModKanjiCode::shiftJis);
}

//
// FUNCTION
// ModKanjiCode::shiftJisToJis -- SHIFTJISからJISへの変換
//
// NOTES
// この関数はchar*で与えられた文字列をSHIFTJISからJISに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::shiftJisToJis(char* destination, const char* source,
							ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::shiftJis, ModKanjiCode::jis);
}

//
// FUNCTION
// ModKanjiCode::eucToWide -- EUCからModWideChar*への変換
//
// NOTES
// この関数はchar*で与えられた文字列をEUCからModWideChar*に変換するのに用いる。
//
// ARGUMENTS
// ModWideChar* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域の要素数
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::eucToWide(ModWideChar* destination, const char* source,
						ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::euc);
}

//
// FUNCTION
// ModKanjiCode::jisToWide -- JISからModWideChar*への変換
//
// NOTES
// この関数はchar*で与えられた文字列をJISからModWideChar*に変換するのに用いる。
//
// ARGUMENTS
// ModWideChar* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域の要素数
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::jisToWide(ModWideChar* destination, const char* source,
						ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::jis);
}

//
// FUNCTION
// ModKanjiCode::wideToShiftJis -- ModWideChar*からSHIFTJISへの変換
//
// NOTES
// この関数はModWideChar*で与えられた文字列をSHIFTJISに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const ModWideChar* source
//		変換する元のワイド文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const ModWideChar*
ModKanjiCode::wideToShiftJis(char* destination, const ModWideChar* source,
						ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::shiftJis);
}

//
// FUNCTION
// ModKanjiCode::shiftJisToWide -- SHIFTJISからModWideChar*への変換
//
// NOTES
// この関数はchar*で与えられた文字列をSHIFTJISからModWideChar*に変換するのに
// 用いる。
//
// ARGUMENTS
// ModWideChar* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域の要素数
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::shiftJisToWide(ModWideChar* destination, const char* source,
						ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::shiftJis);
}

#endif

//
// FUNCTION
// ModKanjiCode::toEuc -- EUCへの変換
//
// NOTES
// この関数はchar*で与えられた文字列を与えられた漢字コードから
// EUCに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
// KanjiCodeType inType
//		source の漢字コード
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::toEuc(char* destination, const char* source,
					ModSize sizeOfDestination, KanjiCodeType inType)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  inType, ModKanjiCode::euc);
}

//
// FUNCTION
// ModKanjiCode::toWide -- SHIFTJISからModWideChar*への変換
//
// NOTES
// この関数はchar*で与えられた文字列をSHIFTJISからModWideChar*に変換するのに
// 用いる。
//
// ARGUMENTS
// ModWideChar* destination
//		変換した文字列を格納するアドレス
// const char* source
//		変換する元の文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域の要素数
// KanjiCodeType inType
//		source の漢字コード
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const char*
ModKanjiCode::toWide(ModWideChar* destination, const char* source,
					 ModSize sizeOfDestination, KanjiCodeType inType)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  inType);
}

//
// FUNCTION
// ModKanjiCode::wideToJis -- ModWideChar*からJISへの変換
//
// NOTES
// この関数はModWideChar*で与えられた文字列をJISに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const ModWideChar* source
//		変換する元のワイド文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const ModWideChar*
ModKanjiCode::wideToJis(char* destination, const ModWideChar* source,
						ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::jis);
}

//
// FUNCTION
// ModKanjiCode::wideToEuc -- ModWideChar*からEUCへの変換
//
// NOTES
// この関数はModWideChar*で与えられた文字列をEUCに変換するのに用いる。
//
// ARGUMENTS
// char* destination
//		変換した文字列を格納するアドレス
// const ModWideChar* source
//		変換する元のワイド文字列を指すポインタ
// ModSize sizeOfDestination
//		destination として確保されている領域のバイト長
//
// RETURN
// source のうち変換できなかった部分の先頭を指すポインタを返す。
// すべて変換できたときは null-terminate 文字を指すポインタを返す。
//
// EXCEPTIONS
// なし
//
inline
const ModWideChar*
ModKanjiCode::wideToEuc(char* destination, const ModWideChar* source,
						ModSize sizeOfDestination)
{
	return ModKanjiCode::transfer(destination, source, sizeOfDestination,
								  ModKanjiCode::euc);
}

//
// FUNCTION bublic
// ModKanjiCode::jjTransfer -- 任意の文字コードから任意の文字コードへの変換
//
// NOTES
// ModKanjiCode::JJTransfer::transfer を呼び出しているだけである。
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
inline
char*
ModKanjiCode::jjTransfer(char*					outString,
						 const int				outBytes,
						 const KanjiCodeType	outType,
						 const char*			inString,
						 const KanjiCodeType	inType)
{
	// jj を包んだものを呼び出すだけ
	return JJTransfer::transfer(outString,
								outBytes,
								outType,
								inString,
								inType);
}

//
// FUNCTION pubblic
// ModKanjiCode::jjGetTransferdSize -- 各種漢字コードの変換後のバイト数
//
// NOTES
// 各種漢字コードの変換後のバイト数を取得。
// ModUncicodeChar の文字列(UCS-2 の文字列)は char* にキャストして渡すこと。
//
// ARGUMENTS
// const char*				src
//		文字列
// const KanjiCodeType		srcType
//		文字列の漢字コード種類
//	ModSize upper
//		指定されたとき
//			変換元文字列のうち、最大でも指定されたバイト数しか変換しない
//		指定されないとき
//			ModSizeMax が指定されたものとみなす
// const KanjiCodeType		dstType
//		変換後の文字列の漢字コード種類
//
// RETURN
// 各種漢字コードの変換後のバイト数(終端文字は含まない)
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		渡された漢字コードには対応していない
//
inline
ModSize
ModKanjiCode::jjGetTransferredSize(const char*				src,
								   const KanjiCodeType		srcType,
								   const KanjiCodeType		dstType)
{
	// JJ ライブラリの機能を呼び出す。引数の順番に注意。
	return JJTransfer::getTransferredSize(dstType, src, srcType, ModSizeMax);
}

inline
ModSize
ModKanjiCode::jjGetTransferredSize(const char*				src,
								   const KanjiCodeType		srcType,
								   ModSize					upper,
								   const KanjiCodeType		dstType)
{
	// JJ ライブラリの機能を呼び出す。引数の順番に注意。
	return JJTransfer::getTransferredSize(dstType, src, srcType, upper);
}

#endif	// __ModKanjiCode_H__

//
// Copyright (c) 1997, 2009, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
