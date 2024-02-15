// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataArrayData -- Common::Dataの配列をあらわすクラス
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

#ifndef __TRMEISTER_COMMON_DATAARRAYDATA_H
#define __TRMEISTER_COMMON_DATAARRAYDATA_H

#include "Common/Module.h"
#include "Common/ArrayData.h"
#include "Common/ObjectPointer.h"

#include "ModVector.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	DataArrayData -- Common::Data の配列を表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION DataArrayData
	: public	ArrayData
{
public:
	//
	//	TYPEDEF
	//	Pointer -- Common::Dataのポインタ
	//
	//	NOTES
	//	Common::ObjectPointer<Common::Data>
	//
	typedef Data::Pointer Pointer;

	//コンストラクタ(1)
	DataArrayData();
	//コンストラクタ(2)
	explicit DataArrayData(const ModVector<Pointer>& vecpValue_);
	//コピーコンストラクタ
	DataArrayData(const DataArrayData& cDataArrayData_);
	//デストラクタ
	virtual ~DataArrayData();

	// 要素の格納に必要な領域を確保する
	void reserve(int n);

	// 要素を破棄する
	void clear();
	//指定した要素を削除する
	void erase(const int nIndex_);

	// シリアル化する
//	Common::ArrayData
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::ScalarData
//	virtual Data::Pointer
//	copy() const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

	//代入オペレータ
	DataArrayData& operator =(const DataArrayData& cDataArrayData_);

	//すべての要素を取り出す
	const ModVector<Pointer>& getValue() const;
	//すべての要素を設定する
	void setValue(const ModVector<Pointer>& vecpValue_);

	//要素数を取り出す
	int getCount() const;
	//要素数を設定する
	void setCount(int n);

	//要素を取り出す
	const Pointer& getElement(int iIndex_);
	const Pointer& getElement(int iIndex_) const;

	//要素を設定する
	void setElement(int iIndex_, const Pointer& pValue_);

	// 配列の先頭に要素を挿入する
	void pushFront(const Pointer& pValue_);
	// 配列の末尾に要素を挿入する
	void pushBack(const Pointer& pValue_);

	// 配列の先頭要素を削除する
	void popFront();
	// 配列の末尾要素を削除する
	void popBack();

	// 文字列の形式で値を得る
//	Common::ArrayData
//	virtual ModUnicodeString
//	getString() const;

//	//比較
//	bool equals(const Data* pData_) const;
//	//大小比較
//	int compareTo(const Data* pData_) const;
	// DISTINCTか調べる
	virtual bool
	distinct(const Data* r) const;
	//範囲が重なっているかどうかチェックする
	bool overlaps(const Data* pData_) const;
//	// 代入を行う
//	virtual bool
//	assign(const Data* r, bool bForAssign_ = true);
//	// 四則演算を行う
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r);

	// 連結する
	void connect(const DataArrayData* pArrayData_);

	//一致する要素が存在するか
	bool contains(const Data* pData_) const;

	// クラスIDを得る
//	Common::ArrayData
//	virtual int
//	getClassID() const;

	// 値を標準出力へ出力する
//	Common::ArrayData
//	virtual void
//	print() const;

	// 付加機能を適用可能か調べる
//	Common::ArrayData
//	virtual bool
//	isApplicable(Function::Value function);
	// 付加機能を適用する
//	Common::ArrayData
//	virtual Pointer
//	apply(Function::Value function);

private:
	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

	// コピーする(自分自身が NULL 値でない)
	virtual Data::Pointer
	copy_NotNull() const;

	// 文字列の形式で値を得る(自分自身が NULL 値でない)
	virtual ModUnicodeString
	getString_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const;
	// 大小比較を行う(キャストなし)
	virtual int
	compareTo_NoCast(const Data& r) const;
	// 代入を行う(キャストなし)
	virtual bool
	assign_NoCast(const Data& r);

	// 付加機能を適用可能か調べる(自分自身が NULL 値でない)
	virtual bool
	isApplicable_NotNull(Function::Value function);
	// 付加機能を適用する(自分自身が NULL 値でない)
	virtual Pointer
	apply_NotNull(Function::Value function);

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	// 末尾にデータをつなげる
	void connect_NotNull(const DataArrayData* pArrayData_);

	//要素をコピーして設定する
	void setCopyValue(const ModVector<Pointer>& vecpValue_);

	// ダンプサイズを得る(自分自身が NULL 値でない)
	virtual ModSize
	getDumpSize_NotNull() const;

	// データに対応するSQLDataを得る
	virtual bool getSQLTypeByValue(SQLData& cResult_);

	//配列データ
	ModVector<Pointer> m_vecpValue;
};

//
//	FUNCTION public
//	DataArrayData -- コンストラクタ(1)
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

inline
DataArrayData::
DataArrayData()
	: ArrayData(DataType::Data)
{
}

