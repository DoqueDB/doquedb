// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModUnicodeString.h -- ModUnicodeString のクラス定義
// 
// Copyright (c) 1999, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModUnicodeString_H__
#define __ModUnicodeString_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModUnicodeChar.h"
#include "ModUnicodeCharTrait.h"
#include "ModOsDriver.h"
#include "ModSerial.h"

//
// CLASS
// ModUnicodeString -- ModUnicodeChar を文字単位とした文字列を表現するクラス
//
// NOTES
// このクラスは UCS-2 を文字単位とした文字列を表現するのに用いる。
// また、UCS-2 文字は ModUnicodeChar 型で表現する。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModUnicodeString
	: public	ModSerializer,
	  public	ModDefaultObject
{
public:
	//
	// コンストラクタ
	//

	// コンストラクタ (1)
	ModUnicodeString();

	// コンストラクタ (2-1)
	ModUnicodeString(
		const char*					string,
		const ModSize				maxLength,
		const ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8);
	ModUnicodeString(
		const ModUnicodeChar*		string,
		const ModSize				maxLength);

	// コンストラクタ (2-2)
	ModUnicodeString(
		const char*					string,
		const ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8);
	ModUnicodeString(
		const ModUnicodeChar*		string);

	// コンストラクタ (3)
	ModUnicodeString(const char						c);
	ModUnicodeString(const ModUnicodeChar			c);
	ModUnicodeString(const ModUnicodeString&		src);

	// デストラクタ
	ModCommonDLL
	~ModUnicodeString();

	//
	// マニピュレータ
	//

	// = 演算オペレータ
	ModUnicodeString&		operator=(const ModUnicodeChar*		src);
	ModCommonDLL
	ModUnicodeString&		operator=(const ModUnicodeString&	src);

	// +=演算オペレータ
	ModUnicodeString& 		operator+=(const char				character);
	ModUnicodeString&		operator+=(const ModUnicodeChar		character);
	ModUnicodeString& 		operator+=(const ModUnicodeChar*	string);
	ModUnicodeString& 		operator+=(const ModUnicodeString&	string);

	// +演算オペレータ
	ModCommonDLL
	ModUnicodeString 		operator+(const ModUnicodeChar*		string);
	ModCommonDLL
	ModUnicodeString		operator+(const ModUnicodeString&	string);

	friend ModCommonDLL ModUnicodeString operator+(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModCommonDLL ModUnicodeString operator+(
		const char* string1, const ModUnicodeString& string2);

	// 文字(列)を末尾に付加する
	ModCommonDLL
	ModUnicodeString&		append(const char			character);
	ModCommonDLL
	ModUnicodeString&		append(const ModUnicodeChar	character);
	ModCommonDLL
	ModUnicodeString&		append(const ModUnicodeChar* string,
								   const ModSize		appendLength	= 0);
	ModCommonDLL
	ModUnicodeString&		append(const ModUnicodeString&	src,
								   const ModSize		appendLength	= 0);

	// 文字列の後ろをつめる
	void					truncate(const ModUnicodeChar* p);

	// 任意の文字を置換する
	void					replace(const ModUnicodeChar oldCharacter,
									const ModUnicodeChar newCharacter);

	// 内部状態を空文字列に相当する状態にする。
	void					clear();

	// "getNewString" で取得したバッファを解放する
	ModCommonDLL
	static void				freeNewString(char* newString_);

	// 内部バッファのサイズを拡張する
	ModCommonDLL
	void					reallocate(const ModSize len);

	// シリアライザ
	ModCommonDLL
	void					serialize(ModArchive& archiver);

	//
	// アクセッサ
	//

	// 任意のマルチバイト文字列を格納したバッファのアドレスを取得
	// (バッファの寿命管理は本クラスが暗黙の内に行なう)
	ModCommonDLL
	const char*				getString(
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8);

	// getString が返す文字列(終端文字を含む)を格納するのに充分な領域の
	// バイト数を返す。getString の実行直後に呼び出すこと。
	ModCommonDLL
	ModSize					getStringBufferSize() const;

	// 任意のマルチバイト文字列を格納したバッファを new して返す
	// (バッファを解放するには、本クラスの freeNewString を明示的に呼び出す)
	ModCommonDLL
	char*					getNewString(
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8) const;

	// 文字列の末尾である終端文字を格納する領域のアドレスを得る
	// (空文字列の場合はヌルポインターを返す)
	const ModUnicodeChar*	getTail() const;

	// ランダムアクセス (範囲検査なし)
	ModUnicodeChar&			operator[](const ModSize i);
	ModUnicodeChar&			operator[](const int i);
	const ModUnicodeChar&	operator[](const ModSize i) const;
	const ModUnicodeChar&	operator[](const int i) const;

	// 保持している Unicode 文字列の文字数を求める(nul 文字は除く)
	ModSize					getLength() const;

	// バッファ領域のサイズ(バイト数)を得る
	ModSize					getBufferSize() const;

	// const ModUnicodeChar* へのキャスト演算子
	operator				const ModUnicodeChar*() const;

	// 全体、または部分文字列のコピーを作る。
	// position、copyLength はオプショナル。
	// コピーする開始位置と長さを文字数で指定。
	ModCommonDLL
	ModUnicodeString		copy(const ModSize start	= 0,
								 const ModSize len		= 0) const;
	ModCommonDLL
	void					copy(ModUnicodeString& target,
								 const ModSize start	= 0,
								 const ModSize len		= 0) const;

	// ランダムアクセス (範囲検査あり)
	ModUnicodeChar&			at(const ModSize i);
	const ModUnicodeChar&	at(const ModSize i) const;


	// 1 文字(ASCII 英数字や Unicode 文字)を探す
	ModUnicodeChar*			search(const char v,
								   const ModBoolean caseSensitive = ModTrue);
	ModUnicodeChar*			search(const ModUnicodeChar v,
								   const ModBoolean caseSensitive = ModTrue);
	const ModUnicodeChar*	search(const char v,
								   const ModBoolean caseSensitive = ModTrue) const;
	const ModUnicodeChar*	search(const ModUnicodeChar v,
								   const ModBoolean caseSensitive = ModTrue) const;

	// Unicode 文字列(配列)を探す
	ModCommonDLL
	ModUnicodeChar*			search(const ModUnicodeChar* s,
								   const ModBoolean caseSensitive = ModTrue);
	const ModUnicodeChar*	search(const ModUnicodeChar* s,
								   const ModBoolean caseSensitive = ModTrue) const;

	// Unicode 文字列(クラス)を探す
	ModUnicodeChar*			search(const ModUnicodeString& s,
								   const ModBoolean caseSensitive = ModTrue);
	const ModUnicodeChar*	search(const ModUnicodeString& s,
								   const ModBoolean caseSensitive = ModTrue) const;

	// 末尾から文字(7bit の ASCII)を探す
	// 見つかればその最後の位置を指すポインタを返す。
	// 見つからなければ文字列の先頭を指すポインタを返す
	ModCommonDLL
	ModUnicodeChar*			rsearch(const char			character,
									const ModBoolean	caseFlag = ModTrue);


	// 比較関数
	// maxLength は比較の対象を先頭からの文字数で指定。

	// Unicode 文字列(クラス)と比較する
	int						compare(const ModUnicodeString& s) const;
	int						compare(const ModUnicodeString& s,
									const ModSize len) const;
	int						compare(const ModUnicodeString& s,
									const ModBoolean caseSensitive,
									const ModSize len = 0) const;

	// Unicode 文字列(ModUnicodeChar 配列)と比較する
	ModCommonDLL
	int						compare(const ModUnicodeChar* s) const;
	ModCommonDLL
	int						compare(const ModUnicodeChar* s,
									const ModSize	len) const;
	ModCommonDLL
	int						compare(const ModUnicodeChar* s,
									const ModBoolean caseSensitive,
									const ModSize	len = 0) const;

	// 比較オペレータ
	// 	右辺が ModUnicodeChar* または char* の場合はコンストラクタがあるので
	//  暗黙のうちに ModUnicodeString にキャストされる。
	//		# ModUnicodeChar* はキャストしなくてもいいように実装している
	//  これらが左辺にあってもコンパイルできるように２引数の演算子も実装する
	//	char* については SUN C++ Compiler V5.3 では以下のエラーが出るので、
	//	陽に演算子を定義する
	//			Error: Overloading ambiguity between "ModUnicodeString::operator==(const ModUnicodeString&) const" and "ModUnicodeString::operator const unsigned short*() const".
	ModBoolean				operator==(const ModUnicodeString& string) const;
	ModBoolean				operator==(const ModUnicodeChar* string) const;
	ModBoolean				operator==(const char* string) const;

	friend ModBoolean operator ==(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModBoolean operator ==(
		const char* string1, const ModUnicodeString& string2);

	ModBoolean				operator!=(const ModUnicodeString& string) const;
	ModBoolean				operator!=(const ModUnicodeChar* string) const;
	ModBoolean				operator!=(const char* string) const;

	friend ModBoolean operator !=(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModBoolean operator !=(
		const char* string1, const ModUnicodeString& string2);

	ModBoolean				operator<(const ModUnicodeString& string) const;
	ModBoolean				operator<(const ModUnicodeChar* string) const;
	ModBoolean				operator<(const char* string) const;

	friend ModBoolean operator <(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModBoolean operator <(
		const char* string1, const ModUnicodeString& string2);

	ModBoolean				operator>(const ModUnicodeString& string) const;
	ModBoolean				operator>(const ModUnicodeChar* string) const;
	ModBoolean				operator>(const char* string) const;

	friend ModBoolean operator >(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModBoolean operator >(
		const char* string1, const ModUnicodeString& string2);

	ModBoolean				operator<=(const ModUnicodeString& string) const;
	ModBoolean				operator<=(const ModUnicodeChar* string) const;
	ModBoolean				operator<=(const char* string) const;

	friend ModBoolean operator <=(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModBoolean operator <=(
		const char* string1, const ModUnicodeString& string2);

	ModBoolean				operator>=(const ModUnicodeString& string) const;
	ModBoolean				operator>=(const ModUnicodeChar* string) const;
	ModBoolean				operator>=(const char* string) const;

	friend ModBoolean operator >=(
		const ModUnicodeChar* string1, const ModUnicodeString& string2);
	friend ModBoolean operator >=(
		const char* string1, const ModUnicodeString& string2);

	ModSize					getAllocateStep() const;
												// バッファ領域の拡張単位を得る
	void					setAllocateStep(ModSize v);
												// バッファ領域の拡張単位を
												// 設定する

	// バッファの再確保(確保)と文字列のコピー (char* 版と Unicode 版)
	// (もともとは private だったが、 inline 関数から呼ばれているため、DLL に
	//  するとリンクエラーが起きた。そのため public に変更した。2000/05/30)
	ModCommonDLL
	void					allocateCopy(const char*			s,
										 const ModSize			len,
										 const ModKanjiCode::KanjiCodeType code);
	ModCommonDLL
	void					allocateCopy(const ModUnicodeChar*	s,
										 const ModSize			len);

private:

	// バッファの確保
	ModCommonDLL
	void					allocate(const ModSize len);

	// 英数字など (ASCII 7 bits)でない場合は例外を送出
	ModCommonDLL
	void					checkAsciiCharacter(const char character) const;

	// デフォルト漢字コードに対応したマルチバイト文字列
	//   通常は d_code だけ設定されている。

	//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

    class CharString
	{
	public:
		// あらゆる文字コードの終端文字の中でもバイト数が最大のもの
		// (単位:バイト)
		static const ModSize		s_biggestNulSize;

		// コンストラクタ、デストラクタ
		ModCommonDLL
		CharString(const ModKanjiCode::KanjiCodeType code);
		ModCommonDLL
		~CharString();

		//
		// マニピュレーター
		//
		void setCode(const ModKanjiCode::KanjiCodeType code);
		ModCommonDLL
		void free();

		//
		// アクセッサ
		//

		// マルチバイト文字列の現在の漢字コードを取得
		ModKanjiCode::KanjiCodeType getCode() const;

		// デフォルト漢字コードに変換したマルチバイト文字列のポインタ。
		// nul-terrminate している。
		const char* getBuffer(const ModUnicodeChar*	unicodeBufferBegin,
							  const ModUnicodeChar*	unicodeBufferEnd,
							  const ModKanjiCode::KanjiCodeType	code);
		// マルチバイト文字列(+終端文字)を格納しているバッファのサイズ
		// (単位はバイト)
		ModSize		getBufferSize() const;

	private:
		// デフォルト漢字コード。常に正しい値が設定されている。
		ModKanjiCode::KanjiCodeType	d_code;

		// UCS-2 文字列の変換結果を格納するマルチバイト文字列バッファ
		//・必要になるまで変換しない。
		//・変換元の UCS-2 文字列が NULL ならば d_buffer も NULL
		//・d_size を最新の値に更新してから変換を行なう
		//・d_buffer が NULL でない時、アロケートされている領域のサイズは
		//  絶対に d_size + 1 である(free で必要)
		//・変換元 UCS-2 文字列が更新されると破棄される。それまで破棄しない。
		char*						d_buffer;

		// マルチバイト文字列のバイト数(nul 文字は除く)
		//・必要になるまで計算しない。
		//・前に計算した値が残っている場合はその値を返す。
		//・変換元の UCS-2 文字列が NULL ならば d_size は 0
		//・d_buffer の値にはアクセスしない
		//・変換元 UCS-2 文字列が更新されるとクリアされる。それまでクリアしない
		ModSize						d_size;

		// (注意) d_buffer と d_size を必ずしも同時に更新する必要はない。
	};
	
	// UCS-2 文字列の内容を保持する配列
	// ・「空文字列状態」ではない場合であれば、null 文字も格納する
	ModUnicodeChar*			d_buffer;

	// 確保した領域の終わりを指すポインタ
	ModUnicodeChar*			d_endOfBuffer;

	// 文字列の終わりを指すポインタ
	// ・null 文字が存在すべきアドレスを指している
	// ・d_endOfString が d_buffer と同じ値であれば「空文字列状態」である
	ModUnicodeChar*			d_endOfString;

	// バッファ領域の拡張単位
	ModSize					d_allocateStep;

	// デフォルト漢字コードに対応したマルチバイト文字列
	CharString				d_charString;

	// バッファ領域の拡張単位のデフォルト値
	ModCommonDLL
	static const ModSize	s_allocateStepDefault;

	// 空文字列の時にバッファを返す時には、この NUL 文字を返す
	ModCommonDLL
	static const ModUnicodeChar	s_nul;
};

//
// FUNCTION 
// ModUnicodeString::ModUnicodeString -- デフォルトコンストラクタ
//
// NOTES
// ModUnicodeString のデフォルトコンストラクタ。
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModUnicodeString::ModUnicodeString()
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	; // do nothing
}

//
// FUNCTION 
// ModUnicodeString -- char*を引数としたコンストラクタ
//
// NOTES
// 引数にchar*をとるModUnicodeStringのコンストラクタ。
//
// ARGUMENTS
// char* string
//		初期化する文字列
// ModSize maxLength
//		初期化に使う文字数(バイト数ではない)。0 を渡すと全体を使う。
// ModKanjiCode::KanjiCodeType code
//		string に使われている漢字コード。
//		無指定時は UTF-8 が使用される。
//
// RETURN
//		なし
//
// EXCEPTIONS
//		なし
//
// その他
//		下位からの例外はそのまま再送出する。
//
inline
ModUnicodeString::ModUnicodeString(
	const char*							string,
	const ModSize						maxLength,
	const ModKanjiCode::KanjiCodeType	code /* = ModKanjiCode::utf8 */)
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	this->allocateCopy(string, maxLength, code);
}

//
// FUNCTION 
// ModUnicodeString -- ModUnicodeChar*を引数としたコンストラクタ
//
// NOTES
// 引数にModUnicodeChar*をとるModUnicodeStringのコンストラクタ。
//
// ARGUMENTS
// ModUnicodeChar* string
//		初期化する文字列
// ModSize maxLength
//		初期化に使う文字数(バイト数ではない)。 0 を渡すと全体を使う。
//
// RETURN
//		なし
//
// EXCEPTIONS
//		なし
//
// その他
//		下位からの例外はそのまま再送出する。
//
inline
ModUnicodeString::ModUnicodeString(
	const ModUnicodeChar*				string,
	const ModSize						maxLength)
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	this->allocateCopy(string, maxLength);
}

//
// FUNCTION 
// ModUnicodeString -- char*を引数としたコンストラクタ
//
// NOTES
// 引数にchar*をとるModUnicodeStringのコンストラクタ。
//
// ARGUMENTS
// char* string
//		初期化する文字列
// ModKanjiCode::KanjiCodeType code
//		string に使われている漢字コード。
//		無指定時は UTF-8 が使用される。
//
// RETURN
//		なし
//
// EXCEPTIONS
//		なし
//
// その他
//		下位からの例外はそのまま再送出する。
//
inline
ModUnicodeString::ModUnicodeString(
	const char*							string,
	const ModKanjiCode::KanjiCodeType	code /* = ModKanjiCode::utf8 */)
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	this->allocateCopy(string, 0, code);
}

//
// FUNCTION 
// ModUnicodeString -- ModUnicodeChar*を引数としたコンストラクタ
//
// NOTES
// 引数にModUnicodeChar*をとるModUnicodeStringのコンストラクタ。
//
// ARGUMENTS
// ModUnicodeChar* string
//		初期化する文字列
//
// RETURN
//		なし
//
// EXCEPTIONS
//		なし
//
// その他
//		下位からの例外はそのまま再送出する。
//
inline
ModUnicodeString::ModUnicodeString(const ModUnicodeChar*	string)
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	this->allocateCopy(string, 0);
}

//	FUNCTION public
//	ModUnicodeString::ModUnicodeString --
//		文字を初期値とする Unicode 文字列を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		char					c
//			生成される Unicode 文字列へ代入する ASCII 文字(英数字)
//		ModUnicodeChar			c
//			生成される Unicode 文字列へ代入する Unicode 文字
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModUnicodeString::ModUnicodeString(const char			character)
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	// 英数字など (ASCII 7 bits)でない場合は例外を送出
	checkAsciiCharacter(character);

	if (character == ModCharTrait::null()) {
		// 空文字列が格納されている状態にする
		this->clear();
	} else {
		// 1 文字(+ nul 文字)のために領域を確保し、ASCII文字(英数字)をバッファに
		// 格納
		this->allocate(1);
	//	*d_buffer = ModUnicodeCharTrait::makeUnicodeChar((char*)&character, code);
		*d_buffer = (ModUnicodeChar)character; // ASCII の前半はコード変換不要
		*(++d_endOfString) = ModUnicodeCharTrait::null();
	}
}

