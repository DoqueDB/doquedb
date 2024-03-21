// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWideString.h -- ModWideString のクラス定義
// 
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModWideString_H__
#define __ModWideString_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModWideChar.h"
#include "ModWideCharTrait.h"
#include "ModSerial.h"
#include "ModOsDriver.h"

//
// CLASS
// ModWideString -- ModWideChar を文字単位とした文字列を表現するクラス
//
// NOTES
// このクラスは ModWideChar を文字単位とした文字列を表現するのに用いる。
//
// ModPureWideStringに変更して、サブクラスとしてメモリハンドルクラスを
// 明示したクラスを作成するには、operator=()の再定義や、copyなど
// オブジェクトを返すメソッドへの対応が必要なことがわかったので断念し、
// 直接、ModDefaultObjectのサブクラスとして作成する。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModWideString
	: public	ModSerializer,
	  public	ModDefaultObject
{
public:
	// コンストラクタ (1)
	ModWideString();
	// コンストラクタ (2)
	ModWideString(const char* string, const ModSize maxLength = 0,
				  ModKanjiCode::KanjiCodeType kanjiCode = ModKanjiCode::euc);
	// コンストラクタ (2-2)
	ModWideString(const char* string, ModKanjiCode::KanjiCodeType kanjiCode);
	// コンストラクタ (3)
	ModWideString(const ModWideChar* string, const ModSize maxLength = 0);
	// コンストラクタ (4)
	ModWideString(char c);
	ModWideString(ModWideChar c);
	ModWideString(const ModWideString& src);
	// デストラクタ
	~ModWideString();

	ModCommonDLL
	void					serialize(ModArchive& archiver);
												// シリアル化する

	ModCommonDLL
	ModWideString&			operator =(const ModWideString& src);
	ModWideString&			operator =(const char* src);
	ModWideString&			operator =(const ModWideChar* src);
												// = 演算子

	// +=演算オペレータ
	ModWideString& operator+=(const char character);
	ModWideString& operator+=(const ModWideString& string);
	ModWideString& operator+=(const char* string);
	ModWideString& operator+=(const ModWideChar character);
	ModWideString& operator+=(const ModWideChar* string);

	ModCommonDLL
	ModWideString&			append(char src);
	ModCommonDLL
	ModWideString&			append(ModWideChar src);
												// 文字を付加する

	ModCommonDLL
	ModWideString&			append(const ModWideString& src,
								   ModSize len = 0);
	ModCommonDLL
	ModWideString& append(
		const char* string, const ModSize appendLength = 0,
		ModKanjiCode::KanjiCodeType kanjiCode = ModKanjiCode::euc
		);
	ModCommonDLL
	ModWideString& append(const ModWideChar* string,
						  const ModSize appendLength = 0);
												// 文字列を付加する

	void					clear();			// 空文字列を代入する

	ModSize					getSize() const;	// 保持しているワイド文字列を
												// EUC で表現するのに
												// 必要なバイト数を求める
	ModSize					getLength() const;	// 保持しているワイド文字列の
												// 長さを求める

	ModWideChar*			getBuffer();
	const ModWideChar*		getBuffer() const;	// バッファ領域を得る
	ModCommonDLL
	const char*				getString(ModKanjiCode::KanjiCodeType code =
														  ModKanjiCode::euc);
												// C 文字列を得る

	operator				const char*();		// const char* への
												// キャスト演算子

	ModWideChar&			operator[](ModSize i);
	ModWideChar&			operator[](int i);
	const ModWideChar&		operator[](ModSize i) const;
	const ModWideChar&		operator[](int i) const;
												// [] 演算子

	// 比較オペレータ
	ModBoolean operator==(const ModWideString& string) const;
	ModBoolean operator==(const char* string) const;
	ModCommonDLL
	ModBoolean operator==(const ModWideChar* string) const;

	friend ModBoolean
		operator ==(const char* string1, const ModWideString& string2);
	friend ModCommonDLL ModBoolean
		operator ==(const ModWideChar* string1, const ModWideString& string2);

	ModBoolean operator!=(const ModWideString& string) const;
	ModBoolean operator!=(const char* string) const;
	ModCommonDLL
	ModBoolean operator!=(const ModWideChar* string) const;

	friend ModBoolean
		operator !=(const char* string1, const ModWideString& string2);
	friend ModCommonDLL ModBoolean
		operator !=(const ModWideChar* string1, const ModWideString& string2);

	ModBoolean operator<(const ModWideString& string) const;
	ModBoolean operator<(const char* string) const;
	ModCommonDLL
	ModBoolean operator<(const ModWideChar* string) const;

	friend ModBoolean
		operator <(const char* string1, const ModWideString& string2);
	friend ModCommonDLL ModBoolean
		operator <(const ModWideChar* string1, const ModWideString& string2);

	ModBoolean operator>(const ModWideString& string) const;
	ModBoolean operator>(const char* string) const;
	ModCommonDLL
	ModBoolean operator>(const ModWideChar* string) const;

	friend ModBoolean
		operator >(const char* string1,	const ModWideString& string2);
	friend ModCommonDLL ModBoolean
		operator >(const ModWideChar* string1, const ModWideString& string2);

	ModBoolean operator<=(const ModWideString& string) const;
	ModBoolean operator<=(const char* string) const;
	ModCommonDLL
	ModBoolean operator<=(const ModWideChar* string) const;

	friend ModBoolean
		operator <=(const char* string1, const ModWideString& string2);
	friend ModCommonDLL ModBoolean
		operator <=(const ModWideChar* string1, const ModWideString& string2);

	ModBoolean operator>=(const ModWideString& string) const;
	ModBoolean operator>=(const char* string) const;
	ModCommonDLL
	ModBoolean operator>=(const ModWideChar* string) const;

	friend ModBoolean
		operator >=(const char* string1, const ModWideString& string2);
	friend ModCommonDLL ModBoolean
		operator >=(const ModWideChar* string1, const ModWideString& string2);

	// +演算オペレータ
	ModCommonDLL
	ModWideString operator+(const ModWideString& string);
	ModCommonDLL
	ModWideString operator+(const char* string);
	ModCommonDLL
	ModWideString operator+(const ModWideChar* string);

	friend ModCommonDLL ModWideString
		operator +(const char* string1, const ModWideString& string2);
	friend ModCommonDLL ModWideString
		operator+(const ModWideChar* string1, const ModWideString& string2);

	// 文字列操作
	// 全体、または部分文字列のコピーを作る。
	// position、copyLength はオプショナル。
	// コピーする開始位置と長さを文字数で指定。

	ModCommonDLL
	ModWideString			copy(ModSize start = 0, ModSize len = 0) const;
												// 部分ワイド文字列の取得

	// 比較関数
	// maxLength は比較の対象を先頭からの文字数で指定。

	int						compare(const ModWideString& s,
									ModSize len = 0) const;
	int						compare(const ModWideString& s,
									ModBoolean caseSensitive,
									ModSize len = 0) const;
												// ワイド文字列と比較する
	int						compare(const char* s,
									ModKanjiCode::KanjiCodeType code =
											ModKanjiCode::euc) const;
	ModCommonDLL
	int						compare(const char* s, ModSize len,
									ModKanjiCode::KanjiCodeType code =
											ModKanjiCode::euc) const;
	int						compare(const char* s, ModBoolean caseSensitive,
									ModKanjiCode::KanjiCodeType code =
											ModKanjiCode::euc) const;
	ModCommonDLL
	int						compare(const char* s, ModBoolean caseSensitive,
									ModSize len,
									ModKanjiCode::KanjiCodeType code =
											ModKanjiCode::euc) const;
												// C 文字列と比較する
	ModCommonDLL
	int						compare(const ModWideChar* s,
									ModSize len = 0) const;
	ModCommonDLL
	int						compare(const ModWideChar* s,
									ModBoolean caseSensitive,
									ModSize len = 0) const;
												// ワイド文字配列と比較する

	ModWideChar*			search(char v,
								   ModBoolean caseSensitive = ModTrue);
	ModWideChar*			search(ModWideChar v,
								   ModBoolean caseSensitive = ModTrue);
	const ModWideChar*		search(char v,
								   ModBoolean caseSensitive = ModTrue) const;
	const ModWideChar*		search(ModWideChar v,
								   ModBoolean caseSensitive = ModTrue) const;
												// ASCII 文字やワイド文字を探す
	ModWideChar*			search(const ModWideString& s,
								   ModBoolean caseSensitive = ModTrue);
	const ModWideChar*		search(const ModWideString& s,
								   ModBoolean caseSensitive = ModTrue) const;
												// ワイド文字列を探す
	ModCommonDLL
	ModWideChar*			search(const char* s,
								   ModBoolean caseSensitive = ModTrue,
								   ModKanjiCode::KanjiCodeType code =
											 	ModKanjiCode::euc);
	const ModWideChar*		search(const char* s,
								   ModBoolean caseSensitive = ModTrue,
								   ModKanjiCode::KanjiCodeType code =
												ModKanjiCode::euc) const;
												// C 文字列を探す
	ModCommonDLL
	ModWideChar*			search(const ModWideChar* s,
								   ModBoolean caseSensitive = ModTrue);
	const ModWideChar*		search(const ModWideChar* s,
								   ModBoolean caseSensitive = ModTrue) const;
												// ワイド文字配列を探す

	ModSize					getAllocateStep() const;
												// バッファ領域の拡張単位を得る
	void					setAllocateStep(ModSize v);
												// バッファ領域の拡張単位を
												// 設定する
private:
	ModCommonDLL
	void					allocateCopy(const char* s, ModSize len = 0,
										 ModKanjiCode::KanjiCodeType code =
											 ModKanjiCode::euc);
	ModCommonDLL
	void					allocateCopy(const ModWideChar* s,
										 ModSize len = 0);
												// 文字列を代入する

	ModCommonDLL
	void					allocate(ModSize len);
												// バッファ領域を確保する
	void					reallocate(ModSize len);
												// バッファ領域を再確保する

	ModSize					getBufferSize() const;
												// バッファ領域のサイズを得る

	ModSize					_allocateStep;		// バッファ領域の拡張単位
	ModCommonDLL
	static const ModSize	_allocateStepDefault;
												// バッファ領域の拡張単位の
												// デフォルト値

	ModWideChar* buffer;				// 文字列の内容を保持する配列
	ModWideChar* endOfBuffer;			// 確保した領域の終わりを指すポインタ
	ModWideChar* endOfString;			// 文字列の終わりを指すポインタ

	ModSize					_size;				// 保持しているワイド文字列を
												// EUC で表現するのに
												// 必要なバイト数
	char* charBuffer;					// マルチバイト文字列を入れる配列
										// **このメンバは buffer と常に整合性が
										// **とれていることは保証されない。
};

