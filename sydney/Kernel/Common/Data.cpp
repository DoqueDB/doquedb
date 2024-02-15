// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.cpp -- データ関連の関数定義Function definition related to data
// 
// Copyright (c) 1999, 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/NullData.h"
#include "Common/ObjectIDData.h"
#include "Common/SQLData.h"

#include "Exception/BadArgument.h"
#include "Exception/ClassCast.h"
#include "Exception/NotSupported.h"
#include "Exception/NullNotAllowed.h"
#include "Exception/Object.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	struct _TableEntry {
		bool _fixed;
		bool _ableToDump;
		ModSize _size;

		_TableEntry(bool fixed_, bool ableToDump_, ModSize size_)
			: _fixed(fixed_), _ableToDump(ableToDump_), _size(size_)
		{}
	};

	const _TableEntry _TypeTable[] =
	{
#define _E(f,a,s) _TableEntry((f), (a), (s))
		_E(true , true , sizeof(int)),			// Integer
		_E(true , true , sizeof(unsigned int)),	// UnsignedInteger
		_E(true , true , sizeof(ModInt64)),		// Integer64
		_E(true , true , sizeof(ModUInt64)),	// UnsignedInteger64
		_E(false, true , 0),					// String
		_E(true , true , sizeof(float)),		// Float
		_E(true , true , sizeof(double)),		// Double
		_E(true, true , 0),			    		// Decimal
		_E(true , true , sizeof(int)),			// Date
		_E(true , true , sizeof(int) + sizeof(unsigned int)), // Time
		_E(false, true , 0),					// Binary
		_E(false, false, 0),					// BitSet
		_E(true , true , sizeof(ObjectIDData::FormerType) + sizeof(ObjectIDData::LatterType)), // ObjectID
		_E(false, true , 0),					// Language
		_E(false, false, 0),					// ColumnMetaData
		_E(false, false, 0),					// Word
		_E(false, false, 0),					// SearchTerm
#undef _E
	};

// キャスト可能性を記述するテーブルTable that describes Cast possibility
namespace _Compatible
{
	enum Value
	{
		// キャストの必要なし
		NoCast = 0,
		// 左辺から右辺にキャスト可能Cast is possible in the right side from the left side. 
		RightCastable,
		// 右辺から左辺にキャスト可能
		LeftCastable,
		// 相互にキャスト可能
		BothCastable,
		// キャスト不能
		NotCompatible,
		// 値の数
		Count
	};

#define C NoCast
#define R RightCastable
#define L LeftCastable
#define B BothCastable
#define N NotCompatible

