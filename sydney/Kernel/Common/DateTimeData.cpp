// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DateTimeData.cpp -- 日時型データの関連の関数定義
// 
// Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"
#include "Common/IntegerArrayData.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Common/DateTimeData.h"
#include "Common/UnicodeString.h"

#include "Exception/ClassCast.h"
#include "Exception/BadArgument.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/InvalidDatetimeFormat.h"
#include "Os/Memory.h"

#include "ModCharTrait.h"
#include "ModTime.h"
#include "ModUnicodeString.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeOstrStream.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
// ダンプサイズ
// ★注意★
// この値にDateDataのダンプサイズを加えた値がこのデータのダンプサイズ
const ModSize _iDumpSize = sizeof(unsigned int);

//
//	CONST local
//	$$::_pszDefaultDateDelimiter
//							-- 年月日を区切るデフォルトのデリミタ
//
//	NOTES
//	年月日を区切るデフォルトのデリミタ（"-"）
//
const char* const _pszDefaultDateDelimiter = "-";
const ModUnicodeString _cstrDefaultDateDelimiter =
	ModUnicodeString(_pszDefaultDateDelimiter, LiteralCode);

//
//	CONST local
//	$$::_pszDefaultDateTimeDelimiter
//							-- 年月日と時分秒を区切るデフォルトのデリミタ
//
//	NOTES
//	年月日と時分秒を区切るデフォルトのデリミタ（" "）
//
const char* const _pszDefaultDateTimeDelimiter = " ";

//
//	CONST local
//	$$::_pszDefaultTimeDelimiter
//							-- 時、分、秒を区切るデフォルトのデリミタ
//	NOTES
//	時、分、秒を区切るデフォルトのデリミタ（":"）
//
const char* const _pszDefaultTimeDelimiter = ":";

//
//	CONST local
//	$$::_pszDefaultMsecDelimiter
//							-- 秒、ミリ秒を区切るデフォルトのデリミタ
//
//	NOTES
//	秒、ミリ秒を区切るデフォルトのデリミタ（"."）
//
const char* const _pszDefaultMsecDelimiter = ".";

namespace _DateTimeData
{
	//時間が正しいかチェックする
	bool checkTime(int iHour_, int iMinute_, int iSecond_, int iMillisecond_);

	// set value from a string data
	template <class _CHAR_, class _TRAITS_, class _DELIMITER_>
	void setValue(DateTimeData& cData_,
				  const _CHAR_* pHead_, const _CHAR_* pTail_, bool bForAssign_,
				  const _DELIMITER_& cDateDelimiter_, int iDateDelimiterLength_,
				  const _DELIMITER_& cDateTimeDelimiter_, int iDateTimeDelimiterLength_,
				  const _DELIMITER_& cTimeDelimiter_, int iTimeDelimiterLength_,
				  const _DELIMITER_& cMsecDelimiter_, int iMsecDelimiterLength_)
	{
		DateData cDateData;
		int iHour = 0;
		int iMinute = 0;
		int iSecond = 0;
		int iMillisecond = 0;
		int iPrecision = 0;

		const _CHAR_* p = pHead_;
		const _CHAR_* q = (p < pTail_)
			? _TRAITS_::find(p, cDateTimeDelimiter_,
							 static_cast<ModSize>(pTail_ - p)) : 0;

		if (q == 0) q = pTail_;

		cDateData.setValue(p, q, cDateDelimiter_, bForAssign_);

		if (p < q) {
			p = q + iDateTimeDelimiterLength_;
			q = (p < pTail_)
				? _TRAITS_::find(p, cTimeDelimiter_,
								 static_cast<ModSize>(pTail_ - p)) : 0;
	
			if ((p < q) && StringData::getInteger(iHour, p, q)) {

				p = q + iTimeDelimiterLength_;
				q = (p < pTail_)
					? _TRAITS_::find(p, cTimeDelimiter_,
									 static_cast<ModSize>(pTail_ - p)) : 0;

				if (q == 0) q = pTail_;

				if ((p < q) && StringData::getInteger(iMinute, p, q)) {

					p = q + 1;
					q = (p < pTail_)
						? _TRAITS_::find(p, cMsecDelimiter_,
										 static_cast<ModSize>(pTail_ - p)) : 0;

					if (q == 0) q = pTail_;

					if ((p < q) && StringData::getInteger(iSecond, p, q)) {

						p = q + iMsecDelimiterLength_;
						q = pTail_;

						if ((p < q) && StringData::getInteger(iMillisecond, p, q)) {
							while (*(q - 1) == ' ') --q;
							iPrecision = static_cast<int>(q - p);
						}
					}
				}
			}
		}
		cData_.setValue(cDateData, iHour, iMinute, iSecond, iMillisecond, iPrecision, bForAssign_);
	}
}

