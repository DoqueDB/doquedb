// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Column.h -- 列オブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_COLUMN_H
#define	__SYDNEY_SCHEMA_COLUMN_H

#include "Schema/Module.h"
#include "Schema/Object.h"
#include "Schema/Default.h"
#include "Schema/Hint.h"

#include "Common/SQLData.h"

#include "ModTypes.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common
{
	class ColumnMetaData;
}
namespace Os
{
	class Path;
}
namespace Statement
{
	class ColumnDefinition;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Table;
class Field;
class File;
#ifdef OBSOLETE // ColumnについているKeyはSource-Destinationでたどるので以下は使用しない
class Key;
#endif
class Sequence;
namespace SystemTable
{
	class Column;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::Column -- 列オブジェクトを表すクラス
//
//	NOTES
//		列の親オブジェクトは、表である

class Column
	: public	Object
{
public:
	friend class SystemTable::Column;

	//	TYPEDEF
	//	Schema::Column::Position --
	//		表の先頭から何番目の列かを表す値の型
	//
	//	NOTES

	typedef ModSize			Position;

	//	CLASS
	//	Schema::Column::Category -- 列の種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Schema::Column::Category::Value -- 列の種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			TupleID,							// タプル ID 列
			Normal,								// 通常列
			Constant,							// 定数列
			Function,							// 関数列
			ValueNum							// 種別数
		};
	};

	//	CLASS
	//	Schema::Column::Function -- 関数列の種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している
	struct Function
	{
		//	ENUM
		//	Schema::Column::Function::Value -- 関数列の種別の値を表す列挙型
		//
		//	NOTES
		enum Value
		{
			Unknown = 0,
			ColumnMetaData,
			FileSize,
			IndexHint,
			PrivilegeFlag,
			PrivilegeObjectType,
			PrivilegeObjectID,
			PartitionCategory,
			FunctionRoutine,
			DatabasePath,
			DatabaseMasterURL,
			ValueNum
		};
	};

	//	CLASS
	//	Schema::Column::DataType -- 列に格納する型を表すクラス
	//
	//	NOTES

	class DataTypeOld;

	class DataType : public Common::SQLData
	{
	public:
		typedef Common::SQLData Super;

		// コンストラクター
		DataType();
		DataType(const Common::SQLData& cDataType_);
		// デストラクター
		~DataType();

		// 代入演算子
		DataType& operator=(const DataType& cDataType_);
		DataType& operator=(const Common::SQLData& cDataType_);
		DataType& operator=(const DataTypeOld& cDataType_);

		// データ型として許されるか調べる
		void check(const Schema::Object::Name& cName_) const;

		// データ型に対応するフィールド型と長さをセットする
		void setFieldType(Common::DataType::Type* pTypeTarget_,
						  ModSize* pLengthTarget_,
						  int* pScaleTarget_,
						  bool* pTargetFixed_) const;

	private:
		friend class Column;
	};

	//	CLASS
	//	Schema::Column::DataTypeOld -- 列に格納する型を表すクラス
	//
	//	NOTES
	//	過去との互換性のため

	class DataTypeOld : public Externalizable
	{
	public:
		// コンストラクター
		DataTypeOld();
		// デストラクター
		~DataTypeOld();

		// アクセッサ
		Common::SQLData::Type::Value getType() const { return m_eType; }
		Common::SQLData::Flag::Value getFlag() const { return m_eFlag; }
		int getLength() const { return m_iLength; }
		int getMaxCardinality() const { return m_iCardinality; }

		// シリアル化のためのメソッド
		int getClassID() const;
		void serialize(ModArchive& cArchiver_);

	private:
		friend class Column;

		Common::SQLData::Type::Value m_eType;
		Common::SQLData::Flag::Value m_eFlag;
		int m_iLength;
		int m_iCardinality;
	};

	//	TYPEDEF public
	//	Schema::Column::Pointer -- Columnを保持するObjectPointer
	//
	//	NOTES

	typedef ColumnPointer Pointer;

	// コンストラクター
	Column();
	// デストラクター
	~Column();

