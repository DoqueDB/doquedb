// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function.cpp -- 関数関連の関数定義
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Function.h"
#include "Schema/Database.h"
#include "Schema/FakeError.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/ObjectID.h"
#include "Schema/Parameter.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Function.h"
#include "Schema/Utility.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "Exception/StoredFunctionAlreadyDefined.h"
#include "Exception/StoredFunctionNotFound.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/Unexpected.h"

#include "Execution/Program.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Optimizer.h"

#include "Statement/FunctionDefinition.h"
#include "Statement/DropFunctionStatement.h"
#include "Statement/Identifier.h"
#include "Statement/ParameterDeclarationList.h"
#include "Statement/ReturnsClause.h"
#include "Statement/Type.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace _Name
	{
		// 関数名の重複を調べる
		bool _checkExistence(Trans::Transaction& cTrans_, const Database& cDatabase_, const Function* pFunction_);
	} // namespace _Name
} // namespace

//////////////////////////
// _Name
//////////////////////////

//	FUNCTION local
//	_Name::_checkExistence -- 関数名の重複を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			関数が属するデータベース
//		const Schema::Function* pFunction_
//			作成しようとしている関数
//
//	RETURN
//		true ... 同じ名前のものが存在している、または作成中である
//		false... 同じ名前のものはない
//
//	EXCEPTIONS
//		Exception::StoredFunctionAlreadyDefined
//			同じ名前のものが存在しており、CanceledWhenDuplicatedがfalseである

bool
_Name::_checkExistence(Trans::Transaction& cTrans_,
					   const Database& cDatabase_,
					   const Function* pFunction_)
{
	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (Manager::ObjectName::reserve(pFunction_) == false) {

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage
				<< "Function definition of the same name in progress("
				<< pFunction_->getName()
				<< ") canceled"
				<< ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Function definition of the same name in progress("
				<< pFunction_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW2(Exception::StoredFunctionAlreadyDefined,
						   pFunction_->getName(), cDatabase_.getName());
		}
	}

	// さらに、同じ名前の関数がすでにないか調べ、
	// 同時に現在の関数をマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときに不完全なFunctionを
	// 読み込んでしまう

	if (cDatabase_.getFunction(pFunction_->getName(), cTrans_)) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pFunction_);

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated function definition("
						   << pFunction_->getName()
						   << ") canceled"
						   << ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated function definition("
						   << pFunction_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::StoredFunctionAlreadyDefined,
						   pFunction_->getName(), cDatabase_.getName());
		}
	}
	return false;
}

///////////////////////
// Schema::Function	 //
///////////////////////

//	FUNCTION public
//		Schema::Function::Function -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//

Function::
Function()
	: Object(Object::Category::Function),
	  m_eVersion(Version::CurrentVersion),
	  m_cReturnType(),
	  m_pRoutine(0)
{
}