inline
ModUnicodeString::ModUnicodeString(const ModUnicodeChar		character)
	: d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(s_allocateStepDefault),
	  d_charString(ModKanjiCode::utf8)
{
	if (character == ModUnicodeCharTrait::null()) {
		// 空文字列が格納されている状態にする
		this->clear();
	} else {
		this->allocate(1);
		*d_buffer = character;
		*(++d_endOfString) = ModUnicodeCharTrait::null();
	}
}

//	FUNCTION public
//	ModUnicodeString::ModUnicodeString --
//		Unicode 文字列を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&		src
//			生成する Unicode 文字列へ代入する Unicode 文字列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModUnicodeString::ModUnicodeString(const ModUnicodeString&		src)
    : d_buffer(0),
	  d_endOfBuffer(0),
	  d_endOfString(0),
	  d_allocateStep(src.d_allocateStep),
	  d_charString(ModKanjiCode::utf8)
{
	if (src.d_buffer != 0) {
		ModSize	len = src.getLength();

		// 複写する文字数ぶんの領域を新規に確保し、
		// 空文字列が格納されていることにする

		this->allocate(len);

		// 与えられた文字列を末尾の ModUnicodeCharTrait::null を含めて複写する

		ModOsDriver::Memory::copy(d_buffer, src.d_buffer,
								  (len + 1) * sizeof(ModUnicodeChar));
		d_endOfString = d_buffer + len;
	}
}

