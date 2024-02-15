// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Object.h -- スキーマオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2007, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_OBJECT_H
#define	__SYDNEY_SCHEMA_OBJECT_H

#include "Schema/Module.h"
#include "Schema/Meta.h"
#include "Schema/ObjectID.h"
#include "Schema/ObjectName.h"
#include "Schema/Externalizable.h"
#include "Schema/FileID.h"
#include "Schema/AreaCategory.h"

#include "Common/SafeExecutableObject.h"

#include "LogicalFile/ObjectID.h"			// typedefのためclass宣言できない

#include "Os/RWLock.h"

#include "ModTypes.h"
#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModOs.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}

_SYDNEY_SCHEMA_BEGIN
class Area;
class AreaContent;
class Cascade;
class Column;
class Constraint;
class Database;
class Field;
class File;
class Function;
class Index;
class Key;
class Partition;
class Privilege;
class Table;
namespace SystemTable
{
	class SystemFile;
	class Area;
	class AreaContent;
	class Cascade;
	class Column;
	class Constraint;
	class Database;
	class Field;
	class File;
	class Function;
	class Index;
	class Key;
	class Partition;
	class Privilege;
	class Table;
}
namespace Utility
{
	class BinaryData;
}

//	TYPEDEF
//	Schema::XXXPointer -- スキーマオブジェクトを保持するObjectPointer
//
//	NOTES
//		相互に使うので宣言だけここで行う

#define _DeclarePointer(__X__) typedef Common::ObjectPointer<__X__> __X__##Pointer
_DeclarePointer(Area);
_DeclarePointer(AreaContent);
_DeclarePointer(Cascade);
_DeclarePointer(Column);
_DeclarePointer(Constraint);
_DeclarePointer(Database);
_DeclarePointer(Field);
_DeclarePointer(File);
_DeclarePointer(Function);
_DeclarePointer(Index);
_DeclarePointer(Key);
_DeclarePointer(Partition);
_DeclarePointer(Privilege);
_DeclarePointer(Table);
#undef _DeclarePointer

//	CLASS
//	Schema::Object -- スキーマオブジェクトを表すクラス
//
//	NOTES
//		スキーマオブジェクトは、スキーマオブジェクト ID で一意に識別される
//		スキーマオブジェクトは、エグゼキューターで処理可能である

class Object
	: public	Common::SafeExecutableObject
	, public	Externalizable // freeze, meltで使用する
{
public:
	friend class SystemTable::SystemFile;
	friend class SystemTable::Area;
	friend class SystemTable::AreaContent;
	friend class SystemTable::Cascade;
	friend class SystemTable::Column;
	friend class SystemTable::Constraint;
	friend class SystemTable::Database;
	friend class SystemTable::Field;
	friend class SystemTable::File;
	friend class SystemTable::Function;
	friend class SystemTable::Index;
	friend class SystemTable::Key;
	friend class SystemTable::Partition;
	friend class SystemTable::Privilege;
	friend class SystemTable::Table;

	//	TYPEDEF
	//	Schema::Object::ID -- スキーマオブジェクト ID を表す型
	//
	//	NOTES

	typedef	ObjectID		ID;

	//	TYPEDEF
	//	Schema::Object::Name -- スキーマオブジェクトの名前を表す型
	//
	//	NOTES

	typedef	ObjectName		Name;

	//	TYPEDEF
	//	Schema::Object::Timestamp --
	//		スキーマオブジェクトの陳腐化を管理するためのタイムスタンプを表す型
	//
	//	NOTES

	typedef unsigned int Timestamp;

	//	TYPEDEF
	//	Schema::Object::Category -- スキーマオブジェクトの種別を表す名前空間
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Schema::Object::Category::Value --
		//		スキーマオブジェクトの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			Database,							// データベース
			Table,								// 表
			Column,								// 列
			Constraint,							// 制約
			Index,								// 索引
			Key,								// キー
			File,								// ファイル
			Field,								// フィールド
			Area,								// エリア
			AreaContent,						// エリア格納情報
			Privilege,							// Privilege
			Cascade,							// 子サーバー
			Partition,							// ルール
			Function,							// 関数
			ValueNum							// 種別数
		};
	};

	//	STRUCT
	//	Schema::Object::Scope --
	//		スキーマオブジェクトのスコープの種別を表す名前空間
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Scope
	{
		//	ENUM
		//	Schema::Object::Scope::Value --
		//		スキーマオブジェクトのスコープの種類の値を表す列挙型
		//
		//	NOTES
		//	SessionTemporaryはSQL規格にはない仕様であり、
		//	定義およびデータがセッションローカルにのみ有効であることを示す

		enum Value
		{
			Unknown =		0,					// 不明
			Permanent,							// 永続的
			GlobalTemporary,					// 大域かつ一時的
			LocalTemporary,						// 局所かつ一時的
			SessionTemporary,					// セッションローカルかつ一時的
			Meta,								// メタデータベースを表す仮想的なもの
			ValueNum							// 種類数
		};
	};

	//	STRUCT
	//	Schema::Object::Status --
	//		スキーマオブジェクトの永続化に関する状態を表す名前空間
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Status
	{
		//	ENUM
		//	Schema::Object::Status::Value --
		//		スキーマオブジェクトの永続化に関する状態を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown = 0,
			Created,							// 作成され、永続化されてない
			Mounted,							// Mount され、永続化されていない
			Changed,							// 変更され、永続化されてない
			Deleted,							// 削除され、永続化されてない
			DeletedInRecovery,					// リカバリー処理の中で削除され、永続化されてない
			Persistent,							// 作成、変更が永続化された
			ReallyDeleted,						// 削除が永続化された
			CreateCanceled,						// 作成がキャンセルされた
			DeleteCanceled,						// 削除が永続化された後にキャンセルされた
			ValueNum
		};
	};

	//	CLASS public
	//	Schema::Object::Pointer -- Objectを保持するObjectPointer
	//
	//	NOTES
	//		Schema::ObjectのObjectPointerだが、
	//		各スキーマオブジェクトのObjectPointerからのコンストラクターを
	//		定義するためにサブクラスとした

	class Pointer : public Common::ObjectPointer<Object>
	{
	public:
		Pointer() : Common::ObjectPointer<Object>() {}
		Pointer(const Pointer& p_) : Common::ObjectPointer<Object>(p_) {}
		Pointer(Object* p_) : Common::ObjectPointer<Object>(p_) {}
		Pointer(const Object* p_) : Common::ObjectPointer<Object>(p_) {}

		template <class __X__>
		Pointer(const Common::ObjectPointer<__X__>& p_) : Common::ObjectPointer<Object>()
		{
			Common::ConvertObjectPointer(*this, p_);
		}
	};

	Object(Category::Value category = Category::Unknown,
		   Scope::Value scope = Scope::Unknown,
		   Status::Value eStatus_ = Status::Unknown,
		   ID::Value id = ID::Invalid,
		   ID::Value parent = ID::Invalid,
		   ID::Value database = ID::Invalid,
		   const Name& name = Name());
												// コンストラクター
	SYD_SCHEMA_FUNCTION
	virtual ~Object();							// デストラクター

	void					clear();			// メンバーをすべて初期化する