//	FUNCTION
//	$$$::_DateTimeData::checkTime -- 時間が正しいかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//	int iMillisecond_
//		ミリ秒
//
//	RETURN
//	正しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし

inline
bool
_DateTimeData::checkTime(int iHour_, int iMinute_, int iSecond_, int iMillisecond_)
{
	bool bResult = false;
	if (iHour_ >= 0 && iHour_ < 24)
	if (iMinute_ >= 0 && iMinute_ < 60)
	if (iSecond_ >= 0 && iSecond_ < 60)
	if (iMillisecond_ >= 0 && iMillisecond_ < 1000)
		bResult = true;
	return bResult;
}

}

// VARIABLE private
// DateTimeData::m_cstrDefaultDateTimeDelimiter -- デフォルトのデリミター
//
// NOTES

//static
ModUnicodeString
DateTimeData::
m_cstrDefaultDateTimeDelimiter = ModUnicodeString(_pszDefaultDateTimeDelimiter, LiteralCode);

// VARIABLE private
// DateTimeData::m_cstrDefaultTimeDelimiter -- デフォルトのデリミター
//
// NOTES

//static
ModUnicodeString
DateTimeData::
m_cstrDefaultTimeDelimiter = ModUnicodeString(_pszDefaultTimeDelimiter, LiteralCode);

// VARIABLE private
// DateTimeData::m_cstrDefaultMsecDelimiter -- デフォルトのデリミター
//
// NOTES

//static
ModUnicodeString
DateTimeData::
m_cstrDefaultMsecDelimiter = ModUnicodeString(_pszDefaultMsecDelimiter, LiteralCode);

//static
ModSize
DateTimeData::
getArchiveSize()
{
	return DateData::getArchiveSize() + _iDumpSize;
}

// FUNCTION public
//	Common::DateTimeData::hashCode -- ハッシュコードを取り出す
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
DateTimeData::
hashCode() const
{
	if (isNull()) return 0;

	return (m_cDateData.hashCode() << 12)
		+ (static_cast<ModSize>(m_iHour) << 8)
		+ (static_cast<ModSize>(m_iMinute) << 4)
		+ (static_cast<ModSize>(m_iSecond) << 2)
		+ static_cast<ModSize>(m_iMillisecond);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iPrecision_
//		ミリ秒を文字列にする際の桁数(default 0)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DateTimeData::
DateTimeData(int iPrecision_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setPrecision(iPrecision_);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModUnicodeString& cstrCalendar_
//		日時文字列(yyyy-mm-dd hh:mm:ss.mmm)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(const ModUnicodeString& cstrCalendar_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(static_cast<const ModUnicodeChar*>(cstrCalendar_),
			 cstrCalendar_.getTail(), true /* for assign */);
}

// FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(2.5)
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

