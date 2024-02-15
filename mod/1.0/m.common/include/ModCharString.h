// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModCharString.h -- ModCharString のクラス定義
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

#ifndef	__ModCharString_H__
#define __ModCharString_H__

#include "ModCommonDLL.h"
#include "ModConfig.h"
#include "ModTypes.h"
#include "ModError.h"
#include "ModCharTrait.h"
#include "ModDefaultManager.h"
#include "ModOsDriver.h"
#include "ModSerial.h"

//
// CLASS
// ModCharString -- char を文字単位とした文字列を表現するクラス
//
// NOTES
// このクラスは char を文字単位とした文字列を表現するのに用いる。
// 別名として ModString という名称も使える。
//

// ModPureCharStringに変更して、サブクラスとしてメモリハンドルクラスを
// 明示したクラスを作成するには、operator=()の再定義や、copyなど
// オブジェクトを返すメソッドへの対応が必要なことがわかったので断念し、
// 直接、ModDefaultObjectのサブクラスとして作成する。

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModCharString
	: public	ModSerializer,
	  public	ModDefaultObject
{
public:
	//	TYPEDEF
	//	ModCharString::CompareUnit -- 最初に比較する単位
	//
	//	NOTES

	typedef
#if MOD_CONF_BYTEORDER == 0
	unsigned int
#else
	unsigned char
#endif
		CompareUnit;

	ModCharString();
	ModCharString(const char* s, ModSize len = 0);
	ModCharString(char c);
	ModCharString(const ModCharString& src);	// コンストラクター
	~ModCharString();							// デストラクター

	ModCommonDLL
	void					serialize(ModArchive& archiver);
												// シリアル化する

	ModCommonDLL
	ModCharString&			operator =(const ModCharString& src);
	ModCharString&			operator =(const char* src);
												// = 演算子

	ModCharString			operator +(char r) const;
	ModCharString			operator +(const ModCharString& r) const;
	ModCharString			operator +(const char* r) const;

	friend ModCharString
		operator +(const char* l, const ModCharString& r);
												// + 演算子

	ModCharString&			operator +=(char r);
	ModCharString&			operator +=(const ModCharString& r);
	ModCharString&			operator +=(const char* r);
												// += 演算子

	ModCommonDLL
	ModCharString&			append(char src);
	ModCommonDLL
	ModCharString&			append(const ModCharString& src, ModSize len = 0);
	ModCommonDLL
	ModCharString&			append(const char* src, ModSize len = 0);
												// 文字、文字列を末尾に付加する

	void					reverse();			// 文字並びを逆転する
	ModCommonDLL
	void					clear();			// 空文字列を代入する

	ModCommonDLL
	ModCharString&			format(const char* format, ...);
												// 書式に応じた文字列を
												// 生成し、代入する

	ModSize					getSize() const;	// 文字列のサイズを得る
	ModSize					getLength() const;	// 文字列の文字数を得る

	char*					getBuffer();
	const char*				getBuffer() const;	// バッファ領域を得る
	const char*				getString() const;	// C 文字列を得る
	const char*				getTail() const;	// C 文字列の末尾である '\0' を
												// 格納する領域のアドレスを得る

	operator				const char*() const;
												// const char* への
												// キャスト演算子

	char&					at(ModSize i);
	char&					at(int i);
	const char&				at(ModSize i) const;
	const char&				at(int i) const;
												// [] 演算子の機能に範囲検査
												// の機能を追加したもの

	char&					operator [](ModSize i);
	char&					operator [](int i);
	const char&				operator [](ModSize i) const;
	const char&				operator [](int i) const;
												// [] 演算子

	// 比較オペレータ
	ModBoolean operator==(const ModCharString& string) const;
	ModBoolean operator==(const char* string) const;

	friend ModBoolean
		operator ==(const char* string1, const ModCharString& string2);

	ModBoolean operator!=(const ModCharString& string) const;
	ModBoolean operator!=(const char* string) const;

	friend ModBoolean
		operator !=(const char* string1, const ModCharString& string2);

	ModBoolean operator<(const ModCharString& string) const;
	ModBoolean operator<(const char* string) const;

	friend ModBoolean
		operator <(const char* string1, const ModCharString& string2);

	ModBoolean operator>(const ModCharString& string) const;
	ModBoolean operator>(const char* string) const;

	friend ModBoolean
		operator >(const char* string1, const ModCharString& string2);

	ModBoolean operator<=(const ModCharString& string) const;
	ModBoolean operator<=(const char* string) const;

	friend ModBoolean
		operator <=(const char* string1, const ModCharString& string2);

	ModBoolean operator>=(const ModCharString& string) const;
	ModBoolean operator>=(const char* string) const;

	friend ModBoolean
		operator >=(const char* string1, const ModCharString& string2);

	int						compare(const ModCharString& s) const;
	int						compare(const ModCharString& s, ModSize len) const;
	int						compare(const ModCharString& s,
									ModBoolean caseFlag) const;
	int						compare(const ModCharString& s,
									ModBoolean caseFlag, ModSize len) const;
	int						compare(const char* s, ModSize len = 0) const;
	int						compare(const char* s, ModBoolean caseFlag,
									ModSize len = 0) const;
												// 文字列と比較する

	char*					search(char c,
								   ModBoolean caseFlag = ModTrue) const;
	char*					search(const ModCharString& s,
								   ModBoolean caseFlag = ModTrue) const;
	char*					search(const char* s,
								   ModBoolean caseFlag = ModTrue) const;
												// 先頭から文字、文字列を探す

	ModCommonDLL
	char*					rsearch(char c, ModBoolean caseFlag = ModTrue);
												// 末尾から文字を探す

	void					truncate(char* p);
	void					truncate(ModSize len);
												// 文字列の後ろをつめる

	ModCommonDLL
	ModCharString			copy(ModSize start = 0, ModSize len = 0) const;
												// 部分文字列の取得

	// 数値として得る
	int toInt() const;
	long toLong() const;
	ModSize toModSize() const;
	ModFileSize toModFileSize() const;

	ModSize					getAllocateStep() const;
												// バッファ領域の拡張単位を得る
	void					setAllocateStep(ModSize v);
												// バッファ領域の拡張単位を
												// 設定する
	ModCommonDLL
	void					reallocate(ModSize len);
												// バッファ領域を再確保する
private:
	ModCommonDLL
	void					allocate(ModSize len);
												// 新規にバッファ領域を確保する
	ModCommonDLL
	void					allocateCopy(const char* s, ModSize len = 0);
												// C 文字列を代入する

	ModSize					getBufferSize() const;
												// バッファ領域のサイズを得る

	ModSize					_allocateStep;		// バッファ領域の拡張単位
	ModCommonDLL
	static const ModSize	_allocateStepDefault;
												// バッファ領域の拡張単位の
												// デフォルト値

	ModCommonDLL
	static const CompareUnit _null;				// バッファ領域を未確保時に
												// 空の場合に空文字列を
												// 得るための '\0'

	char*					buffer;				// C 文字列を保持する
												// バッファ領域
	char*					endOfBuffer;		// バッファ領域の末尾を指す
	char*					endOfString;		// バッファ領域上の C 文字列の
												// 末尾である '\0' の位置
};

