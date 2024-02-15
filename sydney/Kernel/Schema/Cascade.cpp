// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Cascade.cpp -- 子サーバー関連の子サーバー定義
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Cascade.h"
#include "Schema/Database.h"
#include "Schema/FakeError.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/ObjectID.h"
#include "Schema/Parameter.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Cascade.h"
#include "Schema/Utility.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "DServer/Cascade.h"

#include "Exception/CascadeAlreadyDefined.h"
#include "Exception/CascadeNotFound.h"
#include "Exception/InvalidCascade.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Statement/AlterCascadeStatement.h"
#include "Statement/CascadeDefinition.h"
#include "Statement/DropCascadeStatement.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"
#include "Statement/Token.h"
#include "Statement/ValueExpression.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace _Name
	{
		// 子サーバー名の重複を調べる
		bool _checkExistence(Trans::Transaction& cTrans_, const Database& cDatabase_, const Cascade* pCascade_);
	} // namespace _Name

	// ValueExpressionから文字列を得る
	ModUnicodeString
	_convertValueExpressionToString(const Statement::ValueExpression* pValueExpression_)
	{
		if (pValueExpression_->getOperator() != Statement::ValueExpression::op_Literal) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		const Statement::Literal* pLiteral =
			_SYDNEY_DYNAMIC_CAST(const Statement::Literal*,
								 pValueExpression_->getPrimary());
		const Statement::Token& cToken = pLiteral->getToken();
		if (cToken.getHead() == cToken.getTail()) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		return ModUnicodeString(cToken.getHead(), cToken.getLength());
	}

	// Identifierから文字列を得る
	ModUnicodeString
	_convertIdentifierToString(const Statement::Identifier* pIdentifier_)
	{

		if (pIdentifier_
			&& pIdentifier_->getIdentifier()
			&& pIdentifier_->getIdentifier()->getLength()) {

			return *pIdentifier_->getIdentifier();
		}
		return ModUnicodeString();
	}

} // namespace

//////////////////////////
// _Name
//////////////////////////

//	FUNCTION local
//	_Name::_checkExistence -- 子サーバー名の重複を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			子サーバーが属するデータベース
//		const Schema::Cascade* pCascade_
//			作成しようとしている子サーバー
//
//	RETURN
//		true ... 同じ名前のものが存在している、または作成中である
//		false... 同じ名前のものはない
//
//	EXCEPTIONS
//		Exception::CascadeAlreadyDefined
//			同じ名前のものが存在しており、CanceledWhenDuplicatedがfalseである

bool
_Name::_checkExistence(Trans::Transaction& cTrans_,
					   const Database& cDatabase_,
					   const Cascade* pCascade_)
{
	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (Manager::ObjectName::reserve(pCascade_) == false) {

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage
				<< "Cascade definition of the same name in progress("
				<< pCascade_->getName()
				<< ") canceled"
				<< ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Cascade definition of the same name in progress("
				<< pCascade_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW2(Exception::CascadeAlreadyDefined,
						   pCascade_->getName(), cDatabase_.getName());
		}
	}

	// さらに、同じ名前の子サーバーがすでにないか調べ、
	// 同時に現在の子サーバーをマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときに不完全なCascadeを
	// 読み込んでしまう

	if (cDatabase_.getCascade(pCascade_->getName(), cTrans_)) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pCascade_);

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated cascade definition("
						   << pCascade_->getName()
						   << ") canceled"
						   << ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated cascade definition("
						   << pCascade_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::CascadeAlreadyDefined,
						   pCascade_->getName(), cDatabase_.getName());
		}
	}
	return false;
}

///////////////////////////////
// Schema::Cascade::Target	 //
///////////////////////////////

// FUNCTION public
//	Schema::Schema::Cascade::Target::addToLog -- 
//
// NOTES
//
// ARGUMENTS
//	LogData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Schema::Cascade::Target::
addToLog(LogData& cLogData_)
{
	ModVector<ModUnicodeString> vecString;
	vecString.pushBack(m_cstrHost);
	vecString.pushBack(m_cstrPort);
	vecString.pushBack(m_cstrDatabase);

	cLogData_.addStrings(vecString);
}