//
// FUNCTION 
// ModWideString::ModWideString -- デフォルトコンストラクタ
//
// NOTES
// ModWideString のデフォルトコンストラクタ。
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
// その他
// ModStandardManager::allocate の例外参照
//

inline
ModWideString::ModWideString()
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(0),
	  _allocateStep(_allocateStepDefault)
{
	this->allocate(0);
}

//
// FUNCTION 
// ModWideString -- char*を引数としたコンストラクタ
//
// NOTES
// 引数にchar*をとるModWideStringのコンストラクタ。
//
// ARGUMENTS
// const char* string
//		初期化する文字列
// const ModSize maxLength
//		初期化に使う文字数(バイト数ではない)。省略するか 0 を渡すと全体を使う。
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
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
ModWideString::ModWideString(const char* string, const ModSize maxLength,
							 ModKanjiCode::KanjiCodeType kanjiCode)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(0),
	  _allocateStep(_allocateStepDefault)
{
	this->allocateCopy(string, maxLength, kanjiCode);
}

//
// FUNCTION 
// ModWideString -- char*を引数としたコンストラクタ(2)
//
// NOTES
// 引数にchar*をとるModWideStringのコンストラクタ。
//
// ARGUMENTS
// const char* string
//		初期化する文字列
// ModKanjiCode::KanjiCodeType kanjiCode
//		string に使われている漢字コード。
//		無指定時は EUC が使用される。
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
ModWideString::ModWideString(const char* string,
							 ModKanjiCode::KanjiCodeType kanjiCode)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(0),
	  _allocateStep(_allocateStepDefault)
{
	this->allocateCopy(string, 0, kanjiCode);
}

