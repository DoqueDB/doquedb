// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.h -- Collectionで扱うデータ
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_COLLECTION_DATA_H
#define __SYDNEY_EXECUTION_COLLECTION_DATA_H

#include "Execution/Collection/Module.h"

#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "ModLanguageSet.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

//
//	CLASS
//	Execution::Collection::Data -- コレクションで扱うデータ型
//
//	NOTES
//	コレクションの利用メモリを管理するためと、コレクション解放時にメモリ領域も
//	解放したいため、malloc や new など利用せずに mmap でメモリ確保を行う
//	Common::Data のままではアロケーションサイズにバリエーションがありすぎ、
//	管理にコストがかかるので、B木と同じ仕組みで大小比較できるようにシリアル化
//	することとする
//
class Data
{
	//【注意】	本クラスおよび、クラス内クラスは、
	//			メモリから直接キャストされるので、仮想関数やポインターを
	//			保持するメンバーを定義してはならない
	//			また、バイト境界を意識する必要がある
	
 public:
	//
	//	STRUCT
	//	Execution::Collection::Data::Type -- データ型
	//
	//	NOTES
	//	Common::Dataにデータ型を追加した場合には、ここにも追加すること
	//
	struct Type
	{
		enum Value
		{
			None,
			
			Integer,
			UnsignedInteger,
			Integer64,
			UnsignedInteger64,
			String,
			Float,
			Double,
			Decimal,
			Date,
			DateTime,
			Binary,
			BitSet,				// not supported
			ObjectID,
			Language,
			ColumnMetaData,		// not supported
			Word,
			Word_Df,
			Word_Scale,
			SearchTerm,			// not supported

			Array,

			Null,

			Undefined
		};
	};

	//
	//	CLASS
	//	Execution::Collection::Data::Integer
	//		-- 32ビットInteger型
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

		// 値
		ModInt32 m_value;

