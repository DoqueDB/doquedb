// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.h -- Definition file of Data
// 
// Copyright (c) 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __DATA__HEADER__
#define __DATA__HEADER__

#include "Module.h"
#include "Algorithm.h"
#include "SmartPointer.h"
#include "ModAutoPointer.h"
#include "ModCharString.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModUnicodeRegularExpression.h"
#include "ModVector.h"

// Data namespace を使うための define
#define	_DATA_BEGIN		namespace Data {
#define _DATA_END		}
#define _DATA_USING		using namespace Data;

// 空定義
//namespace UNA {
//	class Bitset; 
//}

_UNA_BEGIN 

class Bitset; 
_DATA_BEGIN

//////////////////////////////////////////////////////////////////////////////
//
//	CLASS
//		Data::Object -- データクラスの基本クラス
//
class Object
{
public:

	// ENUM
	//		Type -- データ種別
	enum Type {
		Bool,							// bool 値
		Int,							// 整数値
		String,							// 文字列値
		UnicodeString,					// Unicode 文字列値
		Vector,							// 配列値
		IntVector,						// 整数配列値
		StringVector,					// 文字列配列値
		UnicodeStringVector,			// Unicode 文字列配列値
		RegularExpression,				// 品詞正規表現値
		BitsetOperator,					// Bitset 用オペレータ
		CalcMaterial,					// Common::Type::CalcMaterial 用
		Num
	};

	// コンストラクタ、デストラクタ
	explicit Object(Type type);
	virtual ~Object();

	// 種別取得
	Type			getType() const;

	// 文字列化
	virtual
	ModUnicodeString	getMessage() const = 0;

protected:
private:

	Type			_type;
};

//////////////////////////////////////////////////////////////////////////////
//
//	CLASS
//		DataDefinition -- 型定義の為のラップクラス
//
template < class T , Object::Type type>
class DataDefinition : public Object
{
public:
	DataDefinition();
	explicit DataDefinition(T& value_);
	explicit DataDefinition(const T& value_);
	~DataDefinition();

	// 値設定
	T&			getData();
	const T&		getData() const;
	void			setData(const T& value_);

 	// 文字列化
	ModUnicodeString getMessage() const;

	// キャスト演算子
	operator T ();

protected:
	// データ値
	T			_value;
};

//
// FUNCTION public
//	DataDefinition
//		-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS

template<class T , Object::Type type>
inline
DataDefinition<T, type>::DataDefinition() 
		 : Object(Object::Num)
{
}

template<class T , Object::Type type>
inline
DataDefinition<T, type>::DataDefinition(T& value_)
		 : _value(value_), Object(type) 
{
}

template<class T , Object::Type type>
inline
DataDefinition<T, type>::DataDefinition(const T& value_)
		 : _value(value_), Object(type) 
{
}

//
// FUNCTION public
//	DataDefinition
//		-- デストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
template<class T , Object::Type type>
inline
DataDefinition<T, type>::~DataDefinition() 
{
}

//
// FUNCTION public
//	DataDefinition::getData
//		-- データ値取得
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	T&			データ値
//	const T&	データ値
//
// EXCEPTIONS
//
template<class T , Object::Type type>
inline
T&
DataDefinition<T, type>::getData()
{
	return _value;
}
template<class T , Object::Type type >
inline
const T&
DataDefinition<T, type>::getData() const
{
	return _value;
}

//
// FUNCTION public
//	DataDefinition::setData
//		-- データ値設定
//
// NOTES
//
// ARGUMENTS
//	const T& value_
//
// RETURN
//	なし
//
// EXCEPTIONS
//
template<class T, Object::Type type >
inline
void
DataDefinition<T, type>::setData(const T& value_)
{
	_value = value_;
}

//
// FUNCTION public
//	DataDefinition::setData
//		-- データ値設定
//
// NOTES
//
// ARGUMENTS
//	const T& value_
//
// RETURN
//
// EXCEPTIONS
//
template<class T, Object::Type type >
inline
DataDefinition<T, type>::operator T ()
{
	return _value;
}

//
// FUNCTION public
//	DataDefinition::getMessage
//		-- キャスト演算子
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	T
//		内部データ
//
// EXCEPTIONS
//
template <class T, Object::Type type >
inline
ModUnicodeString
DataDefinition<T, type>::getMessage() const
{
	ModUnicodeOstrStream str;
	str << _value;
	return str.getString();
}

