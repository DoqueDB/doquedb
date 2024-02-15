// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModOstream.cpp -- MOD の出力用ストリーム
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


#include <stdio.h>
#include <math.h>
#include "ModOstream.h"
#include "ModAutoMutex.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModWideString.h"

// 整数を文字列に変換した時の文字数(桁数+符号)(終端文字は含まない)
// (文字列が含んでもよい文字 = '+', '-', 数値(0-9, a-z))
const int
ModOstream::_integerMax = 32;

// 小数部分を文字列に変換した時の文字数(桁数)(小数点、終端文字は含まない)
// (文字列が含んでもよい文字 = 数値(0-9))
const int
ModOstream::_fractionMax = 16;

// 小数部分の桁数を文字列にした場合の文字数
// (_fractionMax が '10' ならば _fractionCharacterMax は 2 となる)
const int
ModOstream::_fractionCharacterMax = 2;

// 指数部分を文字列に変換した時の文字数(終端文字は含まない)
// (文字列が含んでもよい文字 = 'e', '+'or '-', 数値(0-9))
// (数値は最大4桁 -- IEEE の倍精度では 1024 が最大値)
const int
ModOstream::_exponentMax = 6;

// 以下のメンバーを保護するためのラッチ

ModOsMutex
ModOstream::_latch;

//
// double 型の値を変換するためのバッファ
// (詳細な説明はヘッダファイルにあります)
//
char
ModOstream::_buffer[328];


//	FUNCTION public
//	ModOstream::operator << -- << 演算子
//
//	NOTES
//		出力ストリームに与えられた値を追加する
//
//	ARGUMENTS
//		int						value_
//			追加する int
//		unsigned int			value_
//			追加する unsigned int
//		long					value_
//			追加する long
//		unsigned long			value_
//			追加する unsigned long
//		ModInt64				value_
//			追加する ModInt64
//		ModUInt64				value_
//			追加する ModUInt64
//		double					value_
//			追加する double
//		ModCharString&			string_
//			追加する ModCharString
//		ModUnicodeString&		string_
//			追加する ModUnicodeString
//		ModWideString&			string_
//			追加する ModWideString
//
//	RETURN
//		与えられた値を追加後の自分自身
//
//	EXCEPTIONS

ModOstream&
ModOstream::operator <<(const int value_)
{
	unsigned int	absValue = (unsigned int) value_;
	ModBoolean		negative = ModFalse;

	if (value_ < 0 && (flags() & (ModIos::oct | ModIos::hex)) == 0) {
		// oct, hex は負数をキャストした値を表示するのが正しいようだ
		absValue = (unsigned int) ((-1) * value_);
		negative = ModTrue;
	}

	ModOstream::writeInteger(*this, absValue, negative);
	return *this;
}

ModOstream&
ModOstream::operator <<(const unsigned int value_)
{
	ModOstream::writeInteger(*this, (ModUInt64)value_, ModFalse);
	return *this;
}

ModOstream&
ModOstream::operator <<(const long value_)
{
	unsigned long	absValue = (unsigned long) value_;
	ModBoolean		negative = ModFalse;

	if (value_ < 0 && (flags() & (ModIos::oct | ModIos::hex)) == 0) {
		// oct, hex は負数をキャストした値を表示するのが正しいようだ
		absValue = (unsigned long) ((-1) * value_);
		negative = ModTrue;
	}

	ModOstream::writeInteger(*this, absValue, negative);
	return *this;
}

ModOstream&
ModOstream::operator <<(const unsigned long value_)
{
	ModOstream::writeInteger(*this, (ModUInt64)value_, ModFalse);
	return *this;
}

ModOstream&
ModOstream::operator <<(const char* string_)
{
	const ModInt32 tmpWidth = width(0); // フィールド幅を0にリセット

	if  (tmpWidth <= 0) {
		// パディングについて何も考慮せずに表示
		write(string_);
	} else {
		// 「左」か「右」に揃える。
		// 「内部」が指定されている場合は「右」に揃える。
		ModInt32 space = tmpWidth - (ModInt32)ModCharTrait::length(string_);
		if (flags() & ModIos::left) {
			// 左揃えで出力 (文字列を書いてから余白を fill 文字で埋める)
			write(string_);
			for (; space > 0; --space) {
				put(_fill);
			}
		} else {
			// 右揃えで出力 (余白を fill 文字で埋めてから文字列を書く)
			for (; space > 0; --space) {
				put(_fill);
			}
			write(string_);
		}
	}

	return *this;
}

