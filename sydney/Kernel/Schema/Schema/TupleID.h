// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TupleID.h -- タプル ID 関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_TUPLEID_H
#define	__SYDNEY_SCHEMA_TUPLEID_H

#include "Schema/Module.h"
#include "Common/UnsignedIntegerData.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Table;

//	CLASS
//	Schema::TupleID -- タプル ID を表すクラス
//
//	NOTES
//		タプル ID はシーケンスによって生成するので、
//		タプル ID の値の型はシーケンスが生成可能な値を
//		表現可能でなければならない
//		ほとんどエクスポートされるのでクラスごとエクスポートする

class SYD_SCHEMA_FUNCTION TupleID
	: public	Common::UnsignedIntegerData
{
public:
	typedef Common::UnsignedIntegerData Super;

	//	TYPEDEF
	//	Schema::TupleID::Value -- タプル ID の値を表す型
	//
	//	NOTES

	typedef unsigned int	Value;

	//	CONST
	//	Schema::TupleID::Invalid -- 不正なタプル ID の値
	//
	//	NOTES

	enum
	{
		Invalid =			~static_cast<Value>(0)
	};

	TupleID();
	TupleID(Value v);							// コンストラクター

	Value					operator *() const;	// * 単項演算子

	TupleID&				operator =(Value src);
												// = 演算子
	operator				Value() const;		// Value へのキャスト演算子

	bool					operator ==(Value r) const;
												// == 演算子
	bool					operator !=(Value r) const;
												// != 演算子

	using Super::assign;
	static Value			assign(Table& table,
								   Trans::Transaction& cTrans_);
	static Value			assign(Table& table,
								   Value iID_,
								   Trans::Transaction& cTrans_);
												// ある表に新しいタプル ID の
												// 値を生成する
	static void				persist(Table& cTable_,
									Trans::Transaction& cTrans_);
												// タプルIDを永続化する

	ModSize					hashCode() const;	// ハッシュ値を計算する

	bool					isInvalid() const;	// 不正なタプル ID か
};

//	FUNCTION public
//	Schema::TupleID::TupleID --
//		タプル ID を表すクラスのデフォルトコンストラクター
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
TupleID::
TupleID()
	: Common::UnsignedIntegerData(Invalid)
{ }

//	FUNCTION public
//	Schema::TupleID::TupleID --
//		指定された値のタプル ID を表すクラスのコンストラクター
//
//	NOTES
//		explicit で定義していないので、
//		Schema::TupleID::Value から Schema::TupleID への
//		暗黙の変換を定義することになる
//
//	ARGUMENTS
//		Schema::TupleID::Value	value
//			生成するタプル ID の値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TupleID::
TupleID(Schema::TupleID::Value value)
	: Common::UnsignedIntegerData(value)
{ }

//	FUNCTION public
//	Schema::TupleID::operator * == * 単項演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身の値
//
//	EXCEPTIONS
//		なし

inline
TupleID::Value
TupleID::
operator *() const
{
	return getValue_NotNull();
}

//	FUNCTION public
//	Schema::TupleID::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::TupleID::Value	src
//			自分自身の値として代入するタプル ID の値
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
TupleID&
TupleID::
operator =(Value src)
{
	setValue(src);
	return *this;
}

//	FUNCTION public
//	Schema::TupleID::operator Schema::TupleID::Value --
//		Schema::TupleID::Value へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身の表す値
//
//	EXCEPTIONS
//		なし

inline
TupleID::
operator TupleID::Value() const
{
	return getValue_NotNull();
}

//	FUNCTION public
//	Schema::TupleID::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::TupleID::Value	r
//			自分自身と比較するタプル ID の値
//
//	RETURN
//		true
//			自分自身と与えられたタプル ID の値は等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
TupleID::
operator ==(Value r) const
{
	return *(*this) == r;
}

//	FUNCTION public
//	Schema::TupleID::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::TupleID::Value	r
//			自分自身と比較するタプル ID の値
//
//	RETURN
//		true
//			自分自身と与えられたタプル ID の値は等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
TupleID::
operator !=(Value r) const
{
	return *(*this) != r;
}

//	FUNCTION public
//	Schema::TupleID::hashCode -- ハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

inline
ModSize
TupleID::
hashCode() const
{
	return getValue_NotNull() % ~((ModSize) 0);
}

//	FUNCTION public
//	Schema::TupleID::isInvalid -- 不正なタプル ID か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			自分自身は不正なタプル ID である
//		false
//			自分自身は不正なタプル ID でない
//
//	EXCEPTIONS
//		なし

inline
bool
TupleID::
isInvalid() const
{
	return *(*this) == Invalid;
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_TUPLEID_H

//
// Copyright (c) 2000, 2001, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
