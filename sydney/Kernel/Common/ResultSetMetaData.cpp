// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSetMetaData.cpp --
// 
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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
#include "Common/ResultSetMetaData.h"
#include "Common/ClassID.h"
#include "Exception/NullNotAllowed.h"

#include "ModUnicodeOstrStream.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::ResultSetMetaData::ResultSetMetaData -- コンストラクタ
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
//
ResultSetMetaData::ResultSetMetaData()
	: ArrayData(DataType::ColumnMetaData)
{
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::~ResultSetMetaData -- デストラクタ
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
//
ResultSetMetaData::~ResultSetMetaData()
{
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::getCount -- 要素数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		要素数
//
//	EXCEPTIONS
//
int
ResultSetMetaData::getCount() const
{
	return static_cast<int>(m_vecColumnMetaData.getSize());
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::getCapacity -- 現在のバッファの容量を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		現在のバッファ容量
//
//	EXCEPTIONS
//
int
ResultSetMetaData::getCapacity() const
{
	return static_cast<int>(m_vecColumnMetaData.getCapacity());
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::overlaps
//		-- 配列要素が2の時にその範囲が重なっているかどうかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較対象
//
//	RETURN
//	bool
//		ResultSetMetaDataの比較はできないので、常にfalse
//
//	EXCEPTIONS
//
bool
ResultSetMetaData::overlaps(const Data* pData_) const
{
	return false;
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::getElement -- 要素を得る
//
//	NOTES
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//
//	RETURN
//	const Common::ColumnMetaData&
//		カラムメタデータ
//
//	EXCEPTIONS
//
ColumnMetaData&
ResultSetMetaData::getElement(int iIndex_)
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_vecColumnMetaData[iIndex_];
}

const ColumnMetaData&
ResultSetMetaData::getElement(int iIndex_) const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_vecColumnMetaData[iIndex_];
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::setElement -- 要素を設定する
//
//	NOTES
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//	const Common::ColumnMetaData& cValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::setElement(int iIndex_, const ColumnMetaData& cValue_)
{
	if (getCapacity() < (iIndex_ + 1))
		reserve(iIndex_ + 1);
	for (int i = getCount(); i < iIndex_; ++i)
		pushBack(ColumnMetaData());
	pushBack(cValue_);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::clear -- クリアする
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
//
void
ResultSetMetaData::clear()
{
	m_vecColumnMetaData.clear();
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::reserve -- 配列領域を確保する
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
//
void
ResultSetMetaData::reserve(int n)
{
	m_vecColumnMetaData.reserve(n);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::assign -- 要素を設定する
//
//	NOTES
//
//	ARGUMENTS
//	int n
//		設定する要素数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::assign(int n)
{
	m_vecColumnMetaData.assign(n);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::getValue -- すべての要素を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<ColumnMetaData>&
//		すべての要素
//
//	EXCEPTIONS
//
const ModVector<ColumnMetaData>&
ResultSetMetaData::getValue() const
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return m_vecColumnMetaData;
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::operator[] -- 指定要素を得る
//
//	NOTES
//
//	ARGUMENTS
// 	int index_
//		要素番号
//
//	RETURN
//	Common::ColumnMetaData&
//		指定要素のカラムメタデータへの参照
//
//	EXCEPTIONS
//
ColumnMetaData&
ResultSetMetaData::operator[] (int index_)
{
	
	return getElement(index_);
}

const ColumnMetaData&
ResultSetMetaData::operator[] (int index_) const
{
	return getElement(index_);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::pushFront -- 配列の先頭に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::ColumnMetaData& cElement_
//		挿入する要素
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::pushFront(const ColumnMetaData& cElement_)
{
	m_vecColumnMetaData.pushFront(cElement_);
	setNull(false);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::pushBack -- 配列の末尾に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::ColumnMetaData& cElement_
//		挿入する要素
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::pushBack(const ColumnMetaData& cElement_)
{
	m_vecColumnMetaData.pushBack(cElement_);
	setNull(false);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::popFront -- 配列の先頭要素を削除する
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
//
void
ResultSetMetaData::popFront()
{
	m_vecColumnMetaData.popFront();
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::popBack -- 配列の末尾要素を削除する
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
//
void
ResultSetMetaData::popBack()
{
	m_vecColumnMetaData.popBack();
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::erase -- 指定要素を消す
//
//	NOTES
//
//	ARGUMENTS
//	Common::ResultSetMetaData::Iterator position_
//		消す要素のイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::erase(Iterator position_)
{
	m_vecColumnMetaData.erase(position_);
}

//
//	FUNCTION public
//	Common::ResultSetMetaData::erase -- 指定要素を消す
//
//	NOTES
//	first_以上 end_未満の要素を削除する
//
//	ARGUMENTS
//	Common::ResutlSetMetaData::Iterator first_
//		消す先頭要素のイテレータ
//	const Common::ResultSetMetaData::Iterator end_
//		消す末尾要素のイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::erase(Iterator first_, const Iterator end_)
{
	m_vecColumnMetaData.erase(first_, end_);
}

//
//	FUNCTION protected
//	Common::ResultSetMetaData::getClassID -- クラスIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//
int
ResultSetMetaData::getClassID_NotNull() const
{
	return Common::ClassID::ResultSetMetaDataClass;
}

//
//	FUNCTION protected
//	Common::ResultSetMetaData::serialize -- シリアル化
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& archiver_
//		アーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ResultSetMetaData::serialize_NotNull(ModArchive& archiver_)
{
	ArrayData::serialize_NotNull(archiver_);
	
	if (archiver_.isStore())
	{
		Iterator i = begin();
		for (; i != end(); ++i)
			archiver_ << *i;
	}
	else
	{
		clear();
		assign(m_iCount);
		Iterator i = begin();
		for (; i != end(); ++i)
			archiver_ >> *i;
	}
}

//
//	FUNCTION protected
//	Common::ResultSetMetaData::copy_NotNull -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data::Pointer
//		コピー
//
//	EXCEPTIONS
//
Data::Pointer
ResultSetMetaData::copy_NotNull() const
{
	ResultSetMetaData* pCopy = new ResultSetMetaData;
	Data::Pointer result = pCopy;
	pCopy->reserve(getCount());
	for (ConstIterator i = begin(); i != end(); ++i)
		pCopy->pushBack(*i);
	return result;
}

//
//	FUNCTION protected
//	Common::ResultSetMetaData::getString_NotNull -- 文字列の形式で値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUnicodeString
//		文字列形式
//
//	EXCEPTIONS
//
ModUnicodeString
ResultSetMetaData::getString_NotNull() const
{
	ModUnicodeOstrStream cStream;
	cStream << "{";
	for (ConstIterator i = begin(); i != end(); ++i)
	{
		if (i != begin()) cStream << ",";
		cStream << (*i).toString();
	}
	cStream << "}";

	return ModUnicodeString(cStream.getString());
}

//
//	Copyright (c) 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
