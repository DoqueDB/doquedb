// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.h -- B木で扱うデータ
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_DATA_H
#define __SYDNEY_ARRAY_DATA_H

#include "Array/Module.h"

#include "Common/Data.h"						//Common::Data::Pointer
#include "LogicalFile/TreeNodeInterface.h"		//TreeNodeInterface::Type

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Common
{
	class BitSet;
	class DataArrayData;
	class DecimalData;
	class Object;
}
namespace LogicalFile
{
	class FileID;
}

_SYDNEY_ARRAY_BEGIN

class Tree;

//
//	CLASS
//	Array::Data -- Arrayで扱うデータ型
//
//	NOTES
//	ここに現れるクラス内クラスはすべてページのバッファから直接キャストして
//	利用される。そのため、余計なメンバー変数や、virtualな関数等を定義しては
//	ならないし、4バイト境界を意識しなくてはならない。
//
//	データの比較、データの取得、データの書き込み等を行う。
//
//	エントリ全体の比較は Compare が受け持ち、
//	このクラスは、個々のフィールドのデータの比較を受け持つ。
//
//	
//
class Data : public Common::Object
{
 public:
	//
	//	STRUCT
	//	Array::Data::Type -- データ型
	//
	//	NOTES
	//	ここにあるデータ型以外の型はB木に挿入することはできない
	//
	struct Type
	{
		enum Value
		{
			Integer,
			UnsignedInteger,
			Float,					// not supported
			Double,
			CharString,
			UnicodeString,
			NoPadCharString,
			NoPadUnicodeString,
			Date,					// not supported
			DateTime,
			IntegerArray,			// not supported
			UnsignedIntegerArray,	// not supported
			ObjectID,				// not supported
			LanguageSet,
			Integer64,
			Decimal,
			Undefined
		};
	};
	