//
//	FUNCTION public
//		Schema::Function::Function -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			関数を定義するデータベース
//		const Statement::FunctionDefinition& cStatement_
//			関数定義のステートメント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Function::
Function(const Database& cDatabase_, const Statement::FunctionDefinition& cStatement_)
	: Object(Object::Category::Function, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_eVersion(Version::CurrentVersion),
	  m_cReturnType(),
	  m_pRoutine(0)
{
	// 関数定義のSQL文から関数オブジェクトを作る
	// create function <function名> (parameter宣言) returns <return type> return <value expression>

	// 渡されたStatement::Objectの中身は正しいはずである

	const Statement::Identifier* pIdentifier = cStatement_.getFunctionName();
	; _SYDNEY_ASSERT(pIdentifier);
	; _SYDNEY_ASSERT(pIdentifier->getIdentifier());
	; _SYDNEY_ASSERT(pIdentifier->getIdentifier()->getLength());

	setName(*pIdentifier->getIdentifier());

	setReturnType(cStatement_);
	setRoutine(cStatement_);
}

//
//	FUNCTION public
//		Schema::Function::Function -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			関数を定義するデータベース
//		const Schema::LogData& cLogData_
//			関数定義のログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Function::
Function(const Database& cDatabase_, const LogData& cLogData_)
	: Object(Object::Category::Function, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_eVersion(Version::CurrentVersion),
	  m_cReturnType(),
	  m_pRoutine(0)
{
	// ログの内容を反映する
	//	ログの内容:
	//		1．名前
	//		2. ID(使用しない)
	//		3．Version
	//		4. Routine
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える

	if (!unpackMetaField(cLogData_[Log::Name].get(), Meta::Function::Name)
		|| !unpackMetaField(cLogData_[Log::Create::Version].get(), Meta::Function::Version)
		|| !unpackMetaField(cLogData_[Log::Create::ReturnType].get(), Meta::Function::ReturnType)
		|| !unpackMetaField(cLogData_[Log::Create::Routine].get(), Meta::Function::Routine)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

//
//	FUNCTION public
//		Schema::Function::~Function -- デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//

Function::
~Function()
{
	destruct();
}

//	FUNCTION public
//		Schema::Function::getNewInstance -- オブジェクトを新たに取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			元になるデータ
//
//	RETURN
//		新規に作成されたオブジェクト
//
//	EXCEPTIONS

// static
Function*
Function::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Function> pObject = new Function;
	pObject->unpack(cData_);
	return pObject.release();
}

//
//	FUNCTION public
//		Schema::Function::create -- 関数を生成する
//
//	NOTES
//		システム表の永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Database& cDatabase_
//			関数を定義するデータベース
//		const Statement::FunctionDefinition& cStatement_
//			関数定義のステートメント
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			関数抹消を行うトランザクション記述子
//
//	RETURN
//		Schema::Function*
//			生成した関数オブジェクト
//
//	EXCEPTIONS
//		Exception::StoredFunctionAlreadyDefined
//			存在する名前でcreateしようとした

// static
Function*
Function::
create(Database& cDatabase_, const Statement::FunctionDefinition& cStatement_,
	   LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 定義から関数オブジェクトを生成する
	ModAutoPointer<Function> pFunction = new Function(cDatabase_, cStatement_);
	; _SYDNEY_ASSERT(pFunction.get());

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pFunction.get())) {
		return 0;
	}

	// IDをふり、状態を変える
	pFunction->Object::create(cTrans_);

	// ログデータを作る
	pFunction->makeLogData(cLogData_);

	// 生成された関数オブジェクトを返す
    return pFunction.release();
}

//
//	FUNCTION public
//		Schema::Function::create -- 関数を生成する
//
//	NOTES
//		リカバリー中のredoで呼ばれる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			関数定義を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			関数を定義するデータベース
//		const Schema::LogData& cLogData_
//			関数定義のログデータ
//
//	RETURN
//		Schema::Function*
//			生成した関数オブジェクト
//
//	EXCEPTIONS
//		Exception::StoredFunctionAlreadyDefined
//			存在する名前でcreateしようとした

// static
Function*
Function::
create(Trans::Transaction& cTrans_, const Database& cDatabase_,
	   const LogData& cLogData_)
{
	// ログデータから関数オブジェクトを生成する

	ModAutoPointer<Function> pFunction = new Function(cDatabase_, cLogData_);
	; _SYDNEY_ASSERT(pFunction.get());

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pFunction.get())) {
		return 0;
	}

	// IDはログのものを使用する
	Object::ID::Value id = getObjectID(cLogData_);

	// IDの整合性を取り、状態を変える
	pFunction->Object::create(cTrans_, id);

	// 生成された関数オブジェクトを返す
    return pFunction.release();
}

//
//	FUNCTION public
//		Schema::Function::getName -- 関数抹消のSQL文から関数名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropFunctionStatement& cStatement_
//			関数抹消のステートメント
//
//	RETURN
//		対象の関数名
//
//	EXCEPTIONS

