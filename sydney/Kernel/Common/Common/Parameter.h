// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h -- パラメータクラス
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

#ifndef __TRMEISTER_COMMON_PARAMETER_H
#define __TRMEISTER_COMMON_PARAMETER_H

#include "Common/ExecutableObject.h"
#include "Common/Externalizable.h"

#include "ModHashMap.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"

_TRMEISTER_BEGIN

namespace Common
{

class Object;

//
//	CLASS
//	Common::Parameter -- パラメータクラス
//
//	NOTES
//	パラメータを保持するクラス。
//	Execution::Program中で使用されるので、Common::ExecutableObjectの派生クラス
//	にする。
//	このパラメータクラスに格納されたクラスへのポインタのインスタンスは
//	呼び出し側が解放しなければならない。パラメータクラスのデストラクタでは
//	解放されない。
//
class SYD_COMMON_FUNCTION Parameter : public ExecutableObject,
							   public Externalizable
{
public:
	//パラメータの型
	enum Type
	{
		TypeNone = 0,			//未定義値
		TypeString,				//文字列
		TypeInteger,			//32ビット整数
		TypeLongLong,			//64ビット整数
		TypeBoolean,			//真偽値
		TypeDouble,				//double
		TypeObjectPointer		//クラスへのポインタ
	};

	//コンストラクタ
	Parameter();
	//デストラクタ
	~Parameter();
#ifdef OBSOLETE
	//システムパラメータ
	static bool getSystemValue(const ModCharString& key,
							   ModUnicodeString& value);
	static bool getSystemValue(const ModCharString& key, int& value);
	static bool getSystemValue(const ModCharString& key, bool& value);
	static ModUnicodeString getSystemString(const ModCharString& cstrKey_);
	static int getSystemInteger(const ModCharString& cstrKey_);
	static bool getSystemBoolean(const ModCharString& cstrKey_);
#endif
	//String
	ModUnicodeString getString(const ModCharString& cstrKey_) const;
	bool getString(const ModCharString& cstrKey_,
				   ModUnicodeString& cstrValue_) const;
	void setString(const ModCharString& cstrKey_,
				   const ModUnicodeString& cstrValue_);

	//Integer
	int getInteger(const ModCharString& cstrKey_) const;
	bool getInteger(const ModCharString& cstrKey_, int& iValue_) const;
	void setInteger(const ModCharString& cstrKey_, int iValue_);

	//LongLong
	ModInt64 getLongLong(const ModCharString& cstrKey_) const;
	bool getLongLong(const ModCharString& cstrKey_, ModInt64& llValue_) const;
	void setLongLong(const ModCharString& cstrKey_, ModInt64 llValue_);

	//Boolean
	bool getBoolean(const ModCharString& cstrKey_) const;
	bool getBoolean(const ModCharString& cstrKey_, bool& bValue_) const;
	void setBoolean(const ModCharString& cstrKey_, bool bValue_);

	//Double
	double getDouble(const ModCharString& cstrKey_) const;
	bool getDouble(const ModCharString& cstrKey_, double& dValue_) const;
	void setDouble(const ModCharString& cstrKey_, double dValue_);

	//Pointer to Object
	Common::Object* getObjectPointer(const ModCharString& cstrKey_) const;
	bool getObjectPointer(const ModCharString& cstrKey_,
						  const Common::Object*& pObject_) const;
	void setObjectPointer(const ModCharString& cstrKey_,
						  const Common::Object* pObject_);

	//すべてのエントリをクリアする
	void clear();

	//シリアル化(ただしオブジェクトポインタはシリアル化できない)
	void serialize(ModArchive& cArchiver_);
	//クラスIDを得る
	int getClassID() const;

	//文字列で取出す
	ModUnicodeString toString() const;

private:
	//パラメータ値を保持するクラス
	class SYD_COMMON_FUNCTION Value : public ModSerializer
	{
	public:
		//コンストラクタ
		Value();
		//デストラクタ
		~Value();
		
		//コピーコンストラクタ
		Value(const Value& cValue);
		//代入オペレータ
		Value& operator =(const Value& cValue);

		//パラメータの型を得る
		Type getType() const;

		//String
		ModUnicodeString getString() const;
		void setString(const ModUnicodeString& cstrValue_);

		//Integer
		int getInteger() const;
		void setInteger(int iValue_);

		//LongLong
		ModInt64 getLongLong() const;
		void setLongLong(ModInt64 llValue_);

		//Boolean
		bool getBoolean() const;
		void setBoolean(bool bValue_);

		//Double
		double getDouble() const;
		void setDouble(double dValue_);

		//Pointer to Object
		Common::Object* getObjectPointer() const;
		void setObjectPointer(const Common::Object* pObject_);

		//シリアル化
		void serialize(ModArchive& cArchiver_);

	private:
		//パラメータの型
		Type m_eType;
		//パラメータ値
		union
		{
			ModUnicodeString*		m_pstrValue;	//string
			int						m_iValue;		//int
			ModInt64				m_llValue;		//long long
			bool					m_bValue;		//boolean
			double					m_dValue;		//double
			const Common::Object*	m_pObject;		//pointer to object
		} un;

		//真偽値をあらわす文字列
		static const char * const m_pszTrue;
		static const char * const m_pszFalse;
	};

	//マップの型
	typedef ModHashMap<ModCharString, Value, ModHasher<ModCharString> > Map;
	
	//パラメータを格納するマップ
	Map m_mapParameter;

};

}

