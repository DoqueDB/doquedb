// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.h -- ベクターで扱うデータ
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR2_DATA_H
#define __SYDNEY_VECTOR2_DATA_H

#include "Vector2/Module.h"

#include "ModTypes.h"
//@@#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
	class DataArrayData;
}
namespace LogicalFile
{
	class FileID;
}

_SYDNEY_VECTOR2_BEGIN

class FileID;

//
//	CLASS
//	Vector2::Data -- ベクターで扱うデータ型
//
//	NOTES
//	【Dataとは】
//	メモリに書き込まれるデータのこと。
//	ベクターのキーは、メモリの位置で表現するので書き込まない。
//	ベクターのバリューは、使用可能な型に制限がある。
//
//	【キーの扱い方】
//	Vectorはキーをファイルに保存しないので、どこかでキーを無視することが必要。
//	オブジェクト指向的にはキーをLogicalInterfaceで除いといて、
//	与えられたタプルを先頭から書き込めることが望ましい。
//	しかしキーを除いたタプルをわざわざ作るのもコストがかかるので、
//	ここで取り除くことにする。
//
//	[?] Btree2との相違点
//	Common::Objectを継承していない
//	getSize(),dump(),getdata()がstatic。
//	dump(),getdata()の返り値がvoid。
//
//	[?] FileIDもm_vecFieldTypeとm_uiFieldSizeを持っている。
//		FileIDにはDataへの参照を持たせれば十分？
//		今のところDataのm_size, getSize, getCountは使われていない。
//
class Data
{
 public:
	//
	//	STRUCT
	//	Vector2::Data::Type -- データ型
	//
	//	NOTES
	//	ここにあるデータ型以外の型はベクターに挿入することはできない
	//
	struct Type
	{
		enum Value
		{
			Integer,				// not supported
			UnsignedInteger,		// supported with a constraint
			Float,					// not supported
			Double,					// not supported
			Date,					// not supported
			DateTime,				// not supported
			ObjectID
		};
	};

	// 最大フィールド数
	// FileIDで管理するとヘッダが入れ子になってしまう。
	enum { MaxFieldCount = 8 };

	//
	//	CLASS
	//	Vector2::Data::UnsignedInteger -- UnsignedInteger型
	//
	//	NOTES
	//	■ 制約
	//	0xffffffff は格納できない。Vector2内部で、nullとして扱うため。
	//		（補足）
	//		nullを格納するときは 0xffffffff を格納し、
	//		0xffffffff を取得したら、setNull()したCommon::Dataを返す。
	//		なので、0xffffffff を受け渡しすることはない。
	//
	class UnsignedInteger
	{
	public:
		// サイズを返す
		static ModSize getSize()
		{
			return sizeof(unsigned int);
		}
		// ダンプする
		static void dump(const Common::Data& cData_, char*& buf_);
		// 取り出す
		static void getData(Common::Data& cData_, const char*& buf_);
		// nullかどうか
		static bool isNull(const char* buf_);
		// 初期化する
		static void reset(char*& buf_);
	};
	
	//
	//	CLASS
	//	Vector2::Data::ObjectID -- ObjectID型
	//
	class ObjectID
	{
	public:
		// サイズを返す
		static ModSize getSize()
		{
			return 6;
		}
		// ダンプする
		static void dump(const Common::Data& cData_, char*& buf_);
		// 取り出す
		static void getData(Common::Data& cData_, const char*& buf_);
		// nullかどうか
		static bool isNull(const char* buf_);
		// 初期化する
		static void reset(char*& buf_);
	};

	// コンストラクタ
	Data();
	// デストラクタ
	~Data();

	// 型配列を設定する
	void setType(const FileID& cFileID_);

	// 型配列数を得る
	ModSize getCount() const { return m_uiFieldCount; }
	// サイズを得る
	ModSize getSize() const { return m_Size; }

	// ダンプする
	ModSize dump(char* p, const Common::DataArrayData& cData_) const;
	// 更新する
	ModSize update(char* p, const Common::DataArrayData& cData_,
				   const int* pUpdateField_,
				   const ModSize uiFieldCount_) const;

	// すべてのフィールドを初期化する
	ModSize reset(char* p) const;
	// すべてのフィールドがnullかどうか
	bool isNull(const char* p) const;

	// データを得る
	bool getData(const char* p,
				 Common::DataArrayData& cTuple_,
				 const int* pGetField_,
				 const ModSize uiFieldCount_) const;

	// フィールドタイプを得る
	static Type::Value getFieldType(const LogicalFile::FileID& cFileID_,
									int iPosition_,
									ModSize& uiFieldSize_);

 private:
	// 1つダンプする
	void dump(char*& p,
			  const Common::Data& cData_,
			  Type::Value eType_) const;
	// 1つデータを得る
	void getData(const char*& p,
				 Type::Value eType_,
				 Common::Data& cData_) const;
	// １つのデータを初期化する
	void reset(char*& p,
			   Type::Value eType_) const;
	
	// 1つのサイズを得る
	static ModSize getSize(Type::Value eType_);
	// 1つのデータがnullかどうかを得る
	static bool isNull(const char* p,
					   Type::Value eType_);


	// 型の配列
	Type::Value m_pType[MaxFieldCount];
	// フィールドの個数
	ModSize m_uiFieldCount;
	// フィールドサイズの合計
	ModSize m_Size;
};

_SYDNEY_VECTOR2_END
_SYDNEY_END

#endif //__SYDNEY_VECTOR2_DATA_H

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