ModOstream&
ModOstream::operator <<(const ModUnicodeChar* string_)
{
	const ModInt32 tmpWidth = width(0); // フィールド幅を0にリセット

	if  (tmpWidth <= 0) {
		// パディングについて何も考慮せずに表示
		write(string_);
	} else {
		// 「左」か「右」に揃える。
		// 「内部」が指定されている場合は「右」に揃える。
		ModInt32 space =
			tmpWidth - (ModInt32)ModUnicodeCharTrait::length(string_);
		if (flags() & ModIos::left) {
			// 左揃えで出力 (文字列を書いてから余白を fill 文字で埋める)
			write(string_);
			for (; space > 0; --space) {
				put(_fill);
			}
		} else {
			// 右揃えで出力 (余白を fill 文字で埋めてから文字列を書く)
			for (; space > 0; --space) {
				put(_fill);
			}
			write(string_);
		}
	}

	return *this;
}

ModOstream&
ModOstream::operator <<(const ModInt64 value_)
{
	ModUInt64		absValue = (ModUInt64) value_;
	ModBoolean		negative = ModFalse;

	if (value_ < 0 && (flags() & (ModIos::oct | ModIos::hex)) == 0) {
		// oct, hex は負数をキャストした値を表示するのが正しいようだ
		absValue = (ModUInt64) ((-1) * value_);
		negative = ModTrue;
	}

	ModOstream::writeInteger(*this, absValue, negative);
	return *this;
}

ModOstream&
ModOstream::operator <<(const ModUInt64 value_)
{
	ModOstream::writeInteger(*this, value_, ModFalse);
	return *this;
}

//
// ModAutoMutex を使っている理由 (2000/06/19)
//
// double の最大(小)値の指数部分は 308 である。
// そのため Fixed 形式で最大値を表現すると整数部分だけでも 309 桁になる。
// (障害として報告されるまで整数部分の桁数を小さな値と勘違いしていた)
//
// こんな大きな領域(309＋αバイト)を関数スタックに確保するのは異常な事だし、
// sprintf を実行する前に変換後の文字列長さを知ることも難しい。
//
// 結局、C の算術ライブラリにある fcvt (ほとんど fprintf(%w.nf)とおなじ機能)
// と同じように充分な大きさを持った静的なバッファを使って変換することにした。
// そして、マルチスレッド環境でこの静的なバッファを保護するために mutex を
// 使っている。
//

