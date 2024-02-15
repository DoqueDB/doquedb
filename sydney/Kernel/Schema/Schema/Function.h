// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function.h -- 関数関連のクラス定義、関数宣言
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FUNCTION_H
#define	__SYDNEY_SCHEMA_FUNCTION_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Common/Hasher.h"
#include "Common/SQLData.h"
#include "Common/UnicodeString.h"

#include "Execution/Interface/Declaration.h"

#include "ModCharString.h"
#include "ModHashMap.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class FunctionDefinition;
	class DropFunctionStatement;
	class Object;
	class ParameterDeclarationList;
	class ReturnsClause;
	class RoutineBody;
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
	class Function;
}

//	CLASS
//	Schema::Function -- 関数オブジェクトを表すクラス
//
//	NOTES
//		関数の親オブジェクトは存在しない

class Function
	: public Schema::Object
{
public:
	friend class SystemTable::Function;

	struct Version
	{
		enum Value
		{
			Version0 = 0,
			CurrentVersion = Version0,
			ValueNum
		};
	};

	//	TYPEDEF public
	//	Schema::Function::Pointer -- Functionを保持するObjectPointer
	//
	//	NOTES
	typedef FunctionPointer Pointer;

	struct Log {


		//	STRUCT
		//		Schema::Log::Value -- ログの共通要素位置
		//
		//	NOTES
		enum Value {
			Name = 0,						// 関数名
			ID,								// ID
			ValueNum
		};

		//	STRUCT
		//		Schema::Log::Create -- Create ログの要素位置
		//
		//	NOTES
		struct Create {
			enum Value {
				Version = ValueNum,
				ReturnType,
				Routine,
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


	////////////////////
	// Functionのメソッド //
	////////////////////

	// コンストラクター
	Function();
	// デストラクター
	SYD_SCHEMA_FUNCTION
	~Function();

	// DataArrayDataを元にインスタンスを生成する
	static Function*			getNewInstance(const Common::DataArrayData& cData_);

	void					clear();			// メンバーをすべて初期化する

	static Function*		create(Database& cDatabase_,
								   const Statement::FunctionDefinition& cStatement_,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	static Function*		create(Trans::Transaction& cTrans_,
								   const Database& cDatabase_,
								   const LogData& cLogData_);
												// 関数を生成する

	static Name				getName(const Statement::DropFunctionStatement& cStatement_);
												// SQL文から対象の関数名を得る

	static void				drop(Function& cFunction_,
								 LogData& cLogData_,
								 Trans::Transaction& cTrans_);
	static Pointer			drop(Trans::Transaction& cTrans_,
								 Database& cDatabase_,
								 const LogData& cLogData_);
												// 関数の破棄処理をする

	void					drop(Trans::Transaction& cTrans_,
								 bool bRecovery_ = false);

	SYD_SCHEMA_FUNCTION
	static Function*		get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
												// 関数を表すクラスを得る

	const Common::SQLData&	getReturnType() const;
												// 返り値の型を得る
	const Statement::Object* getRoutine() const;
												// 関数本体を得る

	static void				doBeforePersist(const Pointer& pFunction_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pFunction_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pFunction_,
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

	// compile routine
	Execution::Interface::IProgram*
							compile(Trans::Transaction& cTrans_);
	// get number of parameters
	int						getParameterCount();

	// 論理ログ出力用のメソッド
	void					makeLogData(LogData& cLogData_) const;
												// ログデータを作る
	// ログデータからさまざまな情報を取得する
	static ID::Value		getObjectID(const LogData& log_);
												// ログデータより FunctionID を取得する
	static Name				getName(const LogData& cLogData_);
												// ログデータから対象の関数名を得る

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
	Function(const Database& cDatabase_, const Statement::FunctionDefinition& cStatement_);
	Function(const Database& cDatabase_, const LogData& cLogData_);

	void setReturnType(const Statement::FunctionDefinition& cStatement_);
	void setRoutine(const Statement::FunctionDefinition& cStatement_);

	Common::Data::Pointer packIntegerMetaField(int iMemberID_) const;
	Common::Data::Pointer packBinaryMetaField(int iMemberID_) const;

	bool				  unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_);
	bool				  unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_);

//	Object::
//	void					addTimestamp();		// タイムスタンプを進める

	void					destruct();			// デストラクター下位関数

	// 以下のメンバーは、「関数」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	mutable Version::Value m_eVersion;
	mutable Common::SQLData m_cReturnType;
	mutable Statement::Object* m_pRoutine;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_FUNCTION_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
