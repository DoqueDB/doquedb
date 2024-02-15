// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModIos.h -- 出力文字列ストリームの整形
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModIos_H__
#define __ModIos_H__

#include "ModCommon.h"
#include "ModDefaultManager.h"

//	CLASS
//	ModIos -- 全てのストリームクラスの基底クラス
//
//	NOTES

//【注意】	private でないメソッドがすべて inline で、
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものはなく、
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものはないので dllexport しない

class ModIos
	: public	ModDefaultObject
{
public:
	//	ENUM
	//	ModIos::OpenMode -- ストリームのオープンモード
	//
	//	NOTES
	enum OpenMode
	{
		output =	0x02,				// 書き込み用にオープン
										// (バッファの先頭から書き込む)
		append =	0x08				// 書き込み用にオープン
										// (バッファの末尾から書き込む)
	};

	//	ENUM
	//	ModIos::FormatFlag -- 書式フラグ
	//
	//	NOTES
	enum FormatFlag
	{
		left		= 0x0001,		// 左揃えで出力する
		right		= 0x0002,		// 右揃えで出力する
		internal	= 0x0004,		// 正負号や基数指示子の後のパディング
		dec 		= 0x0008,		// 10 進数表記
		oct			= 0x0010,		// 8  進数表記
		hex 		= 0x0020,		// 16 進数表記
		showbase	= 0x0040,		// 基数指示子を出力
		showpos		= 0x0080,		// 正の整数に "+" 記号を追加
		fixed		= 0x0100,		// 123.45 浮動小数点表記を使用
		scientific	= 0x0200		// 1.2345e2 浮動小数点表記を使用
	};

	enum
	{
		// left, right, internal 用のマスク
		adjustfield =	left | right | internal,
		// oct, dec, hex 用のマスク
		basefield =		oct | dec | hex,
		// fixed, scientific 用のマスク
		floatfield =	fixed | scientific
	};

	//
	// アクセッサ
	//

  	int			flags() const;
	ModInt32	width() const;
    char		fill() const;
	ModInt32	precision() const;

	//
	// マニピュレータ
	//

  	int			flags(const int newFlags_);
   	int			setf(const int newFlags_);
	int			setf(const int newFlags_, const int mask_);
	int			unsetf(const int mask_);
    ModInt32	width(const ModInt32 newWidth_);
	char		fill(const char newFill_);
	ModInt32	precision(const ModInt32 newPrecision_);

protected:
	// コンストラクタ、デストラクタ
	ModIos();
	virtual ~ModIos();
	
	// データメンバ
  	int			_flags;		// 書式化フラグを OR 演算した結果
    char		_fill;		// ModSetFill でセット
    ModInt32 	_width;		// ModSetW でセット
    ModInt32	_precision;	// ModSetPrecision でセット

private:
	// コピーができないようにするために、以下のメソッドを非公開とする
	ModIos(const ModIos&);
	ModIos& operator=(const ModIos&);
};

//
// アクセッサ
//


//	FUNCTION public
//	ModIos::flags -- 
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		現在の書式フラグ
//
//	EXCEPTIONS
//
inline
int
ModIos::flags() const
{
	return _flags;
}

//	FUNCTION public
//	ModIos::width -- 
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		現在の width
//
//	EXCEPTIONS
//
inline
ModInt32
ModIos::width() const
{
	return _width;
}

//	FUNCTION public
//	ModIos::fill -- 
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		現在の fill 文字
//
//	EXCEPTIONS
//
inline
char
ModIos::fill() const
{
	return _fill;
}

//	FUNCTION public
//	ModIos::precision -- 
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		現在の precision
//
//	EXCEPTIONS
//
inline
ModInt32
ModIos::precision() const
{
	return _precision;
}



//
// 一般的なマニピュレータ
//



//	FUNCTION public
//	ModIos::flags -- 
//
//	NOTES
//		書式フラグを変更する(上書きする)
//
//	ARGUMENTS
//	const int newFlags_
//		新しい書式フラグ
//
//	RETURN
//		変更前の書式フラグ
//
//	EXCEPTIONS
//
inline
int
ModIos::flags(const int newFlags_)
{
	const int oldFlags = _flags;
    _flags = newFlags_;
	return oldFlags;
}

//	FUNCTION public
//	ModIos::setf -- 現在の書式フラグに追加
//
//	NOTES
//	新しい書式フラグを現在の書式フラグに追加(OR演算)する
//
//	ARGUMENTS
//	const int newFlags
//		追加する書式フラグ
//
//	RETURN
//		追加前の書式フラグ
//
//	EXCEPTIONS
//
inline
int
ModIos::setf(const int newFlags_)
{
	const int oldFlags = _flags;
    _flags |= newFlags_;
	return oldFlags;
}