#ifdef OBSOLETE // Objectの同一性はIDを使って行うのでObjectに対する比較演算子は使用しない
	SYD_SCHEMA_FUNCTION
	bool					operator ==(const Object& r) const;
												// == 演算子
	SYD_SCHEMA_FUNCTION
	bool					operator !=(const Object& r) const;
												// != 演算子

	SYD_SCHEMA_FUNCTION
	ModSize					hashCode() const;	// ハッシュ値を計算する

	SYD_SCHEMA_FUNCTION
	virtual bool			isInvalid() const;	// 不正なオブジェクトか調べる
												// ★注意★
												// 陳腐化を調べるisValidとは
												// 異なるので注意
#endif

	SYD_SCHEMA_FUNCTION
	Timestamp				getTimestamp() const; // タイムスタンプを得る

	// オブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getID() const;
	// オブジェクト ID を設定する
	void					setID(ID::Value id);
	SYD_SCHEMA_FUNCTION
	// 親オブジェクトのオブジェクト ID を得る
	ID::Value				getParentID() const;
	// 親オブジェクトのオブジェクト ID を設定する
	void					setParentID(ID::Value parent);

	SYD_SCHEMA_FUNCTION
	const Name&				getName() const;	// オブジェクトの名前を得る
	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// オブジェクトの種別を得る
	SYD_SCHEMA_FUNCTION
	Scope::Value			getScope() const;	// オブジェクトのスコープを得る
	SYD_SCHEMA_FUNCTION
	Status::Value			getStatus() const;	// オブジェクトの
												// 永続化状態を得る
	Status::Value			setStatus(Status::Value eStatus_) const;
												// オブジェクトの
												// 永続化状態を設定する

	void					create(Trans::Transaction& cTrans_,
								   ID::Value iID = ID::Invalid);
												// IDを新規にふり、
												// 状態を作成にする
	SYD_SCHEMA_FUNCTION
	void					touch();			// 関係するオブジェクトの変更
												// により自身を変更する
	SYD_SCHEMA_FUNCTION
	void					untouch();			// 状態を変更なしにする
	SYD_SCHEMA_FUNCTION
	void					drop(bool bRecovery_ = false, bool bNoUnset_ = false);
												// 状態を削除または作成の取り消しにする
	SYD_SCHEMA_FUNCTION
	void					undoDrop();			// 削除を取り消す

	SYD_SCHEMA_FUNCTION
	Database*				getDatabase(Trans::Transaction& cTrans_) const;
												// オブジェクトが属するデータベースオブジェクトを得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getDatabaseID() const;
												// オブジェクトが属するデータベースID得る

	ID::Value				setDatabaseID(ID::Value iID_);
												// オブジェクトが属するデータベースIDを設定する

	void					setDatabase(Database* pDatabase_);
												// オブジェクトが属するデータベースオブジェクトを設定する

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	//virtual int			getClassID() const;	// このクラスのクラス ID を得る

	// Vector上でのソートに使う関数
	virtual bool			isLessThan(const Object& cOther_) const;

	// スキーマオブジェクトをファイルに格納するときの
	// フィールド情報をFileIDに設定する
	void					setFieldInfo(LogicalFile::FileID& cFileID_);

	// 以下はデータベースファイル化に伴い追加されたメソッド

	const LogicalFile::ObjectID*
							getFileObjectID() const;
												// オブジェクトを格納する
												// ファイルでのオブジェクトIDを
												// 得る
	void					setFileObjectID(const LogicalFile::ObjectID& cID_);
												// オブジェクトを格納する
												// ファイルでのオブジェクトIDを
												// 設定する