// FUNCTION public
//	Schema::Schema::Cascade::Target::setFromLog -- 
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	int iIndex_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Schema::Cascade::Target::
setFromLog(const LogData& cLogData_,
		   int iIndex_)
{
	const ModVector<ModUnicodeString>& vecString =
		cLogData_.getStrings(iIndex_);
	if (vecString.getSize() != 2
		&& vecString.getSize() != 3) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	m_cstrHost = vecString[0];
	m_cstrPort = vecString[1];
	if (vecString.getSize() == 3) {
		m_cstrDatabase = vecString[2];
	}
}

///////////////////////
// Schema::Cascade	 //
///////////////////////

//	FUNCTION public
//		Schema::Cascade::Cascade -- コンストラクター
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

Cascade::
Cascade()
	: Object(Object::Category::Cascade),
	  m_eVersion(Version::CurrentVersion),
	  m_cTarget()
{
}

//
//	FUNCTION public
//		Schema::Cascade::Cascade -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			子サーバーを定義するデータベース
//		const Statement::CascadeDefinition& cStatement_
//			子サーバー定義のステートメント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Cascade::
Cascade(const Database& cDatabase_, const Statement::CascadeDefinition& cStatement_)
	: Object(Object::Category::Cascade, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_eVersion(Version::CurrentVersion),
	  m_cTarget()
{
	// 子サーバー定義のSQL文から子サーバーオブジェクトを作る
	// create cascade <cascade名> on <host name> <port number>

	// 渡されたStatement::Objectの中身は正しいはずである

	const Statement::Identifier* pIdentifier = cStatement_.getCascadeName();
	; _SYDNEY_ASSERT(pIdentifier);
	; _SYDNEY_ASSERT(pIdentifier->getIdentifier());
	; _SYDNEY_ASSERT(pIdentifier->getIdentifier()->getLength());

	setName(_convertIdentifierToString(pIdentifier));

	setTarget(cStatement_);
}

//
//	FUNCTION public
//		Schema::Cascade::Cascade -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			子サーバーを定義するデータベース
//		const Schema::LogData& cLogData_
//			子サーバー定義のログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Cascade::
Cascade(const Database& cDatabase_, const LogData& cLogData_)
	: Object(Object::Category::Cascade, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_eVersion(Version::CurrentVersion),
	  m_cTarget()
{
	// ログの内容を反映する
	//	ログの内容:
	//		1．名前
	//		2. ID(使用しない)
	//		3．Version
	//		4. Routine
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える

	if (!unpackMetaField(cLogData_[Log::Name].get(), Meta::Cascade::Name)
		|| !unpackMetaField(cLogData_[Log::Create::Version].get(), Meta::Cascade::Version)
		|| !unpackMetaField(cLogData_[Log::Create::Target].get(), Meta::Cascade::Target)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

//
//	FUNCTION public
//		Schema::Cascade::~Cascade -- デストラクター
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

Cascade::
~Cascade()
{
	destruct();
}

//	FUNCTION public
//		Schema::Cascade::getNewInstance -- オブジェクトを新たに取得する
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
Cascade*
Cascade::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Cascade> pObject = new Cascade;
	pObject->unpack(cData_);
	return pObject.release();
}

//
//	FUNCTION public
//		Schema::Cascade::create -- 子サーバーを生成する
//
//	NOTES
//		システム表の永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Database& cDatabase_
//			子サーバーを定義するデータベース
//		const Statement::CascadeDefinition& cStatement_
//			子サーバー定義のステートメント
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			子サーバー抹消を行うトランザクション記述子
//
//	RETURN
//		Schema::Cascade*
//			生成した子サーバーオブジェクト
//
//	EXCEPTIONS
//		Exception::CascadeAlreadyDefined
//			存在する名前でcreateしようとした

// static
Cascade*
Cascade::
create(Database& cDatabase_, const Statement::CascadeDefinition& cStatement_,
	   LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 定義から子サーバーオブジェクトを生成する
	ModAutoPointer<Cascade> pCascade = new Cascade(cDatabase_, cStatement_);
	; _SYDNEY_ASSERT(pCascade.get());

	if (ModUnicodeCharTrait::toUInt(pCascade->getTarget().m_cstrPort) == 0) {
		_SYDNEY_THROW0(Exception::InvalidCascade);
	}

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pCascade.get())) {
		return 0;
	}

	// IDをふり、状態を変える
	pCascade->Object::create(cTrans_);

	// ログデータを作る
	pCascade->makeLogData(cLogData_);

	// 生成された子サーバーオブジェクトを返す
    return pCascade.release();
}

//
//	FUNCTION public
//		Schema::Cascade::create -- 子サーバーを生成する
//
//	NOTES
//		リカバリー中のredoで呼ばれる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			子サーバー定義を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			子サーバーを定義するデータベース
//		const Schema::LogData& cLogData_
//			子サーバー定義のログデータ
//
//	RETURN
//		Schema::Cascade*
//			生成した子サーバーオブジェクト
//
//	EXCEPTIONS
//		Exception::CascadeAlreadyDefined
//			存在する名前でcreateしようとした

// static
Cascade*
Cascade::
create(Trans::Transaction& cTrans_, const Database& cDatabase_,
	   const LogData& cLogData_)
{
	// ログデータから子サーバーオブジェクトを生成する

	ModAutoPointer<Cascade> pCascade = new Cascade(cDatabase_, cLogData_);
	; _SYDNEY_ASSERT(pCascade.get());

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pCascade.get())) {
		return 0;
	}

	// IDはログのものを使用する
	Object::ID::Value id = getObjectID(cLogData_);

	// IDの整合性を取り、状態を変える
	pCascade->Object::create(cTrans_, id);

	// 生成された子サーバーオブジェクトを返す
    return pCascade.release();
}

