// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StringArrayData -- 文字列の配列をあらわすクラス
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
#include "Common/StringArrayData.h"
#include "Common/UnicodeString.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "Exception/ClassCast.h"

#include "ModHasher.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeCharTrait.h"
#include "ModMessage.h"
#include <iostream>
using namespace std;

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public
//	Common::StringArrayData::StringArrayData -- コンストラクタ(1)
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
StringArrayData::
StringArrayData()
: ArrayData(DataType::String)
{
}

//
//	FUNCTION public
//	Common::StringArrayData::StringArrayData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& veccValue_
//		intの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
StringArrayData::
StringArrayData(
	const ModVector<ModUnicodeString>& veccValue_)
: ArrayData(DataType::String)
{
	setValue(veccValue_);
}

//
//	FUNCTION public
//	Common::StringArrayData::~StringArrayData -- デストラクタ
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
StringArrayData::
~StringArrayData()
{
}

// FUNCTION public
//	Common::StringArrayData::hashCode -- ハッシュコードを取り出す
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
StringArrayData::
hashCode() const
{
	if (isNull()) return 0;

	ModVector<ModUnicodeString>::ConstIterator iterator = m_veccValue.begin();
	const ModVector<ModUnicodeString>::ConstIterator last = m_veccValue.end();

	ModSize hashValue = 0;
	ModSize g;
	while (iterator != last) {
		hashValue <<= 4;
		hashValue += ModUnicodeStringHasher()(*iterator);
		if (g = hashValue & ((ModSize) 0xf << (ModSizeBits - 4))) {
			hashValue ^= g >> (ModSizeBits - 8);
			hashValue ^= g;
		}
		++iterator;
	}
	return hashValue;
}

//	FUNCTION private
//	Common::StringArrayData::serialize_NotNull -- シリアル化
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
StringArrayData::serialize_NotNull(ModArchive& cArchiver_)
{
	; _TRMEISTER_ASSERT(!isNull());

	ArrayData::serialize_NotNull(cArchiver_);

	if (cArchiver_.isStore())
	{
		//書き出し
		for (int i = 0; i < m_iCount; ++i)
		{
			cArchiver_ << m_veccValue[i];
		}
	}
	else
	{
		m_veccValue.clear();
		m_veccValue.reserve(m_iCount);

		//読み出し
		for (int i = 0; i < m_iCount; ++i)
		{
			ModUnicodeString cstrValue;
			cArchiver_ >> cstrValue;
			m_veccValue.pushBack(cstrValue);
		}
	}
}

//
//	FUNCTION public
//	Common::StringArrayData::getValue -- すべての要素を取り出す
//
//	NOTES
//	すべての要素を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<ModUnicodeString>&
//		intの配列
//
//	EXCEPTIONS
//	なし
//
const ModVector<ModUnicodeString>&
StringArrayData::
getValue() const
{
	return m_veccValue;
}

//
//	FUNCTION public
//	Common::StringArrayData::setValue -- すべての要素を設定する
//
//	NOTES
//	すべての要素を設定する
//
//	ARGUMENTS
//	const ModVector<ModUnicodeString>& veccValue_
//		文字列の配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
StringArrayData::
setValue(const ModVector<ModUnicodeString>& veccValue_)
{
	m_veccValue = veccValue_;
}

//	FUNCTION private
//	Common::StringArrayData::copy_NotNull -- 自分自身のコピーを作成する
//
//	NOTES
//	自分自身のコピーを作成する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data*
//		自分自身のコピー
//
//	EXCEPTIONS

Data::Pointer
StringArrayData::copy_NotNull() const
{
	return new StringArrayData(*this);
}

//	FUNCTION private
//	Common::StringArrayData::cast_NotNull -- キャストする
//
//	NOTES
//	キャストする。
//
//	ARGUMENTS
//	Common::DataType::Type eType_
//		キャストするデータ型。
//
//	RETURN
//	Common::Data*
//		キャストしたデータ。
//
//	EXCEPTIONS
//	Exception::ClassCast
//		キャストに失敗した

Data::Pointer
StringArrayData::cast_NotNull(DataType::Type eType_, bool bForAssign_ /* = false */) const
{
	switch (eType_)
	{
	case DataType::Integer:
		{
			IntegerArrayData* pArray = new IntegerArrayData;
			int n = getCount();
			for (int i = 0; i < n; ++i)
			{
				pArray->setElement(i, ModUnicodeCharTrait::toInt(getElement(i)));
			}
			return pArray;
		}
	case DataType::UnsignedInteger:
		{
			UnsignedIntegerArrayData* pArray = new UnsignedIntegerArrayData;
			int n = getCount();
			for (int i = 0; i < n; ++i)
			{
				// ModSize は unsigned int なのでこれでいい。

				// Urgent Modify
				// ModUnicodeCharTrait が toModSize() をサポートしていないため
				// toInt() で代用
				// pArray->setElement(i, getElement(i).toModSize());
				pArray->setElement(i, ModUnicodeCharTrait::toInt(getElement(i)));
			}
			return pArray;
		}
	case DataType::String:
		return copy();
	}
	_TRMEISTER_THROW0(Exception::ClassCast);
}