// FUNCTION public
// ModUnicodeString::operator = -- = 演算子
//
// NOTES
// 自分自身への Unicode 文字配列を代入する
//
// ARGUMENTS
//	ModUnicodeChar*		src
//		自分自身へ代入するワイド文字配列
//		0 が指定されたとき、空文字列が代入される
//
// RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

inline
ModUnicodeString& 
ModUnicodeString::operator =(const ModUnicodeChar* src)
{
	this->allocateCopy(src, 0);
	return *this;
}

//
// FUNCTION
// ModUnicodeString::operator += -- ASCII 文字を文字列に追加する
//
// NOTES
// 文字列の最後にASCII文字(英数字)を一つ追加する。
//
// ARGUMENTS
// const char character
//		追加する文字
//
// RETURN
// 自身への参照を返す。
//
// EXCEPTIONS
// なし
//
// その他
//		下位からの例外はそのまま再送出する。
//

inline
ModUnicodeString&
ModUnicodeString::operator +=(const char character)
{
	// 英数字など (ASCII 7 bits)でない場合は例外を送出
	checkAsciiCharacter(character);
	return this->append(character);
}

//
// FUNCTION
// ModUnicodeString::operator+= -- ModUnicodeString を文字列に追加する
//
// NOTES
// 文字列の最後に文字列を追加する。
//
// ARGUMENTS
// const ModUnicodeString string
//		追加する文字列
//
// RETURN
//		自分自身への参照を返す。
//
// EXCEPTIONS
//		なし
//
// その他
//		下位からの例外はそのまま再送出する。
//