// static
Object::Name
Function::
getName(const Statement::DropFunctionStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getFunctionName();
	; _SYDNEY_ASSERT(pIdentifier);
	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

//
//	FUNCTION public
//		Schema::Function::getName -- 
//
//	NOTES
//		getNameをオーバーライドしたので
//		Objectのものを再度定義する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		関数名
//
//	EXCEPTIONS

const Object::Name&
Function::
getName() const
{
	return Object::getName();
}

//	FUNCTION public
//		Schema::Function::drop -- 関数の破棄に関する処理をする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Function& cFunction_
//			破棄対象の関数
//		Scheam::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Function::
drop(Function& cFunction_, LogData& cLogData_,
	 Trans::Transaction& cTrans_)
{
	// 破棄マークをつける
	cFunction_.drop(cTrans_);
	// ログデータを作る
	cFunction_.makeLogData(cLogData_);
}

// FUNCTION public
//	Schema::Function::drop -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	const LogData& cLogData_
//	
// RETURN
//	Function::Pointer
//
// EXCEPTIONS

//static
Function::Pointer
Function::
drop(Trans::Transaction& cTrans_,
	 Database& cDatabase_,
	 const LogData& cLogData_)
{
	// get schema object id from log data
	ObjectID::Value id = getObjectID(cLogData_);

	// get function object from the id
	Function* pFunction = cDatabase_.getFunction(id, cTrans_);

	// set drop flag
	pFunction->drop(cTrans_);

	return Pointer(syd_reinterpret_cast<const Function*>(pFunction));
}

//
//	FUNCTION public
//		Schema::Function::drop -- 関数を実際に抹消する
//
//	NOTES
//		システム表のXロックは呼び出し側で行う必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//

void
Function::
drop(Trans::Transaction& cTrans_, bool bRecovery_/* = false */)
{
	// 状態を変化させる
	Object::drop(bRecovery_);
}

//
//	FUNCTION public
//		Schema::Function::get -- スキーマオブジェクトIDを指定して関数を得る
//
//	NOTES
//		マウント中のデータベースに対してこの関数を使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			取得する関数のスキーマオブジェクトID
//		Schema::Database* pDatabase_
//			取得する関数が属するデータベース
//		Trans::Transaction& cTrans_
//			関数を取得しようとしているトランザクション記述子
//
//	RETURN
//		取得した関数のオブジェクト
//
//	EXCEPTIONS
//		???
//

// static
Function*
Function::
get(ID::Value iID_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_);

	if (iID_ == Object::ID::Invalid)
		return 0;

	return (pDatabase_) ? pDatabase_->getFunction(iID_, cTrans_) : 0;
}

// FUNCTION public
//	Schema::Function::getReturnType -- 返り値の型を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::SQLData&
//
// EXCEPTIONS

const Common::SQLData&
Function::
getReturnType() const
{
	return m_cReturnType;
}

// FUNCTION public
//	Schema::Function::getRoutine -- 関数本体を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Statement::Object*
//
// EXCEPTIONS

const Statement::Object*
Function::
getRoutine() const
{
	return m_pRoutine;
}

//	FUNCTION public
//	Schema::Function::doBeforePersist -- 永続化する前に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::FunctionPointer& pFunction_
//			永続化する関数のオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の関数の状態
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Function::
doBeforePersist(const Pointer& pFunction_, Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pFunction_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Changed:
	case Status::Mounted:
	case Status::DeleteCanceled:
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		; // 何もしない
		break;
	}
	default:
		// 何もしない
		break;
	}
}

//	FUNCTION public
//	Schema::Function::doAfterPersist -- 永続化した後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::FunctionPointer& pFunction_
//			永続化した関数のオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の関数の状態
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Function::
doAfterPersist(const Pointer& pFunction_, Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pFunction_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pFunction_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		Database* pDatabase = pFunction_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);
		// 関数はキャッシュに入れない
		(void) pDatabase->addFunction(pFunction_, cTrans_);
		break;
	}
	case Status::Changed:
	{
		break;
	}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらマネージャーの登録からの削除も行う

		// 状態を「実際に削除された」にする

		pFunction_->setStatus(Schema::Object::Status::ReallyDeleted);

		// 下位オブジェクトがあればそれを抹消してからdeleteする
		pFunction_->reset();

		Database* pDatabase = pFunction_->getDatabase(cTrans_);
		if (pDatabase)
			pDatabase->eraseFunction(pFunction_->getID());
		break;
	}
	case Status::CreateCanceled:
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Function);
}

//	FUNCTION public
//	Schema::Function::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::FunctionPointer& pFunction_
//			永続化した関数のオブジェクト
//		Schema::Database& cDatabase_
//			関数が属するデータベース
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Function::
doAfterLoad(const Pointer& pFunction_, Database& cDatabase_, Trans::Transaction& cTrans_)
{
	// システムへ読み出した関数を表すクラスを追加する
	// また、マネージャーにこの関数を表すクラスを
	// スキーマオブジェクトとして管理させる
	// 関数はキャッシュに入れない

	cDatabase_.addFunction(pFunction_, cTrans_);
}

