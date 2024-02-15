// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerArrayData.h -- Integer(32ビット)の配列をあらわすクラス
// 
// Copyright (c) 1999, 2000, 2004, 2005, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_INTEGERARRAYDATA_H
#define __TRMEISTER_COMMON_INTEGERARRAYDATA_H

#include "Common/Module.h"
#include "Common/ArrayData.h"

#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::IntegerArrayData -- Integer(32ビット)の配列をあらわすクラス
//
//	NOTES
//	Integer(32ビット)の配列をあらわすクラス。
//
class SYD_COMMON_FUNCTION IntegerArrayData : public ArrayData
{
public:
	//コンストラクタ(1)
	IntegerArrayData();
	//コンストラクタ(2)
	explicit IntegerArrayData(const ModVector<int>& veciValue_);
	//デストラクタ
	virtual ~IntegerArrayData();

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

	//すべての要素を取り出す
	const ModVector<int>& getValue() const;
	//すべての要素を設定する
	void setValue(const ModVector<int>& veciValue_);

	//要素数を取り出す
	int getCount() const;

	//要素を取り出す
	int getElement(int iIndex_) const;
	//要素を設定する
	void setElement(int iIndex_, int iValue_);

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
	bool contains(int iValue_) const;
	
	// クラスIDを得る
//	Common::ArrayData
//	virtual int
//	getClassID() const;

	// 値を標準出力へ出力する
//	Common::ArrayData
//	virtual void
//	print() const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;
	
private:
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
	ModVector<int> m_veciValue;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_INTEGERARRAYDATA_H

//
//	Copyright (c) 1999, 2000, 2004, 2005, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