//
// FUNCTION 
// ModWideString -- ModWideChar*を引数としたコンストラクタ
//
// NOTES
// 引数にModWideChar*をとるModWideStringのコンストラクタ。
//
// ARGUMENTS
// const ModWideChar* string
//		初期化する文字列
// const ModSize maxLength
//		初期化に使う文字数。省略するか 0 を渡すと全体を使う。
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
ModWideString::ModWideString(const ModWideChar* string,
							 const ModSize maxLength)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(0),
	  _allocateStep(_allocateStepDefault)
{
	this->allocateCopy(string, maxLength);
}

//	FUNCTION public
//	ModWideString::ModWideString --
//		文字を初期値とするワイド文字列を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		char				c
//			生成されるワイド文字列へ代入する ASCII 文字
//		ModWideChar			c
//			生成されるワイド文字列へ代入するワイド文字
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModWideString::ModWideString(char c)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(1),
	  _allocateStep(_allocateStepDefault)
{
	this->allocate(1);

	const char	tmp[] = { c, 0 };
	*this->buffer = ModWideCharTrait::makeWideChar(tmp);
	*(++this->endOfString) = ModWideCharTrait::null();
}

inline
ModWideString::ModWideString(ModWideChar c)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(ModWideCharTrait::byteSize(c)),
	  _allocateStep(_allocateStepDefault)
{
	this->allocate(1);

	*this->buffer = c;
	*(++this->endOfString) = ModWideCharTrait::null();
}