//
//	FUNCTION public
//	Common::Parameter::~Parameter -- デストラクタ
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
Common::Parameter::~Parameter()
{}

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
//
//	RETURN
//	文字列の値。存在しない場合は、空文字列
//
//	EXCEPTIONS
//	なし
//

inline
ModUnicodeString
Common::Parameter::getString(const ModCharString& cstrKey_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	return (i != m_mapParameter.end()) ?
		(*i).second.getString() : ModUnicodeString();
}

//
//	FUNCTION public
//	Common::Parameter::setString -- 文字列を設定する
//
//	NOTES
//	文字列を設定する。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	const ModUnicodeString& cstrValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::setString(
	const ModCharString& cstrKey_, const ModUnicodeString& cstrValue_)
{
	m_mapParameter[cstrKey_].setString(cstrValue_);
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
//
//	RETURN
//	32ビット整数の値。存在しない場合は-1
//
//	EXCEPTIONS
//	なし
//

inline
int
Common::Parameter::getInteger(const ModCharString& cstrKey_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	return (i != m_mapParameter.end()) ? (*i).second.getInteger() : -1;
}

//
//	FUNCTION public
//	Common::Parameter::setInteger -- 32ビット整数を設定する
//
//	NOTES
//	32ビット整数を設定する。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	int iValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::setInteger(const ModCharString& cstrKey_, int iValue_)
{
	m_mapParameter[cstrKey_].setInteger(iValue_);
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
//
//	RETURN
//	64ビット整数の値。存在しない場合は-1
//
//	EXCEPTIONS
//	なし
//

inline
ModInt64
Common::Parameter::getLongLong(const ModCharString& cstrKey_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	return (i != m_mapParameter.end()) ? (*i).second.getLongLong() : -1;
}

//
//	FUNCTION public
//	Common::Parameter::setLongLong -- 64ビット整数を設定する
//
//	NOTES
//	64ビット整数を設定する。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	ModInt64 llValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::setLongLong(
	const ModCharString& cstrKey_, ModInt64 llValue_)
{
	m_mapParameter[cstrKey_].setLongLong(llValue_);
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
//
//	RETURN
//	真偽値。存在しない場合はfalse
//
//	EXCEPTIONS
//	なし
//

inline
bool
Common::Parameter::getBoolean(const ModCharString& cstrKey_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	return (i != m_mapParameter.end()) ? (*i).second.getBoolean() : false;
}

//
//	FUNCTION public
//	Common::Parameter::setBoolean -- 真偽値を設定する
//
//	NOTES
//	真偽値を設定する。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	bool bValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::setBoolean(const ModCharString& cstrKey_, bool bValue_)
{
	m_mapParameter[cstrKey_].setBoolean(bValue_);
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
//
//	RETURN
//	倍精度浮動小数の値。存在しない場合は0.0
//
//	EXCEPTIONS
//	なし
//

inline
double
Common::Parameter::getDouble(const ModCharString& cstrKey_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	return (i != m_mapParameter.end()) ? (*i).second.getDouble() : 0;
}

//
//	FUNCTION public
//	Common::Parameter::setDouble -- 倍精度浮動小数を設定する
//
//	NOTES
//	倍精度浮動小数を設定する。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	double dValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::setDouble(const ModCharString& cstrKey_, double dValue_)
{
	m_mapParameter[cstrKey_].setDouble(dValue_);
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
//
//	RETURN
//	オブジェクトへのポインタ。存在しない場合は0
//
//	EXCEPTIONS
//	なし
//

inline
Common::Object*
Common::Parameter::getObjectPointer(const ModCharString& cstrKey_) const
{
	// マップを検索
	const Map::ConstIterator& i = m_mapParameter.find(cstrKey_);

	return (i != m_mapParameter.end()) ? (*i).second.getObjectPointer() : 0;
}

//
//	FUNCTION public
//	Common::Parameter::setObjectPointer -- オブジェクトへのポインタを設定する
//
//	NOTES
//	オブジェクトへのポインタを設定する。
//
//	ARGUMENTS
//	const ModCharString& cstrKey_
//		キー
//	const Common::Object* pValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::setObjectPointer(
	const ModCharString& cstrKey_, const Common::Object* pValue_)
{
	m_mapParameter[cstrKey_].setObjectPointer(pValue_);
}

//
//	FUNCTION public
//	Common::Parameter::clear -- すべてのエントリを削除する
//
//	NOTES
//	すべてのエントリを削除する。
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
Common::Parameter::clear()
{
	m_mapParameter.clear();
}

//
//	FUNCTION public
//	Common::Parameter::Value::Value -- コンストラクタ
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
Common::Parameter::Value::Value()
	: m_eType(TypeNone)
{}

//
//	FUNCTION public
//	Common::Parameter::Value::~Value -- デストラクタ
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
Common::Parameter::Value::~Value()
{
	//文字列の時のみメモリを解放する
	if (getType() == TypeString)
		delete un.m_pstrValue;
}

//
//	FUNCTION public
//	Common::Parameter::Value::getType -- パラメータタイプを得る
//
//	NOTES
//	パラメータタイプを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::Parameter::Type
//		パラメータタイプ
//
//	EXCEPTIONS
//	なし
//

inline
Common::Parameter::Type
Common::Parameter::Value::getType() const
{
	return m_eType;
}

//
//	FUNCTION public
//	Common::Parameter::Value::setString -- 文字列を設定する
//
//	NOTES
//	文字列を設定する。
//
//	ARGUMENTS
//	const ModUnicodeString& cstrValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::Value::setString(const ModUnicodeString& cstrValue_)
{
	if (getType() == TypeString)
		*un.m_pstrValue = cstrValue_;
	else {
		un.m_pstrValue = new ModUnicodeString(cstrValue_);
		m_eType = TypeString;
	}
}

//
//	FUNCTION public
//	Common::Parameter::Value::setInteger -- 32ビット整数を設定する
//
//	NOTES
//	32ビット整数を設定する。
//
//	ARGUMENTS
//	int iValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::Value::setInteger(int iValue_)
{
	if (getType() == TypeString)
		delete un.m_pstrValue;

	m_eType = TypeInteger;
	un.m_iValue = iValue_;
}

//
//	FUNCTION public
//	Common::Parameter::Value::setLongLong -- 64ビット整数を設定する
//
//	NOTES
//	64ビット整数を設定する。
//
//	ARGUMENTS
//	ModInt64 llValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::Value::setLongLong(ModInt64 llValue_)
{
	if (getType() == TypeString)
		delete un.m_pstrValue;

	m_eType = TypeLongLong;
	un.m_llValue = llValue_;
}

//
//	FUNCTION public
//	Common::Parameter::Value::setBoolean -- 真偽値を設定する
//
//	NOTES
//	真偽値を設定する。
//
//	ARGUMENTS
//	bool bValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::Value::setBoolean(bool bValue_)
{
	if (getType() == TypeString)
		delete un.m_pstrValue;

	m_eType = TypeBoolean;
	un.m_bValue = bValue_;
}

//
//	FUNCTION public
//	Common::Parameter::Value::setDouble -- 倍精度浮動小数を設定する
//
//	NOTES
//	倍精度浮動小数を設定する。
//
//	ARGUMENTS
//	double dValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::Value::setDouble(double dValue_)
{
	if (getType() == TypeString)
		delete un.m_pstrValue;

	m_eType = TypeDouble;
	un.m_dValue = dValue_;
}

//
//	FUNCTION public
//	Common::Parameter::Value::getObjectPointer
//									-- オブジェクトへのポインタを取り出す
//
//	NOTES
//	オブジェクトへのポインタを取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	オブジェクトへのポインタ。変換できないときは0
//
//	EXCEPTIONS
//	なし
//

inline
Common::Object*
Common::Parameter::Value::getObjectPointer() const
{
	return (getType() == TypeObjectPointer) ?
		const_cast<Common::Object*>(un.m_pObject) : 0;
}

//
//	FUNCTION public
//	Common::Parameter::Value::setObjectPointer
//										-- オブジェクトへのポインタを設定する
//
//	NOTES
//	オブジェクトへのポインタを設定する。
//
//	ARGUMENTS
//	const Common::Object* pValue_
//		値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//

inline
void
Common::Parameter::Value::setObjectPointer(const Common::Object* pValue_)
{
	if (getType() == TypeString)
		delete un.m_pstrValue;

	m_eType = TypeObjectPointer;
	un.m_pObject = pValue_;
}

_TRMEISTER_END

#endif // __TRMEISTER_COMMON_PARAMETER_H

//
//	Copyright (c) 1999, 2001, 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