//	FUNCTION public
//	ModIos::setf -- 現在の書式フラグを一部変更
//
//	NOTES
//	現在の書式フラグにマスクをかけ、マスクのビットが立っている位置のフラグを
//	クリアする。もし、引数のフラグ値のビットがマスクした範囲で立っていれば
//	フラグのビットも立てる。
//
//	例えば、書式フラグの基数に関するビットを立てたマスクと、 Dec を立てた
//	フィールド値を渡した場合は現在の基数をクリアして Dec に変更する。
//
//	ARGUMENTS
//	const int newFlags
//		追加する書式フラグ
//
//	const int mask_
//		変更する範囲のビットを立てたマスク
//
//	RETURN
//		追加前の書式フラグ
//
//	EXCEPTIONS
//
inline
int
ModIos::setf(const int newFlags_, const int mask_)
{
	const int oldFlags = _flags;
	_flags = (_flags & ~mask_) | (newFlags_ & mask_);
	return oldFlags;
}

//	FUNCTION public
//	ModIos::unsetf -- 現在の書式フラグを一部クリア
//
//	NOTES
//	現在の書式フラグにマスクをかけ、マスクのビットが立っている位置のフラグを
//	クリアする。
//
//	ARGUMENTS
//	const int mask_
//		クリアするビットを立てたマスク
//
//	RETURN
//		クリア前の書式フラグ
//
//	EXCEPTIONS
//
inline
int
ModIos::unsetf(const int mask_)
{
	const int oldFlags = _flags;
	_flags &= ~mask_;
	return oldFlags;
}

//	FUNCTION public
//	ModIos::width -- 
//
//	NOTES
//		_width を変更
//
//	ARGUMENTS
//	const ModInt32 newWidth
//		新しい width
//
//	RETURN
//		変更前の width
//
//	EXCEPTIONS
//
inline
ModInt32
ModIos::width(const ModInt32 newWidth_)
{
	const ModInt32 oldWidth = _width;
    _width = newWidth_;
	return oldWidth;
}

//	FUNCTION public
//	ModIos::fill -- 
//
//	NOTES
//		fill 文字を変更する
//
//	ARGUMENTS
//	const char character_
//		新しい fill 文字
//
//	RETURN
//		変更前の fill 文字
//
//	EXCEPTIONS
inline
char
ModIos::fill(const char newFill_)
{
	const char oldFill = _fill;
	_fill = newFill_;
	return oldFill;
}

//	FUNCTION public
//	ModIos::precision -- 
//
//	NOTES
//		precision を変更
//
//	ARGUMENTS
//	const ModInt32 newPrecision
//		新しい precision
//
//	RETURN
//		変更前の precision
//
//	EXCEPTIONS
//
inline
ModInt32
ModIos::precision(const ModInt32 newPrecision_)
{
	const ModInt32 oldPrecision = _precision;
    _precision = newPrecision_;
	return oldPrecision;
}

//
// コンストラクタ、デストラクタ
//

//	FUNCTION public
//	ModIos::ModIos -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ModIos::ModIos()
	: _flags(ModIos::right | ModIos::dec | ModIos::fixed),
										// フラグ: 右揃え、10進、fixed形式
	  _fill(' '),						// fill文字			: 空白文字
	  _width(0),						// フィールド幅		: 最小
	  _precision(6)						// 小数点以下の桁数	: 6 桁
{
	;	// do nothing
}

//	FUNCTION public
//	ModIos::~ModIos -- 
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
inline
ModIos::~ModIos()
{
	;	// do nothing
}

//////////////////////////////////////////////////////////////////
// 書式のマニピュレータ
//////////////////////////////////////////////////////////////////

//	FUNCTION
//	ModHex -- 数値の出力を 16 進数にする
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		与えた以後の数値の出力を 16 進数にする
//
//	ARGUMENTS
//		ModIos&		ios_
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
ModIos&
ModHex(ModIos& ios_)
{
	ios_.setf(ModIos::hex, ModIos::basefield);
	return ios_;
}

//	FUNCTION
//	ModOct -- 数値の出力を 8 進数にする
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		与えた以後の数値の出力を 8 進数にする
//
//	ARGUMENTS
//		ModIos&		ios_
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
ModIos&
ModOct(ModIos& ios_)
{
	ios_.setf(ModIos::oct, ModIos::basefield);
	return ios_;
}

//	FUNCTION
//	ModDec -- 数値の出力を 10 進数にする
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		与えた以後の数値の出力を 16 進数にする
//
//	ARGUMENTS
//		ModIos&		ios_
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
ModIos&
ModDec(ModIos& ios_)
{
	ios_.setf(ModIos::dec, ModIos::basefield);
	return ios_;
}

//	FUNCTION
//	ModFixed -- 数値の出力を浮動小数点形式にする
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		与えた以後の数値の出力を浮動小数点形式(dddd.dd)にする
//
//	ARGUMENTS
//		ModIos&		ios_
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
ModIos&
ModFixed(ModIos& ios_)
{
	ios_.setf(ModIos::fixed, ModIos::floatfield);
	return ios_;
}

//	FUNCTION
//	ModScientific -- 数値の出力を科学形式にする
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		与えた以後の数値の出力を科学形式(d.ddddEdd)にする
//
//	ARGUMENTS
//		ModIos&		ios_
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
ModIos&
ModScientific(ModIos& ios_)
{
	ios_.setf(ModIos::scientific, ModIos::floatfield);
	return ios_;
}