//	FUNCTION public
//	Schema::Function::serialize --
//		関数を表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Function::
serialize(ModArchive& archiver)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Function::getClassID -- このクラスのクラス ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

int
Function::
getClassID() const
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Function::clear --
//		関数を表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//		下位オブジェクトのキャッシュからの抹消を行わないので
//		キャッシュに載っているオブジェクトに対してこのメソッドを呼ぶときには
//		あらかじめキャッシュから抹消する処理を行う必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Function::
clear()
{
	destruct();
}

//
//	FUNCTION private
//		Schema::Function::destruct -- デストラクター下位関数
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
Function::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteする前にresetを呼ぶ
	// ここでは領域を開放するのみ

	delete m_pRoutine, m_pRoutine = 0;
}

//
//	FUNCTION public
//		Schema::Function::reset --
//			下位オブジェクトを抹消する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
Function::
reset()
{
	;
}
void
Function::
reset(Database& cDatabase_)
{
	reset();
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// FUNCTION public
//	Schema::Function::compile -- compile routine
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Execution::Interface::IProgram*
//
// EXCEPTIONS

Execution::Interface::IProgram*
Function::
compile(Trans::Transaction& cTrans_)
{
	Execution::Program cProgram;
	cProgram.setImplementation(Execution::Program::Version::V2);
	cProgram.setProgram(Execution::Interface::IProgram::create());
	Opt::Optimizer::compile(getDatabase(cTrans_),
							&cProgram,
							m_pRoutine,
							&cTrans_);
	return cProgram.releaseProgram();
}

// FUNCTION public
//	Schema::Function::getParameterCount -- get number of parameters
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Function::
getParameterCount()
{
	Statement::FunctionDefinition* pDefinition =
		_SYDNEY_DYNAMIC_CAST(Statement::FunctionDefinition*, m_pRoutine);
	if (pDefinition == 0) {
		// can't get count
		return -1;
	}
	return pDefinition->getParam()->getCount();
}

//	FUNCTION public
//	Schema::Function::makeLogData --
//		ログデータを作る
//
//	NOTES
//		引数のログデータには種別が設定されている必要がある
//
//	ARGUMENTS
//		LogData& cLogData_
//			値を設定するログデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Function::
makeLogData(LogData& cLogData_) const
{
	// 全ログに共通のデータ
	//	1. 名前
	//	2. ID
	cLogData_.addData(packMetaField(Meta::Function::Name));
	cLogData_.addData(packMetaField(Meta::Function::ID));

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreateFunction:
	{
		//	関数作成
		//		3．Version
		//		4. ReturnType
		//		5．Routine
		cLogData_.addData(packMetaField(Meta::Function::Version));
		cLogData_.addData(packMetaField(Meta::Function::ReturnType));
		cLogData_.addData(packMetaField(Meta::Function::Routine));
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		break;
	}
	case LogData::Category::DropFunction:
	{
		//	 関数の破棄
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Drop::Num);
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

//
//	FUNCTION public
//		Schema::Function::getObjectID -- ログデータより Function ID を取得する
//
//	NOTES
//
//	ARGUMENTS
//		const LogData& log_
//			ログテータ
//		ObjectID::Value& id_
//			ID 値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

// static 

Object::ID::Value
Function::
getObjectID(const LogData& log_)
{
	return log_.getID(Log::ID);
}

//
//	FUNCTION public
//		Schema::Function::getName -- ログデータから関数名を得る
//
//	NOTES
//		関数の変更または抹消のログデータから取得する
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			関数変更または抹消のログデータ
//
//	RETURN
//		対象の関数名
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログデータから関数名が得られなかった

// static
Object::Name
Function::
getName(const LogData& cLogData_)
{
	// ログデータの内容を取得
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える
	return cLogData_.getString(Log::Name);
}

// FUNCTION private
//	Schema::Function::setReturnType -- 
//
// NOTES
//
// ARGUMENTS
//	const Statement::FunctionDefinition& cStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Function::
setReturnType(const Statement::FunctionDefinition& cStatement_)
{
	m_cReturnType = cStatement_.getReturns()->getDataType();
}

// FUNCTION private
//	Schema::Function::setRoutine -- 
//
// NOTES
//
// ARGUMENTS
//	const Statement::FunctionDefinition& cStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Function::
setRoutine(const Statement::FunctionDefinition& cStatement_)
{
	m_pRoutine = cStatement_.copy();
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「関数」表の構造は以下のとおり
// create table Function (
//		ID		id,
//		name	nvarchar,
//		time	timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Function>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Function>(Meta::MemberType::_type_, &Function::_get_, &Function::_set_)

	Meta::Definition<Function> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Version
		_DEFINE0(Binary),		// ReturnType
		_DEFINE0(Binary),		// Routine
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Function::getMetaFieldNumber --
//		スキーマオブジェクトを格納するファイルのフィールド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

int
Function::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Function::MemberNum);
}

//	FUNCTION public
//	Schema::Function::getMetaFieldDefinition --
//		スキーマオブジェクトを格納するファイルのフィールド定義を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			フィールドのメンバーを識別する番号
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

Meta::MemberType::Value
Function::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Function::packMetaField --
//		スキーマオブジェクトの内容をレコードファイルに格納するため
//		DataArrayDataにする
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		0以外...正しく変換された
//		0    ...変換に失敗した
//
//	EXCEPTIONS

Common::Data::Pointer
Function::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Function>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			return packIntegerMetaField(iMemberID_);
		}
	case Meta::MemberType::Binary:
		{
			return packBinaryMetaField(iMemberID_);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Function::unpackMetaField --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			内容を反映するData
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		true...正しく変換された
//		false..変換に失敗した
//
//	EXCEPTIONS

bool
Function::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Function>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			return unpackIntegerMetaField(pData_, iMemberID_);
		}
	case Meta::MemberType::Binary:
		{
			return unpackBinaryMetaField(pData_, iMemberID_);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	return false;
}