ModOstream&
ModOstream::operator <<(const double value_)
{
	char*			p		= 0;
	ModInt32		tmp		= 0;

	// sprintf に渡す文字列のバッファ ("%." + precisione + 'e'or'f' + NUL文字)
	const ModSize	srcSize	= 2 + _fractionCharacterMax + 1 + 1;
	char 			src[srcSize];

	// 小数点以下の桁は上限桁数まで sprintf から得た文字列を使う。
	// 上限桁数を越えた部分は '0' で埋める
	ModInt32 realPrecision	= _precision;	// sprintf から取得する桁数
	ModInt32 dummyPrecision	= 0;			// '0' で埋める桁数
	if (realPrecision >= _fractionMax) {
		dummyPrecision	= realPrecision - _fractionMax;
		realPrecision	= _fractionMax;
	}

	//
	// sprintf に渡す文字列を作成
	//
	p = src + srcSize;
	*(--p) = ModCharTrait::null();
	if (ModIos::_flags & ModIos::scientific) {
		*(--p) = 'e';
	} else {
		// fixed 形式をデフォルトとする
		*(--p) = 'f';
	}
	tmp = realPrecision;
	do {
		*(--p) = (tmp % 10) + '0';
	    tmp /= 10;
	} while (tmp != 0);
	*(--p) = '.';
	*(--p) = '%';

	if (ModIos::_flags & ModIos::scientific) {
		// Sientific を文字列に変換すると "○.○e+○" という形式になる。
		// 整数部分は常に1桁、小数部分は最大で _fractionMax 桁(16桁)、
		// 指数部分は最大 3 桁(上限値は308)。
		// 従って sprintf の結果は指数の前にある "e+" と小数部の前にある
		// "." を含めて 23 文字＋終端文字で表現できる。
		// 必要な領域はそんなに大きくないので関数スタック上に用意してあげる。
		const ModSize	dstSize = 24;
		char			dst[dstSize];

		// value_ を p の書式で文字列に変換した結果を dst に書き込む 
		sprintf(dst, p, value_);

		doubleOperatorHelper(this, dst, (value_ < 0) ? ModTrue : ModFalse,
							 dummyPrecision);
	} else {
		// sprintf の結果を格納するバッファが static なので保護する
		ModAutoMutex<ModOsMutex> autoMutex(&_latch);
		autoMutex.lock();
		
		// value_ を p の書式で文字列に変換した結果を dst に書き込む 
		sprintf(_buffer, p, value_);

		doubleOperatorHelper(this, _buffer, (value_ < 0) ? ModTrue : ModFalse,
							 dummyPrecision);
	}

	return *this;
}

//
// ModCharString、ModUnicodeString のヘッダファイルを ModOstream.h で
// インクルードするのが嫌なので下の operator<< は inline にしなかった。
// (それぞれのクラスでオーバーロードしたキャストを使っているのが原因)
//

ModOstream&
ModOstream::operator <<(const ModCharString& string_)
{
	return this->operator <<((const char*)string_);
}

ModOstream&
ModOstream::operator <<(const ModUnicodeString& string_)
{
	return this->operator <<((const ModUnicodeChar*)string_);
}


// getString() を利用したいので引数は const にできない
ModOstream&
ModOstream::operator <<(ModWideString& string_)
{
	// ワイド文字から変換可能なコードは EUC と SJIS だけらしいので、
	// 可能な場合だけ変換することにする
	const ModKanjiCode::KanjiCodeType type = ModOs::Process::getEncodingType();
	if (type == ModKanjiCode::euc || type == ModKanjiCode::shiftJis) {
		
		this->write(string_.getString(type));
	}
	return *this;
}

ModOstream&
ModOstream::operator <<(const ModWideChar* string_)
{
	ModWideString	wideString(string_);

	// ワイド文字から変換可能なコードは EUC と SJIS だけらしいので、
	// 可能な場合だけ変換することにする
	const ModKanjiCode::KanjiCodeType type = ModOs::Process::getEncodingType();
	if (type == ModKanjiCode::euc || type == ModKanjiCode::shiftJis) {
		
		this->write(wideString.getString(type));
	}
	return *this;
}


//
// PRIVATE
//