inline
ModUnicodeString&
ModUnicodeString::operator +=(const ModUnicodeString& string)
{
	return this->append(string);
}

//
// FUNCTION
// ModUnicodeString::operator+= -- ModUnicodeChar を文字列に追加する
//
// NOTES
// 文字列の最後にワイド文字を一つ追加する。
//
// ARGUMENTS
// const ModUnicodeChar character
//		追加する文字
//
// RETURN
//		自分自身への参照を返す。
//
// EXCEPTIONS
//		なし
// その他
//		下位からの例外はそのまま再送出する。
//

inline
ModUnicodeString&
ModUnicodeString::operator +=(const ModUnicodeChar character)
{
	return this->append(character);
}

//
// FUNCTION
// ModUnicodeString::operator+= -- ModUnicodeChar* を文字列に追加する
//
// NOTES
// 文字列の最後にModUnicodeChar*で表された文字列を追加する。
//
// ARGUMENTS
// const ModUnicodeChar* string
//		追加する文字列
//
// RETURN
//		自分自身への参照を返す。
//
// EXCEPTIONS
//		なし
//
// その他
//		下位からの例外はそのまま再送出する。
//

inline
ModUnicodeString&
ModUnicodeString::operator +=(const ModUnicodeChar* string)
{
	return this->append(string);
}

