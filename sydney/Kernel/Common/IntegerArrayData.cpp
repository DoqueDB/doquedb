// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerArrayData -- Integer(32ビット)の配列をあらわすクラス
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
//	Common::IntegerArrayData::IntegerArrayData -- コンストラクタ(1)
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
IntegerArrayData::
IntegerArrayData()
: ArrayData(DataType::Integer)
{
}

//
//	FUNCTION public
//	Common::IntegerArrayData::IntegerArrayData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModVector<int>& veciValue_
//		intの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
IntegerArrayData::
IntegerArrayData(const ModVector<int>& veciValue_)
: ArrayData(DataType::Integer)
{
	setValue(veciValue_);
}

//
//	FUNCTION public
//	Common::IntegerArrayData::~IntegerArrayData -- デストラクタ
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
IntegerArrayData::
~IntegerArrayData()
{
}

// FUNCTION public
//	Common::IntegerArrayData::hashCode -- ハッシュコードを取り出す
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
IntegerArrayData::
hashCode() const
{
	if (isNull()) return 0;

	ModVector<int>::ConstIterator iterator = m_veciValue.begin();
	const ModVector<int>::ConstIterator last = m_veciValue.end();

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

//
//	FUNCTION private
//	Common::IntegerArrayData::serialize_NotNull -- シリアル化
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
IntegerArrayData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ArrayData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し
		for (int i = 0; i < m_iCount; ++i)
		{
			cArchiver_ << m_veciValue[i];
		}
	}
	else
	{
		m_veciValue.clear();
		m_veciValue.reserve(m_iCount);

		//読み出し
		for (int i = 0; i < m_iCount; ++i)
		{
			int iValue;
			cArchiver_ >> iValue;
			m_veciValue.pushBack(iValue);
		}
	}
}

//
//	FUNCTION public
//	Common::IntegerArrayData::getValue -- すべての要素を取り出す
//
//	NOTES
//	すべての要素を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<int>&
//		intの配列
//
//	EXCEPTIONS
//	なし
//
const ModVector<int>&
IntegerArrayData::
getValue() const
{
	return m_veciValue;
}

//
//	FUNCTION public
//	Common::IntegerArrayData::setValue -- すべての要素を設定する
//
//	NOTES
//	すべての要素を設定する
//
//	ARGUMENTS
//	const ModVector<int>& veciValue_
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
IntegerArrayData::
setValue(const ModVector<int>& veciValue_)
{
	m_veciValue = veciValue_;
}

//	FUNCTION private
//	Common::IntegerArrayData::copy_NotNull -- 自分自身のコピーを作成する
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
IntegerArrayData::copy_NotNull() const
{
	return new IntegerArrayData(*this);
}