//
// 引数をとる整形マニピュレータのための一時的な構造体とテンプレートクラス
// (C++ 第3版 日本語版 p.723 より引用)
// !! このクラスと ModManipulator は全く同じ役目を持っている。しかしながら、
// !! ModManipulator はメッセージ用に特化されていて使えない。
// !! 今後、ストリームやメッセージを作る直すときは本クラスから派生させ、
// !! このマニピュレータを利用すること。
template <class TYPE>
class ModStreamManipulator
{
public:
	//
	//	FUNCTION public
	//	ModStreamManipulator::ModStreamManipulator -- コンストラクタ
	//
	//	NOTES
	//	コンストラクタ。引数が参照であることに注意。
	//
	//	ARGUMENTS
	//	void (*pFunc_)(ModIos&, const TYPE&)
	//		機能の実現する関数へのポインタ
	//	const TYPE& argument_
	//		マニピュレータの引数
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	その他
	//		下位の例外はそのまま再送
	//
	ModStreamManipulator(void (*pFunc_)(ModIos&, const TYPE&),
						 const TYPE& argument_)
		: _pFunc(pFunc_), _argument(argument_) {}

	//関数へのポインタ
	void (*_pFunc)(ModIos&, const TYPE&);
	//引数
	TYPE _argument;
};

//	FUNCTION
//	ModIosSetW -- 次のフィールドの文字数を指定
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		次のフィールドの文字数を width_ 文字にする。
//
//	ARGUMENTS
//		ModIos&		ios_
//
//		ModInt32	width_
//			次のフィールドの文字数
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
void
ModSetWHelper(ModIos& ios_, const ModInt32& width_)
{
	ios_.width(width_);
}

inline
ModStreamManipulator<ModInt32>
ModIosSetW(const ModInt32 width_)
{
	return ModStreamManipulator<ModInt32> (ModSetWHelper, width_);
}

//	FUNCTION
//	ModIosSetFill -- パディング文字を指定
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		パディング文字数を character_ 文字にする。
//
//	ARGUMENTS
//		ModIos&		ios_
//
//		ModInt32	character_
//			次のフィールドの文字数
//			(ヘルパー関数の型に合わせる)
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
void
ModSetFillHelper(ModIos& ios_, const char& character_)
{
	ios_.fill(character_);
}

inline
ModStreamManipulator<char>
ModIosSetFill(const char character_)
{
	return ModStreamManipulator<char> (ModSetFillHelper, character_);

	// 引数が char 型な理由
	//
	// 今のところ char 型の文字によるパディングしか対応していない。
	// Unicode にも対応するには次のことを考察しなければいけない。
	// １、ModIos クラスのメンバ変数 _fill の型を ModUnicodeChar 型に
	//     変更すること
	// ２、メンバ変数 _fill にセットされた値の正当性検査を適当なところ
	//     で行なうこと。例えば、本クラスの葉性クラス ModOstrStrem は
	//     char型の値しか _fill に使えないので、char の範囲を越えている
	//     場合は何らかのエラー表示が必要になる。でも、ストリームが
	//     安易に例外を出すのは問題がありそうなのでよく考えないといけない。
}

//	FUNCTION
//	ModIosSetPrecision -- precision を指定
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		precision を指定。
//
//	ARGUMENTS
//		ModIos&		ios_
//
//		ModInt32	 precision_
//			precision を指定
//			(ヘルパー関数の型に合わせる)
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
void
ModSetPrecisionHelper(ModIos& ios_, const ModInt32& precision_)
{
	ios_.precision(precision_);
}

inline
ModStreamManipulator<ModInt32>
ModIosSetPrecision(const ModInt32 precision_)
{	
	return ModStreamManipulator<ModInt32> (ModSetPrecisionHelper, precision_);
}

//	FUNCTION
//	ModIosSetBase -- 基数を指定
//
//	NOTES
//		ModIos::operator << への引数として与えることができ、
//		基数を指定。
//
//	ARGUMENTS
//		ModIos&		ios_
//
//		ModInt32	 base_
//			基数を指定。ただし、対応可能なのは 10 進と 16 進だけなので
//			10 と 16 しか指定できない。他の値を指定すると10進と解釈する。
//
//	RETURN
//		引数に渡した ios_
//
//	EXCEPTIONS
//		なし

inline
void
ModSetBaseHelper(ModIos& ios_, const ModInt32& base_)
{
	switch (base_)
	{
	case 8:
		ios_.setf(ModIos::oct, ModIos::basefield);
		break;

	case 16:
		ios_.setf(ModIos::hex, ModIos::basefield);
		break;

	default:
		// デフォルト値は 10進表記
		ios_.setf(ModIos::dec, ModIos::basefield);
		break;
	}
}

inline
ModStreamManipulator<ModInt32>
ModIosSetBase(const ModInt32 base_)
{
	return ModStreamManipulator<ModInt32> (ModSetBaseHelper, base_);
}


#endif	// __ModIos_H__

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
