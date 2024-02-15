// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Key.h -- キーオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2009, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_KEY_H
#define	__SYDNEY_SCHEMA_KEY_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "ModTypes.h"

_SYDNEY_BEGIN

namespace Statement
{
	class ColumnName;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Index;
class Table;
class Column;
class Field;
namespace SystemTable
{
	class Key;
}

//	CLASS
//	Schema::Key -- キーオブジェクトを表すクラス
//
//	NOTES
//		キーの親オブジェクトは、索引である
//		キー ID は、データベース内で一意に生成される

class Key
	: public	Object
{
public:
	friend class SystemTable::Key;

	//	TYPEDEF
	//	Schema::Key::Position --
	//		索引の先頭から何番目のキーかを表す値の型
	//
	//	NOTES

	typedef ModSize			Position;

	//	CLASS
	//	Schema::Key::Order -- キー値の順序を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Order
	{
		//	ENUM
		//	Schema::Key::Order::Value -- キー値の順序の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			Ascending,							// 昇順
			Descending,							// 降順
			ValueNum							// 順序数
		};
	};

	//	CLASS
	//	Schema::Key::Attribute -- キーの属性を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Attribute
	{
		//	ENUM
		//	Schema::Key::Attribute::Value -- キーの属性を表す列挙型
		//
		//	NOTES

		typedef unsigned int Value;
		enum _Value
		{
			None		= 0x00,					// 何もなし
			Searchable	= 0x01,					// 検索キーに使用可能
			NotNull		= 0x02					// NULL値不可
		};
	};

	struct Log
	{
		enum Value
		{
			ColumnID = 0,						// column ID
			Order,								// key order
			Num0,
			ID = Num0,							// ID
			Num1,
			Num = Num1
		};
	};

	//	TYPEDEF public
	//	Schema::Key::Pointer -- Keyを保持するObjectPointer
	//
	//	NOTES

	typedef KeyPointer Pointer;

	// コンストラクター
	Key();
	// デストラクター
	~Key();

	// DataArrayDataを元にインスタンスを生成する
	static Key*				getNewInstance(const Common::DataArrayData& cData_);

	// メンバーをすべて初期化する
	void					clear();

	// キーのスキーマ情報を生成する
	static Pointer			create(Index& index, Position position,
								   const Table& table,
								   const Statement::ColumnName& statement,
								   Trans::Transaction& cTrans_,
								   ID::Value iID_ = ID::Invalid);
	static Pointer			create(Index& index, Position position,
								   const Table& table,
								   ID::Value columnID,
								   Trans::Transaction& cTrans_,
								   ID::Value iID_ = ID::Invalid);
	static Pointer			create(Index& index, Position position,
								   const Table& table,
								   const Common::DataArrayData& cLogData_,
								   Trans::Transaction& cTrans_,
								   ID::Value iID_ = ID::Invalid);

	// キーを表すクラスを得る
	SYD_SCHEMA_FUNCTION
	static Key*				get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Key*				get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);

	// 陳腐化していないか調べる
	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);

	// 永続化前に行う処理
	static void				doBeforePersist(const Pointer& pKey_, Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
	// 永続化後に行う処理
	static void				doAfterPersist(const Pointer& pKey_, Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pKey_,
										Index& cIndex_,
										Trans::Transaction& cTrans_);

//	Object::
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	// 下位オブジェクトを抹消する
	void					reset(Database& cDatabase_);

	// キーが索引の先頭から何番目に位置するか(0番から)
	SYD_SCHEMA_FUNCTION
	Position				getPosition() const;
	// キー値の順序を得る
	SYD_SCHEMA_FUNCTION
	Order::Value			getOrder() const;

	// キーが存在する索引を得る
	SYD_SCHEMA_FUNCTION
	Index*					getIndex(Trans::Transaction& cTrans_) const;

	// キーにした列を得る
	SYD_SCHEMA_FUNCTION
	Column*					getColumn(Trans::Transaction& cTrans_) const;
	// キーにした列の オブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getColumnID() const;
	// キーにした列の オブジェクト ID を設定する
	void					setColumnID(ID::Value id_);

	// キーにした列を設定する
	const Column&			setColumn(Column& column);

	// キーの値を格納するフィールドを得る
	SYD_SCHEMA_FUNCTION
	Field*					getField(Trans::Transaction& cTrans_) const;
	// キーの値を格納するフィールドを設定する
	const Field&			setField(Field& field);

	// キーの値を格納するフィールドのIDを得る
	ID::Value				getFieldID() const;
	// キーの値を格納するフィールドのIDを設定する
	void					setFieldID(ID::Value id_);

	// キーにした列をKeyMapを操作しながら集めるための関数
	void					appendColumn(ModVector<Column*>& vecColumns_,
										 Trans::Transaction& cTrans_);
#ifdef OBSOLETE // Fieldを得る機能は使用されない
	// 値を格納するフィールドをKeyMapを操作しながら集めるための関数
	void					appendField(ModVector<Field*>& vecFields_,
										 Trans::Transaction& cTrans_);
#endif

	// キーが検索に使用できるかを得る
	SYD_SCHEMA_FUNCTION
	bool					isSearchable() const;

	// キーがNULL可能かを得る
	bool					isNullable() const;
	// キーがNULL可能かを設定する
	void					setNullable(bool v_);

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// Vector上でのソートに使う関数
	virtual bool			isLessThan(const Object& cOther_) const;

	// 論理ログ出力用のメソッド
	void					makeLogData(Common::DataArrayData& cLogData_) const;
												// ログデータを作る

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

protected:
//	Object::
//	ID::Value				setID(ID::Value id);
//												// オブジェクト ID を設定する
//	ID::Value				setParentID(ID::Value parent);
//												// 親オブジェクトの
//												// オブジェクト ID を設定する
//	const Name&				setName(const Name& name);
//												// オブジェクトの名前を設定する
//	Category::Value			setCategory(Category::Value category);
//												// オブジェクトの種別を設定する

private:
	// コンストラクター
	Key(const Index& index, Position position, const Table& table,
		const Statement::ColumnName& statement);
	Key(const Index& index, Position position, const Table& table,
		Column& column);

	void					setName(Trans::Transaction& cTrans_);
	void					setAttribute(const Index& cIndex_,
										 const Column* pColumn_);

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「キー」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Position				_position;			// 索引の先頭から何番目のキーか
	Order::Value			_order;				// キー値の順序
	Attribute::Value		_attribute;			// キーの属性

	// このキーが属する索引を表すクラス

	mutable Index*			m_pIndex;

	// キーにした列を表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Column*			_column;
	ID::Value				_columnID;

	// このキーの値を格納するフィールドを表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Field*			_field;
	ID::Value				_fieldID;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_KEY_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2009, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