//	FUNCTION public
//	ModCharString::ModCharString --
//		文字列を表すクラスのデフォルトコンストラクター
//
//	NOTES
//		空文字列を表す
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModCharString::ModCharString()
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  _allocateStep(_allocateStepDefault)
{ }

//	FUNCTION public
//	ModCharString::ModCharString --
//		C 文字列を初期値とする文字列を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		char*				s
//			生成される文字列へ代入される C 文字列を格納する領域の先頭アドレス
//			0 が指定されると、空文字列が初期値となる
//		ModSize				len
//			初期値とする C 文字列が、
//			指定された C 文字列の先頭から何文字目までかを表す
//			ただし、指定された C 文字列の文字数より長くはならない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModCharString::ModCharString(const char* s, ModSize len)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  _allocateStep(_allocateStepDefault)
{
	this->allocateCopy(s, len);
}

//	FUNCTION public
//	ModCharString::ModCharString --
//		文字を初期値とする文字列を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		char				c
//			生成される文字列へ代入する文字
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModCharString::ModCharString(char c)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  _allocateStep(_allocateStepDefault)
{
	if (c != ModCharTrait::null()) {
		this->allocate(1);

		*this->buffer = c;
		*(++this->endOfString) = ModCharTrait::null();
	}
}

//	FUNCTION public
//	ModCharString::ModCharString --
//		文字列を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		src
//			生成する文字列へ代入する文字列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModCharString::ModCharString(const ModCharString& src)
	: buffer(0),
	  endOfBuffer(0),
	  endOfString(0),
	  _allocateStep(_allocateStepDefault)
{
	ModSize len = src.getLength();
	if (len) {

		// 複写する文字数ぶんの領域を新規に確保し、
		// 空文字列が格納されていることにする

		this->allocate(len);

		// 与えられた文字列を末尾の '\0' を含めて複写する

		ModOsDriver::Memory::copy(this->buffer, src.buffer,
								  (len + 1) * sizeof(char));
		this->endOfString = this->buffer + len;
	}
}