	static Column*			getNewInstance(const Common::DataArrayData& cData_);
												// DataArrayDataを元にインスタンスを生成する

	void					clear();			// メンバーをすべて初期化する

	static Pointer			create(Table& table, Position position,
								   const Name& name, Category::Value category,
								   const Common::SQLData& type,
								   const Default& def,
								   Trans::Transaction& cTrans_);
	static Pointer			create(Table& table, Position position,
								   const Statement::ColumnDefinition& cStmt_,
								   Trans::Transaction& cTrans_);
	static Pointer			create(Table& table, Position position,
								   const Common::DataArrayData& cLogData_,
								   Trans::Transaction& cTrans_,
								   const Column* pOriginalColumn_ = 0);
												// 列のスキーマ情報を生成する

	static Pointer			createSystem(Trans::Transaction& cTrans_,
										 Table& table, Position position,
										 const Name& name, Category::Value category,
										 const Common::SQLData& type,
										 const Default& def,
										 ID::Value iObjectID_);
												// システム表の列を表す
												// オブジェクトを生成する

	// Drop column definition
	void					drop(Trans::Transaction& cTrans_,
								 bool bRecovery_ = false, bool bNoUnset_ = false);

	static ID::Value		getObjectID(const Common::DataArrayData& cLogData_);
												// ログに記録されているIDを得る