// static
void
ModOstream::doubleOperatorHelper(ModOstream*		stream_,
								 const char*		buffer_,
								 const ModBoolean	negative_,
								 const ModInt32		dummyPrecision_)
{
	ModInt32		tmp		= 0;

	// ポインタ p をストリームへの書き込みに使うために初期化 (負符号は除去)
	ModBoolean	posFlag		= ModFalse;
	const char*	p			= buffer_;
	if (buffer_[0] == '-') {
		posFlag = ModTrue;	// 負の場合は必ず符号を出力する
		++p;
	}
	// フラグがセットされている場合は負でなくても符号を出力
	if ((stream_->_flags & ModIos::showpos) && *p >= '0' && *p <= '9') {
		// 結果が文字("Nan" や "Inf") でない場合は符号の追加を行なう
		posFlag = ModTrue;
	}
	// fill 文字で埋める余白の長さを求める
	ModInt32 len =
		stream_->width(0) - (ModCharTrait::length(p) + dummyPrecision_);
	if (posFlag) {
		--len;	// 符号を表示する場合は余白を１文字減らす
	}
	
	//
	// ストリームに書き込む
	//
	const char fill = stream_->_fill;	// fill 文字

	// ASCII文字に決まっているので文字コードをunknownにする
	ModKanjiCode::KanjiCodeType ePrevCode = stream_->setEncodingType(ModKanjiCode::unknown);

	// 右揃えの場合のパディング
	if (stream_->flags() & ModIos::right) {
		for (; len > 0; --len) {	// len が正の時だけパディングを実行
			stream_->put(fill);
		}
	}
	
	// 符号を書き込む
	if (posFlag) {
		if (negative_) {
			stream_->put('-');
		} else {
			stream_->put('+');
		}
	}
	
	// 内部揃えの場合のパディング
	if (stream_->flags() & ModIos::internal) {
		for (; len > 0; --len) {	// len が正の時だけパディングを実行
			stream_->put(fill);
		}
	}
	
	// 文字列化した数値の指数部の手前までストリームに書き込む
	// (指数部は後で書く --- fixed は指数部がないので気にしなくていい)
	for (; *p != 'e' && *p != ModCharTrait::null(); ++p) {
		stream_->put(*p);
	}
	
	// precision が巨大な場合は '0' で埋める
	for (tmp = 0; tmp < dummyPrecision_; ++tmp) {
		stream_->put('0');
	}
	
	// 指数部を書き込む
	for (; *p != ModCharTrait::null(); ++p) {
		stream_->put(*p);
	}
	
	// 左揃えの場合のパディング
	if (stream_->flags() & ModIos::left) {
		for (; len > 0; --len) {	// len が正の時だけパディングを実行
			stream_->put(fill);
		}
	}
	// 文字コードを元に戻す
	stream_->setEncodingType(ePrevCode);
}

