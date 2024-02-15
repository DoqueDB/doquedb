// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.h -- データ関連のクラス定義、関数宣言
// 
// Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_DATA_H
#define __TRMEISTER_COMMON_DATA_H

#include "Common/Module.h"
#include "Common/DataOperation.h"
#include "Common/DataType.h"
#include "Common/ExecutableObject.h"
#include "Common/Externalizable.h"
#include "Common/ObjectPointer.h"

#include "ModSerial.h"
#include "ModUnicodeString.h"
#include "ModSerialIO.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

class ArrayData;
class SQLData;


//	CLASS
//	Common::Data -- データを表すクラス
//
//	NOTES

class SYD_COMMON_FUNCTION Data
	: public	ExecutableObject,
	  public	Externalizable
{
public:
	typedef ObjectPointer<Data> Pointer;

	//	ENUM
	//	Common::Data::Function::Value -- データへの付加機能を表す列挙子
	//
	//	NOTES

	struct Function
	{
		typedef unsigned int Value;
		enum {
			None		= 0x00,			// 付加機能なし
			Compressed	= 0x01,			// 圧縮機能
#ifdef OBSOLETE
			Stream		= 0x01 << 1,	// ストリーム機能
#endif
			Unfold		= 0x01 << 2,	// unfold any-type data
			ValueNum
		};
	};

	//コンストラクタ
	Data(DataType::Type eType_, Function::Value eFunction_ = Function::None);
	//デストラクタ
	virtual ~Data();

	//==
	bool operator==(const Data& cData_) const
	{return equals(&cData_);}

	// シリアル化する
	virtual void
	serialize(ModArchive& archiver);

	// コピーする
	virtual Pointer
	copy() const;
	// キャストする
	virtual Pointer
	cast(DataType::Type type, bool bForAssign_ = false) const;
	virtual Pointer
	cast(const Data& target, bool bForAssign_ = false) const;
	virtual Pointer
	castToDecimal(bool bForAssign_ = false) const;

	// 型を得る
	virtual DataType::Type getType() const;
	//要素のデータ型を得る
	virtual Common::DataType::Type getElementType() const;
	// 文字列の形式で値を得る
	virtual ModUnicodeString
	getString() const;
	// 数値で得る
	virtual int getInt() const;
	virtual unsigned int getUnsignedInt() const;

	// スカラデータか調べる
	virtual bool
	isScalar() const;
	static bool
	isScalar(DataType::Type type);

	// NULL 値か調べる
	bool
	isNull() const;
	// NULL 値にする
	virtual void
	setNull(bool v = true);

	// DEFAULT 値か調べる
	bool
	isDefault() const;
	// DEFAULT 値にする
	virtual void
	setDefault(bool v = true);

	// 等しいか調べる
	virtual bool
	equals(const Data* r) const;
	// 大小比較を行う
	virtual int
	compareTo(const Data* r) const;
	// DISTINCTか調べる
	virtual bool
	distinct(const Data* r) const;

	// 代入を行う
	virtual bool
	assign(const Data* r, bool bForAssign_ = true);

	// 四則演算を行う
	virtual bool
	operateWith(DataOperation::Type op, const Data* r, Pointer& result) const;
	virtual bool
	operateWith(DataOperation::Type op, const Data* r);
	// 単項演算を行う
	virtual bool
	operateWith(DataOperation::Type op, Pointer& result) const;
	virtual bool
	operateWith(DataOperation::Type op);

	//範囲
	bool between(const Data* pFrom_, const Data* pTo_) const;
	bool between(const ArrayData* pSpan_) const;
	//包含
	bool in(const Data* pData_) const;

	//文字列を得る
	ModUnicodeString toString() const;

	//付加機能を得る
	Function::Value getFunction() const;
	//付加機能を設定する
	void setFunction(Function::Value iFunction_);
	// 付加機能を適用可能か調べる
	virtual bool
	isApplicable(Function::Value function);
	// 付加機能を適用する
	virtual Pointer
	apply(Function::Value function);

	// ダンプ可能か調べる
	virtual bool
	isAbleToDump() const;
	// 常に固定長であるかを得る
	virtual bool
	isFixedSize() const;
	// ダンプサイズを得る
	virtual ModSize
	getDumpSize() const;

	virtual ModSize setDumpedValue(const char* pszValue_);
	virtual ModSize setDumpedValue(const char* pszValue_, ModSize uSize_);

	// 値をダンプする
	virtual	ModSize	dumpValue(char* p) const;

	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_);
	virtual ModSize setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_);

	virtual ModSize dumpValue(ModSerialIO& cSerialIO_) const;

	// 型を引数にした関数
	static bool		isFixedSize(DataType::Type eType_);
	static bool		isAbleToDump(DataType::Type eType_);
	static bool		isCompatible(DataType::Type eType1_, DataType::Type eType2_);
	static ModSize	getDumpSize(DataType::Type eType_);

	// 型の互換性を得る
	virtual bool isCompatible(const Data* r) const;

	// データ型からSQLDataを得る
	static bool getSQLType(DataType::Type eType_, SQLData& cResult_);
	// データからSQLDataを得る
	virtual bool getSQLType(SQLData& cResult_);

	// クラス内部構造をSQLTypeに合わせる
	virtual void setSQLType(const SQLData& cType_);

	// クラスIDを得る
	virtual int
	getClassID() const;

	// 値を標準出力へ出力する
	virtual void
	print() const;

protected:
	const Data* getTargetData() const;
	void setTargetData(const Data* pData_) const;

	// シリアル化する(自分自身が NULL 値でない)
	virtual void
	serialize_NotNull(ModArchive& archiver);

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

	virtual ModSize
	dumpValue_NotNull(ModSerialIO& cSerialIO_) const;

private:
	// コピーする(自分自身が NULL 値でない)
	virtual Pointer
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
	equals_NoCast(const Data& r) const;

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

	// 実際の値を用いてSQLDataを得る(getSQLTypeの下請け)
	virtual bool getSQLTypeByValue(SQLData& cResult_);

	// データ型
	DataType::Type	m_eType;
	
	mutable const Data* m_pTargetData;

	// 付加機能
	Function::Value m_eFunction;
	// NULL 値であるか
	bool			_isNull;
	// DEFAULT 値であるか
	bool			_isDefault;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_DATA_H

//
//	Copyright (c) 1999, 2001, 2004, 2005, 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