	//
	//	CLASS
	//	Array::Data::Integer -- 32ビットInteger型
	//
	class Integer
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer)
		{
			return _size();
		}
		ModSize getSize() const
		{
			return _size();
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Integer& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		// 丸める
		static bool round(const Common::Data& cData_,
						  LogicalFile::TreeNodeInterface::Type& eMatch_,
						  int& value_);

		// 値
		ModInt32 m_value;

	private:
		static ModSize _size() { return sizeof(ModInt32)/sizeof(ModUInt32); }
	};

	//
	//	CLASS
	//	Array::Data::UnsignedInteger -- 32ビットUnsignedInteger型
	//
	class UnsignedInteger
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer)
		{
			return _size();
		}
		ModSize getSize() const
		{
			return _size();
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const UnsignedInteger& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 値
		ModUInt32 m_value;
		
	private:
		static ModSize _size() { return sizeof(ModUInt32)/sizeof(ModUInt32); }
	};

	//
	//	CLASS
	//	Array::Data::Double -- 64ビットDouble型
	//
	class Double
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer)
		{
			return _size();
		}
		ModSize getSize() const
		{
			return _size();
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Double& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 値
		double m_value;
		
	private:
		static ModSize _size() { return sizeof(double)/sizeof(ModUInt32); }
	};

	//
	//	CLASS
	//	Array::Data::Decimal -- 
	//
	class Decimal
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const Decimal* s
				= syd_reinterpret_cast<const Decimal*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Decimal& cOther_) const;

		// 配列要素長をユニット長にする
		static ModSize calcUnitSize(ModUInt32 uiLength_)
		{
			// 要素数分を足す
			return (static_cast<ModSize>(uiLength_)) / 4 + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		static bool round(const Common::Data& cData_,
					LogicalFile::TreeNodeInterface::Type& eMatch_,
					ModSize iPrecision_, ModSize iScale_, 
					ModUnicodeString& cstrValue_);

		// Get the parameters.
		static void getParameter(const LogicalFile::FileID& cFileID_,
								 int iPosition_,
								 bool isArray_,
								 int& iPrecision_,
								 int& iScale_);

		// 長さ
		ModSize m_length;
		// 値
		ModUInt32 m_value[1];
	};

	//
	//	CLASS
	//	Array::Data::CharString -- Char文字列型
	//
	class CharString
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const CharString* s = syd_reinterpret_cast<const CharString*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const CharString& cOther_) const;

		// 文字列長をユニット長にする
		static ModSize calcUnitSize(unsigned short usLength_)
		{
			// 文字列長分を足して、4バイトバウンダリにする
			return (static_cast<ModSize>(usLength_) + 1) / 4 + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 長さ
		unsigned short m_length;
		// 値
		unsigned char m_value[2];
	};

	//
	//	CLASS
	//	Array::Data::UnicodeString -- Unicode文字列型
	//
	class UnicodeString
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const UnicodeString* s
				= syd_reinterpret_cast<const UnicodeString*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const UnicodeString& cOther_) const;

		// 文字列長をユニット長にする
		static ModSize calcUnitSize(unsigned short usLength_)
		{
			// 文字列長分を足して、4バイトバウンダリにする
			return static_cast<ModSize>(usLength_) / 2 + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 長さ
		unsigned short m_length;
		// 値
		ModUnicodeChar m_value[1];
	};

	//
	//	CLASS
	//	Array::Data::NoPadCharString -- Char文字列型
	//
	class NoPadCharString
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const NoPadCharString* s =
				syd_reinterpret_cast<const NoPadCharString*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}

		// 比較関数
		int compare(const NoPadCharString& cOther_) const;
		bool like(const NoPadCharString& cOther_, ModUnicodeChar escape_) const;

		// 文字列長をユニット長にする
		static ModSize calcUnitSize(unsigned short usLength_)
		{
			// 文字列長分を足して、4バイトバウンダリにする
			return (static_cast<ModSize>(usLength_) + 1) / 4 + 1;
		}
		
		// 長さ
		unsigned short m_length;
		// 値
		unsigned char m_value[2];
	};

	//
	//	CLASS
	//	Array::Data::NoPadUnicodeString -- Unicode文字列型
	//
	class NoPadUnicodeString
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const NoPadUnicodeString* s
				= syd_reinterpret_cast<const NoPadUnicodeString*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}

		// 比較関数
		int compare(const NoPadUnicodeString& cOther_) const;
		bool like(const NoPadUnicodeString& cOther_,
				  ModUnicodeChar escape_) const;
		
		// 文字列長をユニット長にする
		static ModSize calcUnitSize(unsigned short usLength_)
		{
			// 文字列長分を足して、4バイトバウンダリにする
			return static_cast<ModSize>(usLength_) / 2 + 1;
		}

		// 長さ
		unsigned short m_length;
		// 値
		ModUnicodeChar m_value[1];
	};

	//
	//	CLASS
	//	Array::Data::DateTime -- 日時型
	//
	class DateTime
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer)
		{
			return _size();
		}
		ModSize getSize() const
		{
			return _size();
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const DateTime& cOther_) const
		{
			return (m_date < cOther_.m_date) ? -1
				: (m_date > cOther_.m_date) ? 1
				: (m_time < cOther_.m_time) ? -1
				: (m_time > cOther_.m_time) ? 1 : 0;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 値
		ModInt32 m_date;
		ModUInt32 m_time;
		
	private:
		static ModSize _size()
		{
			return (sizeof(ModInt32)+sizeof(ModUInt32))/sizeof(ModUInt32);
		}
	};

	//
	//	CLASS
	//	Array::Data::ObjectID -- ObjectID型
	//
	class ObjectID
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer)
		{
			return _size();
		}
		ModSize getSize() const
		{
			return _size();
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const ObjectID& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		// 値
		ModUInt64 m_value;

	private:
		static ModSize _size() { return sizeof(ModUInt64)/sizeof(ModUInt32); }
	};

	//
	//	CLASS
	//	Array::Data::LanguageSet -- LanguageSet型
	//
	class LanguageSet
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const LanguageSet* s
				= syd_reinterpret_cast<const LanguageSet*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const LanguageSet& cOther_) const;

		// 配列要素長をユニット長にする
		static ModSize calcUnitSize(ModUInt32 uiLength_)
		{
			// 要素数分を足す
			return uiLength_ + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 長さ
		ModUInt32 m_length;
		// 値
		ModUInt32 m_value[1];
	};

	//
	//	CLASS
	//	Array::Data::Integer64 -- 64ビットInteger型
	//
	class Integer64
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer)
		{
			return _size();
		}
		ModSize getSize() const
		{
			return _size();
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Integer64& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		// 丸める
		static bool round(const Common::Data& cData_,
						  LogicalFile::TreeNodeInterface::Type& eMatch_,
						  ModInt64& value_);

		// 値
		ModInt64 m_value;

	private:
		static ModSize _size() { return sizeof(ModInt64)/sizeof(ModUInt32); }
	};

	// コンストラクタ
	Data();
	// デストラクタ
	virtual ~Data();

	// Set the count of data's type.
	void setTypeCount(int iFieldCount_);
	// Get the count of data's type.
	ModSize getTypeCount() const { return m_iFieldCount; }

	// Set the type of data.
	void setType(Data::Type::Value eFieldType_, int iFieldPosition_);
	// Get the type of data.
	Data::Type::Value getType(int iFieldPosition_) const;

	// Set the size of data. All the fields of the data are fixed.
	void setSize(ModSize uiFixedSize_, ModSize uiVariableSize_);
	// Get the size of fixed fields. [unit size]
	ModSize getFixedSize() const { return m_uiFixedSize; }
	// Get the size of variable field. [unit size]
	ModSize getVariableSize() const { return m_uiVariableSize; }
	// Is the size of data fixed?
	bool isFixed() const { return (m_uiVariableSize == 0); }

	// サイズを得る
	ModSize getSize(const ModUInt32* p) const;
	// １つのサイズを得る
