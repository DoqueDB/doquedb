// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StringData.cpp -- 文字列型データ関連の関数定義
// 
// Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "Common/BasicString.h"
#include "Common/BinaryData.h"
#include "Common/ClassID.h"
#include "Common/CompressedData.h"
#include "Common/CompressedStringData.h"
#ifdef OBSOLETE
#include "Common/CompressedStreamStringData.h"
#endif
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/FloatData.h"
#include "Common/Integer64Data.h"
#include "Common/IntegerData.h"
#include "Common/LanguageData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/StringData.h"
#include "Common/SQLData.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/UnsignedIntegerData.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/InvalidCharacter.h"
#include "Exception/NotSupported.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/NumericValueOutOfRange.h"
#include "Exception/MemoryExhaust.h"

#include "Os/Limits.h"
#include "Os/Memory.h"

#include "ModAutoPointer.h"
#include "ModCharString.h"
#include "ModCharTrait.h"
#include "ModHasher.h"
#include "ModNLP.h"
#include "ModUnicodeCharTrait.h"
#include "ModUnicodeRegularExpression.h"

#include <iostream>
#include <stdlib.h>
using namespace std;

#ifdef USE_ICU
#include "unicode/normlzr.h"
#include "unicode/unistr.h"
#endif

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {
#ifdef OBSOLETE // もうテストしなくていいだろう
#ifdef DEBUG
#include "Common/StringTest.h"
#endif
#endif

namespace _Cast
{

	// TEMPLATE FUNCTION public
	//	$$::_Cast::toInteger -- 文字列を整数値に変換するテンプレート関数
	//
	// TEMPLATE ARGUMENTS
	//	class _T_
	//		変換先の整数型
	//
	// NOTES
	//
	// ARGUMENTS
	//	const ModUnicodeChar* pHead_
	//	const ModUnicodeChar* pTail_
	//		対象の文字列の範囲Range of character string of object
	//	_T_& cValue_
	//		返り値、初期値は呼び出し側がセットするThe call side sets a return value and an initial value. 
	//	_T_& cPower_
	//		処理中の指数、初期値は呼び出し側がセットするThe call side sets an index and an initial value under processing. 
	//	bool bOverflowError
	//		trueの場合範囲を超えたら例外When it is time when the range is exceeded, it is an exception. drink
	//	bool bInvalidCharError
	//		trueの場合不正な文字があったら例外An illegal character of it is time when ..drinking.. is an exception in case of being. 
	//	const ModUnicodeChar** ppRestHead_ = 0
	//	const ModUnicodeChar** ppRestTail_ = 0
	//		bOverflowErrorがfalseのとき範囲を超えたら処理し残した部分が入るHowever, the left part enters by processing it when the range is exceeded at false. 
	//	
	// RETURN
	//		bool
	//			true .. 変換成功
	//
	// EXCEPTIONS

	template <class _T_, class _T1_>
	bool
	toInteger(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
			  _T_& cValue_, _T1_& cPower_,
			  bool bOverflowError_, bool bInvalidCharError_,
			  const ModUnicodeChar** ppRestHead_ = 0,
			  const ModUnicodeChar** ppRestTail_ = 0)
	{
		const int iBase = 10;
		bool bNegative = (cValue_ < static_cast<_T_>(0));
		bool bContinuous = (cPower_ != static_cast<_T1_>(1));
		const ModUnicodeChar* p = pHead_;
		if (!bContinuous) {
			// 先頭から空白文字を読み飛ばすThe null character is skipped from the head. 
			while (p < pTail_ && *p == UnicodeChar::usSpace) ++p;
			// 符号
			if (p < pTail_) {
				switch (*p) {
				case UnicodeChar::usHyphen:	bNegative = true; ++p; break;
				case UnicodeChar::usPlus:					  ++p; break;
				default:										   break;
				}
			}
		}
		const ModUnicodeChar* q = pTail_;
		// 最後尾から空白文字を読み飛ばすThe null character is skipped from the tail. 
		if (!bContinuous)
			while (p < q && *(q-1) == UnicodeChar::usSpace) --q;
		// 制限値を得るThe limitation value is obtained. 
		_T_ cLimit = (bNegative) ? Os::Limits<_T_>::getMin() : Os::Limits<_T_>::getMax();
		if (bContinuous)
			cLimit -= cValue_;
		// 後ろから文字列が尽きるまで検査It inspects it from the back until the character string is exhausted. 
		while (p < q) {
			ModUnicodeChar c = *(--q);
			if (ModUnicodeCharTrait::isDigit(c)) {
				int v = c - UnicodeChar::usZero;
				if (bNegative) v = -v;
				// overflow検査
				if ((!bNegative && cLimit/cPower_ < static_cast<_T_>(v)) || (bNegative && cLimit/cPower_ > static_cast<_T_>(v))) {
					if (bOverflowError_)
						// 超えたらエラーIt makes an error when exceeding it. 
						_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
					else {
						// 単純にfalseを返す
						// (型を変えて続ける可能性があるので引数にセットする)
						if (ppRestHead_) *ppRestHead_ = p;
						if (ppRestTail_) *ppRestTail_ = q + 1;
						return false;
					}
				}
				cValue_ += v * static_cast<_T_>(cPower_);
				cLimit -= v * static_cast<_T_>(cPower_);
				cPower_ *= iBase;
			} else {
				// 数値以外の文字がきたらキャスト失敗It fails Cast when characters other than the numerical value come. 
				if (bInvalidCharError_)
					_TRMEISTER_THROW0(Exception::InvalidCharacter);
				else
					return false;
			}
		}
		return true;
	}

	template <class _T_>
	bool toFloat(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
				 _T_& cValue_, bool bForAssign_, bool bUnknownType_ = false)
	{
		; _TRMEISTER_ASSERT(pHead_ != 0 &&  pTail_ != 0);

		const ModUnicodeChar* p = pHead_;
		// 先頭から空白文字を読み飛ばすThe null character is skipped from the head. 
		while (p < pTail_ && *p == UnicodeChar::usSpace) ++p;
		// 最後尾から空白文字を読み飛ばすThe null character is skipped from the tail. 
		const ModUnicodeChar* q = pTail_;
		while (p < q && *(q-1) == UnicodeChar::usSpace) --q;
		
		if (p < q)
		{
			const ModSize size = static_cast<ModSize>(q - p);
			int nDigit = 0;
			ModCharString cstrBuffer;
			cstrBuffer.reallocate(size + 1);
			bool bExponent = false;
			for (; p < q; ++p)
			{
				ModUnicodeChar c = *p;
				if (ModUnicodeCharTrait::isAscii(c))
				{
					cstrBuffer.append(static_cast<char>(c));
					if (c == UnicodeChar::usSmallD || c == UnicodeChar::usLargeD)
						break;
					if (c == UnicodeChar::usSmallE || c == UnicodeChar::usLargeE)
						bExponent = true;
					else if (ModUnicodeCharTrait::isDigit(c)
							 && (nDigit > 0 || c != UnicodeChar::usZero))
						++nDigit;
				}
				else
					break;
			}

			if (p == q)
			{
				if (!bExponent && bUnknownType_)
				{
					// If target is unknown type, when not all digits are not used, toFloat fail.
					if (nDigit > Os::Limits<_T_>::getDig())
					{
						return false;
					}
				}

				//[sign] [digits] [.digits] [ {e | E}[sign]digits]
				char* pszStopString = 0;
				double dblValue = ::strtod(cstrBuffer, &pszStopString);			
				if (pszStopString && *pszStopString == '\0')
				{
					// 制限値を得るThe limitation value is obtained. 
					_T_ cLimit = Os::Limits<_T_>::getMax();

					// 結果がLIMITを超えているか
					if (cLimit < static_cast<_T_>(dblValue) || -cLimit > static_cast<_T_>(dblValue)) {
						if (bForAssign_)
							// 代入のためのキャストならエラー
							_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
						else
							// 比較のためなら単純にfalse
							return false;
					}

					cValue_ = static_cast<_T_>(dblValue);
					return true;
				}
			}
		}
		
		if (bForAssign_)
			_TRMEISTER_THROW0(Exception::InvalidCharacter);
		else
			return false;
	}

	
	template <class _T_>
	bool
	toDecimal(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
			  _T_& cValue_, bool bForAssign_)
	{
		DecimalData dValue;
		
		if (!dValue.castFromString(pHead_, pTail_, bForAssign_))
			return false;
		cValue_ = static_cast<_T_>(dValue);
		return true;
	}

	// TEMPLATE FUNCTION public
	//	$$::_Cast::toInteger -- 文字列を整数値に変換するテンプレート関数(char*版)
	//
	// TEMPLATE ARGUMENTS
	//	class _T_
	//		変換先の整数型
	//
	// NOTES
	//
	// ARGUMENTS
	//	const char* pHead_
	//	const char* pTail_
	//		対象の文字列の範囲
	//	_T_& cValue_
	//		返り値、初期値は呼び出し側がセットする
	//	_T_& cPower_
	//		処理中の指数、初期値は呼び出し側がセットする
	//	bool bOverflowError
	//		trueの場合範囲を超えたら例外
	//	bool bInvalidCharError
	//		trueの場合不正な文字があったら例外
	//	const char** ppRestHead_ = 0
	//	const cahr** ppRestTail_ = 0
	//		bOverflowErrorがfalseのとき範囲を超えたら処理し残した部分が入る
	//	
	// RETURN
	//		bool
	//			true .. 変換成功
	//
	// EXCEPTIONS

	template <class _T_, class _T1_>
	bool
	toInteger(const char* pHead_, const char* pTail_,
			  _T_& cValue_, _T1_& cPower_,
			  bool bOverflowError_, bool bInvalidCharError_,
			  const char** ppRestHead_ = 0,
			  const char** ppRestTail_ = 0)
	{
		const int iBase = 10;
		bool bNegative = (cValue_ < static_cast<_T_>(0));
		bool bContinuous = (cPower_ != static_cast<_T1_>(1));
		const char* p = pHead_;
		if (!bContinuous) {
			// 先頭から空白文字を読み飛ばす
			while (p < pTail_ && *p == ' ') ++p;
			// 符号
			if (p < pTail_) {
				switch (*p) {
				case '-':	bNegative = true; ++p; break;
				case '+':					  ++p; break;
				default:						   break;
				}
			}
		}
		const char* q = pTail_;
		// 最後尾から空白文字を読み飛ばす
		if (!bContinuous)
			while (p < q && *(q-1) == ' ') --q;
		// 制限値を得る
		_T_ cLimit = (bNegative) ? Os::Limits<_T_>::getMin() : Os::Limits<_T_>::getMax();
		if (bContinuous)
			cLimit -= cValue_;
		// 後ろから文字列が尽きるまで検査
		while (p < q) {
			char c = *(--q);
			if (ModCharTrait::isDigit(c)) {
				int v = c - '0';
				if (bNegative) v = -v;
				// overflow検査
				if ((!bNegative && cLimit/cPower_ < static_cast<_T_>(v)) || (bNegative && cLimit/cPower_ > static_cast<_T_>(v))) {
					if (bOverflowError_)
						// 超えたらエラー
						_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
					else {
						// 単純にfalseを返す
						// (型を変えて続ける可能性があるので引数にセットする)
						if (ppRestHead_) *ppRestHead_ = p;
						if (ppRestTail_) *ppRestTail_ = q + 1;
						return false;
					}
				}
				cValue_ += v * static_cast<_T_>(cPower_);
				cLimit -= v * static_cast<_T_>(cPower_);
				cPower_ *= iBase;
			} else {
				// 数値以外の文字がきたらキャスト失敗
				if (bInvalidCharError_)
					_TRMEISTER_THROW0(Exception::InvalidCharacter);
				else
					return false;
			}
		}
		return true;
	}

	// 文字列を浮動小数点値に変換するテンプレート関数(char*版)
	template <class _T_>
	bool toFloat(const char* pHead_, const char* pTail_,
				 _T_& cValue_, bool bForAssign_, bool bUnknownType_ = false)
	{
		; _TRMEISTER_ASSERT(pHead_ != 0 &&  pTail_ != 0);

		const char* p = pHead_;
		// 先頭から空白文字を読み飛ばすThe null character is skipped from the head. 
		while (p < pTail_ && *p == ' ') ++p;
		// 最後尾から空白文字を読み飛ばすThe null character is skipped from the tail. 
		const char* q = pTail_;
		while (p < q && *(q-1) == ' ') --q;

		if (p < q)
		{
			ModCharString cstrBuffer(p, ModSize(q-p));
			//[sign] [digits] [.digits] [ {e | E}[sign]digits]
			char* pszStopString = 0;
			double dblValue = ::strtod(cstrBuffer, &pszStopString);			
			if (pszStopString && *pszStopString == '\0')
			{
				int nDigit = 0;
				bool bExponent = false;
				for (; p < q; ++p) 
				{
					char c = *p;
					if (c == 'd' || c == 'D')
						break;
					if (c == 'e' || c == 'E')
						bExponent = true;
					else if (ModCharTrait::isDigit(c)
							 && (nDigit > 0 || c != '0'))
						++nDigit;
				}
				if (p == q) // can not find 'd', 'D' before q
				{
					if (!bExponent && bUnknownType_)
					{
						if (nDigit > Os::Limits<_T_>::getDig())//count and compare the digits to check precision
							return false;
					}

					// 制限値を得るThe limitation value is obtained. 
					_T_ cLimit = Os::Limits<_T_>::getMax();

					// 結果がLIMITを超えているか
					if (cLimit < static_cast<_T_>(dblValue) || -cLimit > static_cast<_T_>(dblValue)) {
						if (bForAssign_)
							// 代入のためのキャストならエラー
							_TRMEISTER_THROW0(Exception::NumericValueOutOfRange);
						else
							// 比較のためなら単純にfalse
							return false;
					}

					cValue_ = static_cast<_T_>(dblValue);
					return true;
				}
			}
		}

		if (bForAssign_)
			_TRMEISTER_THROW0(Exception::InvalidCharacter);
		else
			return false;
	}


	template <class _T_>
	bool toDecimal(const char* pHead_, const char* pTail_,
				 _T_& cValue_, bool bForAssign_)
	{
		DecimalData  dValue;
		if ( !dValue.castFromString(pHead_, pTail_, bForAssign_))
			return false;

		cValue_ = static_cast<_T_>(dValue);
		return true;
	}
} // namespace _Cast

} // namespace $$

