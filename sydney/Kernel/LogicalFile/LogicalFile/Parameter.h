// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h -- 論理ファイルで使用するパラメーターの基底クラス
// 
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_PARAMETER_H
#define __SYDNEY_LOGICALFILE_PARAMETER_H

#include "SyDefault.h"

#include "LogicalFile/Module.h"
#include "LogicalFile/Externalizable.h"

#include "Common/SafeExecutableObject.h"
#include "Common/ObjectPointer.h"
#include "Os/CriticalSection.h"

#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"
#include "ModPair.h"
#include "ModVector.h"
#include "ModHashMap.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

class FileID; // メンバーとしてFileIDも持てるようにするため

namespace Parameter
{

class Base;

//	CLASS
//	LogicalFile::Parameter::Key -- FileIDおよびOpenOptionで使用されるキーの型
//
//	NOTES
//		FieldIndex[1]を(KeyNumber::FieldIndex, 1)のようにPairで表す

class SYD_LOGICALFILE_FUNCTION Key
	: public ModPair<int, int>//<Type, IndexType>
	, public ModSerializer
{
public:
	typedef int Type;					// キーに使う型
	typedef int IndexType;				// 配列要素の添え字に使う型

	Key() : ModPair<int, int>() {}
	Key(int key_) : ModPair<int, int>(key_, 0) {}
	Key(int key_, int index_) : ModPair<int, int>(key_, index_) {}

	Key& set(int key_, int index_ = 0) {first = key_; second = index_; return *this;}
	Type getKey() const {return first;}
	IndexType getIndex() const {return second;}

	void serialize(ModArchive& cArchiver_);
	void toString(ModUnicodeOstrStream& cStream_) const;

	ModSize hashValue() const;

	// KeyをHashMapに格納するときに用いるハッシュ関数
	class Hasher
	{
	public:
		ModSize operator()(const Key& key) const {return key.hashValue();}
	};
};

//	STRUCT
//	LogicalFile::Parameter::LayoutMapEntry -- パラメーターのレイアウトを表すマップのエントリー
//
//	NOTES

struct LayoutMapEntry {
	struct Type {
		enum Value {
			None,
			String,
			Integer,
			LongLong,
			Boolean,
			Double,
			ObjectPointer,
			FileID,
			ValueNum
		};
	};

	LayoutMapEntry()
		: m_eType(Type::None), m_bArray(false), m_iOffset(-1)
	{}
	LayoutMapEntry(Type::Value eType_, bool bArray_, int iOffset_)
		: m_eType(eType_), m_bArray(bArray_), m_iOffset(iOffset_)
	{}