//	FUNCTION public
//	ModWideString::ModWideString --
//		ワイド文字列を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModWideString&		src
//			生成するワイド文字列へ代入するワイド文字列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModWideString::ModWideString(const ModWideString& src)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  charBuffer(0),
	  _size(src.getSize()),
	  _allocateStep(_allocateStepDefault)
{
	ModSize	len = src.getLength();

	// 複写する文字数ぶんの領域を新規に確保し、
	// 空文字列が格納されていることにする

	this->allocate(len);

	// 与えられた文字列を末尾の ModWideCharTrait::null を含めて複写する

	ModOsDriver::Memory::copy(this->buffer, src.buffer,
							  (len + 1) * sizeof(ModWideChar));
	this->endOfString = this->buffer + len;
}

//
// FUNCTION 
// ModWideString::~ModWideString -- デストラクタ
//
// NOTES
// ModWideStringのデストラクタ
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
// その他
// ModStandardManager::free の例外参照
//

inline
ModWideString::~ModWideString()
{
	if (this->buffer) {
		ModStandardManager::free(this->buffer, this->getBufferSize());
		this->buffer = 0;
	}
	if (this->charBuffer) {
		ModStandardManager::free(this->charBuffer,
								 (this->getSize() + 1) * sizeof(char));
		this->charBuffer = 0;
	}
}

//	FUNCTION public
//	ModWideString::operator = -- = 演算子
//
//	NOTES
//		自分自身へ C 文字列またはワイド文字配列を代入する
//
//	ARGUMENTS
//		char*				src
//			自分自身へ代入する C 文字列
//			0 が指定されたとき、空文字列が代入される
//		ModWideChar*		src
//			自分自身へ代入するワイド文字配列
//			0 が指定されたとき、空文字列が代入される
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

inline
ModWideString& 
ModWideString::operator =(const char* src)
{
	this->allocateCopy(src);
	return *this;
}

inline
ModWideString& 
ModWideString::operator =(const ModWideChar* src)
{
	this->allocateCopy(src);
	return *this;
}

//
// FUNCTION
// ModWideString::operator += -- ASCII 文字を文字列に追加する
//
// NOTES
// 文字列の最後にASCII文字を一つ追加する。
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
ModWideString&
ModWideString::operator +=(const char character)
{
	return this->append(character);
}

//
// FUNCTION
// ModWideString::operator+= -- ModWideString を文字列に追加する
//
// NOTES
// 文字列の最後に文字列を追加する。
//
// ARGUMENTS
// const ModWideString string
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
ModWideString&
ModWideString::operator +=(const ModWideString& string)
{
	return this->append(string);
}

//
// FUNCTION
// ModWideString::operator+= -- char* を文字列に追加する
//
// NOTES
// 文字列の最後にchar*で表された文字列を追加する。
//
// ARGUMENTS
// const char* string
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
ModWideString&
ModWideString::operator +=(const char* string)
{
	return this->append(string);
}