protected:
	const Name&				setName(const ModUnicodeString& name);
												// オブジェクトの名前を設定する
	const Name&				setName(const ModCharString& name);
												// オブジェクトの名前を設定する
												// (コード変換あり)
	Category::Value			setCategory(Category::Value category);
												// オブジェクトの種別を設定する
	Scope::Value			setScope(Scope::Value scope);
												// オブジェクトの
												// スコープを設定する
	Timestamp				setTimestamp(Timestamp iTimestamp_);
												// タイムスタンプを進める
	Timestamp				addTimestamp();		// タイムスタンプを進める

	// スキーマオブジェクトの内容をDataArrayDataにする
	virtual Common::DataArrayData* pack() const;
	// スキーマオブジェクトにDataArrayDataを反映する
	virtual void			unpack(const Common::DataArrayData& cData);

	// pack、unpack、setFieldInfoに使う関数
	virtual int				getMetaFieldNumber() const = 0;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const = 0;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

	// pack、unpackで使用する関数
	static
	Common::Data::Pointer	pack(int iValue_);
	static
	Common::Data::Pointer	pack(unsigned int iValue_);
	static
	Common::Data::Pointer	pack(const ModVector<unsigned int>& vecValue_);
	static
	Common::Data::Pointer	pack(const ModUnicodeString& cValue_);
	static
	Common::Data::Pointer	pack(const ModVector<ModUnicodeString>& vecValue_);
	static
	Common::Data::Pointer	pack(Category::Value eCategory_);
	static
	Common::Data::Pointer	pack(AreaCategory::Value eCategory_);
	static
	bool					unpack(const Common::Data* pElement_, int& iValue_);
	static
	bool					unpack(const Common::Data* pElement_, unsigned int& iValue_);
	static
	bool					unpack(const Common::Data* pElement_, ModVector<unsigned int>& vecValue_);
	static
	bool					unpack(const Common::Data* pElement_, ModUnicodeString& cValue_);
	static
	bool					unpack(const Common::Data* pElement_, ModVector<ModUnicodeString>& vecValue_);
	static
	bool					unpack(const Common::Data* pElement_, Category::Value& eCategory_);
	static
	bool					unpack(const Common::Data* pElement_, AreaCategory::Value& eCategory_);

	Utility::BinaryData&	getArchiver() const;

	Os::RWLock&				getRWLock() const;
												// 読み書きロックを得る
private:
	Object(const Object& );
	Object& operator =(const Object& );
	void					destruct();			// デストラクターの下位関数

	LogicalFile::ObjectID*	m_pFileObjectID;	// レコードファイルの
												// オブジェクトID
	ID::Value				_id;				// オブジェクト ID
	ID::Value				_parent;			// 親オブジェクトの
												// オブジェクト ID
	Name					_name;				// オブジェクトの名前
	Category::Value			_category;			// オブジェクトの種別
	Scope::Value			_scope;				// オブジェクトのスコープ
	ID::Value				m_iDatabaseID;		// オブジェクトが属するデータベースのID
	mutable Database*		m_pDatabase;		// オブジェクトが属するデータベースオブジェクト
	mutable Status::Value	m_eStatus;			// オブジェクトの永続化状態
	Timestamp				m_iTimestamp;		// オブジェクトのタイムスタンプ
	mutable Utility::BinaryData* m_pArchiver;
	Os::RWLock				m_cRWLock;			// メンバー操作の排他制御に使う
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_OBJECT_H

//
// Copyright (c) 2001, 2002, 2004, 2005, 2007, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
