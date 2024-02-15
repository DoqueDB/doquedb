// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Mount.cpp -- マウント関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Admin/Database.h"
#include "Admin/Debug.h"
#include "Admin/FakeError.h"
#include "Admin/LogData.h"
#include "Admin/Mount.h"
#include "Admin/Operation.h"
#include "Admin/Replicator.h"

#include "Checkpoint/Executor.h"
#include "Checkpoint/TimeStamp.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Common/StringArrayData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Common/UnsignedIntegerData.h"
#include "Exception/AreaNotFound.h"
#include "Exception/DatabaseAlreadyDefined.h"
#include "Exception/DatabaseNotMountable.h"
#include "Exception/FileAlreadyExisted.h"
#include "Exception/InvalidPath.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/Message.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Lock/Name.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Path.h"
#include "Schema/Area.h"
#include "Schema/AreaMap.h"
#include "Schema/AreaContentMap.h"
#include "Schema/Database.h"
#include "Schema/FileMap.h"
#include "Schema/Hold.h"
#include "Schema/Index.h"
#include "Schema/IndexMap.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#ifdef OBSOLETE
#include "Schema/Redo.h"
#endif
#include "Schema/SystemTable_Database.h"
#include "Schema/SystemTable_Area.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/Table.h"
#include "Schema/TableMap.h"
#include "Schema/Undo.h"
#include "Server/Session.h"
#include "Statement/AlterAreaAction.h"
#include "Statement/AlterAreaStatement.h"
#include "Statement/AreaDataDefinition.h"
#include "Statement/AreaElementList.h"
#include "Statement/DropAreaStatement.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"
#include "Statement/MountDatabaseStatement.h"
#include "Statement/OptionalAreaParameter.h"
#include "Statement/OptionalAreaParameterList.h"
#include "Statement/Type.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoLogFile.h"
#include "Trans/Transaction.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

//
//	MACRO
//		_BEGIN_DB_REORGANIZE_RECOVERY -- 「データベース」表書き換え時のエラー処理開始
//		_END_DB_REORGANIZE_RECOVERY	  -- 「データベース」表書き換え時のエラー処理終了
//
//	NOTES
//		二つはペアでエラー処理の前後に使用すること
//		この間にエラーが発生したら自動的にSystemTableがUnavailableになる

#define _BEGIN_DB_REORGANIZE_RECOVERY \
									if (Schema::SystemTable::isAvailable()) { \
										try {
#define _END_DB_REORGANIZE_RECOVERY	\
										} catch (Exception::Object& e) { \
											SydErrorMessage << "Error recovery failed. FATAL. " << e << ModEndl; \
											/* システム表を使用不可能にする*/ \
											Schema::SystemTable::setAvailability(false); \
											/* エラー処理中に発生した例外は再送しない */ \
											/* thru. */ \
										} catch (...) { \
											SydErrorMessage << "Mount error recovery failed. FATAL." << ModEndl; \
											/* システム表を使用不可能にする*/ \
											Schema::SystemTable::setAvailability(false); \
											/* エラー処理中に発生した例外は再送しない */ \
											/* thru. */ \
										} \
									}

#define ADMIN_MOUNT_FAKE_ERROR(__key__, __value__)	ADMIN_FAKE_ERROR("Admin::Mount", __key__, __value__)

///////////////////
// AreaModifyInfo
///////////////////

//	FUNCTION public
//	Admin::Mount::AreaModifyInfo::AreaModifyInfo -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object& cStatement_
//			エリア定義変更のStatementオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::AreaModifyInfo::
AreaModifyInfo(Statement::Object& cStatement_)
	: m_ID(Schema::Object::ID::Invalid),
	  m_Persist(true),
	  m_NeedToDelete(false),
	  m_Object(0)
{
	using namespace Statement;

	switch (cStatement_.getType()) {
	case ObjectType::AlterAreaStatement:
		{
			AlterAreaStatement& cAlter =
				_SYDNEY_DYNAMIC_CAST(AlterAreaStatement&, cStatement_);
			m_eType = ModifyType::Alter;
			m_Name = *(cAlter.getAreaName()->getIdentifier());
			break;
		}
	case ObjectType::DropAreaStatement:
		{
			DropAreaStatement& cDrop
				= _SYDNEY_DYNAMIC_CAST(DropAreaStatement&, cStatement_);
			m_eType = ModifyType::Drop;
			m_Name = *(cDrop.getName()->getIdentifier());
			m_vecPostPath.clear();
			break;
		}
	// DROP ALLではこのオブジェクトは作られない
	case ObjectType::OptionalAreaParameter:
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

//	FUNCTION public
//	Admin::Mount::AreaModifyInfo::AreaModifyInfo -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Area& cArea_
//			DROP対象のエリア
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::AreaModifyInfo::
AreaModifyInfo(Schema::Area& cArea_)
	: m_Persist(true),
	  m_NeedToDelete(false)
{
	m_eType = ModifyType::Drop;
	m_Name = cArea_.getName();
	m_ID = cArea_.getID();
	m_vecPrevPath = cArea_.getPath();
	m_vecPostPath.clear();
	m_Object = &cArea_;
}

//	FUNCTION public
//	Admin::Mount::AreaModifyInfo::setStatementData -- 変更後のパスをセットする
//
//	NOTES
//		Statement::Object&を引数としたコンストラクターなどで
//		m_eTypeに値が入っていることが前提
//
//	ARGUMENTS
//		Statement::Object& cStatement_
//			エリアの定義変更を表すStatementオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mount::AreaModifyInfo::
setStatementData(Statement::Object& cStatement_)
{
	using namespace Statement;

	switch (cStatement_.getType()) {
	case ObjectType::AlterAreaStatement:
		{
			; _SYDNEY_ASSERT(m_eType == ModifyType::Alter);
			; _SYDNEY_ASSERT(m_Object);

			AlterAreaStatement& cAlter =
				_SYDNEY_DYNAMIC_CAST(AlterAreaStatement&, cStatement_);

			AlterAreaAction* pAction = cAlter.getAlterAreaAction();
			; _SYDNEY_ASSERT(pAction);
			m_Object->setMovePrepare(pAction->getActionType(),
									 *(pAction->getAreaElementList()),
									 m_vecPrevPath, m_vecPostPath);
			break;
		}
	case ObjectType::DropAreaStatement:
		{
			; _SYDNEY_ASSERT(m_eType == ModifyType::Drop);
			; _SYDNEY_ASSERT(m_Object);

			// ログやエラー処理で必要なのでパスをメンバーに入れる
			m_vecPrevPath = m_Object->getPath();
			m_vecPostPath.clear();
			break;
		}
	}
}

//	FUNCTION public
//	Admin::Mount::AreaModifyInfo::setAreaData -- IDや変更前のパスをセットする
//
//	NOTES
//		Statement::Object&を引数としたコンストラクターなどで
//		m_eType, m_Nameに値が入っていることが前提
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			データベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mount::AreaModifyInfo::
setAreaData(Trans::Transaction& cTrans_, const Schema::Database& cDatabase_)
{
	using namespace Statement;

	Schema::Area* pArea = cDatabase_.getArea(m_Name, cTrans_);
	if (pArea) {
		m_ID = pArea->getID();
		m_Object = pArea;
	} else {
		_SYDNEY_THROW2(Exception::AreaNotFound, m_Name, cDatabase_.getName());
	}
}

//	FUNCTION public
//	Admin::Mount::AreaModifyInfo::pack -- 構造体の内容をDataArrayDataにする
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

Common::DataArrayData*
Mount::AreaModifyInfo::
pack() const
{
	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;
	pResult->reserve(MemberNum);

	pResult->pushBack(new Common::IntegerData(static_cast<int>(m_eType)));
	pResult->pushBack(new Common::StringData(m_Name));
	pResult->pushBack(new Common::UnsignedIntegerData(m_ID));
	pResult->pushBack(new Common::StringArrayData(m_vecPrevPath));
	pResult->pushBack(new Common::StringArrayData(m_vecPostPath));

	return pResult.release();
}

//	FUNCTION public
//	Admin::Mount::AreaModifyInfo::unpack -- DataArrayDataから構造体の内容をセットする
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			内容の元になるデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mount::AreaModifyInfo::
unpack(const Common::DataArrayData& cData_)
{
	if (cData_.getCount() != MemberNum) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	for (int i = 0; i < MemberNum; ++i) {
		const Common::Data::Pointer& pElement = cData_.getElement(i);

		switch (i) {
		case 0:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Integer) {
					m_eType = static_cast<ModifyType::Value>(
								 _SYDNEY_DYNAMIC_CAST(const Common::IntegerData&, *pElement).getValue());
					continue;
				}
				break;
			}
		case 1:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::String) {
					m_Name = _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *pElement).getValue();
					continue;
				}
				break;
			}
		case 2:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::UnsignedInteger) {
					m_ID = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *pElement).getValue();
					continue;
				}
				break;
			}
		case 3:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::String) {
					m_vecPrevPath = _SYDNEY_DYNAMIC_CAST(const Common::StringArrayData&, *pElement).getValue();
					continue;
				}
				break;
			}
		case 4:
			{
				const Common::Data::Pointer& pElement = cData_.getElement(i++);
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::String) {
					m_vecPostPath = _SYDNEY_DYNAMIC_CAST(const Common::StringArrayData&, *pElement).getValue();
					continue;
				}
				break;
			}
		}
		// ここに来るということはデータの内容がおかしいということ
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

///////////////////
// TableModifyInfo
///////////////////

