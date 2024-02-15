// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MergeDaemon.cpp -- マージを実行するスレッドを管理する
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/MergeDaemon.h"

#include "FullText2/FileID.h"
#include "FullText2/FullTextFile.h"
#include "FullText2/MergeReserve.h"

#include "Checkpoint/Daemon.h"

#include "Common/Message.h"
#include "Common/ExceptionMessage.h"
#include "Common/AutoCaller.h"

#include "Lock/Name.h"

#include "LogicalFile/LogData.h"

#include "Trans/AutoLatch.h"
#include "Trans/AutoTransaction.h"
#include "Trans/Transaction.h"

#include "Schema/Table.h"
#include "Schema/File.h"
#include "Schema/Database.h"

#include "Os/Thread.h"

#include "Exception/ModLibraryError.h"
#include "Exception/Unexpected.h"
#include "Exception/Cancel.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/LockTimeout.h"
#include "Exception/TableNotFound.h"
#include "Exception/FileNotFound.h"

#include <new>

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::MergeDaemon::MergeDaemon -- コンストラクタ
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
//
MergeDaemon::MergeDaemon()
{
}

//
//	FUNCTION public
//	FullText2::MergeDaemon::~MergeDaemon -- デストラクタ
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
//
MergeDaemon::~MergeDaemon()
{
}

//
//	FUNCTION public
//	FullText2::MergeDaemon::runnable -- 実行スレッド
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
//
void
MergeDaemon::runnable()
{
	// このスレッドのプライオリティを上げる
	Os::Thread::setPriority(Os::Thread::Priority::AboveNormal);
	
	for (;;)
	{
		Lock::FileName cFileName;
		int iElement;
		while (MergeReserve::getFront(cFileName, iElement, 500) == false)
		{
			// 中断要求をチェックする
			if (isAborted() == true)
				return;
		}
		
		try
		{
			// マージを実行する
			merge(cFileName, iElement);
		}
		catch (Exception::Unexpected& e)
		{
			SydErrorMessage << e << ModEndl;
		}
		catch (Exception::Cancel&)
		{
			return;
		}
		catch (Exception::DatabaseNotFound& e)
		{
			SydInfoMessage << e << ModEndl;
		}
		catch (Exception::TableNotFound& e)
		{
			SydInfoMessage << e << ModEndl;
		}
		catch (Exception::FileNotFound& e)
		{
			SydInfoMessage << e << ModEndl;
		}
		catch (Exception::Object& e)
		{
			SydErrorMessage << e << ModEndl;
		}
		catch (ModException& e)
		{
			SydErrorMessage << _MOD_EXCEPTION(e) << ModEndl;
			if (e.getErrorNumber() == ModCommonErrorUnexpected)
				_SYDNEY_RETHROW;
			Common::Thread::resetErrorCondition();
		}
#ifndef NO_CATCH_ALL
		catch (std::exception& e)
		{
			SydErrorMessage << "std::exception occurred. "
							<< e.what() << ModEndl;
		}
		catch (...)
		{
			SydErrorMessage << "Unexpexted Exception" << ModEndl;
			throw;
		}
#endif

		// エントリを削除する
		MergeReserve::popFront();
	}
}

