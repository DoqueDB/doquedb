// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileReflect.cpp -- 論理ログの反映に関連するクラスの定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#include "Schema/FileReflect.h"
#include "Schema/AutoLatch.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/LogData.h"
#include "Schema/Message.h"
#include "Schema/TreeNode.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/UnsignedIntegerArrayData.h"

#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/FileDriverManager.h"

#include "Opt/LogData.h"

#include "Trans/LogData.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::FileReflect::FileReflect --
//		コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::File& cFile_
//			再構成の対象となっているファイル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

FileReflect::
FileReflect(Trans::Transaction& cTrans_, const File& cFile_)
	: m_cTrans(cTrans_), m_cFile(cFile_),
	  m_pAccess(0),
	  m_eLogType(Opt::LogData::Type::Undefined)
{
	m_pAccess = cFile_.getAccessFile(cTrans_);
}

//	FUNCTION public
//	Schema::FileReflect::~FileReflect --
//		デストラクター
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

FileReflect::
~FileReflect()
{
	m_eLogType = Opt::LogData::Type::Undefined;

	// 以下のオブジェクトは生成管理させている
//	delete m_pAccess, m_pAccess = 0;
}

//	FUNCTION public
//	Schema::FileReflect::isUndoNeeded --
//		ログ内容をUNDOする必要があるかを調べる
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Log::Data& cLogData_
//			ログ情報
//
//	RETURN
//		true	UNDOの必要がある
//		false	UNDOの必要がない
//
//	EXCEPTIONS

bool
FileReflect::
isUndoNeeded(const Trans::Log::Data& cLogData_)
{
	if (!analyzeLogData(cLogData_)) {
		// このログは関係なかったので処理しない
		return false;
	}

	// 検索用のファイルに条件設定してFetchまたはOpenする

	//	DELETE: 削除する「前」のデータが「なければ」UNDOする
	//	INSERT: 挿入した「後」のデータが「あれば」UNDOする
	//	UPDATE: 更新した「後」のデータが「あれば」UNDOする

	Common::Data::Pointer cTupleID(getTupleID());

	m_pAccess->openSearchFile((m_eLogType == Opt::LogData::Type::Delete) ? &m_cPreKey : &m_cPostKey,
							  &cTupleID);

	// タプルIDが一致するデータがあるかを得る
	bool bExists = m_pAccess->isExists(cTupleID.get());

	return ((m_eLogType == Opt::LogData::Type::Delete && !bExists)
			|| (m_eLogType == Opt::LogData::Type::Insert && bExists)
			|| (m_eLogType == Opt::LogData::Type::Update && bExists));
}

//	FUNCTION public
//	Schema::FileReflect::isRedoNeeded --
//		ログ内容をREDOする必要があるかを調べる
//
//	NOTES
//
//	ARGUMENTS
//		const Trans::Log::Data& cLogData_
//			ログ情報
//
//	RETURN
//		true	REDOの必要がある
//		false	REDOの必要がない
//
//	EXCEPTIONS

bool
FileReflect::
isRedoNeeded(const Trans::Log::Data& cLogData_)
{
	if (!analyzeLogData(cLogData_)) {
		// このログは関係なかったので処理しない
		return false;
	}

	// 検索用のファイルに条件設定してFetchまたはOpenする

	//	DELETE: 削除する「前」のデータが「あれば」REDOする
	//	INSERT: 挿入した「後」のデータが「なければ」REDOする
	//	UPDATE: 更新する「前」のデータが「あれば」REDOする

	Common::Data::Pointer cTupleID(getTupleID());

	m_pAccess->openSearchFile((m_eLogType == Opt::LogData::Type::Insert) ? &m_cPostKey : &m_cPreKey,
							  &cTupleID);

	// タプルIDが一致するデータがあるかを得る
	bool bExists = m_pAccess->isExists(cTupleID.get());

	return ((m_eLogType == Opt::LogData::Type::Delete && bExists)
			|| (m_eLogType == Opt::LogData::Type::Insert && !bExists)
			|| (m_eLogType == Opt::LogData::Type::Update && bExists));
}