//	FUNCTION public
//	Admin::Mount::TableModifyInfo::TableModifyInfo -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table& cTable_
//			Tableのスキーマオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::TableModifyInfo::
TableModifyInfo(Schema::Table& cTable_)
	: m_Persist(true)
{
	m_Name = cTable_.getName();
	m_ID = cTable_.getID();
	m_vecPrevID = cTable_.getAreaID();
	m_vecPostID = cTable_.getAreaID();
	m_Object = &cTable_;
}

//	FUNCTION public
//	Admin::Mount::TableModifyInfo::TableModifyInfo -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Index& cIndex_
//			Indexのスキーマオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::TableModifyInfo::
TableModifyInfo(Schema::Index& cIndex_)
	: m_Persist(true)
{
	m_Name = cIndex_.getName();
	m_ID = cIndex_.getID();
	cIndex_.getAreaID(m_vecPrevID);
	cIndex_.getAreaID(m_vecPostID);
	m_Object = &cIndex_;
}

//	FUNCTION public
//	Admin::Mount::TableModifyInfo::pack -- 構造体の内容をDataArrayDataにする
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

Common::DataArrayData*
Mount::TableModifyInfo::
pack() const
{
	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;
	pResult->reserve(MemberNum);
	pResult->pushBack(new Common::StringData(m_Name));
	pResult->pushBack(new Common::UnsignedIntegerData(m_ID));
	pResult->pushBack(new Common::UnsignedIntegerArrayData(m_vecPrevID));
	pResult->pushBack(new Common::UnsignedIntegerArrayData(m_vecPostID));
	return pResult.release();
}

//	FUNCTION public
//	Admin::Mount::TableModifyInfo::unpack -- DataArrayDataから構造体の内容をセットする
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			内容の元になるデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mount::TableModifyInfo::
unpack(const Common::DataArrayData& cData_)
{
	if (cData_.getCount() != MemberNum) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	for (int i = 0; i < cData_.getCount(); ++i) {
		const Common::Data::Pointer& pElement = cData_.getElement(i);
		switch (i) {
		case 0:
			{
				if (pElement->getType() == Common::DataType::String) {
					m_Name = _SYDNEY_DYNAMIC_CAST(const Common::StringData&, *pElement).getValue();
					continue;
				}
				break;
			}
		case 1:
			{
				if (pElement->getType() == Common::DataType::UnsignedInteger) {
					m_ID = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&, *pElement).getValue();
					continue;
				}
				break;
			}
		case 2:
			{
				if (pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::UnsignedInteger) {
					m_vecPrevID = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&, *pElement).getValue();
					continue;
				}
				break;
			}
		case 3:
			{
				if (pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::UnsignedInteger) {
					m_vecPostID = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&, *pElement).getValue();
					continue;
				}
				break;
			}
		}
		// ここに来ることは失敗を意味する
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

//	FUNCTION public
//	Admin::Mount::TableModifyInfo::setAreaModifyInfo --
//		エリアの変更内容が適用されるか調べる
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<Admin::Mount::AreaModifyInfo>& vecAreaModifyInfo_
//			エリアの変更内容が記録された配列
//
//	RETURN
//		true .. 変更がある
//		false.. 変更がない
//
//	EXCEPTIONS

bool
Mount::TableModifyInfo::
setAreaModifyInfo(const ModVector<AreaModifyInfo>& vecAreaModifyInfo_)
{
	bool bResult = false;

	ModSize nArea = vecAreaModifyInfo_.getSize();
	ModSize n = m_vecPrevID.getSize();
	for (ModSize i = 0; i < n; ++i) {
		if (m_vecPrevID[i] != Schema::Object::ID::Invalid) {
			// DROPされるエリアにあるか調べる
			for (ModSize iArea = 0; iArea < nArea; ++iArea) {
				if (vecAreaModifyInfo_[iArea].m_eType == AreaModifyInfo::ModifyType::Drop
					&& vecAreaModifyInfo_[iArea].m_ID == m_vecPrevID[i]) {
					// DROPされるので変更後のエリアIDを変える
					m_vecPostID[i] = Schema::Object::ID::Invalid;
					bResult = true;
				}
			}
		}
	}
	return bResult;
}

//////////////////////////////////////////////////////////////
// Mount
//////////////////////////////////////////////////////////////

//	FUNCTION public
//	Admin::Mount::Mount -- マウントを表すクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& trans
//			マウントを行うトランザクションのトランザクション記述子
//		Schema::LogData&	logData
//			マウントを再実行するための論理ログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::Mount(Trans::Transaction& trans, const Schema::LogData& logData)
	: m_cTrans(trans),
	  m_pSession(0),
	  m_strDbName(Schema::Database::getName(logData)),
	  m_pDatabase(Schema::Database::createForMount(logData, trans)),
	  _option(Option::None),
	  _unmounted(Boolean::Unknown),
	  _dbRecovery(0),
	  _starting(logData.getTimeStamp()),
	  _notified(false)
{
	; _SYDNEY_ASSERT(
		logData.getSubCategory() == Schema::LogData::Category::Mount);

	// ログデータから情報を取り出す
	//
	//【注意】	ログデータにはデータベース用の情報も含まれているので、
	//			Schema::Database::Log::Create::Num から処理すること

	int i = Schema::Database::Log::Create::Num;
	unpackMetaField(logData[i++].get(), Utility::Meta::Mount::Option);
	unpackMetaField(logData[i++].get(), Utility::Meta::Mount::Unmounted);
	unpackMetaField(logData[i++].get(), Utility::Meta::Mount::AreaInfo);
	unpackMetaField(logData[i++].get(), Utility::Meta::Mount::TableInfo);
	unpackMetaField(logData[i++].get(), Utility::Meta::Mount::IndexInfo);
	if (logData.getCount() > Utility::Meta::Mount::MasterURL) {
		unpackMetaField(logData[i++].get(), Utility::Meta::Mount::MasterURL);
	}
}

//	FUNCTION public
//	Admin::Mount::Mount -- マウントを表すクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& trans
//			マウントを行うトランザクションのトランザクション記述子
//		const Schema::Database::Pointer&	database
//			マウントするデータベースのスキーマオブジェクト
//		Schema::LogData&	logData
//			マウントを再実行するための論理ログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::Mount(Trans::Transaction& trans,
			 const Schema::Database::Pointer& database, const Schema::LogData& logData)
	: m_cTrans(trans),
	  m_pSession(0),
	  m_strDbName(database->getName()),
	  m_pDatabase(database),
	  _option(Option::None),
	  _unmounted(Boolean::Unknown),
	  _dbRecovery(0),
	  _starting(logData.getTimeStamp()),
	  _notified(false)
{
#ifdef OBSOLETE // データベース単位のログはない
	if (logData.getSubCategory() == Schema::LogData::Category::Mount) {

		// ログデータから情報を取り出す
		//
		//【注意】	ログデータにはデータベース用の情報も含まれているので、
		//			Schema::Database::Log::Create::Num から処理すること

		int i = Schema::Database::Log::Create::Num;
		unpackMetaField(logData[i++].get(), Utility::Meta::Mount::Option);
		unpackMetaField(logData[i++].get(), Utility::Meta::Mount::Unmounted);
		unpackMetaField(logData[i++].get(), Utility::Meta::Mount::AreaInfo);
		unpackMetaField(logData[i++].get(), Utility::Meta::Mount::TableInfo);
		unpackMetaField(logData[i++].get(), Utility::Meta::Mount::IndexInfo);
	}
#else
	; _SYDNEY_ASSERT(
		logData.getSubCategory() != Schema::LogData::Category::Mount);
#endif
}

//	FUNCTION public
//	Admin::Mount::~Mount -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Mount::~Mount()
{
	if (_notified)

		// マウント中の回復処理が終了したことを通知する

		Schema::Manager::Recovery::notifyDone(*m_pDatabase), _notified = false;

	//【注意】	同時に、データベースはクローズされる

	delete _dbRecovery, _dbRecovery = 0;

	destructAreaModifyInfo();
}

//	FUNCTION public
//	Admin::Mount::execute -- データベースをマウントする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object*	pcStatement_
//			MOUNT 文を表すクラス
//
//	RETURN
//		マウントされたデータベースを表す
//		スキーマオブジェクトを格納する領域の先頭アドレス
//
//	EXCEPTIONS