//
//	FUNCTION public
//		Schema::Cascade::getName -- 子サーバー抹消のSQL文から子サーバー名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::AlterCascadeStatement& cStatement_
//			子サーバー変更のステートメント
//
//	RETURN
//		対象の子サーバー名
//
//	EXCEPTIONS

// static
Object::Name
Cascade::
getName(const Statement::AlterCascadeStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getCascadeName();
	; _SYDNEY_ASSERT(pIdentifier);
	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

//
//	FUNCTION public
//		Schema::Cascade::getName -- 子サーバー抹消のSQL文から子サーバー名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropCascadeStatement& cStatement_
//			子サーバー抹消のステートメント
//
//	RETURN
//		対象の子サーバー名
//
//	EXCEPTIONS

// static
Object::Name
Cascade::
getName(const Statement::DropCascadeStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getCascadeName();
	; _SYDNEY_ASSERT(pIdentifier);
	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

//
//	FUNCTION public
//		Schema::Cascade::getName -- 
//
//	NOTES
//		getNameをオーバーライドしたので
//		Objectのものを再度定義する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		子サーバー名
//
//	EXCEPTIONS

const Object::Name&
Cascade::
getName() const
{
	return Object::getName();
}

//	FUNCTION public
//		Schema::Cascade::drop -- 子サーバーの破棄に関する処理をする
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Cascade& cCascade_
//			破棄対象の子サーバー
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
Cascade::
drop(Cascade& cCascade_, LogData& cLogData_,
	 Trans::Transaction& cTrans_)
{
	// 破棄マークをつける
	cCascade_.drop(cTrans_);
	// ログデータを作る
	cCascade_.makeLogData(cLogData_);
}

// FUNCTION public
//	Schema::Cascade::drop -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	const LogData& cLogData_
//	
// RETURN
//	Cascade::Pointer
//
// EXCEPTIONS

//static
Cascade::Pointer
Cascade::
drop(Trans::Transaction& cTrans_,
	 Database& cDatabase_,
	 const LogData& cLogData_)
{
	// get schema object id from log data
	ObjectID::Value id = getObjectID(cLogData_);

	// get cascade object from the id
	Cascade* pCascade = cDatabase_.getCascade(id, cTrans_);

	// set drop flag
	pCascade->drop(cTrans_);

	return Pointer(syd_reinterpret_cast<const Cascade*>(pCascade));
}

//
//	FUNCTION public
//		Schema::Cascade::drop -- 子サーバーを実際に抹消する
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
Cascade::
drop(Trans::Transaction& cTrans_, bool bRecovery_/* = false */)
{
	// 状態を変化させる
	Object::drop(bRecovery_);
}

// FUNCTION public
//	Schema::Cascade::alter -- 子サーバー変更の準備をする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Cascade& cCascade_
//	const Statement::AlterCascadeStatement& cStatement_
//	Target& cPrevTarget_
//	Target& cPostTarget_
//	LogData& cLogData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Cascade::
alter(Trans::Transaction& cTrans_,
	  Cascade& cCascade_,
	  const Statement::AlterCascadeStatement& cStatement_,
	  Target& cPrevTarget_,
	  Target& cPostTarget_,
	  LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// get target specification from alter statement
	cPostTarget_.m_cstrHost = _convertValueExpressionToString(cStatement_.getHost());
	cPostTarget_.m_cstrPort = _convertValueExpressionToString(cStatement_.getPort());
	cPostTarget_.m_cstrDatabase = _convertIdentifierToString(cStatement_.getDatabase());

	if (ModUnicodeCharTrait::toUInt(cPostTarget_.m_cstrPort) == 0) {
		_SYDNEY_THROW0(Exception::InvalidCascade);
	}

	// get previous specification from object
	cPrevTarget_.m_cstrHost = cCascade_.getTarget().m_cstrHost;
	cPrevTarget_.m_cstrPort = cCascade_.getTarget().m_cstrPort;
	cPrevTarget_.m_cstrDatabase = cCascade_.getTarget().m_cstrDatabase;

	if (cPostTarget_.m_cstrHost == cPrevTarget_.m_cstrHost
		&& cPostTarget_.m_cstrPort == cPrevTarget_.m_cstrPort
		&& cPostTarget_.m_cstrDatabase == cPrevTarget_.m_cstrDatabase) {
		// not changed -> do nothing
		return false;
	}

	// create log data
	cCascade_.makeLogData(cLogData_);
	// add target list
	cPrevTarget_.addToLog(cLogData_);
	cPostTarget_.addToLog(cLogData_);
	return true;
}

// FUNCTION public
//	Schema::Cascade::alter -- 子サーバー変更の準備をする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const LogData& cLogData_
//	Target& cPrevTarget_
//	Target& cPostTarget_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Cascade::
alter(Trans::Transaction& cTrans_,
	  const LogData& cLogData_,
	  Target& cPrevTarget_,
	  Target& cPostTarget_)
{
	cPostTarget_.setFromLog(cLogData_, Log::Alter::PostTarget);
	return true;
}

//
//	FUNCTION public
//		Schema::Cascade::get -- スキーマオブジェクトIDを指定して子サーバーを得る
//
//	NOTES
//		マウント中のデータベースに対してこの子サーバーを使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			取得する子サーバーのスキーマオブジェクトID
//		Schema::Database* pDatabase_
//			取得する子サーバーが属するデータベース
//		Trans::Transaction& cTrans_
//			子サーバーを取得しようとしているトランザクション記述子
//
//	RETURN
//		取得した子サーバーのオブジェクト
//
//	EXCEPTIONS
//		???
//

// static
Cascade*
Cascade::
get(ID::Value iID_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_);

	if (iID_ == Object::ID::Invalid)
		return 0;

	return (pDatabase_) ? pDatabase_->getCascade(iID_, cTrans_) : 0;
}

// CASCADE public
//	Schema::Cascade::getTarget -- 対象を指定する情報を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Cascade::Target&
//
// EXCEPTIONS

const Cascade::Target&
Cascade::
getTarget() const
{
	return m_cTarget;
}

// FUNCTION public
//	Schema::Cascade::setTarget -- 対象を指定する情報を設定する
//
// NOTES
//
// ARGUMENTS
//	const Target& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Cascade::
setTarget(const Target& cTarget_)
{
	m_cTarget = cTarget_;
}

//	FUNCTION public
//	Schema::Cascade::doBeforePersist -- 永続化する前に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::CascadePointer& pCascade_
//			永続化する子サーバーのオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の子サーバーの状態
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
Cascade::
doBeforePersist(const Pointer& pCascade_, Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pCascade_.get());

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
//	Schema::Cascade::doAfterPersist -- 永続化した後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::CascadePointer& pCascade_
//			永続化した子サーバーのオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の子サーバーの状態
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
Cascade::
doAfterPersist(const Pointer& pCascade_, Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pCascade_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pCascade_->getDatabaseID();
	Database* pDatabase = pCascade_->getDatabase(cTrans_);

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		; _SYDNEY_ASSERT(pDatabase);
		// 子サーバーはキャッシュに入れない
		(void) pDatabase->addCascade(pCascade_, cTrans_);

		// tell dserver about cascade change
		DServer::Cascade::resetCascade(*pDatabase);
		break;
	}
	case Status::Changed:
	{
		; _SYDNEY_ASSERT(pDatabase);
		// tell dserver about cascade change
		DServer::Cascade::resetCascade(*pDatabase);
		break;
	}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらマネージャーの登録からの削除も行う

		// 状態を「実際に削除された」にする

		pCascade_->setStatus(Schema::Object::Status::ReallyDeleted);

		// 下位オブジェクトがあればそれを抹消してからdeleteする
		pCascade_->reset();

		if (pDatabase) {
			pDatabase->eraseCascade(pCascade_->getID());
			// tell dserver about cascade change
			DServer::Cascade::resetCascade(*pDatabase);
		}
		break;
	}
	case Status::CreateCanceled:
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Cascade);
}

