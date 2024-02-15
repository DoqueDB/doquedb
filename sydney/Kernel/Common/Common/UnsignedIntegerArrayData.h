// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UnsignedIntegerArrayData.h
//						-- UnsignedInteger(32ビット)の配列をあらわすクラス
// 
// Copyright (c) 1999, 2000, 2004, 2005, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_UNSIGNEDINTEGERARRAYDATA_H
#define __TRMEISTER_COMMON_UNSIGNEDINTEGERARRAYDATA_H

#include "Common/ArrayData.h"
#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::UnsignedIntegerArrayData
//							-- UnsignedInteger(32ビット)の配列をあらわすクラス
//
//	NOTES
//	UnsignedInteger(32ビット)の配列をあらわすクラス。
//
class SYD_COMMON_FUNCTION UnsignedIntegerArrayData : public ArrayData
{
public:
	//コンストラクタ(1)
	UnsignedIntegerArrayData();
	//コンストラクタ(2)
	explicit UnsignedIntegerArrayData(
		const ModVector<unsigned int>& veciValue_);
	//デストラクタ
	virtual ~UnsignedIntegerArrayData();

	// 要素の格納に必要な領域を確保する
	void reserve(int n);

	// 要素を廃棄する
	void clear();
	// 指定した要素を削除する
	void erase(const int nIndex_);

	// シリアル化する
//	Common::ArrayData
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::ArrayData
//	virtual Data::Pointer
//	copy() const;
	// キャストする
//	Common::ArrayData
//	virtual Pointer
//	cast(DataType::Type type) const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

	//すべての要素を取り出す
	const ModVector<unsigned int>& getValue() const;
	//すべての要素を設定する
	void setValue(const ModVector<unsigned int>& veciValue_);

	//要素数を取り出す
	int getCount() const;

	//要素を取り出す
	unsigned int getElement(int iIndex_) const;
	//要素を設定する
	void setElement(int iIndex_, unsigned int uiValue_);

	// 配列の先頭に要素を挿入する
	void pushFront(unsigned int uiValue_);
	// 配列の末尾に要素を挿入する
	void pushBack(unsigned int uiValue_);

	// 配列の先頭要素を削除する
	void popFront();
	// 配列の末尾要素を削除する
	void popBack();

	// 文字列の形式で値を得る
//	Common::ArrayData
//	virtual ModUnicodeString
//	getString() const;

	//比較
	bool equals(const Data* pData_) const;
	//大小比較
	int compareTo(const Data* pData_) const;
	//範囲が重なっているかどうかチェックする
	bool overlaps(const Data* pData_) const;

	//一致する要素が存在するか
	bool contains(unsigned int uiValue_) const;

	//ダンプしたデータを設定する
	ModSize setDumpedValue(ModSerialIO& cSerialIO_, ModSize uiSize_);
	
	// クラスIDを得る
//	Common::ArrayData
//	virtual int
//	getClassID() const;

	// 値を標準出力へ出力する
//	Common::ArrayData
//	virtual void
//	print() const;

private:
	// データをダンプする
	ModSize dumpValue_NotNull(ModSerialIO& cSerialIO_) const;
	
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Data::Pointer
	copy_NotNull() const;
	// キャストする(自分自身が NULL 値でない)
	virtual Pointer
	cast_NotNull(DataType::Type type, bool bForAssign_ = false) const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 代入を行う(キャストなし)
	virtual bool
	assign_NoCast(const Data& r);

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	// ダンプサイズを得る(自分自身が NULL 値でない)
	virtual ModSize
	getDumpSize_NotNull() const;

	//配列データ
	ModVector<unsigned int> m_vecuiValue;
};

//	FUNCTION public
//	Common::UnsignedIntegerArrayData::reserve
//		-- 要素の格納に必要な領域を確保する
//
//	NOTES
//
//	ARGUMENTS
//		int	n
//			確保する領域で格納可能な要素数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
UnsignedIntegerArrayData::reserve(int n)
{
	m_vecuiValue.reserve(n);
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::clear -- すべての要素を破棄する
//
//	NOTES
//	すべての要素を破棄する。
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

inline
void
UnsignedIntegerArrayData::clear()
{
	m_vecuiValue.clear();
}

//
//	FUNCTION public
//	Common::UnsignedIntegerArrayData::erase -- 指定した要素を削除する
//
//	NOTES
//	指定した要素を削除する
//
//	ARGUMENTS
//	const int nIndex_
//		削除する要素のインデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
UnsignedIntegerArrayData::erase(const int nIndex_)
{
	m_vecuiValue.erase(m_vecuiValue.begin() + nIndex_);
}

//	FUNCTION public
//	Common::UnsignedIntegerArrayData::pushFront -- 配列の先頭に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int uiValue_
//			挿入する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
UnsignedIntegerArrayData::pushFront(unsigned int uiValue_)
{
	m_vecuiValue.pushFront(uiValue_);
	setNull(false);
}

//	FUNCTION public
//	Common::UnsignedIntegerArrayData::pushBack -- 配列の末尾に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int uiValue_
//			挿入する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
UnsignedIntegerArrayData::pushBack(unsigned int uiValue_)
{
	m_vecuiValue.pushBack(uiValue_);
	setNull(false);
}

//	FUNCTION public
//	Common::UnsignedIntegerArrayData::popFront -- 配列の先頭要素を削除する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
inline
void
UnsignedIntegerArrayData::popFront()
{
	m_vecuiValue.popFront();
}

//	FUNCTION public
//	Common::UnsignedIntegerArrayData::popBack -- 配列の末尾要素を削除する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
inline
void
UnsignedIntegerArrayData::popBack()
{
	m_vecuiValue.popBack();
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_UNSIGNEDINTEGERARRAYDATA_H

//
//	Copyright (c) 1999, 2000, 2004, 2005, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
