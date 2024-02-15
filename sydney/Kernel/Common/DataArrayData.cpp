// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataArrayData.cpp -- データの配列をあらわすデータ
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
#include "Common/DataArrayData.h"
#include "Common/InputArchive.h"
#include "Common/NullData.h"
#include "Common/OutputArchive.h"
#include "Common/SQLData.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/NullNotAllowed.h"

#include "ModAutoPointer.h"
#include "ModHasher.h"
#include "ModUnicodeOstrStream.h"

#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{

namespace _DataArrayData
{
	// 0 を指すオブジェクトポインタ
	DataArrayData::Pointer	_nullPointer(static_cast<const Common::Data*>(0));
}

}

// FUNCTION public
//	Common::DataArrayData::hashCode -- ハッシュコードを取り出す
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
DataArrayData::
hashCode() const
{
	if (isNull()) return 0;

	ModVector<Pointer>::ConstIterator iterator = m_vecpValue.begin();
	const ModVector<Pointer>::ConstIterator last = m_vecpValue.end();

	ModSize hashValue = 0;
	ModSize g;
	while (iterator != last) {
		hashValue <<= 4;
		hashValue += (*iterator)->hashCode();
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
		++iterator;
	}
	return hashValue;
}

//	FUNCTION private
//	Common::DataArrayData::serialize_NotNull -- シリアル化
//
//	NOTES
//	シリアル化
//
//	ARGUMENTS
//	cArchiver_	アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
DataArrayData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ArrayData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し
		OutputArchive& cOutput
			= _SYDNEY_DYNAMIC_CAST(OutputArchive&, cArchiver_);
		for (int i = 0; i < m_iCount; ++i)
		{
			cOutput.writeObject(m_vecpValue[i].get());
		}
	}
	else
	{
		setCount(m_iCount);

		//読み出し
		InputArchive& cInput
			= _SYDNEY_DYNAMIC_CAST(InputArchive&, cArchiver_);
		for (int i = 0; i < m_iCount; ++i)
		{
			Pointer p = getElement(i);
			Data* pData = dynamic_cast<Data*>(cInput.readObject(p.get()));
			if (pData != p.get())
				setElement(i, pData);
		}
	}
}

//
//	FUNCTION public
//	Common::DataArrayData::setCount -- 要素数を設定する
//
//	NOTES
//	もし現在の領域が必要なサイズ分より小さければnullポインターを割り当て、
//	大きければ削除する
//
//	ARGUMENTS
//	int n
//		必要なサイズ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataArrayData::setCount(int n)
{
	if (m_vecpValue.getCapacity() < static_cast<ModSize>(n))
		m_vecpValue.reserve(n);
	
	if (getCount() > n)
	{
		m_vecpValue.erase(m_vecpValue.begin() + n, m_vecpValue.end());
	}
	else if (getCount() < n)
	{
		int i = getCount();
		for (; i < n; ++i)
			pushBack(Pointer());
	}
}

//
//	FUNCTION public
//	Common::DataArrayData::getElement -- 要素を取り出す
//
//	NOTES
//	要素を取り出す
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//
//	RETURN
//	Common::DataArrayData::Pointer
//		その要素番号のオブジェクトポインタ
//
//	EXCEPTIONS
//	なし
//

const Common::DataArrayData::Pointer&
DataArrayData::
getElement(int iIndex_)
{
	; _TRMEISTER_ASSERT(iIndex_ < getCount());
	return m_vecpValue[iIndex_];
}

//
//	FUNCTION public
//	Common::DataArrayData::getElement -- 要素を取り出す
//
//	NOTES
//	要素を取り出す
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//
//	RETURN
//	Common::DataArrayData::Pointer
//		その要素番号のオブジェクトポインタ
//
//	EXCEPTIONS
//	なし
//

const Common::DataArrayData::Pointer&
DataArrayData::
getElement(int iIndex_) const
{
	; _TRMEISTER_ASSERT(iIndex_ < getCount());
	return m_vecpValue[iIndex_];
}

//
//	FUNCTION public
//	Common::DataArrayData::setElement -- 要素を設定する
//
//	NOTES
//	要素を設定する
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//	Common::DataArrayData::Pointer pValue_
//		要素
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