// FUNCTION public
//	Common::StringData::hashCode -- ハッシュコードを取り出す
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
StringData::
hashCode() const
{
	if (isNull()) return 0;

	return ModUnicodeStringHasher()(m_cstrValue);
}

//
//	FUNCTION private
//	Common::StringData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化を行う
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
StringData::
serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ScalarData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し

		cArchiver_ << m_cstrValue;
	}
	else
	{
		//読み出し

		cArchiver_ >> m_cstrValue;
	}
}

//	FUNCTION private
//	Common::StringData::cast_NotNull -- キャストする
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			このデータ型にキャストする
//
//	RETURN
//		キャストされたデータへのポインタ
//
//	EXCEPTIONS
//		Exception::ClassCast
//			不可能な型へキャストしようとした

Data::Pointer
StringData::cast_NotNull(DataType::Type type, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (type) {
	case DataType::Integer:
		{
			int iValue = 0;
			ModInt64 iPower = 1;
			if (_Cast::toInteger(static_cast<const ModUnicodeChar*>(m_cstrValue),
								 m_cstrValue.getTail(), iValue, iPower,
								 bForAssign_, bForAssign_))
				return new IntegerData(iValue);
			else
				return NullData::getInstance();
		}
	case DataType::UnsignedInteger:
		{
			unsigned int uValue = 0;
			ModInt64 uPower = 1;
			if (_Cast::toInteger(static_cast<const ModUnicodeChar*>(m_cstrValue),
								 m_cstrValue.getTail(), uValue, uPower,
								 bForAssign_, bForAssign_))
				return new UnsignedIntegerData(uValue);
			else
				return NullData::getInstance();
		}
	case DataType::Integer64:
		{
			ModInt64 illValue = 0;
			ModInt64 illPower = 1;
			if (_Cast::toInteger(static_cast<const ModUnicodeChar*>(m_cstrValue),
								 m_cstrValue.getTail(), illValue, illPower,
								 bForAssign_, bForAssign_))
				return new Integer64Data(illValue);
			else
				return NullData::getInstance();
		}
	case DataType::UnsignedInteger64:
		{
			ModUInt64 ullValue = 0;
			ModUInt64 ullPower = 1;
			if (_Cast::toInteger(static_cast<const ModUnicodeChar*>(m_cstrValue),
								 m_cstrValue.getTail(), ullValue, ullPower,
								 bForAssign_, bForAssign_))
				return new UnsignedInteger64Data(ullValue);
			else
				return NullData::getInstance();
		}
	case DataType::ObjectID:
		{
			ModUInt64 ullValue = 0;
			ModUInt64 ullPower = 1;
			if (_Cast::toInteger(static_cast<const ModUnicodeChar*>(m_cstrValue),
								 m_cstrValue.getTail(), ullValue, ullPower,
								 bForAssign_, bForAssign_))
				return new ObjectIDData(ullValue);
			else
				return NullData::getInstance();
		}
	case DataType::String:
		return copy();

	case DataType::Language:
		return new LanguageData(m_cstrValue);

	case DataType::Float:
		{
			float fValue;
			if (_Cast::toFloat(static_cast<const ModUnicodeChar*>(m_cstrValue),
							   m_cstrValue.getTail(), fValue, bForAssign_))
				return new FloatData(fValue);
			else
				return NullData::getInstance();
		}
	case DataType::Double:
		{
			double dblValue;
			if (_Cast::toFloat(static_cast<const ModUnicodeChar*>(m_cstrValue),
							   m_cstrValue.getTail(), dblValue, bForAssign_))
				return new DoubleData(dblValue);
			else
				return NullData::getInstance();
		}
	case DataType::Decimal:
		{
			const DecimalData* pDecimal = _SYDNEY_DYNAMIC_CAST(const DecimalData*, getTargetData());
			if (0 != pDecimal)
			{
				DecimalData ddValue(pDecimal->getPrecision(), pDecimal->getScale());

				if (ddValue.castFromString(static_cast<const ModUnicodeChar*>(m_cstrValue), 
										m_cstrValue.getTail(), bForAssign_))
					return new DecimalData(ddValue);
				else
					return NullData::getInstance();
			}
			break;
		}
	case DataType::Date:
		{
			ModAutoPointer<DateData> pDateData = new DateData;
			pDateData->setValue(static_cast<const ModUnicodeChar*>(m_cstrValue),
								m_cstrValue.getTail(), bForAssign_);
			return pDateData.release();
		}
	case DataType::DateTime:
		{
			ModAutoPointer<DateTimeData> pDateTimeData = new DateTimeData;
			pDateTimeData->setValue(static_cast<const ModUnicodeChar*>(m_cstrValue),
									m_cstrValue.getTail(), bForAssign_);
			return pDateTimeData.release();
		}
	case DataType::Binary:

		// SQL Server の仕様にあわせて、文字列をバイナリにキャストするときは、
		// 文字列の末尾の終端文字はバイナリに含まれない

		return new BinaryData(
			static_cast<const ModUnicodeChar*>(m_cstrValue),
			getLength() * sizeof(ModUnicodeChar));

	case DataType::Null:
		return NullData::getInstance();
	}

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION private
//	Common::StringData::cast_NotNull -- 与えられたデータの型へキャストする
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&		target
//			このデータの型へキャストする
//
//	RETURN
//		自分自身の型をキャストした結果得られるデータを指すオブジェクトポインタ
//
//	EXCEPTIONS

Data::Pointer
StringData::cast_NotNull(const Data& target, bool bForAssign_ /* = false */) const
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (target.getType()) {
	case DataType::String:

		// 符号化方式、Collationも引き継ぐ
		const StringData& cTarget = _SYDNEY_DYNAMIC_CAST(const StringData&, target);
		return new StringData(
			m_cstrValue,
			cTarget.getEncodingForm(),
			cTarget.getCollation());
	}

	return cast(target.getType(), bForAssign_);
}