//	FUNCTION public
//	ModUnicodeString::truncate -- 文字列の後方を縮める
//
//	NOTES
//	文字列の後方を縮める
//
//	ARGUMENTS
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModUnicodeString::truncate(const ModUnicodeChar* p)
{
	if (d_buffer <= p && p < d_endOfString) {
		// Unicode のバッファが更新される時は、内容の矛盾が起きないように
		// マルチバイト用のバッファを破棄
		d_charString.free();

		*(d_endOfString = const_cast<ModUnicodeChar*>(p)) = ModUnicodeCharTrait::null();
	}
}


//	FUNCTION public
//	ModUnicodeString::replace -- 任意の文字を置換する
//
//	NOTES
//		任意の文字を置換する
//
//	ARGUMENTS
//	const ModUnicodeChar oldCharacter
//		置換する前の文字(ASCII : 7 bit)
//	const ModUnicodeChar oldCharacter
//		置換した後の文字(ASCII : 7 bit)
//	   
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModUnicodeString::replace(const ModUnicodeChar oldCharacter,
						  const ModUnicodeChar newCharacter)
{
	// Unicode のバッファが更新される時は、内容の矛盾が起きないように
	// マルチバイト用のバッファを破棄
	d_charString.free();

	ModReplace(d_buffer, d_endOfString, oldCharacter, newCharacter);
}


//	FUNCTION public
//	ModUnicodeString::clear -- 空文字列に相当する状態にする。
//
//	NOTES
//		格納している Unicode 文字列を空文字列に相当する状態にする。
//		具体的には d_endOfString と d_buffer が一致した状態である。
//		ただし、Unicode 文字列が格納されるバッファを破棄または縮小しない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
ModUnicodeString::clear()
{
	d_charString.free();

	// 格納文字列の先頭と終わりを一致させる
	if (d_endOfString = d_buffer) {
		// バッファが存在する場合は安全のために nul 文字を先頭に入れておく。
		// (バッファの縮小、破棄は行なわない)
		*d_endOfString = ModUnicodeCharTrait::null();
	}
}

//	FUNCTION public
//	ModUnicodeString::getLength -- 保持する Unicode 文字列の文字数を求める
//
//	NOTES
//		保持する Unicode 文字列のうち、
//		末尾の ModUnicodeCharTrait::null のぶんは除いたぶんの文字数を求める
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた文字数
//
//	EXCEPTIONS
//		なし

inline
const ModUnicodeChar*
ModUnicodeString::getTail() const
{
	return (d_endOfString) ? d_endOfString : &s_nul;
}


//	FUNCTION public
//	ModUnicodeString::getLength -- 保持する Unicode 文字列の文字数を求める
//
//	NOTES
//		保持する Unicode 文字列のうち、
//		末尾の ModUnicodeCharTrait::null のぶんは除いたぶんの文字数を求める
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めた文字数
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModUnicodeString::getLength() const
{
	return (ModSize) (d_endOfString - d_buffer);
}

//	FUNCTION public
//	ModUnicodeString::getBufferSize --
//		Unicode 文字列を格納するためのバッファ領域のサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファ領域のサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModUnicodeString::getBufferSize() const
{
	return (ModSize) (d_endOfBuffer - d_buffer) * sizeof(ModUnicodeChar);
}

// FUNCTION public
// ModUnicodeString::operator const ModUnicodeChar* -- const ModUnicodeChar* へのキャスト演算子
//
// NOTES
// 内部に持っている Unicode 文字列(ModUnicodeChar の配列)のアドレスを返す。
//
// ARGUMENTS
//		なし
//
// RETURN
// 内部に持っている Unicode 文字列(ModUnicodeChar の配列)のアドスレス。

inline
ModUnicodeString::operator const ModUnicodeChar*() const
{
	return (d_buffer) ? d_buffer : &s_nul;
}

//	FUNCTION public
//	ModUnicodeString::at -- 範囲検査付きランダムアクセス
//
//	NOTES
//		格納する文字列の先頭からなん文字目かの文字を得る
//
//	ARGUMENTS
//		ModSize				i
//			得たい文字が文字列の先頭からなん文字目か
//			先頭の文字は 0 文字目で、
//			最後の文字は ModUnicodeString::getLength() - 1 文字目になる
//		int					i
//			ModSize の場合と同様
//
//	RETURN
//		先頭から指定された数番目の文字
//
//	EXCEPTIONS
//		ModCommonErrorBadArgument
//			指定された数番目の文字はない

inline
ModUnicodeChar&
ModUnicodeString::at(const ModSize i)
{
	if (i >= this->getLength())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	// 呼び出し側でバッファ領域を更新するかもしれないので、
	// 矛盾が起きないようにマルチバイト文字列を解放しておく
	d_charString.free();

	return d_buffer[i];
}