//	ModSize getSize(const ModUInt32* p, int n_) const;

	// サイズを得る
	ModSize getSize(const Common::DataArrayData& cData_) const;
	// １つのサイズを得る
	ModSize getSize(const Common::Data& cData_, int n_) const;

	// ダンプする
	ModSize dump(ModUInt32* p, const Common::DataArrayData& cData_) const;
	// １つダンプする
	ModSize dump(ModUInt32* p, const Common::Data& cData_, int n_) const;

	// Get data.
	void getData(const ModUInt32* p,
				 int iFieldPosition_,
				 Common::Data::Pointer pData_) const;
	// Get ModUInt32 data.
	ModUInt32 getModUInt32Data(const ModUInt32* p,
							   int iFieldPosition_) const;
	// Get ModUInt32 data.
	ModUInt32 getModUInt32Data(const ModUInt32* p,
							   int iFieldPosition_,
							   Common::DataArrayData& cTuple_) const;
	// Get ModUInt32 data by BitSet.
	ModUInt32 getModUInt32DataByBitSet(const ModUInt32* p,
									   int iFieldPosition_,
									   Common::BitSet& cBitSet_) const;

	// フィールド型を得る
	static Data::Type::Value getFieldType(const LogicalFile::FileID& cFileID_,
										  int iPosition_,
										  bool& isFixed_,
										  bool& isArray_);

	// フィールドサイズを得る
//	static ModSize getFieldSize(const LogicalFile::FileID& cFileID_,
//								const Type::Value* cFieldTypes_,
//								int iFiledCount_);
	// フィールドサイズを一つ得る
	static ModSize getFieldSize(const LogicalFile::FileID& cFileID_,
								int iPosition_,
								Type::Value eType_,
								bool isArray_);

	// 1つのサイズを得る
	static ModSize getSize(const ModUInt32*& p, Type::Value eType_);
	// 1つのサイズを得る
	static ModSize getSize(const Common::Data& cData_, Type::Value eType_);

	// タイプからCommon::Dataを得る
	static Common::Data::Pointer makeData(const LogicalFile::FileID& cFileID_,
										  int iPosition_,
										  Type::Value eType_,
										  bool isArray_);

	// 1つダンプする
	static void dumpOneData(ModUInt32* p,
							const Common::Data& cData_,
							Type::Value eType_)
		{ dump(p, cData_, eType_); }

private:
	//
	//	CONST
	//	MaxFieldCount -- The Maximum number of fields
	//
	//	NOTES
	//	Value, RowID, Index, PageID
	//	[YET!] It may be better to define this number in Tree or FileID.
	//
	static const int MaxFieldCount = 4;

	// 1つダンプする
	static void dump(ModUInt32*& p,
					 const Common::Data& cData_,
					 Type::Value eType_);
	
	void getData(const ModUInt32*& p,
				 Type::Value eType_,
				 Common::Data& cData_) const;
	
	// The array of the types
	Type::Value m_cFieldTypes[MaxFieldCount];
	// The number of the types
	int m_iFieldCount;

	// The total of the field size except variable field [unit size]
	// The unit size is byte / sizeof(ModUInt32).
	// The reason is keeping byte boundary.
	ModSize m_uiFixedSize;

	// The size of variable filed. [unit size]
	// When the field does not exist, the size is set to 0.
	ModSize m_uiVariableSize;
};

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_DATA_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
