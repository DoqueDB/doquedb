// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.h -- B木で扱うデータ
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_DATA_H
#define __SYDNEY_BTREE2_DATA_H

#include "Btree2/Module.h"

#include "PhysicalFile/Page.h"

#include "Common/Data.h"
#include "Common/Object.h"
#include "Common/DataArrayData.h"
#include "Common/DecimalData.h"
#include "Common/BitSet.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Os/Memory.h"

#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE2_BEGIN

//
//	CLASS
//	Btree2::Data -- B木で扱うデータ型
//
//	NOTES
//	ここに現れるクラス内クラスはすべてページのバッファから直接キャストして
//	利用される。そのため、余計なメンバー変数や、virtualな関数等を定義しては
//	ならないし、4バイト境界を意識しなくてはならない。
//
class Data : public Common::Object
{
 public:
	//
	//	STRUCT
	//	Btree2::Data::Type -- データ型
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
			Header,
			Undefined
		};
	};
	
	//
	//	CLASS
	//	Btree2::Data::Integer -- 32ビットInteger型
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
	//	Btree2::Data::UnsignedInteger -- 32ビットUnsignedInteger型
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

#ifdef OBSOLETE
	//
	//	CLASS
	//	Btree2::Data::Float -- 32ビットFloat型
	//
	class Float
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
		int compare(const Float& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 値
		float m_value;
		
	private:
		static ModSize _size() { return sizeof(float)/sizeof(ModUInt32); }
	};
#endif

	//
	//	CLASS
	//	Btree2::Data::Double -- 64ビットDouble型
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
	//	Btree2::Data::Decimal -- 
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
			//【注意】	Common::DecimalData の getDumpedSize は4バイト単位
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

		// 長さ
		ModSize m_length;
		// 値
		ModUInt32 m_value[1];
	};

	//
	//	CLASS
	//	Btree2::Data::CharString -- Char文字列型
	//
	class CharString
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const CharString* s
				= syd_reinterpret_cast<const CharString*>(pBuffer_);
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
	//	Btree2::Data::UnicodeString -- Unicode文字列型
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
	//	Btree2::Data::NoPadCharString -- Char文字列型
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
	//	Btree2::Data::NoPadUnicodeString -- Unicode文字列型
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

#ifdef OBSOLETE
	//
	//	CLASS
	//	Btree2::Data::Date -- 日付型
	//
	class Date
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
		int compare(const Date& cOther_) const
		{
			return (m_value < cOther_.m_value) ? -1
				: (m_value == cOther_.m_value) ? 0 : 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 値
		ModInt32 m_value;
		
	private:
		static ModSize _size() { return sizeof(ModInt32)/sizeof(ModUInt32); }
	};