//	FUNCTION public
//	ModCharString::~ModCharString -- 文字列を表すクラスのデストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
ModCharString::~ModCharString()
{
	if (this->buffer) {
		ModStandardManager::free(this->buffer, this->getBufferSize());
		this->buffer = 0;
	}
}

//	FUNCTION public
//	ModCharString::operator = -- = 演算子
//
//	NOTES
//		自分自身へ C 文字列を代入する
//
//	ARGUMENTS
//		char*				src
//			自分自身へ代入する C 文字列
//			0 が指定されたとき、空文字列が代入される
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

inline
ModCharString& 
ModCharString::operator =(const char* src)
{
	this->allocateCopy(src);
	return *this;
}

//	FUNCTION public
//	ModCharString::operator + -- + 演算子
//
//	NOTES
//		与えられた文字、文字列または C 文字列を
//		自分自身の格納文字列の末尾に付加した文字列を得る
//
//	ARGUMENTS
//		char				r
//			自分自身の格納文字列の末尾に付加する文字
//		const ModCharString&	r
//			自分自身の格納文字列の末尾に付加する文字列
//		const char*			r
//			自分自身の格納文字列の末尾に付加する C 文字列
//
//	RETURN
//		与えられた文字、文字列または C 文字列を
//		自分自身の格納文字列の末尾に付加した文字列
//
//	EXCEPTIONS

inline
ModCharString
ModCharString::operator +(char r) const
{
	return ModCharString(*this) += r;
}

inline
ModCharString
ModCharString::operator +(const ModCharString& r) const
{
	return ModCharString(*this) += r;
}

inline
ModCharString
ModCharString::operator +(const char* r) const
{
	return ModCharString(*this) += r;
}

//	FUNCTION
//	operator + -- + 演算子
//
//	NOTES
//		右辺に与えられた文字列を
//		左辺に与えられた C 文字列の末尾に付加した文字列を得る
//
//	ARGUMENTS
//		const char*			l
//			右辺に与えられた文字列を末尾に付加する C 文字列
//		const ModCharString&	r
//			左辺に与えられた C 文字列の末尾に付加する文字列
//
//	RETURN
//		右辺に与えられた文字列を
//		左辺に与えられた C 文字列の末尾に付加した文字列
//
//	EXCEPTIONS

inline
ModCharString
operator +(const char* l, const ModCharString& r)
{
	return ModCharString(l) += r;
}

//	FUNCTION public
//	ModCharString::operator += -- += 演算子
//
//	NOTES
//		与えられた文字、文字列または C 文字列を
//		自分自身の格納文字列の末尾に付加する
//
//	ARGUMENTS
//		const char			r
//			自分自身の格納文字列の末尾に付加する文字
//		const ModCharString&	r
//			自分自身の格納文字列の末尾に付加する文字列
//		const char*			r
//			自分自身の格納文字列の末尾に付加する C 文字列
//
//	RETURN
//		与えられた文字、文字列または C 文字列を
//		自分自身の格納文字列の末尾に付加後の自分自身
//
//	EXCEPTIONS

inline
ModCharString&
ModCharString::operator +=(char r)
{
	return this->append(r);
}

inline
ModCharString&
ModCharString::operator +=(const ModCharString& r)
{
	return this->append(r, 0);
}

inline
ModCharString&
ModCharString::operator +=(const char* r)
{
	return this->append(r, 0);
}

//	FUNCTION public
//	ModCharString::reverse -- 文字列中の文字の並びの順序を逆にする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModCharString::reverse()
{
	ModCharTrait::reverse(this->buffer, this->endOfString - 1);
}

//	FUNCTION public
//	ModCharString::getSize -- 文字列のサイズを得る
//
//	NOTES
//		C 文字列として表したときの
//		末尾の '\0' を除いたバイト数を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列のサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModCharString::getSize() const
{
	return (ModSize) (this->getTail() - this->getString()) * sizeof(char);
}