DateTimeData::
DateTimeData(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(pHead_, pTail_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(3)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModUnicodeString& cstrCalendar_
//		日時文字列
//	const ModUnicodeString& cstrDateDelimiter_
//		年、月、日のデリミタ
//	const ModUnicodeString& cstrDateTimeDelimiter_
//		年月日、時分秒のデリミタ
//	const ModUnicodeString& cstrTimeDelimiter_
//		時、分、秒のデリミタ
//	const ModUnicodeString& cstrMsecDelimiter_
//		秒、ミリ秒のデリミタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(const ModUnicodeString& cstrCalendar_,
			 const ModUnicodeString& cstrDateDelimiter_,
			 const ModUnicodeString& cstrDateTimeDelimiter_,
			 const ModUnicodeString& cstrTimeDelimiter_,
			 const ModUnicodeString& cstrMsecDelimiter_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(static_cast<const ModUnicodeChar*>(cstrCalendar_),
			 cstrCalendar_.getTail(),
			 cstrDateDelimiter_,
			 cstrDateTimeDelimiter_,
			 cstrTimeDelimiter_,
			 cstrMsecDelimiter_,
			 true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(4)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const Common::DateData& cDate_
//		日付データ
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(const DateData& cDate_,
				   int iHour_, int iMinute_, int iSecond_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(cDate_, iHour_, iMinute_, iSecond_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(5)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const Common::DateData& cDate_
//		日付データ
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//	int iMillisecond_
//		ミリ秒
//	int iPrecidion_
//		ミリ秒を文字列にする際の桁数(default 3)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(const DateData& cDate_,
				   int iHour_, int iMinute_, int iSecond_,
				   int iMillisecond_, int iPrecision_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(cDate_, iHour_, iMinute_, iSecond_,
			 iMillisecond_, iPrecision_,
			 true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(6)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iYear_
//		年
//	int iMonth_
//		月
//	int iDate_
//		日
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(int iYear_, int iMonth_, int iDate_,
				   int iHour_, int iMinute_, int iSecond_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(iYear_, iMonth_, iDate_, iHour_, iMinute_, iSecond_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(7)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iYear_
//		年
//	int iMonth_
//		月
//	int iDate_
//		日
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//	int iMillisecond_
//		ミリ秒
//	int iPrecision_
//		ミリ秒を文字列にする際の桁数(default 3)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(int iYear_, int iMonth_, int iDate_,
				   int iHour_, int iMinute_, int iSecond_,
				   int iMillisecond_, int iPrecision_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(iYear_, iMonth_, iDate_, iHour_, iMinute_, iSecond_,
			 iMillisecond_, iPrecision_,
			 true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::DateTimeData -- コンストラクタ(8)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cCalendar_
//		要素6(年、月、日、時、分、秒)または、
//		要素7(年、月、日、時、分、秒、ミリ秒)または、
//		要素8(年、月、日、時、分、秒、ミリ秒、桁数)のintの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateTimeData::
DateTimeData(const IntegerArrayData& cIntegerArrayData_)
: ScalarData(DataType::DateTime),
  m_cstrDateTimeDelimiter(),
  m_cstrTimeDelimiter(),
  m_cstrMsecDelimiter(),
  m_iHour(0), m_iMinute(0), m_iSecond(0), m_iMillisecond(0), m_iPrecision(3)
{
	setValue(cIntegerArrayData_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::~DateTimeData -- デストラクタ
//
//	NOTES
//	デストラクタ
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
DateTimeData::
~DateTimeData()
{
}

//
//	FUNCTION private
//	Common::DateTimeData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し
		cArchiver_(m_cDateData);
		cArchiver_ << m_iHour << m_iMinute << m_iSecond;
		cArchiver_ << m_iMillisecond << m_iPrecision;
	}
	else
	{
		//読み出し
		cArchiver_(m_cDateData);
		cArchiver_ >> m_iHour >> m_iMinute >> m_iSecond;
		cArchiver_ >> m_iMillisecond >> m_iPrecision;
	}
}

//
//	FUNCTION private
//	Common::DateTimeData::copy_NotNull -- コピーする
//
//	NOTES
//	自分自身のコピーを作成する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data::Pointer
//		自分自身のコピー
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Data::Pointer
DateTimeData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new DateTimeData(*this);
}

//
//	FUNCTION private
//	Common::DateTimeData::cast_NotNull -- キャストする
//
//	NOTES
//	指定の型にキャストする。
//
//	ARGUMENTS
//	Common::DataType::Type eType_
//		キャストする型
//
//	RETURN
//	Common::Data::Pointer
//		キャストしたデータ。
//
//	EXCEPTIONS
//	Exception::ClassCast
//		キャストに失敗した
//	その他
//		下位の例外はそのまま再送
//
Data::Pointer
DateTimeData::
cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_)
	{
	case DataType::String:
		return new StringData(getString());
	case DataType::Date:
		return new DateData(getYear(), getMonth(), getDate());
	case DataType::DateTime:
		return copy();
	case DataType::Null:
		return NullData::getInstance();
	default:
		break;
	}
	_TRMEISTER_THROW0(Exception::ClassCast);
}

//
//	FUNCTION public
//	Common::DateTimeData::getValue -- 値を取り出す
//
//	NOTES
//	値を取出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::IntegerArrayData*
//		要素7のintの配列。インスタンスは呼び出し元で解放しなければならない。
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

IntegerArrayData*
DateTimeData::
getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	IntegerArrayData* pIntegerArrayData = m_cDateData.getValue();
	pIntegerArrayData->setElement(3, m_iHour);
	pIntegerArrayData->setElement(4, m_iMinute);
	pIntegerArrayData->setElement(5, m_iSecond);
	pIntegerArrayData->setElement(6, m_iMillisecond);
	return pIntegerArrayData;
}

//
//	FUNCTION public
//	DateTimeData::getYear -- 年を得る
//
//	NOTES
//	年を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		年の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getYear() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_cDateData.getYear();
}

//
//	FUNCTION public
//	Common::DateTimeData::getMonth -- 月を得る
//
//	NOTES
//	月を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		月の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getMonth() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_cDateData.getMonth();
}

//
//	FUNCTION public
//	Common::DateTimeData::getDate -- 日を得る
//
//	NOTES
//	日を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		日の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getDate() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_cDateData.getDate();
}

//
//	FUNCTION public
//	Common::DateTimeData::getHour -- 時を得る
//
//	NOTES
//	時を得る。
//
//	ARGUMENTS
//	なし
//	RETURN
//	int
//		時の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getHour() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iHour;
}

//
//	FUNCTION public
//	Common::DateTimeData::getMinute -- 分を得る
//
//	NOTES
//	分を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		分の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getMinute() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iMinute;
}