//	FUNCTION public
//	Schema::FileReflect::undo --
//		UNDOする
//
//	NOTES
//		直前にisUndoNeededが呼ばれていることが前提
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileReflect::
undo()
{
	switch (m_eLogType) {
	case Opt::LogData::Type::Insert:
	{
		// 更新用のファイルがオープンされていなければオープンする
		m_pAccess->openUpdateFile(true /* noUpdate */);

		// InsertのUNDOはDeleteである
		; _SYDNEY_ASSERT(m_cPostKey.getCount());

		Common::DataArrayData cDelKey;
		cDelKey = m_cPostKey;			// ポインターのコピー
		if (!m_cFile.isKeyUnique()) {
			cDelKey.pushBack(getTupleID());
		}
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportReorganization)
			<< "Reflect(undo): file: " << m_pAccess->getUpdateFile().toString()
			<< " logType: insert"
			<< " key: " << cDelKey.toString()
			<< ModEndl;
#endif
		m_pAccess->getUpdateFile().expunge(&cDelKey);
		break;
	}
	case Opt::LogData::Type::Delete:
	{
		// 更新用のファイルがオープンされていなければオープンする
		m_pAccess->openUpdateFile(true /* noUpdate */);

		// DeleteのUNDOはInsertである
		; _SYDNEY_ASSERT(m_cPreKey.getCount());

		Common::DataArrayData cInsertData;
		cInsertData = m_cPreKey;		// ポインターのコピー
		cInsertData.pushBack(getTupleID());
		if (m_cFile.getObjectID(m_cTrans, true /* for put */)) {
			cInsertData.pushFront(new LogicalFile::ObjectID());
		}
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportReorganization)
			<< "Reflect(undo): file: " << m_pAccess->getUpdateFile().toString()
			<< " logType: delete"
			<< " data: " << cInsertData.toString()
			<< ModEndl;
#endif
		m_pAccess->getUpdateFile().insert(&cInsertData);
		break;
	}
	case Opt::LogData::Type::Update:
	{
		// 更新用のファイルがオープンされていなければオープンする
		m_pAccess->openUpdateFile();

		// UpdateのUNDOはUpdateである
		; _SYDNEY_ASSERT(m_cPreKey.getCount());
		; _SYDNEY_ASSERT(m_cPostKey.getCount());
		; _SYDNEY_ASSERT(m_cPreKey.getCount() == m_cPostKey.getCount());

		Common::DataArrayData cUpdateKey;
		cUpdateKey = m_cPostKey;		// ポインターのコピー
		if (!m_cFile.isKeyUnique()) {
			cUpdateKey.pushBack(getTupleID());
		}
		Common::DataArrayData cUpdateData;
		cUpdateData = m_cPreKey;		// ポインターのコピー
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportReorganization)
			<< "Reflect(undo): file: " << m_pAccess->getUpdateFile().toString()
			<< " logType: update"
			<< " key: " << cUpdateKey.toString()
			<< " data: " << cUpdateData.toString()
			<< ModEndl;
#endif
		m_pAccess->getUpdateFile().update(&cUpdateKey, &cUpdateData);
		break;
	}
	default:
		break;
	}
}

