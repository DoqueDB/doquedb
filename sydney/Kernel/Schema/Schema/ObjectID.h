// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectID.h -- スキーマオブジェクト ID 関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_OBJECTID_H
#define	__SYDNEY_SCHEMA_OBJECTID_H

#include "Schema/Module.h"
#include "Schema/Externalizable.h"

#include "Common/UnsignedIntegerData.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Trans {
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;

//	CLASS
//	Schema::ObjectID -- スキーマオブジェクト ID を表すクラス
//
//	NOTES
//		スキーマオブジェクト ID はシーケンスによって生成するので、
//		スキーマオブジェクト ID の値の型はシーケンスが生成可能な値を
//		表現可能でなければならない
//		ほとんどがエクスポートされるのでクラスごとエクスポートする

class SYD_SCHEMA_FUNCTION ObjectID
	: public	Common::UnsignedIntegerData
#ifdef OBSOLETE
	, public	Externalizable
#endif
{
public:
	typedef Common::UnsignedIntegerData Super;

	//	TYPEDEF
	//	Schema::ObjectID::Value -- オブジェクト ID の値を表す型
	//
	//	NOTES

	typedef	unsigned int		Value;

	//	CONST
	//	Schema::ObjectID::SystemTable -- システム表のオブジェクト ID の値
	//
	//	NOTES
	//		ObjectIDの初期値でもある

	//	CONST
	//	Schema::Manager::ObjectID::Invalid -- 不正なオブジェクト ID の値
	//
	//	NOTES

	enum
	{
		SystemTable =	static_cast<Value>(0),
		Invalid		=	~static_cast<Value>(0)
	};

	ObjectID();
	ObjectID(Value value);						// コンストラクター

	Value					operator *() const;	// * 単項演算子

	ObjectID&				operator =(Value src);
												// = 演算子
	operator				Value() const;		// Value へのキャスト演算子

	bool					operator ==(Value r) const;
												// == 演算子
	bool					operator !=(Value r) const;
												// != 演算子
	bool					operator <(Value r) const;
												// < 演算子

	using Super::assign;
	static Value			assign(Trans::Transaction& cTrans_,
								   Database* pDatabase_, Value iID_ = Invalid,
								   Value iInitID_ = Invalid);
												// 新しいオブジェクト ID の
												// 値を生成する
	static void				persist(Trans::Transaction& cTrans_,
									Database* pDatabase_);
												// オブジェクトIDを永続化する
	static Value			getLastValue(Trans::Transaction& cTrans_,
										 Database* pDatabase_);
												// 最後のオブジェクト ID の
												// 値を得る

	ModSize					hashCode() const;	// ハッシュ値を計算する

	bool					isSystemTable() const;
												// システム表の
												// オブジェクト ID か
	bool					isInvalid() const;	// 不正なオブジェクト ID か

#ifdef OBSOLETE // serializeしない
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	virtual int				getClassID() const;	// このクラスのクラス ID を得る
#endif
};

//	FUNCTION public
//	Schema::ObjectID::ObjectID --
//		オブジェクト ID を表すクラスのデフォルトコンストラクター
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
ObjectID::
ObjectID()
	: Common::UnsignedIntegerData(Invalid)
{ }

//	FUNCTION public
//	Schema::ObjectID::ObjectID --
//		指定された値のオブジェクト ID を表すクラスのコンストラクター
//
//	NOTES
//		explicit で定義していないので、
//		Schema::ObjectID::Value から Schema::ObjectID への
//		暗黙の変換を定義することになる
//
//	ARGUMENTS
//		Schema::ObjectID::Value	value
//			生成するオブジェクト ID の値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ObjectID::
ObjectID(Value value)
	: Common::UnsignedIntegerData(value)
{ }

//	FUNCTION public
//	Schema::ObjectID::operator * == * 単項演算子
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
ObjectID::Value
ObjectID::
operator *() const
{
	return getValue_NotNull();
}

//	FUNCTION public
//	Schema::ObjectID::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	src
//			自分自身の値として代入するオブジェクト ID の値
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
ObjectID&
ObjectID::
operator =(Value src)
{
	setValue(src);
	return *this;
}

//	FUNCTION public
//	Schema::ObjectID::operator Schema::ObjectID::Value --
//		Schema::ObjectID::Value へのキャスト演算子
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
ObjectID::
operator ObjectID::Value() const
{
	return getValue_NotNull();
}

//	FUNCTION public
//	Schema::ObjectID::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	r
//			自分自身と比較するオブジェクト ID の値
//
//	RETURN
//		true
//			自分自身と与えられたオブジェクト ID の値は等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
ObjectID::
operator ==(Value r) const
{
	return *(*this) == r;
}

//	FUNCTION public
//	Schema::ObjectID::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	r
//			自分自身と比較するオブジェクト ID の値
//
//	RETURN
//		true
//			自分自身と与えられたオブジェクト ID の値は等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
ObjectID::
operator !=(Value r) const
{
	return *(*this) != r;
}

//	FUNCTION public
//	Schema::ObjectID::operator < -- < 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	r
//			自分自身と比較するオブジェクト ID の値
//
//	RETURN
//		true
//			自分自身は与えられたオブジェクト ID より小さい
//		false
//			自分自身は小さくない
//
//	EXCEPTIONS
//		なし

inline
bool
ObjectID::
operator <(Value r) const
{
	return *(*this) < r;
}

//	FUNCTION public
//	Schema::ObjectID::hashCode -- ハッシュ値を計算する
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
ObjectID::
hashCode() const
{
	return getValue_NotNull() % ~((ModSize) 0);
}

//	FUNCTION public
 //	Schema::ObjectID::isSystemTable -- システム表のオブジェクト ID か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			自分自身はシステム表のオブジェクト ID である
//		false
//			自分自身はシステム表のオブジェクト ID でない
//
//	EXCEPTIONS
//		なし

inline
bool
ObjectID::
isSystemTable() const
{
	return *(*this) == SystemTable;
}

//	FUNCTION public
//	Schema::ObjectID::isInvalid -- 不正なオブジェクト ID か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			自分自身は不正なオブジェクト ID である
//		false
//			自分自身は不正なオブジェクト ID でない
//
//	EXCEPTIONS
//		なし

inline
bool
ObjectID::
isInvalid() const
{
	return *(*this) == Invalid;
}

#ifdef OBSOLETE // serializeしない
//	FUNCTION public
//	Schema::ObjectID::getClassID -- このクラスのクラス ID を得る
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
int
ObjectID::
getClassID() const
{
	return Externalizable::Category::ObjectID +
		Common::Externalizable::SchemaClasses;
}
#endif

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_OBJECTID_H

//
// Copyright (c) 2000, 2001, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