inline
const ModUnicodeChar&
ModUnicodeString::at(const ModSize i) const
{
	if (i >= this->getLength())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return d_buffer[i];
}

//	FUNCTION public
//	ModUnicodeString::operator[] -- [] 演算子
//
//	NOTES
//		格納する文字列の先頭からなん文字目かの文字を得る
//		処理は速いが範囲チェックを行なわない
//
//	ARGUMENTS
//		ModSize				i
//			得たい文字が文字列の先頭からなん文字目か
//			先頭の文字は 0 文字目で、
//			最後の文字は ModUnicodeString::getLength() - 1 文字目になる
//		int					i
//			ModSize の場合と同様
//
//	RETURN
//		先頭から指定された数番目の文字
//
//	EXCEPTIONS
//		なし

inline
ModUnicodeChar&
ModUnicodeString::operator [](const ModSize i)
{
	// 呼び出し側でバッファ領域を更新するかもしれないので、
	// 矛盾が起きないようにマルチバイト文字列を解放しておく
	d_charString.free();

	return d_buffer[i];
}

inline
ModUnicodeChar&
ModUnicodeString::operator [](const int i)
{
	//【注意】	Visual C++ 6.0 では、引数が ModSize の [] 演算子に
	//			整数リテラルを引数に与えるとコンパイルエラーになるので、
	//			引数が int のものも定義する
	return d_buffer[i];
}

inline
const ModUnicodeChar&
ModUnicodeString::operator [](const ModSize i) const
{
	return d_buffer[i];
}

inline
const ModUnicodeChar&
ModUnicodeString::operator [](const int i) const
{
	//【注意】	Visual C++ 6.0 では、引数が ModSize の [] 演算子に
	//			整数リテラルを引数に与えるとコンパイルエラーになるので、
	//			引数が int のものも定義する
	return d_buffer[i];
}

//
// FUNCTION
// ModUnicodeString::operator== -- == オペレータ
//
// NOTES
// この関数は ModUnicodeString 同士の同値性を判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		比較対象の ModUnicodeString 型
// const ModUnicodeChar* string
//		比較対象の ModUnicodeChar* 型
// const char* string
//		比較対象の char* 型
//
// RETURN
// 両者の表現する文字列の内容が一致する場合は ModTrue を返し、
// 一致しない場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModUnicodeString::operator ==(const ModUnicodeString& string) const
{
	return (this->compare(string)) ? ModFalse : ModTrue;
}

inline
ModBoolean
ModUnicodeString::operator ==(const ModUnicodeChar* string) const
{
	return (this->compare(string)) ? ModFalse : ModTrue;
}

