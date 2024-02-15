// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Name.h -- ロック名関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOCK_NAME_H
#define	__SYDNEY_LOCK_NAME_H

#include "Lock/Module.h"

_SYDNEY_BEGIN
_SYDNEY_LOCK_BEGIN

//	CLASS
//	Lock::Name -- ロック名を表すクラス
//
//	NOTES

class Name
{
public:
	//	STRUCT
	//	Lock::Name::Category -- ロック名の種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Lock::Name::Category::Value -- ロック名の種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			//【注意】	以下の列挙の順序は重要
			//			Trans, Schema でこの列挙型を使った配列を作っている

			Unknown =			0,				// 不明
			Database,							// データベース
			Table,								// 表
			Tuple,								// タプル
			LogicalLog,							// 論理ログファイル
			File,								// ファイル
			ValueNum							// 値の種別数
		};
	};

	//	TYPEDEF
	//	Lock::Name::Value -- ロック名の値を表す型
	//
	//	NOTES
	//		ロック名の値とは、名前から生成されたハッシュ値である

	typedef	unsigned int	Value;

	//	TYPEDEF
	//	Lock::Name::Part -- ロック名を構成する一部分を表す型
	//
	//	NOTES

	typedef	unsigned int	Part;

	//【注意】	Microsoft C++ Compiler Version 13.10.3077 で、
	//			~static_cast<Part>(0) と書くと C2226 のエラーになる

	// デフォルトコンストラクター
	Name(Category::Value category = Category::Unknown,
		 Part part0 = ~(static_cast<Part>(0)),
		 Part part1 = ~(static_cast<Part>(0)),
		 Part part2 = ~(static_cast<Part>(0)));
	// コピーコンストラクター
	Name(const Name& src);

	// = 演算子
	Name&					operator =(const Name& r);
	// * 単項演算子
	Value					operator *() const;
	// == 演算子
	SYD_LOCK_FUNCTION
	bool					operator ==(const Name&	r) const;
	// != 演算子
	bool					operator !=(const Name&	r) const;

	// ロック名の種別を得る
	Category::Value			getCategory() const;
	// 値を得る
	SYD_LOCK_FUNCTION
	Value					getValue() const;
	// 親のロック名を得る
	SYD_LOCK_FUNCTION
	const Name				getParent() const;
	// 与えられたロック名が自分の表すオブジェクトの子孫か
	SYD_LOCK_FUNCTION
	bool					isDescendant(const Name& r) const;

protected:
	// ロック名を構成する部分を格納するためのバッファ領域
	Part					_buf[3];

private:
	// コンストラクター下位関数
	SYD_LOCK_FUNCTION
	void
	construct(Category::Value category, const Part* parts);
	SYD_LOCK_FUNCTION
	void
	construct(Category::Value category, Part part0, Part part1, Part part2);

	// ロック名の種別
	Category::Value			_category;
};

//	CLASS
//	Lock::DatabaseName -- データベース用のロック名を表すクラス
//
//	NOTES

class DatabaseName
	: public	Name
{
public:
	// デフォルトコンストラクター
	DatabaseName(Part db = ~(static_cast<Part>(0)));
#ifdef OBSOLETE
	// ロック名のデータベース部分を得る
	Part		getDatabasePart() const;
#endif
};

//	CLASS
//	Lock::TableName -- テーブル用のロック名を表すクラス
//
//	NOTES

class TableName
	: public	Name
{
public:
	// デフォルトコンストラクター
	TableName(Part db = ~(static_cast<Part>(0)),
			  Part table = ~(static_cast<Part>(0)));
#ifdef OBSOLETE
	// ロック名のデータベース部分を得る
	Part		getDatabasePart() const;
	// ロック名のテーブル部分を得る
	Part		getTablePart() const;
#endif
};

//	CLASS
//	Lock::TupleName -- タプル用のロック名を表すクラス
//
//	NOTES

