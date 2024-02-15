// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessage.cpp -- エラーメッセージを作成するクラス
// 
// Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include <stdarg.h>
#include <stdio.h>

#include "Exception/ErrorMessage.h"
#include "Exception/AutoCriticalSection.h"

//
// 以下は例外用
//
#include "Exception/AllNumberFiles.h"

//
// 以下はverify用ここに加えた場合にはMessage/English.cpp, Japanese.cppにも加える
//
// Admin			0x0101xxxx
// Buffer			0x0102xxxx
// Checkpoint		0x0103xxxx
// Common			0x0104xxxx
// Communication	0x0105xxxx
// Exception		0x0106xxxx
// Execution		0x0107xxxx
// Lock				0x0108xxxx
// LogicalFile		0x0109xxxx
// LogicalLog		0x0110xxxx
// Message			0x0111xxxx
// Opt				0x0112xxxx
// Os				0x0113xxxx
// PhysicalFile		0x0114xxxx
#include "PhysicalFile/MessageAll_Number.h"
// Schema			0x0115xxxx
#include "Schema/MessageAll_Number.h"
// Server			0x0116xxxx
// Statement		0x0117xxxx
// Trans			0x0118xxxx
// Version			0x0119xxxx
#include "Version/MessageAll_Number.h"
//
// Btree			0x0130xxxx
#ifndef SYD_CPU_SPARC
#include "Btree/MessageAll_Number.h"
#endif
// FileCommon		0x0131xxxx
// FullText			0x0132xxxx
#include "FullText/MessageAll_Number.h"
// Inverted			0x0133xxxx
#include "Inverted/MessageAll_Number.h"
// Record			0x0134xxxx
#include "Record/MessageAll_Number.h"
// Vector			0x0135xxxx
#ifndef SYD_CPU_SPARC
#include "Vector/MessageAll_Number.h"
#endif
// Lob				0x0136xxxx
#include "Lob/MessageAll_Number.h"
// Btree2			0x0137xxxx
#include "Btree2/MessageAll_Number.h"
// Vector2			0x0138xxxx
#include "Vector2/MessageAll_Number.h"
// Bitmap			0x0139xxxx
#include "Bitmap/MessageAll_Number.h"
// Record2			0x0140xxxx
#include "Record2/MessageAll_Number.h"
// Array			0x0141xxxx
#include "Array/MessageAll_Number.h"
// FullText2		0x0142xxxx
#include "FullText2/MessageAll_Number.h"

#include "ModCriticalSection.h"
#include "ModDefaultManager.h"
#include "ModMap.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"

_TRMEISTER_USING
_TRMEISTER_EXCEPTION_USING

namespace {

	const	ModUnicodeChar	_ucsPercent		=	0x0025;		// '%'
	const	ModUnicodeChar	_ucsHyphen		=	0x002d;		// '-'
	const	ModUnicodeChar	_ucsPeriod		=	0x002e;		// '.'
	const	ModUnicodeChar	_ucsZero		=	0x0030;		// '0'
	const	ModUnicodeChar	_ucsLargeE		=	0x0045;		// 'E'
	const	ModUnicodeChar	_ucsLargeX		=	0x0058;		// 'X'
	const	ModUnicodeChar	_ucsSmallC		=	0x0063;		// 'c'
	const	ModUnicodeChar	_ucsSmallD		=	0x0064;		// 'd'
	const	ModUnicodeChar	_ucsSmallE		=	0x0065;		// 'e'
	const	ModUnicodeChar	_ucsSmallF		=	0x0066;		// 'f'
	const	ModUnicodeChar	_ucsSmallH		=	0x0068;		// 'h'
	const	ModUnicodeChar	_ucsSmallI		=	0x0069;		// 'i'
	const	ModUnicodeChar	_ucsSmallL	   	=	0x006c;		// 'l'
	const	ModUnicodeChar	_ucsSmallO		=	0x006f;		// 'o'
	const	ModUnicodeChar	_ucsSmallS		=	0x0073;		// 's'
	const	ModUnicodeChar	_ucsSmallU		=	0x0075;		// 'u'
	const	ModUnicodeChar	_ucsSmallX		=	0x0078;		// 'x'


	//
	//	VARIALBLE local
	//	_cCriticalSection -- 排他制御用のクリティカルセクション
	//
	//	NOTES
	//	メッセージ引数フォーマットマップ登録時の排他制御を行なう
	//
	ModCriticalSection	_cCriticalSection;