Schema::Database*
Mount::execute(const Statement::MountDatabaseStatement* pcStatement_)
{
	; _SYDNEY_ASSERT(pcStatement_);

	// マウントオプションを調べる

	if (pcStatement_->isUsingSnapshot())
		_option |= Option::UsingSnapshot;
	if (pcStatement_->isWithRecovery())
		_option |= Option::WithRecovery;
	if (pcStatement_->isDiscardLogicalLog())
		_option |= Option::DiscardLogicalLog;

	// メタデータベースとデータベース表を
	// データベース表を操作するためにロックする

	Schema::Manager::SystemTable::hold(
		m_cTrans, Schema::Hold::Target::MetaDatabase,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadWrite);
	Schema::Manager::SystemTable::hold(
		m_cTrans, Schema::Hold::Target::MetaTable,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadWrite);

	// マウントするデータベースのスキーマオブジェクトを表すクラスを生成する

	m_pDatabase = Schema::Database::createForMount(
		m_strDbName, *(pcStatement_->getOptionList()), m_cTrans);

	if (!m_pDatabase.get()) {

		// すでに存在するデータベースと
		// 同じ名前でマウントしようとした

		SydErrorMessage << "Database " << m_strDbName << " is exist"
						<< ModEndl;
		_SYDNEY_THROW1(Exception::DatabaseAlreadyDefined, m_strDbName);
	}

	Schema::Manager::ObjectPath::AutoWithdraw w2(m_pDatabase.get());

	if (m_pDatabase->checkPath(m_cTrans, 0, 0, true /* allow existence */)) {
		// Another object using the path
		_SYDNEY_THROW1(Exception::InvalidPath, m_strDbName);
	}

	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	m_cTrans.setLog(*m_pDatabase.get());
	{
	//【注意】	ModAutoPointer<Schema::Database>::~ModAutoPointer より前に
	//			Schema::Manager::ObjectName::AutoWithdraw::~AutoWithdraw が
	//			呼び出される必要がある

	Schema::Manager::ObjectName::AutoWithdraw w(m_pDatabase.get());

	if (!Schema::SystemTable::isAvailable())

		// メタデータベースは利用不可である

		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(m_cTrans, *m_pDatabase, pcStatement_, m_pSession);

	if (Statement::Literal* pMasterUrl = pcStatement_->getMasterUrl()) {
		; _SYDNEY_ASSERT(pMasterUrl->isStringLiteral());

		Common::Data::Pointer pData = pMasterUrl->createData();
		; _SYDNEY_ASSERT(pData.get());

		// マスターデータベースに接続できるか確認する
		ModUnicodeString url = pData->getString();
		url = Replicator::checkConnectMaster(url);

		// set master url
		m_pDatabase->setMasterURL(url);
		// start slave
		m_pDatabase->setSlaveStarted(true);
	}

	// マウントするデータベースの情報を格納するデータベース表のタプルを
	// データベース表のタプルを操作するためにロックする

	Schema::Manager::SystemTable::hold(
		m_cTrans, Schema::Hold::Target::MetaTuple,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadWrite,
		m_pDatabase->getID());

	// データベース表のデータベース表を操作するためのロックを、
	// データベース表のタプルを操作するためのロックに変換する

	Schema::Manager::SystemTable::convert(
		m_cTrans, Schema::Hold::Target::MetaTable,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadWrite,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForWrite);

	// システム用の論理ログファイルを
	// データベース表のタプルを操作するためにロックする

	Schema::Manager::SystemTable::hold(
		m_cTrans, Schema::Hold::Target::LogicalLog,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadWrite);

	// マウント中の回復処理を開始することを通知する

	; _SYDNEY_ASSERT(!_notified);
	Schema::Manager::Recovery::notifyBegin(*m_pDatabase), _notified = true;

	// 回復処理を開始する時点のタイムスタンプを得る

	findStartLSN();

	if (_unmounted != Boolean::True && m_pDatabase->isReadOnly())

		// アンマウントされていないデータベースは
		// READ ONLY マウントできない

		_SYDNEY_THROW1(Exception::DatabaseNotMountable, m_strDbName);

	// マウントするデータベース用のシステム表を
	// 構成するファイルをマウントする

	m_pDatabase->mountSystemTable(m_cTrans);

	// エラー処理のためにマウント処理の進行状況を記憶する

	enum {
		MetaMounted,
		Mounted,
		DatabaseStored,
		Persisted
	} stat = MetaMounted;

	try {
		switch (_unmounted) {
		case Boolean::True:

			// アンマウントされたデータベースはなにもしない

			break;

		case Boolean::False:

			// バックアップされたデータベースをマウントするとき

			if (_option & Option::UsingSnapshot) {

				// バックアップしたトランザクションの開始時点に戻す

				m_pDatabase->restoreSystemTable(m_cTrans, _starting);
				break;
			}
			// thru

		case Boolean::Unknown:

			// アンマウントされたものでも、
			// バックアップされたものでもないデータベースをマウントするとき

			if (!_dbRecovery->_recovered)

				// マウントするデータベース用のシステム表を
				// 構成するファイルを回復処理の起点の状態に戻す

				m_pDatabase->recoverSystemTable(m_cTrans, _starting);
		}

		ADMIN_MOUNT_FAKE_ERROR("Execute", "Recovered");

		// エリア定義の変更内容を得る

		setAreaModifyInfo(pcStatement_);

		// 表と索引定義の変更内容を得る

		setTableModifyInfo();

		// マウントすることを表す論理ログを
		// システム用の論理ログファイルに記録する
		//
		//【注意】	この論理ログを記録した時間を使って、
		//			REDO 時に論理ログファイルを読み戻すので、
		//			findStartLSN で現在のタイムスタンプを修正後にする必要がある

		Schema::LogData logData(Schema::LogData::Category::Mount);
		makeLogData(logData);
		{
		const Lock::Name& lockName = m_cTrans.getLogInfo(
			Trans::Log::File::Category::System).getLockName();

		Trans::AutoLatch latch(m_cTrans, lockName);
		m_cTrans.storeLog(Trans::Log::File::Category::System, logData);
		}
		ADMIN_MOUNT_FAKE_ERROR("Execute", "Logged");

		// マウント時に指定されたエリア定義の変更指定を
		// エリアのスキーマオブジェクトに適用する

		applyAreaModifyInfo();
		ADMIN_MOUNT_FAKE_ERROR("Execute", "AreaApplied");

		// マウント時に指定された表と索引定義の変更指定を
		// 表、索引のスキーマオブジェクトに適用する

		applyTableModifyInfo();
		ADMIN_MOUNT_FAKE_ERROR("Execute", "TableApplied");

		// データベースを構成するファイルに対してマウントすることを通知する

		m_pDatabase->mount(m_cTrans);
		stat = Mounted;
		ADMIN_MOUNT_FAKE_ERROR("Execute", "Mounted");

		if (!m_pDatabase->isReadOnly()) {

			// 先ほど記録したものと同じものを
			// マウントするデータベース用の論理ログファイルに記録する
			//
			//【注意】	マウント後の論理ログファイルを使うために
			//			Schema::Database::mount より後に行う

			const Lock::Name& lockName = m_cTrans.getLogInfo(
				Trans::Log::File::Category::Database).getLockName();

			Trans::AutoLatch latch(m_cTrans, lockName);
			m_cTrans.storeLog(Trans::Log::File::Category::Database, logData);

			if (_option & Option::DiscardLogicalLog) {

				// 不要な論理ログの削除を指示
				// 実際には、トランザクションのコミット時に削除される

				m_cTrans.discardLog(true);
			}
			
			if (_dbRecovery->_slave)
			{
				// スレーブ処理終了のログを記録する
				// 起動時にこのログを参照し、トランザクションが復元される

				// ログデータを作成する
				
				Log::ReplicationEndData logData2;
				logData2.setBeginLSN(_dbRecovery->_runningLSN);
				logData2.setLastMasterLSN(_dbRecovery->_lastMasterLSN);

				// ログを記録する
				
				m_cTrans.storeLog(Trans::Log::File::Category::Database,
								  logData2, _dbRecovery->_lastMasterLSN);
			}
		}
		ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabaseLogged");
		ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabaseLogged_Fatal");

		// マウントするデータベースのスキーマオブジェクトを
		// スキーマデータベースの「データベース」表に登録する

		Schema::SystemTable::Database().store(m_cTrans, m_pDatabase);
		stat = DatabaseStored;

		ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabasePersisted");
		ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabasePersisted_Fatal");

		// ObjectIDを永続化する
		Schema::ObjectID::persist(m_cTrans, 0);

		stat = Persisted;
		ADMIN_MOUNT_FAKE_ERROR("Execute", "Persisted");
		ADMIN_MOUNT_FAKE_ERROR("Execute", "Persisted_Fatal");

		// 表、索引定義の変更を永続化する
		// このとき、ファイル、エリア格納関係の変更も永続化される

		persistTableModifyInfo();
		ADMIN_MOUNT_FAKE_ERROR("Execute", "TablePersisted");
		ADMIN_MOUNT_FAKE_ERROR("Execute", "TablePersisted_Fatal");

		// エリア定義の変更を永続化する
		// エリア格納関係の永続化の後でないと正しく永続化できない

		persistAreaModifyInfo();
		ADMIN_MOUNT_FAKE_ERROR("Execute", "AreaPersisted");
		ADMIN_MOUNT_FAKE_ERROR("Execute", "AreaPersisted_Fatal");

		// ObjectIDを永続化する
		Schema::ObjectID::persist(m_cTrans, m_pDatabase.get());

		// 回復処理を行う
		//
		//【注意】	回復処理中にマウントするデータベースの
		//			スキーマオブジェクトを取得しようとするので、
		//			「データベース」表に登録済である必要がある

		recover();
		ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabaseRecovered");
		ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabaseRecovered_Fatal");

	} catch (...) {

		_BEGIN_DB_REORGANIZE_RECOVERY;

		switch (stat) {
		case Persisted:

			// エリア定義の変更を取り消す
			//
			//【注意】	DROP の取り消しで CREATE されるので
			//			表と索引の変更の取り消しより前に行う必要がある

			applyAreaModifyInfo(true);

			// エリア定義の変更取り消しを永続化する
			// applyでエリアオブジェクトを使用するので
			// 永続化しないと使えない

			ADMIN_MOUNT_FAKE_ERROR("Execute", "AreaPersisted_Fatal");
			persistAreaModifyInfo(true);

			// 表と索引定義の変更を取り消す

			applyTableModifyInfo(true);

			// 表と索引定義の変更取り消しを永続化する

			ADMIN_MOUNT_FAKE_ERROR("Execute", "TablePersisted_Fatal");
			persistTableModifyInfo(true);

			// mountに使用したパスは変更後のものなので
			// 再びエリア、表、索引の変更を適用する
			// (永続化はしない)

			applyAreaModifyInfo();
			applyTableModifyInfo();

			// thru.

		case DatabaseStored:

			// データベースを構成するファイルをアンマウントする

			m_pDatabase->unmount(m_cTrans, true);

			// データベース表に登録した内容を削除する

			ADMIN_MOUNT_FAKE_ERROR("Execute", "Persisted_Fatal");
			Schema::SystemTable::Database().store(m_cTrans, m_pDatabase);

			// ObjectIDを永続化する
			ADMIN_MOUNT_FAKE_ERROR("Execute", "DatabasePersisted_Fatal");
			Schema::ObjectID::persist(m_cTrans, 0);
			break;

		case Mounted:

			// データベースを構成するファイルをアンマウントする

			m_pDatabase->unmount(m_cTrans, true);
			break;

		case MetaMounted:

			// マウントするデータベース用のシステム表を
			// 構成するファイルをアンマウントする

			m_pDatabase->unmountSystemTable(m_cTrans, true);
			break;
		}

		_END_DB_REORGANIZE_RECOVERY;

		_SYDNEY_RETHROW;
	}

	if (!m_pDatabase->isReadOnly())

		// マウントしたデータベースに対してチェックポイント処理を行う

		Checkpoint::Executor::cause(m_cTrans, *m_pDatabase);

	// 他のセッションのスキーマ情報のキャッシュを作り直させる

	Schema::Manager::SystemTable::addTimestamp(m_cTrans);

	if (m_pDatabase->isSlaveStarted())
	{
		// 非同期レプリケーションスレッドを起動する

		Replicator::start(m_cTrans, *m_pDatabase);
	}

	// マウントされたデータベースのスキーマオブジェクトを返す

	return m_pDatabase.get();
	}
}