//
//	FUNCTION public
//	DataArrayData -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//	引数vecpValue_の中身はコピーされる。
//
//	ARGUMENTS
//	ModVector<Pointer>&
//		vevpValue_	オブジェクトポインタの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
DataArrayData::
DataArrayData(const ModVector<Pointer>& vecpValue_)
	: ArrayData(DataType::Data)
{
	setValue(vecpValue_);
}

//
//	FUNCTION public
//	Common::DataArrayData::DataArrayData -- コピーコンストラクタ
//
//	NOTES
//	コピーコンストラクタ。
//	引数 cDataArrayData_ の中身はコピーされる。
//
//	ARGUMENTS
//	const Common::DataArrayData& cDataArrayData_
//		データ配列データへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
DataArrayData::
DataArrayData(const DataArrayData& cDataArrayData_)
	: ArrayData(DataType::Data)
{
	setCopyValue(cDataArrayData_.m_vecpValue);
}

//
//	FUNCTION public
//	Common::DataArrayData::~DataArrayData -- デストラクタ
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

inline
DataArrayData::
~DataArrayData()
{
	clear();
}

//
//	FUNCTION public
//	Common::DataArrayData::clear -- すべての要素を破棄する
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
DataArrayData::
clear()
{
	m_vecpValue.clear();
}

//
//	FUNCTION public
//	Common::DataArrayData::erase -- 指定した要素を削除する
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
DataArrayData::
erase(const int nIndex_)
{
	m_vecpValue.erase(m_vecpValue.begin() + nIndex_);
}

//
//	FUNCTION public
//	Common::DataArrayData::operator= -- 代入オペレータ
//
//	NOTES
//	代入オペレータ。
//	引数の中身はコピーせずに同じポインタを格納する。
//
//	ARGUMENTS
//	const Common::DataArrayData& cDataArrayData
//		データ配列データへの参照
//
//	RETURN
//	Common::DataArrayData&
//		自分自身への参照
//
//	EXCEPTIONS
//	なし
//

inline
DataArrayData&
DataArrayData::
operator =(const DataArrayData& cDataArrayData_)
{
	if (this != &cDataArrayData_)
		setValue(cDataArrayData_.m_vecpValue);
	return *this;
}

//
//	FUNCTION public
//	Common::DataArrayData::getValue -- すべての要素を取り出す
//
//	NOTES
//	すべての要素を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModVector<Common::DataArrayData::Pointer>&
//		オブジェクトポインタの配列
//
//	EXCEPTIONS
//	なし
//

inline
const ModVector<DataArrayData::Pointer>&
DataArrayData::
getValue() const
{
	return m_vecpValue;
}

//
//	FUNCTION public
//	Common::DataArrayData::setValue -- すべての要素を設定する
//
//	NOTES
//	すべての要素を設定する。
//	引数の中身はコピーせずに同じポインタを格納する。
//
//	ARGUMENTS
//	const ModVector<Pointer>& vecpValue_
//		オブジェクトポインタの配列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
DataArrayData::
setValue(const ModVector<Pointer>& vecpValue_)
{
	m_vecpValue = vecpValue_;
}

//	FUNCTION private
//	Common::DataArrayData::copy_NotNull -- 自分自身のコピーを作成する
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

inline
Data::Pointer
DataArrayData::copy_NotNull() const
{
	return new DataArrayData(*this);
}

//
//	FUNCTION public
//	Common::DataArrayData::getCount -- 要素数を取り出す
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

inline
int
DataArrayData::
getCount() const
{
	return (int)m_vecpValue.getSize();
}

//	FUNCTION public
//	Common::DataArrayData::reserve -- 要素の格納に必要な領域を確保する
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
DataArrayData::
reserve(int n)
{
	m_vecpValue.reserve(n);
}

//	FUNCTION public
//	Common::DataArrayData::pushFront -- 配列の先頭に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData::Pointer	pValue_
//			挿入する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
DataArrayData::
pushFront(const Pointer& pValue_)
{
	m_vecpValue.pushFront(pValue_);
	setNull(false);
}

//	FUNCTION public
//	Common::DataArrayData::pushBack -- 配列の末尾に要素を挿入する
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData::Pointer	pValue_
//			挿入する要素
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
DataArrayData::
pushBack(const Pointer& pValue_)
{
	m_vecpValue.pushBack(pValue_);
	setNull(false);
}

//	FUNCTION public
//	Common::DataArrayData::popFront -- 配列の先頭要素を削除する
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
DataArrayData::
popFront()
{
	m_vecpValue.popFront();
}

//	FUNCTION public
//	Common::DataArrayData::popBack -- 配列の末尾要素を削除する
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
DataArrayData::
popBack()
{
	m_vecpValue.popBack();
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_DATAARRAYDATA_H

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
