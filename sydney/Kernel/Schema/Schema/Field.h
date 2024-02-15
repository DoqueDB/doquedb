// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Field.h -- フィールドオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FIELD_H
#define	__SYDNEY_SCHEMA_FIELD_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Common/DataType.h"
#include "Common/Hasher.h"
#include "Common/StringData.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "ModTypes.h"
#include "ModVector.h"
#include "ModHashMap.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Column;
class Key;
class File;
class FileVerify;
class Field;
namespace SystemTable
{
	class Field;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::Field -- フィールドオブジェクトを表すクラス
//
//	NOTES
//		フィールドの親オブジェクトは、ファイルである

class Field
	: public	Object
{
public:
	friend class SystemTable::Field;
	friend class File;					// setNameのため
	friend class FileVerify;			// setNameのため

	//	TYPEDEF
	//	Schema::Field::Position --
	//		ファイルの先頭から何番目のフィールドかを表す値の型
	//
	//	NOTES

	typedef ModSize			Position;

	//	CLASS
	//	Schema::Field::Category -- フィールドの種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Schema::Field::Category::Value -- フィールドの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			ObjectID,							// オブジェクト ID フィールド
			Key,								// キーフィールド
			Data,								// データフィールド
			Function,							// 関数フィールド
			ValueNum							// 種別数
		};
	};

	//	CLASS
	//	Schema::Field::Function -- 関数フィールドの種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Function
	{
		//	ENUM
		//	Schema::Field::Function::Value --
		//		関数フィールドの種別の値を表す列挙型
		//
		//	NOTES
		//	★注意★
		//	この列挙型を変更したらField.cppのfunctionNamePrefix、functionType、functionElemenTypeを変更すること
		//	Plan::Scalar::FieldImpl.cppの_FunctionFieldSpecも変更すること
		//	この値は永続化されないので、途中に追加しても問題ない

		enum Value
		{
			Undefined =		0,					// 関数フィールドではない
			Score,								// スコア取得
			Section,							// セクション番号取得
			Word,								// 拡張検索語取得
			WordDf,								// 拡張検索語取得(DFのみ)
			WordScale,							// 拡張検索語取得(Scaleのみ)
			AverageLength,						// 平均文字数/平均単語数
			AverageCharLength,					// 平均文字数
			AverageWordCount,					// 平均単語数
			Tf,									// 文書ごとの語の出現数
			Count,								// 格納されているタプル数
			ClusterId,							// 結果のクラスター番号
			ClusterWord,						// 結果のクラスター用特徴語配列
			Kwic,								// Key Word In Context
			Existence,							// Existence of each word
			MinKey,								// 第一キーの最小値
			MaxKey,								// 第一キーの最大値
			NeighborId,							// Neighbor.id
			NeighborDistance,					// Neighbor.distance
			ValueNum,							// 種別数
			/********************/
			FullTextMin = Score,				// 全文索引用Functionの最小値
			FullTextMax = MinKey - 1,			// 全文索引用Functionの最大値
			VectorMin = Count,					// Vector用Functionの最小値
			VectorMax = Count,					// Vector用Functionの最大値
			BtreeMin = MinKey,					// Btree用Functionの最小値
			BtreeMax = MaxKey,					// Btree用Functionの最大値
			KdTreeMin = NeighborId,				// KdTree用Functionの最小値
			KdTreeMax = NeighborDistance,		// KdTree用Functionの最大値
		};
	};

	//	CLASS
	//	Schema::Field::Permission --
	//		フィールドに対して許可されている操作を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Permission
	{
		//	ENUM
		//	Schema::Field::Permission::Value --
		//		フィールドに対して許可されている操作の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			None =			0x0,				// なし
			Getable =		0x1,				// 読み込み可
			Putable =		0x2,				// 書き込み可
			All =			Getable|Putable		// 読み書き可
		};
	};

	struct Log
	{
		enum Value
		{
			ID = 0,							// ID
			Num
		};
	};

	//	TYPEDEF
	//	Schema::Field::Type -- フィールドに格納するデータの型
	//
	//	NOTES

	typedef	Common::DataType::Type		Type;

	//	TYPEDEF
	//	Schema::Field::Length -- フィールドに格納するデータの長さ
	//
	//	NOTES

	typedef	ModSize			Length;

	//	TYPEDEF
	//	Schema::Field::Scale -- Scale value for Decimal type field
	//
	//	NOTES

	typedef	int				Scale;

	//	TYPEDEF public
	//	Schema::Field::Pointer -- Fieldを保持するObjectPointer
	//
	//	NOTES

	typedef FieldPointer Pointer;

	// コンストラクター
	Field();
	// デストラクター
	~Field();

	static Field*			getNewInstance(const Common::DataArrayData& cData_);
												// DataArrayDataを元にインスタンスを生成する

	void					clear();			// メンバーをすべて初期化する

	static Pointer			create(File& file, Position position,
								   Category::Value category,
								   Permission::Value permission,
								   Type type, Length length,
								   Trans::Transaction& cTrans_,
								   ObjectID::Value iID_ = ObjectID::Invalid);
	static Pointer			create(File& file, Position position,
								   Category::Value category,
								   Permission::Value permission,
								   Field& source,
								   Trans::Transaction& cTrans_,
								   ObjectID::Value iID_ = ObjectID::Invalid);
	static Pointer			create(File& file, Position position,
								   Category::Value category,
								   Permission::Value permission,
								   Column& column,
								   Trans::Transaction& cTrans_,
								   ObjectID::Value iID_ = ObjectID::Invalid);
	static Pointer			create(File& file, Position position,
								   Category::Value category,
								   Permission::Value permission,
								   Field& source, Column& column,
								   Trans::Transaction& cTrans_,
								   ObjectID::Value iID_ = ObjectID::Invalid);
	static Pointer			create(File& file, Position position,
								   Key& cKey_,
								   Permission::Value permission,
								   Trans::Transaction& cTrans_,
								   ObjectID::Value iID_ = ObjectID::Invalid);
	static Pointer			create(File& file, Position position,
								   Function::Value function, Column& column,
								   Trans::Transaction& cTrans_);
	static Pointer			create(File& file, Position position,
								   Field& field,
								   Trans::Transaction& cTrans_,
								   ObjectID::Value iID_ = ObjectID::Invalid);
												// スキーマ定義を生成する

	static Pointer			createSystem(Trans::Transaction& cTrans_,
										 File& file, Position position,
										 Category::Value category,
										 Permission::Value permission,
										 Column& column,
										 Type type,
										 ID::Value iObjectID_);
	static Pointer			createSystem(Trans::Transaction& cTrans_,
										 File& file, Position position,
										 Category::Value category,
										 Permission::Value permission,
										 Type type,
										 ID::Value iObjectID_);
	static Pointer			createSystem(Trans::Transaction& cTrans_,
										 File& file, Position position,
										 Category::Value category,
										 Permission::Value permission,
										 Field& source,
										 ID::Value iObjectID_);

	SYD_SCHEMA_FUNCTION
	static Field*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Field*			get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);
												// フィールドを表すクラスを得る

	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);
												// 陳腐化していないか調べる

	static void				doBeforePersist(const Pointer& pField_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pField_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pField_,
										File& cFile_,
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
												// フィールドがファイルの
												// 先頭から何番目に位置するか
	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// フィールドの種別を得る

	SYD_SCHEMA_FUNCTION
	Function::Value			getFunction() const;
												// 関数フィールドの種別を得る

	SYD_SCHEMA_FUNCTION
	Type					getType() const;	// フィールドに格納する
												// データの型
	SYD_SCHEMA_FUNCTION
	Type					getElementType() const;
												// 配列フィールドに格納する
												// データの要素の型
	void					setType(const Column& column);
												// ある列の値を格納する
												// フィールドの型を設定する
	void					setType(const Field& source);
												// あるフィールドを派生元とする
												// フィールドの型を設定する
	void					setType(Function::Value function);
												// ある関数の返り値を表す
												// フィールドの型を設定する

	SYD_SCHEMA_FUNCTION
	Length					getLength() const;	// フィールドに格納する
												// データの最大長(B 単位)
												// 配列の場合要素数の最大値
	SYD_SCHEMA_FUNCTION
	Length					getElementLength() const;
												// 配列フィールドに格納する
												// 要素の最大長(B 単位)
	void					setLength(const Column& column);
												// ある列の値を格納する
												// フィールドの最大長を設定する
	void					setLength(const Field& source);
												// あるフィールドを派生元とする
												// フィールドの最大長を設定する
	// get Scale value of Decimal type field
	Scale					getScale() const;

	// フィールドの文字列データの符号化形式を得る
	Common::StringData::EncodingForm::Value
	getEncodingForm(Trans::Transaction& trans);
	// 配列フィールドの要素の文字列データの符号化形式を得る
	Common::StringData::EncodingForm::Value
	getElementEncodingForm(Trans::Transaction& trans);

	SYD_SCHEMA_FUNCTION
	const Common::Data*		getDefaultData() const;
												// フィールドに格納すべき
												// デフォルトのデータを得る
	void					setDefaultData(const Column& column);
												// ある列の値を格納する
												// フィールドのデフォルトの
												// データを得る
	void					setDefaultData(const Field& field);
												// あるフィールドを派生元とする
												// フィールドのデフォルトの
												// データを得る

	SYD_SCHEMA_FUNCTION
	File*					getFile(Trans::Transaction& cTrans_) const;
												// フィールドが存在する
												// ファイルを得る

	SYD_SCHEMA_FUNCTION
	Field*					getSource(Trans::Transaction& cTrans_) const;
												// フィールドの派生元を得る
	// フィールドの派生元のオブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getSourceID() const;

	// フィールドの派生元のオブジェクトIDを設定する
	void					setSourceID(ID::Value id_);

	// フィールドの派生先として登録されている
	// フィールドを表すクラスに関する操作

	SYD_SCHEMA_FUNCTION
	const ModVector<Field*>&
							getDestination(Trans::Transaction& cTrans_) const;
												// すべての登録の取得
	Field&					addDestination(Trans::Transaction& cTrans_, Field& field);
												// 登録の追加
	void					eraseDestination(const Field& field);
												// 登録の抹消
	void					resetDestination();	// 全登録の抹消
	void					clearDestination();	// 全登録を抹消し、
												// 管理用のベクターを破棄する

	// フィールドを使用する列のIDを得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getColumnID() const;
	// フィールドを使用する列のIDを設定する
	void					setColumnID(ID::Value id_);
	SYD_SCHEMA_FUNCTION
	Column*					getColumn(Trans::Transaction& cTrans_) const;
												// フィールドを使用する列を得る
	// clear cache for corresponding column
	void					clearColumn();

	SYD_SCHEMA_FUNCTION
	Column*					getRelatedColumn(Trans::Transaction& cTrans_) const;
												// フィールドに関係する列を得る
	// フィールドを使用するキーのIDを得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getKeyID() const;
	// フィールドを使用するキーのIDを設定する
	void					setKeyID(ID::Value id_);

	SYD_SCHEMA_FUNCTION
	Key*					getKey(Trans::Transaction& cTrans_) const;
												// フィールドを
												// 使用するキーを得る

	SYD_SCHEMA_FUNCTION
	bool					isObjectID() const;	// オブジェクト ID フィールドか
	SYD_SCHEMA_FUNCTION
	bool					isKey() const;		// キーフィールドか
	SYD_SCHEMA_FUNCTION
	bool					isFirstKey(Trans::Transaction& cTrans_) const;
												// 第一キーフィールドか
	SYD_SCHEMA_FUNCTION
	bool					isStringKey() const;
												// 文字列型キーフィールドか
	SYD_SCHEMA_FUNCTION
	bool					isData() const;		// データフィールドか

	SYD_SCHEMA_FUNCTION
	bool					isFunction() const;	// 関数フィールドか

	SYD_SCHEMA_FUNCTION
	bool					isGetable() const;	// 値を取得可能なフィールドか
	SYD_SCHEMA_FUNCTION
	bool					isPutable() const;	// 値を設定可能なフィールドか

	SYD_SCHEMA_FUNCTION
	bool					isTupleID(Trans::Transaction& cTrans_) const;
												// タプル ID を
												// 格納するフィールドか
	SYD_SCHEMA_FUNCTION
	bool					isNullable(Trans::Transaction& cTrans_) const;
												// NULL値を格納する
												// 可能性のあるフィールドか
	SYD_SCHEMA_FUNCTION
	bool					isFixed() const;	// 固定長のフィールドか

	SYD_SCHEMA_FUNCTION
	bool					hasFunction(Function::Value eFunction_,
										Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	bool					hasFunction2(LogicalFile::TreeNodeInterface::Type eFunction_,
										 Trans::Transaction& cTrans_) const;
												// 関数を処理できるか

	// 関係するオブジェクトを陳腐化する
	void					touchRelated(Status::Value eStatus_, Trans::Transaction& cTrans_);

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// Vector上でのソートに使う関数
	virtual bool			isLessThan(const Object& cOther_) const;

	// Functionに対応する型を得る関数
	static bool				getFunctionType(Function::Value eFunction_, Type& cType_, Type& cElementType_);

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

	// スキーマで判定してファイルドライバーに渡すField Hintのための定数
	static const ModUnicodeString& getHintNotNull();
	static const ModUnicodeString& getHintFixed();

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
	Field(const File& file, Position position,
		  Category::Value category, Permission::Value permission,
		  Type type, Length length);
	Field(const File& file, Position position,
		  Category::Value category, Permission::Value permission,
		  Field& source);
	Field(const File& file, Position position,
		  Category::Value category, Permission::Value permission,
		  Column& column);
	Field(const File& file, Position position,
		  Category::Value category, Permission::Value permission,
		  Field& source, Column& column);
	Field(const File& file, Position position,
		  Key& cKey_, Permission::Value permission,
		  Field& source, Column& column);
	Field(const File& file, Position position,
		  Function::Value function, Column& column);
	Field(const File& file, Position position,
		  Field& field);
	void					destruct();			// デストラクター下位関数

	const Name&				setName(Trans::Transaction& cTrans_);
	void					doSetName(const Name& cParentName_);
												// フィールドの名前を設定する

	Common::Data::Pointer	packIntegerMetaField(int iMemberID_) const;
	bool					unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_);

	void					setColumnAttribute(const Column& cColumn_);

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「フィールド」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Position				_position;			// ファイルの先頭から
												// 何番目のフィールドか
	Category::Value			_category;			// フィールドの種別
	Function::Value			_function;			// 関数フィールドの種別
	Permission::Value		_permission;		// フィールドへ許可された操作

	// このフィールドが属するファイル(オブジェクトIDは_parentである)
	mutable File*			m_pFile;
	
	// このフィールドの派生下を表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Field*			_source;
	ID::Value				_sourceID;

	mutable ModVector<Field*>* _destinations;	// このフィールドの派生先

	// このフィールドを使用する列を表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Column*			_column;
	ID::Value				_columnID;

	// このフィールドを使用するキーを表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Key*			_key;
	ID::Value				_keyID;
	mutable int				m_iIsFirstKey;

	// 以下のメンバーは、「列」表を検索して得られる

	Type					_type;				// フィールドに格納する値の型
	Type					_elementType;		// 配列フィールドの要素の型
	Length					_length;			// フィールドに格納する値の
												// 最大長(単位 B)
												// 配列フィールドの場合は最大要素数
	Length					_elementLength;		// 配列フィールドの要素の
												// 最大長(単位 B)
	Scale					m_iScale;			// Scale for decimal type field
	bool					m_bIsFixed;			// 固定長か
	Common::Data::Pointer	_default;			// デフォルト値
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_FIELD_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