//
//	FUNCTION public
//	Common::DateTimeData::getSecond -- 秒を得る
//
//	NOTES
//	秒を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		秒の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getSecond() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iSecond;
}

//
//	FUNCTION public
//	Common::DateTimeData::getMillisecond -- ミリ秒を得る
//
//	NOTES
//	ミリ秒を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ミリ秒の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getMillisecond() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iMillisecond;
}

//
//	FUNCTION public
//	Common::DateTimeData::getPrecision -- ミリ秒を文字列にする際の桁数を得る
//
//	NOTES
//	ミリ秒を文字列にする際の桁数を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ミリ秒を文字列にする際の桁数の値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateTimeData::
getPrecision() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iPrecision;
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(1)
//
//	NOTES
//	文字列の値を設定する
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDate_
//		日時文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const ModUnicodeString& cstrDate_, bool bForAssign_ /* = false */)
{
	setValue(static_cast<const ModUnicodeChar*>(cstrDate_),
			 cstrDate_.getTail(),
			 bForAssign_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(1')
//
//	NOTES
//	文字列の値を設定する
//
//	ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//		日時文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
		 bool bForAssign_ /* = false */)
{
	setValue(pHead_,
			 pTail_,
			 m_cDateData.getDelimiter(),
			 m_cstrDateTimeDelimiter,
			 m_cstrTimeDelimiter,
			 m_cstrMsecDelimiter,
			 bForAssign_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(2)
//
//	NOTES
//	文字列の値を設定する
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDate_
//		日時文字列
//	const ModUnicodeString& cstrDateDelimiter_
//		年、月、秒のデリミタ
//	const ModUnicodeString& cstrDateTimeDelimiter_
//		年月日、時分秒のデリミタ
//	const ModUnicodeString& cstrTimeDelimiter_
//		時、分、秒のデリミタ
//	const ModUnicodeString& cstrMsecDelimiter_
//		秒、ミリ秒のデリミタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const ModUnicodeString& cstrDate_,
		 const ModUnicodeString& cstrDateDelimiter_,
		 const ModUnicodeString& cstrDateTimeDelimiter_,
		 const ModUnicodeString& cstrTimeDelimiter_,
		 const ModUnicodeString& cstrMsecDelimiter_,
		 bool bForAssign_ /* = false */)
{
	setValue(static_cast<const ModUnicodeChar*>(cstrDate_),
			 cstrDate_.getTail(),
			 cstrDateDelimiter_, cstrDateTimeDelimiter_,
			 cstrTimeDelimiter_, cstrMsecDelimiter_,
			 bForAssign_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(2')
//
//	NOTES
//	文字列の値を設定する
//
//	ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//		日時文字列
//	const ModUnicodeString& cstrDateDelimiter_
//		年、月、秒のデリミタ
//	const ModUnicodeString& cstrDateTimeDelimiter_
//		年月日、時分秒のデリミタ
//	const ModUnicodeString& cstrTimeDelimiter_
//		時、分、秒のデリミタ
//	const ModUnicodeString& cstrMsecDelimiter_
//		秒、ミリ秒のデリミタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const ModUnicodeChar* pHead_,
		 const ModUnicodeChar* pTail_,
		 const ModUnicodeString& cstrDateDelimiter_,
		 const ModUnicodeString& cstrDateTimeDelimiter_,
		 const ModUnicodeString& cstrTimeDelimiter_,
		 const ModUnicodeString& cstrMsecDelimiter_,
		 bool bForAssign_ /* = false */)
{
	const ModUnicodeString& cstrDateDelimiter =
		cstrDateDelimiter_.getLength() ? cstrDateDelimiter_ : _cstrDefaultDateDelimiter;
	const ModUnicodeString& cstrDateTimeDelimiter =
		cstrDateTimeDelimiter_.getLength() ? cstrDateTimeDelimiter_ : m_cstrDefaultDateTimeDelimiter;
	const ModUnicodeString& cstrTimeDelimiter =
		cstrTimeDelimiter_.getLength() ? cstrTimeDelimiter_ : m_cstrDefaultTimeDelimiter;
	const ModUnicodeString& cstrMsecDelimiter =
		cstrMsecDelimiter_.getLength() ? cstrMsecDelimiter_ : m_cstrDefaultMsecDelimiter;

	_DateTimeData::setValue<ModUnicodeChar, ModUnicodeCharTrait, ModUnicodeString>(
						   *this,
						   pHead_, pTail_, bForAssign_,
						   static_cast<const ModUnicodeChar*>(cstrDateDelimiter),
						   cstrDateDelimiter.getLength(),
						   static_cast<const ModUnicodeChar*>(cstrDateTimeDelimiter),
						   cstrDateTimeDelimiter.getLength(),
						   static_cast<const ModUnicodeChar*>(cstrTimeDelimiter),
						   cstrTimeDelimiter.getLength(),
						   static_cast<const ModUnicodeChar*>(cstrMsecDelimiter),
						   cstrMsecDelimiter.getLength());
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(3)
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	const Common::DateData& cDate_
//		日付データ
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const DateData& cDate_,
		 int iHour_, int iMinute_, int iSecond_,
		 bool bForAssign_ /* = false */)
{
	setValue(cDate_, iHour_, iMinute_, iSecond_, 0, 0, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(4)
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	const Common::DateData& cDate_
//		日付データ
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//	int iMillisecond_
//		ミリ秒
//	int iPrecision_
//		ミリ秒を文字列にする際の桁数(default 3)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const DateData& cDate_,
		 int iHour_, int iMinute_, int iSecond_,
		 int iMillisecond_, int iPrecision_,
		 bool bForAssign_ /* = false */)
{
	if (!cDate_.isNull()) {
		setTime(iHour_, iMinute_, iSecond_, iMillisecond_, iPrecision_, bForAssign_);
		m_cDateData = cDate_;
	}
	else
		setNull();
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(5)
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	int iYear_
//		年
//	int iMonth_
//		月
//	int iDate_
//		日
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(int iYear_, int iMonth_, int iDate_,
		 int iHour_, int iMinute_, int iSecond_,
		 bool bForAssign_ /* = false */)
{
	setValue(iYear_, iMonth_, iDate_, iHour_, iMinute_, iSecond_, 0, 0, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(6)
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	int iYear_
//		年
//	int iMonth_
//		月
//	int iDate_
//		日
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//	int iMillisecond_
//		ミリ秒
//	int iPrecision_
//		ミリ秒を文字列にする際の桁数(default 3)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(int iYear_, int iMonth_, int iDate_,
		 int iHour_, int iMinute_, int iSecond_,
		 int iMillisecond_, int iPrecision_,
		 bool bForAssign_ /* = false */)
{
	m_cDateData.setValue(iYear_, iMonth_, iDate_, bForAssign_);
	if (!m_cDateData.isNull())
		setTime(iHour_, iMinute_, iSecond_, iMillisecond_, iPrecision_, bForAssign_);
	else
		setNull();
}

//
//	FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(7)
//
//	NOTES
//	値を設定する
//
//	ARGUMENTS
//	const CommonIntegerArrayData& cIntegerArrayData_
//		要素6(年、月、日、時、分、秒)または、
//		要素7(年、月、日、時、分、秒、ミリ秒)または、
//		要素8(年、月、日、時、分、秒、ミリ秒、桁数)のintの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::InvalidDatetimeformat
//		引数の配列の要素数が不正である
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setValue(const IntegerArrayData& cIntegerArrayData_, bool bForAssign_ /* = false */)
{
	int	numElement = cIntegerArrayData_.getCount();

	if (numElement < 6)
	{
		if (bForAssign_)
			_TRMEISTER_THROW0(Exception::InvalidDatetimeFormat);
		else {
			setNull();
			return;
		}
	}

	m_cDateData.setValue(cIntegerArrayData_.getElement(0),
						 cIntegerArrayData_.getElement(1),
						 cIntegerArrayData_.getElement(2),
						 bForAssign_);
	setHour(cIntegerArrayData_.getElement(3));
	setMinute(cIntegerArrayData_.getElement(4));
	setSecond(cIntegerArrayData_.getElement(5));
	if (numElement > 6)
	{
		setMillisecond(cIntegerArrayData_.getElement(6));
	}
	if (numElement > 7)
	{
		setPrecision(cIntegerArrayData_.getElement(7));
	}
}

// FUNCTION public
//	Common::DateTimeData::setValue -- 値を設定する(8) -- char*版
//
// NOTES
//
// ARGUMENTS
//	const char* pHead_
//	const char* pTail_
//	const char* pDateDelimiter_ = 0
//	const char* pDateTimeDelimiter_ = 0
//	const char* pTimeDelimiter_ = 0
//	const char* pMsecDelimiter_ = 0
//	bool bForAssign_ = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
DateTimeData::
setValue(const char* pHead_, const char* pTail_,
		 const char* pDateDelimiter_ /* = 0 */,
		 const char* pDateTimeDelimiter_ /* = 0 */,
		 const char* pTimeDelimiter_ /* = 0 */,
		 const char* pMsecDelimiter_ /* = 0 */,
		 bool bForAssign_ /* = false */)
{
	const char* pDateDelimiter = pDateDelimiter_ ? pDateDelimiter_ : _pszDefaultDateDelimiter;
	const char* pDateTimeDelimiter = pDateTimeDelimiter_ ? pDateTimeDelimiter_ : _pszDefaultDateTimeDelimiter;
	const char* pTimeDelimiter = pTimeDelimiter_ ? pTimeDelimiter_ : _pszDefaultTimeDelimiter;
	const char* pMsecDelimiter = pMsecDelimiter_ ? pMsecDelimiter_ : _pszDefaultMsecDelimiter;

	_DateTimeData::setValue<char, ModCharTrait, const char>(*this,
													  pHead_, pTail_, bForAssign_,
													  *pDateDelimiter, 1,
													  *pDateTimeDelimiter, 1,
													  *pTimeDelimiter, 1,
													  *pMsecDelimiter, 1);
}

//	FUNCTION public
//	Common::DateTimeData::setCurrent -- 値を現在時刻に設定する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DateTimeData::
setCurrent()
{
	const ModTime&	cTime = ModTime::getCurrentTime();

	setValue(cTime.getYear(), cTime.getMonth(), cTime.getDay(),
			 cTime.getHour(), cTime.getMinute(), cTime.getSecond(),
			 cTime.getMilliSecond(), m_iPrecision, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateTimeData::setYear -- 年を設定する
//
//	NOTES
//	年を設定する
//
//	ARGUMENTS
//	int iYear_
//		年
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setYear(int iYear_)
{
	m_cDateData.setYear(iYear_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setMonth -- 月を設定する
//
//	NOTES
//	月を設定する
//
//	ARGUMENTS
//	int iMonth_
//		月
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setMonth(int iMonth_)
{
	m_cDateData.setMonth(iMonth_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setDate -- 日を設定する
//
//	NOTES
//	日を設定する
//
//	ARGUMENTS
//	int iDate_
//		日
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setDate(int iDate_)
{
	m_cDateData.setDate(iDate_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setHour -- 時を設定する
//
//	NOTES
//	時を設定する
//
//	ARGUMENTS
//	int iHour_
//		時
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setHour(int iHour_)
{
	setTime(iHour_, m_iMinute, m_iSecond, m_iMillisecond, m_iPrecision);
}

//
//	FUNCTION public
//	Common::DateTimeData::setMinute -- 分を設定する
//
//	NOTES
//	分を設定する
//
//	ARGUMENTS
//	int iMinute_
//		分
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setMinute(int iMinute_)
{
	setTime(m_iHour, iMinute_, m_iSecond, m_iMillisecond, m_iPrecision);
}

//
//	FUNCTION public
//	Common::DateTimeData::setSecond -- 秒を設定する
//
//	NOTES
//	秒を設定する
//
//	ARGUMENTS
//	int iSecond_
//		秒
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setSecond(int iSecond_)
{
	setTime(m_iHour, m_iMinute, iSecond_, m_iMillisecond, m_iPrecision);
}

//
//	FUNCTION public
//	Common::DateTimeData::setMillisecond -- ミリ秒を設定する
//
//	NOTES
//	ミリ秒を設定する
//
//	ARGUMENTS
//	int iMillisecond_
//		ミリ秒
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateTimeData::
setMillisecond(int iMillisecond_)
{
	setTime(m_iHour, m_iMinute, m_iSecond, iMillisecond_, m_iPrecision);
}

//
//	FUNCTION public
//	Common::DateTimeData::setPrecision -- ミリ秒を文字列にする際の桁数を設定する
//
//	NOTES
//	ミリ秒を文字列にする際の桁数を設定する
//
//	ARGUMENTS
//	int iPrecision_
//		桁数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void
DateTimeData::
setPrecision(int iPrecision_)
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	switch (iPrecision_)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		m_iPrecision = iPrecision_;
		break;
	default:
		m_iPrecision = 0;
	}
}

//
//	FUNCTION private
//	Common::DateTimeData::getString_NotNull -- 文字列でデータを得る
//
//	NOTES
//	文字列でデータを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	文字列
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
DateTimeData::
getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeOstrStream costrStr;
	
	// 文字列整形
	costrStr << m_cDateData.getString()
			 << (m_cstrDateTimeDelimiter.getLength() ? m_cstrDateTimeDelimiter : m_cstrDefaultDateTimeDelimiter)
			 << ModIosSetW(2) << ModIosSetFill('0')
			 << m_iHour
			 << (m_cstrTimeDelimiter.getLength() ? m_cstrTimeDelimiter : m_cstrDefaultTimeDelimiter)
			 << ModIosSetW(2) << ModIosSetFill('0')
			 << m_iMinute
			 << (m_cstrTimeDelimiter.getLength() ? m_cstrTimeDelimiter : m_cstrDefaultTimeDelimiter)
			 << ModIosSetW(2) << ModIosSetFill('0')
			 << m_iSecond;

	if (m_iPrecision > 0)
	{
		// 数値排出用ストリーム設定
		costrStr << (m_cstrMsecDelimiter.getLength() ? m_cstrMsecDelimiter : m_cstrDefaultMsecDelimiter)
				 << ModIosSetW(m_iPrecision) << ModIosSetFill('0')
				 << m_iMillisecond;
	}

	return costrStr.getString();
}

// FUNCTION public
//	Common::DateTimeData::setDateDelimiter -- 年、月、日を区切るデリミタを設定する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrDelimiter_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
DateTimeData::
setDateDelimiter(const ModUnicodeString& cstrDelimiter_)
{
	m_cDateData.setDelimiter(cstrDelimiter_);
}

//
//	FUNCTION public
//	Common::DateTimeData::setDateTimeDelimiter
//							-- 年月日と時分秒を区切るデリミタを設定する
//
//	NOTES
//	年月日と時分秒を区切るデリミタを設定する
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDelimiter_
//		デリミタ文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void
DateTimeData::
setDateTimeDelimiter(const ModUnicodeString& cstrDelimiter_)
{
	m_cstrDateTimeDelimiter = cstrDelimiter_;
}

//
//	FUNCTION public
//	Common::DateTimeData::setTimeDelimiter
//							-- 時、分、秒を区切るデリミタを設定する
//
//	NOTES
//	時、分、秒を区切るデリミタを設定する
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDelimiter_
//		デリミタ文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void
DateTimeData::
setTimeDelimiter(const ModUnicodeString& cstrDelimiter_)
{
	m_cstrTimeDelimiter = cstrDelimiter_;
}

//
//	FUNCTION public
//	Common::DateTimeData::setMsecDelimiter
//							-- 秒、ミリ秒を区切るデリミタを設定する
//
//	NOTES
//	秒、ミリ秒を区切るデリミタを設定する
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDelimiter_
//		デリミタ文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void
DateTimeData::
setMsecDelimiter(const ModUnicodeString& cstrDelimiter_)
{
	m_cstrMsecDelimiter = cstrDelimiter_;
}

//	FUNCTION private
//	Common::DateTimeData::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//		比較するデータは自分自身と同じ型である必要がある
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		0
//			左辺と右辺は等しい
//		-1
//			左辺のほうが小さい
//		1
//			右辺のほうが小さい
//
//	EXCEPTIONS
//		なし

int
DateTimeData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::DateTime);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DateTimeData& data = _SYDNEY_DYNAMIC_CAST(const DateTimeData&, r);

	return (getYear() < data.getYear()) ? -1 :
		(getYear() > data.getYear()) ? 1 :
		(getMonth() < data.getMonth()) ? -1 :
		(getMonth() > data.getMonth()) ? 1 :
		(getDate() < data.getDate()) ? -1 :
		(getDate() > data.getDate()) ? 1 :
		(getHour() < data.getHour()) ? -1 :
		(getHour() > data.getHour()) ? 1 :
		(getMinute() < data.getMinute()) ? -1 :
		(getMinute() > data.getMinute()) ? 1 :
		(getSecond() < data.getSecond()) ? -1 :
		(getSecond() > data.getSecond()) ? 1 :
		(getMillisecond() == data.getMillisecond()) ? 0 :
		(getMillisecond() < data.getMillisecond()) ? -1 : 1;
}

//
//	FUNCTION private
//	Common::DateTimeData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::DateTimeDataクラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
DateTimeData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::DateTimeDataClass;
}

//
//	FUNCTION private
//	Common::DateTimeData::print_NotNull -- 値を表示する
//
//	NOTES
//	値を表示する
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
void
DateTimeData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeString str(getString());
	cout << "time: " << str.getString(LiteralCode) << endl;
}

//
//	FUNCTION private
//	Common::DateTimeData::setTime -- 時間のみ設定する
//
//	NOTES
//	時間のみ設定する
//
//	ARGUMENTS
//	int iHour_
//		時
//	int iMinute_
//		分
//	int iSecond_
//		秒
//	int iMillisecond_
//		ミリ秒
//	int iPrecision_
//		ミリ秒を文字列にする際の桁数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::InvalidDatetimeFormat
//		引数の時間が不正である
//
void
DateTimeData::
setTime(int iHour_, int iMinute_, int iSecond_,
		int iMillisecond_, int iPrecision_,
		bool bForAssign_ /* = false */)
{
	if (_DateTimeData::checkTime(iHour_, iMinute_, iSecond_, iMillisecond_)) {
		setNull(false);
		setPrecision(iPrecision_);
		m_iHour = iHour_;
		m_iMinute = iMinute_;
		m_iSecond = iSecond_;
		m_iMillisecond = iMillisecond_;
	}
	else if (bForAssign_)
		_TRMEISTER_THROW0(Exception::InvalidDatetimeFormat);
	else
		setNull();
}

// FUNCTION public
//	Common::DateTimeData::assign_NoCast -- 代入を行う(キャストなし)
//
// NOTES
//
// ARGUMENTS
//	const Data& r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
DateTimeData::
assign_NoCast(const Data& r)
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (r.isNull()) {
		setNull();
		return false;
	}
#endif

	; _TRMEISTER_ASSERT(r.getType() == getType());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DateTimeData& data = _SYDNEY_DYNAMIC_CAST(const DateTimeData&, r);
	setValue(data.getYear(), data.getMonth(), data.getDate(),
			 data.getHour(), data.getMinute(), data.getSecond(),
			 data.getMillisecond(), data.getPrecision());
	setNull(false);
	return true;
}

//	FUNCTION private
//	Common::DateTimeData::isAbleToDump_NotNull --
//		dumpできるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
bool
DateTimeData::
isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::DateTimeData::isFixedSize_NotNull --
//		常に固定長であるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
bool
DateTimeData::
isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::DateTimeData::getDumpSize_NotNull --
//		ダンプサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//
//	EXCEPTIONS

//virtual
ModSize
DateTimeData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize + DateData::getArchiveSize();
}

//	FUNCTION public
//	Common::DateTimeData::setDumpedValue --
//		dumpされたデータから自身の値をsetする
//
//	NOTES
//
//	ARGUMENTS
//		const char* pszValue_
//			dumpされた領域の先頭
//		ModSize uSize_(省略可)
//			指定した場合正しい値かを検証する
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
DateTimeData::
setDumpedValue(const char* pszValue_)
{
	ModSize iDumpSize = m_cDateData.setDumpedValue(pszValue_);
	unsigned int uiValue;
	Os::Memory::copy(&uiValue, pszValue_ + iDumpSize, _iDumpSize);
	m_iMillisecond = uiValue & 0x00007FFF;
	uiValue >>= 15;
	m_iSecond = uiValue & 0x0000003F;
	uiValue >>= 6;
	m_iMinute = uiValue & 0x0000003F;
	uiValue >>= 6;
	m_iHour = uiValue;

	setNull(false);

	return _iDumpSize + iDumpSize;
}

//virtual
ModSize
DateTimeData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != getDumpSize()) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(pszValue_);
}

ModSize
DateTimeData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != getDumpSize()) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	ModSize iDumpSize = m_cDateData.setDumpedValue(cSerialIO_);
	unsigned int uiValue;
	cSerialIO_.readSerial(&uiValue, _iDumpSize, ModSerialIO::dataTypeVariable);

	m_iMillisecond = uiValue & 0x00007FFF;
	uiValue >>= 15;
	m_iSecond = uiValue & 0x0000003F;
	uiValue >>= 6;
	m_iMinute = uiValue & 0x0000003F;
	uiValue >>= 6;
	m_iHour = uiValue;

	setNull(false);

	return _iDumpSize + iDumpSize;
}

ModSize
DateTimeData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModSize iDumpSize = m_cDateData.dumpValue(cSerialIO_);
	unsigned int uiValue = m_iHour;
	uiValue <<= 6;
	uiValue |= (m_iMinute & 0x0000003F);
	uiValue <<= 6;
	uiValue |= (m_iSecond & 0x0000003F);
	uiValue <<= 15;
	uiValue |= (m_iMillisecond & 0x00007FFF);
	cSerialIO_.writeSerial(&uiValue, _iDumpSize, ModSerialIO::dataTypeVariable);
	return _iDumpSize + iDumpSize;
}

//	FUNCTION private
//	Common::DateTimeData::dumpValue_NotNull --
//		自身の値をdumpする
//
//	NOTES
//
//	ARGUMENTS
//		char* pszResult_
//			dumpする領域の先頭
//
//	RETURN
//		値に使ったバイト数
//
//	EXCEPTIONS

//virtual
ModSize
DateTimeData::
dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModSize iDumpSize = m_cDateData.dumpValue(pszResult_);
	unsigned int uiValue = m_iHour;
	uiValue <<= 6;
	uiValue |= (m_iMinute & 0x0000003F);
	uiValue <<= 6;
	uiValue |= (m_iSecond & 0x0000003F);
	uiValue <<= 15;
	uiValue |= (m_iMillisecond & 0x00007FFF);
	Os::Memory::copy(pszResult_ + iDumpSize, &uiValue, _iDumpSize);
	return _iDumpSize + iDumpSize;
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