Data::Pointer
StringData::castToDecimal(bool bForAssign_) const
{
	DecimalData cDecimal(0,0);
	if (cDecimal.castFromString(static_cast<const ModUnicodeChar*>(m_cstrValue), 
										m_cstrValue.getTail(), bForAssign_))
		return new DecimalData(cDecimal);
	else
		return NullData::getInstance();
}

//	FUNCTION private
//	Common::StringData::compareTo_NoCast -- 大小比較を行う
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
StringData::compareTo_NoCast(const Data& r) const
{
#ifdef OBSOLETE // castの結果Nullになることがある場合生かす
	if (isNull())
		return NullData::getInstance()->compareTo(r);
#endif

	; _TRMEISTER_ASSERT(r.getType() == DataType::String);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	// SQL規格ではPAD SPACEのCollationでは文字列の比較は短いほうの後ろに<space>を埋めたもので比較することになっている
	// SydneyのデフォルトCollationはPAD SPACEである
	// 文字列のコピーを避けるため、一旦短いほうの長さだけ比較して残りの部分に空白以外があるかを調べることにする
	//
	// hint nontruncateのついた列に対してはNO PADの動作をしなければならない

	const StringData& data = _SYDNEY_DYNAMIC_CAST(const StringData&, r);

	const ModUnicodeString& d1 = m_cstrValue;
	const ModUnicodeString& d2 = data.m_cstrValue;

	ModSize l1 = d1.getLength();
	ModSize l2 = d2.getLength();

	ModSize lmin = ModMin(l1, l2);

	int result = (lmin > 0 ? d1.compare(d2, lmin) : 0);
	if (result == 0) {
		if (getCollation() == Collation::Type::NoPad || data.getCollation() == Collation::Type::NoPad) {

			result = (l1 < l2 ? -1 : (l1 > l2 ? 1 : 0));

		} else {
			const ModUnicodeChar* p1 = d1;
			const ModUnicodeChar* end1 = p1 + l1;
			for (p1 += lmin; p1 != end1; ++p1) {
				if (*p1 < UnicodeChar::usSpace) {
					// 左辺に空白より小さい文字があった → 左辺のほうが小さい
					result = -1;
					break;
				}
				if (*p1 > UnicodeChar::usSpace) {
					// 左辺に空白より大きい文字があった → 右辺のほうが小さい
					result = 1;
					break;
				}
			}
			const ModUnicodeChar* p2 = d2;
			const ModUnicodeChar* end2 = p2 + l2;
			for (p2 += lmin; p2 != end2; ++p2) {
				if (*p2 < UnicodeChar::usSpace) {
					// 右辺に空白より小さい文字があった → 右辺のほうが小さい
					result = 1;
					break;
				}
				if (*p2 > UnicodeChar::usSpace) {
					// 右辺に空白より大きい文字があった → 左辺のほうが小さい
					result = -1;
					break;
				}
			}
		}
	}
	return result;
}