	Type::Value m_eType;
	bool m_bArray;
	int m_iOffset;
};

//	TYPEDEF
//	LogicalFile::Parameter::LayoutMap -- パラメーターのレイアウトを表すマップ
//
//	NOTES

typedef ModVector<LayoutMapEntry> LayoutMap;

//	TEMPLATE CLASS
//	LogicalFile::Parameter::Element -- 単一のパラメーターを表す
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//
//	NOTES
//		第一要素でパラメーター値の有無を表し、第二要素でパラメーター値を表す

template <class __T__>
class Element
	: public ModPair<bool, __T__>
{
public:
	Element() : ModPair<bool, __T__>(false, __T__()) {}
	Element(__T__ v) : ModPair<bool, __T__>(true, v) {}
	Element(const Element& v) : ModPair<bool, __T__>(v) {}

	bool operator!() const {return !this->first;}
	__T__& getValue() {return this->second;}
	const __T__& getValue() const {return this->second;}

	// メンバーが配列の場合に保持する型
	typedef ModVector<Element<__T__> > Array;
};

// Elementをストリームに書き出す関数

template <class __T__>
ModOstream& operator<<(ModOstream& cStream_,
					   const Element<__T__>& cValue_);

//	TEMPLATE CLASS
//	LogicalFile::Parameter::ValueExtractor -- パラメーター値を操作する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LayoutMapEntry::Type::Value eType_
//		パラメーターの型を表すenum型(debug用)
//
//	NOTES
//		インスタンスを作らないのでCommon::Objectを継承しない

template <class __T__, LayoutMapEntry::Type::Value eType_>
class ValueExtractor
{
public:
	// 値を取得する
	static bool getValue(const Base& cParameter_, const Key& cKey_, __T__& cValue_);
	// 値を設定する
	static void setValue(Base& cParameter_, const Key& cKey_, const __T__& cValue_);
	// 値をコピーする
	static void copyValue(Base& cDst_, const Base& cSrc_, const LayoutMapEntry& cEntry_);
	// 値を破棄する
	static void deleteValue(Base& cParameter_, const LayoutMapEntry& cEntry_);
	// シリアル化
	static void serialize(Base& cParameter_, const LayoutMapEntry& cEntry_, Key::Type iKey_, ModArchive& cArchiver_);
	// 文字列化
	static void toString(const Base& cParameter_, const LayoutMapEntry& cEntry_, ModUnicodeOstrStream& cStream_);
//private:
public: // テンプレートクラスをfriend宣言できないのでpublicにする
	// 配列でない値をシリアライズする
	static void serializeScalar(__T__& cValue_, ModArchive& cArchiver_);
	// 配列要素をシリアライズする
	static void serializeVectorElement(Element<__T__>& cElement_, ModArchive& cArchiver_);
	// 配列でない値を文字列化する
	static void toStringScalar(const __T__& cValue_, ModUnicodeOstrStream& cStream_);
	// 配列要素を文字列化する
	static void toStringVectorElement(const Element<__T__>& cElement_, ModUnicodeOstrStream& cStream_);