class TupleName
	: public	Name
{
public:
	// デフォルトコンストラクター
	TupleName(Part db = ~(static_cast<Part>(0)),
			  Part table = ~(static_cast<Part>(0)),
			  Part tuple = ~(static_cast<Part>(0)));
#ifdef OBSOLETE
	// ロック名のデータベース部分を得る
	Part		getDatabasePart() const;
	// ロック名のテーブル部分を得る
	Part		getTablePart() const;
	// ロック名のタプル部分を得る
	Part		getTuplePart() const;
#endif
};

//	CLASS
//	Lock::FileName -- ファイル用のロック名を表すクラス
//
//	NOTES

class FileName
	: public	Name
{
public:
	// デフォルトコンストラクター
	FileName(Part db = ~(static_cast<Part>(0)),
			 Part table = ~(static_cast<Part>(0)),
			 Part file = ~(static_cast<Part>(0)));

	// ロック名のデータベース部分を得る
	Part		getDatabasePart() const;
	// ロック名のテーブル部分を得る
	Part		getTablePart() const;
	// ロック名のファイル部分を得る
	Part		getFilePart() const;
};

//	CLASS
//	Lock::LogicalLogName -- 論理ログファイル用のロック名を表すクラス
//
//	NOTES

class LogicalLogName
	: public	Name
{
public:
	// デフォルトコンストラクター
	LogicalLogName(Part db = ~(static_cast<Part>(0)));

	// ロック名のデータベース部分を得る
	Part		getDatabasePart() const;
};

//	FUNCTION public
//	Lock::Name::Name -- ロック名を表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Category::Value	category
//			指定されたとき
//				生成するロック名の種別
//			指定されないとき
//				Lock::Name::Unknown が指定されたものとみなす
//		Lock::Name::Part	part0
//			指定されたとき
//				ロック名の先頭から0番目の部分
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	part1
//			指定されたとき
//				ロック名の先頭から1番目の部分
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	part2
//			指定されたとき
//				ロック名の先頭から2番目の部分
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Name(Category::Value category, Part part0, Part part1, Part part2)
{
	construct(category, part0, part1, part2);
}

//	FUNCTION public
//	Lock::Name::Name -- ロック名を表すクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Name&	src
//			生成されたロック名に代入するロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Name(const Name& src)
{
	construct(src.getCategory(), src._buf);
}

//	FUNCTION public
//	Lock::Name::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name&			r
//			自分自身に代入するロック名
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS
//		なし

inline
Lock::Name&
Lock::Name::operator =(const Name& r)
{
	if (this != &r)
		construct(r.getCategory(), r._buf);

	return *this;
}

//	FUNCTION public
//	Lock::Name::operator * -- * 単項演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		名前から生成したハッシュ値
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Value
Lock::Name::operator *() const
{
	return getValue();
}

//	FUNCTION public
//	Lock::Name::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		const Lock::Name&	r
//			自分自身と比較するロック名
//
//	RETURN
//		true
//			与えられたロック名と自分自身は等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
Lock::Name::operator !=(const Name& r) const
{
	return !(*this == r);
}

//	FUNCTION public
//	Lock::Name::getCategory -- ロック名の種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたロック名の種別
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Category::Value
Lock::Name::getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Lock::DatabaseName::DatabaseName --
//		データベース用のロック名を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	db
//			指定されたとき
//				ロック名を生成するデータベースののスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
DatabaseName::DatabaseName(Part db)
	: Name(Name::Category::Database, db)
{}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::DatabaseName::getDatabasePart --
//		データベース用のロック名のデータベース部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたデータベース用のロック名のデータベース部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::DatabaseName::getDatabasePart() const
{
	return _buf[0];
}
#endif