//	FUNCTION public
//	Common::StringData::similar
//		自分自身に対して正規表現がマッチするか調べる
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeRegularExpression* pattern
//			自分自身に対してマッチするか調べる正規表現を表すデータ
//
//	RETURN
//		true
//			マッチした
//		false
//			マッチしなかった
//
//	EXCEPTIONS
//		Exception::BadArgument
//			パターンとしてNULL 値が与えられた
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

bool
StringData::similar(
	ModUnicodeRegularExpression* pattern) const
{
	; _TRMEISTER_ASSERT(pattern);

	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return similar_NotNull(pattern);
}

bool
StringData::similar_NotNull(
	ModUnicodeRegularExpression* pattern) const
{
	; _TRMEISTER_ASSERT(pattern);
	; _TRMEISTER_ASSERT(!isNull());

	const ModUnicodeString& v = m_cstrValue;

	int result = pattern->advance(v);
	return result != 0 && (pattern->matchEnd() - pattern->matchBegin()) == v.getLength();
}

//
//	FUNCTION public
//	Common::StringData::connect -- 文字列連結を行う
//
//	NOTES
//	文字列連結を行う。
//
//	ARGUMENTS
//	const Common::StringData* pStringData_
//		連結する文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			NULL 値が与えられた
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

void
StringData::connect(const StringData* pStringData_)
{
	; _TRMEISTER_ASSERT(pStringData_);

	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	if (pStringData_->isNull())
		_TRMEISTER_THROW0(Exception::BadArgument);

	connect_NotNull(pStringData_);
}

void
StringData::connect_NotNull(const StringData* pStringData_)
{
	; _TRMEISTER_ASSERT(pStringData_);
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!pStringData_->isNull());

	m_cstrValue += pStringData_->m_cstrValue;
}

//	FUNCTION public
//	Common::StringData::getEncodingForm -- 符号化方式を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた符号化方式を表す値
//
//	EXCEPTIONS
//		なし

StringData::EncodingForm::Value
StringData::getEncodingForm() const
{
	return _encodingForm;
}

//	FUNCTION public
//	Common::StringData::getCollation -- 比較方式を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた比較方式を表す値
//
//	EXCEPTIONS
//		なし

Collation::Type::Value
StringData::getCollation() const
{
	return _collation;
}

// FUNCTION public
//	Common::StringData::setEncodingForm -- 符号化方式を設定する
//
// NOTES
//	serializeの拡張用。データ自体には何も作用しない
//
// ARGUMENTS
//	EncodingForm::Value eEncodingForm_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
StringData::
setEncodingForm(EncodingForm::Value eEncodingForm_)
{
	_encodingForm = eEncodingForm_;
}

// FUNCTION public
//	Common::StringData::setCollation -- Collationを設定する
//
// NOTES
//	serializeの拡張用。データ自体には何も作用しない
//
// ARGUMENTS
//	Collation::Type::Value eCollation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
StringData::
setCollation(Collation::Type::Value eCollation_)
{
	_collation = eCollation_;
}

//
//	FUNCTION private
//	Common::StringData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::StringData クラスのクラスID
//
//	EXCEPTIONS
//	なし
//
int
StringData::
getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return ClassID::StringDataClass;
}

//
//	FUNCTION private
//	Common::StringData::print_NotNull -- 値を表示する
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
StringData::
print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	cout << "string: " << getString().getString(Common::LiteralCode) << endl;
}

//	FUNCTION private
//	Common::StringData::copy_NotNull -- 自分自身の複製を生成する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		生成された複製へのポインタ
//
//	EXCEPTIONS

Data::Pointer
StringData::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return new StringData(*this);
}

//	FUNCTION public
//	Common::StringData::getValue -- 値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた値
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

const ModUnicodeString&
StringData::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return getValue_NotNull();
}

const ModUnicodeString&
StringData::getValue_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return m_cstrValue;
}

//	FUNCTION protected
//	Common::StringData::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		ModUnicodeString&	cstrValue_
//			設定する文字列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
StringData::setValue(const ModUnicodeString& cstrValue_)
{
	m_cstrValue = cstrValue_;
	setNull(false);
}