//	FUNCTION public
//	Admin::Mount::redo -- マウント処理を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::LogData&	logData
//			REDO するマウント処理を表す論理ログデータ
//
//	RETURN
//		REDO した結果、マウントされたデータベースを
//		復旧するためのオブジェクトを格納する領域の先頭アドレス
//
//	EXCEPTIONS

Recovery::Database*
Mount::redo(const Schema::LogData& logData)
{
	// マウントするデータベースの
	// スキーマオブジェクトを表すクラスは生成されている必要がある

	; _SYDNEY_ASSERT(m_pDatabase.get());

	// 回復処理を開始する時点はすでに得られている必要がある

	; _SYDNEY_ASSERT(!_starting.isIllegal());

	//【注意】	ModAutoPointer<Schema::Database>::~ModAutoPointer より前に
	//			Schema::Manager::ObjectName::AutoWithdraw::~AutoWithdraw が
	//			呼び出される必要がある

	Schema::Manager::ObjectName::AutoWithdraw w(m_pDatabase.get());
	Schema::Manager::ObjectPath::AutoWithdraw w2(m_pDatabase.get());

	// 回復処理を開始する時点のタイムスタンプを得る

	findStartLSN();

	// マウントするデータベース用のシステム表を
	// 構成するファイルをマウントする

	m_pDatabase->mountSystemTable(m_cTrans);

	// エラー処理のためにマウント処理の進行状況を記憶する

	enum {
		MetaMounted,
		Mounted
	} stat = MetaMounted;

	try {
		if (!m_pDatabase->isReadOnly() && !_dbRecovery->_recovered)

			// マウントするデータベース用のシステム表を
			// 構成するファイルを回復処理の起点の状態に戻す

			m_pDatabase->recoverSystemTable(m_cTrans, _starting);

		// マウントするデータベースのオブジェクトが登録される前に
		// マウント処理に必要なスキーマオブジェクトを
		// 特別なモードで読み込んでおく

		preloadAreaObject();
		preloadTableObject();

		// データベースを構成するファイルに対してマウントすることを通知する

		m_pDatabase->mount(m_cTrans);
		stat = Mounted;

		if (!m_pDatabase->isReadOnly() && !_dbRecovery->_recovered) {

			// データベースを構成するファイルのうち、
			// システム表を構成するもの以外を回復処理の起点の状態に戻す

			m_pDatabase->recover(m_cTrans, _starting, true);
			_dbRecovery->_recovered = true;
		}

		// マウントするデータベースのスキーマオブジェクトを
		// スキーマデータベースの「データベース」表に登録する

		Schema::SystemTable::Database().store(m_cTrans, m_pDatabase);

		// ObjectIDを永続化する
		Schema::ObjectID::persist(m_cTrans, 0);

	} catch (...) {

		switch (stat) {
		case Mounted:

			// データベースを構成するファイルをアンマウントする

			m_pDatabase->unmount(m_cTrans, true);
			break;

		case MetaMounted:

			// マウントするデータベース用のシステム表を
			// 構成するファイルをアンマウントする

			m_pDatabase->unmountSystemTable(m_cTrans, true);
			break;
		}
		_SYDNEY_RETHROW;
	}

	Recovery::Database* p = 0;
	ModSwap(_dbRecovery, p);
	return p;
}

#ifdef OBSOLETE // データベース単位のUNDO処理は呼ばれない
//	FUNCTION public
//	Admin::Mount::undo
//		-- MOUNT 処理をUNDOする
//
//	NOTES
//		LogDataを引数にコンストラクトしたオブジェクトを使用する
//
//	ARGUMENTS
//		bool redone_
//			後でredoされるかを示す
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
undo(bool redone_)
{
	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*m_pDatabase);

	// AreaのUNDO処理を行う
	undoAreaModifyInfo(redone_);
	// Table以下のUNDO処理を行う
	undoTableModifyInfo(redone_);
}
#endif

//	FUNCTION private
//	Admin::Mount::checkAreaPath
//		-- SQL 文の Area 変更リストに使用されているパスが使われているかチェックする
//
//	NOTES
//
//		注：Schema も同等の関数を持っているが、公開されていない。
//
//	ARGUMENTS
//		Utility::PathList& cPathList
//			使用されるパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
Mount::
checkAreaPath()
{
	ModVector<AreaModifyInfo>::ConstIterator iterator = m_vecAreaModifyInfo.begin();
	const ModVector<AreaModifyInfo>::ConstIterator& end = m_vecAreaModifyInfo.end();

	// チェック済みのパスを記録し、Modify指定内でぶつかるパスがないかも同時に調べる
	ModVector<ModUnicodeString> vecCheckedPath;
	for (; iterator != end; ++iterator) {
		// ALTER AREAに関する変更のときのみ調べればよい
		if ((*iterator).m_eType == AreaModifyInfo::ModifyType::Alter) {
			const ModVector<ModUnicodeString>& vecPath = (*iterator).m_vecPostPath;
			if (!vecPath.isEmpty()) {
				// IDからエリアオブジェクトを得る
				Schema::Area* pArea = Schema::Area::get((*iterator).m_ID, m_pDatabase.get(), m_cTrans);
				// 移動後のパスを渡して検査する
				if (pArea->checkPath(m_cTrans, &vecPath, false /* recovery */, true /* mount */)) {
					// 重複がある
					// ★注意★
					// 移動後のパスが存在していないときもエラーだが
					// この場合はcheckPathが例外を発生している
					// 分かりにくいと思うがパスが重複するときの例外の種類が
					// 再構成時とMOUNTで異なるのと、エラーコードのようなものを
					// 入れるのに抵抗があったのでこのようになっている

					//【注意】	どのパスが重複したかわからないので、
					//			Os::Path() を与える

					_SYDNEY_THROW1(Exception::FileAlreadyExisted, Os::Path());
				}
				// これまで調べたパスとのバッティングを調べる
				ModSize nChecked = vecCheckedPath.getSize();
				if (nChecked) {
					ModSize n = vecPath.getSize();
					for (ModSize i = 0; i < n; ++i) {
						for (ModSize iChecked = 0; iChecked < nChecked; ++iChecked) {
							if (Os::Path::compare(vecCheckedPath[iChecked], vecPath[i])
								!= Os::Path::CompareResult::Unrelated) {
								// バッティングしている
								_SYDNEY_THROW1(
									Exception::FileAlreadyExisted, vecPath[i]);
							}
						}
					}
				}
				// 重複がなかったのでチェック済みに加える
				vecCheckedPath.insert(vecCheckedPath.end(),
									  vecPath.begin(), vecPath.end());
			}
		}
	}
}

