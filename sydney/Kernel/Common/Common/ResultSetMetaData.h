// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSetMetaData.h -- 
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

#ifndef __TRMEISTER_COMMON_RESULTSETMETADATA_H
#define __TRMEISTER_COMMON_RESULTSETMETADATA_H

#include "Common/Module.h"
#include "Common/ArrayData.h"
#include "Common/ColumnMetaData.h"

#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::ResultSetMetaData -- 結果集合のメタデータ
//
//	NOTES
//	ColumnMetaDataの配列で表現される
//
class SYD_COMMON_FUNCTION ResultSetMetaData : public ArrayData
{
public:
	// イテレータ
	typedef ModVector<ColumnMetaData>::Iterator Iterator;
	typedef ModVector<ColumnMetaData>::ConstIterator ConstIterator;
	
	// コンストラクタ
	ResultSetMetaData();
	// デストラクタ
	virtual ~ResultSetMetaData();

	// 要素数を得る
	int getCount() const;
	// 現在のバッファの容量を得る
	int getCapacity() const;

	// 配列要素が2の時にその範囲が重なっているかどうかチェックする
	bool overlaps(const Data* pData_) const;

	// 要素を取り出す
	ColumnMetaData& getElement(int iIndex_);
	const ColumnMetaData& getElement(int iIndex_) const;
	// 要素を設定する
	void setElement(int iIndex_, const ColumnMetaData& cValue_);

	// クリアする
	void clear();
	// 配列領域を確保する
	void reserve(int n);

	// 要素を設定する
	void assign(int n);

	// すべての要素を取り出す
	const ModVector<ColumnMetaData>& getValue() const;

	// 指定要素を取り出す
	ColumnMetaData& operator[] (int index_);
	const ColumnMetaData& operator[] (int index_) const;

	// 配列の先頭に要素を挿入する
	void pushFront(const ColumnMetaData& cElement_);
	// 配列の末尾に要素を挿入する
	void pushBack(const ColumnMetaData& cElement_);

	// 配列の先頭要素を削除する
	void popFront();
	// 配列の末尾要素を削除する
	void popBack();

	// 指定要素を消す
	void erase(Iterator position_);
	void erase(Iterator first_, const Iterator last_);

	// イテレータを得る
	Iterator begin() { return m_vecColumnMetaData.begin(); }
	ConstIterator begin() const { return m_vecColumnMetaData.begin(); }
	Iterator end() { return m_vecColumnMetaData.end(); }
	ConstIterator end() const { return m_vecColumnMetaData.end(); }

protected:
	// シリアル化
	void serialize_NotNull(ModArchive& archiver_);

	// コピーする
	Data::Pointer copy_NotNull() const;

	// 文字列の形式で値を得る
	ModUnicodeString getString_NotNull() const;

	// クラスIDを得る
	int getClassID_NotNull() const;

private:
	// ColumnMetaDataの配列
	ModVector<ColumnMetaData> m_vecColumnMetaData;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_RESULTSETMETADATA_H

//
//	Copyright (c) 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