//	FUNCTION protected
//	Common::StringData::setValue -- 値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Common::StringData&	cOther_
//			設定する文字列データ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
StringData::setValue(const StringData& cOther_)
{
	if (cOther_.isNull())
		setNull(true);
	else {
		setFunction(cOther_.getFunction());
		m_cstrValue = cOther_.getValue();

		/*【暫定】符号化方式も複写する。本当は使用方法から検討すべき */
		_encodingForm = cOther_.getEncodingForm();

		setNull(false);
	}
}

// FUNCTION public
//	Common::StringData::assign_NoCast -- 代入を行う(キャストなし)
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
StringData::
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

	const StringData& data = _SYDNEY_DYNAMIC_CAST(const StringData&, r);
	setValue(data.m_cstrValue);
	return true;
}

//
//	FUNCTION private
//	Common::StringData::isApplicable_NotNull --
//		付加機能を適用可能かを得る
//
//	NOTES
//		圧縮と圧縮ストリームに対応する
//
//	ARGUMENTS
//		Common::Data::Function::Value iFunction_
//			適用しようとしている付加機能
//
//	RETURN
//		true ... 適用できる
//		false... 適用できない
//
//	EXCEPTIONS

bool
StringData::
isApplicable_NotNull(Function::Value iFunction_)
{
	; _TRMEISTER_ASSERT(!isNull());

#ifdef OBSOLETE
	if ((iFunction_ & Function::Compressed)
		&& !(iFunction_ & ~(Function::Compressed | Function::Stream))) {
#endif
	if ((iFunction_ & Function::Compressed)
		&& !(iFunction_ & ~Function::Compressed)) {
		const ModUnicodeChar* pValue = static_cast<const ModUnicodeChar*>(m_cstrValue);
		ModSize iValueSize = m_cstrValue.getLength() * sizeof(ModUnicodeChar);

		return CompressedData::isCompressable(syd_reinterpret_cast<const char*>(pValue), iValueSize);
	}
	return false;
}

//
//	FUNCTION private
//	Common::StringData::apply_NotNull --
//		付加機能を適用したCommon::Dataを得る
//
//	NOTES
//		圧縮と圧縮ストリームに対応する
//
//	ARGUMENTS
//		Common::Data::Function::Value iFunction_
//			適用する付加機能
//
//	RETURN
//		付加機能を適用したCommon::Data
//
//	EXCEPTIONS
//		Exception::NotSupported
//			applyに対応していない

Data::Pointer
StringData::
apply_NotNull(Function::Value iFunction_)
{
	; _TRMEISTER_ASSERT(!isNull());

	switch (iFunction_) {
	case Function::Compressed:
	{
		return new CompressedStringData(m_cstrValue);
	}
#ifdef OBSOLETE
	case (Function::Compressed | Function::Stream):
	{
		return new CompressedStreamStringData(m_cstrValue);
	}
#endif
	default:
		break;
	}
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	Common::StringData::isAbleToDump_NotNull -- ダンプ可能か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ダンプ可能である
//		false
//			ダンプ不可である
//
//	EXCEPTIONS
//		なし

bool
StringData::isAbleToDump_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	return true;
}

//	FUNCTION public
//	Common::StringData::getDumpSize --
//		保持する値をダンプしたときの文字列データのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Common::StringData::EncodingForm::Value	encodingForm
//			保持する値をこの符号化方式で変換し、ダンプしたときのサイズを求める
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
StringData::getDumpSize(EncodingForm::Value encodingForm) const
{
	return (isNull()) ?
		NullData::getInstance()->getDumpSize() :
		getDumpSize_NotNull(encodingForm);
}

ModSize
StringData::getDumpSize_NotNull(EncodingForm::Value encodingForm) const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModSize len;

	switch (encodingForm) {
	case EncodingForm::UCS2:
		len = m_cstrValue.getLength() * sizeof(ModUnicodeChar);
		break;
	case EncodingForm::Unknown:
		len = m_cstrValue.getLength();
		break;
	default:
		len = ModKanjiCode::jjGetTransferredSize(
			syd_reinterpret_cast<const char*>(
				static_cast<const ModUnicodeChar*>(m_cstrValue)),
			ModKanjiCode::ucs2, getLength() * sizeof(ModUnicodeChar),
			static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm)));
	}

	return len;
}

//	FUNCTION public
//	Common::StringData::setDumpedValue --
//		ダンプされた文字列データを使って値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		char*				src
//			ダンプされた文字列データを格納する領域の先頭アドレス
//		ModSize				n
//			ダンプされた文字列データを格納する領域のサイズ(B 単位)
//		Common::StringData::EncodingForm::Value	encodingForm
//			ダンプされた文字列データの符号化方式
//
//	RETURN
//		設定されたダンプされた文字列データのサイズ(B 単位)
//
//	EXCEPTIONS

// virtual
ModSize
StringData::setDumpedValue(
	const char* src, ModSize n, EncodingForm::Value encodingForm)
{
	if (n) {

		// ModUnicodeString::allocateCopy は複写する文字数を要求するので、
		// 複写するバイト数を文字数へ変換する

		ModSize len;

		switch (encodingForm) {
		case EncodingForm::UCS2:
			len = n / sizeof(ModUnicodeChar);
			
			// 高速化のためまたキャストに戻した。本来ならlibCommonを修正する
			// べきであるが、WindowsとLinuxなら問題ないのでとりあえずこうする
			
			m_cstrValue.allocateCopy(
				syd_reinterpret_cast<const ModUnicodeChar*>(src),
				len);
			break;
		case EncodingForm::Unknown:
			len = n;
			m_cstrValue.allocateCopy(
				src, len,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm)));
			break;
		default:
			len = ModKanjiCode::jjGetTransferredSize(
				src,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm)),
				n, ModKanjiCode::ucs2) / sizeof(ModUnicodeChar);
			m_cstrValue.allocateCopy(
				src, len,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm)));
		}
	} else
		m_cstrValue.clear();

	// NULL 値でなくする

	setNull(false);

	return n;
}