//
//	FUNCTION private
//	FullText2::MergeDaemon::merge -- マージを実行する
//
//	NOTES
//
//	ARGUMENTS
// 	const Lock::FileName& cFileName_
//		マージするファイルのロック名
//	int iElement_
//		要素番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MergeDaemon::merge(const Lock::FileName& cFileName_, int iElement_)
{
	// 全文索引のマージは高負荷な処理なので、
	// マージ中は同期処理の実行を禁止する
	Checkpoint::Daemon::AutoDisabler
		disabler0(Checkpoint::Daemon::Category::FileSynchronizer);
	
	// 全文索引のマージ中にチェックポイントが発生すると、
	// 自動リカバリに失敗してしまう
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	// トランザクションを開始する
	Trans::AutoTransaction pTransaction(Trans::Transaction::attach());
	pTransaction->begin(cFileName_.getDatabasePart(),
						Trans::Transaction::Category::ReadWrite,
						Trans::Transaction::IsolationLevel::ReadCommitted);
	
	Schema::Database* pDatabase = 0;
	try
	{
		// データベースを必要なロックをかけて得る
		pDatabase = Schema::Database::getLocked(
			*pTransaction,
			cFileName_.getDatabasePart(),
			Lock::Name::Category::Tuple,
			Schema::Hold::Operation::ReadForImport,
			Lock::Name::Category::Tuple,
			Schema::Hold::Operation::ReadForImport,
			false,
			1000);
	}
	catch (Exception::LockTimeout&)
	{
		// lockが取得できなかった場合はマージをスキップする
		return;
	}
	if (pDatabase == 0)
	{
		ModUnicodeOstrStream stream;
		stream << "ID=" << cFileName_.getDatabasePart();
		_SYDNEY_THROW1(Exception::DatabaseNotFound, stream.getString());
	}

	// スーパーユーザーモードの場合はマージをスキップする
	if (pDatabase->isSuperUserMode())
		return;
	
	// キャッシュが破棄されないようにopenする
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// ログを設定する
	pTransaction->setLog(*pDatabase);

	// 論理ログを書き出す
	{
		LogicalFile::FullTextMergeLog cLog;
		const Lock::Name& lockName = pTransaction->getLogInfo(
			Trans::Log::File::Category::Database).getLockName();
		
		Trans::AutoLatch latch(*pTransaction, lockName);
		pTransaction->storeLog(Trans::Log::File::Category::Database, cLog);
	}

	// バッチインサートとの競合を防止するため
	// マージデーモンでもテーブルをロックする
	Lock::TableName cTableName(cFileName_.getDatabasePart(),
							   cFileName_.getTablePart());
	pTransaction->lock(cTableName,
					   Lock::Mode::VS,
					   Lock::Duration::Inside);
	
	// FileIDを得る
	const LogicalFile::FileID& cFileID = getFileID(pDatabase,
												   *pTransaction,
												   cFileName_);
	FileID cFullTextFileID(cFileID);
	
	SydMessage << "Start FullText Index Merge ("
			   << cFullTextFileID.getPath() << ":"
			   << iElement_ << ")" << ModEndl;
	
	// ファイルを得る
	FullTextFile cIndexFile(cFullTextFileID);

	// マージ用のオープン
	cIndexFile.openForMerge(*pTransaction, iElement_);

	try
	{
		// 索引単位ごとのマージ
		while (cIndexFile.mergeList() == true)
		{
			// 中断チェック
			if (isAborted() == true)
			{
				_SYDNEY_THROW0(Exception::Cancel);
			}
		}
	}
	catch (...)
	{
		// マージ用のクローズ
		cIndexFile.closeForMerge(false);
		// コミット
		pTransaction->commit();

		SydMessage << "Cancel FullText Index Merge" << ModEndl;
		
		_SYDNEY_RETHROW;
	}

	// マージ用のクローズ
	cIndexFile.closeForMerge(true);

	// コミット
	pTransaction->commit();

	SydMessage << "End FullText Index Merge" << ModEndl;
}

//
//	FUNCTION public
//	FullText2::MergeDaemon::getFileID -- FileIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//		データベース
//	Trans::Transaction& cTransaction_
//		トランザクション
//	const Lock::FileName& cFileName_
//		ロックのファイル名
//
//	RETURN
//	const LogicalFile::FileID&
//		FileID
//
//	EXCEPTIONS
//
const LogicalFile::FileID&
MergeDaemon::getFileID(Schema::Database* pDatabase_,
					   Trans::Transaction& cTransaction_,
					   const Lock::FileName& cFileName_)
{
	// 表を得る
	Schema::Table* pTable = pDatabase_->getTable(cFileName_.getTablePart(),
												 cTransaction_);
	if (pTable == 0)
	{
		ModUnicodeOstrStream stream;
		stream << "ID=" << cFileName_.getTablePart();
		_SYDNEY_THROW2(Exception::TableNotFound,
					   stream.getString(),
					   pDatabase_->getName());
	}

	// ファイルを得る
	Schema::File* pFile = pTable->getFile(cFileName_.getFilePart(),
										  cTransaction_);
	if (pFile == 0)
	{
		ModUnicodeOstrStream stream;
		stream << "ID=" << cFileName_.getFilePart();
		_SYDNEY_THROW1(Exception::FileNotFound,
					   stream.getString());
	}

	return pFile->getFileID();
}


//
//	Copyright (c) 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
