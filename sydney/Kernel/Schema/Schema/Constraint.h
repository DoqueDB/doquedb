// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Constraint.h -- 制約オブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_CONSTRAINT_H
#define	__SYDNEY_SCHEMA_CONSTRAINT_H

#include "Schema/Module.h"
#include "Schema/Object.h"
#include "Schema/Hint.h"

#include "ModTypes.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement {
	class TableConstraintDefinition;
}

namespace Trans {
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Index;
class Table;
namespace SystemTable
{
	class Constraint;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::Constraint -- 制約オブジェクトを表すクラス
//
//	NOTES
//		制約の親オブジェクトは、表である

class Constraint
	: public	Object
{
public:
	friend class SystemTable::Constraint;

	//	TYPEDEF
	//	Schema::Constraint::Position --
	//		表の先頭から何番目の制約かを表す値の型
	//
	//	NOTES

	typedef	ModSize			Position;

	//	CLASS
	//	Schema::Constraint::Category -- 制約の種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Schema::Constraint::Category::Value -- 制約の種別の値を表す列挙型
		//
		//	NOTES
		//	If following enum was changed, Plan/RelationNode_Check_Constraint should be modified

		enum Value
		{
			Unknown =		0,					// 不明
			OldPrimaryKey,						// Primary key (old version)
			OldUnique,							// Unique (old version)
			//-------------------------
			// above categories are left for backward compatibility
			//-------------------------
			PrimaryKey,							// Primary key
			Unique,								// Unique
			ForeignKey,							// Foreign key
			ReferedKey,							// Implicit constraint for foreign key
			ValueNum
		};
	};

	struct Log {

		//	ENUM
		//		Schema::Log::Value -- ログの共通要素位置
		//
		//	NOTES
		enum Value {
			ID = 0,						// ID
			Name,						// 制約名
			Category,					// 種別
			ColumnIDs,					// 列ID配列(索引IDなどを含む場合あり)
			Hint,						// ヒント
			ValueNum0,
			IndexDefinition = ValueNum0,// 索引定義
			ValueNum1,
			ValueNum = ValueNum1
		};
	};

	//	TYPEDEF public
	//	Schema::Constraint::Pointer -- Constraintを保持するObjectPointer
	//
	//	NOTES

	typedef ConstraintPointer Pointer;

	// コンストラクター
	Constraint();
	// デストラクター
	~Constraint();

	static Constraint*		getNewInstance(const Common::DataArrayData& cData_);
												// DataArrayDataを元にインスタンスを生成する

	void					clear();			// メンバーをすべて初期化する

	// 制約のスキーマ定義を生成する
	static Pointer			create(Table& cTable_, Position iPosition_,
								   const Statement::TableConstraintDefinition& cStatement_,
								   Trans::Transaction& cTrans_);
	static Pointer			create(Table& cTable_, const Index& cIndex_,
								   const Index& cReferedIndex_,
								   Trans::Transaction& cTrans_,
								   ID::Value iID_ = ID::Invalid);
	static Pointer			create(Table& cTable_, Position iPosition_,
								   const Common::DataArrayData& cLogData_,
								   Trans::Transaction& cTrans_);
	// create corresponding files
	void					create(Trans::Transaction& cTrans_);

	// 制約のスキーマ定義を破棄する
	void					drop(Trans::Transaction& cTrans_,
								 bool bRecovery_ = false, bool bNoUnset_ = false);

	SYD_SCHEMA_FUNCTION
	static Constraint*		get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Constraint*		get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);
												// 制約を表すクラスを得る

	// Check category value
	bool isPrimaryKey() const;			// primary key constraint?
	bool isUnique() const;				// unique constraint?
	bool isForeignKey() const;			// foreign key constraint?
	bool isReferedKey() const;			// implicit constraint for foreign key?
	bool isReferenceConstraint() const;	// foreign key or refered key?

	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);
												// 陳腐化していないか調べる

	static void				doBeforePersist(const Pointer& pConstraint_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pConstraint_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pConstraint_,
										Table& cTable_,
										Trans::Transaction& cTrans_);
	void					doAfterLoad(Table& cTable_,
										Trans::Transaction& cTrans_);

