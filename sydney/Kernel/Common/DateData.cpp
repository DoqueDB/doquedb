// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DateData.cpp -- 日付型データ関連の関数定義
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
#include "Common/DateData.h"
#include "Common/IntegerArrayData.h"
#include "Common/NullData.h"
#include "Common/StringData.h"
#include "Common/DateTimeData.h"
#include "Common/UnicodeString.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/InvalidDatetimeFormat.h"
#include "Os/Memory.h"

#include "ModCharTrait.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeCharTrait.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
//
//	CONST local
//	$$::_iDumpSize -- ダンプサイズ
//
//	NOTES
//	Dateはダンプするときはintにする
//
const ModSize _iDumpSize = sizeof(int);

//
//	CONST local
//	$$::_pszDefaultDemimiter -- デフォルトデリミタ
//
//	NOTES
//	年、月、日を切り出すデフォルトのデリミタ
//
const char* const _pszDefaultDelimiter = "-";

//
//	CONST local
//	$$::_iDayTable -- 月の最後の日
//
//	NOTES
//	その月の最終の日付
//
const int _iDayTable[2][13] = {
	//  1   2   3   4   5   6   7   8   9   10  11  12
	{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
	{0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	//うるう年
};

namespace _DateData
{
	//日付が正しいかチェックする
	bool checkCalendar(int iYear_, int iMonth_, int iDate_);
}

//	FUNCTION
//	$$$::_DateData::checkCalendar -- 日付が正しいかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int iYear_
//		年
//	int iMonth_
//		月
//	int iDate_
//		日
//
//	RETURN
//	正しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし

inline
bool
_DateData::checkCalendar(int iYear_, int iMonth_, int iDate_)
{
	bool bResult = false;
	// 年は23bitで表せる値でなければならない
	if (iYear_ <= 4194303 && iYear_ >= -4194304) {
		if (iYear_ >= 1)
		{
			if (iMonth_ >= 1 && iMonth_ <= 12)
			{
				//うるう年の計算
				int n = (iYear_%4 == 0 && iYear_%100 != 0 || iYear_%400 == 0);
				if (iDate_ >= 1 && iDate_ <= _iDayTable[n][iMonth_])
				{
					bResult = true;
				}
			}
		}
	}
	return bResult;
}

}

// VARIABLE
// DateData::m_cstrDefaultDelimiter -- デフォルトのデリミター
//
// NOTES

//static
ModUnicodeString
DateData::
m_cstrDefaultDelimiter = ModUnicodeString(_pszDefaultDelimiter, LiteralCode);

//static
ModSize
DateData::
getArchiveSize()
{
	return _iDumpSize;
}

// FUNCTION public
//	Common::DateData::hashCode -- ハッシュコードを取り出す
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
DateData::
hashCode() const
{
	if (isNull()) return 0;

	return (static_cast<ModSize>(m_iYear) << 4)
		+ (static_cast<ModSize>(m_iMonth) << 2)
		+ static_cast<ModSize>(m_iDate);
}

//
//	FUNCTION public
//	Common::DateData::DateData -- コンストラクタ(1)
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
DateData::
DateData()
: ScalarData(DataType::Date),
  m_cstrDelimiter(),
  m_iYear(0), m_iMonth(0), m_iDate(0)
{
}

//
//	FUNCTION public
//	Common::DateData::DateData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModUnicodeString& cstrCalendar_
//		日付文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateData::
DateData(const ModUnicodeString& cstrCalendar_)
	: ScalarData(DataType::Date),
	  m_cstrDelimiter()
{
	setValue(static_cast<const ModUnicodeChar*>(cstrCalendar_),
			 cstrCalendar_.getTail(), true /* for assign */);
}

// FUNCTION public
//	Common::DateData::DateData -- コンストラクタ(2.5)
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

DateData::
DateData(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
	: ScalarData(DataType::Date),
	  m_cstrDelimiter()
{
	setValue(pHead_, pTail_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateData::DateData -- コンストラクタ(3)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModUnicodeString& cstrCalendar_
//		日付文字列
//	const ModUnicodeString& cstrDelimiter_
//		デリミタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateData::
DateData(const ModUnicodeString& cstrCalendar_,
						   const ModUnicodeString& cstrDelimiter_)
: ScalarData(DataType::Date),
  m_cstrDelimiter()
{
	setValue(static_cast<const ModUnicodeChar*>(cstrCalendar_),
			 cstrCalendar_.getTail(), cstrDelimiter_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateData::DateData -- コンストラクタ(4)
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
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateData::
DateData(int iYear_, int iMonth_, int iDate_)
: ScalarData(DataType::Date),
  m_cstrDelimiter()
{
	setValue(iYear_, iMonth_, iDate_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateData::DateData -- コンストラクタ(5)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const DBIntegerArrayData& cCalendar_
//		要素3(年、月、日)のint配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DateData::
DateData(const IntegerArrayData& cCalendar_)
: ScalarData(DataType::Date),
  m_cstrDelimiter()
{
	setValue(cCalendar_, true /* for assign */);
}

//
//	FUNCTION public
//	Common::DateData::~DateData -- デストラクタ
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
DateData::
~DateData()
{
}

//
//	FUNCTION private
//	Common::DateData::serialize_NotNull -- シリアル化
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
DateData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
		//書き出し
		cArchiver_ << m_iYear << m_iMonth << m_iDate;
	else
		//読み出し
		cArchiver_ >> m_iYear >> m_iMonth >> m_iDate;
}

//
//	FUNCTION private
//	Common::DateData::copy_NotNull -- コピーする
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

Data::Pointer
DateData::
copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new DateData(*this);
}

//
//	FUNCTION private
//	Common::DateData::cast_NotNull -- キャストする
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
DateData::
cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (eType_)
	{
	case DataType::String:
		return new StringData(getString());
	case DataType::Date:
		return copy();
	case DataType::DateTime:
		return new DateTimeData(getYear(), getMonth(), getDate(), 0, 0, 0);
	case DataType::Null:
		return NullData::getInstance();
	default:
		break;
	}
	_TRMEISTER_THROW0(Exception::ClassCast);
}

//
//	FUNCTION public
//	Common::DateData::getValue -- 値を得る
//
//	NOTES
//	年、月、日の値を配列で得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::IntegerArrayData*
//		intの配列。インスタンスは呼び出し元で解放しなければならない。
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

IntegerArrayData*
DateData::
getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	IntegerArrayData* pIntegerArrayData = new IntegerArrayData;
	pIntegerArrayData->setElement(0, m_iYear);
	pIntegerArrayData->setElement(1, m_iMonth);
	pIntegerArrayData->setElement(2, m_iDate);
	return pIntegerArrayData;
}

//
//	FUNCTION public
//	Common::DateData::getYear -- 年を得る
//
//	NOTES
//	年を得る
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
DateData::
getYear() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iYear;
}

//
//	FUNCTION public
//	Common::DateData::getMonth -- 月を得る
//
//	NOTES
//	月を得る
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
DateData::
getMonth() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iMonth;
}

//
//	FUNCTION public
//	Common::DateData::getDate -- 日を得る
//
//	NOTES
//	日を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		日を得る
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

int
DateData::
getDate() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_iDate;
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(1)
//
//	NOTES
//	文字列で日付を設定する。
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDate_
//		日付の文字列(yyyy-mm-dd)
//	bool bForAssign_ = false
//		true ... 形式が不正なとき例外を投げる
//		false .. 形式が不正なときNULLにする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateData::
setValue(const ModUnicodeString& cstrDate_, bool bForAssign_ /* = false */)
{
	setValue(static_cast<const ModUnicodeChar*>(cstrDate_), cstrDate_.getTail(), bForAssign_);
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(1')
//
//	NOTES
//	文字列で日付を設定する。
//
//	ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//		日付の文字列(yyyy-mm-dd)
//	bool bForAssign_ = false
//		true ... 形式が不正なとき例外を投げる
//		false .. 形式が不正なときNULLにする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateData::
setValue(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, bool bForAssign_ /* = false */)
{
	setValue(pHead_, pTail_, m_cstrDelimiter, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(2)
//
//	NOTES
//	文字列で値を設定する。
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDate_
//		日付の文字列(yyyy?mm?dd)
//	const ModUnicodeString& cstrDelimiter_
//		デリミタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateData::
setValue(const ModUnicodeString& cstrDate_,
		 const ModUnicodeString& cstrDelimiter_,
		 bool bForAssign_ /* = false */)
{
	setValue(static_cast<const ModUnicodeChar*>(cstrDate_), cstrDate_.getTail(),
			 cstrDelimiter_, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(2')
//
//	NOTES
//	文字列で値を設定する。
//
//	ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//		日付の文字列(yyyy?mm?dd)
//	const ModUnicodeString& cstrDelimiter_
//		デリミタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateData::
setValue(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_, 
		 const ModUnicodeString& cstrDelimiter_,
		 bool bForAssign_ /* = false */)
{
	int iYear = 0;
	int iMonth = 0;
	int iDate = 0;
	const ModUnicodeChar* p = pHead_;
	ModSize n = static_cast<ModSize>(pTail_ - pHead_);
	iYear = ModUnicodeCharTrait::toInt(p);

	const ModUnicodeString& cstrDelimiter =
		cstrDelimiter_.getLength() ? cstrDelimiter_
		: m_cstrDefaultDelimiter;

	p = ModUnicodeCharTrait::find(p, cstrDelimiter, n);
	int iLength = static_cast<int>(cstrDelimiter.getLength());
	if (p)
	{
		p += iLength;
		iMonth = ModUnicodeCharTrait::toInt(p);

		p = ModUnicodeCharTrait::find(p, cstrDelimiter, n);
		if (p)
		{
			p += iLength;
			iDate = ModUnicodeCharTrait::toInt(p);
		}
	}
	setValue(iYear, iMonth, iDate, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(3)
//
//	NOTES
//	年、月、日をそれぞれ設定する。
//
//	ARGUMENTS
//	int iYear_
//		年
//	int iMonth_
//		月
//	int iDate_
//		日
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	Exception::InvalidDatetimeFormat
//		引数の日付が不正である
//
void
DateData::
setValue(int iYear_, int iMonth_, int iDate_, bool bForAssign_ /* = false */)
{
	if (_DateData::checkCalendar(iYear_, iMonth_, iDate_)) {
		m_iYear = iYear_;
		m_iMonth = iMonth_;
		m_iDate = iDate_;

		setNull(false);
	}
	else if (bForAssign_)
		_TRMEISTER_THROW0(Exception::InvalidDatetimeFormat);
	else
		setNull();
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(4)
//
//	NOTES
//	intの配列で日付を設定する
//
//	ARGUMENTS
//	const Common::IntegerArrayData& cCalendar_
//		intの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateData::
setValue(const IntegerArrayData& cCalendar_, bool bForAssign_ /* = false */)
{
	int iYear = 0;
	int iMonth = 0;
	int iDate = 0;
	if (cCalendar_.getCount() >= 3)
	{
		iYear = cCalendar_.getElement(0);
		iMonth = cCalendar_.getElement(1);
		iDate = cCalendar_.getElement(2);
	}
	setValue(iYear, iMonth, iDate, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateData::setValue -- 値を設定する(7) -- char*版
//
//	NOTES
//	文字列で日付を設定する。
//
//	ARGUMENTS
//	const char* pHead_
//	const char* pTail_
//		日付の文字列(yyyy-mm-dd)
//	char cDelimiter_
//		日付の年月日を区切る文字へのポインター
//	bool bForAssign_ = false
//		true ... 形式が不正なとき例外を投げる
//		false .. 形式が不正なときNULLにする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DateData::
setValue(const char* pHead_, const char* pTail_,
		 char cDelimiter_,
		 bool bForAssign_ /* = false */)
{
	int iYear = 0;
	int iMonth = 0;
	int iDate = 0;

	const char* p = pHead_;
	const char* q = (p < pTail_ )
		? ModCharTrait::find(p, cDelimiter_, static_cast<ModSize>(pTail_ - p))
		: 0;

	if ((p < q) && StringData::getInteger(iYear, p, q)) {

		p = q + 1;
		q = (p < pTail_) ?
			ModCharTrait::find(p, cDelimiter_, static_cast<ModSize>(pTail_ - p))
			: 0;

		if ((p < q) && StringData::getInteger(iMonth, p, q)) {

			p = q + 1;
			q = pTail_;

			if ((p < q) && StringData::getInteger(iDate, p, q)) {
				;
			}
		}
	}
	setValue(iYear, iMonth, iDate, bForAssign_);
}

//
//	FUNCTION public
//	Common::DateData::setYear -- 年を設定する
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
DateData::
setYear(int iYear_)
{
	setValue(iYear_, m_iMonth, m_iDate);
}

//
//	FUNCTION public
//	Common::DateData::setMonth -- 月を設定する
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
DateData::
setMonth(int iMonth_)
{
	setValue(m_iYear, iMonth_, m_iDate);
}

//
//	FUNCTION public
//	Common::DateData::setDate -- 日を設定する
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
DateData::
setDate(int iDate_)
{
	setValue(m_iYear, m_iMonth, iDate_);
}

//
//	FUNCTION private
//	Common::DateDate::getString_NotNull -- 文字列で値を取出す
//
//	NOTES
//	文字列で値を取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		日付の文字列
//
//	EXCEPTIONS

ModUnicodeString
DateData::
getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	const ModUnicodeString& cstrDelimiter = getDelimiter();

	// 年の出力
	ModUnicodeOstrStream cstrCalendar;

	cstrCalendar << ModIosSetW(4) << ModIosSetFill('0')
				 << m_iYear << cstrDelimiter
				 << ModIosSetW(2) << ModIosSetFill('0')
				 << m_iMonth << cstrDelimiter
				 << ModIosSetW(2) << ModIosSetFill('0')
				 << m_iDate;

	return cstrCalendar.getString();
}

//
//	FUNCTION public
//	Common::DateData::getDelimiter -- デリミタを取出す
//
//	NOTES
//	デリミタを取出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		デリミタ文字列
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

const ModUnicodeString&
DateData::
getDelimiter() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_cstrDelimiter.getLength() ? m_cstrDelimiter : m_cstrDefaultDelimiter;
}

//
//	FUNCTION public
//	Common::DateData::setDelimiter -- デリミタを設定する
//
//	NOTES
//	デリミタを設定する
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
DateData::
setDelimiter(const ModUnicodeString& cstrDelimiter_)
{
	m_cstrDelimiter = cstrDelimiter_;
}

//	FUNCTION private
//	Common::DateData::compareTo_NoCast -- 大小比較を行う
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
DateData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::Date);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	const DateData& data = _SYDNEY_DYNAMIC_CAST(const DateData&, r);

	return (getYear() < data.getYear()) ? -1 :
		(getYear() > data.getYear()) ? 1 :
		(getMonth() < data.getMonth()) ? -1 :
		(getMonth() > data.getMonth()) ? 1 :
		(getDate() == data.getDate()) ? 0 :
		(getDate() < data.getDate()) ? -1 : 1;
}

//
//	FUNCTION private
//	Common::DateData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DateDataクラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
DateData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::DateDataClass;
}

//
//	FUNCTION private
//	Common::DateData::print_NotNull -- 値を表示する
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
DateData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModUnicodeString cstrTmp(getString());
	cout << "date: " << cstrTmp.getString(Common::LiteralCode) << endl;
}

// FUNCTION public
//	Common::DateData::assign_NoCast -- 代入を行う(キャストなし)
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
DateData::
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

	const DateData& data = _SYDNEY_DYNAMIC_CAST(const DateData&, r);
	m_iYear = data.m_iYear;
	m_iMonth = data.m_iMonth;
	m_iDate = data.m_iDate;
	m_cstrDelimiter = data.m_cstrDelimiter;
	setNull(false);
	return true;
}

//	FUNCTION private
//	Common::DateData::isAbleToDump_NotNull --
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
DateData::
isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::DateData::isFixedSize_NotNull --
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
DateData::
isFixedSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION private
//	Common::DateData::getDumpSize_NotNull --
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
DateData::
getDumpSize_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return _iDumpSize;
}

//	FUNCTION public
//	Common::DateData::setDumpedValue --
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
DateData::
setDumpedValue(const char* pszValue_)
{
	int iValue;
	Os::Memory::copy(&iValue, pszValue_, _iDumpSize);
	m_iDate = iValue & 0x0000001F;
	iValue >>= 5;
	m_iMonth = iValue & 0x0000000F;
	iValue >>= 4;
	m_iYear = iValue;

	setNull(false);

	return _iDumpSize;
}

//virtual
ModSize
DateData::
setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}
	return setDumpedValue(pszValue_);
}

ModSize
DateData::
setDumpedValue(ModSerialIO& cSerialIO_)
{
	int iValue = 0;
	cSerialIO_.readSerial(&iValue, _iDumpSize, ModSerialIO::dataTypeVariable);
	m_iDate = iValue & 0x0000001F;
	iValue >>= 5;
	m_iMonth = iValue & 0x0000000F;
	iValue >>= 4;
	m_iYear = iValue;

	setNull(false);

	return _iDumpSize;
}

ModSize
DateData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	if (uSize_ != _iDumpSize) {
		_TRMEISTER_THROW0(Exception::BadArgument);
	}

	return setDumpedValue(cSerialIO_);
}

ModSize
DateData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	int iValue = m_iYear;
	iValue <<= 4;
	iValue |= (m_iMonth & 0x0000000F);
	iValue <<= 5;
	iValue |= (m_iDate & 0x0000001F);
	cSerialIO_.writeSerial(&iValue, _iDumpSize, ModSerialIO::dataTypeVariable);

	return _iDumpSize;
}

//	FUNCTION private
//	Common::DateData::dumpValue_NotNull --
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
DateData::dumpValue_NotNull(char* pszResult_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	int iValue = m_iYear;
	iValue <<= 4;
	iValue |= (m_iMonth & 0x0000000F);
	iValue <<= 5;
	iValue |= (m_iDate & 0x0000001F);
	(void) Os::Memory::copy(pszResult_, &iValue, _iDumpSize);

	return _iDumpSize;
}

//
//	Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