//	FUNCTION public
//	ModCharString::getLength -- 文字列の文字数を得る
//
//	NOTES
//		C 文字列として表したときの
//		末尾の '\0' を除いた文字数を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列の文字数
//
//	EXCEPTIONS
//		なし

inline
ModSize
ModCharString::getLength() const
{
	return (ModSize) (this->getTail() - this->getString());
}

//	FUNCTION private
//	ModCharString::getBufferSize --
//		C 文字列を格納するためのバッファ領域のサイズを得る
//
//	NOTES
//		C 文字列を格納するためのバッファ領域のサイズで、
//		ModCharString::getSize + sizeof(char) 以上である
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
ModCharString::getBufferSize() const
{
	return (ModSize) (this->endOfBuffer - this->buffer) * sizeof(char);
}

//	FUNCTION public
//	ModCharString::getBuffer -- C 文字列を格納するためのバッファ領域を得る
//
//	NOTES
//		呼び出し側でC 文字列の末尾を表す '\0' を書き換えてはならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		C 文字列を格納するためのバッファ領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
char*
ModCharString::getBuffer()
{
	return (this->buffer) ?
		this->buffer : const_cast<char*>((const char*) &_null);
}

inline
const char*
ModCharString::getBuffer() const
{
	return (this->buffer) ? this->buffer : (const char*) &_null;
}

//	FUNCTION public
//	ModCharString::getString -- C 文字列を得る
//
//	NOTES
//		文字列を表す C 文字列を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた C 文字列を格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
const char*
ModCharString::getString() const
{
	return this->getBuffer();
}

//	FUNCTION public
//	ModCharString::getTail --
//		C 文字列の末尾を表す '\0' を格納する領域のアドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		C 文字列の末尾を表す '\0' を格納する領域のアドレス
//
//	EXCEPTIONS
//		なし

inline
const char*
ModCharString::getTail() const
{
	return (this->endOfString) ? this->endOfString : (const char*) &_null;
}

//	FUNCTION public
//	ModCharString::operator const char* -- const char* へのキャスト演算子
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
//		なし

inline
ModCharString::operator const char*() const
{
	return this->getString();
}

//	FUNCTION public
//	ModCharString::at -- [] 演算子 に範囲検査を付加したもの
//
//	NOTES
//		格納する文字列の先頭からなん文字目かの文字を得る
//		ただし、C 文字列として表したときの末尾の '\0' は得ることができない
//
//	ARGUMENTS
//		ModSize				i
//			得たい文字が文字列の先頭からなん文字目か
//			先頭の文字は 0 文字目で、
//			最後の文字は ModCharString::getLength() - 1 文字目になる
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
char&
ModCharString::at(ModSize i)
{
	if (i >= this->getLength())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return this->buffer[i];
}

inline
char&
ModCharString::at(int i)
{
	return this->at((ModSize) i);
}

inline
const char&
ModCharString::at(ModSize i) const
{
	if (i >= this->getLength())
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument, ModErrorLevelError);

	return this->buffer[i];
}

inline
const char&
ModCharString::at(int i) const
{
	return this->at((ModSize) i);
}

//	FUNCTION public
//	ModCharString::operator [] -- [] 演算子
//
//	NOTES
//		格納する文字列の先頭からなん文字目かの文字を得る
//		ただし、C 文字列として表したときの末尾の '\0' は得ることができない
//
//	ARGUMENTS
//		ModSize				i
//			得たい文字が文字列の先頭からなん文字目か
//			先頭の文字は 0 文字目で、
//			最後の文字は ModCharString::getLength() - 1 文字目になる
//		int					i
//			ModSize の場合と同様
//
//	RETURN
//		先頭から指定された数番目の文字
//
//	EXCEPTIONS
//		範囲検査を行なわないので BadArgument は返らない。
//		もし範囲検査が必要ならば at() を使用すること。

inline
char&
ModCharString::operator [](ModSize i)
{
	return this->buffer[i];
}

inline
char&
ModCharString::operator [](int i)
{
	//【注意】	Visual C++ 6.0 では、引数が ModSize の [] 演算子に
	//			整数リテラルを引数に与えるとコンパイルエラーになるので、
	//			引数が int のものも定義する
	return this->buffer[i];
}

inline
const char&
ModCharString::operator [](ModSize i) const
{
	return this->buffer[i];
}