//	FUNCTION private
//	Common::IntegerArrayData::cast_NotNull -- キャストする
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
IntegerArrayData::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	switch (eType_)
	{
	case DataType::Integer:
		return copy();
	case DataType::UnsignedInteger:
	{
		UnsignedIntegerArrayData* pArray = new UnsignedIntegerArrayData;
		int n = getCount();
		for (int i = 0; i < n; ++i)
		{
			pArray->setElement(i, static_cast<int>(getElement(i)));
		}
		return pArray;
	}
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
//	Common::IntegerArrayData::getCount -- 要素数を取り出す
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
IntegerArrayData::
getCount() const
{
	return static_cast<int>(m_veciValue.getSize());
}

//
//	FUNCTION public
//	Common::IntegerArrayData::getElement -- 要素を取り出す
//
//	NOTES
//	要素を取り出す
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//
//	RETURN
//	int
//		その要素の値
//
//	EXCEPTIONS
//	なし
//
int
IntegerArrayData::
getElement(int iIndex_) const
{
	return m_veciValue[iIndex_];
}

//
//	FUNCTION public
//	Common::IntegerArrayData::setElement -- 要素を設定する
//
//	NOTES
//	要素を設定する
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//	int iValue_
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
IntegerArrayData::
setElement(int iIndex_, int iValue_)
{
	m_veciValue.reserve(iIndex_ + 1);

	while (iIndex_ >= getCount())
	{
		m_veciValue.pushBack(0);
	}
	m_veciValue[iIndex_] = iValue_;
}

//
//	FUNCTION public
//	Common::IntegerArrayData::equals -- 比較
//
//	NOTES
//	等しいかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較するIntegerArrayDataへのポインタ
//
//	RETURN
//	等しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
IntegerArrayData::
equals(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {

		const IntegerArrayData* pIntegerArrayData =
			_SYDNEY_DYNAMIC_CAST(const IntegerArrayData*, pData_);

		if (getCount() == pIntegerArrayData->getCount())
		{
			ModVector<int>::ConstIterator i;
			ModVector<int>::ConstIterator j;
			for (i = m_veciValue.begin(),
				j = pIntegerArrayData->m_veciValue.begin();
				i != m_veciValue.end(); ++i, ++j)
			{
				if ((*i) != (*j)) break;
			}
			if (i == m_veciValue.end()) bResult = true;
		}
	}
	return bResult;
}

//
//	FUNCTION public
//	Common::IntegerArrayData::compareTo -- 大小比較
//
//	NOTES
//	大小比較
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較するIntegerArrayDataへのポインタ
//
//	RETURN
//	自分の方が大きい場合は正の値、小さい場合は負の値、等しい場合は0
//
//	EXCEPTIONS
//	Exception::ClassCast
//		Common::IntegerArrayDataへのキャストに失敗した
//
int
IntegerArrayData::
compareTo(const Data* pData_) const
{
	int iResult = 0;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {

		const IntegerArrayData* pIntegerArrayData =
			_SYDNEY_DYNAMIC_CAST(const IntegerArrayData*, pData_);

		int iThisCount = getCount();
		int iOtherCount = pIntegerArrayData->getCount();

		for (int i = 0; i < iThisCount; ++i)
		{
			if (i >= iOtherCount)
			{
				iResult = 1;
				break;
			}

			int iThisElement = getElement(i);
			int iOtherElement = pIntegerArrayData->getElement(i);
			
			if (iThisElement > iOtherElement)
			{
				iResult = 1;
				break;
			}
			else if (iThisElement < iOtherElement)
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
//	Common::IntegerArrayData::overlaps
//								-- 範囲が重なっているかどうかチェックする
//
//	NOTES
//	範囲が重なっているかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		要素数2のCommon::IntegerArrayDataへのポインタ
//
//	RETURN
//	重なっている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
IntegerArrayData::
overlaps(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {

		const IntegerArrayData* pIntegerArrayData =
			_SYDNEY_DYNAMIC_CAST(const IntegerArrayData*, pData_);

		if (pIntegerArrayData->getCount() >= 2
			&& getCount() >= 2)
		{
			int iThisStart = getElement(0);
			int iThisStop = getElement(1);
			int iOtherStart = pIntegerArrayData->getElement(0);
			int iOtherStop = pIntegerArrayData->getElement(1);
			if (iThisStart > iThisStop)
			{
				int v = iThisStart;
				iThisStart = iThisStop;
				iThisStop = v;
			}
			if (iOtherStart > iOtherStop)
			{
				int v = iOtherStart;
				iThisStart = iThisStop;
				iThisStop = v;
			}

			if (iThisStart <= iOtherStop && iThisStop >= iOtherStart)
				bResult = true;
		}
	}
	
	return bResult;
}

//
//	FUNCTION public
//	Common::IntegerArrayData -- 包含しているかどうか
//
//	NOTES
//	iValue_が自分自身お要素に含まれているかどうか(inの逆)
//
//	ARGUMENTS
//	int iValue_
//		値
//
//	RETURN
//	含まれている場合はtrue、それ以外の場合はfalse
//
//	EXCETPTIONS
//	なし
//
bool
IntegerArrayData::
contains(int iValue_) const
{
	bool bResult = false;
	for (ModVector<int>::ConstIterator i = m_veciValue.begin();
		 i != m_veciValue.end(); ++i)
	{
		if ((*i) == iValue_)
		{
			bResult = true;
			break;
		}
	}
	return bResult;
}

//	FUNCTION private
//	Common::IntegerArrayData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::IntegerArrayDataクラスのクラスID
//
//	EXCEPTIONS
//	なし

int
IntegerArrayData::getClassID_NotNull() const
{
	return ClassID::IntegerArrayDataClass;
}

//	FUNCTION private
//	Common::IntegerArrayData::print_NotNull -- 値を表示する
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
IntegerArrayData::print_NotNull() const
{
	int n = 0;
	for (ModVector<int>::ConstIterator i = m_veciValue.begin();
		 i != m_veciValue.end(); ++i)
	{
		cout << "array[" << n++ << "]: "
			<< "integer: " << *i << endl;
	}
}

//	FUNCTION protected
//	Common::IntegerArrayData::getDumpSize_NotNull -- ダンプサイズを得る
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
IntegerArrayData::
getDumpSize_NotNull() const {
	; _TRMEISTER_ASSERT(!isNull());

	// !! 注意
	// ここでの値は各要素のダンプサイズの和に過ぎない.
	// dumpValue() を実装する際には再考せよ.

	return m_iCount * sizeof( int );
}

//	FUNCTION private
//	Common::IntegerArrayData::getString_NotNull -- 文字列で取り出す
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
IntegerArrayData::getString_NotNull() const
{
	ModUnicodeOstrStream cStream;
	cStream << "{";
	for (ModVector<int>::ConstIterator i = m_veciValue.begin();
		 i != m_veciValue.end(); ++i)
	{
		if (i != m_veciValue.begin()) cStream << ",";
		cStream << *i;
	}
	cStream << "}";

	return ModUnicodeString(cStream.getString());
}

// FUNCTION private
//	Common::IntegerArrayData::assign_NoCast -- 代入を行う(キャストなし)
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
IntegerArrayData::
assign_NoCast(const Data& r)
{
	if (r.getType() == getType()
		&& r.getElementType() == getElementType()) {
		const IntegerArrayData& data = _SYDNEY_DYNAMIC_CAST(const IntegerArrayData&, r);
		const ModVector<int>& vecRhs = data.getValue();
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