void
DataArrayData::
setElement(int iIndex_, const Pointer& pValue_)
{
	if (m_vecpValue.getCapacity() < static_cast<ModSize>(iIndex_ + 1)) {
		m_vecpValue.reserve(iIndex_ + 1);
	}

	for (int i = getCount(); i <= iIndex_; ++i)
		m_vecpValue.pushBack(_DataArrayData::_nullPointer);

	m_vecpValue[iIndex_] = pValue_;
	setNull(false);
}

// FUNCTION public
//	Common::DataArrayData::distinct -- DISTINCTか調べる
//
// NOTES
//
// ARGUMENTS
//	const Data* r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
DataArrayData::
distinct(const Data* r) const
{
	// array is not distinct when both are null or all elements are not distinct
	if (isNull())
		return !(r->isNull());
	else if (r->isNull())
		return true;

	if (r && r->getType() == getType()
		&& r->getElementType() == getElementType()) {

		const DataArrayData* pDataArrayData =
			_SYDNEY_DYNAMIC_CAST(const DataArrayData*, r);
		; _SYDNEY_ASSERT(pDataArrayData);

		int n = getCount();
		if (n == pDataArrayData->getCount()) {
			for (int i = 0; i < n; ++i) {
				// if any element is distinct, result is true
				if (getElement(i)->distinct(pDataArrayData->getElement(i).get())) {
					return true;
				}
			}
			return false;
		}
		// if element count is different, those are distinct
		return true;
	}
	_SYDNEY_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	Common::DataArrayData::overlaps -- 範囲が重なっているかどうかチェックする
//	
//	NOTES
//	範囲が重なっているかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		要素数2のデータ配列データ
//
//	RETURN
//	重なっている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
DataArrayData::
overlaps(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {

		const DataArrayData* pDataArrayData =
			_SYDNEY_DYNAMIC_CAST(const DataArrayData*, pData_);

		if (pDataArrayData->getCount() >= 2
			&& getCount() >= 2)
		{
			const Data* pThisStart = getElement(0).get();
			const Data* pThisStop = getElement(1).get();
			const Data* pOtherStart = pDataArrayData->getElement(0).get();
			const Data* pOtherStop = pDataArrayData->getElement(1).get();
			if (pThisStart->compareTo(pThisStop) > 0)
			{
				const Data* pData = pThisStart;
				pThisStart = pThisStop;
				pThisStop = pData;
			}
			if (pOtherStart->compareTo(pOtherStop) > 0)
			{
				const Data* pData = pOtherStart;
				pOtherStart = pOtherStop;
				pOtherStop = pData;
			}

			if (pThisStart->compareTo(pOtherStop) <= 0
				&& pThisStop->compareTo(pOtherStart) >= 0)
			{
				bResult = true;
			}
		}
	}

	return bResult;
}

// FUNCTION public
//	Common::DataArrayData::connect -- 連結する
//
// NOTES
//
// ARGUMENTS
//	const DataArrayData* pArrayData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
DataArrayData::
connect(const DataArrayData* pArrayData_)
{
	; _TRMEISTER_ASSERT(pArrayData_);

	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	if (pArrayData_->isNull())
		_TRMEISTER_THROW0(Exception::BadArgument);

	connect_NotNull(pArrayData_);
}

//
//	FUNCTION public
//	Common::DataArrayData::contains -- 包含しているかどうか
//
//	NOTES
//	引数pData_が自分自身の要素に含まれているかどうか(inの逆)
//
//	ARGUMENTS
//	const Common::Data* pData_
//		データ
//
//	RETURN
//	含まれている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
DataArrayData::
contains(const Data* pData_) const
{
	bool bResult = false;
	for (ModVector<Pointer>::ConstIterator i = m_vecpValue.begin();
		 i != m_vecpValue.end(); ++i)
	{
		if ((*i).get())
		{
			bResult = (*i)->equals(pData_);
			if (bResult == true)
				break;
		}
	}
	return bResult;
}

//	FUNCTION private
//	Common::DataArrayData::getClassID_NotNull -- クラス ID を得る
//
//	NOTES
//	クラス ID を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		DataArrayData クラスのクラス ID
//
//	EXCEPTIONS
//	なし

int
DataArrayData::getClassID_NotNull() const
{
	return ClassID::DataArrayDataClass;
}

//	FUNCTION private
//	Common::DataArrayData::print_NotNull -- 値を表示する
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
DataArrayData::print_NotNull() const
{
	int n = 0;
	for (ModVector<Pointer>::ConstIterator i = m_vecpValue.begin();
		 i != m_vecpValue.end(); ++i)
	{
		cout << "array[" << n++ << "]: ";
		if ( i->get() != 0 )
		{
			(*i)->print();
		}
		else
		{
			cout << "0" << endl;
		}
	}
}

// FUNCTION private
//	Common::DataArrayData::connect_NotNull -- 末尾にデータをつなげる
//
// NOTES
//
// ARGUMENTS
//	const DataArrayData* pArrayData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
DataArrayData::
connect_NotNull(const DataArrayData* pArrayData_)
{
	m_vecpValue.insert(m_vecpValue.end(),
					   pArrayData_->m_vecpValue.begin(),
					   pArrayData_->m_vecpValue.end());
}

//	FUNCTION private
//	Common::DataArrayData::getString_NotNull -- 文字列で取り出す
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
//	なし

ModUnicodeString
DataArrayData::getString_NotNull() const
{
	ModUnicodeOstrStream cStream;
	cStream << "{";
	for (ModVector<Pointer>::ConstIterator i = m_vecpValue.begin();
		 i != m_vecpValue.end(); ++i)
	{
		if (i != m_vecpValue.begin()) cStream << ",";
		
		if (i->get() != 0)
		{
			cStream << (*i)->getString();
		}
		else
		{
			cStream << "(0)";
		}
	}
	cStream << "}";

	return ModUnicodeString(cStream.getString());
}

//
//	FUNCTION public
//	Common::DataArrayData::equals_NoCast -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		データ配列データへのポインタ
//
//	RETURN
//	等しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
DataArrayData::
equals_NoCast(const Data& rData_) const
{
	bool bResult = false;
	if (rData_.getType() == getType()
		&& rData_.getElementType() == getElementType()) {

		const DataArrayData& cDataArrayData =
			_SYDNEY_DYNAMIC_CAST(const DataArrayData&, rData_);

		if (getCount() == cDataArrayData.getCount())
		{
			ModVector<Pointer>::ConstIterator i;
			ModVector<Pointer>::ConstIterator j;
			for (i = m_vecpValue.begin(),
				 j = cDataArrayData.m_vecpValue.begin();
				 i != m_vecpValue.end(); ++i, ++j)
			{
				if (((*i).get() == 0 || (*j).get() == 0)
					&& !((*i).get() == 0 && (*j).get() == 0))
					break;
				if ((*i).get() && (*j).get() && (*i)->equals((*j).get()) == false)
					break;
			}
			if (i == m_vecpValue.end()) bResult = true;
		}
	}
	return bResult;
}

//
//	FUNCTION public
//	Common::DataArrayData::compareTo_NoCast -- 大小比較
//
//	NOTES
//	大小比較
//
//	ARGUMENTS
//	const Common::Data& rData_
//		データ配列データへのポインタ
//
//	RETURN
//	自分の方が大きい場合は正の値、小さい場合は負の値、等しい場合は0
//
//	EXCEPTIONS
//	Exception::ClassCast
//		Common::DataArrayDataへのキャストに失敗した
//
int
DataArrayData::
compareTo_NoCast(const Data& rData_) const
{
	int iResult = 0;

	if (rData_.getType() == getType()
		&& rData_.getElementType() == getElementType()) {

		const DataArrayData& cDataArrayData =
			_SYDNEY_DYNAMIC_CAST(const DataArrayData&, rData_);

		int iThisCount = getCount();
		int iOtherCount = cDataArrayData.getCount();

		for (int i = 0; i < iThisCount; ++i)
		{
			if (i >= iOtherCount)
			{
				iResult = 1;
				break;
			}

			const Data* pThisElement = getElement(i).get();
			const Data* pOtherElement = cDataArrayData.getElement(i).get();

			if (pThisElement == 0 && pOtherElement != 0)
			{
				iResult = -1;
				break;
			}

			if (pThisElement != 0 && pOtherElement == 0)
			{
				iResult = 1;
				break;
			}

			if (pThisElement != 0 && pOtherElement != 0)
			{
				iResult = pThisElement->compareTo(pOtherElement);
				if (iResult != 0)
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

// FUNCTION public
//	Common::DataArrayData::assign_NoCast -- 代入を行う
//
// NOTES
//
// ARGUMENTS
//	const Data* r
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
DataArrayData::
assign_NoCast(const Data& r)
{
	if (r.getType() == getType()
		&& r.getElementType() == getElementType()) {
		const DataArrayData& data = _SYDNEY_DYNAMIC_CAST(const DataArrayData&, r);
		const ModVector<Pointer>& vecRhs = data.getValue();
		if (!data.isNull()) {
			setValue(vecRhs);
			return true;
		}
	}
	setNull();
	return false;
}

//	FUNCTION private
//	Common::DataArrayData::isApplicable_NotNull --
//		付加機能を適用可能かを得る
//
//	NOTES
//		自身が持っているデータのうちひとつでも
//		isApplicable==trueならtrueを返す
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
DataArrayData::isApplicable_NotNull(Function::Value iFunction_)
{
	bool bResult = false;
	for (ModVector<Pointer>::ConstIterator i = m_vecpValue.begin();
		 i != m_vecpValue.end(); ++i)
	{
		if ((*i).get() && (*i)->isApplicable(iFunction_)) {
			bResult = true;
			break;
		}
	}
	return bResult;
}

//	FUNCTION private
//	Common::BinaryData::apply_NotNull --
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
DataArrayData::apply_NotNull(Function::Value iFunction_)
{
	ModAutoPointer<DataArrayData> pResult = new DataArrayData;
	pResult->reserve(m_vecpValue.getSize());

	for (ModVector<Pointer>::ConstIterator i = m_vecpValue.begin();
		 i != m_vecpValue.end(); ++i)
	{
		if ((*i).get() && (*i)->isApplicable(iFunction_)) {
			pResult->pushBack((*i)->apply(iFunction_));

		} else {
			pResult->pushBack(*i);
		}
	}
	
	return pResult.release();
}

//
//	FUNCTION private
//	Common::DataArrayData::setCopyValue -- 要素をコピーして設定する
//
//	NOTES
//	要素をコピーして設定する。
//
//	ARGUMENTS
//	const ModVector<Common::DataArrayData::Pointer>& vecpValue_
//		コピーするデータの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DataArrayData::
setCopyValue(const ModVector<Pointer>& vecpValue_)
{
	clear();
	m_vecpValue.reserve(vecpValue_.getSize());

	for (ModVector<Pointer>::ConstIterator i = vecpValue_.begin();
		 i != vecpValue_.end(); ++i)
	{
		if ((*i).get()) m_vecpValue.pushBack((*i)->copy());
		else m_vecpValue.pushBack(Pointer());
	}
}

//	FUNCTION private
//	Common::DataArrayData::getDumpSize_NotNull -- ダンプサイズを得る
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
DataArrayData::
getDumpSize_NotNull() const {
	; _TRMEISTER_ASSERT(!isNull());

	// !! 注意
	// ここでの値は各要素のダンプサイズの和に過ぎない.
	// dumpValue() を実装する際には再考せよ.

	ModSize iDumpSize = 0;
//	ModSize iNullDataSize = NullData::getInstance()->getDumpSize();	// NOT SUPPORTED
;;	ModSize iNullDataSize = 0;	// 暫定値

	if (int n = getCount()) {
		for( int i = 0; i < n; ++i ){
			Common::Data::Pointer pCommonData = getElement( i );

			iDumpSize += ( pCommonData->isNull() ) ?
				iNullDataSize :
				pCommonData->getDumpSize();
		}
	}

	return iDumpSize;
}

// データに対応するSQLDataを得る
//virtual
bool
DataArrayData::
getSQLTypeByValue(SQLData& cResult_)
{
	// 要素がすべて同じ型であればその型を使う
	DataType::Type eElementType = DataType::Data;
	if (int n = getCount()) {
		for (int i = 0; i < n; ++i) {
			Common::Data::Pointer pElement = getElement(i);
			if (!pElement->isNull()) {
				if (eElementType == DataType::Data)
					eElementType = pElement->getType();
				else
					if (eElementType != pElement->getType())
						// 異なる型の要素があれば失敗
						return false;
			}
		}
		// DataTypeから型を作り、maxCardinalityに要素数をセットする
		if (Data::getSQLType(eElementType, cResult_)) {
			cResult_.setMaxCardinality(n);
			return true;
		}
	}
	// ここに来たら失敗
	return false;
}

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