//	FUNCTION private
//	Admin::Mount::findStartLSN -- 回復処理を開始する時点のタイムスタンプを得る
//
//	NOTES
//		現在のタイムスタンプを適正な値に設定する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mount::findStartLSN()
{
	; _SYDNEY_ASSERT(m_pDatabase.get());
	; _SYDNEY_ASSERT(!_dbRecovery);

	// マウントするデータベースの回復処理を行うためのクラスを生成する
	//
	//【注意】	同時に、 キャッシュが破棄されないように
	//			データベースをオープンされる

	Trans::Log::AutoFile logFile(m_pDatabase->getLogFile(true));
	_dbRecovery = new Recovery::Database(*m_pDatabase, _starting);
	; _SYDNEY_ASSERT(_dbRecovery);

	// マウント処理なので、フラグを立てる
	
	_dbRecovery->_mount = true;

	// マウントするデータベース用の論理ログファイルは
	// マウントされていないので、調べる間だけ一時的にマウントする

	logFile->mount();

	Trans::TimeStamp	mostRecent;

	try {
		// マウントするデータベース用の論理ログファイルを調べて、
		// データベースの回復処理を開始する時点のタイムスタンプを得る

		if (_starting.isIllegal()) {

			// 通常時のとき

			Boolean::Value unmounted = Boolean::Unknown;
			Boolean::Value backup = Boolean::Unknown;

			_dbRecovery->findStartLSN(
				m_cTrans, _option, unmounted, backup, mostRecent);
			; _SYDNEY_ASSERT(!mostRecent.isIllegal());

			_unmounted =
				(unmounted == Boolean::True) ? Boolean::True :
				(backup == Boolean::True) ? Boolean::False : Boolean::Unknown;
		} else {

			// REDO のとき
			//
			//【注意】	読み取り専用データベースのマウントを
			//			REDO するときは回復処理は必要ないが、
			//			タイムスタンプを設定しなおす必要がある可能性がある

			_dbRecovery->findStartLSN(m_cTrans);
			mostRecent = _dbRecovery->_starting;
		}
	} catch (...) {

		logFile->unmount();
		_SYDNEY_RETHROW;
	}

	// マウントするデータベースの論理ログファイルはアンマウントしておく
	//
	//【注意】	ここでアンマウントしておかないと、
	//			Schema::Database::mount を呼び出す前にエラーが起きると、
	//			論理ログファイルがマウントされたままになってしまう

	logFile->unmount();

	// このとき、回復処理を開始する時点のタイムスタンプとして
	// 以下のタイムスタンプを得ている
	//
	// 通常時のとき
	//		アンマウントされたデータベースをマウントしようとしているとき
	//			アンマウントしたトランザクションの
	//			コミットを表す論理ログを記録したときのタイムスタンプ
	//		バックアップを開始したトランザクションの開始時点の状態に
	//		データベースをマウントしようとしているとき
	//			バックアップを開始したトランザクションの開始時タイムスタンプ
	//		バックアップの開始時点の状態に
	//		データベースをマウントしようとしているとき
	//			バックアップの開始を表す論理ログを記録したときのタイムスタンプ
	//		上記以外のデータベースを WITH RECOVERY 指定ありで
	//		マウントしようとしているとき
	//			そのデータベースに対する前々回のチェックポイント処理が
	//			終了したときのタイムスタンプ
	//
	// REDO 時のとき
	//		データベースをマウント後にそのデータベースに対する
	//		前々回のチェックポイント処理が終了したときのタイムスタンプ

	_starting = _dbRecovery->_starting;
	; _SYDNEY_ASSERT(!_starting.isIllegal());

	// 回復処理を開始する時点のタイムスタンプを現在のタイムスタンプとする
	//
	//【注意】	論理ログを記録しないようにする

	Trans::TimeStamp::assign(_starting, true);

	const Lock::LogicalLogName& lockName = logFile->getLockName();

	if (Checkpoint::TimeStamp::getMostRecent() < mostRecent)

		// システムの前回のチェックポイントより
		// 論理ログファイルに記録されている
		// マウントするデータベースの前回のチェックポイントは後なので、
		// マウントするデータベースの前回のチェックポイントとして設定する

		Checkpoint::TimeStamp::assign(lockName, mostRecent, true);

	else if (Checkpoint::TimeStamp::getSecondMostRecent() < mostRecent) {

		// システムの前々回のチェックポイントより
		// 論理ログファイルに記録されている
		// マウントするデータベースの前回のチェックポイントは後なので、
		// マウントするデータベースの前々回のチェックポイントとして設定する

		Checkpoint::TimeStamp::assign(lockName, mostRecent, false);
		Checkpoint::TimeStamp::assign(
			lockName, Checkpoint::TimeStamp::getMostRecent(), false);
	}
}

//	FUNCTION private
//	Admin::Mount::recover -- データベースに回復処理を行う
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

void
Mount::recover()
{
	; _SYDNEY_ASSERT(m_pDatabase.get());
	; _SYDNEY_ASSERT(!_starting.isIllegal());

	switch (_unmounted) {
	case Boolean::True:

		// アンマウントされたデータベースはなにもしない

		break;

	case Boolean::False:

		// バックアップされたデータベースをマウントするとき

		if (_option & Option::UsingSnapshot) {

			// バックアップしたトランザクションの開始時点の状態に回復する

			m_pDatabase->restore(m_cTrans, _starting);
			break;
		}
		// thru

	case Boolean::Unknown:
		if (!_dbRecovery->_recovered) {

			// データベースを構成するファイルのうち、
			// システム表を構成するもの以外を回復処理の起点の状態に戻す

			m_pDatabase->recover(m_cTrans, _starting, true);
			_dbRecovery->_recovered = true;
		}


		if (_unmounted == Boolean::Unknown)
			
			// ロールフォワードのための処理である

			_dbRecovery->_rollforward = true;

		// データベースに対して、
		// その時点に実行中のトランザクションのうち、
		// バックアップ開始時までに
		// コミットされなかったものによる操作を UNDO する

		_dbRecovery->undoAll(m_cTrans);

		// データベースに対して、
		// その時点からバックアップの開始時までに
		// コミットされたトランザクションによる操作を REDO する

		_dbRecovery->redoAll(m_cTrans);
	}
}

//	FUNCTION private
//	Admin::Mount::makeLogData -- MOUNT 文用のログデータを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::LogData&	logData
//			作成されるログデータ
//		Schema::Database&	database
//			マウントするデータベースのスキーマオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mount::makeLogData(Schema::LogData& logData)
{
	; _SYDNEY_ASSERT(m_pDatabase.get());

	// データベースに関する情報を作成する

	m_pDatabase->makeLogData(m_cTrans, logData);

	// マウント独自の情報を作成する

	logData.addData(packMetaField(Utility::Meta::Mount::Option));
	logData.addData(packMetaField(Utility::Meta::Mount::Unmounted));
	logData.addData(packMetaField(Utility::Meta::Mount::AreaInfo));
	logData.addData(packMetaField(Utility::Meta::Mount::TableInfo));
	logData.addData(packMetaField(Utility::Meta::Mount::IndexInfo));
	logData.addData(packMetaField(Utility::Meta::Mount::MasterURL));
}

//	FUNCTION private
//	Admin::Mount::packMetaField --
//		MOUNT 文用のログデータのある種別の情報を作成する
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Utility::Meta::Mount::Type	type
//			作成したい情報の種別
//
//	RETURN
//		0 以外の値
//			作成された情報のオブジェクトポインタ
//		0
//			作成できなかった
//
//	EXCEPTIONS

Common::Data::Pointer
Mount::packMetaField(Utility::Meta::Mount::Type type) const
{
	switch (type) {
	case Utility::Meta::Mount::Option:

		// マウントオプション

		return new Common::UnsignedIntegerData(_option);

	case Utility::Meta::Mount::Unmounted:

		// アンマウントされたデータベースをマウントするか

		return new Common::UnsignedIntegerData(_unmounted);

	case Utility::Meta::Mount::AreaInfo:
	{
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();

		ModVector<AreaModifyInfo>::ConstIterator it = m_vecAreaModifyInfo.begin();
		const ModVector<AreaModifyInfo>::ConstIterator& fin = m_vecAreaModifyInfo.end();
		
		// AreaModifyInfo の要素分確保する
		pData->reserve(m_vecAreaModifyInfo.getSize());

		for ( ; it != fin; ++it ) {
			// AreaModifyInfoをDataArrayDataにしてセットする
			pData->pushBack((*it).pack());
		}

		return pData.release();
	}
	case Utility::Meta::Mount::TableInfo:
	{
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();

		ModVector<TableModifyInfo>::ConstIterator it = m_vecTableModifyInfo.begin();
		const ModVector<TableModifyInfo>::ConstIterator& fin = m_vecTableModifyInfo.end();
		
		// TableModifyInfo の要素分確保する
		pData->reserve(m_vecTableModifyInfo.getSize());

		for ( ; it != fin; ++it ) {
			// TableModifyInfoをDataArrayDataにしてセットする
			pData->pushBack((*it).pack());
		}

		return pData.release();
	}
	case Utility::Meta::Mount::IndexInfo:
	{
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();

		ModVector<IndexModifyInfo>::ConstIterator it = m_vecIndexModifyInfo.begin();
		const ModVector<IndexModifyInfo>::ConstIterator& fin = m_vecIndexModifyInfo.end();
		
		// IndexModifyInfo の要素分確保する
		pData->reserve(m_vecIndexModifyInfo.getSize());

		for ( ; it != fin; ++it ) {
			// IndexModifyInfoをDataArrayDataにしてセットする
			pData->pushBack((*it).pack());
		}

		return pData.release();
	}
	case Utility::Meta::Mount::MasterURL:
	{
		// Master URL
		return new Common::StringData(m_pDatabase->getMasterURL());		
	}
	}
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Admin::Mount::unpackMetaField --
//		MOUNT 文用のログデータのある種別の情報の値を得る
//
//	NOTES
//
//	ARGUMENTS
//		Common::Data*		data
//			ログ情報の要素
//		Admin::Utility::Meta::Mount::Type	type
//			値を得たい情報の種別
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		LogItemCorrupted
//			ログデータがおかしいため、値が得られなかった

void
Mount::unpackMetaField(const Common::Data* data,
					   Utility::Meta::Mount::Type type)
{
	if (data && data->getType() == getMetaFieldType(type))
		switch (type) {
		case Utility::Meta::Mount::Option:
			_option = _SYDNEY_DYNAMIC_CAST(
				const Common::UnsignedIntegerData*, data)->getValue();
			break;

		case Utility::Meta::Mount::Unmounted:
			_unmounted =
				static_cast<Boolean::Value>(
					_SYDNEY_DYNAMIC_CAST(
						const Common::UnsignedIntegerData*, data)->getValue());
			break;

		case Utility::Meta::Mount::AreaInfo:
		{
			const Common::DataArrayData* array =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, data);
			; _SYDNEY_ASSERT(array);
			
			const int n = array->getCount();
			m_vecAreaModifyInfo.reserve(n);

			for (int i = 0; i < n; ++i) {
				const Common::DataArrayData::Pointer&
					elm = array->getElement(i);
				
				if (elm->getType() != Common::DataType::Array ||
					elm->getElementType() != Common::DataType::Data)
					goto broken;

				AreaModifyInfo info;
				info.unpack(_SYDNEY_DYNAMIC_CAST(
								const Common::DataArrayData&, *elm));
				m_vecAreaModifyInfo.pushBack(info);
			}
			break;
		}
		case Utility::Meta::Mount::TableInfo:
		{
			const Common::DataArrayData* array =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, data);
			; _SYDNEY_ASSERT(array);
			
			const int n = array->getCount();
			m_vecTableModifyInfo.reserve(n);

			for (int i = 0; i < n; ++i) {
				const Common::DataArrayData::Pointer&
					elm = array->getElement(i);
				
				if (elm->getType() != Common::DataType::Array ||
					elm->getElementType() != Common::DataType::Data)
					goto broken;

				TableModifyInfo info;
				info.unpack(_SYDNEY_DYNAMIC_CAST(
								const Common::DataArrayData&, *elm));
				m_vecTableModifyInfo.pushBack(info);
			}
			break;
		}
		case Utility::Meta::Mount::IndexInfo:
		{
			const Common::DataArrayData* array =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, data);
			; _SYDNEY_ASSERT(array);
			
			const int n = array->getCount();
			m_vecIndexModifyInfo.reserve(n);

			for (int i = 0; i < n; ++i) {
				const Common::DataArrayData::Pointer&
					elm = array->getElement(i);
				
				if (elm->getType() != Common::DataType::Array ||
					elm->getElementType() != Common::DataType::Data)
					goto broken;

				IndexModifyInfo info;
				info.unpack(_SYDNEY_DYNAMIC_CAST(
								const Common::DataArrayData&, *elm));
				m_vecIndexModifyInfo.pushBack(info);
			}
			break;
		}
		case Utility::Meta::Mount::MasterURL:
		{
			; _SYDNEY_ASSERT(m_pDatabase.get());
			m_pDatabase->setMasterURL(data->getString());
			break;
		}
		}

	return;