inline
const char&
ModCharString::operator [](int i) const
{
	//【注意】	Visual C++ 6.0 では、引数が ModSize の [] 演算子に
	//			整数リテラルを引数に与えるとコンパイルエラーになるので、
	//			引数が int のものも定義する

	return this->buffer[i];
}

// FUNCTION
// ModCharString::operator== -- == オペレータ
//
// NOTES
// この関数は ModCharString 同士の同一性を判定するのに用いる。
//
// ARGUMENTS
// const ModCharString& string
//		比較対象の ModCharString 型
//
// RETURN
// 両者の表現する文字列の内容が一致する場合は ModTrue を返し、
// 一致しない場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし

inline
ModBoolean
ModCharString::operator ==(const ModCharString& string) const
{
	return (this->compare(string)) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator== -- char*との==オペレータ
//
// NOTES
// このオペレータは ModCharString と char* の比較を行なうために用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列
//
// RETURN
// ModCharString と char* の表す文字列がすべて一致している場合は ModTrue を、
// 一致していない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator ==(const char* string) const
{
	return (this->compare(string)) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator== -- char* を左辺にとる==オペレータ
//
// NOTES
// このオペレータは char* と ModCharString を比較するのに用いる。
//
// ARGUMENTS
// const char* string1
//		== の左辺にくる char* のオペランド
// const ModCharString& string2
//		== の右辺にくる ModCharString のオペランド
//
// RETURN
// string1 と string2 の表す文字列が一致している場合は ModTrue を返し、
// 一致していない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
operator ==(const char* string1, const ModCharString& string2)
{
	return (string2.compare(string1)) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator!= -- != オペレータ
//
// NOTES
// このオペレータは ModCharString 同士の同一性を判定するのに用いる。
//
// ARGUMENTS
// const ModCharString& string
//		比較対象の ModCharString 型
//
// RETURN
// 両者の表現する文字列の内容が一致する場合は ModTrue を返し、
// 一致しない場合は ModFalse を返す。
//
// EXCEPTIONS
// その他
// なし

inline
ModBoolean
ModCharString::operator !=(const ModCharString& string) const
{
	return (this->compare(string)) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator!= -- char*との!=オペレータ
//
// NOTES
// このオペレータは ModCharString と char* の比較を行なうために用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の char 配列
//
// RETURN
// ModCharString と char* の表す文字列が一致していない場合は ModTrue を、
// すべて一致している場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator !=(const char* string) const
{
	return (this->compare(string)) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator!= -- char* を左辺にとる!=オペレータ
//
// NOTES
// このオペレータは char* と ModCharString を比較するのに用いる。
//
// ARGUMENTS
// const char* string1
//		== の左辺にくる char* のオペランド
// const ModCharString& string2
//		== の右辺にくる ModCharString のオペランド
//
// RETURN
// string1 と string2 の表す文字列が一致していない場合は ModTrue を返し、
// すべて一致している場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
operator !=(const char* string1, const ModCharString& string2)
{
	return (string2.compare(string1)) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator < -- < オペレータ
//
// NOTES
// このオペレータは ModCharString 同士の大小比較を行なうために用いる。
//
// ARGUMENTS
// const ModCharString& string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 大きい場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator <(const ModCharString& string) const
{
	return (this->compare(string) < 0) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator< -- char*との < オペレータ
//
// NOTES
// このオペレータは ModCharString と char* の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 大きい場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator <(const char* string) const
{
	return (this->compare(string) < 0) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator< -- char* を左辺にとる < オペレータ
//
// NOTES
// このオペレータは char* と ModCharString の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string1
//		< オペランドの左辺にくる比較対象の文字列
// const ModCharString& string2
//		< オペランドの右辺にくる比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 大きい場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
operator <(const char* string1, const ModCharString& string2)
{
	return (string2.compare(string1) > 0) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator> -- > オペレータ
//
// NOTES
// このオペレータは ModCharString 同士の大小比較を行なうために用いる。
//
// ARGUMENTS
// const ModCharString& string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 小さい場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator >(const ModCharString& string) const
{
	return (this->compare(string) > 0) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator> -- char*との > オペレータ
//
// NOTES
// このオペレータは ModCharString と char* の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 小さい場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator >(const char* string) const
{
	return (this->compare(string) > 0) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator> -- char* を左辺にとる > オペレータ
//
// NOTES
// このオペレータは char* と ModCharString の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string1
//		> オペランドの左辺にくる比較対象の文字列
// const ModCharString& string2
//		> オペランドの右辺にくる比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 小さい場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
operator >(const char* string1, const ModCharString& string2)
{
	return (string2.compare(string1) < 0) ? ModTrue : ModFalse;
}

// FUNCTION
// ModCharString::operator<= -- <= オペレータ
//
// NOTES
// このオペレータは ModCharString 同士の大小比較を行なうために用いる。
//
// ARGUMENTS
// const ModCharString& string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 大きいかすべて一致する場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator <=(const ModCharString& string) const
{
	return (this->compare(string) > 0) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator<= -- char*との <= オペレータ
//
// NOTES
// このオペレータは ModCharString と char* の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 大きいかすべて一致する場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator <=(const char* string) const
{
	return (this->compare(string) > 0) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator<= -- char* を左辺にとる<=オペレータ
//
// NOTES
// このオペレータは char* と ModCharString の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string1
//		<= オペランドの左辺にくる比較対象の文字列
// const ModCharString& string2
//		<= オペランドの右辺にくる比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 大きいかすべて一致する場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
operator <=(const char* string1, const ModCharString& string2)
{
	return (string2.compare(string1) < 0) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator>= -- >= オペレータ
//
// NOTES
// このオペレータは ModCharString 同士の大小比較を行なうために用いる。
//
// ARGUMENTS
// const ModCharString& string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 小さいかすべて一致する場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator >=(const ModCharString& string) const
{
	return (this->compare(string) < 0) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator>= -- char*との >= オペレータ
//
// NOTES
// このオペレータは ModCharString と char* の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string
//		比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 小さいかすべて一致する場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
ModCharString::operator >=(const char* string) const
{
	return (this->compare(string) < 0) ? ModFalse : ModTrue;
}

// FUNCTION
// ModCharString::operator>= -- char* を左辺にとる>=オペレータ
//
// NOTES
// このオペレータは char* と ModCharString の大小比較を行なうために用いる。
//
// ARGUMENTS
// const char* string1
//		>= オペランドの左辺にくる比較対象の文字列
// const ModCharString& string2
//		>= オペランドの右辺にくる比較対象の文字列
//
// RETURN
// 先頭から調べて最初の異なる文字の ASCII コードが string の方が
// 小さいかすべて一致する場合は ModTrue を、そうでない場合は ModFalse を返す。
//
// EXCEPTIONS
// なし

inline
ModBoolean
operator >=(const char* string1, const ModCharString& string2)
{
	return (string2.compare(string1) > 0) ? ModFalse : ModTrue;
}

//	FUNCTION public
//	ModCharString::compare -- 文字列の大小比較を行う
//
//	NOTES
//		自分自身と与えられた文字列の先頭からひと文字づつ取り出して、
//		その ASCII コードを比較し、異なるか、または指定された
//		文字数比較するまで、続ける
//
//	ARGUMENTS
//		ModCharString&		s
//			自分自身と比較する文字列
//		ModBoolean			caseFlag
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
//			最初に異なる文字の ASCII コードが
//			与えられたものより自分自身のもののほうが大きい
//		0
//			自分自身と与えられたもののすべての文字の ASCII コードが等しい
//		-1
//			最初に異なる文字の ASCII コードが
//			自分自身のものより与えられたもののほうが大きい
//
//	EXCEPTIONS
//		なし

inline
int
ModCharString::compare(const ModCharString& s) const
{
	//【注意】	実行速度の面から、
	//			ModCharString::compare(const ModCharString&, ModSize) と分けた

	if (this == &s || this->buffer == s.buffer)
		return 0;

	const char*	p = this->getString();
	const char* q = s.getString();

	// 文字列の先頭から sizeof(CompareUnit) バイトぶんは、直接比較する

	if (*((CompareUnit*) p) < *((CompareUnit*) q))
		return -1;
	if (*((CompareUnit*) p) > *((CompareUnit*) q))
		return 1;

	// 文字列の先頭から sizeof(CompareUnit) バイトぶんは等しかったので、
	// OS ドライバーの memcmp で比較する
	//
	//【注意】	比較するサイズは、双方のバイト数の小さいほう + '\0' のぶん
	//			にしておけば、双方のバイト数の小さいほうまで
	//			まったく等しくても、'\0' に対応する片方の文字は
	//			必ず異なるので、うまくいくはず

	return ModOsDriver::Memory::compare(p, q,
				(ModMin(this->getLength(), s.getLength()) + 1) * sizeof(char));
}

inline
int
ModCharString::compare(const ModCharString& s, ModSize len) const
{
	if (len == 0)
		return this->compare(s);

	if (this == &s || this->buffer == s.buffer)
		return 0;

	const char* p = this->getString();
	const char* q = s.getString();

	if (sizeof(CompareUnit) <= len * sizeof(char)) {

		// 文字列の先頭から sizeof(CompareUnit) バイトぶんは、直接比較する

		if (*((CompareUnit*) p) < *((CompareUnit*) q))
			return -1;
		if (*((CompareUnit*) p) > *((CompareUnit*) q))
			return 1;
	} else {

		// 比較する部分のサイズが sizeof(CompareUnit) バイトより少ないとき、
		// 先頭の 1 バイトのみ直接比較する

		if (*((unsigned char*) p) < *((unsigned char*) q))
			return -1;
		if (*((unsigned char*) p) > *((unsigned char*) q))
			return 1;
	}

	// 文字列の先頭から sizeof(CompareUnit) または 1 バイトぶんは
	// 等しかったので、OS ドライバーの Memory::compare で比較する

	return ModOsDriver::Memory::compare(p, q,
	ModMin(len, ModMin(this->getLength(), s.getLength()) + 1) * sizeof(char));
}

inline
int
ModCharString::compare(const ModCharString& s, ModBoolean caseFlag) const
{
	return (caseFlag) ? this->compare(s) :
		(this == &s || this->buffer == s.buffer) ? 0 :
		ModCharTrait::compare(this->getString(), s.getString(), ModFalse, 0);
}

inline
int
ModCharString::compare(const ModCharString& s,
					   ModBoolean caseFlag, ModSize len) const
{
	return (len == 0) ? this->compare(s, caseFlag) :
		(caseFlag) ? this->compare(s, len) :
		(this == &s || this->buffer == s.buffer) ? 0 :
		ModCharTrait::compare(this->getString(), s.getString(), ModFalse, len);
}

//	FUNCTION public
//	ModCharString::compare -- 文字列と C 文字列の大小比較を行う
//
//	NOTES
//		自分自身と与えられた C 文字列の先頭からひと文字づつ取り出して、
//		その ASCII コードを比較し、異なるか、または指定された
//		文字数比較するまで、続ける
//
//	ARGUMENTS
//		char*				s
//			自分自身と比較する C 文字列
//		ModBoolean			caseFlag
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
//			最初に異なる文字の ASCII コードが
//			与えられたものより自分自身のもののほうが大きい
//		0
//			自分自身と与えられたもののすべての文字の ASCII コードが等しい
//		-1
//			最初に異なる文字の ASCII コードが
//			自分自身のものより与えられたもののほうが大きい
//
//	EXCEPTIONS
//		なし

inline
int
ModCharString::compare(const char* s, ModSize len) const
{
	return ModCharTrait::compare(this->getString(), s, len);
}

inline
int
ModCharString::compare(const char* s, ModBoolean caseFlag, ModSize len) const
{
	return ModCharTrait::compare(this->getString(), s, caseFlag, len);
}

//	FUNCTION public
//	ModCharString::search -- 文字を検索する
//
//	NOTES
//		自分自身に与えられた文字と同じ文字があるか
//		先頭から調べていく
//
//	ARGUMENTS
//		char				c
//			自分自身の中を調べる文字
//		ModBoolean			caseFlag
//			ModTrue または指定されないとき
//				大文字、小文字を区別する
//			ModFalse
//				大文字、小文字を区別しない
//
//	RETURN
//		0 以外
//			与えられた文字と一致する文字の先頭のアドレス
//		0
//			与えられた文字と一致する文字はなかった
//
//	EXCEPTIONS
//		なし

inline
char*
ModCharString::search(char c, ModBoolean caseFlag) const
{
	return ModCharTrait::find(this->getString(), c, caseFlag);
}

//	FUNCTION public
//	ModCharString::search -- 文字列を検索する
//
//	NOTES
//		自分自身に与えられた文字列のまったく同じ部分文字列があるか
//		先頭から調べていく
//
//	ARGUMENTS
//		ModCharString&		s
//			自分自身の中を調べる文字列
//		char*				s
//			自分自身の中を調べる文字列
//		ModBoolean			caseFlag
//			ModTrue または指定されないとき
//				大文字、小文字を区別する
//			ModFalse
//				大文字、小文字を区別しない
//
//	RETURN
//		0 以外
//			与えられた文字列と一致する部分の先頭のアドレス
//		0
//			与えられた文字列と一致する部分はなかった
//
//	EXCEPTIONS
//		なし

inline
char*
ModCharString::search(const ModCharString& s, ModBoolean caseFlag) const
{
	return ModCharTrait::find(this->getString(), s.getString(), caseFlag);
}

inline
char*
ModCharString::search(const char* s, ModBoolean caseFlag) const
{
	return ModCharTrait::find(this->getString(), s, caseFlag);
}

//	FUNCTION public
//	ModCharString::truncate -- 文字列の後ろをつめる
//
//	NOTES
//
//	ARGUMENTS
//		char*				p
//			格納する C 文字列の新しい末尾
//			この位置に '\0' が設定される
//			指定された位置が格納する C 文字列上でなければ、なにもしない
//		ModSize				len
//			新しい文字列の文字数
//			格納する文字列の先頭から指定された数の文字を取り出して
//			新しい文字列とする
//			指定された数が格納する文字列の文字数より大きいときはなにもしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModCharString::truncate(char* p)
{
	if (this->buffer <= p && p < this->endOfString) {
		*(this->endOfString = const_cast<char*>(p)) = ModCharTrait::null();

		// 文字列の比較は CompareUnit 単位で行うので、
		// 最低でもバッファの先頭から sizeof(CompareUnit) バイトぶんのうち
		// 使用していない部分は 0 埋めしておく必要がある

		const char* q = this->buffer + sizeof(CompareUnit) / sizeof(char);
		for (; ++p < q; *p = ModCharTrait::null()) ;
	}
}

inline
void
ModCharString::truncate(ModSize len)
{
	this->truncate(this->buffer + len);
}

//
// FUNCTION
// ModCharString::toInt -- int に変換する
//
// NOTES
// この関数は ModCharString で表現されている文字列の先頭から
// 続く数字列を10進の数値に変換するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 先頭から続く数字列を10進の数値に変換したものを返す。
// オーバーフローには考慮しない。
//
// EXCEPTIONS
// なし
//
inline int
ModCharString::toInt() const
{
	return ModCharTrait::toInt(this->buffer);
}

//
// FUNCTION
// ModCharString::toLong -- long に変換する
//
// NOTES
// この関数は ModCharString で表現されている文字列の先頭から
// 続く数字列を10進の数値に変換するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 先頭から続く数字列を10進の数値に変換したものを返す。
// オーバーフローには考慮しない。
//
// EXCEPTIONS
// なし
//
inline long
ModCharString::toLong() const
{
	return ModCharTrait::toLong(this->buffer);
}

//
// FUNCTION
// ModCharString::toModSize -- ModSize に変換する
//
// NOTES
// この関数は ModCharString で表現されている文字列の先頭から
// 続く数字列を10進の数値に変換するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 先頭から続く数字列を10進の数値に変換したものを返す。
// オーバーフローには考慮しない。
//
// EXCEPTIONS
// なし
//
inline ModSize
ModCharString::toModSize() const
{
	return ModCharTrait::toModSize(this->buffer);
}

//
// FUNCTION
// ModCharString::toModFileSize -- ModFileSize に変換する
//
// NOTES
// この関数は ModCharString で表現されている文字列の先頭から
// 続く数字列を10進の数値に変換するために用いる。
//
// ARGUMENTS
// なし
//
// RETURN
// 先頭から続く数字列を10進の数値に変換したものを返す。
// オーバーフローには考慮しない。
//
// EXCEPTIONS
// なし
//
inline ModFileSize
ModCharString::toModFileSize() const
{
	return ModCharTrait::toModFileSize(this->buffer);
}

//	FUNCTION public
//	ModCharString::getAllocateStep --
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
ModCharString::getAllocateStep() const
{
	return _allocateStep;
}

//	FUNCTION public
//	ModCharString::setAllocateStep --
//		文字列を格納するバッファ領域の拡張単位を変更する
//
//	NOTES
//
//	ARGUMENTS
//		ModSize				v
//			新しい値(単位 B)
//			0 を指定すると、ModCharString::_allocateStepDefault に変更される
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
ModCharString::setAllocateStep(ModSize v)
{
	_allocateStep =
		(v) ? ModMax((ModSize) sizeof(CompareUnit), v) : _allocateStepDefault;
}

#endif	// __ModCharString_H__

//
// Copyright (c) 1997, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