//	FUNCTION public
//	Schema::Cascade::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::CascadePointer& pCascade_
//			永続化した子サーバーのオブジェクト
//		Schema::Database& cDatabase_
//			子サーバーが属するデータベース
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
Cascade::
doAfterLoad(const Pointer& pCascade_, Database& cDatabase_, Trans::Transaction& cTrans_)
{
	// システムへ読み出した子サーバーを表すクラスを追加する
	// また、マネージャーにこの子サーバーを表すクラスを
	// スキーマオブジェクトとして管理させる
	// 子サーバーはキャッシュに入れない

	cDatabase_.addCascade(pCascade_, cTrans_);
}

//	FUNCTION public
//	Schema::Cascade::serialize --
//		子サーバーを表すクラスのシリアライザー
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
Cascade::
serialize(ModArchive& archiver)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Cascade::getClassID -- このクラスのクラス ID を得る
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
Cascade::
getClassID() const
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Cascade::clear --
//		子サーバーを表すクラスのメンバーをすべて初期化する
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
Cascade::
clear()
{
	destruct();
}

//
//	FUNCTION private
//		Schema::Cascade::destruct -- デストラクター下位子サーバー
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
Cascade::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteする前にresetを呼ぶ
	// ここでは領域を開放するのみ
	;
}