broken:
	_SYDNEY_THROW0(Exception::LogItemCorrupted);
}

//	FUNCTION private
//	Admin::Mount::getMetaFieldType --
//		MOUNT 文用のログデータのある種別の情報の型を得る
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Utility::Meta::Mount::Type	type
//			型を得たい情報の種別
//
//	RETURN
//		得られた型
//
//	EXCEPTIONS
//		なし

// static
Common::DataType::Type
Mount::getMetaFieldType(Utility::Meta::Mount::Type type)
{
	switch (type) {
	case Utility::Meta::Mount::Option:
	case Utility::Meta::Mount::Unmounted:
		return Common::DataType::UnsignedInteger;
	case Utility::Meta::Mount::AreaInfo:
	case Utility::Meta::Mount::TableInfo:
	case Utility::Meta::Mount::IndexInfo:
		return Common::DataType::Array;
	case Utility::Meta::Mount::MasterURL:
		return Common::DataType::String;
	}
	return Common::DataType::Undefined;
}

//
//	FUNCTION private
//	Admin::Mount::setAreaModifyInfo -- Area の定義変更内容をメンバーにセットする
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::MountDatabaseStatement* pcStatement_
//			処理対象のMOUNT文
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

void
Mount::
setAreaModifyInfo(const Statement::MountDatabaseStatement* pMountStatement_)
{
	using namespace Statement;

	// マウントするデータベースの「エリア」表を読み込んでおく
	// 引数のtrueは読み込んだオブジェクトのメンバーとして
	// データベースオブジェクトを設定することを意味する
	const Schema::AreaMap& cAreas = m_pDatabase->loadArea(m_cTrans, true /* recovery */);
	{
		// AreaContentも読み込んでおく
		Schema::AreaMap::ConstIterator iterator = cAreas.begin();
		const Schema::AreaMap::ConstIterator& end = cAreas.end();
		for (; iterator != end; ++iterator) {
			(void)Schema::AreaMap::getValue(iterator)->loadContent(m_cTrans, true /* recovery */);
		}
	}

	// Areaの定義変更を記録するVectorを初期化する
	m_vecAreaModifyInfo.clear();

	// Areaの定義変更を表す構文要素があればそれにしたがって処理する
	OptionalAreaParameter* pAreaParam = pMountStatement_->getAreaParameter();
	if (pAreaParam) {
		switch (pAreaParam->getOptionType()) {
		case OptionalAreaParameter::ParameterList:
			{
				// Drop or Alter
				OptionalAreaParameterList* pStatementList
					= _SYDNEY_DYNAMIC_CAST(OptionalAreaParameterList*, pAreaParam->getOption());
				; _SYDNEY_ASSERT(pStatementList);
				int n = pStatementList->getCount();
				for (int i = 0; i < n; ++i) {
					Statement::Object* pObject = pStatementList->getAt(i);
					; _SYDNEY_ASSERT(pObject);
					; _SYDNEY_ASSERT((pObject->getType() == ObjectType::AlterAreaStatement)
									 || (pObject->getType() == ObjectType::DropAreaStatement));

					m_vecAreaModifyInfo.pushBack(AreaModifyInfo(*pObject));
					// エリアの情報をセットする
					AreaModifyInfo& cEntry = m_vecAreaModifyInfo.getBack();
					cEntry.setAreaData(m_cTrans, *m_pDatabase);
					// ALTER文のとき変更後のパスを入れる
					cEntry.setStatementData(*pObject);
				}
				break;
			}
		case OptionalAreaParameter::DropAllArea:
			{
				// Drop all
				// すべてのエリアについてDROPの情報をセットする

				m_vecAreaModifyInfo.reserve(cAreas.getSize());
				Schema::AreaMap::ConstIterator iterator = cAreas.begin();
				const Schema::AreaMap::ConstIterator& end = cAreas.end();
				for (; iterator != end; ++iterator) {
					m_vecAreaModifyInfo.pushBack(AreaModifyInfo(*Schema::AreaMap::getValue(iterator)));
				}

				break;
			}
		default:
			; _SYDNEY_ASSERT(false);
		}
	}

	// 変更後のパスが他のデータベースやエリアと重なっていないか調べる

	checkAreaPath();
}

//	FUNCTION private
//	Admin::Mount::applyAreaModifyInfo -- Area の定義変更内容をAreaオブジェクトに反映する
//
//	NOTES
//		ここではまだ永続化されない
//
//	ARGUMENTS
//		bool bUndo_ = false
//			永続化後のエラー処理で元に戻すための呼び出しならtrue
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

void
Mount::
applyAreaModifyInfo(bool bUndo_ /* = false */)
{
	using namespace Statement;

	// Area定義変更構造体の内容にしたがって反映する
	ModSize n = m_vecAreaModifyInfo.getSize();
	for (ModSize i = 0; i < n; ++i) {
		const AreaModifyInfo& cInfo = m_vecAreaModifyInfo[i];
		switch (cInfo.m_eType) {
		case AreaModifyInfo::ModifyType::Alter:
			{
				// ALTER AREA
				Schema::Area* pArea = (cInfo.m_Object)
					? cInfo.m_Object
					: (cInfo.m_Object = m_pDatabase->getArea(cInfo.m_ID, m_cTrans));
				; _SYDNEY_ASSERT(pArea);
				// ファイルは移動せずにパス指定だけ変更する
				if (!bUndo_) {
					// 通常処理
					pArea->moveForMount(m_cTrans, cInfo.m_vecPrevPath, cInfo.m_vecPostPath);
				} else {
					// エラー処理
					pArea->moveForMount(m_cTrans, cInfo.m_vecPostPath, cInfo.m_vecPrevPath, true /* undo */);
					if (!cInfo.m_Persist) {
						// 永続化前ならエリアの状態を変更なしに戻す
						pArea->untouch();
					}
				}
				// 状態の変更はmoveForMountの中で行われている
				cInfo.m_Persist = !cInfo.m_Persist; // 永続化が必要かどうかのフラグを反転する
				break;
			}
		case AreaModifyInfo::ModifyType::Drop:
			{
				// DROP AREA
				if (!bUndo_) {
					// 通常処理
					Schema::Area* pArea = (cInfo.m_Object)
						? cInfo.m_Object
						: (cInfo.m_Object = m_pDatabase->getArea(cInfo.m_ID, m_cTrans));
					; _SYDNEY_ASSERT(pArea);
					// エリアに破棄マークをつける
					pArea->dropForMount(m_cTrans);
				} else {
					// エラー処理
					if (!cInfo.m_Persist) {
						// まだ永続化していないので削除を取り消す
						; _SYDNEY_ASSERT(cInfo.m_Object);
						cInfo.m_Object->undoDrop();
					} else {
						// すでに削除を永続化してしまっていたので作り直す
						; _SYDNEY_ASSERT(!cInfo.m_Object);
						ModAutoPointer<Schema::Area> pArea =
							Schema::Area::createForMount(m_cTrans, *m_pDatabase, cInfo.m_ID, cInfo.m_Name, cInfo.m_vecPrevPath);
						// 以前はここでIDの変換が必要なものとして登録していたが
						// 同じIDを用いて作り直すので変換は不要
						cInfo.m_NeedToDelete = true;
						cInfo.m_Object = pArea.release();
					}
				}
				// 状態の変更はdropやcreateで行われている
				cInfo.m_Persist = !cInfo.m_Persist; // 永続化が必要かどうかのフラグを反転する

				break;
			}
		default:
			; _SYDNEY_ASSERT(false);
		}
	}
}

