// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.cpp -- パラメータクラス
// 
// Copyright (c) 1999, 2001, 2003, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ClassID.h"
#include "Common/Parameter.h"
#ifdef OBSOLETE
#include "Common/SystemParameter.h"
#endif
#include "Common/UnicodeString.h"

#include "ModMessage.h"
#include "ModUnicodeOstrStream.h"

_TRMEISTER_USING

namespace {
	//	CONST local
	//	_lMapSize -- Mapのコンストラクターの引数に渡す値
	//	_bMapEnableLink

	ModSize		_lMapSize = 7;
	ModBoolean	_bMapEnableLink = ModFalse;
}

//
//	VARIABLE priavte
//	Common::Parameter::Value::m_pszTrue  -- 真偽値をあらわす文字列
//	Common::Parameter::Value::m_pszFalse
//
//	NOTES
//	真偽値をあらわす文字列
//
const char* const Common::Parameter::Value::m_pszTrue = "TRUE";
const char* const Common::Parameter::Value::m_pszFalse = "FALSE";

//
//	FUNCTION public
//	Common::Parameter::Parameter -- コンストラクタ
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

Common::Parameter::Parameter()
	: m_mapParameter(_lMapSize, _bMapEnableLink)
{}

#ifdef OBSOLETE
//	FUNCTION public
//	Common::Parameter::getSystemValue --
//		システムパラメーターから文字列値を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&			key
//			このキーのシステムパラメーターの文字列値を得る
//		ModUnicodeString&		value
//			得られた文字列値を格納する
//
//	RETURN
//		true
//			値が得られた
//		false
//			値は得られなかった
//
//	EXCEPTIONS

bool
Common::Parameter::getSystemValue(const ModCharString& key,
								  ModUnicodeString& value)
{
	return SystemParameter::getValue(key, value);
}

//	FUNCTION public
//	Common::Parameter::getSystemValue --
//		システムパラメーターから整数値を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		key
//			このキーのシステムパラメーターの整数値を得る
//		int&				value
//			得られた整数値を格納する
//
//	RETURN
//		true
//			値が得られた
//		false
//			値は得られなかった
//
//	EXCEPTIONS

bool
Common::Parameter::getSystemValue(const ModCharString& key, int& value)
{
	return SystemParameter::getValue(key, value);
}

//	FUNCTION public
//	Common::Parameter::getSystemValue --
//		システムパラメーターから真偽値を得る
//
//	NOTES
//
//	ARGUMENTS
//		ModCharString&		key
//			このキーのシステムパラメーターの真偽値を得る
//		bool&				value
//			得られた真偽値を格納する
//
//	RETURN
//		true
//			値が得られた
//		false
//			値は得られなかった
//
//	EXCEPTIONS

bool
Common::Parameter::getSystemValue(const ModCharString& key, bool& value)
{
	return SystemParameter::getValue(key, value);
}

//
//	FUNCTION public staitc
//	Common::Parameter::getSystemString -- システムパラメータを得る
//
//	NOTES
//	文字列型のシステムパラメータを得る
//
//	ARGUMETNS
//	const ModCharString& cstrKey_
//		キー
//
//	RETURN
//	文字列の値。存在しない場合は、空文字列
//
//	EXCEPTIONS
//	なし
//
ModUnicodeString
Common::Parameter::getSystemString(const ModCharString& cstrKey_)
{
	return SystemParameter::getString(cstrKey_);
}

//
//	FUNCTION public static
//	Common::Parameter::getSystemInteger -- システムパラメータを得る
//
//	NOTES
//	32ビット整数型のシステムパラメータを得る
//
//	ARGUMENTS
//	const ModCharString& 	cstrKey_
//		キー
//
//	RETURN
//	32ビット整数の値。存在しない場合は-1
//
//	EXCEPTIONS
//	なし
//
int
Common::Parameter::getSystemInteger(const ModCharString& cstrKey_)
{
	return SystemParameter::getInteger(cstrKey_);
}

//
//	FUNCTION public static
//	Common::Parameter::getSystemBoolean -- システムパラメータを得る
//
//	NOTES
//	真偽値型のシステムパラメータを得る
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//
//	RETURN
//	真偽値。存在しない場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
Common::Parameter::getSystemBoolean(const ModCharString& cstrKey_)
{
	return SystemParameter::getBoolean(cstrKey_);
}
#endif