	//	STRUCT local
	//	__FormatTableEntry -- メッセージ引数フォーマット表のエントリー
	//
	//	NOTES

	struct _FormatTableEntry
	{
		//メッセージ番号(エラー番号)
		ErrorNumber::Type m_uiErrorNumber;
		//メッセージ引数フォーマット
		const char* m_pszFormat;
	};

	//
	//	VARIABLE local
	//	_cFormatTable -- メッセージ引数フォーマットのテーブル
	//
	//	NOTES
	//	メッセージ引数フォーマットのテーブル
	//
	_FormatTableEntry _cFormatTable[] = {
#include "Exception/MessageArgument.h"
#include "PhysicalFile/MessageArgument.h"
#include "Schema/MessageArgument.h"
#include "Version/MessageArgument.h"
#ifndef SYD_CPU_SPARC
#include "Btree/MessageArgument.h"
#endif
#include "FullText/MessageArgument.h"
#include "Inverted/MessageArgument.h"
#include "Record/MessageArgument.h"
#ifndef SYD_CPU_SPARC
#include "Vector/MessageArgument.h"
#endif
#include "Lob/MessageArgument.h"
#include "Btree2/MessageArgument.h"
#include "Vector2/MessageArgument.h"
#include "Bitmap/MessageArgument.h"
#include "Record2/MessageArgument.h"
#include "Array/MessageArgument.h"
#include "FullText2/MessageArgument.h"
		{0,	0}
	};

	//	CLASS local
	//	_FormatMap
	//		-- メッセージ引数フォーマット用マップ
	//
	//	NOTES
	//	メッセージ引数フォーマットを登録するマップクラス
	//  char* のデータを ModUnicodeChar* のデータに変換して保存する
	//	再登録を避ける為、マップインスタンスは隠蔽してます
	//
	class _FormatMap : public ModDefaultObject
	{
	public:
		// コンストラクタ
		_FormatMap();
		// デストラクタ
		~_FormatMap();

		// 引数フォーマット登録
		void registFormatData(const ErrorNumber::Type & uiMsgNum_,
							  const char * pszArgFmt_,
							  const unsigned int & uiArgFmtSize_);
		// 引数フォーマット登録問い合わせ
		bool isRegisted(const ErrorNumber::Type & uiMsgNum_);
						
		// 引数フォーマット取得
		const ModUnicodeChar* getUnicodeCharData(
										const ErrorNumber::Type & uiMsgNum_);
		
		// 引数フォーマット取得
		const ModUnicodeChar* operator [] (const ErrorNumber::Type uiMsgNum_);

	private:
		// マップインスタンス
		typedef ModMap<ErrorNumber::Type, ModUnicodeChar*, ModLess<ErrorNumber::Type> >
						UnicodeCharPtrMap;
		UnicodeCharPtrMap		m_mapUnicodeCharPtr;
	};
	
	//////////////////////
	// _FormatMapの実装 //
	//////////////////////
	
	//
	//	FUNCTION public
	//	_FormatMap::_FormatMap
	//		-- コンストラクタ
	//
	//	NOTES
	//	コンストラクタ
	//
	//	ARGUMENTS
	//	なし
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	_FormatMap::
	_FormatMap()
	{
	}
	
	//
	//	FUNCTION public
	//	_FormatMap::~_FormatMap -- デストラクタ
	//
	//	NOTES
	//	デストラクタ
	//	登録されている引数フォーマット文字列をすべて解放する
	//
	//	ARGUMENTS
	//	
	//
	//	RETURN
	//	
	//
	//	EXCEPTIONS
	//	なし
	//
	_FormatMap::
	~_FormatMap()
	{
		UnicodeCharPtrMap::Iterator i = m_mapUnicodeCharPtr.begin();
		while ( i != m_mapUnicodeCharPtr.end() )
		{
			delete [] (*i).second;
			i++;
		}
	}
	
