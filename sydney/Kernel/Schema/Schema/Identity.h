// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Identity.h -- タプル ID 関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_IDENTITY_H
#define	__SYDNEY_SCHEMA_IDENTITY_H

#include "Schema/Module.h"
#include "Common/IntegerData.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Column;

//	CLASS
//	Schema::Identity -- IDENTITY COLUMNの値を表すクラス
//
//	NOTES
//		IDENTITY COLUMNの値はシーケンスによって生成するので、
//		IDENTITY COLUMNの値の型はシーケンスが生成可能な値を
//		表現可能でなければならない
//		ほとんどエクスポートされるのでクラスごとエクスポートする

class SYD_SCHEMA_FUNCTION Identity
	: public	Common::IntegerData
{
public:
	typedef Common::IntegerData Super;

	//	TYPEDEF
	//	Schema::Identity::Value -- IDENTITY COLUMNの値を表す型
	//
	//	NOTES

	typedef int				Value;

	//	CONST
	//	Schema::Identity::Invalid -- 不正なIDENTITY COLUMNの値
	//
	//	NOTES

	enum
	{
		Invalid =			ModInt32Min
	};

	// コンストラクター
	Identity();
	Identity(Value v);

	// * 単項演算子
	Value					operator *() const;

	// = 演算子
	Identity&				operator =(Value src);
	// Value へのキャスト演算子
	operator				Value() const;

	// == 演算子
	bool					operator ==(Value r) const;
	// != 演算子
	bool					operator !=(Value r) const;

	// ある表に新しいIDENTITY COLUMNの値を生成する
	using Super::assign;
	static Value			assign(Column& cColumn_,
								   Trans::Transaction& cTrans_);
	static Value			assign(Column& cColumn_,
								   Value iID_,
								   Trans::Transaction& cTrans_);
	// IDENTITY COLUMNの値を永続化する
	static void				persist(Column& cColumn_,
									Trans::Transaction& cTrans_);
	// 常に最大値を生成するかを得る
	static bool				isGetMax(Column& cColumn_,
									 Trans::Transaction& cTrans_);

	// 不正な値か
	bool					isInvalid() const;
};

//	FUNCTION public
//	Schema::Identity::Identity --
//		IDENTITY COLUMNの値 を表すクラスのデフォルトコンストラクター
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
Identity::
Identity()
	: Super(Invalid)
{ }

//	FUNCTION public
//	Schema::Identity::Identity --
//		指定された値のIDENTITY COLUMNの値 を表すクラスのコンストラクター
//
//	NOTES
//		explicit で定義していないので、
//		Schema::Identity::Value から Schema::Identity への
//		暗黙の変換を定義することになる
//
//	ARGUMENTS
//		Schema::Identity::Value	value
//			生成するIDENTITY COLUMNの値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Identity::
Identity(Schema::Identity::Value value)
	: Super(value)
{ }

//	FUNCTION public
//	Schema::Identity::operator * == * 単項演算子
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
Identity::Value
Identity::
operator *() const
{
	return getValue();
}

//	FUNCTION public
//	Schema::Identity::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Identity::Value	src
//			自分自身の値として代入するIDENTITY COLUMNの値
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
Identity&
Identity::
operator =(Value src)
{
	setValue(src);
	return *this;
}

//	FUNCTION public
//	Schema::Identity::operator Schema::Identity::Value --
//		Schema::Identity::Value へのキャスト演算子
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
Identity::
operator Identity::Value() const
{
	return getValue();
}

//	FUNCTION public
//	Schema::Identity::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Identity::Value	r
//			自分自身と比較するIDENTITY COLUMNの値
//
//	RETURN
//		true
//			自分自身と与えられたIDENTITY COLUMNの値は等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
Identity::
operator ==(Value r) const
{
	return *(*this) == r;
}

//	FUNCTION public
//	Schema::Identity::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Identity::Value	r
//			自分自身と比較するIDENTITY COLUMNの値
//
//	RETURN
//		true
//			自分自身と与えられたIDENTITY COLUMNの値は等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
Identity::
operator !=(Value r) const
{
	return *(*this) != r;
}

//	FUNCTION public
//	Schema::Identity::isInvalid -- 不正なIDENTITY COLUMNの値か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			自分自身は不正なIDENTITY COLUMNの値である
//		false
//			自分自身は不正なIDENTITY COLUMNの値でない
//
//	EXCEPTIONS
//		なし

inline
bool
Identity::
isInvalid() const
{
	return *(*this) == Invalid;
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_IDENTITY_H

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