//
//	FUNCTION public
//		Schema::Cascade::reset --
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
Cascade::
reset()
{
	;
}
void
Cascade::
reset(Database& cDatabase_)
{
	reset();
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

//	FUNCTION public
//	Schema::Cascade::makeLogData --
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
Cascade::
makeLogData(LogData& cLogData_) const
{
	// 全ログに共通のデータ
	//	1. 名前
	//	2. ID
	cLogData_.addData(packMetaField(Meta::Cascade::Name));
	cLogData_.addData(packMetaField(Meta::Cascade::ID));

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreateCascade:
	{
		//	子サーバー作成
		//		3．Version
		//		4. Target
		cLogData_.addData(packMetaField(Meta::Cascade::Version));
		cLogData_.addData(packMetaField(Meta::Cascade::Target));
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		break;
	}
	case LogData::Category::AlterCascade:
	{
		//	子サーバー変更
		//		-- 以下は呼び出し側で入れる
		//		3. Target(変更前)
		//		4．Target(変更後)
		break;
	}
	case LogData::Category::DropCascade:
	{
		//	 子サーバーの破棄
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
//		Schema::Cascade::getObjectID -- ログデータより Cascade ID を取得する
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
Cascade::
getObjectID(const LogData& log_)
{
	return log_.getID(Log::ID);
}

//
//	FUNCTION public
//		Schema::Cascade::getName -- ログデータから子サーバー名を得る
//
//	NOTES
//		子サーバーの変更または抹消のログデータから取得する
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			子サーバー変更または抹消のログデータ
//
//	RETURN
//		対象の子サーバー名
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログデータから子サーバー名が得られなかった

// static
Object::Name
Cascade::
getName(const LogData& cLogData_)
{
	// ログデータの内容を取得
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える
	return cLogData_.getString(Log::Name);
}

// CASCADE private
//	Schema::Cascade::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	const Statement::CascadeDefinition& cStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Cascade::
setTarget(const Statement::CascadeDefinition& cStatement_)
{
	m_cTarget.m_cstrHost = _convertValueExpressionToString(cStatement_.getHost());
	m_cTarget.m_cstrPort = _convertValueExpressionToString(cStatement_.getPort());
	m_cTarget.m_cstrDatabase = _convertIdentifierToString(cStatement_.getDatabase());
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「子サーバー」表の構造は以下のとおり
// create table Cascade (
//		ID		id,
//		name	nvarchar,
//		time	timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Cascade>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Cascade>(Meta::MemberType::_type_, &Cascade::_get_, &Cascade::_set_)

	Meta::Definition<Cascade> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Version
		_DEFINE0(StringArray),	// Target
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Cascade::getMetaFieldNumber --
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
Cascade::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Cascade::MemberNum);
}

//	FUNCTION public
//	Schema::Cascade::getMetaFieldDefinition --
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
Cascade::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Cascade::packMetaField --
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
Cascade::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Cascade>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			return packIntegerMetaField(iMemberID_);
		}
	case Meta::MemberType::StringArray:
		{
			return packStringArrayMetaField(iMemberID_);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Cascade::unpackMetaField --
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
Cascade::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Cascade>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			return unpackIntegerMetaField(pData_, iMemberID_);
		}
	case Meta::MemberType::StringArray:
		{
			return unpackStringArrayMetaField(pData_, iMemberID_);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	return false;
}

// CASCADE public
//	Schema::Cascade::packIntegerMetaField -- 
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
Cascade::
packIntegerMetaField(int iMemberID_) const
{
	switch (iMemberID_) {
	case Meta::Cascade::Version:
		{
			return pack(static_cast<int>(m_eVersion));
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

// CASCADE public
//	Schema::Cascade::packStringArrayMetaField -- 
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
Cascade::
packStringArrayMetaField(int iMemberID_) const
{
	switch (iMemberID_) {
	case Meta::Cascade::Target:
		{
			ModVector<ModUnicodeString> vecString;
			vecString.pushBack(m_cTarget.m_cstrHost);
			vecString.pushBack(m_cTarget.m_cstrPort);
			vecString.pushBack(m_cTarget.m_cstrDatabase);
			return pack(vecString);
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

// CASCADE public
//	Schema::Cascade::unpackIntegerMetaField -- 
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
Cascade::
unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		return false;
	} else {
		int value;
		if (unpack(pData_, value)) {
			switch (iMemberID_) {
			case Meta::Cascade::Version:
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

// CASCADE public
//	Schema::Cascade::unpackStringArrayMetaField -- 
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
Cascade::
unpackStringArrayMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		return false;

	} else if (pData_) {
		ModVector<ModUnicodeString> vecString;
		if (unpack(pData_, vecString)) {
			switch (iMemberID_) {
			case Meta::Cascade::Target:
				{
					if (vecString.getSize() >= 2) {
						m_cTarget.m_cstrHost = vecString[0];
						m_cTarget.m_cstrPort = vecString[1];
						if (vecString.getSize() >= 3) {
							m_cTarget.m_cstrDatabase = vecString[2];
						}
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

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