// FUNCTION public
//	Schema::Function::packIntegerMetaField -- 
//
// NOTES
//
// ARGUMENTS
//	int iMemberID_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

Common::Data::Pointer
Function::
packIntegerMetaField(int iMemberID_) const
{
	switch (iMemberID_) {
	case Meta::Function::Version:
		{
			return pack(static_cast<int>(m_eVersion));
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

// FUNCTION public
//	Schema::Function::packBinaryMetaField -- 
//
// NOTES
//
// ARGUMENTS
//	int iMemberID_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

Common::Data::Pointer
Function::
packBinaryMetaField(int iMemberID_) const
{
	Utility::BinaryData& cArchiver = getArchiver();
	switch (iMemberID_) {
	case Meta::Function::ReturnType:
		{
			return cArchiver.put(&m_cReturnType);
		}
	case Meta::Function::Routine:
		{
			return cArchiver.put(m_pRoutine);
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

// FUNCTION public
//	Schema::Function::unpackIntegerMetaField -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	int iMemberID_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Function::
unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		return false;
	} else {
		int value;
		if (unpack(pData_, value)) {
			switch (iMemberID_) {
			case Meta::Function::Version:
				{
					if (value >= 0 && value < Version::ValueNum) {
						m_eVersion = static_cast<Version::Value>(value);
						return true;
					}
					break;
				}
			default:
				break;
			}
		}
	}
	return false;
}

// FUNCTION public
//	Schema::Function::unpackBinaryMetaField -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	int iMemberID_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Function::
unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		return false;

	} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
		const Common::BinaryData* pBinary =
			_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

		Utility::BinaryData& cArchiver = getArchiver();

		switch (iMemberID_) {
		case Meta::Function::ReturnType:
			{
				ModAutoPointer<Common::Externalizable> pData = cArchiver.get(pBinary);

				if (Common::SQLData* pType = dynamic_cast<Common::SQLData*>(pData.get())) {
					m_cReturnType = *pType;
					return true;
				}
				break;
			}
		case Meta::Function::Routine:
			{
				ModAutoPointer<Statement::Object> pRoutine =
					dynamic_cast<Statement::Object*>(cArchiver.get(pBinary));
				if (pRoutine.get()) {
					m_pRoutine = pRoutine.release();
				}
				return true;
			}
		default:
			break;
		}
	}
	return false;
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