// virtual
ModSize
StringData::setDumpedValue(ModSerialIO& cSerialIO_, ModSize n, EncodingForm::Value encodingForm_)
{
	if (n) 
	{
		// ModUnicodeString::allocateCopy は複写する文字数を要求するので、
		// 複写するバイト数を文字数へ変換する

		ModSize len;
		char* src = 0;

		switch (encodingForm_) 
		{
		case EncodingForm::UCS2:
			len = n;
			src = new char[len];
			cSerialIO_.readSerial(src, len, ModSerialIO::dataTypeCharacterArray);

			// 高速化のためまたキャストに戻した。本来ならlibCommonを修正する
			// べきであるが、WindowsとLinuxなら問題ないのでとりあえずこうする

			len /= sizeof(ModUnicodeChar);
			m_cstrValue.allocateCopy(
				syd_reinterpret_cast<const ModUnicodeChar*>(src),
				len);
		
			delete[] src;
			break;
		case EncodingForm::Unknown:
			len = n;
			src = new char[len];
			cSerialIO_.readSerial(src, len, ModSerialIO::dataTypeCharacterArray);
			m_cstrValue.allocateCopy(
				src, len,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm_)));
			delete[] src;
			break;
		default:
			src = new char[n];
			cSerialIO_.readSerial(src, n, ModSerialIO::dataTypeCharacterArray);
			len = ModKanjiCode::jjGetTransferredSize(
				src,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm_)),
				n, ModKanjiCode::ucs2) / sizeof(ModUnicodeChar);
			m_cstrValue.allocateCopy(
				src, len,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm_)));
			delete[] src;
			break;
		}
	} 
	else
		m_cstrValue.clear();

	// NULL 値でなくする

	setNull(false);

	return n;
}

ModSize
StringData::dumpValue(ModSerialIO& cSerialIO_, EncodingForm::Value encodingForm_) const
{
	return (isNull()) ?
		NullData::getInstance()->dumpValue(cSerialIO_) :
	dumpValue_NotNull(cSerialIO_, encodingForm_);
}

ModSize
StringData::dumpValue_NotNull(ModSerialIO& cSerialIO_, EncodingForm::Value encodingForm_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	const ModSize size = getDumpSize_NotNull(encodingForm_);
	if (size) 
	{
		switch (encodingForm_) 
		{
		case EncodingForm::UCS2:
			cSerialIO_.writeSerial(
				syd_reinterpret_cast<const char*>(static_cast<const ModUnicodeChar*>(m_cstrValue)), 
				size, ModSerialIO::dataTypeCharacterArray);
			break;
		case EncodingForm::Unknown:
			{
				const ModUnicodeChar* p = m_cstrValue;
				
				//that's not good for call cSerialIO_.writeSerial many times.
				//for (ModSize i = 0; i < size; ++i, ++p)
				//	cSerialIO_.writeSerial(p, sizeof(const char));

				//it's not good too for a tempary buffer
				char* buf = new char[size];
				for (ModSize i = 0; i < size; ++i, ++p)
					*(buf+i) = static_cast<unsigned char>(*p);
				//then write 
				cSerialIO_.writeSerial(buf, size, ModSerialIO::dataTypeCharacterArray);
				delete[] buf;
			}
			break;
		default:
			{
				char* dst = new char[size];
				(void) ModKanjiCode::jjTransfer(
					dst, size,
					static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm_)),
					syd_reinterpret_cast<const char*>(
					static_cast<const ModUnicodeChar*>(m_cstrValue)),
					ModKanjiCode::ucs2);

				cSerialIO_.writeSerial(dst, size, ModSerialIO::dataTypeCharacterArray);
				delete[] dst;
			}
		}
	}

	return size;
}

//	FUNCTION public
//	Common::StringData::dumpValue -- 保持する値をダンプする
//
//	NOTES
//
//	ARGUMENTS
//		char*				dst
//			保持する値がダンプされる領域の先頭アドレス
//		Common::StringData::EncodingForm::Value	encodingForm
//			保持する値はこの符号化方式で変換され、ダンプされる
//
//	RETURN
//		ダンプされた文字列データのサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
StringData::dumpValue(char* dst, EncodingForm::Value encodingForm) const
{
	return (isNull()) ?
		NullData::getInstance()->dumpValue(dst) :
		dumpValue_NotNull(dst, encodingForm);
}

ModSize
StringData::dumpValue_NotNull(
			char* dst, EncodingForm::Value encodingForm) const
{
	; _TRMEISTER_ASSERT(!isNull());

	const ModSize size = getDumpSize_NotNull(encodingForm);
	if (size) 
	{
		; _TRMEISTER_ASSERT(dst);

		switch (encodingForm) 
		{
		case EncodingForm::UCS2:
			(void) Os::Memory::copy(
				dst,
				syd_reinterpret_cast<const char*>(
					static_cast<const ModUnicodeChar*>(m_cstrValue)), size);
			break;
		case EncodingForm::Unknown:
			{
				const ModUnicodeChar* p = m_cstrValue;
				for (ModSize i = 0; i < size; ++i, ++dst, ++p)
					*dst = static_cast<unsigned char>(*p);
			}
			break;
		default:
			(void) ModKanjiCode::jjTransfer(
				dst, size,
				static_cast<ModKanjiCode::KanjiCodeType>(int(encodingForm)),
				syd_reinterpret_cast<const char*>(
					static_cast<const ModUnicodeChar*>(m_cstrValue)),
				ModKanjiCode::ucs2);
		}
	}
	return size;
}