	//
	//	FUNCTION public
	//	_FormatMap::registFormatData -- 引数フォーマット文字列登録
	//
	//	NOTES
	//	引数フォーマット文字列を登録する
	//
	//	ARGUMENTS
	//	const ErrorNumber::Type & uiMsgNum_
	//		メッセージ番号(エラー番号)
	//
	//	const char * pszArgFmt_
	//		メッセージ引数フォーマット文字列
	//
	//	const unsigned int & uiArgFmtSize_
	//		メッセージ引数フォーマット文字列長さ
	//
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし
	//
	void
	_FormatMap::
	registFormatData(const ErrorNumber::Type & uiMsgNum_,
					 const char * pszArgFmt_,
					 const unsigned int & uiArgFmtSize_)
	{
		// 未登録時にのみ登録する
		if (m_mapUnicodeCharPtr.find(uiMsgNum_) == m_mapUnicodeCharPtr.end()) {

			ModUnicodeChar* p = new ModUnicodeChar[uiArgFmtSize_];

			try {
				for (unsigned int i = 0; i < uiArgFmtSize_; ++i)
					p[i] = (pszArgFmt_[i]) ?
						ModUnicodeString(pszArgFmt_[i])[0] :
						ModUnicodeCharTrait::null();

				m_mapUnicodeCharPtr[uiMsgNum_] = p;

			} catch (...) {
				delete p;
				throw;
			}
		}
	}
	
	//
	//	FUNCTION public
	//	_FormatMap::isRegisted -- 引数フォーマット登録問い合わせ
	//
	//	NOTES
	//	引数フォーマット文字列の登録状況を問い合わる
	//
	//	ARGUMENTS
	//	ErrorNumber::Type & uiMsgNum_
	//		メッセージ番号(エラー番号)
	//
	//	RETURN
	//	bool	true  : 登録済
	//			false : 未登録	
	//
	//	EXCEPTIONS
	//	なし
	//
	bool
	_FormatMap::
	isRegisted(const ErrorNumber::Type & uiMsgNum_)
	{
		UnicodeCharPtrMap::Iterator i = m_mapUnicodeCharPtr.find(uiMsgNum_);
		return i != m_mapUnicodeCharPtr.end() ? true : false;
	}
	
	//
	//	FUNCTION public
	//	_FormatMap::getUnicodeCharData -- 引数フォーマット取得
	//
	//	NOTES
	//	引数フォーマット文字列を取得する
	//
	//	ARGUMENTS
	//	const ErrorNumber::Type & uiMsgNum_
	//		メッセージ番号(エラー番号)
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		引数フォーマット文字列
	//		未登録時、0 を返します
	//
	//	EXCEPTIONS
	//	なし
	//
	const ModUnicodeChar*
	_FormatMap::
	getUnicodeCharData(const ErrorNumber::Type & uiMsgNum_)
	{
		ModUnicodeChar* pRet = 0;
		UnicodeCharPtrMap::Iterator i = m_mapUnicodeCharPtr.find(uiMsgNum_);
		if ( i != m_mapUnicodeCharPtr.end() )
		{
			pRet = (*i).second;
		}
		return pRet;
	}
	
	//
	//	FUNCTION public
	//	_FormatMap::operator [] -- 引数フォーマット文字列
	//
	//	NOTES
	//	引数フォーマット文字列を取得する
	//
	//	ARGUMENTS
	//	const ErrorNumber::Type uiMsgNum_
	//		メッセージ番号(エラー番号)
	//
	//	RETURN
	//	const ModUnicodeChar*
	//		引数フォーマット文字列
	//		未登録時、0 を返します
	//
	//	EXCEPTIONS
	//	なし
	//
	const ModUnicodeChar*
	_FormatMap::
	operator [] (const ErrorNumber::Type uiMsgNum_)
	{
		return getUnicodeCharData(uiMsgNum_);
	}

	//
	//	VARIABLE local
	//	_cMapArgFormat -- メッセージ引数フォーマットマップ
	//
	//	NOTES
	//	メッセージ引数フォーマットを登録するマップ
	// 
	_FormatMap		_cMapArgFormat;

	//
	//	ENUM local
	//	FormatType -- フォーマット値種別
	//
	//	NOTES
	//	メッセージ引数フォーマット
	//
	enum _FormatType
	{
		StringType,
		ShortType,
		UShortType,
		LongType,
		ULongType,
		IntType,
		UIntType,
		Int64Type,
		FixFloatType,
		ScientFloatType
	};

	//
	//	FUNCTION local
	//	_RegistFormatForMap --_cMapArgFormat にメッセージ引数を登録する
	//
	//	NOTES
	//	_cMapArgFormat にメッセージ引数を新たに登録する
	//	登録済時は何もしない
	//
	//	ARGUMENTS
	//	ErrorNumber::Type uiMsgNum_
	//		メッセージ番号
	//
	//	RETURN
	//	なし
	//
	//	EXCEPTIONS
	//	なし