	// 共有以外の値を取得する
	static bool getExtraValue(const Base& cParameter_, const Key& cKey_, __T__& cValue_);
	// 共有以外の値を設定する
	static void setExtraValue(Base& cParameter_, const Key& cKey_, const __T__& cValue_);
	// 古いバージョンのエントリーをシリアル化する
	static void serializeOldVersionEntry(Base& cParameter_, const LayoutMapEntry& cEntry_, Key::Type iKey_, ModArchive& cArchiver_);
};

//	CLASS
//	LogicalFile::Parameter::ExtraElement -- 共通以外のパラメーターを表す
//
//	NOTES

class ExtraElement
	: public Common::Object,
	  public ModSerializer
{
public:
	ExtraElement()
		: m_eType(LayoutMapEntry::Type::None)
	{}
	~ExtraElement()
	{
		clear();
	}

	ExtraElement(const ExtraElement& v);
	ExtraElement& operator =(const ExtraElement& v);

private:
	friend class Base;
	friend class ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>;
	friend class ValueExtractor<int, LayoutMapEntry::Type::Integer>;
	friend class ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>;
	friend class ValueExtractor<bool, LayoutMapEntry::Type::Boolean>;
	friend class ValueExtractor<double, LayoutMapEntry::Type::Double>;
	friend class ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>;
	friend class ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>;

	//pointer
	const void* getPointer() const;
	void setPointer(const void* pPointer_, LayoutMapEntry::Type::Value eType_);

	//String
	void setString(const ModUnicodeString& cstrValue_);

	//Integer
	void setInteger(int iValue_);

	//LongLong
	void setLongLong(ModInt64 llValue_);

	//Boolean
	void setBoolean(bool bValue_);

	//Double
	void setDouble(double dValue_);

	//Pointer to Object
	void setObjectPointer(const Common::Object* pObject_);

	//FileID
	void setFileID(const Common::ObjectPointer<FileID>& pFileID_);

	void serialize(ModArchive& cArchiver_);
	void toString(ModUnicodeOstrStream& cStream_) const;

	void clear();

	LayoutMapEntry::Type::Value m_eType;
	union {
		ModUnicodeString*		m_pstrValue;	//string
		int						m_iValue;		//int
		ModInt64				m_llValue;		//long long
		bool					m_bValue;		//boolean
		double					m_dValue;		//double
		const Common::Object*	m_pObject;		//pointer to object
		Common::ObjectPointer<FileID>*
								m_pFileID;		//pointer to FileID
	} un;
};

//	TYPEDEF
//	LogicalFile::Parameter::ExtraValues -- 共通以外のパラメーター値を保持する型
//
//	NOTES

typedef ModHashMap<Key, ExtraElement, Key::Hasher> ExtraValues;

//	CLASS
//	LogicalFile::Parameter::Base -- パラメータークラスの基底クラス
//
//	NOTES

class SYD_LOGICALFILE_FUNCTION Base
	: public Common::SafeExecutableObject,
	  public Externalizable
{
public:
	//String
	ModUnicodeString getString(const Key& cKey_) const;
	bool getString(const Key& cKey_,
				   ModUnicodeString& cstrValue_) const;
	void setString(const Key& cKey_,
				   const ModUnicodeString& cstrValue_);

	//Integer
	int getInteger(const Key& cKey_) const;
	bool getInteger(const Key& cKey_, int& iValue_) const;
	void setInteger(const Key& cKey_, int iValue_);

	//LongLong
	ModInt64 getLongLong(const Key& cKey_) const;
	bool getLongLong(const Key& cKey_, ModInt64& llValue_) const;
	void setLongLong(const Key& cKey_, ModInt64 llValue_);

	//Boolean
	bool getBoolean(const Key& cKey_) const;
	bool getBoolean(const Key& cKey_, bool& bValue_) const;
	void setBoolean(const Key& cKey_, bool bValue_);

	//Double
	double getDouble(const Key& cKey_) const;
	bool getDouble(const Key& cKey_, double& dValue_) const;
	void setDouble(const Key& cKey_, double dValue_);

	//Pointer to Object
	Common::Object* getObjectPointer(const Key& cKey_) const;
	bool getObjectPointer(const Key& cKey_,
						  const Common::Object*& pObject_) const;
	void setObjectPointer(const Key& cKey_,
						  const Common::Object* pObject_);

	//FileID
	Common::ObjectPointer<FileID> getFileID(const Key& cKey_) const;
	bool getFileID(const Key& cKey_, Common::ObjectPointer<FileID>& pFileID_) const;
	void setFileID(const Key& cKey_, const Common::ObjectPointer<FileID>& pFileID_);

	//すべてのエントリをクリアする
	void clear();

	//シリアル化(ただしオブジェクトポインタはシリアル化できない)
	void serialize(ModArchive& cArchiver_);

	// 代入オペレーター
	Base& operator =(const Base& cOther_);

	//文字列で取出す
	ModUnicodeString toString() const;

	// バージョン番号を得る
	int getVersion() const;

	// デストラクター
	virtual ~Base();
	//クラスIDを得る
	virtual int getClassID() const = 0;

protected:
	Base();
	Base(const Base& cOther_);

	// バージョン番号を渡してレイアウトマップを初期化する
	void setLayoutMap(int iVersion_);

	// バージョン番号に対応したレイアウトマップを取得する
	virtual const LayoutMap* getLayoutMap(int iVersion_) const = 0;

	// 初期化配列からレイアウトマップを作る
	static void assignMap(LayoutMap& cMap_, int iMaxKey_,
						  const ModPair<int, LayoutMapEntry>* pSpec_,
						  const ModPair<int, LayoutMapEntry>* pLast_);

private:
	friend class ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>;
	friend class ValueExtractor<int, LayoutMapEntry::Type::Integer>;
	friend class ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>;
	friend class ValueExtractor<bool, LayoutMapEntry::Type::Boolean>;
	friend class ValueExtractor<double, LayoutMapEntry::Type::Double>;
	friend class ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>;
	friend class ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>;

	int m_iVersion;						// レイアウトマップのバージョン番号
	const LayoutMap* m_pMap;
	ExtraValues	m_cExtraValues;

	mutable Os::CriticalSection m_cLatch;
										// メンバーを保護する
										// FileIDをメンバーに入れられるようになったので
										// 複数のスレッドから同じインスタンスに
										// アクセスする可能性がある
};

} // namespace Parameter

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_PARAMETER_H

//
//	Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
