// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Cascade.h -- 子サーバー関連のクラス定義、子サーバー宣言
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_CASCADE_H
#define	__SYDNEY_SCHEMA_CASCADE_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Common/Hasher.h"
#include "Common/UnicodeString.h"

#include "ModCharString.h"
#include "ModHashMap.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class CascadeDefinition;
	class AlterCascadeStatement;
	class DropCascadeStatement;
	class Object;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class LogData;
namespace SystemTable
{
	class Cascade;
}

//	CLASS
//	Schema::Cascade -- 子サーバーオブジェクトを表すクラス
//
//	NOTES
//		子サーバーの親オブジェクトは存在しない

class Cascade
	: public Schema::Object
{
public:
	friend class SystemTable::Cascade;

	struct Version
	{
		enum Value
		{
			Version0 = 0,
			Version1,
			CurrentVersion = Version0,
			ValueNum
		};
	};

	//	TYPEDEF public
	//	Schema::Cascade::Pointer -- Cascadeを保持するObjectPointer
	//
	//	NOTES
	typedef CascadePointer Pointer;

	struct Log {


		//	STRUCT
		//		Schema::Log::Value -- ログの共通要素位置
		//
		//	NOTES
		enum Value {
			Name = 0,						// 子サーバー名
			ID,								// ID
			ValueNum,
		};

		//	STRUCT
		//		Schema::Log::Create -- Create ログの要素位置
		//
		//	NOTES
		struct Create {
			enum Value {
				Version = ValueNum,
				Target,
				Num
			};
		};

		//	STRUCT
		//		Schema::Log::Alter -- Alter ログの要素位置
		//
		//	NOTES
		struct Alter {
			enum Value {
				PrevTarget = ValueNum,
				PostTarget,
				Num
			};
		};

		//	STRUCT
		//		Schema::Log::Drop -- Drop ログの要素位置
		//
		//	NOTES
		struct Drop {
			enum Value {
				Num = ValueNum
			};
		};
	};


	// STRUCT
	//	Cascade::Target -- 子サーバーを指定する情報
	//
	// NOTES

	struct Target
	{
		ModUnicodeString m_cstrHost;
		ModUnicodeString m_cstrPort;
		ModUnicodeString m_cstrDatabase;

		Target() : m_cstrHost(), m_cstrPort(), m_cstrDatabase() {}

		void addToLog(LogData& cLogData_);
		void setFromLog(const LogData& cLogData_,
						int iIndex_);
	};

	////////////////////
	// Cascadeのメソッド //
	////////////////////

	// コンストラクター
	Cascade();
	// デストラクター
	SYD_SCHEMA_FUNCTION
	~Cascade();

	// DataArrayDataを元にインスタンスを生成する
	static Cascade*			getNewInstance(const Common::DataArrayData& cData_);

	void					clear();			// メンバーをすべて初期化する

	static Cascade*			create(Database& cDatabase_,
								   const Statement::CascadeDefinition& cStatement_,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	static Cascade*			create(Trans::Transaction& cTrans_,
								   const Database& cDatabase_,
								   const LogData& cLogData_);
												// 子サーバーを生成する

	static Name				getName(const Statement::AlterCascadeStatement& cStatement_);
	static Name				getName(const Statement::DropCascadeStatement& cStatement_);
												// SQL文から対象の子サーバー名を得る

	static void				drop(Cascade& cCascade_,
								 LogData& cLogData_,
								 Trans::Transaction& cTrans_);
	static Pointer			drop(Trans::Transaction& cTrans_,
								 Database& cDatabase_,
								 const LogData& cLogData_);
												// 子サーバーの破棄処理をする
	void					drop(Trans::Transaction& cTrans_,
								 bool bRecovery_ = false);

	static bool				alter(Trans::Transaction& cTrans_,
								  Cascade& cCascade_,
								  const Statement::AlterCascadeStatement& cStatement_,
  								  Target& cPrevTarget_,
  								  Target& cPostTarget_,
								  LogData& cLogData_);
	static bool				alter(Trans::Transaction& cTrans_,
								  const LogData& cLogData_,
  								  Target& cPrevTarget_,
  								  Target& cPostTarget_);
												// 子サーバー変更の準備をする

	SYD_SCHEMA_FUNCTION
	static Cascade*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
												// 子サーバーを表すクラスを得る

	const Target&			getTarget() const;
												// 対象を指定する情報を得る
	void					setTarget(const Target& cTarget_);
												// 対象を指定する情報を設定する

	static void				doBeforePersist(const Pointer& pCascade_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pCascade_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pCascade_,
										Database& cDatabase_,
										Trans::Transaction& cTrans_);

//	Object::
//	Timestamp				getTimestamp() const; // タイムスタンプを得る
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	void					reset(Database& cDatabase_);
	void					reset();
												// 下位オブジェクトを抹消する

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// 再構成用のメソッド

	// 論理ログ出力用のメソッド
	void					makeLogData(LogData& cLogData_) const;
												// ログデータを作る
	// ログデータからさまざまな情報を取得する
	static ID::Value		getObjectID(const LogData& log_);
												// ログデータより CascadeID を取得する
	static Name				getName(const LogData& cLogData_);
												// ログデータから対象の子サーバー名を得る

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);
protected:
private:
	// コンストラクター
	Cascade(const Database& cDatabase_, const Statement::CascadeDefinition& cStatement_);
	Cascade(const Database& cDatabase_, const LogData& cLogData_);

	void					setTarget(const Statement::CascadeDefinition& cStatement_);

	Common::Data::Pointer	packIntegerMetaField(int iMemberID_) const;
	Common::Data::Pointer	packStringArrayMetaField(int iMemberID_) const;

	bool					unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_);
	bool					unpackStringArrayMetaField(const Common::Data* pData_, int iMemberID_);

//	Object::
//	void					addTimestamp();		// タイムスタンプを進める

	void					destruct();			// デストラクター下位子サーバー

	// 以下のメンバーは、「子サーバー」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	mutable Version::Value m_eVersion;
	mutable Target m_cTarget;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_CASCADE_H

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