	// static
	void
	_RegistFormatForMap(ErrorNumber::Type uiMsgNum_)
	{
		AutoCriticalSection cAuto(_cCriticalSection);

		if ( !_cMapArgFormat.isRegisted(uiMsgNum_) )
		{
			int i(0);
		
			while (_cFormatTable[i].m_pszFormat)
			{			
				if (_cFormatTable[i].m_uiErrorNumber == uiMsgNum_)
				{
					const char* pArgFmt = _cFormatTable[i].m_pszFormat;
				
					// フォーマット文字長取得
					ErrorNumber::Type uiStrLen(0);
					while ( !((pArgFmt[uiStrLen]   == '\000') &&
							  (pArgFmt[uiStrLen+1] == '\000')) )
					{
						uiStrLen++;
					}

					// オフセット
					uiStrLen += 2;

					// 登録
					_cMapArgFormat.registFormatData(uiMsgNum_,
													pArgFmt,
													uiStrLen);
					break;
				}
				i++;
			}
		}
	}

	//
	//	FUNCTION local
	//	_InitOstrStream --ModUnicodeOstrStream の設定を行なう
	//
	//	NOTES
	//	書式文字列に基づいて ModUnicodeOstrStream の設定を行なう
	//
	//	ARGUMENTS
	//	ModUnicodeOstrStream & ostr_
	//		出力ストリーム
	//	const ModUnicodeString * pszFormat_
	//		フォーマット文字列
	//	unsigned int uiFormatLen_
	//
	//	const _FormatType & eValue_
	//
	//
	//	RETURN
	//	ModUnicodeOstrStream &
	//		出力ストリーム
	//
	//	EXCEPTIONS
	//	なし

	ModUnicodeOstrStream &
	_InitOstrStream(ModUnicodeOstrStream & ostr_,
					const ModUnicodeChar * pszFormat_,
					unsigned int		   uiFormatLen_,
					const _FormatType &    eValue_)
	{
		bool bPeriodFlg(false);
		ModUnicodeString strWidth;
		ModUnicodeString strPrecision;

		//--- 書式解析 ---
		for (unsigned int i = 0; i < uiFormatLen_; ++i, ++pszFormat_)
		{
#ifndef SYD_COVERAGE // periodのある書式を使ったメッセージはない
			if ( *pszFormat_ == _ucsPeriod )
			{
				bPeriodFlg = true;
			}
#endif

#ifndef SYD_COVERAGE // 幅指定のある書式を使ったメッセージはない
			// 幅指定文字列作成( 0は含まない )
			if ( ModUnicodeCharTrait::isDigit(*pszFormat_) &&
				 *pszFormat_ != _ucsZero )
			{
				if ( bPeriodFlg )
				{
					strPrecision += *pszFormat_;
				}
				else
				{
					strWidth += *pszFormat_;
				}
				continue;
			}
#endif

#ifndef SYD_COVERAGE // 書式値のある書式を使ったメッセージはない
			// 書式値フィールド
			switch(*pszFormat_)
			{
			case _ucsHyphen: // '-'
				// 現在は Mod が left マニピュレータをサポートしていないため
				// omit
				// ostr_ << ModLeft;
				break;

			case _ucsZero: // '0'
				ostr_ << ModIosSetFill('0');
				break;
			}
#endif
		}

#ifndef SYD_COVERAGE // 出力値指定のある書式を使ったメッセージはない
		//--- 出力値指定 ---
		switch(eValue_)
		{
		case FixFloatType:
			ostr_ << ModFixed;
			break;
		case ScientFloatType:
			ostr_ << ModScientific;
			break;
		}
#endif

#ifndef SYD_COVERAGE // 出力幅指定のある書式を使ったメッセージはない
		//--- 出力幅指定 ---
		if ( strWidth.getLength() > 0 )
		{
			ostr_ << ModIosSetW(int(ModUnicodeCharTrait::toInt(strWidth)));
		}
		if ( strPrecision.getLength() > 0 )
		{
			ostr_ << ModIosSetPrecision(
								int(ModUnicodeCharTrait::toInt(strPrecision)));
		}
#endif

		return ostr_;
	}