//	FUNCTION public
//	Schema::FileReflect::redo --
//		REDOする
//
//	NOTES
//		直前にisRedoNeededが呼ばれていることが前提
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileReflect::
redo()
{
	switch (m_eLogType) {
	case Opt::LogData::Type::Insert:
	{
		// 更新用のファイルがオープンされていなければオープンする
		m_pAccess->openUpdateFile(true /* noUpdate */);

		// InsertのREDOはInsertである
		; _SYDNEY_ASSERT(m_cPostKey.getCount());

		Common::DataArrayData cInsertData;
		cInsertData = m_cPostKey;		// ポインターのコピー
		cInsertData.pushBack(getTupleID());
		if (m_cFile.getObjectID(m_cTrans, true /* for put */)) {
			cInsertData.pushFront(new LogicalFile::ObjectID());
		}
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportReorganization)
			<< "Reflect(redo): file: " << m_pAccess->getUpdateFile().toString()
			<< " logType: insert"
			<< " data: " << cInsertData.toString()
			<< ModEndl;
#endif
		m_pAccess->getUpdateFile().insert(&cInsertData);
		break;
	}
	case Opt::LogData::Type::Delete:
	{
		// 更新用のファイルがオープンされていなければオープンする
		m_pAccess->openUpdateFile(true /* noUpdate */);

		// DeleteのREDOはDeleteである
		; _SYDNEY_ASSERT(m_cPreKey.getCount());

		Common::DataArrayData cDelKey;
		cDelKey = m_cPreKey;			// ポインターのコピー
		if (!m_cFile.isKeyUnique()) {
			cDelKey.pushBack(getTupleID());
		}
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportReorganization)
			<< "Reflect(redo): file: " << m_pAccess->getUpdateFile().toString()
			<< " logType: delete"
			<< " key: " << cDelKey.toString()
			<< ModEndl;
#endif
		m_pAccess->getUpdateFile().expunge(&cDelKey);
		break;
	}
	case Opt::LogData::Type::Update:
	{
		// 更新用のファイルがオープンされていなければオープンする
		m_pAccess->openUpdateFile();

		// UpdateのREDOはUpdateである
		; _SYDNEY_ASSERT(m_cPreKey.getCount());
		; _SYDNEY_ASSERT(m_cPostKey.getCount());
		; _SYDNEY_ASSERT(m_cPreKey.getCount() == m_cPostKey.getCount());

		Common::DataArrayData cUpdateKey;
		cUpdateKey = m_cPreKey;			// ポインターのコピー
		if (!m_cFile.isKeyUnique()) {
			cUpdateKey.pushBack(getTupleID());
		}
		Common::DataArrayData cUpdateData;
		cUpdateData = m_cPostKey;		// ポインターのコピー
#ifdef DEBUG
		SydSchemaParameterMessage(Message::ReportReorganization)
			<< "Reflect(redo): file: " << m_pAccess->getUpdateFile().toString()
			<< " logType: update"
			<< " key: " << cUpdateKey.toString()
			<< " data: " << cUpdateData.toString()
			<< ModEndl;
#endif
		m_pAccess->getUpdateFile().update(&cUpdateKey, &cUpdateData);
		break;
	}
	default:
		break;
	}
}

//	FUNCTION private
//	Schema::FileReflect::analyzeLogData --
//		ログ情報を解析する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true	反映の必要がある
//		false	反映の必要がない
//
//	EXCEPTIONS