//
//	FUNCTION public
//	Common::StringArrayData::getCount -- 要素数を取り出す
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
StringArrayData::
getCount() const
{
	return static_cast<int>(m_veccValue.getSize());
}

//
//	FUNCTION public
//	Common::StringArrayData::getElement -- 要素を取り出す
//
//	NOTES
//	要素を取り出す
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//
//	RETURN
//	const ModUnicodeString&
//		その要素の値
//
//	EXCEPTIONS
//	なし
//
const ModUnicodeString&
StringArrayData::
getElement(int iIndex_) const
{
	return m_veccValue[iIndex_];
}

//
//	FUNCTION public
//	Common::StringArrayData::setElement -- 要素を設定する
//
//	NOTES
//	要素を設定する
//
//	ARGUMENTS
//	int iIndex_
//		要素番号
//	const ModUnicodeString& cstrValue_
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
StringArrayData::
setElement(int iIndex_, const ModUnicodeString& cstrValue_)
{
	m_veccValue.reserve(iIndex_ + 1);

	while (iIndex_ >= getCount())
	{
		m_veccValue.pushBack(ModUnicodeString());
	}
	m_veccValue[iIndex_] = cstrValue_;
}

//
//	FUNCTION public
//	Common::StringArrayData::equals -- 比較
//
//	NOTES
//	等しいかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較するStringArrayDataへのポインタ
//
//	RETURN
//	等しい場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
StringArrayData::
equals(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {
		const StringArrayData* pStringArrayData =
			_SYDNEY_DYNAMIC_CAST(const StringArrayData*, pData_);

		if (getCount() == pStringArrayData->getCount())
		{
			ModVector<ModUnicodeString>::ConstIterator i;
			ModVector<ModUnicodeString>::ConstIterator j;
			for (i = m_veccValue.begin(),
				j = pStringArrayData->m_veccValue.begin();
				i != m_veccValue.end(); ++i, ++j)
			{
				if ((*i) != (*j)) break;
			}
			if (i == m_veccValue.end()) bResult = true;
		}
	}
	return bResult;
}

