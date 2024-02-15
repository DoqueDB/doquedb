// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DefaultData.h -- DEFAULT データ関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_DEFAULTDATA_H
#define __TRMEISTER_COMMON_DEFAULTDATA_H

#include "Common/Module.h"
#include "Common/ClassID.h"
#include "Common/ScalarData.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::DefaultData -- DEFAULT データを表すクラス
//
//	NOTES
//
class SYD_COMMON_FUNCTION DefaultData
	: public	Common::ScalarData
{
public:
	// インスタンスを得る
	static const DefaultData*
	getInstance();

	//デストラクタ
	virtual ~DefaultData();

	//シリアル化
	void serialize(ModArchive& cArchiver_);

	// コピーする
	virtual Data::Pointer
	copy() const;
	// キャストする
//	Common::Data
//	virtual Pointer
//	cast(DataType::Type type) const;
//	Common::Data
//	virtual Pointer
//	cast(const Data& target) const;

	//文字列でデータを得る
	ModUnicodeString getString() const;

//	// NULL 値にする
//	virtual void
//	setNull(bool v = true);

	// 等しいか調べる
//	Common::Data
	virtual bool
	equals(const Data* r) const
	{return equals_NoCast(*r);}
	// 大小比較を行う
//	Common::Data
	virtual int
	compareTo(const Data* r) const
	{return compareTo_NoCast(*r);}

	// 代入を行う
	virtual bool
	assign(const Data* r, bool bForAssign_ = true);
	// 四則演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	// 単項演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, Pointer& result) const;
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op);

	// 付加機能を適用可能か調べる
//	Common::ScalarData
//	virtual bool
//	isApplicable(Function::Value function);
	// 付加機能を適用する
//	Common::ScalarData
//	virtual Pointer
//	apply(Function::Value function);

	// ダンプ可能か調べる
	virtual bool
	isAbleToDump() const;
	// 常に固定長であるかを得る
	virtual bool
	isFixedSize() const;
	// ダンプサイズを得る
	virtual ModSize
	getDumpSize() const;
	// 値をダンプする
	virtual	ModSize
	dumpValue(char* p) const;

	virtual	ModSize 
	dumpValue(ModSerialIO& cSerialIO_) const;

	// クラスIDを得る
	virtual int
	getClassID() const;

	// 値を標準出力へ出力する
	virtual void
	print() const;

	//ハッシュコードを取り出す
	virtual ModSize hashCode() const;

private:
	SYD_COMMON_FUNCTION
	friend Externalizable* getClassInstance(int iClassID_);

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const;
	// 大小比較を行う(キャストなし)
	virtual int
	compareTo_NoCast(const Data& r) const;

	//コンストラクタ
	DefaultData();
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_DEFAULTDATA_H

//
//	Copyright (c) 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
