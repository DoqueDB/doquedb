// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Integer64Data.h -- 64 ビット長整数型データ関連のクラス定義、関数宣言
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2007, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_INTEGER64DATA_H
#define __TRMEISTER_COMMON_INTEGER64DATA_H

#include "Common/Module.h"
#include "Common/ScalarData.h"

#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	CLASS
//	Common::Integer64Data -- 64 ビット長の整数型データを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION Integer64Data
	: public	ScalarData
{
public:
	//コンストラクタ(1)
	Integer64Data();
	//コンストラクタ(2)
	explicit Integer64Data(ModInt64 llValue_);
	//デストラクタ
	virtual ~Integer64Data();

	// シリアル化する
//	Common::ScalarData
//	virtual void
//	serialize(ModArchive& archiver);

	// コピーする
//	Common::ScalarData
//	virtual Data::Pointer
//	copy() const;
	// キャストする
//	Common::ScalarData
//	virtual Pointer
//	cast(DataType::Type type) const;
	virtual Pointer
	castToDecimal(bool bForAssign_ = false) const;

	//値を得る
	ModInt64 getValue() const;
	//値を設定する
	void setValue(ModInt64 llValue_);

	// 文字列の形式で値を得る
//	Common::ScalarData
//	virtual ModUnicodeString
//	getString() const;

	// 等しいか調べる
//	Common::Data
//	virtual bool
//	equals(const Data* r) const;
	// 大小比較を行う
//	Common::Data
//	virtual int
//	compareTo(const Data* r) const;

	// 代入を行う
//	Common::Data
//	virtual bool
//	assign(const Data* r);
	// 四則演算を行う
//	Common::Data
//	virtual bool
//	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	// 単項演算を行う
//	Common::ScalarData
//	virtual bool
//	operateWith(DataOperation::Type op, Pointer& result) const;
//	virtual bool
//	operateWith(DataOperation::Type op);

	// クラスIDを得る
//	Common::ScalarData
//	virtual int
//	getClassID() const;

	// 値を標準出力へ出力する
//	Common::ScalarData
//	virtual void
//	print() const;
	
	// 付加機能を適用可能か調べる
//	Common::ScalarData
//	virtual bool
//	isApplicable(Function::Value function);
	// 付加機能を適用する
//	Common::ScalarData
//	virtual Pointer
//	apply(Function::Value function);

	// ダンプ可能か調べる
//	Common::ScalarData
//	virtual bool
//	isAbleToDump() const;
	// 常に固定長であるかを得る
//	Common::ScalarData
//	virtual bool
//	isFixedSize() const;
	// ダンプサイズを得る
//	Common::ScalarData
//	virtual ModSize
//	getDumpSize() const;
	// 自分自身にダンプされた値を設定する
	virtual ModSize setDumpedValue(const char* pszValue_);
	virtual ModSize setDumpedValue(const char* pszValue_, ModSize uSize_);

	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_);

	// 値をダンプする
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(char* p) const;
//	Common::ScalarData
//	virtual	ModSize
//	dumpValue(ModSerialIO& cSerialIO_) const;

	static ModSize getArchiveSize();

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

	// 数値で得る(自分自身が NULL 値でない)
	virtual int getInt_NotNull() const;
	virtual unsigned int getUnsignedInt_NotNull() const;

	// 等しいか調べる(キャストなし)
	virtual bool
	equals_NoCast(const Data& r) const
	{
		return !compareTo_NoCast(r);
	}
	// 大小比較を行う(キャストなし)
	virtual int
	compareTo_NoCast(const Data& r) const;

	// 代入を行う(キャストなし)
	virtual bool
	assign_NoCast(const Data& r);
	// 単項演算を行う(自分自身が NULL 値でない)
	virtual bool
	operateWith_NotNull(DataOperation::Type op, Pointer& result) const;
	virtual bool
	operateWith_NotNull(DataOperation::Type op);
	// 四則演算を行う(キャストなし)
	virtual bool
	operateWith_NoCast(DataOperation::Type op, const Data& r);

	// 付加機能を適用可能か調べる(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual bool
//	isApplicable_NotNull(Function::Value function);
	// 付加機能を適用する(自分自身が NULL 値でない)
//	Common::ScalarData
//	virtual Pointer
//	apply_NotNull(Function::Value function);

	// ダンプ可能か調べる(自分自身が NULL 値でない)
	virtual bool
	isAbleToDump_NotNull() const;
	// 常に固定長であるかを得る(自分自身が NULL 値でない)
	virtual bool
	isFixedSize_NotNull() const;
	// ダンプサイズを得る(自分自身が NULL 値でない)
	virtual ModSize
	getDumpSize_NotNull() const;
	// 値をダンプする(自分自身が NULL 値でない)
	virtual	ModSize
	dumpValue_NotNull(char* p) const;

	virtual	ModSize
	dumpValue_NotNull(ModSerialIO& cSerialIO_) const;

	// クラスIDを得る(自分自身が NULL 値でない)
	virtual int
	getClassID_NotNull() const;

	// 値を標準出力へ出力する(自分自身が NULL 値でない)
	virtual void
	print_NotNull() const;

	//データ
	ModInt64 m_llValue;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_INTEGER64DATA_H

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