//	FUNCTION private
//	Admin::Mount::persistAreaModifyInfo -- Areaを永続化する
//
//	NOTES
//
//	ARGUMENTS
//		bool bUndo_ = false
//			永続化後のエラー処理で元に戻すための呼び出しならtrue
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

void
Mount::
persistAreaModifyInfo(bool bUndo_ /* = false */)
{
	; _SYDNEY_ASSERT(m_pDatabase.get());

	ModSize n = m_vecAreaModifyInfo.getSize();
	for (ModSize i = 0; i < n; ++i) {
		const AreaModifyInfo& cInfo = m_vecAreaModifyInfo[i];
		if (!cInfo.m_Persist) {
			// 永続化が必要なオブジェクトについて永続化する
			if (cInfo.m_NeedToDelete) {
				// Adminモジュール内で作成したオブジェクトならArea::Pointerにして渡す
				Schema::SystemTable::Area(*m_pDatabase).store(m_cTrans, Schema::Area::Pointer(cInfo.m_Object));
				// オブジェクトの消滅管理はSchemaに移動した
				cInfo.m_NeedToDelete = false;
				cInfo.m_Object = 0;
			} else {
				Schema::SystemTable::Area(*m_pDatabase).store(m_cTrans, *cInfo.m_Object);
				if (cInfo.m_eType == AreaModifyInfo::ModifyType::Drop)
					cInfo.m_Object = 0;
			}
			cInfo.m_Persist = true;
		}
	}
}

#ifdef OBSOLETE // データベース単位のUNDOは呼ばれない
//	FUNCTION private
//	Admin::Mount::undoAreaModifyInfo -- Areaに関するUNDO処理をする
//
//	NOTES
//
//	ARGUMENTS
//		bool redone_
//			後でREDOされるかを示す
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
undoAreaModifyInfo(bool redone_)
{
	ModSize n = m_vecAreaModifyInfo.getSize();
	for (ModSize i = 0; i < n; ++i) {
		const AreaModifyInfo& cInfo = m_vecAreaModifyInfo[i];
		switch (cInfo.m_eType) {
		case AreaModifyInfo::ModifyType::Alter:
			{
				// AlterのUNDO処理のうちファイルの移動や破棄を伴わない処理を行う
				Schema::Manager::SystemTable::UndoUtility::
					undoAlterArea(m_cTrans, cInfo.m_ID, cInfo.m_vecPrevPath, cInfo.m_vecPostPath,
								  m_strDbName, redone_, true /* mount */);
				break;
			}
		case AreaModifyInfo::ModifyType::Drop:
			{
				// DropのUNDO処理のうちファイルの移動や破棄を伴わない処理を行う
				Schema::Manager::SystemTable::UndoUtility::
					undoDropArea(m_cTrans, cInfo.m_ID, cInfo.m_vecPrevPath,
								  m_strDbName, redone_);
				break;
			}
		}
	}
}
#endif

#ifdef OBSOLETE
//	FUNCTION private
//	Admin::Mount::redoAreaModifyInfo -- Areaに関するREDO処理をする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
redoAreaModifyInfo()
{
	// エリアに関するスキーマオブジェクトを読み込んでおく
	preloadAreaObject();
	ModSize n = m_vecAreaModifyInfo.getSize();
	for (ModSize i = 0; i < n; ++i) {
		const AreaModifyInfo& cInfo = m_vecAreaModifyInfo[i];
		// Areaオブジェクトを得る
		cInfo.m_Object = m_pDatabase->getArea(cInfo.m_ID, m_cTrans);
		if (!cInfo.m_Object) {
			_SYDNEY_THROW2(Exception::AreaNotFound, cInfo.m_Name, m_pDatabase->getName());
		}
		switch (cInfo.m_eType) {
		case AreaModifyInfo::ModifyType::Alter:
			{
				// AlterのREDO処理のうちファイルの移動や破棄を伴わない処理を行う
				Schema::Manager::SystemTable::RedoUtility::
					alterArea(m_cTrans, cInfo.m_ID, cInfo.m_vecPrevPath, cInfo.m_vecPostPath,
							  m_strDbName, *cInfo.m_Object, true /* mount */);
				cInfo.m_Persist = false;
				break;
			}
		case AreaModifyInfo::ModifyType::Drop:
			{
				// DropのREDO処理のうちファイルの移動や破棄を伴わない処理を行う
				Schema::Manager::SystemTable::RedoUtility::
					dropArea(m_cTrans, cInfo.m_ID,
							 m_strDbName, m_pDatabase.get(), true /* mount */);
				cInfo.m_Persist = false;
				break;
			}
		}
	}
}
#endif

//	FUNCTION private
//	Admin::Mount::destructAreaModifyInfo -- Areaの変更内容を表す構造体の解放処理をする
//
//	NOTES
//		AreaModifyInfoのデストラクターを使おうとすると
//		AreaModifyInfoをExportしなければならなくなるため、内部で解放処理を行う
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
destructAreaModifyInfo()
{
	ModSize n = m_vecAreaModifyInfo.getSize();
	for (ModSize i = 0; i < n; ++i) {
		const AreaModifyInfo& cInfo = m_vecAreaModifyInfo[i];
		if (cInfo.m_NeedToDelete) {
			delete cInfo.m_Object, cInfo.m_Object = 0;
		}
	}
}

//	FUNCTION private
//	Admin::Mount::preloadAreaObject -- Areaに関するスキーマオブジェクトを読み込んでおく
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
preloadAreaObject()
{
	const Schema::AreaMap& cAreas = m_pDatabase->loadArea(m_cTrans, true /* recovery */);
	{
		// AreaContentも読み込んでおく
		Schema::AreaMap::ConstIterator iterator = cAreas.begin();
		const Schema::AreaMap::ConstIterator& end = cAreas.end();
		for (; iterator != end; ++iterator) {
			(void)Schema::AreaMap::getValue(iterator)->loadContent(m_cTrans, true /* recovery */);
		}
	}
}

//
//	FUNCTION private
//	Admin::Mount::setTableModifyInfo -- Table以下の定義変更内容をメンバーにセットする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

void
Mount::
setTableModifyInfo()
{
	// TableとIndexの定義変更を記録するVectorを初期化する
	m_vecTableModifyInfo.clear();
	m_vecIndexModifyInfo.clear();

	// マウントするデータベースの「表」表を読み込んでおく
	// 引数のtrueは読み込んだオブジェクトのメンバーとして
	// データベースオブジェクトを設定することを意味する

	const Schema::TableMap& cTables =
		m_pDatabase->loadTable(m_cTrans, true /* recovery */);

	// すべての表と索引について、
	// 割り当てられているエリアがDROP操作対象だったら
	// その割り当てを落とす
	Schema::TableMap::ConstIterator iterator = cTables.begin();
	const Schema::TableMap::ConstIterator& end = cTables.end();
	for (; iterator != end; ++iterator) {
		Schema::Table::Pointer pTable =
			Schema::TableMap::getValue(iterator);

		// 表に対応したTableModifyInfoを作る
		TableModifyInfo tableInfo(*pTable);
		// 表に割り当てられているエリアがDROP対象かを調べて変更後のエリアとしてセットする
		if (tableInfo.setAreaModifyInfo(m_vecAreaModifyInfo))
			// 変更があるなら変更内容として加える
			m_vecTableModifyInfo.pushBack(tableInfo);

// ファイル名はシステム表から読み込まれるようになったので「列」表を読み込む必要はない
//		// ヒープファイル名の作成に使用されるので「列」表も読み込んでおく
//		(void)pTable->loadColumn(m_cTrans, true /* recovery */);

		// 「索引」表も読み込んでおく
		const Schema::IndexMap& cIndexMap =
			pTable->loadIndex(m_cTrans, true /* recovery */);

		// ファイルの定義も変更する可能性があるので「ファイル」表も読み込んでおく
		(void)pTable->loadFile(m_cTrans, true /* recovery */);
										// ファイルは索引オブジェクトを参照することがあるので
										// loadIndexの後でなければならない

		Schema::IndexMap::ConstIterator
			indexIterator = cIndexMap.begin();
		const Schema::IndexMap::ConstIterator&
			indexEnd = cIndexMap.end();

		for (; indexIterator != indexEnd; ++indexIterator) {
			Schema::Index::Pointer pIndex =
				Schema::IndexMap::getValue(indexIterator);

			// 索引に対応したIndexModifyInfoを作る
			IndexModifyInfo indexInfo(*pIndex);
			// 索引に割り当てられているエリアがDROP対象かを調べて変更後のエリアとしてセットする
			if (indexInfo.setAreaModifyInfo(m_vecAreaModifyInfo))
				// 変更があるなら変更内容として加える
				m_vecIndexModifyInfo.pushBack(indexInfo);
		}
	}
}

//	FUNCTION private
//	Admin::Mount::applyTableModifyInfo -- Table の定義変更内容をTableオブジェクトに反映する
//
//	NOTES
//		ここではまだ永続化されない
//
//	ARGUMENTS
//		bool bUndo_ = false
//			永続化後のエラー処理で元に戻すための呼び出しならtrue
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