	SYD_SCHEMA_FUNCTION
	static Column*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Column*			get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);
												// 列を表すクラスを得る

	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);
												// 陳腐化していないか調べる

	static void				doBeforePersist(const Pointer& pColumn_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pColumn_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pColumn_,
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

	SYD_SCHEMA_FUNCTION
	Position				getPosition() const;
												// 列が表の先頭から
												// 何番目に位置するか
	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// 列の種別を得る

	SYD_SCHEMA_FUNCTION
	Function::Value			getFunction(Trans::Transaction& cTrans_) const;
												// 関数列の種別を得る

	SYD_SCHEMA_FUNCTION
	const DataType&			getType() const;	// 列の型を得る

	// 対応するCommon::Dataの型を得る
	SYD_SCHEMA_FUNCTION
	void					getFieldType(Common::DataType::Type& eType_,
										 ModSize& iLength_, int& iScale_, bool& bFixed_,
										 Common::DataType::Type& eElementType_,
										 ModSize& iElementLength_, bool& bElementFixed_) const;

	SYD_SCHEMA_FUNCTION
	bool					isNullable() const;	// 列にNotNull制約がついていないかを得る
	void					setNullable(bool bNullable_);
												// 列にNotNull制約がついていないかを設定する

	SYD_SCHEMA_FUNCTION
	bool					isInHeap() const;	// 列のヒントにHEAPがあるか

	SYD_SCHEMA_FUNCTION
	bool					isNonTruncate() const;	// 列のヒントにNONTRUNCATEがあるか

	SYD_SCHEMA_FUNCTION
	bool					isLob() const;	// LOB型の列か

	// 列の値がUNIQUEであるかを得る
	SYD_SCHEMA_FUNCTION
	bool					isUnique(Trans::Transaction& cTrans_) const;
	// 列の値がCase Insensitiveであるかを得る
	SYD_SCHEMA_FUNCTION
	bool					isCaseInsensitive(Trans::Transaction& cTrans_) const;
	// 列の値が固定長であるかを得る
	SYD_SCHEMA_FUNCTION
	bool					isFixed() const;

	// Check whether new type is alterable
	bool					isAbleToAlter(const Statement::ColumnDefinition& cDefinition_,
										  ColumnPointer& pNewColumn_);

	SYD_SCHEMA_FUNCTION
	Table*					getTable(Trans::Transaction& cTrans_) const;
												// 列が存在する表を得る
	SYD_SCHEMA_FUNCTION
	const Default&			getDefault() const;	// デフォルト値を得る

	// 列の値を格納するフィールドを得る
	SYD_SCHEMA_FUNCTION
	Field*					getField(Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Field*					getField(Trans::Transaction& cTrans_,
									 File* pFile_) const;
	ID::Value				getFieldID() const;
	// 列の値を格納するフィールドを設定する
	const Field&			setField(Field& field);
	void					setFieldID(ID::Value id);

	SYD_SCHEMA_FUNCTION
	const ModVector<Key*>&	getKey(Trans::Transaction& cTrans_) const;
												// 列に対するキーを表す
												// クラスをすべて得る
#ifdef OBSOLETE // _keysへの挿入にはModVector::insertを使っているのでaddKeyは使用しない
	Key&					addKey(Key& key);	// 列に対するキーを表す
												// クラスを登録する
#endif
	void					resetKey();
												// 列に対するキーを表す
												// クラスが登録されて
												// いないことにする
	void					clearKey();
												// 列に対するキーを
												// 管理するベクターを破棄する

	// 索引フィールドを得る
	SYD_SCHEMA_FUNCTION
	bool					getIndexField(Trans::Transaction& cTrans_,
										  ModVector<Field*>& vecField_);

	////////////////////////////////////////
	// Sequenceを使って値を取得する列の管理
	///////////////////////////////////////

	// タプル ID を格納する列か
	SYD_SCHEMA_FUNCTION
	bool					isTupleID() const;

	// Identity Column を格納する列か
	bool					isIdentity() const;
	// Defaultがcurrent timestampである列か
	bool					isCurrentTimestamp() const;

	// 値を取得するSequenceオブジェクトを得る
	Sequence&				getSequence(Trans::Transaction& cTrans_);

	// シーケンスファイルを移動する
	void					moveSequence(Trans::Transaction& cTrans_,
										 const Os::Path& cPrevPath_,
										 const Os::Path& cPostPath_,
										 bool bUndo_, bool bRecovery_);

	// Sequenceオブジェクトを破棄する
	void					clearSequence();

	////////////////////////////////////////

	SYD_SCHEMA_FUNCTION
	const Hint*				getHint() const; 	// 列のヒントを得る
	void					setHint(Hint* pHint_);
												// 列のヒントをセットする


	// ColumnMetaDataをセットする
	void					setMetaData(Trans::Transaction& cTrans_,
										Common::ColumnMetaData& cMetaData_) const;

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// ソートの並び順を決めるための比較関数
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
//	void					addTimestamp();		// タイムスタンプを進める
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
	Column(const Table& table, Position position,
		   const Name& name, Category::Value category,
		   const Common::SQLData& type, const Default& def);
	Column(const Table& table, Position position,
		   const Statement::ColumnDefinition& statement);
	Column(const Table& table, Position position,
		   const Common::DataArrayData& cLogData_);
	Column(const Column& original, const Common::SQLData& type);
	void					destruct();			// デストラクター下位関数

	// 列についているIndexを調べないと分からない特性を調べる
	void					investigateIndex(Trans::Transaction& cTrans_) const;
	// 調査結果をクリアする
	void					clearFlag();

	Common::Data::Pointer	packIntegerMetaField(int iMemberID_) const;
	Common::Data::Pointer	packBinaryMetaField(int iMemberID_) const;

	bool					unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_);
	bool					unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_);

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「列」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Position				_position;			// 表の先頭から何番目の列か
	Category::Value			_category;			// 列の種別
	DataType				_type;				// 列の型
	Default					_default;			// デフォルト指定
	bool					m_bNullable;		// nullの可不可
	Hint*					m_pHint;			// 列のヒント

	// この列が属する表のオブジェクト

	mutable Table*			m_pTable;

	// この列の値を格納するフィールドを表すクラスと、
	// そのスキーマオブジェクト ID
	//
	//【注意】	水平分割を導入するときは、フィールドを直接参照せずに、
	//			水平分割クラスを介して、フィールドを参照することになる ?

	mutable Field*			_field;
	ID::Value				_fieldID;

	// 以下のメンバーは、「キー」表を検索して得られる

	mutable ModVector<Key*>*
							_keys;				// この列に対するキー

	// 以下のメンバーは実行中に必要に応じてセットされる
	mutable int				m_iFlag;			// 列の特性

	Sequence*				m_pSequence; 		// 値を取得するSequenceオブジェクト
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_COLUMN_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