	private:
		static ModSize _size() { return sizeof(ModInt32)/sizeof(ModUInt32); }
	};

	//
	//	CLASS
	//	Execution::Collection::Data::UnsignedInteger
	//		-- 32ビットUnsignedInteger型
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
	//	Execution::Collection::Data::Integer64
	//		-- 64ビットInteger型
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

		// 値
		ModInt64 m_value;

	private:
		static ModSize _size() { return sizeof(ModInt64)/sizeof(ModUInt32); }
	};

	//
	//	CLASS
	//	Execution::Collection::Data::UnsignedInteger64
	//		-- 64ビットInteger型
	//
	class UnsignedInteger64
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
		int compare(const UnsignedInteger64& cOther_) const
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
	//	Execution::Collection::Data::String
	//		-- 文字列型
	//
	class String
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const String* s
				= syd_reinterpret_cast<const String*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const String& cOther_) const;

		// 文字列長をユニット長にする
		static ModSize calcUnitSize(ModSize length_)
		{
			// 文字列長分を足して、4バイトバウンダリにする
			return (length_ + 1) / 2 + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		ModUInt32* dump(const ModUnicodeString& cValue_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		void getData(ModUnicodeString& cValue_) const;
		
		// 長さ
		ModSize m_length;
		// 値
		ModUnicodeChar m_value[1];
	};

	//
	//	CLASS
	//	Execution::Collection::Data::Float
	//		-- 32ビットFloat型
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

	//
	//	CLASS
	//	Execution::Collection::Data::Double
	//		-- 64ビットDouble型
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
	//	Execution::Collection::Data::Decimal
	//		-- デシマル型
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
			return calcUnitSize(m_precision, m_scale);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Decimal& cOther_) const;

		// プレシジョンとスケールをユニット長にする
		static ModSize calcUnitSize(int precision_, int scale_);

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		// precision
		int		m_precision;
		// scale
		int		m_scale;
		// 値
		ModUInt32 m_value[1];
	};

	//
	//	CLASS
	//	Execution::Collection::Data::Date -- 日付型
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

	//
	//	CLASS
	//	Execution::Collection::Data::DateTime
	//		-- 日時型
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
	//	Execution::Collection::Data::Binary
	//		-- バイナリ型
	//
	class Binary
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const Binary* s
				= syd_reinterpret_cast<const Binary*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Binary& cOther_) const;

		// 文字列長をユニット長にする
		static ModSize calcUnitSize(ModSize length_)
		{
			// 4バイト単位にする
			return (length_ + 3) / 4 + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		
		// 長さ
		ModSize m_length;
		// 値
		char m_value[1];
	};

	//
	//	CLASS
	//	Execution::Collection::Data::ObjectID
	//		-- ObjectID型
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
	//	Execution::Collection::Data::Language
	//		-- Language型
	//
	class Language
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const Language* s
				= syd_reinterpret_cast<const Language*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const
		{
			return calcUnitSize(m_length);
		}
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Language& cOther_) const;

		// 配列要素長をユニット長にする
		static ModSize calcUnitSize(ModUInt32 uiLength_)
		{
			// 要素数分を足す
			return uiLength_ + 1;
		}

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		ModUInt32* dump(const ModLanguageSet& cValue_);
		// 取り出す
		void getData(Common::Data& cData_) const;
		void getData(ModLanguageSet& cValue_) const;
		
		// 長さ
		ModSize m_length;
		// 値
		ModUInt32 m_value[1];
	};

	//
	//	CLASS
	//	Execution::Collection::Data::Word
	//		-- Word型
	//
	class Word
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const Word* s
				= syd_reinterpret_cast<const Word*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const;
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Word& cOther_) const;
		int compare_df(const Word& cOther_) const;
		int compare_scale(const Word& cOther_) const;

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		// その他のサイズ
		static ModSize getOtherSize()
			{ return (sizeof(double) + sizeof(int) * 2) / 4; }
		
		// スケール
		double m_scale;
		// 文書頻度
		int m_df;
		// カテゴリ
		int m_category;
		// 文字列と言語のデータ領域
		ModUInt32 m_value[1];
	};

	//
	//	CLASS
	//	Execution::Collection::Data::Array
	//		-- DataArrayData型
	//
	class Array
	{
	public:
		// サイズを返す
		static ModSize getSize(const ModUInt32* pBuffer_)
		{
			const Array* s
				= syd_reinterpret_cast<const Array*>(pBuffer_);
			return s->getSize();
		}
		ModSize getSize() const;
		static ModSize getSize(const Common::Data& cData_);

		// 比較関数
		int compare(const Array& cOther_) const;

		// ダンプする
		ModUInt32* dump(const Common::Data& cData_);
		// 取り出す
		void getData(Common::Data& cData_) const;

		// 要素データの先頭を得る
		const ModUInt32* getElementData() const;
		ModUInt32* getElementData();

		// 要素数
		int m_count;
		// 型
		int m_type[1];
	};

	// コンストラクタ
	Data();
	
	// デストラクタ
	~Data();

	// サイズを得る
	static ModSize getSize(const ModUInt32* p);
	// サイズを得る
	static ModSize getSize(const Common::DataArrayData& cData_);
	
	// 1つのサイズを得る
	static ModSize getSize(const Common::Data& cData_);
	// 1つのサイズを得る
	static ModSize getSize(const ModUInt32*& p, Type::Value eType_);

	// ダンプする
	void dump(const Common::DataArrayData& cData_);
	// 1つダンプする
	static void dump(ModUInt32*& p, const Common::Data& cData_);

	// データを得る
	void getData(Common::DataArrayData& cTuple_) const;

	// 要素を得る
	const ModUInt32* getElement(int n_, Type::Value& t_) const;

	// 比較する
	int compare(const Data* other_,
				int iElement_, Type::Value t_ = Type::None) const;
	// 比較する
	static int compare(const ModUInt32* p0_, Type::Value t0_,
					   const ModUInt32* p1_, Type::Value t1_);

	// タイプを得る
	static Type::Value getType(const Common::Data& cData_);
	// Common::Dataを得る
	static Common::Data::Pointer makeData(Type::Value eType_);

	// 1つデータを得る
	static void getData(const ModUInt32*& p,
						Type::Value eType_,
						Common::Data& cData_);
	
private:
	// 要素数
	int m_count;
	// 型
	int m_type[1];
};

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif //__SYDNEY_EXECUTION_COLLECTION_DATA_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