#endif

	//
	//	CLASS
	//	Btree2::Data::DateTime -- 日時型
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
	//	Btree2::Data::ObjectID -- ObjectID型
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
	//	Btree2::Data::LanguageSet -- LanguageSet型
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
	//	Btree2::Data::Integer64 -- 64ビットInteger型
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

	//
	//	CLASS
	//	Btree2::Data::Header -- 32ビットのHeader型
	//
	class Header
	{
	public:
		// サイズを返す
		static ModSize getSize()
		{
			return _size();
		}

		// クリアする
		static void clear(ModUInt32*& p)
		{
			Os::Memory::reset(p, _size() * sizeof(ModUInt32));
			p += _size();
		}

		// 削除フラグ
		bool isExpunge() const
			{
				unsigned char f = 1;
				return m_value[0] & f;
			}
		void setExpungeFlag(bool flag)
			{
				unsigned char f = 1;
				if (flag)
					m_value[0] |= f;
				else
					m_value[0] &= ~f;
			}

		// null
		bool isNull(int n) const
			{
				unsigned char f = (1 << n);
				return m_value[1] & f;
			}
		void setNullFlag(int n, bool flag)
			{
				unsigned char f = (1 << n);
				if (flag)
					m_value[1] |= f;
				else
					m_value[1] &= ~f;
			}
		unsigned char& getNullBitmap()
			{
				return m_value[1];
			}
		const unsigned char& getNullBitmap() const
			{
				return m_value[1];
			}

		// 値
		unsigned char m_value[4];
		
	private:
		static ModSize _size() { return sizeof(ModUInt32)/sizeof(ModUInt32); }
	};

	// コンストラクタ
	Data();
	// デストラクタ
	virtual ~Data();

	// 型配列を設定する
	void setType(const ModVector<Type::Value>& vecType_,
				 ModSize size_, bool bHeader_);
	// 型配列数を得る
	ModSize getTypeCount() const { return m_vecType.getSize(); }
	// 固定長のサイズを得る
	ModSize getFixedSize() const { return m_Size; }

	// サイズを得る -- nullがないもの
	ModSize getSize(const ModUInt32* p) const;
	// サイズを得る -- nullがあるもの
	ModSize getSize(const ModUInt32* p, unsigned char nullBitmap_) const;

	// サイズを得る
	ModSize getSize(const Common::DataArrayData& cData_) const;
	// １つのサイズを得る
	ModSize getSize(const Common::Data& cData_, int n_) const;
	// 1つのサイズを得る
	static ModSize getSize(const Common::Data& cData_, Type::Value eType_);
	// 1つのサイズを得る
	static ModSize getSize(const ModUInt32*& p, Type::Value eType_);

	// ダンプする -- nullがないもの
	ModSize dump(ModUInt32* p, const Common::DataArrayData& cData_) const;
	// ダンプする -- nullがあるもの
	ModSize dump(ModUInt32* p, const Common::DataArrayData& cData_,
				 unsigned char& nullBitmap_) const;
	// 1つダンプする
	static void dump(ModUInt32*& p, const Common::Data& cData_,
					 Type::Value eType_);

	// ページIDを得る -- nullがないもの
	PhysicalFile::PageID getPageID(const ModUInt32* p) const;
	// ページIDを得る -- nullがあるもの
	PhysicalFile::PageID getPageID(const ModUInt32* p,
								   unsigned char nullBitmap_) const;

	// ROWIDを得る
	ModUInt32 getRowID(const ModUInt32* p) const;

	// データを得る -- nullがないもの
	void getData(const ModUInt32* p,
				 unsigned char ucBitSet_,
				 Common::DataArrayData& cTuple_,
				 unsigned int& uiTupleID_) const;
	// データを得る -- nullがあるもの
	void getData(const ModUInt32* p,
				 unsigned char nullBitmap_,
				 unsigned char ucBitSet_,
				 Common::DataArrayData& cTuple,
				 unsigned int& uiTupleID_) const;

	// ビットセットで取り出す -- nullがないもの
	void getBitSet(Common::BitSet& cBitSet_,
				   const ModUInt32* p, int iPosition_) const;
	// ビットセットで取り出す -- nullがあるもの
	void getBitSet(Common::BitSet& cBitSet_,
				   const ModUInt32* p, unsigned char nullBitmap_,
				   int iPosition_) const;

	// フィールド型を得る
	static Data::Type::Value getFieldType(const LogicalFile::FileID& cFileID_,
										  int iPosition_,
										  bool isVersion3_,
										  bool isVersion4_,
										  bool& isFixed_);

	// フィールドサイズを得る
	static ModSize getFieldSize(const LogicalFile::FileID& cFileID_,
								int iPosition_,
								Type::Value eType_);

	// タイプからCommon::Dataを得る
	static Common::Data::Pointer makeData(Type::Value eType_,
										  int precision_, int scale_);

private:
	// 1つデータを得る
	void getData(const ModUInt32*& p,
				 Type::Value eType_,
				 Common::Data& cData_) const;
	
	// 型の配列
	ModVector<Type::Value> m_vecType;
	// すべてのフィールドが固定長の場合、そのサイズ
	ModSize m_Size;
	// ヘッダーがあるか否か
	bool m_bHeader;
};

_SYDNEY_BTREE2_END
_SYDNEY_END

#endif //__SYDNEY_BTREE2_DATA_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2007, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