// FUNCTION public
//	Common::StringData::getInteger -- 数値に変換する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	Common::DataType::Type eType_
//	bool bForAssign_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//static
Common::Data::Pointer
StringData::
getInteger(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
		   Common::DataType::Type eType_, bool bForAssign_)
{
	bool bRepeat = (eType_ == Common::DataType::Undefined);
										// 型が指定されていない場合はWhen the type is not specified
										// 表現できる最小の型で返すIt returns it by the expressible minimum type. 
	Common::DataType::Type eType =
		(eType_ == Common::DataType::Undefined) ? Common::DataType::Integer
		: eType_;
										// まずはintから試すFirst of all, it tries from int. 
										// 指定されている場合はその型で試すIt tries by the type when specified. 
#if SYD_BYTEORDER == SYD_ORDER_HL

   struct {
	   int m_iValue;
	   unsigned int m_uValue;
	   ModInt64 m_illValue;
	   ModUInt64 m_ullValue;
   } cValue, cPower;
   cValue.m_ullValue =cValue.m_iValue=cValue.m_uValue=cValue.m_illValue= 0;
   cPower.m_ullValue =cPower.m_iValue=cPower.m_uValue=cPower.m_illValue= 1;

#else

	union {
		int m_iValue;
		unsigned int m_uValue;
		ModInt64 m_illValue;
		ModUInt64 m_ullValue;
	} cValue, cPower;

	// 一番大きい型で初期化しておけばいいだろうYou will only have to initialize it by the largest type. 
	cValue.m_ullValue = 0;
	cPower.m_ullValue = 1;
#endif
	
	const ModUnicodeChar* pRestHead = 0;
	const ModUnicodeChar* pRestTail = 0;
	const ModUnicodeChar* pHead = pHead_;
	const ModUnicodeChar* pTail = pTail_;

	while (true) {
		switch (eType) {
		case Common::DataType::Integer:
			{
				if (_Cast::toInteger(pHead, pTail,
									 cValue.m_iValue, cPower.m_illValue,
									 !bRepeat && bForAssign_, /* overflow no error */
									 bForAssign_,
									 &pRestHead, &pRestTail))
					return new IntegerData(cValue.m_iValue);

				if (bRepeat && pRestHead) {
					eType = Common::DataType::Integer64;
					cValue.m_illValue = cValue.m_iValue;
					cPower.m_illValue = cPower.m_illValue;
					// no more large size
					//bRepeat = false;
					pHead = pRestHead;
					pTail = pRestTail;
				}				
				else 
				{
					if (bForAssign_)
						_TRMEISTER_THROW0(Exception::InvalidCharacter);
					else
						return NullData::getInstance();
				}
				break;
			}
		case Common::DataType::UnsignedInteger:
			{
				if (_Cast::toInteger(pHead, pTail,
									 cValue.m_uValue, cPower.m_illValue,
									 !bRepeat && bForAssign_, /* overflow no error */
									 bForAssign_,
									 &pRestHead, &pRestTail))
					return new UnsignedIntegerData(cValue.m_uValue);

				if (bRepeat && pRestHead) {
					eType = Common::DataType::Integer64;
					cValue.m_illValue = cValue.m_uValue;
					cPower.m_illValue = cPower.m_illValue;
					pHead = pRestHead;
					pTail = pRestTail;
				}				
				else 
				{
					if (bForAssign_)
						_TRMEISTER_THROW0(Exception::InvalidCharacter);
					else
						return NullData::getInstance();
				}
				break;
			}
		case Common::DataType::Integer64:
			{
				if (_Cast::toInteger(pHead, pTail,
									 cValue.m_illValue, cPower.m_illValue,
									 !bRepeat && bForAssign_, /* overflow no error */
									 bForAssign_,
									 &pRestHead, &pRestTail))
					return new Integer64Data(cValue.m_illValue);

				if (bRepeat && pRestHead) {
					eType = Common::DataType::Decimal;
					//cValue.m_illValue = cValue.m_iValue;
					//cPower.m_illValue = cPower.m_iValue;
					// no more large size
					//bRepeat = false;
					pHead = pRestHead;
					pTail = pRestTail;
				}				
				else 
				{
					if (bForAssign_)
						_TRMEISTER_THROW0(Exception::InvalidCharacter);
					else
						return NullData::getInstance();
				}
				break;
			}
		case Common::DataType::UnsignedInteger64:
			{
				if (_Cast::toInteger(pHead, pTail,
									 cValue.m_ullValue, cPower.m_ullValue,
									 !bRepeat && bForAssign_, /* overflow no error */
									 bForAssign_,
									 &pRestHead, &pRestTail))
					return new UnsignedInteger64Data(cValue.m_ullValue);
				
				if (bRepeat && pRestHead) {
					eType = Common::DataType::Decimal;
					//cValue.m_illValue = cValue.m_iValue;
					//cPower.m_illValue = cPower.m_iValue;
					// no more large size
					//bRepeat = false;
					pHead = pRestHead;
					pTail = pRestTail;
				}
				else 
				{
					if (bForAssign_)
						_TRMEISTER_THROW0(Exception::InvalidCharacter);
					else
						return NullData::getInstance();
				}
				break;
			}
		case Common::DataType::Decimal:
			{
				DecimalData dValue;
				if (_Cast::toDecimal(pHead_, pTail_, dValue, bForAssign_))
					return new DecimalData(dValue);
				else 
					return NullData::getInstance();
				break;
			}
		
		}
	}
	return NullData::getInstance();
}

// FUNCTION public
//	Common::StringData::getFloat -- 数値に変換する
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	Common::DataType::Type eType_
//	bool bForAssign_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//static
Common::Data::Pointer
StringData::
getFloat(const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_,
		 Common::DataType::Type eType_, bool bForAssign_)
{
	if (eType_ == Common::DataType::Float) {
		// FloatDataを作る
		float fValue;
		if (_Cast::toFloat(pHead_, pTail_, fValue, bForAssign_))
			return new FloatData(fValue);
		else
			return NullData::getInstance();
	} else {
		// Floatが指定されなければ常にDoubleDataを作る
		double dblValue;
		if (_Cast::toFloat(pHead_, pTail_, dblValue, bForAssign_, eType_ == DataType::Undefined))
		{
			return new DoubleData(dblValue);
		}
		else if (eType_ == DataType::Undefined)
		{
			DecimalData dValue;
			if (_Cast::toDecimal(pHead_, pTail_, dValue, bForAssign_))
				return new DecimalData(dValue);
			else
				return NullData::getInstance(); 
		}
		else
		{
			return NullData::getInstance();
		}
	}
}

