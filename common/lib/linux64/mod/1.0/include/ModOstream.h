// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModOstream.h -- 出力ストリーム
// 
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModOstream_H__
#define __ModOstream_H__

#include "ModCommonDLL.h"
#include "ModCommon.h"
#include "ModDefaultManager.h"
#include "ModIos.h"
#include "ModOsMutex.h"

class ModCharString;
class ModUnicodeString;

//	CLASS
//	ModOstream -- 出力ストリームの基底クラス
//
//	NOTES
//		・出力ストリームクラスは本クラスを継承しなければいけない。
//		・本クラスでは operator<< を実装している。
//		・本クラスを継承したクラスは互いを operator<< の引数に渡せる。
//		・本クラスを継承したクラスはストリームの実体(メモリや標準出力)に文字を
//		  書き込むためのインタフェース(put, write)を実装しなければならない。
//

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModOstream
	: virtual public	ModIos
{
public:
	//
	// コンストラクタ、デストラクタ
	//
	ModOstream();
	~ModOstream();

	//
	// マニピュレータ
	//

	// 文字を追加
	virtual ModOstream&	put(const char character_) = 0;
	virtual ModOstream&	put(const ModUnicodeChar character_) = 0;

	// 文字列を追加
	virtual ModOstream& write(const char* string_) = 0;
	virtual ModOstream& write(const ModUnicodeChar* string_) = 0;

	// ストリームの同期をとる。デフォルトではなにもしない
	virtual void flush()
	{}

	// 演算子
	ModOstream&	operator <<(const char character_);		// 文字として解釈
	ModOstream&	operator <<(const unsigned char character_);
	ModOstream&	operator <<(const ModUnicodeChar character_);

	ModCommonDLL
	ModOstream&	operator <<(const int value_);			// 数値として解釈
	ModCommonDLL
	ModOstream&	operator <<(const unsigned int value_);

	ModCommonDLL
	ModOstream&	operator <<(const long value_);		// 数値として解釈
	ModCommonDLL
	ModOstream&	operator <<(const unsigned long value_);

	ModCommonDLL
	ModOstream&	operator <<(const ModInt64 value_);		// 数値として解釈
	ModCommonDLL
	ModOstream&	operator <<(const ModUInt64 value_);

	ModCommonDLL
	ModOstream&	operator <<(const double value_);		// 数値として解釈
	ModOstream&	operator <<(const float value_);

	ModCommonDLL
	ModOstream&	operator <<(const char* string_);		// 文字列として解釈
	ModCommonDLL
	ModOstream&	operator <<(const ModCharString& string_);

	ModCommonDLL
	ModOstream&	operator <<(const ModUnicodeChar* string_); // 文字列として解釈
	ModCommonDLL
	ModOstream&	operator <<(const ModUnicodeString& string_);

	// ModOstrStream との互換性のためにワイド文字をサポート
	// (下の operator だけ ModOstrStream.h で実装したいところだが、
	//  そうしてしまうと ModOstream の operator << とマッチしなくなる)
	ModCommonDLL
	ModOstream&	operator <<(const ModWideChar* string_);
	ModCommonDLL
	ModOstream&	operator <<(ModWideString& string_);

	ModOstream&	operator <<(ModOstream& (*m_)(ModOstream&));
	ModOstream&	operator <<(ModIos& (*m_)(ModIos&));

	// 引数つきマニピュレータのための operator<< は引数にテンプレートを
	// 使っているのでクラスの外で定義している
	// ModOstream& operator<<(ModOstream& stream_,
	//						  const ModStreamManipulator<TYPE>& smanip_);

	// 文字列の解釈方法(マルチバイト文字のエンコード法方)を変更
	ModKanjiCode::KanjiCodeType setEncodingType(const ModKanjiCode::KanjiCodeType newEncodingType_);

protected:
	// 本当は ModIos にあるべきだが、いろいろと考察中
	ModKanjiCode::KanjiCodeType	_encodingType;	// 文字列の解釈方法

private:
	static void doubleOperatorHelper(ModOstream*		stream_,
									 const char*		buffer_,
									 const ModBoolean	negative_,
									 const ModInt32		dummyPrecision_);

	static void	writeInteger(ModOstream&		stream_,
							 const ModUInt64	absValue_,
							 const ModBoolean	negative_);

	// 整数を文字列に変換した時の文字数(桁数+符号)(終端文字は含まない)
	// (文字列が含んでもよい文字 = '+', '-', 数値(0-9, a-z))
	static const int			_integerMax;

	// 小数部分を文字列に変換した時の文字数(桁数)(小数点、終端文字は含まない)
	// (文字列が含んでもよい文字 = 数値(0-9))
	static const int			_fractionMax;
	// 小数部分の桁数を文字列にした場合の文字数
	// (_fractionMax が '10' ならば _fractionCharacterMax は 2 となる)
	static const int			_fractionCharacterMax;

	// 指数部分を文字列に変換した時の文字数(終端文字は含まない)
	// (文字列が含んでもよい文字 = "e+"or"e-", 数値(0-9))
	static const int			_exponentMax;

	// 以下のメンバーを保護するためのラッチ
	static ModOsMutex			_latch;

	// double 型の数値を Fixed 形式の文字列に変換するときに使う領域
	//
	// Fiexd の場合は整数部分だけでも最大 309 桁(Solaris,NT の double の
	// 上限値より)なので、符号(1文字)小数点(1文字)と小数部分(_fractionMax 桁
	// (16桁))を含めると 327 文字＋終端文字で表現できる。
	static char					_buffer[]; // 328 バイト
};