void
Mount::
applyTableModifyInfo(bool bUndo_ /* = false */)
{
	// 表の変更を適用する
	ModSize nTable = m_vecTableModifyInfo.getSize();
	for (ModSize iTable = 0; iTable < nTable; ++iTable) {
		const TableModifyInfo& info = m_vecTableModifyInfo[iTable];
		Schema::Table* pTable =
			_SYDNEY_DYNAMIC_CAST(Schema::Table*, info.m_Object);
		if (!bUndo_) {
			// 通常処理
			pTable->move(m_cTrans, info.m_vecPrevID, info.m_vecPostID,
						 bUndo_, false /* not recovery */, true /* mount */);
			// 状態を「変更」にする
			pTable->touch();
		} else {
			// エラー処理
			// 取り消し処理をする
			// 以前はAreaのエラー処理でIDが変わっているかもしれないので
			// IDの読み替えをしていたがIDは不変なので読み替えは不要
			pTable->move(m_cTrans, info.m_vecPostID, info.m_vecPrevID,
						 bUndo_, false /* not recovery */, true /* mount */);
			if (!info.m_Persist) {
				// 永続化される前なら状態の変更をなしにする
				pTable->untouch();
			} else {
				// 状態を「変更」にする
				pTable->touch();
			}
		}
		info.m_Persist = !info.m_Persist; // 永続化が必要かどうかのフラグを反転する
	}
	// 索引の変更を適用する
	ModSize nIndex = m_vecIndexModifyInfo.getSize();
	for (ModSize iIndex = 0; iIndex < nIndex; ++iIndex) {
		IndexModifyInfo& info = m_vecIndexModifyInfo[iIndex];
		Schema::Index* pIndex =
			_SYDNEY_DYNAMIC_CAST(Schema::Index*, info.m_Object);
		if (!bUndo_) {
			// 通常処理
			pIndex->moveArea(m_cTrans, info.m_vecPrevID, info.m_vecPostID,
							 bUndo_, false /* not recovery */, true /* mount */);
			// 状態を「変更」にする
			pIndex->touch();
		} else {
			// エラー処理
			// 変更を取り消す
			pIndex->moveArea(m_cTrans, info.m_vecPostID, info.m_vecPrevID,
							 bUndo_, false /* not recovery */, true /* mount */);
			if (!info.m_Persist) {
				// 永続化される前なら状態の変更をなしにする
				pIndex->untouch();
			} else {
				// 状態を「変更」にする
				pIndex->touch();
			}
		}
		info.m_Persist = !info.m_Persist; // 永続化が必要かどうかのフラグを反転する
	}
}

//	FUNCTION private
//	Admin::Mount::persistTableModifyInfo -- Table以下のオブジェクトを永続化する
//
//	NOTES
//
//	ARGUMENTS
//		bool bUndo_ = false
//			永続化後のエラー処理で元に戻すための呼び出しならtrue
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//

void
Mount::
persistTableModifyInfo(bool bUndo_ /* = false */)
{
	; _SYDNEY_ASSERT(m_pDatabase.get());

	ModSize nTable = m_vecTableModifyInfo.getSize();
	for (ModSize iTable = 0; iTable < nTable; ++iTable) {
		const TableModifyInfo& cInfo = m_vecTableModifyInfo[iTable];
		if (!cInfo.m_Persist) {
			// 永続化が必要なオブジェクトについて永続化する
			Schema::SystemTable::Table(*m_pDatabase).store(
				m_cTrans, _SYDNEY_DYNAMIC_CAST(
					Schema::Table&, *cInfo.m_Object));
			// 永続化が成功したらオブジェクトの消滅管理もやめる
			cInfo.m_Persist = true;
		}
	}
	ModSize nIndex = m_vecIndexModifyInfo.getSize();
	for (ModSize iIndex = 0; iIndex < nIndex; ++iIndex) {
		const IndexModifyInfo& cInfo = m_vecIndexModifyInfo[iIndex];
		if (!cInfo.m_Persist) {
			// 永続化が必要なオブジェクトについて永続化する
			Schema::SystemTable::Index(*m_pDatabase).store(
				m_cTrans, _SYDNEY_DYNAMIC_CAST(
					Schema::Index&, *cInfo.m_Object));
			// 永続化が成功したらオブジェクトの消滅管理もやめる
			cInfo.m_Persist = true;
		}
	}
	// FileとAreaContentの永続化も行う
	Schema::SystemTable::File(*m_pDatabase).store(m_cTrans);
	Schema::SystemTable::AreaContent(*m_pDatabase).store(m_cTrans);
}

#ifdef OBSOLETE // データベース単位のUNDOは呼ばれない
//	FUNCTION private
//	Admin::Mount::undoTableModifyInfo -- Table以下のオブジェクトに関するUNDO処理をする
//
//	NOTES
//
//	ARGUMENTS
//		bool redone_
//			後でREDOされるかを示す
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
undoTableModifyInfo(bool redone_)
{
	ModSize nTable = m_vecTableModifyInfo.getSize();
	for (ModSize iTable = 0; iTable < nTable; ++iTable) {
		const TableModifyInfo& cInfo = m_vecTableModifyInfo[iTable];
		// AlterのUNDO処理のうちファイルの移動や破棄を伴わない処理を行う
		Schema::Manager::SystemTable::UndoUtility::
			undoAlterTable(m_cTrans, cInfo.m_ID, cInfo.m_vecPostID,
						   m_strDbName, redone_, true /* mount */);
	}
	ModSize nIndex = m_vecIndexModifyInfo.getSize();
	for (ModSize iIndex = 0; iIndex < nIndex; ++iIndex) {
		const IndexModifyInfo& cInfo = m_vecIndexModifyInfo[iIndex];
		// AlterのUNDO処理のうちファイルの移動や破棄を伴わない処理を行う
		Schema::Manager::SystemTable::UndoUtility::
			undoAlterIndex(m_cTrans, cInfo.m_ID, cInfo.m_vecPostID,
						   m_strDbName, redone_, true /* mount */);
	}
}
#endif

#ifdef OBSOLETE
//	FUNCTION private
//	Admin::Mount::redoTableModifyInfo -- Table以下のオブジェクトに関するREDO処理をする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
redoTableModifyInfo()
{
	// 後の処理で使用するスキーマオブジェクトを読み込んでおく
	preloadTableObject();

	ModSize nTable = m_vecTableModifyInfo.getSize();
	for (ModSize iTable = 0; iTable < nTable; ++iTable) {
		const TableModifyInfo& cInfo = m_vecTableModifyInfo[iTable];
		// Tableオブジェクトを取得する
		// MOUNTのREDOの前にCREATE TABLEのREDOが行われることはないので
		// IDは変わっていないはず
		cInfo.m_Object = m_pDatabase->getTable(cInfo.m_ID, m_cTrans);
		// AlterのREDO処理のうちファイルの移動や破棄を伴わない処理を行う
		Schema::Manager::SystemTable::RedoUtility::
			alterTable(m_cTrans, cInfo.m_ID, cInfo.m_vecPrevID, cInfo.m_vecPostID,
					   m_strDbName, _SYDNEY_DYNAMIC_CAST(Schema::Table&, *(cInfo.m_Object)),
					   true /* mount */);
		cInfo.m_Persist = false;
	}
	ModSize nIndex = m_vecIndexModifyInfo.getSize();
	for (ModSize iIndex = 0; iIndex < nIndex; ++iIndex) {
		const IndexModifyInfo& cInfo = m_vecIndexModifyInfo[iIndex];
		// Indexオブジェクトを取得する
		// MOUNTのREDOの前にCREATE INDEXのREDOが行われることはないので
		// IDは変わっていないはず
		cInfo.m_Object = Schema::Index::get(cInfo.m_ID, m_pDatabase.get(), m_cTrans);
		// AlterのREDO処理のうちファイルの移動や破棄を伴わない処理を行う
		Schema::Manager::SystemTable::RedoUtility::
			alterIndex(m_cTrans, cInfo.m_ID, cInfo.m_vecPrevID, cInfo.m_vecPostID,
					   m_strDbName, _SYDNEY_DYNAMIC_CAST(Schema::Index&, *(cInfo.m_Object)),
					   true /* mount */);
		cInfo.m_Persist = false;
	}
}
#endif

//	FUNCTION private
//	Admin::Mount::preloadTableObject -- Table以下のスキーマオブジェクトを読み込んでおく
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Mount::
preloadTableObject()
{
	// マウントするデータベースの「表」表を読み込んでおく
	// 引数のtrueは読み込んだオブジェクトのメンバーとして
	// データベースオブジェクトを設定することを意味する

	const Schema::TableMap& cTables =
		m_pDatabase->loadTable(m_cTrans, true /* recovery */);

// ファイル名はシステム表から読み込まれるようになったので「列」表を読み込む必要はない

	// すべての表について「索引」表を同様に読み込んでおく
//	// ヒープファイル名の作成に使用されるので「列」表も読み込んでおく
	// ファイルの定義も変更する可能性があるので「ファイル」表も読み込んでおく
	Schema::TableMap::ConstIterator iterator = cTables.begin();
	const Schema::TableMap::ConstIterator& end = cTables.end();
	for (; iterator != end; ++iterator) {
		Schema::Table::Pointer pTable = Schema::TableMap::getValue(iterator);
//		(void)pTable->loadColumn(m_cTrans, true /* recovery */);
		(void)pTable->loadIndex(m_cTrans, true /* recovery */);
		(void)pTable->loadFile(m_cTrans, true /* recovery */);
										// ファイルは索引オブジェクトを参照することがあるので
										// loadIndexの後でなければならない
	}		
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