bool
FileReflect::
analyzeLogData(const Trans::Log::Data& cLogData_)
{
	; _SYDNEY_ASSERT(cLogData_.getCategory() == Trans::Log::Data::Category::TupleModify);

	reset();
	; _SYDNEY_ASSERT(m_cPreKey.getCount() == 0);
	; _SYDNEY_ASSERT(m_cPostKey.getCount() == 0);

	const Common::DataArrayData& cData = dynamic_cast<const Opt::LogData&>(cLogData_).getData();

	// ログのデータは
	//	<データ> = <LogType>, <TableID>, <TupleID>, <ColumnID[]>, <Value[]>(, <Value[]>)
	// のようになっているはず

#ifdef DEBUG
	SydSchemaParameterMessage(Message::ReportReorganization)
		<< "Analyze LogData: " << cData.toString()
		<< ModEndl;
#endif

	const int _iDataCountExpected = 5;

	int iCount = cData.getCount();
	if (iCount < _iDataCountExpected) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	int i = 0;
	Common::Data* pElement = 0;

	// LogType
	{
		; _SYDNEY_ASSERT(i < iCount);
		int iValue = LogData::getInteger(cData.getElement(i++));
		switch (iValue) {
		case Opt::LogData::Type::Insert:
		case Opt::LogData::Type::Delete:
		case Opt::LogData::Type::Update:
			{
				m_eLogType = static_cast<Opt::LogData::Type::Value>(iValue);
				break;
			}
		case Opt::LogData::Type::Delete_Undo:
			{
				m_eLogType = Opt::LogData::Type::Delete;
				break;
			}
		case Opt::LogData::Type::Update_Undo:
			{
				m_eLogType = Opt::LogData::Type::Update;
				break;
			}
		case Opt::LogData::Type::Undefined:
		default:
			{
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
		}
	}

	// TableID
	{
		; _SYDNEY_ASSERT(i < iCount);
		ObjectID::Value tableID = LogData::getID(cData.getElement(i++));

		// 操作された表が作成中のファイルが属する表でなければ
		// 反映する必要はない
		if (tableID != m_cFile.getTableID()) {
			return false;
		}
	}

	// TupleID
	{
		; _SYDNEY_ASSERT(i < iCount);
		unsigned int uValue = LogData::getUnsignedInteger(cData.getElement(i++));
		m_cTupleID.setValue(uValue);
	}

	// ColumnID[]
	// Value[]
	{
		// ColumnID[]
		; _SYDNEY_ASSERT(i < iCount);

		// ログに記録されている列IDの一覧を得る(比較しやすいようにIDSetを使う)
		IDSet vecColumnIDs(LogData::getIDs(cData.getElement(i++)));
		ModSize nCol = vecColumnIDs.getSize();

		// 記録されている列IDにキーのものが含まれているか
		if (vecColumnIDs.containsAny(m_pAccess->getKeyColumnIDs())) {
			if (!vecColumnIDs.containsAll(m_pAccess->getKeyColumnIDs())) {
				// ひとつでもキーが見つかっているなら、
				// オプティマイザーが必要な列をログに出力しているはず
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
		} else {
			// ログの内容にキーが関係していなければ反映する必要はない
			return false;
		}

		// Value[]

		// キーフィールドに対応する列IDを得る
		const IDSet& cKeyColumnIDs = m_pAccess->getKeyColumnIDs();
		ModSize nKey = cKeyColumnIDs.getSize();

		// 挿入以外のログにはタプルを特定するキーデータが入っている
		Common::DataArrayData* pPreKeyData = 0;
		Common::DataArrayData* pPostKeyData = 0;
		if (m_eLogType != Opt::LogData::Type::Insert) {
			; _SYDNEY_ASSERT(i < iCount);
			pElement = cData.getElement(i++).get();
			if (!pElement
				|| pElement->getType() != Common::DataType::Array
				|| pElement->getElementType() != Common::DataType::Data) {
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
			pPreKeyData =
				_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pElement);
			; _SYDNEY_ASSERT(pPreKeyData);
		}
		// 削除以外のログには変更後のデータが入っている
		if (m_eLogType != Opt::LogData::Type::Delete) {
			; _SYDNEY_ASSERT(i < iCount);
			pElement = cData.getElement(i++).get();
			if (!pElement
				|| pElement->getType() != Common::DataType::Array
				|| pElement->getElementType() != Common::DataType::Data) {
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
			pPostKeyData =
				_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pElement);
			; _SYDNEY_ASSERT(pPostKeyData);
		}

		// キーの列IDとログデータの列IDが一致する位置のデータを取得する
		bool bPreAllIsNull = true;		// NULLでないキーがあればfalse
		bool bPostAllIsNull = true;		// NULLでないキーがあればfalse

		for (ModSize iKey = 0; iKey < nKey; ++iKey) {
			for (ModSize iCol = 0; iCol < nCol; ++iCol) {
				if (vecColumnIDs[iCol] == cKeyColumnIDs[iKey]) {
					if (pPreKeyData) {
						m_cPreKey.pushBack(pPreKeyData->getElement(iCol));
						if (bPreAllIsNull && m_cFile.isKeyImportable(pPreKeyData->getElement(iCol))) {
							bPreAllIsNull = false;
						}
					}
					if (pPostKeyData) {
						m_cPostKey.pushBack(pPostKeyData->getElement(iCol));
						if (bPostAllIsNull && m_cFile.isKeyImportable(pPostKeyData->getElement(iCol))) {
							bPostAllIsNull = false;
						}
					}
					break;
				}
			}
		}
		; _SYDNEY_ASSERT(!pPreKeyData || static_cast<ModSize>(m_cPreKey.getCount()) == nKey);
		; _SYDNEY_ASSERT(!pPostKeyData || static_cast<ModSize>(m_cPostKey.getCount()) == nKey);

		if (pPreKeyData && bPreAllIsNull) {
			// 変更前のすべてのキーがNULLなのでdeleteなら何もしない
			// updateならinsertにする

			m_cPreKey.clear();			// 変更前データはもはや使用しない
			if (m_eLogType == Opt::LogData::Type::Delete) {
				return false;

			} else {
				m_eLogType = Opt::LogData::Type::Insert;
			}
		}

		if (pPostKeyData && bPostAllIsNull) {
			// 変更後のすべてのキーがNULLなのでinsertなら何もしない
			// updateならdeleteにする

			m_cPostKey.clear();			// 変更前データはもはや使用しない
			if (m_eLogType == Opt::LogData::Type::Insert) {
				return false;

			} else {
				m_eLogType = Opt::LogData::Type::Delete;
			}
		}
	}

	return true;
}

//	FUNCTION private
//	Schema::FileReflect::reset --
//		ログ情報の解析結果を消去する
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
FileReflect::
reset()
{
	m_eLogType = Opt::LogData::Type::Undefined;

	m_cPreKey.clear();
	m_cPostKey.clear();
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