//
// FUNCTION
// ModWideString::operator+= -- ModWideChar を文字列に追加する
//
// NOTES
// 文字列の最後にワイド文字を一つ追加する。
//
// ARGUMENTS
// const ModWideChar character
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
ModWideString&
ModWideString::operator +=(const ModWideChar character)
{
	return this->append(character);
}

//
// FUNCTION
// ModWideString::operator+= -- ModWideChar* を文字列に追加する
//
// NOTES
// 文字列の最後にModWideChar*で表された文字列を追加する。
//
// ARGUMENTS
// const ModWideChar* string
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
ModWideString&
ModWideString::operator +=(const ModWideChar* string)
{
	return this->append(string);
}

//	FUNCTION public
//	ModWideString::clear -- ワイド文字列を格納していれば、空文字列を格納する
//
//	NOTES
//		ワイド文字列を格納していれば、そのワイド文字列は破棄し、
//		空文字列を格納する
//		ただし、ワイド文字列を格納するためのバッファ領域を破棄または縮小しない
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
ModWideString::clear()
{
	if (this->charBuffer) {
		ModStandardManager::free(this->charBuffer,
								 (this->getSize() + 1) * sizeof(char));
		this->charBuffer = 0;
	}

	// 格納文字列の先頭と終わりを一致させる

	if (this->endOfString = this->buffer) {
		*this->endOfString = ModWideCharTrait::null();
		_size = 0;
	}
}

//	FUNCTION public
//	ModWideString::getSize --
//		保持するワイド文字列を EUC で表現するのに必要なサイズを求める
//
//	NOTES
//		保持するワイド文字列を EUC で表現するのに必要なサイズを求める
//		ただし、末尾の ModCharTrait::null のぶんは除く
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		求めたサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModWideString::getSize() const
{
	return _size;
}

//	FUNCTION public
//	ModWideString::getLength -- 保持するワイド文字列の長さを求める
//
//	NOTES
//		保持するワイド文字列のうち、
//		末尾の ModWideCharTrait::null のぶんは除いたぶんの文字数を求める
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
ModWideString::getLength() const
{
	return (ModSize) (this->endOfString - this->buffer);
}

//	FUNCTION private
//	ModWideString::getBufferSize --
//		ワイド文字列を格納するためのバッファ領域のサイズを得る
//
//	NOTES
//		ワイド文字列を格納するためのバッファ領域のサイズで、
//		ModWideString::getSize + sizeof(char) 以上である
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
ModWideString::getBufferSize() const
{
	return (ModSize) (this->endOfBuffer - this->buffer) * sizeof(ModWideChar);
}

//	FUNCTION public
//	ModWideString::getBuffer --
//		ModWideChar 配列を格納するためのバッファ領域を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModWideChar 配列を格納するためのバッファ領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
ModWideChar* 
ModWideString::getBuffer()
{
	if (this->charBuffer) {

		// 呼び出し側でバッファ領域を更新するかもしれないので、
		// バッファ領域と同値の C 文字列を解放しておく

		ModStandardManager::free(this->charBuffer,
								 (this->getSize() + 1) * sizeof(char));
		this->charBuffer = 0;
	}
	return this->buffer;
}

inline
const ModWideChar* 
ModWideString::getBuffer() const
{
	return this->buffer;
}

//	FUNCTION public
//	ModWideString::operator const char* -- const char* へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		C 文字列を格納する領域の先頭アドレス
//
//	EXCEPTIONS

inline
ModWideString::operator const char*()
{
	return this->getString(ModKanjiCode::euc);
}

//	FUNCTION public
//	ModWideString::operator[] -- [] 演算子
//
//	NOTES
//		格納する文字列の先頭からなん文字目かの文字を得る
//
//	ARGUMENTS
//		ModSize				i
//			得たい文字が文字列の先頭からなん文字目か
//			先頭の文字は 0 文字目で、
//			最後の文字は ModWideString::getLength() - 1 文字目になる
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
ModWideChar&
ModWideString::operator [](ModSize i)
{
	if (i >= this->getLength())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	if (this->charBuffer) {

		// 呼び出し側でバッファ領域を更新するかもしれないので、
		// バッファ領域と同値の C 文字列を解放しておく

		ModStandardManager::free(this->charBuffer,
								 (this->getSize() + 1) * sizeof(char));
		this->charBuffer = 0;
	}
	return this->buffer[i];
}