//
// コンストラクタ、デストラクタ
//

//	FUNCTION public
//	ModOstream::ModOstream --
//		出力ストリームを表すクラスのデフォルトコンストラクター
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
ModOstream::ModOstream()
	: ModIos(),
	  _encodingType(ModOs::Process::getEncodingType())
{ }

//	FUNCTION public
//	ModOstream::~ModOstream -- 出力ストリームを表すクラスのデストラクター
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
ModOstream::~ModOstream()
{
	; // do nothing
}

//
// アクセッサ
//

//
// マニピュレータ
//

//	FUNCTION public
//	ModOstream::operator << -- << 演算子
//
//	NOTES
//	出力ストリームに与えられた値を追加する。
//	char, unsigned char, ModUnicodeChar は文字として扱う。
//	(ModUnicodeChar は unsigned short である。そのため unsigned short 型の
//	値は数字ではなく文字として解釈されるので注意せよ)
//
//	ARGUMENTS
//		char				character_
//			追加する char 文字
//
//		unsigned char		character_
//			追加する unsigned char 文字
//
//		ModUnicodeChar		character_
//			追加する Unicode 文字
//
//		int					value_
//			追加する int 値
//
//		unsigned int		value_
//			追加する unsigned int 値
//
//		float				value_
//			追加する float 値
//
//		char*				string_
//			追加する char* 文字列
//
//		ModUnicodeChar*		string_
//			追加する ModUnicodeChar* 文字列
//
//	RETURN
//		与えられた値を追加後の自分自身
//
//	EXCEPTIONS

inline
ModOstream&
ModOstream::operator <<(const char character_)
{
	return this->put(character_);
}

inline
ModOstream&
ModOstream::operator <<(const unsigned char character_)
{
	return this->put((char) character_);
}

inline
ModOstream&
ModOstream::operator <<(const ModUnicodeChar character_)
{
	return this->put(character_);
}

inline
ModOstream&
ModOstream::operator <<(const float value_)
{
	return this->operator <<((double) value_);
}

//	FUNCTION public
//	ModOstream::operator << -- << 演算子
//
//	NOTES
//		与えられたマニピュレーターを適用する
//
//	ARGUMENTS
//		ModOstream& (*m_)(ModOstream&)
//			適用するマニピュレーター
//
//	RETURN
//		与えられたマニピュレーターを適用後の自分自身
//
//	EXCEPTIONS

inline
ModOstream&
ModOstream::operator <<(ModOstream& (*m_)(ModOstream&))
{
	return m_(*this);
}

inline
ModOstream&
ModOstream::operator <<(ModIos& (*m_)(ModIos&))
{
	m_(*this);
	return *this;
}

template <class TYPE>
inline
ModOstream&
operator<<(ModOstream& stream_, const ModStreamManipulator<TYPE>& smanip_)
{
	(*smanip_._pFunc)(stream_, smanip_._argument);
	return stream_;
}

//	FUNCTION public
//	ModOstream::setEncodingType -- エンコーディング方法を変更
//
//	NOTES
//	ストリームに入力されるマルチバイト文字 (char と char*) のエンコード方法は
//	デフォルトでは ModOs::Process::getEncodingType で取得される種類になる。
//
//	しかし、入力するマルチバイト文字が常に ASCII の前半 7bit に限定されている
//	場合は ModKanjiCode::unknown を設定すべきである。この値が設定されると
//	文字列のコード変換をしないので処理が早くなる。
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
ModKanjiCode::KanjiCodeType
ModOstream::setEncodingType(const ModKanjiCode::KanjiCodeType newEncodingType_)
{
	const ModKanjiCode::KanjiCodeType oldEncodingType = _encodingType;
	_encodingType = newEncodingType_;
	return oldEncodingType;
}

//////////////////////////////////////////////////
// flush と endl
//////////////////////////////////////////////////

//	FUNCTION public
//	ModFlush -- ストリームをフラッシュする
//
//	NOTES
//	ストリームをフラッシュする
//
//	ARGUMENTS
//	ModOstream& stream_
//
//	RETURN
//	ModOstream&
//
//	EXCEPTIONS
//
inline
ModOstream& 
ModFlush(ModOstream& stream_)
{
	stream_.flush();
	return stream_;
}

//	FUNCTION public
//	ModEndl -- 改行マークをストリームに書き込む
//
//	NOTES
//	改行マークをストリームに書き込む
//
//	ARGUMENTS
//	ModOstream& stream_
//
//	RETURN
//	ModOstream&
//
//	EXCEPTIONS
//
inline
ModOstream& 
ModEndl(ModOstream& stream_)
{
	return ModFlush(stream_.put('\n'));
}

#endif	// __ModOstream_H__

//
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