inline
ModBoolean
ModUnicodeString::operator ==(const char* string) const
{
	return (this->compare(ModUnicodeString(string))) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModUnicodeString::operator== -- ModUnicodeChar* を左辺にとる == オペレータ
//
// NOTES
// この関数は ModUnicodeChar* と ModUnicodeString の同値性を判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1
//		比較される char 配列へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// string1 と string2 の内容が一致する場合は ModTrue を返し、
// 一致しない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//		

inline
ModBoolean
operator ==(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return (string2.compare(string1)) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModUnicodeString::operator== -- char* を左辺にとる == オペレータ
//
// NOTES
// この関数は char* と ModUnicodeString の同値性を判定するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// string1 と string2 の内容が一致する場合は ModTrue を返し、
// 一致しない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//		

inline
ModBoolean
operator ==(const char* string1, const ModUnicodeString& string2)
{
	return (ModUnicodeString(string1).compare(string2)) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModUnicodeString::operator!= -- != オペレータ
//
// NOTES
// この関数は ModUnicodeString 同士の非同値性を判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		比較対象の ModUnicodeString 型
// const ModUnicodeChar* string
//		比較対象の ModUnicodeChar* 型
// const char* string
//		比較対象の char* 型
//
// RETURN
// 両者の表現する文字列の内容が一致しない場合は ModTrue を返し、
// 一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModUnicodeString::operator !=(const ModUnicodeString& string) const
{
	return (this->compare(string)) ? ModTrue : ModFalse;
}

inline
ModBoolean
ModUnicodeString::operator !=(const ModUnicodeChar* string) const
{
	return (this->compare(string)) ? ModTrue : ModFalse;
}

inline
ModBoolean
ModUnicodeString::operator !=(const char* string) const
{
	return (this->compare(ModUnicodeString(string))) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModUnicodeString::operator!= -- 2 引数の != オペレータ
//
// NOTES
// この関数は ModUnicodeChar* と ModUnicodeString の非同一性を判定するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1 (const char* string1)
//		比較される ModUnicodeChar 配列(char 配列)へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// string1 と string2 の内容が一致しない場合は ModTrue を返し、
// 一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
operator !=(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return (string2.compare(string1)) ? ModTrue : ModFalse;
}

inline
ModBoolean
operator !=(const char* string1, const ModUnicodeString& string2)
{
	return (string2.compare(ModUnicodeString(string1))) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModUnicodeString::operator< -- < オペレータ
//
// NOTES
// この関数は ModUnicodeString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		比較対象の ModUnicodeString 型
// const ModUnicodeChar* string
//		比較対象の ModUnicodeChar* 型
// const char* string
//		比較対象の char* 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string のものの方が大きい場合は ModTrue を返し、
// string のものの方が小さい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModUnicodeString::operator <(const ModUnicodeString& string) const
{
	return (this->compare(string) < 0) ? ModTrue : ModFalse;
}

inline
ModBoolean
ModUnicodeString::operator <(const ModUnicodeChar* string) const
{
	return (this->compare(string) < 0) ? ModTrue : ModFalse;
}

inline
ModBoolean
ModUnicodeString::operator <(const char* string) const
{
	return (this->compare(ModUnicodeString(string)) < 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModUnicodeString::operator< -- ２引数の < オペレータ
//
// NOTES
// この関数は ModUnicodeChar* と ModUnicodeString の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1
//		比較される char 配列へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string2 のものの方が大きい場合は ModTrue を返し、
// string2 のものの方が小さい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator <(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return (string2.compare(string1) > 0) ? ModTrue : ModFalse;
}

inline
ModBoolean
operator <(const char* string1, const ModUnicodeString& string2)
{
	return (ModUnicodeString(string1).compare(string2) < 0)
		? ModTrue : ModFalse;
}

//
// FUNCTION
// ModUnicodeString::operator> -- > オペレータ
//
// NOTES
// この関数は ModUnicodeString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		比較対象の ModUnicodeString 型
// const ModUnicodeChar* string
//		比較対象の ModUnicodeChar* 型
// const char* string
//		比較対象の char* 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string のものの方が小さい場合は ModTrue を返し、
// string のものの方が大きい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModUnicodeString::operator >(const ModUnicodeString& string) const
{
	return (this->compare(string) > 0) ? ModTrue : ModFalse;
}

inline
ModBoolean
ModUnicodeString::operator >(const ModUnicodeChar* string) const
{
	return (this->compare(string) > 0) ? ModTrue : ModFalse;
}

inline
ModBoolean
ModUnicodeString::operator >(const char* string) const
{
	return (this->compare(ModUnicodeString(string)) > 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModUnicodeString::operator> -- 2引数の > オペレータ
//
// NOTES
// この関数は ModUnicodeChar* と ModUnicodeString の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1
//		比較される ModUnicodeChar 配列へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string2 のものの方が小さい場合は ModTrue を返し、
// string2 のものの方が大きい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator >(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return (string2.compare(string1) < 0) ? ModTrue : ModFalse;
}

inline
ModBoolean
operator >(const char* string1, const ModUnicodeString& string2)
{
	return (ModUnicodeString(string1).compare(string2) > 0)
		? ModTrue : ModFalse;
}

//
// FUNCTION
// ModUnicodeString::operator<= -- <= オペレータ
//
// NOTES
// この関数は ModUnicodeString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		比較対象の ModUnicodeString 型
// const ModUnicodeChar* string
//		比較対象の ModUnicodeChar* 型
// const char* string
//		比較対象の char* 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string のものの方が大きい場合かすべて一致する場合は
// ModTrue を返し、string のものの方が小さい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModUnicodeString::operator <=(const ModUnicodeString& string) const
{
	return (this->compare(string) > 0) ? ModFalse : ModTrue;
}

inline
ModBoolean
ModUnicodeString::operator <=(const ModUnicodeChar* string) const
{
	return (this->compare(string) > 0) ? ModFalse : ModTrue;
}

inline
ModBoolean
ModUnicodeString::operator <=(const char* string) const
{
	return (this->compare(ModUnicodeString(string)) > 0) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModUnicodeString::operator<= -- 2引数の <= オペレータ
//
// NOTES
// この関数は ModUnicodeChar* と ModUnicodeString の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1
//		比較される ModUnicodeChar 配列へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string2 のものの方が大きい場合かすべて一致する場合は
// ModTrue を返し、string2 のものの方が小さい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator <=(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return (string2.compare(string1) < 0) ? ModFalse : ModTrue;
}


inline
ModBoolean
operator <=(const char* string1, const ModUnicodeString& string2)
{
	return (ModUnicodeString(string1).compare(string2) > 0)
		? ModFalse : ModTrue;
}

//
// FUNCTION
// ModUnicodeString::operator>= -- >= オペレータ
//
// NOTES
// この関数は ModUnicodeString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeString& string
//		比較対象の ModUnicodeString 型
// const ModUnicodeChar* string
//		比較対象の ModUnicodeChar* 型
// const char* string
//		比較対象の char* 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string のものの方が小さい場合かすべて一致する場合は
// ModTrue を返し、string のものの方が大きい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModUnicodeString::operator >=(const ModUnicodeString& string) const
{
	return (this->compare(string) < 0) ? ModFalse : ModTrue;
}

inline
ModBoolean
ModUnicodeString::operator>=(const ModUnicodeChar* string) const
{
	return (this->compare(string) < 0) ?  ModFalse : ModTrue;
}

inline
ModBoolean
ModUnicodeString::operator>=(const char* string) const
{
	return (this->compare(ModUnicodeString(string)) < 0) ?  ModFalse : ModTrue;
}

//
// FUNCTION
// ModUnicodeString::operator>= -- 2引数の >= オペレータ
//
// NOTES
// この関数は ModUnicodeChar* と ModUnicodeString の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModUnicodeChar* string1
//		比較される ModUnicodeChar 配列へのポインタ
// const ModUnicodeString string2
//		比較する ModUnicodeString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// Unicode 文字コードが string2 のものの方が小さい場合かすべて一致する場合は
// ModTrue を返し、string2 のものの方が大きい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator >=(const ModUnicodeChar* string1, const ModUnicodeString& string2)
{
	return (string2.compare(string1) > 0) ? ModFalse : ModTrue;
}


inline
ModBoolean
operator >=(const char* string1, const ModUnicodeString& string2)
{
	return (ModUnicodeString(string1).compare(string2) < 0)
		?  ModFalse : ModTrue;
}

//	FUNCTION public
//	ModUnicodeString::compare -- Unicode 文字列(クラス)と比較する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&		s
//			自分自身と比較する文字列
//		ModBoolean			caseSensitive
//			ModTrue または指定されないとき
//				大文字、小文字を区別する
//			ModFalse
//				大文字、小文字を区別しない
//		ModSize				len
//			0 以外
//				先頭から比較する最大文字数
//			0 または指定されないとき
//				文字列全体を比較する
//
//	RETURN
//		1
//			最初に異なるワイド文字が
//			与えられたものより自分自身のもののほうが大きい
//		0
//			自分自身と与えられたもののすべての文字が等しい
//		-1
//			最初に異なる文字が
//			自分自身のものより与えられたもののほうが大きい
//
//	EXCEPTIONS
//		なし

inline
int
ModUnicodeString::compare(const ModUnicodeString& s) const
{
	// (注意) 本クラスをconst ModUnicodeChar* にキャストすると UCS-2 文字列の
	//        バッファのアドレスが得られる
	return this->compare((const ModUnicodeChar*)s);
}

inline
int
ModUnicodeString::compare(const ModUnicodeString& s, const ModSize len) const
{
	// (注意) 本クラスをconst ModUnicodeChar* にキャストすると UCS-2 文字列の
	//        バッファのアドレスが得られる
	return this->compare((const ModUnicodeChar*)s, len);
}

inline
int
ModUnicodeString::compare(const ModUnicodeString& s,
						  const ModBoolean caseSensitive,
						  const ModSize len) const
{
	// (注意) 本クラスをconst ModUnicodeChar* にキャストすると UCS-2 文字列の
	//        バッファのアドレスが得られる
	return this->compare((const ModUnicodeChar*)s, caseSensitive, len);
}

//	FUNCTION public
//	ModUnicodeString::search -- ASCII 文字(英数字)や Unicode 文字を探す
//
//	NOTES
//
//	ARGUMENTS
//		char				v
//			探す ASCII 文字(英数字)
//		ModUnicodeChar			v
//			探すUnicode文字
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//
//	RETURN
//		0 以外の値
//			見つかったUnicode文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
ModUnicodeChar*
ModUnicodeString::search(const char			character,
						 const ModBoolean	caseSensitive)
{
	// 英数字など (ASCII 7 bits)でない場合は例外を送出
	checkAsciiCharacter(character);
	// ASCII のコード表の前半は UCS-2 のコード表と一致するのでキャスト可能
	ModUnicodeChar unicodeChar = (ModUnicodeChar)character;

	return ModUnicodeCharTrait::find(d_buffer, unicodeChar, caseSensitive);
}

inline
const ModUnicodeChar*
ModUnicodeString::search(const char			character,
						 const ModBoolean	caseSensitive) const
{
	// 英数字など (ASCII 7 bits)でない場合は例外を送出
	checkAsciiCharacter(character);
	// ASCII のコード表の前半は UCS-2 のコード表と一致するのでキャスト可能
	ModUnicodeChar unicodeChar = (ModUnicodeChar)character;

	return ModUnicodeCharTrait::find(d_buffer, unicodeChar, caseSensitive);
}

inline
ModUnicodeChar*
ModUnicodeString::search(const ModUnicodeChar v, const ModBoolean caseSensitive)
{
	return ModUnicodeCharTrait::find(d_buffer, v, caseSensitive);
}

inline
const ModUnicodeChar*
ModUnicodeString::search(const ModUnicodeChar v, const ModBoolean caseSensitive) const
{
	return ModUnicodeCharTrait::find(d_buffer, v, caseSensitive);
}

//	FUNCTION public
//	ModUnicodeString::search -- Unicode 文字列を探す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&		s
//			探すUnicode文字列
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//
//	RETURN
//		0 以外の値
//			見つかったUnicode文字列の先頭の文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
ModUnicodeChar*
ModUnicodeString::search(const ModUnicodeString& s, const ModBoolean caseSensitive)
{
	// (注意) 本クラスをconst ModUnicodeChar* にキャストすると UCS-2 文字列の
	//        バッファのアドレスが得られる
	return this->search((const ModUnicodeChar*)s, caseSensitive);
}

inline
const ModUnicodeChar*
ModUnicodeString::search(const ModUnicodeString& s, const ModBoolean caseSensitive) const
{
	// (注意) 本クラスをconst ModUnicodeChar* にキャストすると UCS-2 文字列の
	//        バッファのアドレスが得られる
	return this->search((const ModUnicodeChar*)s, caseSensitive);
}

//	FUNCTION public
//	ModUnicodeString::search -- Unicode 文字配列を探す
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeChar*		s
//			探す Unicode 文字配列
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//
//	RETURN
//		0 以外の値
//			見つかったUnicode 文字配列の先頭の文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
const ModUnicodeChar*
ModUnicodeString::search(const ModUnicodeChar*	s,
						 const ModBoolean		caseSensitive) const
{
	return const_cast<ModUnicodeString*>(this)->search(s, caseSensitive);
}

//	FUNCTION public
//	ModUnicodeString::getAllocateStep --
//		文字列を格納するバッファ領域の拡張単位を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバッファ領域の拡張単位(単位 B)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModUnicodeString::getAllocateStep() const
{
	return d_allocateStep;
}

//	FUNCTION public
//	ModUnicodeString::setAllocateStep --
//		文字列を格納するバッファ領域の拡張単位を変更する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				v
//			新しい値(単位 B)
//			0 を指定すると、ModUnicodeString::_allocateStepDefault に変更される
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModUnicodeString::setAllocateStep(ModSize v)
{
	d_allocateStep =	(v) ? v : s_allocateStepDefault;
}

#endif	// __ModUnicodeString_H__

//
// Copyright (c) 1999, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