// FUNCTION public
//	Common::StringData::getInteger -- 数値に変換する(Common::Dataをあらかじめ用意するバージョン)
//
// NOTES
//
// ARGUMENTS
//	Common::Data& cData_
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
StringData::
getInteger(Data& cData_,
		   const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
{
	switch (cData_.getType()) {
	case DataType::Integer:
		{
			int iValue = 0;
			ModInt64 iPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 iValue, iPower,
								 false, false, /* no throw */
								 0, 0)) {
				_SYDNEY_DYNAMIC_CAST(IntegerData&, cData_).setValue(iValue);
				return true;
			}
			break;
		}
	case DataType::UnsignedInteger:
		{
			unsigned int uValue = 0;
			ModInt64 uPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 uValue, uPower,
								 false, false, /* no throw */
								 0, 0 )) {
				_SYDNEY_DYNAMIC_CAST(UnsignedIntegerData&, cData_).setValue(uValue);
				return true;
			}
			break;
		}
	case DataType::Integer64:
		{
			ModInt64 illValue = 0;
			ModInt64 illPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 illValue, illPower,
								 false, false, /* no throw */
								 0, 0 )) {
				_SYDNEY_DYNAMIC_CAST(Integer64Data&, cData_).setValue(illValue);
				return true;
			}
			break;
		}
	case DataType::UnsignedInteger64:
		{
			ModUInt64 ullValue = 0;
			ModUInt64 ullPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 ullValue, ullPower,
								 false, false, /* no throw */
								 0, 0 )) {
				_SYDNEY_DYNAMIC_CAST(UnsignedInteger64Data&, cData_).setValue(ullValue);
				return true;
			}
			break;
		}
				
	case Common::DataType::Decimal:
		{
			DecimalData dValue;
			if (_Cast::toDecimal(pHead_, pTail_, dValue, false))
			{
				DecimalData& decimal = _SYDNEY_DYNAMIC_CAST(DecimalData&, cData_);
				int iPartLen1 = decimal.getPrecision()- decimal.getScale();
				int iPartLen2 = dValue.getPrecision()- dValue.getScale();
				if (iPartLen1 >= iPartLen2)
				{
					decimal.assign(&dValue);
					return true;
				}
			}

			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// FUNCTION public
//	Common::StringData::getFloat -- 数値に変換する(Common::Dataをあらかじめ用意するバージョン)
//
// NOTES
//
// ARGUMENTS
//	Common::Data& cData_
//	const ModUnicodeChar* pHead_
//	const ModUnicodeChar* pTail_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
StringData::
getFloat(Common::Data& cData_,
		 const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
{
	switch (cData_.getType()) {
	case DataType::Float:
		{
			// FloatDataを作る
			float fValue;
			if (_Cast::toFloat(pHead_, pTail_, fValue, false /* no throw */)) {
				_SYDNEY_DYNAMIC_CAST(FloatData&, cData_).setValue(fValue);
				return true;
			}
			break;
		}
	case DataType::Double:
		{
			// DoubleDataを作る
			double dblValue;
			if (_Cast::toFloat(pHead_, pTail_, dblValue, false /* no throw */)) {
				_SYDNEY_DYNAMIC_CAST(DoubleData&, cData_).setValue(dblValue);
				return true;
			}
			break;
		}
	case Common::DataType::Decimal:
		{
			DecimalData dValue;
			if (_Cast::toDecimal(pHead_, pTail_, dValue, false))
			{
				DecimalData& decimal = _SYDNEY_DYNAMIC_CAST(DecimalData&, cData_);
				int iPartLen1 = decimal.getPrecision()- decimal.getScale();
				int iPartLen2 = dValue.getPrecision()- dValue.getScale();
				if (iPartLen1 >= iPartLen2)
				{
					decimal.assign(&dValue);
					return true;
				}
			}

			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}


////static 
//bool 
//StringData::
//getDecimal(Data& cData_,
//		 const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
//{
//	return false;
//}
// 数値に変換する(Common::Dataをあらかじめ用意するバージョンchar*版)
//static
bool
StringData::
getInteger(Data& cData_, const char* pHead_, const char* pTail_)
{
	switch (cData_.getType()) {
	case DataType::Integer:
		{
			int iValue = 0;
			ModInt64 iPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 iValue, iPower,
								 false, false, /* no throw */
								 0, 0)) {
				_SYDNEY_DYNAMIC_CAST(IntegerData&, cData_).setValue(iValue);
				return true;
			}
			break;
		}
	case DataType::UnsignedInteger:
		{
			unsigned int uValue = 0;
			ModInt64 uPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 uValue, uPower,
								 false, false, /* no throw */
								 0, 0 )) {
				_SYDNEY_DYNAMIC_CAST(UnsignedIntegerData&, cData_).setValue(uValue);
				return true;
			}
			break;
		}
	case DataType::Integer64:
		{
			ModInt64 illValue = 0;
			ModInt64 illPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 illValue, illPower,
								 false, false, /* no throw */
								 0, 0 )) {
				_SYDNEY_DYNAMIC_CAST(Integer64Data&, cData_).setValue(illValue);
				return true;
			}
			break;
		}
	case DataType::UnsignedInteger64:
		{
			ModUInt64 ullValue = 0;
			ModUInt64 ullPower = 1;
			if (_Cast::toInteger(pHead_, pTail_,
								 ullValue, ullPower,
								 false, false, /* no throw */
								 0, 0 )) {
				_SYDNEY_DYNAMIC_CAST(UnsignedInteger64Data&, cData_).setValue(ullValue);
				return true;
			}
			break;
		}
	case Common::DataType::Decimal:
		{
			DecimalData dValue;
			if (_Cast::toDecimal(pHead_, pTail_, dValue, false))
			{
				DecimalData& decimal = _SYDNEY_DYNAMIC_CAST(DecimalData&, cData_);
				int iPartLen1 = decimal.getPrecision()- decimal.getScale();
				int iPartLen2 = dValue.getPrecision()- dValue.getScale();
				if (iPartLen1 >= iPartLen2)
				{
					decimal.assign(&dValue);
					return true;
				}
			}

			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// 数値に変換する(Common::Dataをあらかじめ用意するバージョンchar*版)
//static
bool
StringData::
getFloat(Data& cData_, const char* pHead_, const char* pTail_)
{
	switch (cData_.getType()) {
	case DataType::Float:
		{
			// FloatDataを作る
			float fValue;
			if (_Cast::toFloat(pHead_, pTail_, fValue, false /* no throw */)) {
				_SYDNEY_DYNAMIC_CAST(FloatData&, cData_).setValue(fValue);
				return true;
			}
			break;
		}
	case DataType::Double:
		{
			// DoubleDataを作る
			double dblValue;
			if (_Cast::toFloat(pHead_, pTail_, dblValue, false /* no throw */)) {
				_SYDNEY_DYNAMIC_CAST(DoubleData&, cData_).setValue(dblValue);
				return true;
			}
			break;
		}
	case Common::DataType::Decimal:
		{
			DecimalData dValue;
			if (_Cast::toDecimal(pHead_, pTail_, dValue, false))
			{
				DecimalData& decimal = _SYDNEY_DYNAMIC_CAST(DecimalData&, cData_);
				int iPartLen1 = decimal.getPrecision()- decimal.getScale();
				int iPartLen2 = dValue.getPrecision()- dValue.getScale();
				if (iPartLen1 >= iPartLen2)
				{
					decimal.assign(&dValue);
					return true;
				}
			}

			break;
		}
	default:
		{
			break;
		}
	}
	return false;
}

// privitiveデータで得るバージョン
//static
bool
StringData::
getInteger(int& iValue_,
		   const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
{
	int iPower = 1;
	return _Cast::toInteger(pHead_, pTail_,
							iValue_, iPower,
							false, false, /* no throw */
							0, 0);
}

//static
bool
StringData::
getFloat(double& dblValue_,
		 const ModUnicodeChar* pHead_, const ModUnicodeChar* pTail_)
{
	return _Cast::toFloat(pHead_, pTail_, dblValue_, false /* no throw */);
}

// primitiveデータで得る -- char*版
//static
bool
StringData::
getInteger(int& iValue_, const char* pHead_, const char* pTail_)
{
	int iPower = 1;
	return _Cast::toInteger(pHead_, pTail_,
							iValue_, iPower,
							false, false, /* no throw */
							0, 0);
}

//static
bool
StringData::
getFloat(double& dblValue_, const char* pHead_, const char* pTail_)
{
	return _Cast::toFloat(pHead_, pTail_, dblValue_, false /* no throw */);
}

// FUNCTION public
//	Common::StringData::getSQLTypeByValue -- 実際の値を用いてSQLDataを得る(getSQLTypeの下請け)
//
// NOTES
//
// ARGUMENTS
//	SQLData& cType_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
StringData::
getSQLTypeByValue(SQLData& cType_)
{
	if (_encodingForm != EncodingForm::UCS2) {
		// NCharではなくChar
		cType_.setType(SQLData::Type::Char);
	}
	if (!isNull()) {
		// 文字列長をLengthにセットする
		cType_.setLength(getLength());
		// データからセットする場合はFlagがFixedになる
		cType_.setFlag(SQLData::Flag::Fixed);
	}
	return true;
}

//
//	Copyright (c) 1999, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