	const Value _compatibleTable[][DataType::MaxScalar - DataType::MinScalar] =
	{
		//// 右辺
		//// Int UIt I64 U64 Str Flt Dbl Dec Dat Tim Bin Bit OID Lan Met Wrd STm	// 左辺
		//{  C,  B,  B,  B,  B,  B,  B,  N,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// Int
		//{  B,  C,  B,  B,  B,  B,  B,  N,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// UIt
		//{  B,  B,  C,  B,  B,  B,  B,  N,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// I64
		//{  B,  B,  B,  C,  B,  B,  B,  N,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// U64
		//{  B,  B,  B,  B,  C,  B,  B,  N,  B,  B,  R,  N,  B,  B,  N,  N,  N},	// Str
		//{  B,  B,  B,  B,  B,  C,  R,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Flt
		//{  B,  B,  B,  B,  B,  L,  C,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Dbl
		//{  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Dec
		//{  N,  N,  N,  N,  B,  N,  N,  N,  C,  R,  N,  N,  N,  N,  N,  N,  N},	// Dat
		//{  N,  N,  N,  N,  B,  N,  N,  N,  L,  C,  N,  N,  N,  N,  N,  N,  N},	// Tim
		//{  N,  N,  N,  N,  L,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N,  N},	// Bin
		//{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N},	// Bit
		//{  L,  L,  L,  L,  B,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N},	// OID
		//{  N,  N,  N,  N,  B,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N},	// Lan
		//{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N},	// Met
		//{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N},	// Wrd
		//{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C}		// STm
				// 右辺
		// Int UIt I64 U64 Str Flt Dbl Dec Dat Tim Bin Bit OID Lan Met Wrd STm	// 左辺
		{  C,  B,  B,  B,  B,  B,  B,  B,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// Int
		{  B,  C,  B,  B,  B,  B,  B,  B,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// UIt
		{  B,  B,  C,  B,  B,  B,  B,  B,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// I64
		{  B,  B,  B,  C,  B,  B,  B,  N,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// U64
		{  B,  B,  B,  B,  C,  B,  B,  B,  B,  B,  R,  N,  B,  B,  N,  N,  N},	// Str
		{  B,  B,  B,  B,  B,  C,  R,  B,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Flt
		{  B,  B,  B,  B,  B,  L,  C,  B,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Dbl
		{  B,  B,  B,  N,  B,  B,  B,  C,  N,  N,  B,  N,  N,  N,  N,  N,  N},	// Dec
		{  N,  N,  N,  N,  B,  N,  N,  N,  C,  R,  N,  N,  N,  N,  N,  N,  N},	// Dat
		{  N,  N,  N,  N,  B,  N,  N,  N,  L,  C,  N,  N,  N,  N,  N,  N,  N},	// Tim
		{  N,  N,  N,  N,  L,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N,  N},	// Bin
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N},	// Bit
		{  L,  L,  L,  L,  B,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N},	// OID
		{  N,  N,  N,  N,  B,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N},	// Lan
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N},	// Met
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N},	// Wrd
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C}	// STm
	};

#undef C
#undef R
#undef L
#undef B
#undef N

	Value
	getValue(const DataType::Type& ltype, const DataType::Type& rtype)
	{
		if (ltype == rtype)
			return NoCast;

		if (!(Data::isScalar(ltype) && Data::isScalar(rtype)))
			return NotCompatible;

		return _compatibleTable
			[ltype - DataType::MinScalar][rtype - DataType::MinScalar];
	}

	Value
	getValue(const Data& l, const Data& r)
	{
		; _TRMEISTER_ASSERT(!l.isNull());
		; _TRMEISTER_ASSERT(!r.isNull());

		const DataType::Type ltype = l.getType();
		const DataType::Type rtype = r.getType();

		return getValue(ltype, rtype);
	}
}

// 比較可能性を記述するテーブル
namespace _Comparable
{
	enum Value
	{
		// キャストの必要なし
		NoCast = 0,
		// 左辺から右辺にキャストして比較
		RightComparable,
		// 右辺から左辺にキャストして比較
		LeftComparable,

		DecimalComparable,
		// 比較不能
		NotComparable,
		// 値の数
		Count
	};

#define C NoCast
#define R RightComparable
#define L LeftComparable
#define	D DecimalComparable
#define N NotComparable

	const Value _comparableTable[][DataType::MaxScalar - DataType::MinScalar] =
	{
		// 右辺
		// Int UIt I64 U64 Str Flt Dbl Dec Dat Tim Bin Bit OID Lan Met Wrd STm	// 左辺
		{  C,  L,  R,  R,  L,  R,  R,  D,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// Int
		{  R,  C,  R,  R,  L,  R,  R,  D,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// UIt
		{  L,  L,  C,  L,  L,  R,  R,  D,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// I64
		{  L,  L,  R,  C,  L,  R,  R,  D,  N,  N,  N,  N,  R,  N,  N,  N,  N},	// U64
		{  L,  L,  L,  L,  C,  L,  L,  D,  L,  L,  R,  N,  L,  L,  N,  N,  N},	// Str
		{  L,  L,  L,  L,  L,  C,  R,  D,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Flt
		{  L,  L,  L,  L,  L,  L,  C,  D,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Dbl
		{  D,  D,  D,  D,  D,  D,  D,  C,  N,  N,  N,  N,  N,  N,  N,  N,  N},	// Dec
		{  N,  N,  N,  N,  L,  N,  N,  N,  C,  R,  N,  N,  N,  N,  N,  N,  N},	// Dat
		{  N,  N,  N,  N,  L,  N,  N,  N,  L,  C,  N,  N,  N,  N,  N,  N,  N},	// Tim
		{  N,  N,  N,  N,  L,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N,  N},	// Bin
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N,  N},	// Bit
		{  L,  L,  L,  L,  L,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N,  N},	// OID
		{  N,  N,  N,  N,  L,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N,  N},	// Lan
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N,  N},	// Met
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C,  N},	// Wrd
		{  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  N,  C},	// STm
	};

#undef C
#undef R
#undef L
#undef N

	Value
	getValue(const DataType::Type& ltype, const DataType::Type& rtype)
	{
		if (ltype == rtype)
			return NoCast;

		if (!(Data::isScalar(ltype) && Data::isScalar(rtype)))
			return NotComparable;

		return _comparableTable
			[ltype - DataType::MinScalar][rtype - DataType::MinScalar];
	}

	Value
	getValue(const Data& l, const Data& r)
	{
		// Nullはすべての型と比較可能である
		if (l.isNull() || r.isNull())
			return NoCast;

		; _TRMEISTER_ASSERT(!l.isNull());
		; _TRMEISTER_ASSERT(!r.isNull());

		const DataType::Type ltype = l.getType();
		const DataType::Type rtype = r.getType();

		return getValue(ltype, rtype);
	}
}

// Common::DataからSQLDataを取得するためのテーブル
namespace _SQLDataType
{
#define XXX SQLData::Type::NoType
#define Chr SQLData::Type::Char
#define NCh SQLData::Type::NChar
#define Int SQLData::Type::Int
#define Flt SQLData::Type::Float
#define Dtt SQLData::Type::DateTime
#define UID SQLData::Type::UniqueIdentifier
#define Bin SQLData::Type::Binary
#define Img SQLData::Type::Image
#define NTx SQLData::Type::NText
#define Ftx SQLData::Type::Fulltext
#define Lan SQLData::Type::Language
#define BLB SQLData::Type::BLOB
#define CLB SQLData::Type::CLOB
#define NLB SQLData::Type::NCLOB
#define Dec SQLData::Type::Decimal
#define Dat SQLData::Type::Date
#define Tim SQLData::Type::Time
#define Tms SQLData::Type::Timestamp
#define UIt SQLData::Type::UInt
#define Wrd SQLData::Type::Word
#define Big SQLData::Type::BigInt

#define N Common::SQLData::Flag::None
#define U Common::SQLData::Flag::Unlimited
#define F Common::SQLData::Flag::Fixed
#define V Common::SQLData::Flag::Variable

	// Common::DataTypeからEntryを得るためのテーブル
	const SQLData _Table[Common::DataType::MaxScalar - Common::DataType::MinScalar] =
	{
#define _S(t,f,l,s) SQLData((t), (f), (l), (s))
		//	Type	Flag	Length	Scale
		_S(Int,		F,		10,		0),	// Int
		_S(UIt,		F,		10,		0),	// UIt
		_S(Big,		F,		19,		0),	// I64
		_S(UIt,		F,		20,		0),	// U64
		_S(NCh,		U,		 0,		0),	// Str
		_S(Flt,		F,		 6,		0),	// Flt
		_S(Flt,		F,		15,		0),	// Dbl
		//_S(Dec,		U,		 0,		0),	// Dec
		_S(Dec,		F,		 0,		0),	// Dec
		_S(Dat,		F,		10,		0),	// Dat
		_S(Tms,		F,		23,		0),	// Tim
		_S(Bin,		U,		 0,		0),	// Bin
		_S(XXX,		N,		 0,		0),	// Bit
		_S(XXX,		N,		 0,		0),	// OID
		_S(Lan,		N,		 0,		0),	// Lan
		_S(XXX,		N,		 0,		0),	// Met
		_S(Wrd,		N,		 0,		0),	// Wrd
#undef _S
	};

#undef N
#undef U
#undef F
#undef V

#undef XXX
#undef Chr
#undef NCh
#undef Int
#undef Flt
#undef Dtt
#undef UID
#undef Bin
#undef Img
#undef NTx
#undef Ftx
#undef Lan
#undef BLB
#undef CLB
#undef NLB
#undef Dec
#undef Dat
#undef Tim
#undef Tms
#undef UIt
#undef Wrd
#undef Big

} // namespace _SQLDataType

}

//
//	FUNCTION public
//	Common::Data::Data -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	Common::DataType::Type eType_
//		データ型
//	Common::Data::Function::Value eFunction_ = None
//		付加機能
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Data::Data(DataType::Type eType_, Function::Value eFunction_)
	: m_eType(eType_),
	  m_pTargetData(0),
	  m_eFunction(eFunction_),
	  _isNull(eType_ == DataType::Null),
	  _isDefault(eType_ == DataType::Default)
{}

//
//	FUNCTION public
//	Common::Data::~Data -- デストラクタ
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
Data::
~Data()
{
}

//	FUNCTION public
//	Common::Data::serialize -- シリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&	archiver
//			使用するアーカイバ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Data::serialize(ModArchive& archiver)
{
	if (isNull())
		const_cast<NullData*>(NullData::getInstance())->serialize(archiver);
	else
		serialize_NotNull(archiver);
}

void
Data::serialize_NotNull(ModArchive& archiver)
{
	//【注意】	以前は型を書いていたが意味ないのでやめた
}

//	FUNCTION public
//	Common::Data::copy -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身のコピー
//
//	EXCEPTIONS

Data::Pointer
Data::copy() const
{
	return (isNull()) ? NullData::getInstance() : copy_NotNull();
}

Data::Pointer
Data::copy_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::cast -- キャストする
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			キャストする型Type that is Cast
//
//	RETURN
//		自分自身の型をキャストした結果のデータを指すオブジェクトポインタ
//It is an object pointer that indicates the data of the result that is Cast as for own type. 
//	EXCEPTIONS
//		Exception::ClassCast
//			この型のデータは指定された型にキャストできない
//As for the data of this type, it cannot be Cast in the specified type. 

Data::Pointer
Data::cast(DataType::Type type, bool bForAssign_ /* = false */) const
{
	return (isNull() || type == DataType::Null) ?
		NullData::getInstance() : cast_NotNull(type, bForAssign_);
}

Data::Pointer
Data::cast_NotNull(DataType::Type type, bool bForAssign_ /* = false */) const
{
	if (type == getType())
		return copy();

	_TRMEISTER_THROW0(Exception::ClassCast);
}

Data::Pointer
Data::castToDecimal(bool bForAssign_ /*= false*/) const
{
	if (getType() == DataType::Decimal)
		return copy();

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION public
//	Common::Data::cast -- 与えられたデータの型へキャストする
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&		target
//			このデータの型へキャストするIt is Cast to the type of this data. 
//
//	RETURN
//		自分自身の型をキャストした結果得られるデータを指すオブジェクトポインタThe object pointer that indicates the obtained data. Cast of own type
//
//	EXCEPTIONS

Data::Pointer
Data::cast(const Data& target, bool bForAssign_ /* = false */) const
{
	if (isNull() || target.isNull())
		return NullData::getInstance();

	setTargetData(&target);
	Data::Pointer pData = cast(target.getType(), bForAssign_);
	setTargetData(0);

	return pData;
}

void 
Data::setTargetData(const Data* pData_) const
{
	m_pTargetData = pData_;
}

const Data*
Data::getTargetData() const
{
	return m_pTargetData;
}

//
//	FUNCTION public
//	Common::Data::getType -- データ型を得る
//
//	NOTES
//	データ型を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataType::Type
//		データ型
//
//	EXCEPTIONS
//	なし
//
DataType::Type
Data::
getType() const
{
	return m_eType;
}

//	FUNCTION public
//	Common::Data::getString -- 文字列の形式で値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文字列
//
//	EXCEPTIONS

ModUnicodeString
Data::getString() const
{
	return (isNull()) ?
		NullData::getInstance()->getString() : getString_NotNull();
}

ModUnicodeString
Data::getString_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Common::Data::getInt -- 数値で得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Data::
getInt() const
{
	if (isNull()) {
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	}
	return getInt_NotNull();
}

// FUNCTION public
//	Common::Data::getUnsignedInt -- 数値で得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	unsigned int
//
// EXCEPTIONS

//virtual
unsigned int
Data::
getUnsignedInt() const
{
	if (isNull()) {
		_TRMEISTER_THROW0(Exception::NullNotAllowed);
	}
	return getUnsignedInt_NotNull();
}

// FUNCTION private
//	Common::Data::getInt_NotNull -- 数値で得る(自分自身が NULL 値でない)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Data::
getInt_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Common::Data::getUnsignedInt_NotNull -- 数値で得る(自分自身が NULL 値でない)
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	unsigned int
//
// EXCEPTIONS

//virtual
unsigned int
Data::
getUnsignedInt_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::isScalar -- スカラデータか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		false
//			スカラデータでない
//
//	EXCEPTIONS
//		なし

bool
Data::isScalar() const
{
	return false;
}

//	FUNCTION public
//	Common::Data::isScalar --
//		与えられたデータ型がスカラデータのデータ型かを調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			調べるデータ型
//
//	RETURN
//		true
//			スカラデータ型である
//		false
//			スカラデータ型でない
//
//	EXCEPTIONS
//		なし

// static
bool
Data::isScalar(DataType::Type type)
{
	return type >= DataType::MinScalar && type < DataType::MaxScalar;
}

//	FUNCTION public
//	Common::Data::isNull -- NULL 値か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			NULL 値である
//		false
//			NULL 値でない
//
//	EXCEPTIONS
//		なし

bool
Data::isNull() const
{
	return _isNull;
}

//	FUNCTION public
//	Common::Data::setNull -- NULL 値にする
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			true または指定されないとき
//				NULL 値にする
//			false
//				NULL 値でなくする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Data::setNull(bool v)
{
	_isNull = v;
	// NULLにしても非NULLにしてもDEFAULTではなくなる
	_isDefault = false;
}

// FUNCTION public
//	Common::Data::isDefault -- DEFAULT か調べる
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Data::
isDefault() const
{
	return _isDefault;
}

// FUNCTION public
//	Common::Data::setDefault -- DEFAULT 値にする
//
// NOTES
//
// ARGUMENTS
//	bool v = true
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Data::
setDefault(bool v /* = true */)
{
	_isDefault = v;
	if (v) {
		// default値にしたとき初期値はNULLにしておく
		_isNull = true;
	}
}

//	FUNCTION public
//	Common::Data::equals -- 等しいか調べる
//
//	NOTES
//		自分自身を左辺に、指定したデータを右辺に与えて等しいか調べる
//
//	ARGUMENTS
//		Common::Data*	r
//			右辺に与えられたデータを格納する領域の先頭アドレス
//	
//	RETURN
//		true
//			自分自身と与えたデータは等しい
//		false
//			自分自身と与えたデータは等しくない
//
//	EXCEPTIONS
//		BadArgument
//			右辺の値が指定されていない

bool
Data::equals(const Data* r) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺のいずれかが NULL であれば、
	// NULL より常に他の型の値のほうが大きいとみなす

	if (isNull())
		return r->isNull();
	else if (r->isNull())
		return false;

	// 比較するために左辺と右辺のどちらを片方の型にキャストすべきか調べる
	// 左辺と右辺のどちらかが NULL の場合は NoCast になる

	switch (_Comparable::getValue(*this, *r)) {
	case _Comparable::LeftComparable:
	{
		// 右辺のデータを左辺の型にキャストする

		Pointer p(r->cast(*this));
		if (p->isNull())
			return false;
		else
			return equals_NoCast(*p);
	}
	case _Comparable::RightComparable:
	{
		// 左辺のデータを右辺の型にキャストする

		Pointer p(cast(*r));
		return _SYDNEY_DYNAMIC_CAST(Data&, *p).equals_NoCast(*r);
	}
	case _Comparable::DecimalComparable:
	{
		Common::Data::Pointer pThisData = this;
		Common::Data::Pointer pData = r;
		if (pData->getType() == Common::DataType::Decimal)
		{
			pData = this;
			; _TRMEISTER_ASSERT(this->getType() != Common::DataType::Decimal);
			pThisData = r;
		}	
		Common::Data::Pointer pDecData(pData->castToDecimal());
		if (pDecData->isNull())
			return false;
		else
			return pThisData->equals_NoCast(*pDecData);
	}
	case _Comparable::NoCast:

		// キャストする必要がないので、そのまま比較する

		return equals_NoCast(*r);
	}

	// キャストできないということは、等しくない

	return false;
}

//	FUNCTION private
//	Common::Data::equals_NoCast -- 等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		値を返すことはない
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型が等しいか調べる機能は提供されていない

bool
Data::equals_NoCast(const Data& r) const
{
	if (isNull())
		return NullData::getInstance()->equals(&r);

	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	// キャストなしの等号を定義可能なデータ型を表すクラスで上書きされる

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::compareTo -- 大小比較を行う
//
//	NOTES
//		自分自身を左辺に、指定したデータを右辺に与えて大小比較する
//
//	ARGUMENTS
//		Common::Data*	r
//			右辺に与えられたデータを格納する領域の先頭アドレス
//
//	RETURN
//		0
//			自分自身と与えたデータは等しい
//		-1
//			自分自身のほうが小さい
//		1
//			与えたデータのほうが小さい
//
//	EXCEPTIONS
//		Exception::BadArgument
//			右辺の値が指定されていない
//		Exception::ClassCast
//			右辺または左辺の値を片方の値の型へキャストできない

int
Data::compareTo(const Data* r) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺のいずれかが NULL であれば、
	// NULL より常に他の型の値のほうが大きいとみなす

	if (isNull())
		return (r->isNull()) ? 0 : -1;
	else if (r->isNull())
		return 1;

	// 比較するために左辺と右辺のどちらを片方の型にキャストすべきか調べる
	// 左辺と右辺のどちらかが NULL の場合は NoCast になる

	_Comparable::Value temp = _Comparable::getValue(*this, *r);
	switch (temp) {
	case _Comparable::LeftComparable:
	{
		// 右辺のデータを左辺の型にキャストする
		// ★注意★
		// 互いにキャスト可能な型は常に左辺の型に揃えることにする
		Pointer p(r->cast(*this));
		if (p->isNull())
			return false;
		else
			return compareTo_NoCast(*p);
	}
	case _Comparable::RightComparable:
	{
		// 左辺のデータを右辺の型にキャストする

		Pointer p(cast(*r));
		if (p->isNull())
			return false;
		else
			return _SYDNEY_DYNAMIC_CAST(Data&, *p).compareTo_NoCast(*r);
	}
	case _Comparable::DecimalComparable:
	{
		Common::Data::Pointer pThisData = this;
		Common::Data::Pointer pData = r;
		if (pData->getType() == Common::DataType::Decimal)
		{
			pData = this;
			; _TRMEISTER_ASSERT(this->getType() != Common::DataType::Decimal);
			pThisData = r;
		}	
		Common::Data::Pointer pDecData(pData->castToDecimal());
		if (pDecData->isNull())
		{
			Pointer pp(pThisData->cast(*pData));
			; _TRMEISTER_ASSERT(!pp->isNull());
			if (r->getType() == Common::DataType::Decimal)
				return -(_SYDNEY_DYNAMIC_CAST(Data&, *pp).compareTo_NoCast(*pData));
			else
				return _SYDNEY_DYNAMIC_CAST(Data&, *pp).compareTo_NoCast(*pData);
		}
		else
		{
			if (r->getType() == Common::DataType::Decimal)
				return -pThisData->compareTo_NoCast(*pDecData);
			else
				return pThisData->compareTo_NoCast(*pDecData);
		}
	}
	case _Comparable::NoCast:

		// キャストする必要がないので、そのまま比較する

		return compareTo_NoCast(*r);
	}

	// キャストできない

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION private
//	Common::Data::compareTo_NoCast -- 大小比較を行う
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//
//	RETURN
//		値を返すことはない
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型のデータの大小比較は提供されていない

int
Data::compareTo_NoCast(const Data& r) const
{
	if (isNull())
		return NullData::getInstance()->compareTo(&r);

	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Common::Data::distinct -- DISTINCTか調べる
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
Data::
distinct(const Data* r) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 両辺がNULLか等しい場合、DISTINCTでない

	if (isNull())
		return !(r->isNull());
	else if (r->isNull())
		return true;

	return compareTo(r) != 0;
}

//	FUNCTION public
//	Common::Data::assign -- 代入を行う
//
//	NOTE
//
//	ARGUMENTS
//		Common::Data*	r
//			右辺に与えられたデータを格納する領域の先頭アドレスThe first address in area where data given right is stored
//			
//	RETURN
//		true
//			演算結果は NULL でない
//		false
//			演算結果は NULL である
//
//	EXCEPTIONS
//		Exception::BadArgument
//			右辺の値が指定されていないA right value is not specified. 
//		Exception::ClassCast
//			右辺または左辺の値を片方の値の型へキャストできないIt cannot be Cast to the type of one of values as for a value that is right or left. 

bool
Data::assign(const Data* r, bool bForAssign_ /* = true */)
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 右辺の値が NULL であれば、
	// 演算結果を NULL にする

	if (r->isNull()) {
		setNull();
		return false;
	}
	// 右辺の値が DEFAULT であれば、
	// 演算結果を DEFAULT にする
	if (r->isDefault()) {
		setDefault();
		return true;
	}

	// 左辺がNULLのままではキャストできないのでNULLでなくする
	setNull(false);

	// 右辺が NULL でないので、
	// 演算するために右辺を左辺の型にキャストできるか調べる

	switch (_Compatible::getValue(*this, *r)) {
	case _Compatible::LeftCastable:
	case _Compatible::BothCastable:
		{
			// 右辺のデータを左辺の型にキャストする

			Pointer p(r->cast(*this, bForAssign_));
			if (p->isNull()) {
				setNull();
				return false;
			} else
				return assign_NoCast(*p);
		}
	case _Compatible::RightCastable:
		{
			// 左辺のデータを右辺の型に
			// キャストするような演算はこのメソッドではできない

			break;
		}
	case _Compatible::NoCast:

		// キャストする必要がないので、そのまま実行する

		return assign_NoCast(*r);
	}

	// キャストできない

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION public
//	Common::Data::operateWith -- 四則演算を行う
//
//	NOTE
//
//	ARGUMENTS
//		Common::DataOperation::Type	op
//			行う演算の種類
//		Common::Data*	r
//			右辺に与えられたデータを格納する領域の先頭アドレス
//		Common::Data::Pointer&	result
//			演算結果を指すオブジェクトポインタ
//			
//	RETURN
//		true
//			演算結果は NULL でない
//		false
//			演算結果は NULL である
//
//	EXCEPTIONS
//		Exception::BadArgument
//			右辺の値が指定されていない
//		Exception::ClassCast
//			右辺または左辺の値を片方の値の型へキャストできない

bool
Data::operateWith(DataOperation::Type op, const Data* r, Pointer& result) const
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺の値のいずれかが NULL であれば、
	// 演算結果を NULL にする

	if (isNull() || r->isNull()) {
		result = NullData::getInstance();
		return false;
	}

	// 左辺と右辺の両方が NULL でないので、
	// 演算するために左辺と右辺のどちらを片方の型にキャストすべきか調べる

	switch (_Compatible::getValue(*this, *r)) {
	case _Compatible::LeftCastable:
	case _Compatible::BothCastable:
	{
		// 右辺のデータを左辺の型にキャストする
		// ★注意★
		// 互いにキャスト可能な型は常に左辺の型に揃えることにする

		Pointer p(r->cast(*this, true /* for assign */));
		result = copy();
		return _SYDNEY_DYNAMIC_CAST(Data&, *result).operateWith_NoCast(op, *p);
	}
	case _Compatible::RightCastable:
	{
		// 左辺のデータを右辺の型にキャストする

		Pointer p(cast(*r, true /* for assign */));
		if (!_SYDNEY_DYNAMIC_CAST(Data&, *p).operateWith_NoCast(op, *r))
			return false;

		// 演算結果を左辺の型にキャストする

		result = p->cast(*this, true /* for assign */);
		return !result->isNull();
	}
	case _Compatible::NoCast:

		// キャストする必要がないので、そのまま実行する

		result = copy();
		return _SYDNEY_DYNAMIC_CAST(Data&, *result).operateWith_NoCast(op, *r);
	}

	// キャストできない

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION public
//	Common::Data::operateWith -- 四則演算を行う
//
//	NOTE
//		thisの内容が変わる
//		左辺をキャストする必要があるような演算はできない
//
//	ARGUMENTS
//		Common::DataOperation::Type	op
//			行う演算の種類
//		Common::Data*	r
//			右辺に与えられたデータを格納する領域の先頭アドレス
//			
//	RETURN
//		true
//			演算結果は NULL でない
//		false
//			演算結果は NULL である
//
//	EXCEPTIONS
//		Exception::BadArgument
//			右辺の値が指定されていない
//		Exception::ClassCast
//			右辺または左辺の値を片方の値の型へキャストできない

bool
Data::operateWith(DataOperation::Type op, const Data* r)
{
	if (!r)

		// 右辺の値が指定されていない

		_TRMEISTER_THROW0(Exception::BadArgument);

	// 左辺または右辺の値のいずれかが NULL であれば、
	// 演算結果を NULL にする

	if (isNull() || r->isNull()) {
		setNull();
		return false;
	}

	// 左辺と右辺の両方が NULL でないので、
	// 演算するために左辺と右辺のどちらを片方の型にキャストすべきか調べる

	switch (_Compatible::getValue(*this, *r)) {
	case _Compatible::LeftCastable:
	case _Compatible::BothCastable:
	{
		// 右辺のデータを左辺の型にキャストする
		// ★注意★
		// 互いにキャスト可能な型は常に左辺の型に揃えることにする

		Pointer p(r->cast(*this, true /* for assign */));
		return operateWith_NoCast(op, *p);
	}
	case _Compatible::RightCastable:
	{
		// 左辺のデータを右辺の型に
		// キャストするような演算はこのメソッドではできない

		_TRMEISTER_THROW0(Exception::ClassCast);
	}
	case _Compatible::NoCast:

		// キャストする必要がないので、そのまま比較する

		return operateWith_NoCast(op, *r);
	}

	// キャストできない

	_TRMEISTER_THROW0(Exception::ClassCast);
}

//	FUNCTION public
//	Common::Data::operateWith -- 単項演算を行う
//
//	NOTE
//
//	ARGUMENTS
//		Common::DataOperation::Type	op
//			行う演算の種類
//		Common::Data::Pointer&	result
//			演算結果を指すオブジェクトポインタ
//			
//	RETURN
//		false
//			演算しなかった
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型のデータの単項演算は提供されていない

bool
Data::operateWith(DataOperation::Type op, Pointer& result) const
{
	if (isNull()) {
		result = NullData::getInstance();
		return false;
	}
	return operateWith_NotNull(op, result);
}

bool
Data::operateWith_NotNull(DataOperation::Type op, Pointer& result) const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

bool
Data::operateWith(DataOperation::Type op)
{
	if (isNull())

		// NULL 値は NULL 値のままである

		return false;

	return operateWith_NotNull(op);
}

bool
Data::operateWith_NotNull(DataOperation::Type op)
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	Common::Data::assign_NoCast -- 代入を行う
//
//	NOTE
//
//	ARGUMENTS
//		Common::Data&	r
//			右辺に与えられたデータ
//			
//	RETURN
//		値を返すことはない
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型のデータの代入は提供されていない

bool
Data::assign_NoCast(const Data& r)
{
	; _TRMEISTER_ASSERT(!r.isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION private
//	Common::Data::operateWith_NoCast -- 四則演算を行う
//
//	NOTE
//
//	ARGUMENTS
//		Common::DataOperation::Type	op
//			行う演算の種類
//		Common::Data&	r
//			右辺に与えられたデータ
//			
//	RETURN
//		値を返すことはない
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型のデータの四則演算は提供されていない

bool
Data::operateWith_NoCast(DataOperation::Type op, const Data& r)
{
	; _TRMEISTER_ASSERT(!isNull());
	; _TRMEISTER_ASSERT(!r.isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//
//	FUNCTION public
//	Common::Data::getElementType -- 要素のデータ型を得る
//
//	NOTES
//	要素のデータ型を得る
//	Array以外のデータについてはUndefinedを返す
//Undefined is returned about data other than Array that obtains the data type of the element. 
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::DataType::Type
//		データ型
//
//	EXCEPTIONS
//	なし
//
DataType::Type
Data::
getElementType() const
{
	// ArrayDataでオーバーライドする
	return DataType::Undefined;
}

//
//	FUNCTION public
//	Common::Data::getFunction -- 付加機能を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Data::Function::Value
//		付加機能を表す列挙子
//
//	EXCEPTIONS
//	なし
//
Data::Function::Value
Data::
getFunction() const
{
	return m_eFunction;
}

//
//	FUNCTION public
//	Common::Data::setFunction -- 付加機能を設定する
//
//	NOTES
//
//	ARGUMENTS
//	Common::Data::Function::Value eFunction_
//		付加機能を表す列挙子
//
//	RETURN
//
//	EXCEPTIONS
//	なし
//
void
Data::
setFunction(Function::Value eFunction_)
{
	m_eFunction |= eFunction_;
}

//
//	FUNCTION public
//	Common::Data::between -- 範囲に含まれるかどうか
//
//	NOTES
//	範囲に含まれるかどうか
//
//	ARGUMENTS
//	const Common::Data::Pointer pFrom_
//		ここから
//	const Common::Data::Pointer pTo_
//		ここまで
//
//	RETURN
//	範囲に含まれる場合ははtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Data::between(const Data* pFrom_, const Data* pTo_) const
{
	try {
		return compareTo(pFrom_) >= 0 && compareTo(pTo_) <= 0;

	} catch (Exception::Object&) {

		// 例外を無視する

		;
	}
	return false;
}

//
//	FUNCTION public
//	Common::Data::between -- 範囲に入っているかどうか
//
//	NOTES
//	範囲に入っているかどうか
//
//	ARGUMENTS
//	const Common::ArrayData* pSpan_
//		要素数2のDataArrayData
//
//	RETURN
//	入っている場合はtrue、それ以外はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Data::
between(const ArrayData* pSpan_) const
{
	bool bResult = false;

	if (pSpan_ && pSpan_->getElementType() == DataType::Data) {
		const DataArrayData* pSpan =
			_SYDNEY_DYNAMIC_CAST(const DataArrayData*, pSpan_);

		if (pSpan->getCount() == 2)
		{
			const Data* pFrom = pSpan->getElement(0).get();
			const Data* pTo = pSpan->getElement(1).get();
			bResult = between(pFrom, pTo);
		}
	}
	return bResult;
}

//
//	FUNCTION public
//	Common::Data::in -- 包含されているかどうか
//
//	NOTES
//	pData_の各要素中に一致するものがあるかどうか
//
//	ARGUMENTS
//	const Data::Data* pData_
//		DataArrayData
//
//	RETURN
//	一致するものが存在した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Data::
in(const Data* pData_) const
{
	bool bResult = false;
	if (pData_ && pData_->getType() == DataType::Array
		&& pData_->getElementType() == DataType::Data) {

		const DataArrayData* pDataArrayData =
			_SYDNEY_DYNAMIC_CAST(const DataArrayData*, pData_);

		int iCount = pDataArrayData->getCount();
		for (int i = 0; i < iCount; i++)
		{
			bResult = equals(pDataArrayData->getElement(i).get());
			if (bResult == true)
				break;
		}
	}

	return bResult;
}

//	FUNCTION public
//	Common::Data::toString -- 文字列で取り出す
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		取り出した文字列
//
//	EXCEPTIONS

ModUnicodeString
Data::toString() const
{
	return getString();
}

//	FUNCTION public
//	Common::Data::isApplicable -- 付加機能を適用可能かを得る
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data::Function::Value	function
//			調べる付加機能
//
//	RETURN
//		true
//			適用できる
//		false
//			適用できない
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

bool
Data::isApplicable(Function::Value function)
{
	if (isNull())
		// nullには付加機能を適用できない
		return false;

	return isApplicable_NotNull(function);
}

bool
Data::isApplicable_NotNull(Function::Value function)
{
	; _TRMEISTER_ASSERT(!isNull());

	return false;
}

//	FUNCTION public
//	Common::Data::apply -- 付加機能を適用したデータを得る
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data::Function::Value	function
//			適用する付加機能
//
//	RETURN
//		付加機能を適用した Common::Data
//
//	EXCEPTIONS
//		Exception::NullNotAllowed
//			自分自身は NULL 値である

Data::Pointer
Data::apply(Function::Value function)
{
	if (isNull())
		_TRMEISTER_THROW0(Exception::NullNotAllowed);

	return apply_NotNull(function);
}

Data::Pointer
Data::apply_NotNull(Function::Value function)
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::isAbleToDump -- ダンプできるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ダンプできる
//		false
//			ダンプできない
//
//	EXCEPTIONS
//		なし

bool
Data::isAbleToDump() const
{
	return (isNull()) ?
		NullData::getInstance()->isAbleToDump() : isAbleToDump_NotNull();
}

bool
Data::isAbleToDump_NotNull() const
{
	return false;
}

//	FUNCTION public
//	Common::Data::isFixedSize -- 常に固定長であるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			固定長である
//		false
//			固定長でない
//
//	EXCEPTIONS
//		なし

bool
Data::isFixedSize() const
{
	return (isNull()) ?
		NullData::getInstance()->isFixedSize() : isFixedSize_NotNull();
}

bool
Data::isFixedSize_NotNull() const
{
	return false;
}

//	FUNCTION public
//	Common::Data::getDumpSize -- ダンプサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたダンプサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
Data::getDumpSize() const
{
	return (isNull()) ?
		NullData::getInstance()->getDumpSize() : getDumpSize_NotNull();
}

ModSize
Data::getDumpSize_NotNull() const
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::setDumpedValue --
//		dumpされたデータから自身の値をsetするSet does the value from data that dump is done. 
//
//	NOTES
//
//	ARGUMENTS
//		const char* pszValue_
//			dumpされた領域の先頭Head of area where dump was done
//		ModSize uSize_(省略可)
//			サイズを指定する必要のあるデータの場合指定するIt specifies it for data that should specify the size. 
//
//	RETURN
//		値を返すことはない
//
//	EXCEPTIONS
//		Exception::NotSupported
//			この型のデータにダンプデータを設定できない

ModSize
Data::setDumpedValue(const char* pszValue_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

ModSize
Data::setDumpedValue(const char* pszValue_, ModSize uSize_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::dumpValue -- 自分の値を与えられた領域にダンプするIt dumps it to the area to which my value was given. 
//
//	NOTES
//
//	ARGUMENTS
//		char* 		dst
//			自分の値をダンプする領域の先頭Head of area where my value is dumped
//
//	RETURN
//		ダンプサイズ(B 単位)
//
//	EXCEPTIONS

ModSize
Data::dumpValue(char* dst) const
{
	return (isNull()) ?
		NullData::getInstance()->dumpValue(dst) : dumpValue_NotNull(dst);
}


ModSize
Data::setDumpedValue(ModSerialIO& cSerialIO_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

ModSize 
Data::setDumpedValue(ModSerialIO& cSerialIO_, ModSize uSize_)
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

ModSize 
Data::dumpValue(ModSerialIO& cSerialIO_) const
{
	return (isNull()) ?
		NullData::getInstance()->dumpValue(cSerialIO_) : dumpValue_NotNull(cSerialIO_);
}

ModSize
Data::dumpValue_NotNull(ModSerialIO& cSerialIO_) const
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

ModSize
Data::dumpValue_NotNull(char* dst) const
{
	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::isFixedSize -- あるデータ型のデータが固定長か調べる
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			調べるデータのデータ型
//
//	RETURN
//		true
//			固定長である
//		false
//			可変長である
//
//	EXCEPTIONS
//		なし

//static
bool
Data::isFixedSize(DataType::Type type)
{
	return (isScalar(type)) ?
		_TypeTable[type - DataType::MinScalar]._fixed : false;
}

//	FUNCTION public
//	Common::Data::isAbleToDump --
//		あるデータ型のデータがダンプ可能かを得る
//
//	NOTES
//		Scalar型のデータ以外にはダンプ可能でもfalseを返す
//		それ以外のデータ型についてはインスタンスメソッドの方を使う
//
//	ARGUMENTS
//		Common::DataType::Type eType_
//			調べるデータのデータ型
//
//	RETURN
//		true	ダンプ可能である
//		false	ダンプ可能でない、または型だけからは調べられない
//
//	EXCEPTIONS
//		なし

//static
bool
Data::isAbleToDump(DataType::Type type)
{
	return (isScalar(type)) ?
		_TypeTable[type - DataType::MinScalar]._ableToDump : false;
}

// FUNCTION public
//	Common::Data::isCompatible -- データ型のペアがどちらかにキャスト可能かを得る
//
// NOTES
//
// ARGUMENTS
//	DataType::Type eType1_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Data::
isCompatible(DataType::Type eType1_, DataType::Type eType2_)
{
	return _Compatible::getValue(eType1_, eType2_) != _Compatible::NotCompatible;
}

//	FUNCTION public
//	Common::Data::getDumpSize --
//		あるデータ型のデータをダンプしたときのサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type eType_
//			調べるデータのデータ型
//
//	RETURN
//		得られたサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

//static
ModSize
Data::getDumpSize(DataType::Type type)
{
	return (isFixedSize(type)) ?
		_TypeTable[type - DataType::MinScalar]._size : 0;
}

// FUNCTION public
//	Common::Data::isCompatible -- 型の互換性を得る
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
Data::
isCompatible(const Data* r) const
{
	; _SYDNEY_ASSERT(r);
	// default: compare datatype
	return getType() == r->getType()
		&& getElementType() == r->getElementType();
}

// FUNCTION public
//	Common::Data::getSQLType -- データ型からSQLDataを得る
//
// NOTES
//
// ARGUMENTS
//	DataType::Type eType_
//	SQLData& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Data::
getSQLType(DataType::Type eType_, SQLData& cResult_)
{
	if (isScalar(eType_)) {
		// テーブルからデータを得る
		cResult_ =  _SQLDataType::_Table[eType_ - DataType::MinScalar];
		return cResult_.getType() != SQLData::Type::NoType;
	}
	if (eType_ == DataType::Null || eType_ == DataType::Array) {
		cResult_ = SQLData();
		return true;
	}
	return false;
}

// FUNCTION public
//	Common::Data::getSQLType -- データからSQLDataを得る
//
// NOTES
//
// ARGUMENTS
//	SQLData& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Data::
getSQLType(SQLData& cResult_)
{
	if (getSQLType(getType(), cResult_))
		return getSQLTypeByValue(cResult_);

	return false;
}

// FUNCTION public
//	Common::Data::setSQLType -- クラス内部構造をSQLTypeに合わせる
//
// NOTES
//
// ARGUMENTS
//	const SQLData& cType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Data::
setSQLType(const SQLData& cType_)
{
	// default: なにもしない
	;
}

//	FUNCTION public
//	Common::Data::getClassID -- クラスIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたクラスID
//
//	EXCEPTIONS
//		なし

int
Data::getClassID() const
{
	return (isNull()) ?
		NullData::getInstance()->getClassID() : getClassID_NotNull();
}

int
Data::getClassID_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Common::Data::print -- 値を標準出力へ出力する
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

void
Data::print() const
{
	if (isNull())
		NullData::getInstance()->print();
	else
		print_NotNull();
}

void
Data::print_NotNull() const
{
	; _TRMEISTER_ASSERT(!isNull());

	_TRMEISTER_THROW0(Exception::NotSupported);
}

// 実際の値を用いてSQLDataを得る(getSQLTypeの下請け)
//virtual
bool
Data::
getSQLTypeByValue(SQLData& cResult_)
{
	// defaultは何もしないで成功を意味するtrueを返す
	return true;
}

//
//	Copyright (c) 1999, 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