inline
ModWideChar&
ModWideString::operator [](int i)
{
	//【注意】	Visual C++ 6.0 では、引数が ModSize の [] 演算子に
	//			整数リテラルを引数に与えるとコンパイルエラーになるので、
	//			引数が int のものも定義する

	return this->operator []((ModSize) i);
}

inline
const ModWideChar&
ModWideString::operator [](ModSize i) const
{
	if (i >= this->getLength())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return this->buffer[i];
}

inline
const ModWideChar&
ModWideString::operator [](int i) const
{
	//【注意】	Visual C++ 6.0 では、引数が ModSize の [] 演算子に
	//			整数リテラルを引数に与えるとコンパイルエラーになるので、
	//			引数が int のものも定義する

	return this->operator []((ModSize) i);
}

//
// FUNCTION
// ModWideString::operator== -- == オペレータ
//
// NOTES
// この関数は ModWideString 同士の同一性を判定するのに用いる。
//
// ARGUMENTS
// const ModWideString& string
//		比較対象の ModWideString 型
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
ModWideString::operator ==(const ModWideString& string) const
{
	return (this->compare(string)) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator== -- char*との == オペレータ
//
// NOTES
// この関数は ModWideString と char* の同一性を判定するのに用いる
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列へのポインタ
//
// RETURN
// ModWideString の表す文字列と string の内容が一致する場合は ModTrue を返し、
// 一致しない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideString::operator ==(const char* string) const
{
	return (this->compare(string)) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator== -- char* を左辺にとる == オペレータ
//
// NOTES
// この関数は char* と ModWideString の同一性を判定するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModWideString string2
//		比較する ModWideString への参照
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
operator ==(const char* string1, const ModWideString& string2)
{
	return (string2.compare(string1)) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator!= -- != オペレータ
//
// NOTES
// この関数は ModWideString 同士の非同一性を判定するのに用いる。
//
// ARGUMENTS
// const ModWideString& string
//		比較対象の ModWideString 型
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
ModWideString::operator !=(const ModWideString& string) const
{
	return (this->compare(string)) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator!= -- char*との != オペレータ
//
// NOTES
// この関数は ModWideString と char* の非同一性を判定するのに用いる
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列へのポインタ
//
// RETURN
// ModWideString の表す文字列と string の内容が一致しない場合は ModTrue を返し、
// 一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// なし
//

inline
ModBoolean
ModWideString::operator !=(const char* string) const
{
	return (this->compare(string)) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator!= -- char* を左辺にとる != オペレータ
//
// NOTES
// この関数は char* と ModWideString の非同一性を判定するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModWideString string2
//		比較する ModWideString への参照
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
operator !=(const char* string1, const ModWideString& string2)
{
	return (string2.compare(string1)) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator< -- < オペレータ
//
// NOTES
// この関数は ModWideString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModWideString& string
//		比較対象の ModWideString 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が大きい場合は ModTrue を返し、
// string のものの方が小さい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator <(const ModWideString& string) const
{
	return (this->compare(string) < 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator< -- char*との < オペレータ
//
// NOTES
// この関数は ModWideString と char* の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列へのポインタ
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が大きい場合は ModTrue を返し、
// string のものの方が小さい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator <(const char* string) const
{
	return (this->compare(string) < 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator< -- char* を左辺にとる < オペレータ
//
// NOTES
// この関数は char* と ModWideString の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModWideString string2
//		比較する ModWideString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string2 のものの方が大きい場合は ModTrue を返し、
// string2 のものの方が小さい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator <(const char* string1, const ModWideString& string2)
{
	return (string2.compare(string1) > 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator> -- > オペレータ
//
// NOTES
// この関数は ModWideString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModWideString& string
//		比較対象の ModWideString 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が小さい場合は ModTrue を返し、
// string のものの方が大きい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator >(const ModWideString& string) const
{
	return (this->compare(string) > 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator> -- char*との > オペレータ
//
// NOTES
// この関数は ModWideString と char* の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列へのポインタ
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が小さい場合は ModTrue を返し、
// string のものの方が大きい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator >(const char* string) const
{
	return (this->compare(string) > 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator> -- char* を左辺にとる > オペレータ
//
// NOTES
// この関数は char* と ModWideString の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModWideString string2
//		比較する ModWideString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string2 のものの方が小さい場合は ModTrue を返し、
// string2 のものの方が大きい場合かすべて一致する場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator >(const char* string1, const ModWideString& string2)
{
	return (string2.compare(string1) < 0) ? ModTrue : ModFalse;
}

//
// FUNCTION
// ModWideString::operator<= -- <= オペレータ
//
// NOTES
// この関数は ModWideString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModWideString& string
//		比較対象の ModWideString 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が大きい場合かすべて一致する場合は
// ModTrue を返し、string のものの方が小さい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator <=(const ModWideString& string) const
{
	return (this->compare(string) > 0) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator<= -- char*との <= オペレータ
//
// NOTES
// この関数は ModWideString と char* の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列へのポインタ
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が大きい場合かすべて一致する場合は
// ModTrue を返し、string のものの方が小さい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator <=(const char* string) const
{
	return (this->compare(string) > 0) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator<= -- char* を左辺にとる <= オペレータ
//
// NOTES
// この関数は char* と ModWideString の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModWideString string2
//		比較する ModWideString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string2 のものの方が大きい場合かすべて一致する場合は
// ModTrue を返し、string2 のものの方が小さい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator <=(const char* string1, const ModWideString& string2)
{
	return (string2.compare(string1) < 0) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator>= -- >= オペレータ
//
// NOTES
// この関数は ModWideString 同士の大小を比較するのに用いる。
//
// ARGUMENTS
// const ModWideString& string
//		比較対象の ModWideString 型
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が小さい場合かすべて一致する場合は
// ModTrue を返し、string のものの方が大きい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator >=(const ModWideString& string) const
{
	return (this->compare(string) < 0) ? ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator>= -- char*との >= オペレータ
//
// NOTES
// この関数は ModWideString と char* の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列へのポインタ
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string のものの方が小さい場合かすべて一致する場合は
// ModTrue を返し、string のものの方が大きい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
ModWideString::operator>=(const char* string) const
{
	return (this->compare(string) < 0) ?  ModFalse : ModTrue;
}

//
// FUNCTION
// ModWideString::operator>= -- char* を左辺にとる >= オペレータ
//
// NOTES
// この関数は char* と ModWideString の大小を比較するのに用いる。
//
// ARGUMENTS
// const char* string1
//		比較される char 配列へのポインタ
// const ModWideString string2
//		比較する ModWideString への参照
//
// RETURN
// 両者の表現する文字列の内容で先頭から見て初めて一致しない場所の
// EUCコードが string2 のものの方が小さい場合かすべて一致する場合は
// ModTrue を返し、string2 のものの方が大きい場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし
//

inline
ModBoolean
operator >=(const char* string1, const ModWideString& string2)
{
	return (string2.compare(string1) > 0) ? ModFalse : ModTrue;
}

//	FUNCTION public
//	ModWideString::compare -- C 文字列と比較する
//
//	NOTES
//
//	ARGUMENTS
//		char*				s
//			自分自身と比較する C 文字列
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
//		ModKanjiCode::KanjiCodeType	code
//			指定されたとき
//				比較する C 文字列の日本語コード
//			指定されないとき
//				ModKanjiCode::euc が指定されたものとみなす
//
//	RETURN
//		1
//			最初に異なるワイド文字が
//			与えられたものより自分自身のもののほうが大きい
//		0
//			自分自身と与えられたもののすべてのワイド文字が等しい
//		-1
//			最初に異なるワイド文字が
//			自分自身のものより与えられたもののほうが大きい
//
//	EXCEPTIONS
//		なし

inline
int
ModWideString::compare(const char* s,
					   ModKanjiCode::KanjiCodeType code) const
{
	return this->compare(s, 0, code);
}

inline
int
ModWideString::compare(const char* s, ModBoolean caseSensitive,
					   ModKanjiCode::KanjiCodeType code) const
{
	return this->compare(s, caseSensitive, 0, code);
}

//	FUNCTION public
//	ModWideString::compare -- ワイド文字列と比較する
//
//	NOTES
//
//	ARGUMENTS
//		ModWideString&		s
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
//			自分自身と与えられたもののすべてのワイド文字が等しい
//		-1
//			最初に異なるワイド文字が
//			自分自身のものより与えられたもののほうが大きい
//
//	EXCEPTIONS
//		なし

inline
int
ModWideString::compare(const ModWideString& s, ModSize len) const
{
	return this->compare(s.getBuffer(), len);
}

inline
int
ModWideString::compare(const ModWideString& s,
					   ModBoolean caseSensitive,
					   ModSize len) const
{
	return this->compare(s.getBuffer(), caseSensitive, len);
}

//	FUNCTION public
//	ModWideString::search -- ASCII 文字やワイド文字を探す
//
//	NOTES
//
//	ARGUMENTS
//		char				v
//			探す ASCII 文字
//		ModWideChar			v
//			探すワイド文字
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//
//	RETURN
//		0 以外の値
//			見つかったワイド文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
ModWideChar*
ModWideString::search(char v, ModBoolean caseSensitive)
{
	//【注意】	ASCII 文字であれば、ModWideChar にキャストして調べればよい

	return ModWideCharTrait::find(this->buffer, v, caseSensitive);
}

inline
const ModWideChar*
ModWideString::search(char v, ModBoolean caseSensitive) const
{
	//【注意】	ASCII 文字であれば、ModWideChar にキャストして調べればよい

	return ModWideCharTrait::find(this->buffer, v, caseSensitive);
}

inline
ModWideChar*
ModWideString::search(ModWideChar v, ModBoolean caseSensitive)
{
	return ModWideCharTrait::find(this->buffer, v, caseSensitive);
}

inline
const ModWideChar*
ModWideString::search(ModWideChar v, ModBoolean caseSensitive) const
{
	return ModWideCharTrait::find(this->buffer, v, caseSensitive);
}

//	FUNCTION public
//	ModWideString::search -- ワイド文字列を探す
//
//	NOTES
//
//	ARGUMENTS
//		ModWideString&		s
//			探すワイド文字列
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//
//	RETURN
//		0 以外の値
//			見つかったワイド文字列の先頭の文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
ModWideChar*
ModWideString::search(const ModWideString& s, ModBoolean caseSensitive)
{
	return this->search(s.getBuffer(), caseSensitive);
}

inline
const ModWideChar*
ModWideString::search(const ModWideString& s, ModBoolean caseSensitive) const
{
	return this->search(s.getBuffer(), caseSensitive);
}

//	FUNCTION public
//	ModWideString::search -- C 文字列を探す
//
//	NOTES
//
//	ARGUMENTS
//		char*				s
//			探す C 文字列
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//		ModKanjiCode::KanjiCodeType	code
//			指定したとき
//				探す C 文字列の日本語コード
//			指定されないとき
//				ModKanjiCode::euc が指定されたものとみなす
//
//	RETURN
//		0 以外の値
//			見つかった C 文字列の先頭の文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
const ModWideChar*
ModWideString::search(const char* s, ModBoolean caseSensitive,
					  ModKanjiCode::KanjiCodeType code) const
{
	return const_cast<ModWideString*>(this)->search(s, caseSensitive, code);
}

//	FUNCTION public
//	ModWideString::search -- ワイド文字配列を探す
//
//	NOTES
//
//	ARGUMENTS
//		ModWideChar*		s
//			探すワイド文字配列
//		ModBoolean			caseSensitive
//			ModTrue または指定されたとき
//				大文字小文字を区別して比較する
//			ModFalse
//				大文字小文字を区別せずに比較する
//
//	RETURN
//		0 以外の値
//			見つかったワイド文字配列の先頭の文字を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし

inline
const ModWideChar*
ModWideString::search(const ModWideChar* s, ModBoolean caseSensitive) const
{
	return const_cast<ModWideString*>(this)->search(s, caseSensitive);
}

//	FUNCTION public
//	ModWideString::getAllocateStep --
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
ModWideString::getAllocateStep() const
{
	return _allocateStep;
}

//	FUNCTION public
//	ModWideString::setAllocateStep --
//		文字列を格納するバッファ領域の拡張単位を変更する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				v
//			新しい値(単位 B)
//			0 を指定すると、ModWideString::_allocateStepDefault に変更される
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModWideString::setAllocateStep(ModSize v)
{
	_allocateStep =	(v) ? v : _allocateStepDefault;
}

#endif	// __ModWideString_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