//
//	FUNCTION public
//	Common::StringArrayData::compareTo -- 大小比較
//
//	NOTES
//	大小比較
//
//	ARGUMENTS
//	const Common::Data* pData_
//		比較するStringArrayDataへのポインタ
//
//	RETURN
//	自分の方が大きい場合は正の値、小さい場合は負の値、等しい場合は0
//
//	EXCEPTIONS
//	Exception::ClassCast
//		Common::StringArrayDataへのキャストに失敗した
//
int
StringArrayData::
compareTo(const Data* pData_) const
{
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {

		const StringArrayData* pStringArrayData = 
			_SYDNEY_DYNAMIC_CAST(const StringArrayData*, pData_);

		int iThisCount  = getCount();
		int iOtherCount = pStringArrayData->getCount();

		int iResult = 0;
		for (int i = 0; i < iThisCount; ++i) {
			if (i >= iOtherCount)
			{
				iResult = 1;
				break;
			}
			const ModUnicodeString cThisElement = getElement(i);
			const ModUnicodeString cOtherElement = pStringArrayData->getElement(i);
			
			iResult = cThisElement.compare(cOtherElement);
			if (iResult != 0)
				break;
		}
		if (iResult == 0 && iThisCount < iOtherCount) iResult = -1;

		return iResult;

	} else {
		//型が違うのでエラー
		throw Exception::ClassCast(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION public
//	Common::StringArrayData::overlaps
//								-- 範囲が重なっているかどうかチェックする
//
//	NOTES
//	範囲が重なっているかどうかチェックする
//
//	ARGUMENTS
//	const Common::Data* pData_
//		要素数2のCommon::StringArrayDataへのポインタ
//
//	RETURN
//	重なっている場合はtrue、それ以外の場合はfalse
//		引数が適切でない場合もfalse
//
//	EXCEPTIONS
//	なし
//
bool
StringArrayData::
overlaps(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == getType()
		&& pData_->getElementType() == getElementType()) {

		const StringArrayData* pStringArrayData =
			_SYDNEY_DYNAMIC_CAST(const StringArrayData*, pData_);

		if (pStringArrayData->getCount() >= 2
			&& getCount() >= 2)
		{
			ModUnicodeString cThisStart = getElement(0);
			ModUnicodeString cThisStop  = getElement(1);
			ModUnicodeString cOtherStart = pStringArrayData->getElement(0);
			ModUnicodeString cOtherStop  = pStringArrayData->getElement(1);

			//Start > Stopの場合、両者を入れ替える

			//if (UnicodeChar::compareString(cThisStart, cThisStop) > 0)
			if (cThisStart > cThisStop)
			{
				ModUnicodeString tmp = cThisStart;
				cThisStart = cThisStop;
				cThisStop = tmp;
			}
			//if (UnicodeChar::compareString(cOtherStart, cOtherStop) > 0)
			if (cOtherStart > cOtherStop)
			{
				ModUnicodeString tmp = cOtherStart;
				cOtherStart = cOtherStop;
				cOtherStop = tmp;
			}

			if (cThisStart <= cOtherStop && cThisStop >= cOtherStart)
			//if    (UnicodeChar::compareString(cThisStart, cOtherStop)  <= 0 
			//    && UnicodeChar::compareString(cThisStop,  cOtherStart) >= 0)
				bResult = true;
		}
	}
	
	return bResult;
}

//
//	FUNCTION public
//	Common::StringArrayData -- 包含しているかどうか
//
//	NOTES
//	iValue_が自分自身の要素に含まれているかどうか(inの逆)
//
//	ARGUMENTS
//	const ModUnicodeString& cstrValue_
//		値
//
//	RETURN
//	含まれている場合はtrue、それ以外の場合はfalse
//
//	EXCETPTIONS
//	なし
//
bool
StringArrayData::
contains(const ModUnicodeString& cstrValue_) const
{
	bool bResult = false;
	for (ModVector<ModUnicodeString>::ConstIterator i = m_veccValue.begin();
		 i != m_veccValue.end(); ++i)
	{
		if ((*i) == cstrValue_)
		{
			bResult = true;
			break;
		}
	}
	return bResult;
}

//	FUNCTION private
//	Common::StringArrayData::getClassID_NotNull -- クラスIDを得る
//
//	NOTES
//	シリアライズのためのクラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		Common::StringArrayDataクラスのクラスID
//
//	EXCEPTIONS
//	なし

int
StringArrayData::getClassID_NotNull() const
{
	return ClassID::StringArrayDataClass;
}

//	FUNCTION private
//	Common::StringArrayData::print_NotNull -- 値を表示する
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
StringArrayData::print_NotNull() const
{
	int n = 0;
	for (ModVector<ModUnicodeString>::ConstIterator i = m_veccValue.begin();
		 i != m_veccValue.end(); ++i)
	{
		ModUnicodeString & strTmp = const_cast<ModUnicodeString&>(*i);
		cout << "array[" << n++ << "]: "
			<< "string: " << strTmp.getString(Common::LiteralCode) << endl;
	}
}

//	FUNCTION protected
//	Common::StringArrayData::getDumpSize_NotNull -- ダンプサイズを得る
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
StringArrayData::
getDumpSize_NotNull() const {
	; _TRMEISTER_ASSERT(!isNull());

	// !! 注意
	// ここでの値は各要素のダンプサイズの和に過ぎない.
	// dumpValue() を実装する際には再考せよ.

	ModSize iDumpSize = 0;
	for( int i = 0; i < m_iCount; ++i ){
		const ModUnicodeString& s = getElement( i );

		// EncodingForm がわからないので UCS2 ということにする
		// ここも dumpValue() を実装する際には再考すること
		iDumpSize += s.getLength() * sizeof( ModUnicodeChar );
	}

	return iDumpSize;
}

//	FUNCTION private
//	Common::StringArrayData::getString_NotNull -- 文字列で取り出す
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
StringArrayData::getString_NotNull() const
{
	ModUnicodeOstrStream cStream;
	cStream << "{";
	for (ModVector<ModUnicodeString>::ConstIterator i = m_veccValue.begin();
		 i != m_veccValue.end(); ++i)
	{
		if (i != m_veccValue.begin()) cStream << ",";
		cStream << *i;
	}
	cStream << "}";

	return ModUnicodeString(cStream.getString());
}

// FUNCTION public
//	Common::StringArrayData::assign_NoCast -- 代入を行う(キャストなし)
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
StringArrayData::
assign_NoCast(const Data& r)
{
	if (r.getType() == getType()
		&& r.getElementType() == getElementType()) {
		const StringArrayData& data = _SYDNEY_DYNAMIC_CAST(const StringArrayData&, r);
		const ModVector<ModUnicodeString>& vecRhs = data.getValue();
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