//	Object::
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	void					reset(Database& cDatabase_);
												// 下位オブジェクトを抹消する

	// 列ID配列を得る
	SYD_SCHEMA_FUNCTION
	const ModVector<ID::Value>&
							getColumnID() const;
	// 列ID配列を設定する
	void					setColumnID(const ModVector<ID::Value>& vecID_);

	void					resetColumnID();	// 列ID配列を初期化する
	void					clearColumnID();	// 列ID配列を破棄する

	SYD_SCHEMA_FUNCTION
	Position				getPosition() const;
												// 制約が表の先頭から
												// 何番目に位置するか
	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// 制約の種別を得る
	SYD_SCHEMA_FUNCTION
	bool					isClustered() const;
												// clustered指定の有無を得る

	SYD_SCHEMA_FUNCTION
	Table*					getTable(Trans::Transaction& cTrans_) const;
												// 制約が存在する表を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getTableID() const;	// 制約が存在する表の
												// オブジェクトIDを得る

	// 制約に対応して作成された索引のIDを得る
	ID::Value				getIndexID() const;
	// 制約に対応して作成された索引を得る
	Index*					getIndex(Trans::Transaction& cTrans_) const;
	// 制約に対応して作成された索引のIDをセットする
	void					setIndexID(ID::Value iID_);
	// 制約に対応して作成された索引のキーIDの配列を得る
	const ModVector<ID::Value>& getIndexKeyID() const;
	// 制約に対応して作成された索引のキーIDの配列をセットする
	void setIndexKeyID(const ModVector<ID::Value>& vecKeyIDs_);

	// 外部キー制約の参照する表のIDを得る
	ID::Value				getReferedTableID() const;
	// 外部キー制約の参照する索引のIDを得る
	ID::Value				getReferedIndexID() const;
	// 外部キー制約と対となる制約のIDを得る
	ID::Value				getReferedConstraintID() const;
	// 外部キー制約と対となる制約のIDをセットする
	void					setReferedConstraintID(ID::Value iID_);

	Name					createName(const Name& cParentName_);
												// 制約の名前を生成する
	void					rename(const Name& cName_);
											// 名前を変更する

	// 制約のヒントを得る
	const Hint*				getHint() const;
	// 制約のヒントを設定する
	void					setHint(Hint* pHint);
	// 制約のヒントを消去
	void					clearHint();

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// 論理ログ出力用のメソッド
	void					makeLogData(Trans::Transaction& cTrans_,
										Common::DataArrayData& cLogData_) const;
												// ログデータを作る
	static ID::Value		getObjectID(const Common::DataArrayData& cLogData_);
												// ログに記録されているIDを得る

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

	// read/write IDs (used in pack/unpack)
	const ModVector<ID::Value>& getIDs() const;
	void setIDs(const ModVector<ID::Value>& vecIDs_);

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
	// ログ用にヒントを変換する
	Common::Data::Pointer packHint() const;
	bool unpackHint(const Common::Data* pData_);

	// コンストラクター
	Constraint(const Table& cTable_, Position iPosition_,
			   const Statement::TableConstraintDefinition& cStatement_);
	Constraint(const Table& cTable_, const Index& cIndex_, const Index& cReferedIndex_);
	Constraint(const Table& cTable_, Position iPosition_,
			   const Common::DataArrayData& cLogData_,
			   Trans::Transaction& cTrans_);
	void					destruct();			// デストラクター下位関数

	// Undo情報を検査して反映する
	void					checkUndo(const Database& cDatabase_, const Table& cTable_);

	// create corresponding index
	void createIndex(Trans::Transaction& cTrans_, Table& cTable_,
					 const Common::DataArrayData* pLogData_ = 0);

	// check an constraint whether it corresponds to a foreign key
	ID::Value checkForeignKey(Trans::Transaction& cTrans_,
							  const Table& cTable_, const Constraint& cConstraint_,
							  const ModVector<ID::Value>& vecReferedColumnID_);

	// determine refered index ID from table constraint definition
	ID::Value getReferedIndexID(Trans::Transaction& cTrans_,
								const Table& cReferedTable_,
								const Statement::TableConstraintDefinition& cStatement_);

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「制約」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Position				_position;			// 表の先頭から何番目の制約か
	Category::Value			_category;			// 制約の種別
	bool					_clustered;			// clustered指定の有無
	Hint*					m_pHint;			// 制約に対するヒント

	mutable Table*			m_pTable;			// 制約が存在する表のオブジェクト
	ModVector<ID::Value>*	m_vecColumnID;		// 対象列のオブジェクトID

	ID::Value				m_iIndexID;			// 制約に対応して作成される索引のID
	ID::Value				m_iReferedTableID;	// 外部キー制約が使用する表のID
	ID::Value				m_iReferedIndexID;	// 外部キー制約が使用する索引のID
	ID::Value				m_iReferedConstraintID;	// 外部キー制約が対で使用する制約のID

	ModVector<ID::Value>	m_vecKeyID;

	mutable ModVector<ID::Value>* m_vecID;		// pack/unpackでのみ使われる
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_CONSTRAINT_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