	//
	//	FUNCTION local
	//	_Copy
	//
	void _Copy(ModUnicodeChar* buf_,
			   ModUnicodeChar* append_,
			   ModSize& maxLen_)
	{
		ModSize len = ModUnicodeCharTrait::length(append_);
		if (len > maxLen_)
		{
			// 超えちゃうので書ける分の半分ぐらい書く
			len = maxLen_ / 2;
		}
		
		if (len)
		{
			ModUnicodeCharTrait::copy(buf_, append_, len);
		}
		*(buf_ + len) = 0;

		maxLen_ -= len;
	}
}

//
//	FUNCTION public static
//	Exception::ErrorMessage::getArgumentFormat --
//		メッセージ引数フォーマットを得る
//
//	NOTES
//	エラーメッセージ引数フォーマットを得る。
//
//	ARGUMENTS
//	Exception::ErrorNumber::Type iErrorNumber_
//		メッセージ番号
//
//	RETURN
//	const ModUnicodeChar*
//		メッセージ引数フォーマット 得られない場合は0を返す
//
//	EXCEPTION
//	なし

// static
const ModUnicodeChar*
ErrorMessage::
getArgumentFormat(ErrorNumber::Type uiErrorNumber_)
{
	const ModUnicodeChar* pArgFmt = 0;

	// マップ登録(登録済時、何もしない)
	_RegistFormatForMap(uiErrorNumber_);
	
	// マップ内の検索
	return _cMapArgFormat[uiErrorNumber_];
}

//
//	FUNCTION public static
//	Exception::ErrorMessage::getArgumentNumber -- メッセージ引数の数を得る
//
//	NOTES
//	エラーメッセージ引数の数を得る。
//
//	ARGUMENTS
//	ErrorNumber::Type iErrorNumber_
//		メッセージ番号
//
//	RETURN
//	int
//		引数の数。得られない場合は-1を返す。
//
//	EXCEPTION
//	なし

// static
int
ErrorMessage::
getArgumentNumber(ErrorNumber::Type uiErrorNumber_)
{
	const ModUnicodeChar* p = 0;
	int iCount = -1;

	// マップ登録(登録済時、何もしない)
	_RegistFormatForMap(uiErrorNumber_);
	
	// マップ内の検索
	p = _cMapArgFormat[uiErrorNumber_];
	if ( p )
	{
		iCount = 0;
		while(*p)
		{
			iCount++;
			while (*p++);
		}

	}
	return iCount;
}

//
//	FUNCTION public static
//	Exception::ErrorMessage::makeMessageArgument --
//		エラーメッセージ引数を作成する
//
//	NOTES
//	エラーメッセージを作成するための引数を作成する。
//	メッセージ引数フォーマットは以下のような形式でなければならない。
//		"<引数1形式>\0<引数2形式>\0...<引数N形式>\0\0"
//	たとえば以下のようになる。
//		"%s\0%d\0%04X\0\0"
//	メッセージフォーマットは以下のような形式でかかれている
//		"error message %1 error message %2 %3"
//	文字列の%1,%2,...,%nは番号順にその後の引数形式を埋め込む場所を
//	指定している。%そのものを印字したいときは"%%"と記述する。
//	また、nの値は1から9までである。
//	メッセージ引数フォーマット%?の?に指定できる文字は、
//		s, d, i, c, u, o, x, X, f, e, E,
//		hd, hi, hu, ho, hx, hX,
//		ld, li, lu, lo, lx, lX,
//		lld, lli, llu, llo, llx, llX
//	のみである。
//
//	ARGUMENTS
//	ModUnicodeChar* pszArgument_
//		作成した引数を格納するバッファ
//	ErrorNumber::Type uiErrorNumber_
//		エラーメッセージのメッセージ番号
//	...
//		エラーメッセージ引数
//
//	RETURN
//	ModUnicodeChar*
//		エラーメッセージ引数を展開した文字列配列。
//		intやdoubleの引数を文字列に変換して以下のような形式で格納する。
//			"<引数1>\0<引数2>\0...<引数N>\0\0"
//
//	EXCEPTIONS
//	なし

