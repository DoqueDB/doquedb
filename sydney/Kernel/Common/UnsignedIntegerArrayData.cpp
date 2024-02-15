// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnsignedIntegerArrayData -- UnsignedInteger(32ビット)の配列をあらわすクラス
// 
// Copyright (c) 1999, 2000, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Common/StringArrayData.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "Exception/ClassCast.h"

#include "ModHasher.h"
#include "ModUnicodeOstrStream.h"
#include "ModMessage.h"
#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::UnsignedIntegerArrayData
//													-- コンストラクタ(1)
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
UnsignedIntegerArrayData::
UnsignedIntegerArrayData()
: ArrayData(DataType::UnsignedInteger)
{
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::UnsignedIntegerArrayData
//													-- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModVector<unsigned int>& vecuiValue_
//		unsigned intの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
UnsignedIntegerArrayData::
UnsignedIntegerArrayData(
	const ModVector<unsigned int>& vecuiValue_)
: ArrayData(DataType::UnsignedInteger)
{
	setValue(vecuiValue_);
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::~UnsignedIntegerArrayData -- デストラクタ
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
UnsignedIntegerArrayData::
~UnsignedIntegerArrayData()
{
}

// FUNCTION public
//	Common::UnsignedIntegerArrayData::hashCode -- ハッシュコードを取り出す
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
UnsignedIntegerArrayData::
hashCode() const
{
	if (isNull()) return 0;

	ModVector<unsigned int>::ConstIterator iterator = m_vecuiValue.begin();
	const ModVector<unsigned int>::ConstIterator last = m_vecuiValue.end();

	ModSize hashValue = 0;
	ModSize g;
	while (iterator != last) {
		hashValue <<= 4;
		hashValue += static_cast<ModSize>(*iterator);
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
		++iterator;
	}
	return hashValue;
}

//	FUNCTION private
//	Common::UnsignedIntegerArrayData::serialize_NotNull -- シリアル化
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

void
UnsignedIntegerArrayData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ArrayData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し
		for (int i = 0; i < m_iCount; ++i)
		{
			cArchiver_ << m_vecuiValue[i];
		}
	}
	else
	{
		m_vecuiValue.clear();
		m_vecuiValue.reserve(m_iCount);

		//読み出し
		for (int i = 0; i < m_iCount; ++i)
		{
			unsigned int uiValue;
			cArchiver_ >> uiValue;
			m_vecuiValue.pushBack(uiValue);
		}
	}
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::getValue -- すべての要素を取り出す
//
//	NOTES
//	すべての要素を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<unsigned int>&
//		unsigned intの配列
//
//	EXCEPTIONS
//	なし
//
const ModVector<unsigned int>&
UnsignedIntegerArrayData::
getValue() const
{
	return m_vecuiValue;
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::setValue -- すべての要素を設定する
//
//	NOTES
//	すべての要素を設定する
//
//	ARGUMENTS
//	const ModVector<unsigned int>& vecuiValue_
//		unsigned intの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
UnsignedIntegerArrayData::
setValue(
	const ModVector<unsigned int>& vecuiValue_)
{
	m_vecuiValue = vecuiValue_;
}

//	FUNCTION private
//	Common::UnsignedIntegerArrayData::copy_NotNull --
//		自分自身のコピーを作成する
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
UnsignedIntegerArrayData::copy_NotNull() const
{
	return new UnsignedIntegerArrayData(*this);
}

//	FUNCTION private
//	Common::UnsignedIntegerArrayData::cast_NotNull -- キャストする
//
//	NOTES
//	キャストする。
//
//	ARGUMENTS
//	Common::DataType::Type eType_
//		キャストするデータ型。
//
//	RETURN
//	Common::Data::Pointer
//		キャストしたデータ。
//
//	EXCEPTIONS
//	Exception::ClassCast
//		キャストに失敗した

Data::Pointer
UnsignedIntegerArrayData::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	switch (eType_) {
	case DataType::Integer:
	{
		IntegerArrayData* pArray = new IntegerArrayData;
		int n = getCount();
		for (int i = 0; i < n; ++i)
		{
			pArray->setElement(i, static_cast<int>(getElement(i)));
		}
		return pArray;
	}
	case DataType::UnsignedInteger:
		return copy();
	case DataType::String:
	{
		StringArrayData* pArray = new StringArrayData;
		int n = getCount();
		for (int i = 0; i < n; ++i)
		{
			ModUnicodeOstrStream ostr;
			ostr << getElement(i);
			pArray->setElement(i, ostr.getString());
		}
		return pArray;
	}
	case DataType::Null:
		return NullData::getInstance();
	default:
		break;
	}
	_TRMEISTER_THROW0(Exception::ClassCast);
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::getCount -- 要素数を取り出す
//
//	NOTES
//	要素数を取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		要素数
//
//	EXCEPTIONS
//	なし
//
int
UnsignedIntegerArrayData::
getCount() const
{
	return static_cast<int>(m_vecuiValue.getSize());
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::getElement -- 要素を取り出す
//
//	NOTES
//	要素を取り出す
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//
//	RETURN
//	unsigned int
//		その要素の値
//
//	EXCEPTIONS
//	なし
//
unsigned int
UnsignedIntegerArrayData::
getElement(int iIndex_) const
{
	return m_vecuiValue[iIndex_];
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::setElement -- 要素を設定する
//
//	NOTES
//	要素を設定する
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//	unsigned int uiValue_
//		要素
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
UnsignedIntegerArrayData::
setElement(int iIndex_,
											 unsigned int uiValue_)
{
	m_vecuiValue.reserve(iIndex_ + 1);

	while (iIndex_ >= getCount())
	{
		m_vecuiValue.pushBack(0);
	}
	m_vecuiValue[iIndex_] = uiValue_;
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::equals -- 比較
//
//	NOTES
//	等しいかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較するUnsignedIntegerArrayDataへのポインタ
//
//	RETURN
//	等しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
UnsignedIntegerArrayData::
equals(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {
		const UnsignedIntegerArrayData* pUnsignedIntegerArrayData =
			_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerArrayData*, pData_);

		if (getCount() == pUnsignedIntegerArrayData->getCount())
		{
			ModVector<unsigned int>::ConstIterator i;
			ModVector<unsigned int>::ConstIterator j;
			for (i = m_vecuiValue.begin(),
				j = pUnsignedIntegerArrayData->m_vecuiValue.begin();
				i != m_vecuiValue.end(); ++i, ++j)
			{
				if ((*i) != (*j)) break;
			}
			if (i == m_vecuiValue.end()) bResult = true;
		}
	}
	return bResult;
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::compareTo -- 大小比較
//
//	NOTES
//	大小比較
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較するUnsignedIntegerArrayDataへのポインタ
//
//	RETURN
//	自分の方が大きい場合は正の値、小さい場合は負の値、等しい場合は0
//
//	EXCEPTIONS
//	Exception::ClassCast
//		Common::UnsignedIntegerArrayDataへのキャストに失敗した
//
int
UnsignedIntegerArrayData::
compareTo(const Data* pData_) const
{
	int iResult = 0;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {
		const UnsignedIntegerArrayData* pUnsignedIntegerArrayData =
			_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerArrayData*, pData_);

		int iThisCount = getCount();
		int iOtherCount = pUnsignedIntegerArrayData->getCount();

		for (int i = 0; i < iThisCount; ++i)
		{
			if (i >= iOtherCount)
			{
				iResult = 1;
				break;
			}

			unsigned int uiThisElement = getElement(i);
			unsigned int uiOtherElement
				= pUnsignedIntegerArrayData->getElement(i);
			
			if (uiThisElement > uiOtherElement)
			{
				iResult = 1;
				break;
			}
			else if (uiThisElement < uiOtherElement)
			{
				iResult = -1;
				break;
			}
		}
		if (iResult == 0 && iThisCount < iOtherCount) iResult = -1;
	} else {
		//型が違うのでエラー
		_TRMEISTER_THROW0(Exception::ClassCast);
	}

	return iResult;
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::overlaps
//								-- 範囲が重なっているかどうかチェックする
//
//	NOTES
//	範囲が重なっているかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		要素数2のCommon::UnsignedIntegerArrayDataへのポインタ
//
//	RETURN
//	重なっている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
UnsignedIntegerArrayData::
overlaps(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {
		const UnsignedIntegerArrayData* pUnsignedIntegerArrayData =
			_SYDNEY_DYNAMIC_CAST(const UnsignedIntegerArrayData*, pData_);
		if (pUnsignedIntegerArrayData->getCount() >= 2
			&& getCount() >= 2)
		{
			unsigned int uiThisStart = getElement(0);
			unsigned int uiThisStop = getElement(1);
			unsigned int uiOtherStart = pUnsignedIntegerArrayData->getElement(0);
			unsigned int uiOtherStop = pUnsignedIntegerArrayData->getElement(1);
			if (uiThisStart > uiThisStop)
			{
				unsigned int v = uiThisStart;
				uiThisStart = uiThisStop;
				uiThisStop = v;
			}
			if (uiOtherStart > uiOtherStop)
			{
				unsigned int v = uiOtherStart;
				uiThisStart = uiThisStop;
				uiThisStop = v;
			}

			if (uiThisStart <= uiOtherStop && uiThisStop >= uiOtherStart)
				bResult = true;
		}
	}
	
	return bResult;
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData -- 包含しているかどうか
//
//	NOTES
//	iValue_が自分自身お要素に含まれているかどうか(inの逆)
//
//	ARGUMENTS
//	unsigned int uiValue_
//		値
//
//	RETURN
//	含まれている場合はtrue、それ以外の場合はfalse
//
//	EXCETPTIONS
//	なし
//
bool
UnsignedIntegerArrayData::
contains(unsigned int uiValue_) const
{
	bool bResult = false;
	for (ModVector<unsigned int>::ConstIterator i = m_vecuiValue.begin();
		 i != m_vecuiValue.end(); ++i)
	{
		if ((*i) == uiValue_)
		{
			bResult = true;
			break;
		}
	}
	return bResult;
}

//
//	FUNCTION public
//	Common::UnsignedIntergerArrayData::setDumpedValue
//		-- ダンプしたデータを設定する
//
//	NOTES
//
//	ARGUMENTS
//	ModSerialIO& cSerialIO_
//		ダンプしたデータを取り出すアーカイバー
//	ModSize uiSize_
//		ダンプ時のサイズ(バイト)
//
//	RETURN
//	ModSize
//		実際に書き出したサイズ(バイト)
//
//	EXCEPTIONS
//
ModSize
UnsignedIntegerArrayData::setDumpedValue(ModSerialIO& cSerialIO_,
										 ModSize uiSize_)
{
	; _TRMEISTER_ASSERT((uiSize_ % sizeof(unsigned int)) == 0);
	
	m_vecuiValue.clear();

	if (uiSize_)
	{
		m_vecuiValue.assign(uiSize_ / sizeof(unsigned int), 0);
		cSerialIO_.readSerial(&(*m_vecuiValue.begin()), uiSize_,
							  ModSerialIO::dataTypeIntegerArray);
	}

	// NULL 値でなくする
	setNull(false);

	return uiSize_;
}

//
//	FUNCTION private
//	Common::UnsignedIntegerArrayData::dumpValue_NotNull
//		-- データをダンプする
//
//	NOTES
//
//	ARGUMENTS
//	ModSerialIO& cSerialIO
//		データをダンプするアーカイバー
//
//	RETURN
//	ModSize
//		実際に書き出したサイズ(バイト)
//
//	EXCEPTIONS
//
ModSize
UnsignedIntegerArrayData::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	; _TRMEISTER_ASSERT(!isNull());

	ModSize size = m_vecuiValue.getSize() * sizeof(unsigned int);

	if (size > 0)
		cSerialIO_.writeSerial(&(*m_vecuiValue.begin()),
							   size,
							   ModSerialIO::dataTypeIntegerArray);

	return size;
}

//	FUNCTION private
//	Common::UnsignedIntegerArrayData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::UnsignedIntegerArrayDataクラスのクラスID
//
//	EXCEPTIONS
//	なし

int
UnsignedIntegerArrayData::getClassID_NotNull() const
{
	return ClassID::UnsignedIntegerArrayDataClass;
}

//	FUNCTION private
//	Common::UnsignedIntegerArrayData::print_NotNull -- 値を表示する
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

void
UnsignedIntegerArrayData::print_NotNull() const
{
	int n = 0;
	for (ModVector<unsigned int>::ConstIterator i = m_vecuiValue.begin();
		 i != m_vecuiValue.end(); ++i)
	{
		cout << "array[" << n++ << "]: "
			<< "UnsignedInteger: " << *i << endl;
	}
}

//	FUNCTION protected
//	Common::UnsignedIntegerArrayData::getDumpSize_NotNull -- ダンプサイズを得る
//
//	NOTES
//	ダンプサイズを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ダンプサイズ
//
//	EXCEPTIONS
ModSize
UnsignedIntegerArrayData::
getDumpSize_NotNull() const {
	; _TRMEISTER_ASSERT(!isNull());

	// !! 注意
	// ここでの値は各要素のダンプサイズの和に過ぎない.
	// dumpValue() を実装する際には再考せよ.

	return m_iCount * sizeof( unsigned int );
}

//	FUNCTION private
//	Common::UnsignedIntegerArrayData::getString_NotNull -- 文字列で取り出す
//
//	NOTES
//	文字列で取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

ModUnicodeString
UnsignedIntegerArrayData::getString_NotNull() const
{
	ModUnicodeOstrStream cStream;
	cStream << "{";
	for (ModVector<unsigned int>::ConstIterator i = m_vecuiValue.begin();
		 i != m_vecuiValue.end(); ++i)
	{
		if (i != m_vecuiValue.begin()) cStream << ",";
		cStream << *i;
	}
	cStream << "}";

	return ModUnicodeString(cStream.getString());
}

// FUNCTION private
//	Common::UnsignedIntegerArrayData::assign_NoCast -- 代入を行う(キャストなし)
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
UnsignedIntegerArrayData::
assign_NoCast(const Data& r)
{
	if (r.getType() == getType()
		&& r.getElementType() == getElementType()) {
		const UnsignedIntegerArrayData& data = _SYDNEY_DYNAMIC_CAST(const UnsignedIntegerArrayData&, r);
		const ModVector<unsigned int>& vecRhs = data.getValue();
		if (!data.isNull()) {
			setValue(vecRhs);
			return true;
		}
	}
	setNull();
	return false;
}

//
//	Copyright (c) 1999, 2000, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