//	FUNCTION private
//	ModOstream:: -- 
//
//	NOTES
//	浮動小数(float, double)以外の数値を文字列に変換して出力ストリームに
//	書き込む。
//
//	absValue_ には数値の絶対値を渡し、数値が負の場合は negative_ を真にする。
//
//	ARGUMENTS
//	ModOstream&	stream_
//		文字列の書き込み先
//	const ModUInt64			absValue_
//		小数(float, double)以外の数値の絶対値
//	const ModBoolean		negative_
//		absValue_ を負の値として文字列を作成したいときは ModTrue とする
//		
//	RETURN
//		なし
//
//	EXCEPTIONS
void
ModOstream::writeInteger(ModOstream&				stream_,
						 const ModUInt64			absValue_,
						 const ModBoolean			negative_)
{
	// (数値用のバッファサイズについて)
	// 最も桁数が大ききなるのは8進数で 0xffffffffffffffff を表現した
	// 1777777777777777777777 である。つまり、数値は最大22文字である。
	// 終端文字も含めると数値には最大23文字の領域が必要である。
	const int	numberBufferSize = 23;
	char		numberBuffer[numberBufferSize];
	char*		pNumber = numberBuffer + numberBufferSize;

	// (符号と基数指示子用のバッファサイズについて)
	// 符号には1文字必要(10進数の場合だけ)で、基数指示子のためには
	// 最大2文字必要である(8進:"0", 10進:なし, 16進:"0x")。
	// 終端文字も含めると数値以外に最大3文字の領域が必要である。
	const int	extendBufferSize = 3;
	char		extendBuffer[extendBufferSize];
	char*		pExtend = extendBuffer + extendBufferSize;

	ModUInt64	tmpAbsValue			= absValue_;

	// 終端文字を書いてバッファを初期化
	*(--pNumber) = ModCharTrait::null();
	*(--pExtend) = ModCharTrait::null();

	//
	// バッファに出力
	//
	if (stream_.flags() & ModIos::oct) {
		// 8進表記の文字列を作成
		do {
			*(--pNumber) = (int)(tmpAbsValue & 7) + '0';
			tmpAbsValue = tmpAbsValue >> 3;
		} while (tmpAbsValue != 0);

		// 数値が "0" でない場合だけ基数指示子を出力
		if ((stream_.flags() & ModIos::showbase) && (*pNumber != '0')) {
			*(--pExtend) = '0';
		}
		// 8進、16進はフラグ showpos を無視するように実装するらしい
		// (VC, CC, g++ で "+" が表示されないことを確認)

	} else if (stream_.flags() & ModIos::hex) {
		// 16進表記の文字列を作成
		const char* data = "0123456789abcdef";
		do {
			*(--pNumber) = data[(tmpAbsValue & 15)];
			tmpAbsValue = tmpAbsValue >> 4;
		} while (tmpAbsValue != 0);

		// 数値が "0" でない場合だけ基数指示子を出力
		if ((stream_.flags() & ModIos::showbase) && (*pNumber != '0')) {
			*(--pExtend) = 'x';
			*(--pExtend) = '0';
		}
		// 8進、16進はフラグ showpos を無視するように実装するらしい
		// (VC, CC, g++ で "+" が表示されないことを確認)

	} else {
		// 10進表記の文字列を作成
		do {
			*(--pNumber) = (int)(tmpAbsValue % 10) + '0';
			tmpAbsValue /= 10;
		} while (tmpAbsValue != 0);

		if (negative_ == ModTrue || (stream_.flags() & ModIos::showpos)) {
			// 負の場合、または showops が指定されている場合は符号を表示
			// ちなみに、ゼロは正の値として扱うのが正しい実装のようだ。
			*(--pExtend) = negative_ ? (absValue_ == 0 ? '+' : '-') : '+';
		}
    }
	// 文字列を末尾から先頭に向けて追加していった結果、先頭を越えていない
	// 事を確認しておく
	; ModAssert(pNumber >= numberBuffer);
	; ModAssert(pExtend >= extendBuffer);

	// 作成した文字列から終端文字を除いた文字数を求める
	ModInt32	length =
		(ModInt32)(
			(numberBuffer + numberBufferSize - pNumber - 1) +
			(extendBuffer + extendBufferSize - pExtend - 1));
	// フィールド幅と fill 文字を取得
	ModInt32	width	= stream_.width(0); // 出力したらフィールド幅をクリア
	const char	fill	= stream_.fill();

	// すべてASCII文字なので文字コードはunknownにしておく
	ModKanjiCode::KanjiCodeType ePrevCode = stream_.setEncodingType(ModKanjiCode::unknown);

	// バッファの内容を書き込む。余白は fill 文字で埋める。
	//
	// バッファの内容は全て ASCII 文字(7bit) になるはずなので put に渡した
	// 時にコード変換のオーバーヘッドが生じない。
	// （注意	fill 文字に半角カナ文字がセットされている可能性もあるので、
	//			よほどの事がない限りエンコーディングタイプを変えて fill
	//			文字を書き込んではいけない。)

	if (stream_.flags() & ModIos::internal) {
		// 内部揃え(pExtend は左揃え。pNumber は右揃え)
		for (; *pExtend != ModCharTrait::null(); ++pExtend) {
			stream_.put(*pExtend);
		}
		for (; width > length; --width) {
			stream_.put(fill);
		}
		for (; *pNumber != ModCharTrait::null(); ++pNumber) {
			stream_.put(*pNumber);
		}
	} else if (stream_.flags() & ModIos::left) {
		// 左揃えで出力 (バッファを書いてから余白を fill 文字で埋める)
		for (; *pExtend != ModCharTrait::null(); ++pExtend) {
			stream_.put(*pExtend);
		}
		for (; *pNumber != ModCharTrait::null(); ++pNumber) {
			stream_.put(*pNumber);
		}
		for (; width > length; --width) {
			stream_.put(fill);
		}
	} else {
		// 右揃えで出力 (余白を fill 文字で埋めてからバッファを書く)
		for (; width > length; --width) {
			stream_.put(fill);
		}
		for (; *pExtend != ModCharTrait::null(); ++pExtend) {
			stream_.put(*pExtend);
		}
		for (; *pNumber != ModCharTrait::null(); ++pNumber) {
			stream_.put(*pNumber);
		}
	}
	// 文字コードを戻しておく
	stream_.setEncodingType(ePrevCode);
}

//
// Copyright (c) 2000, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