//////////////////////////////////////////////////////////////////////////////
//	TYPEDEF
// 		-- 各データ型の定義
typedef		DataDefinition<ModBoolean, Object::Bool>
												Bool;
typedef		DataDefinition<int, Object::Int>	Int;
typedef		DataDefinition<ModCharString, Object::String>
												String;
typedef		DataDefinition<ModUnicodeString, Object::UnicodeString>
												UnicodeString;
//typedef	DataDefinition< ModVector<Common::SmartPointer<Object> >, Object::Vector >
//												Vector;
typedef		DataDefinition< ModVector<int> , Object::IntVector>
												IntVector;
typedef		DataDefinition< ModVector<ModCharString> , Object::StringVector>
												StringVector;
typedef		DataDefinition< ModVector<ModUnicodeString> , Object::UnicodeStringVector>
												UnicodeStringVector;
//typedef	DataDefinition< ModAutoPointer<ModUnicodeRegularExpression>, Object::RegularExpression>
//												RegularExpression;
typedef		DataDefinition< Type::RegularExpression, Object::RegularExpression >
												RegularExpression;
typedef		DataDefinition< Algorithm::Operator<Bitset>, Object::BitsetOperator >
												BitsetOperator;
typedef		DataDefinition< Type::CalcMaterial, Object::CalcMaterial >
												CalcMaterialOperator;

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION public
//	Bool::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
Bool::getMessage() const
{
	ModUnicodeOstrStream str;
	str << (_value ? "true" : "false");
	return str.getString();
}

//
// FUNCTION public
//	UnicodeString::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
UnicodeString::getMessage() const
{
	return _value;
}

#ifdef OBSOLETE
//
// FUNCTION public
//	IntVector::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
IntVector::getMessage() const
{
	ModUnicodeOstrStream str;
	str << "array{";
	ModVector<int>::ConstIterator it  = _value.begin();
	ModVector<int>::ConstIterator fin = _value.end();
	for ( ; it != fin; ++it ) {
		str << *it << " , ";
	}
	str << "}";

	return str.getString();
}

//
// FUNCTION public
//	StringVector::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
StringVector::getMessage() const
{
	ModUnicodeOstrStream str;
	str << "array{";
	ModVector<ModCharString>::ConstIterator it  = _value.begin();
	ModVector<ModCharString>::ConstIterator fin = _value.end();
	for ( ; it != fin; ++it ) {
		str << *it << " , ";
	}
	str << "}";

	return str.getString();
}

//
// FUNCTION public
//	UnicodeString::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
UnicodeStringVector::getMessage() const
{
	ModUnicodeOstrStream str;
	str << "array{";
	ModVector<ModUnicodeString>::ConstIterator it  = _value.begin();
	ModVector<ModUnicodeString>::ConstIterator fin = _value.end();
	for ( ; it != fin; ++it ) {
		str << *it << " , ";
	}
	str << "}";

	return str.getString();
}
#endif

//
// FUNCTION public
//	RegularExpression::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
RegularExpression::getMessage() const
{
	// 評価文字列を取ってくる手段を知らないので omit
	return ModUnicodeString("no implement");
}

//
// FUNCTION public
//	BitsetOperator::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
BitsetOperator::getMessage() const
{
	return ModUnicodeString("Bitset operator");
}

#ifdef OBSOLETE
//
// FUNCTION public
//	CalcMaterialOperator::getMessage
//		-- 文字化されたデータを取得する
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	ModUnicodeString
//		文字化されたデータ
//
// EXCEPTIONS
//
ModTemplateNull
inline
ModUnicodeString
CalcMaterialOperator::getMessage() const
{
	using namespace UNA::Type;

	ModUnicodeOstrStream str;
	switch ( _value._type ) {
	case CalcType::Add:		str << "+"; 	break;
	case CalcType::Sub:		str << "-"; 	break;
	case CalcType::Multi:		str << "*";	break;
	case CalcType::Div:		str << "/";	break;
	}
	str << _value._material;
	return str.getString();
}
#endif

_DATA_END

_UNA_END

#endif // __DATA__HEADER__

//
// Copyright (c) 2004, 2005, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