//
//	FUNCTION public
//	Common::Parameter::getString -- 文字列を取り出す
//
//	NOTES
//	文字列を取り出す。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	ModUnicodeString& cstrValue_
//		値
//
//	RETURN
//	キーが存在しかつ型が同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::getString(const ModCharString& cstrKey_,
							 ModUnicodeString& cstrValue_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	if (i != m_mapParameter.end() && (*i).second.getType() == TypeString) {
		cstrValue_ = (*i).second.getString();
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::getInteger -- 32ビット整数を取り出す
//
//	NOTES
//	32ビット整数を取り出す。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	int& iValue_
//		値
//
//	RETURN
//	キーが存在しかつ型が同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::getInteger(
	const ModCharString& cstrKey_, int& iValue_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	if (i != m_mapParameter.end() && (*i).second.getType() == TypeInteger) {
		iValue_ = (*i).second.getInteger();
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::getLongLong -- 64ビット整数を取り出す
//
//	NOTES
//	64ビット整数を取り出す。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	ModInt64& llValue_
//		値
//
//	RETURN
//	キーが存在しかつ型が同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::getLongLong(const ModCharString& cstrKey_,
							   ModInt64& llValue_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	if (i != m_mapParameter.end() && (*i).second.getType() == TypeLongLong) {
		llValue_ = (*i).second.getLongLong();
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::getBoolean -- 真偽値を取り出す
//
//	NOTES
//	真偽値を取り出す。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	bool& bValue_
//		値
//
//	RETURN
//	キーが存在しかつ型が同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::getBoolean(const ModCharString& cstrKey_,
							  bool& bValue_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	if (i != m_mapParameter.end() && (*i).second.getType() == TypeBoolean) {
		bValue_ = (*i).second.getBoolean();
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::getDouble -- 倍精度浮動小数を取り出す
//
//	NOTES
//	倍精度浮動小数を取り出す。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	double& dValue_
//		値
//
//	RETURN
//	キーが存在しかつ型が同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::getDouble(const ModCharString& cstrKey_,
							 double& dValue_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	if (i != m_mapParameter.end() && (*i).second.getType() == TypeDouble) {
		dValue_ = (*i).second.getDouble();
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::getObjectPointer -- オブジェクトへのポインタを取り出す
//
//	NOTES
//	オブジェクトへのポインタを取り出す。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	const Common::Object*& pValue_
//		値
//
//	RETURN
//	キーが存在しかつ型が同じ場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::getObjectPointer(const ModCharString& cstrKey_,
									const Common::Object*& pValue_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	if (i != m_mapParameter.end() &&
		(*i).second.getType() == TypeObjectPointer) {
		pValue_ = (*i).second.getObjectPointer();
		return true;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::serialize -- シリアル化
//
//	NOTES
//	シリアル化する。
//	ただしオブジェクトポインタ(TypeObjectPointer)のエントリは0が書込まれる。
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Common::Parameter::serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//書出し
		//総数を書出す
		int number = m_mapParameter.getSize();
		cArchiver_ << number;
		for (Map::Iterator i = m_mapParameter.begin();
			 i != m_mapParameter.end(); ++i)
		{
			//パラメータ名を書く
			cArchiver_ << (*i).first;
			
			//値を書く
			cArchiver_ << (*i).second;
		}
	}
	else
	{
		//読出し
		//マップをクリアする
		m_mapParameter.erase(m_mapParameter.begin(), m_mapParameter.end());
		//総数を得る
		int number;
		cArchiver_ >> number;

		Map::ValueType	v;
		for (int i = 0 ; i < number; ++i)
		{
			//パラメータ名を得る
			cArchiver_ >> v.first;
			
			//値を得る
			cArchiver_ >> v.second;
			//マップにしまう
			m_mapParameter.insert(v);
		}
	}
}

//
//	FUNCTION public
//	Common::Parameter::getClassID -- クラスIDを得る
//
//	NOTES
//	クラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//	なし
//
int
Common::Parameter::getClassID() const
{
	return ClassID::ParameterClass;
}

//
//	FUNCTION public
//	Common::Parameter::toString -- 文字列で取出す
//
//	NOTES
//	文字列で取出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//  ModUnicodeString
//		文字列データ
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModUnicodeString
Common::Parameter::toString() const
{
	ModUnicodeOstrStream o;

	o << "Parameter : ";

	for (Map::ConstIterator i = m_mapParameter.begin();
		 i != m_mapParameter.end(); ++i)
	{
		o << "(Key:" << (*i).first << ",Type:";
		switch ((*i).second.getType())
		{
		case TypeNone:
			o << "None";
			break;
		case TypeString:
			o << "String";
			break;
		case TypeInteger:
			o << "Integer";
			break;
		case TypeLongLong:
			o << "LongLong";
			break;
		case TypeBoolean:
			o << "Boolean";
			break;
		case TypeDouble:
			o << "Double";
			break;
		case TypeObjectPointer:
			o << "ObjectPointer";
		}
		o << ",Value:" << (*i).second.getString() << ")";
	}
	
	return ModUnicodeString(o.getString());
}

//
//	以下 class Common::Parameter::Value の各メソッド
//

//
//	FUNCTION public
//	Common::Parameter::Value::Value -- コピーコンストラクタ
//
//	NOTES
//	コピーコンストラクタ
//
//	ARGUMENTS
//	const Common::Parameter::Value& cValue_
//		コピー元のオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Common::Parameter::Value::Value(
	const Value& cValue_)
: m_eType(cValue_.m_eType)
{
	switch (m_eType)
	{
	case TypeString:
		un.m_pstrValue = new ModUnicodeString(*cValue_.un.m_pstrValue);
		break;
	case TypeInteger:
		un.m_iValue = cValue_.un.m_iValue;
		break;
	case TypeLongLong:
		un.m_llValue = cValue_.un.m_llValue;
		break;
	case TypeBoolean:
		un.m_bValue = cValue_.un.m_bValue;
		break;
	case TypeDouble:
		un.m_dValue = cValue_.un.m_dValue;
		break;
	case TypeObjectPointer:
		un.m_pObject = cValue_.un.m_pObject;
		break;
	case TypeNone:
		break;
	}
}

//
//	FUNCTION public
//	Common::Parameter::Value::operator = -- 代入オペレータ
//
//	NOTES
//	代入オペレータ
//
//	ARGUMENTS
//	const Common::Parameter::Value& cValue_
//		代入するオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

Common::Parameter::Value&
Common::Parameter::Value::operator =(const Value& cValue_)
{
	switch (cValue_.getType()) {
	case TypeString:
		setString(*cValue_.un.m_pstrValue);		break;
	case TypeInteger:
		setInteger(cValue_.un.m_iValue);		break;
	case TypeLongLong:
		setLongLong(cValue_.un.m_llValue);		break;
	case TypeBoolean:
		setBoolean(cValue_.un.m_bValue);		break;
	case TypeDouble:
		setDouble(cValue_.un.m_dValue);			break;
	case TypeObjectPointer:
		setObjectPointer(cValue_.un.m_pObject);	break;
	case TypeNone:
		if (getType() == TypeString)
			delete un.m_pstrValue;
		m_eType = TypeNone;
	}

	return *this;
}

//
//	FUNCTION public
//	Common::Parameter::Value::getString -- 文字列を取り出す
//
//	NOTES
//	文字列を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	文字列の値。変換できない時は空文字列
//
//	EXCEPTIONS
//	なし
//

ModUnicodeString
Common::Parameter::Value::getString() const
{
	ModUnicodeOstrStream cStream;

	switch (getType()) {
	case TypeString:
		return *un.m_pstrValue;
	case TypeBoolean:
		return un.m_bValue ? m_pszTrue : m_pszFalse;

	case TypeInteger:
		cStream << un.m_iValue;		break;
	case TypeLongLong:
		cStream << un.m_llValue;	break;
	case TypeDouble:
		cStream << un.m_dValue;		break;
	}

	return cStream.getString();
}

//
//	FUNCTION public
//	Common::Parameter::Value::getInteger -- 32ビット整数を取り出す
//
//	NOTES
//	32ビット整数を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	32ビット整数の値。変換できない時は-1
//
//	EXCEPTIONS
//	なし
//

int
Common::Parameter::Value::getInteger() const
{
	switch (getType()) {
	case TypeString:
		return ModUnicodeCharTrait::toInt(*un.m_pstrValue);
	case TypeInteger:
		return un.m_iValue;
	case TypeLongLong:
		return static_cast<int>(un.m_llValue);
	case TypeBoolean:
		return un.m_bValue;
	case TypeDouble:
		return static_cast<int>(un.m_dValue);
	}

	return -1;
}

//
//	FUNCTION public
//	Common::Parameter::Value::getLongLong -- 64ビット整数を取り出す
//
//	NOTES
//	64ビット整数を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	64ビット整数の値。変換できない時は-1
//
//	EXCEPTIONS
//	なし
//

ModInt64
Common::Parameter::Value::getLongLong() const
{
	switch (getType()) {
	case TypeString:
		return ModUnicodeCharTrait::toLong(*un.m_pstrValue);
	case TypeInteger:
		return un.m_iValue;
	case TypeLongLong:
		return un.m_llValue;
	case TypeBoolean:
		return un.m_bValue;
	case TypeDouble:
		return static_cast<ModInt64>(un.m_dValue);	//これもだめ
	}

	return -1;
}

//
//	FUNCTION public
//	Common::Parameter::Value::getBoolean -- 真偽値を取り出す
//
//	NOTES
//	真偽値を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	真偽値。変換できない時はfalse
//
//	EXCEPTIONS
//	なし
//

bool
Common::Parameter::Value::getBoolean() const
{
	switch (getType()) {
	case TypeString:
		return !ModUnicodeCharTrait::compare(
			*un.m_pstrValue, _TRMEISTER_U_STRING(m_pszTrue), ModFalse);
	case TypeInteger:
		return un.m_iValue;
	case TypeLongLong:
		return un.m_llValue;
	case TypeBoolean:
		return un.m_bValue;
	case TypeDouble:
		return un.m_dValue;
	}

	return false;
}

//
//	FUNCTION public
//	Common::Parameter::Value::getDouble -- 倍精度浮動小数を取り出す
//
//	NOTES
//	倍精度浮動小数を取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	倍精度浮動小数の値。変換できないときは0.0
//
//	EXCEPTIONS
//	なし
//

double
Common::Parameter::Value::getDouble() const
{
	switch (getType()) {
	case TypeString:
		return ModUnicodeCharTrait::toFloat(*un.m_pstrValue);	//だめ
	case TypeInteger:
		return un.m_iValue;
	case TypeLongLong:
		return static_cast<double>(un.m_llValue);
	case TypeBoolean:
		return un.m_bValue ? 1.0 : 0.0;
	case TypeDouble:
		return un.m_dValue;
	}

	return 0;
}

//
//	FUNCTION public
//	Common::Parameter::Value::serialize -- シリアル化
//
//	NOTES
//	シリアル化
//
//	ARGUMENTS
//	ModArchiver& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Common::Parameter::Value::serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//書出し
		//タイプを書く
		int iType = m_eType;
		cArchiver_ << iType;
		//値を書く
		switch (m_eType)
		{
		case TypeString:
			{
				cArchiver_ << *un.m_pstrValue;
			}
			break;
		case TypeInteger:
			cArchiver_ << un.m_iValue;
			break;
		case TypeLongLong:
			cArchiver_ << un.m_llValue;
			break;
		case TypeBoolean:
			{
				int v = (un.m_bValue==true)?1:0;
				cArchiver_ << v;
			}
			break;
		case TypeDouble:
			cArchiver_ << un.m_dValue;
			break;
		case TypeNone:
		case TypeObjectPointer:
			break;
		}
	}
	else
	{
		//読込み
		//タイプを読む
		int iType;
		cArchiver_ >> iType;
		//値を読む
		switch (iType)
		{
		case TypeString:
			{
				// cArchiver_ << *un.m_pstrValue;
				ModUnicodeString cstrValue;
				cArchiver_ >> cstrValue;
				setString(cstrValue);
			}
			break;
		case TypeInteger:
			{
				int iValue;
				cArchiver_ >> iValue;
				setInteger(iValue);
			}
			break;
		case TypeLongLong:
			{
				ModInt64 llValue;
				cArchiver_ >> llValue;
				setLongLong(llValue);
			}
			break;
		case TypeBoolean:
			{
				int v;
				cArchiver_ >> v;
				setBoolean((v==1)?true:false);
			}
			break;
		case TypeDouble:
			{
				double dValue;
				cArchiver_ >> dValue;
				setDouble(dValue);
			}
			break;
		case TypeNone:
			break;
		case TypeObjectPointer:
			setObjectPointer(0);
			break;
		}
	}
}

//
//	Copyright (c) 1999, 2001, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