//	FUNCTION public
//	Lock::TableName::TableName --
//		テーブル用のロック名を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	db
//			指定されたとき
//				ロック名を生成するテーブルの存在する
//				データベースのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	table
//			指定されたとき
//				ロック名を生成するテーブルのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TableName::TableName(Part db, Part table)
	: Name(Name::Category::Table, db, table)
{}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::TableName::getDatabasePart --
//		テーブル用のロック名のデータベース部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたテーブル用のロック名のデータベース部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::TableName::getDatabasePart() const
{
	return _buf[0];
}

//	FUNCTION public
//	Lock::TableName::getTablePart --
//		テーブル用のロック名のテーブル部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたテーブル用のロック名のテーブル部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::TableName::getTablePart() const
{
	return _buf[1];
}
#endif

//	FUNCTION public
//	Lock::TupleName::TupleName --
//		タプル用のロック名を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	db
//			指定されたとき
//				ロック名を生成するタプルの存在する
//				データベースのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	table
//			指定されたとき
//				ロック名を生成するタプルの存在する
//				テーブルのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	tuple
//			指定されたとき
//				ロック名を生成するタプルのタプルID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TupleName::TupleName(Part db, Part table, Part tuple)
	: Name(Name::Category::Tuple, db, table, tuple)
{}

#ifdef OBSOLETE
//	FUNCTION public
//	Lock::TupleName::getDatabasePart --
//		タプル用のロック名のデータベース部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタプル用のロック名のデータベース部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::TupleName::getDatabasePart() const
{
	return _buf[0];
}

//	FUNCTION public
//	Lock::TupleName::getTablePart --
//		タプル用のロック名のテーブル部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタプル用のロック名のテーブル部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::TupleName::getTablePart() const
{
	return _buf[1];
}

//	FUNCTION public
//	Lock::TupleName::getTuplePart --
//		タプル用のロック名のファイル部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタプル用のロック名のファイル部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::TupleName::getTuplePart() const
{
	return _buf[2];
}
#endif

//	FUNCTION public
//	Lock::FileName::FileName --
//		ファイル用のロック名を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	db
//			指定されたとき
//				ロック名を生成するファイルの存在する
//				データベースのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	table
//			指定されたとき
//				ロック名を生成するファイルの存在する
//				テーブルのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//		Lock::Name::Part	file
//			指定されたとき
//				ロック名を生成するファイルのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
FileName::FileName(Part db, Part table, Part file)
	: Name(Category::File, db, table, file)
{}

//	FUNCTION public
//	Lock::FileName::getDatabasePart --
//		ファイル用のロック名のデータベース部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル用のロック名のデータベース部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::FileName::getDatabasePart() const
{
	return _buf[0];
}

//	FUNCTION public
//	Lock::FileName::getTablePart --
//		ファイル用のロック名のテーブル部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル用のロック名のテーブル部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::FileName::getTablePart() const
{
	return _buf[1];
}

//	FUNCTION public
//	Lock::FileName::getFilePart --
//		ファイル用のロック名のファイル部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル用のロック名のファイル部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::FileName::getFilePart() const
{
	return _buf[2];
}

//	FUNCTION public
//	Lock::LogicalLogName::LogicalLogName --
//		論理ログファイル用のロック名を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Lock::Name::Part	db
//			指定されたとき
//				ロック名を生成する論理ログファイルに記録する操作を行う
//				データベースのスキーマオブジェクトID
//			指定されないとき
//				~static_cast<Lock::Name::Part>(0) が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
LogicalLogName::LogicalLogName(Part db)
	: Name(Category::LogicalLog, db)
{}

//	FUNCTION public
//	Lock::LogicalLogName::getDatabasePart --
//		論理ログファイル用のロック名のデータベース部分を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた論理ログファイル用のロック名のデータベース部分
//
//	EXCEPTIONS
//		なし

inline
Lock::Name::Part
Lock::LogicalLogName::getDatabasePart() const
{
	return _buf[0];
}

_SYDNEY_LOCK_END
_SYDNEY_END

#endif	// __SYDNEY_LOCK_NAME_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