// static
ModUnicodeChar*
ErrorMessage::
makeMessageArgument(ModUnicodeChar* pszArgument_,
					ErrorNumber::Type uiErrorNumber_, ...)
{
	va_list ap;
	va_start(ap, uiErrorNumber_);

	const ModUnicodeChar* p = getArgumentFormat(uiErrorNumber_);
	ModUnicodeChar* s = pszArgument_;
	ModUnicodeChar* d = pszArgument_;
	*d = 0;
	if (p)
	{
		// 引数の数を求める
		int acount = getArgumentNumber(uiErrorNumber_);
		
		// バッファの最大長を得る
		//【注意】
		// バッファとは、Exception::Object の m_pszErrorMessageArgument[] のこと
		ModSize maxLen = 1024 - (acount + 1);
		
		ModUnicodeOstrStream cOstrBuff;
		const ModUnicodeChar* pFormat;
		while (*p)
		{
			//文字列に変換する
			const ModUnicodeChar* p0 = p;

			// '%' の次まで読み飛ばし
			while (*p0++ != _ucsPercent);
			pFormat = p0;

			// 0-9, '-', '.' の読み飛ばし
			while (ModUnicodeCharTrait::isDigit(*p0) == ModTrue
				   || *p0 == _ucsHyphen
				   || *p0 == _ucsPeriod) p0++;

			switch (*p0)
			{
			case _ucsSmallS:		// 's'
				{
					_InitOstrStream(
						cOstrBuff, pFormat,
						static_cast<unsigned int>(p0 - pFormat),
						StringType);
					ModUnicodeChar * n = va_arg(ap, ModUnicodeChar*);
					cOstrBuff << n;
					_Copy(d, cOstrBuff.getString(), maxLen);
					cOstrBuff.clear();
				}
				break;
#ifndef SYD_COVERAGE // shortを引数にするメッセージはない
			case _ucsSmallH:		// 'h'
				//short
				p0++;
				switch (*p0)
				{
				case _ucsSmallD:	// 'd'
				case _ucsSmallI:	// 'i'
					{
						_InitOstrStream(
							cOstrBuff, pFormat,
							static_cast<unsigned int>(p0 - pFormat),
							ShortType);
						short n = va_arg(ap, short);
						cOstrBuff << n;
						_Copy(d, cOstrBuff.getString(), maxLen);
						cOstrBuff.clear();
					}
					break;
				case _ucsSmallU:	// 'u'
				case _ucsSmallO:	// 'o'
				case _ucsSmallX:	// 'x'
				case _ucsLargeX:	// 'X'
					{
						_InitOstrStream(
							cOstrBuff, pFormat,
							static_cast<unsigned int>(p0 - pFormat),
							UShortType);
						unsigned short n = va_arg(ap, unsigned short);
						if (*p0 == _ucsSmallX || *p0 == _ucsLargeX) {
							cOstrBuff << ModHex << n << ModDec;
						} else {
							cOstrBuff << n;
						}
						_Copy(d, cOstrBuff.getString(), maxLen);
						cOstrBuff.clear();
					}
					break;
				}
				break;
#endif
			case _ucsSmallL:		// 'l'
				//long
				p0++;
				switch (*p0)
				{
				case _ucsSmallL:	// 'l'
					//long long
					p0++;
					switch (*p0)
					{
					case _ucsSmallD:	// 'd'
					case _ucsSmallI:	// 'i'
						{
							_InitOstrStream(
								cOstrBuff, pFormat,
								static_cast<unsigned int>(p0 - pFormat),
								Int64Type);
							ModInt64 n = va_arg(ap, ModInt64);
							cOstrBuff << n;
							_Copy(d, cOstrBuff.getString(), maxLen);
							cOstrBuff.clear();
						}
						break;
					case _ucsSmallU:	// 'u'
					case _ucsSmallO:	// 'o'
					case _ucsSmallX:	// 'x'
					case _ucsLargeX:	// 'X'
						{
							_InitOstrStream(
								cOstrBuff, pFormat,
								static_cast<unsigned int>(p0 - pFormat),
								Int64Type);
							ModInt64 n = va_arg(ap, ModInt64);
							if (*p0 == _ucsSmallX || *p0 == _ucsLargeX) {
								cOstrBuff << ModHex << n << ModDec;
							} else {
								cOstrBuff << n;
							}
							_Copy(d, cOstrBuff.getString(), maxLen);
							cOstrBuff.clear();
						}
						break;
					}
					break;
#ifndef SYD_COVERAGE // %ld, %liを使うメッセージはない
				case _ucsSmallD:		// 'd'
				case _ucsSmallI:		// 'i'
					{
						_InitOstrStream(
							cOstrBuff, pFormat,
							static_cast<unsigned int>(p0 - pFormat),
							LongType);
						long n = va_arg(ap, long);
						cOstrBuff << n;
						_Copy(d, cOstrBuff.getString(), maxLen);
						cOstrBuff.clear();
					}
					break;
#endif
				case _ucsSmallU:		// 'u'
				case _ucsSmallO:		// 'o'
				case _ucsSmallX:		// 'x'
				case _ucsLargeX:		// 'X'
					{
						_InitOstrStream(
							cOstrBuff, pFormat,
							static_cast<unsigned int>(p0 - pFormat),
							ULongType);
						unsigned long n = va_arg(ap, unsigned long);
						if (*p0 == _ucsSmallX || *p0 == _ucsLargeX) {
							cOstrBuff << ModHex << n << ModDec;
						} else {
							cOstrBuff << n;
						}
						_Copy(d, cOstrBuff.getString(), maxLen);
						cOstrBuff.clear();
					}
					break;
				}
				break;
			case _ucsSmallD:			// 'd'
			case _ucsSmallI:			// 'i' 
			case _ucsSmallC:			// 'c'
				{
					_InitOstrStream(
						cOstrBuff, pFormat,
						static_cast<unsigned int>(p0 - pFormat),
						IntType);
					int n = va_arg(ap, int);
					cOstrBuff << n;
					_Copy(d, cOstrBuff.getString(), maxLen);
					cOstrBuff.clear();
				}
				break;
#ifndef SYD_COVERAGE // uint, floatを引数にするメッセージはない
			case _ucsSmallU:			// 'u'
			case _ucsSmallO:			// 'o'
			case _ucsSmallX:			// 'x'
			case _ucsLargeX:			// 'X'
				{
					_InitOstrStream(
						cOstrBuff, pFormat,
						static_cast<unsigned int>(p0 - pFormat),
						UIntType);
					ErrorNumber::Type n = va_arg(ap, ErrorNumber::Type);
					if (*p0 == _ucsSmallX || *p0 == _ucsLargeX) {
						cOstrBuff << ModHex << n << ModDec;
					} else {
						cOstrBuff << n;
					}
					_Copy(d, cOstrBuff.getString(), maxLen);
					cOstrBuff.clear();
				}
				break;
			case _ucsSmallF:			// 'f'
				{
					_InitOstrStream(
						cOstrBuff, pFormat,
						static_cast<unsigned int>(p0 - pFormat),
						FixFloatType);
					double n = va_arg(ap, double);
					cOstrBuff << n;
					_Copy(d, cOstrBuff.getString(), maxLen);
					cOstrBuff.clear();
				}
			case _ucsSmallE:			// 'e'
			case _ucsLargeE:			// 'E'
				{
					_InitOstrStream(
						cOstrBuff, pFormat,
						static_cast<unsigned int>(p0 - pFormat),
						ScientFloatType);
					double n = va_arg(ap, double);
					cOstrBuff << n;
					_Copy(d, cOstrBuff.getString(), maxLen);
					cOstrBuff.clear();
				}
				break;
#endif
			} // switch (*p) end

			//\0まで読み飛ばす
			while (*p++);
			while (*d++);

		} // while (*p)  end
	}
	else
	{
		//フォーマットが得られなかったので"\000\000"を埋める
		d++;
	}
	*d = 0;
	
	va_end(ap);

	return pszArgument_;
}

//
//	FUNCTION static
//	Exception::ErrorMessage::getMessageArgumentElement --
//		エラーメッセージ引数の要素を取出す
//
//	NOTES
//	エラーメッセージ引数の指定の要素番号の文字列を取出す。
//
//	ARGUMENTS
//	const char* pszArgument_
//		エラーメッセージ引数
//	int iElement_
//		要素番号
//
//	RETURN
//	const char*
//		指定の要素番号の文字列
//
//	EXCEPTIONS
//	なし
//

//static
const ModUnicodeChar*
Exception::ErrorMessage::
getMessageArgumentElement(const ModUnicodeChar* pszArgument_, int iElement_)
{
	const ModUnicodeChar* p = pszArgument_;
	while (iElement_ != 0)
	{
		//次まで読み飛ばす
		while (*p++);
		iElement_--;
	}
	return p;
}

//
//	Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
