// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Database.cpp -- データベースの回復関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2006, 2007, 2009, 2010, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "SyTypes.h"

#include "Admin/Database.h"
#include "Admin/LogData.h"
#include "Admin/Transaction.h"
#include "Admin/Utility.h"

#include "Checkpoint/LogData.h"
#include "Common/Assert.h"
#include "Common/UnsignedInteger64Data.h"
#include "DServer/Branch.h"
#include "Exception/DatabaseNotMountable.h"
#include "Exception/LogFileCorrupted.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/SnapshotDiscarded.h"
#include "Schema/Database.h"
#include "Schema/LogData.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoLogFile.h"
#include "Trans/LogData.h"

#include "ModAlgorithm.h"
#include "ModMap.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING
_SYDNEY_ADMIN_RECOVERY_USING

namespace
{
}

//	FUNCTION public
//	Admin::Recovery::Database::Database --
//		データベースの回復処理を行うクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			REDO する操作の対象となるデータベースの
//			スキーマオブジェクトを表すクラス
//		Trans::TimeStamp&	starting
//			指定されたとき
//				回復処理の開始時点を表すタイムスタンプ
//			指定されないとき
//				Trans::TimeStamp() が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Recovery::
Database::Database(Schema::Database& database)
	: _database(database),
	  _lsn(Trans::Log::IllegalLSN),
	  _recovered(true),
	  _first(true),
	  _rollforward(false),
	  _mount(false),
	  _lastMasterLSN(Trans::Log::IllegalLSN),
	  _slave(false)
{
	// キャッシュが破棄されないようにデータベースの使用開始を宣言する

	_database.open();

	// スレーブデータベースか
	
	_slave = (_database.getMasterURL().getLength() != 0 ? true : false);
}

Recovery::
Database::Database(Schema::Database& database,
				   const Trans::TimeStamp& starting)
	: _database(database),
	  _lsn(Trans::Log::IllegalLSN),
	  _starting(starting),
	  _recovered(true),
	  _first(true),
	  _rollforward(false),
	  _mount(false),
	  _lastMasterLSN(Trans::Log::IllegalLSN),
	  _slave(false)
{
	// キャッシュが破棄されないようにデータベースの使用開始を宣言する

	_database.open();
	
	// スレーブデータベースか
	
	_slave = (_database.getMasterURL().getLength() != 0 ? true : false);
}

//	FUNCTION public
//	Admin::Recovery::Database::~Database --
//		データベースの回復処理を行うクラスのデストラクター
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

Recovery::
Database::~Database()
{
	// データベースの使用終了を宣言し、キャッシュをクリアされやすくする

	_database.close(true);
}

//	FUNCTION public
//	Admin::Recovery::Database::findStartLSN --
//		スキーマデータベースの回復処理の基点となる
//		論理ログのログシーケンス番号を求める
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//		Checkpoint::Database::UnavailableMap&	unavailableDatabase
//			回復処理を開始する時点に利用不可な
//			データベースの名前を管理するマップ
//
//	RETURN
//		true
//			タイムスタンプファイルに記録されているタイムスタンプを
//			回復処理の開始する時点のタイムスタンプによって更新すべき
//		false
//			更新すべきでない
//
//	EXCEPTIONS

bool
Recovery::
Database::findStartLSN(
	Trans::Transaction& trans,
	Checkpoint::Database::UnavailableMap& unavailableDatabase)
{
	//
	//【注意】
	//	タイムスタンプファイルに記録されているタイムスタンプを
	//	回復処理の開始する時点のタイムスタンプによって更新するかどうかという
	//	戻り値を返しているが、回復処理の起点は _starting メンバ変数から
	//	論理ログを辿っていけば問題はないし
	//	タイムスタンプを小さくする更新はありえないので、
	//	戻り値を返す必要はないし、呼び出し側も見ていない
	//	
	
	// 記憶している情報の検証

	; _SYDNEY_ASSERT(_lsn == Trans::Log::IllegalLSN);
	; _SYDNEY_ASSERT(!_logData.isOwner());
	; _SYDNEY_ASSERT(_starting.isIllegal());
	; _SYDNEY_ASSERT(_recovered);
	_first = false;
	; _SYDNEY_ASSERT(_undoLSN.isEmpty());
	; _SYDNEY_ASSERT(_redoLSN.isEmpty());
	; _SYDNEY_ASSERT(_committedLSN.isEmpty());
	; _SYDNEY_ASSERT(_noRedoLSN.isEmpty());
	; _SYDNEY_ASSERT(_finishedLSN.isEmpty());
	; _SYDNEY_ASSERT(_runningLSN.isEmpty());
	; _SYDNEY_ASSERT(!unavailableDatabase.getSize());

	Trans::Log::LSN				lsn;
	ModVector<Trans::Log::LSN>	rollbackedLSN;
	ModVector<Trans::Log::LSN>	noUndoLSN;

	unsigned int rest = 0;
	Boolean::Value synchronized = Boolean::Unknown;

	// システム用の論理ログファイルの末尾に記録されている
	// 論理ログのログシーケンス番号を得る

	Trans::Log::AutoFile logFile(_database.getLogFile());
	{
	Trans::AutoLatch latch(trans, logFile->getLockName());
	lsn = logFile->getLastLSN();
	}
	// 論理ログファイルの先頭に向かって
	// ひとつひとつ論理ログを読み出していく

	while (lsn != Trans::Log::IllegalLSN) {

		// 論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data>	data;
		{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		data = logFile->load(lsn);
		}
		if (!data.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::TransactionCommit:
		{
			// トランザクションのコミットを表す論理ログである

			const Trans::Log::TransactionCommitData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionCommitData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)

				// 不正なログシーケンス番号が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// コミットされたトランザクションの開始を表す
			// 論理ログのログシーケンス番号を覚えておく

			_committedLSN.pushBack(tmp.getBeginTransactionLSN());

			//【注意】	コミットはデータベースを更新しないので、
			//			これだけだとリカバリの必要はない

			break;
		}
		case Trans::Log::Data::Category::TransactionRollback:
		{
			// トランザクションのロールバックを表す論理ログである

			const Trans::Log::TransactionRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionRollbackData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// ロールバックされたトランザクションの開始を表す
			// 論理ログのログシーケンス番号を覚えておく

			rollbackedLSN.pushBack(tmp.getBeginTransactionLSN());

			// 回復処理の開始時点からデータベースは更新されているので、
			// まず、その時点にデータベースを構成する論理ファイルを
			// リカバリする必要がある
			//
			//【注意】	トランザクションを開始して、更新操作を表す
			//			論理ログをひとつも記録せずにロールバックしたときは、
			//			このトランザクションに関する論理ログは
			//			ひとつも記録されないので、
			//			ロールバックを表す論理ログが記録されていれば、
			//			リカバリする必要があるとみなしてよい

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::StatementRollback:
		{
			// SQL 文のロールバックを表す論理ログである

			const Trans::Log::StatementRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::StatementRollbackData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (_committedLSN.isFound(tmp.getBeginTransactionLSN())) {
				if (!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

					// コミットされたトランザクションで
					// 実行された SQL 文のうち、ロールバックされたものは
					// REDO しないようにする必要がある
					//
					//【注意】	この SQL 文は UNDO もしないことになる

					_noRedoLSN.pushBack(tmp.getEndStatementLSN());

			} else if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()))
				if (!noUndoLSN.isFound(tmp.getEndStatementLSN()))

					// 障害発生時までに終了しなかったトランザクションで
					// 実行された SQL 文のうち、ロールバックされたものは
					// UNDO しないようにする必要がある

					noUndoLSN.pushBack(tmp.getEndStatementLSN());

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::CheckpointSystem:
		{
			// システムに関するチェックポイント処理の終了を表す論理ログである

			const Checkpoint::Log::CheckpointSystemData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::CheckpointSystemData&, *data);

			// これまでにいくつ読み出したか数えておく

			if ((++rest > 1 || tmp.isSynchronized()) && !tmp.isUnavailable()) {

				// 2 つ目のものか、
				// バッファとディスクの内容が完全に一致しているときのもので、
				// かつ、メタデータベースが利用可能であれば、
				// この時点ではシステムの一貫性は確実に保証されている

				if ((_starting = tmp.getFinishTimeStamp()).isIllegal())

					// 不正なタイムスタンプが得られた

					_SYDNEY_THROW0(Exception::LogItemCorrupted);

				// この時点で利用不可なデータベースの一覧を得る

				unavailableDatabase = tmp.getUnavailableDatabase();

				// この時点でヒューリスティックに解決済の
				// トランザクションブランチの情報を得て、
				// ヒューリスティックに解決済の
				// トランザクションブランチが存在するようにする

				Trans::Branch::redo(tmp);

				// この論理ログ(正確には、この論理ログの直後)から
				// 回復処理を開始すればよい

				return true;
			}

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::TimeStampAssign:
		{
			// 新たなタイムスタンプの生成を表す論理ログである

			const Trans::Log::TimeStampAssignData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TimeStampAssignData&, *data);

			// 生成されたタイムスタンプ値を覚えておく

			if ((_starting = tmp.getAssigned()).isIllegal())

				// 不正なタイムスタンプが得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::SchemaModify:
		{
			// スキーマの更新操作を表す論理ログである

			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()) &&
				!noUndoLSN.isFound(tmp.getEndStatementLSN()) &&
				!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

				// UNDO 済でないスキーマ操作は UNDO する

				Transaction::undoReorg(
					trans, *data, _database,
					!tmp.isUndoable() ||
					_committedLSN.isFound(tmp.getBeginTransactionLSN()));

			if (!tmp.isUndoable())

				// UNDO 不能なスキーマ操作は必ず REDO する

				_redoLSN.pushBack(lsn);

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeBegin:
		{
			// バージョンファイルの同期の開始を表す論理ログである
#ifdef DEBUG
			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);

			// 常に UNDO 不可である

			; _SYDNEY_ASSERT(!tmp.isUndoable());
#endif
			if (synchronized) {

				// 同期処理によってデータベースは更新されているか、
				// (同期処理の終了を表す論理ログが記録されていないから)
				// 同期処理中に障害が発生しているので、REDO する

				_redoLSN.pushBack(lsn);

				_recovered = false;
			}

			// 同期処理でデータベースが更新されたか忘れる

			synchronized = Boolean::Unknown;
			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeEnd:
		{
			// バージョンファイルの同期の終了を表す論理ログである

			const Checkpoint::Log::FileSynchronizeEndData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::FileSynchronizeEndData&, *data);

			// 同期処理でデータベースが更新されたか覚えておく

			synchronized = (tmp.isModified()) ? Boolean::True : Boolean::False;
			break;
		}
		case Trans::Log::Data::Category::BranchHeurDecide:
		case Trans::Log::Data::Category::BranchForget:

			// トランザクションブランチをヒューリスティックに解決した、
			// またはヒューリスティックに解決済なトランザクションブランチを
			// 抹消したことを表す論理ログである

			// 必ず REDO する
			//
			//【注意】	ただし、データベースを更新する
			//			論理ログファイルはリカバリする必要はない

			_redoLSN.pushBack(lsn);
			break;

#ifdef DEBUG
		case Trans::Log::Data::Category::TransactionPrepare:
			; _SYDNEY_ASSERT(false);
#endif
		}

		// 処理した論理ログのログシーケンス番号とログデータをおぼえておく

		_lsn = lsn;
		_logData = data;

		// 直前の論理ログのログシーケンス番号を得る
		{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		lsn = logFile->getPrevLSN(lsn);
		}
	}

	// 論理ログファイルの先頭まで読み出した
	//
	// 論理ログファイルの末尾から
	// 2 つ目のチェックポイント処理を表す論理ログが見つからないので、
	// 回復処理は論理ログファイルの先頭から行うことになる

	if (_starting.isIllegal()) {

		// 回復処理を開始する時点のタイムスタンプが得られていなければ、
		// システム用の論理ログを記録し始めてから、
		// 新たなタイムスタンプを生成していないということなので、
		// タイムスタンプファイルに記録されているタイムスタンプから
		// 回復処理を開始すればよい

		_starting = Trans::TimeStamp::getPersisted();
		return false;
	}

	return true;
}

//	FUNCTION public
//	Admin::Recovery::Database::findStartLSN --
//		通常のデータベースの回復処理の基点となる
//		論理ログのログシーケンス番号を求める
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Recovery::
Database::findStartLSN(Trans::Transaction& trans)
{
	// 記憶している情報の検証

	; _SYDNEY_ASSERT(_lsn == Trans::Log::IllegalLSN);
	; _SYDNEY_ASSERT(!_logData.isOwner());
	; _SYDNEY_ASSERT(!_starting.isIllegal());
	; _SYDNEY_ASSERT(_recovered);
	; _SYDNEY_ASSERT(_first);
	; _SYDNEY_ASSERT(_undoLSN.isEmpty());
	; _SYDNEY_ASSERT(_redoLSN.isEmpty());
	; _SYDNEY_ASSERT(_committedLSN.isEmpty());
	; _SYDNEY_ASSERT(_noRedoLSN.isEmpty());
	; _SYDNEY_ASSERT(_finishedLSN.isEmpty());
	; _SYDNEY_ASSERT(_runningLSN.isEmpty());
	; _SYDNEY_ASSERT(_lastMasterLSN == Trans::Log::IllegalLSN);

	Trans::Log::LSN				lsn;
	ModVector<Trans::Log::LSN>	rollbackedLSN;
	ModVector<Trans::Log::LSN>	noUndoLSN;

	Boolean::Value synchronized = Boolean::Unknown;

	bool replicationEnd = false;

	// データベース用の論理ログファイルのリカバリを実行し、
	// ログファイルの末尾に記録されている論理ログのログシーケンス番号を得る

	Trans::Log::AutoFile logFile(_database.getLogFile());
	
	{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		logFile->recover();
		lsn = logFile->getLastLSN();
		
		if (_slave)
		{
			// マスター側の論理ログの最後のログシーケンス番号を得る
			
			_lastMasterLSN = logFile->getMasterLSN();
			if (_lastMasterLSN == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogFileCorrupted);
		}

		// システムのタイムスタンプより、
		// データベースのタイムスタンプが大きかったら、
		// タイムスタンプファイルに格納されているタイムスタンプ値で上書きする
	
		Trans::TimeStamp t = logFile->getTimeStamp();

		if (logFile->getVersion() >= LogicalLog::VersionNumber::Second)
			if (!t.isIllegal()) _starting = t;
		else
			if (!t.isIllegal() && _starting < t) _starting = t;
	}
	
	// 論理ログファイルの先頭に向かって
	// ひとつひとつ論理ログを読み出していく

	while (lsn != Trans::Log::IllegalLSN) {

		// 論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data>	data;
		{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		data = logFile->load(lsn);
		}
		if (!data.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);

		if (data->getTimeStamp().isIllegal())

			// 不正なタイムスタンプが得られた

			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		if ((_slave == false || replicationEnd == true) &&
			data->getTimeStamp() < _starting)

			// 回復処理を開始する時点のタイムスタンプより
			// 前に記録された論理ログを読み出した

			// この論理ログ(正確には、この論理ログの直後)から
			// 回復処理を開始すればよい
			//
			// このとき UNDO が必要なのは、
			// この時点以降で終了しなかったスキーマ操作のみである

			goto unmount_logfile;

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::TransactionBegin:

			// トランザクションの開始を表す論理ログである

			if (_slave && !_finishedLSN.isFound(lsn))
			{
				// 復元する必要のあるトランザクション
				
				_runningLSN.pushBack(lsn);
			}
			break;
			
		case Trans::Log::Data::Category::TransactionCommit:
		{
			// トランザクションのコミットを表す論理ログである

			const Trans::Log::TransactionCommitData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionCommitData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)

				// 不正なログシーケンス番号が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// コミットされたトランザクションの開始を表す
			// 論理ログのログシーケンス番号を覚えておく

			_committedLSN.pushBack(tmp.getBeginTransactionLSN());
			_finishedLSN.pushBack(tmp.getBeginTransactionLSN());
			break;
		}
		case Trans::Log::Data::Category::TransactionRollback:
		{
			// トランザクションのロールバックを表す論理ログである

			const Trans::Log::TransactionRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionRollbackData&, *data);
			
			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// ロールバックされたトランザクションの開始を表す
			// 論理ログのログシーケンス番号を覚えておく

			rollbackedLSN.pushBack(tmp.getBeginTransactionLSN());
			_finishedLSN.pushBack(tmp.getBeginTransactionLSN());

			// 回復処理の開始時点からデータベースは更新されているので、
			// まず、その時点にデータベースを構成する論理ファイルを
			// リカバリする必要がある

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::TransactionPrepare:
		{
			// トランザクションのコミット準備完了を表す論理ログである

			const Trans::Log::TransactionPrepareData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionPrepareData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getXID().isIllegal() || tmp.getXID().isNull())

				// 不正なログシーケンス番号または
				// トランザクションブランチ識別子が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (!_committedLSN.isFound(tmp.getBeginTransactionLSN()) &&
				!rollbackedLSN.isFound(tmp.getBeginTransactionLSN())) {

				// 障害発生時までに終了していない
				//『コミット準備済』のトランザクションは、ロールバックする

				rollbackedLSN.pushBack(tmp.getBeginTransactionLSN());

				// ヒューリスティックに解決したことにする

				Trans::Branch::redo(
					Trans::Log::BranchHeurDecideData(
						tmp.getXID(), Trans::Branch::HeurDecision::Rollback));
			}

			//【注意】	コミット準備完了はデータベースを更新しないので、
			//			これだけだとリカバリの必要はない

			break;
		}
		case Trans::Log::Data::Category::StatementRollback:
		{
			// SQL 文のロールバックを表す論理ログである

			const Trans::Log::StatementRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::StatementRollbackData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (_committedLSN.isFound(tmp.getBeginTransactionLSN())) {
				if (!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

					// コミットされたトランザクションで
					// 実行された SQL 文のうち、ロールバックされたものは
					// REDO しないようにする必要がある
					//
					//【注意】	この SQL 文は UNDO もしないことになる

					_noRedoLSN.pushBack(tmp.getEndStatementLSN());

			} else if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()))
				if (!noUndoLSN.isFound(tmp.getEndStatementLSN()))

					// 障害発生時までに終了しなかったトランザクションで
					// 実行された SQL 文のうち、ロールバックされたものは
					// UNDO しないようにする必要がある

					noUndoLSN.pushBack(tmp.getEndStatementLSN());

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::CheckpointDatabase:
		{
			// データベースに関する
			// チェックポイント処理の終了を表す論理ログである

			const Checkpoint::Log::CheckpointDatabaseData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::CheckpointDatabaseData&, *data);

			if (tmp.getFinishTimeStamp().isIllegal())

				// 不正なタイムスタンプが得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// チェックポイント処理の終了時のタイムスタンプが
			// 回復処理を開始する時点のタイムスタンプ以下か調べる

			if (tmp.getFinishTimeStamp() <= _starting ||
				tmp.isSynchronized()) {

				// 回復するデータベースを更新したトランザクションのうち、
				// 障害発生するまでにコミットしなかったものの、
				// チェックポイント処理の直前に記録した
				// 論理ログのログシーケンス番号を求める

				const ModVector<Checkpoint::Log::CheckpointDatabaseData::
							TransactionInfo>& info = tmp.getTransactionInfo();
				const unsigned int n = info.getSize();

				for (unsigned int i = 0; i < n; ++i) {
					if (info[i]._beginLSN == Trans::Log::IllegalLSN ||
						info[i]._lastLSN == Trans::Log::IllegalLSN ||
						info[i]._preparedXID.isIllegal())

						// 不正なログシーケンス番号または
						// トランザクションブランチ識別子が得られた

						_SYDNEY_THROW0(Exception::LogItemCorrupted);

					// トランザクションが残っている場合には
					// リカバリは必要
					
					_recovered = false;

					if (!_committedLSN.isFound(info[i]._beginLSN)) {
						_undoLSN.pushBack(info[i]._lastLSN);

						if (!info[i]._preparedXID.isNull() &&
							!rollbackedLSN.isFound(info[i]._beginLSN))

							// ヒューリスティックに解決したことにする

							Trans::Branch::redo(
								Trans::Log::BranchHeurDecideData(
									info[i]._preparedXID,
									Trans::Branch::HeurDecision::Rollback));
					}

					if (_slave && !tmp.isTerminated())
					{
						// 終了時のログではないので、
						// 復元すべきトランザクションが記録されている

						if (!_finishedLSN.isFound(info[i]._beginLSN))
							
							// 終了していないトランザクションなので、
							// 復元する必要がある
							//
							//【注意】	終了しているトランザクションは
							//			自動リカバリで復元されるが、
							//			終了していないトランザクションは、
							//			自動リカバリでは undo されてしまうため、
							//			ここで復元する必要がある
						
							_runningLSN.pushBack(info[i]._beginLSN);
					}
				}

				// この論理ログ(正確には、この論理ログの直後)から
				// 回復処理を開始すればよい

				_starting = tmp.getFinishTimeStamp();

				if (_slave == false || replicationEnd == true ||
					!tmp.isTerminated())

					// スレーブデータベースではないか、
					// スレーブデータベースであっても、
					// すでにReplicationEndのログがあるか、
					// まだ、ReplicationEndのログに出会ってなくても、
					// このチェックポイント処理が終了時のものではないので、
					// これ以上論理ログをたどる必要はない
					
					goto unmount_logfile;
			}

			_recovered = false;
			break;
		}

		case Trans::Log::Data::Category::TupleModify:
		case Trans::Log::Data::Category::DriverModify:

			// タプルまたはドライバーの更新を表す論理ログである
			
			_recovered = false;

			break;

		case Trans::Log::Data::Category::XATransaction:
		{
			// 子サーバにトランザクションブランチを発行したことを表す
			// 論理ログである
			//
			// 実行したトランザクションが終了していなかったら、
			// 子サーバのこのトランザクションブランチが残っているかも
			// しれないので、REDO する

			const Trans::Log::XATransactionData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::XATransactionData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (!_finishedLSN.isFound(tmp.getBeginTransactionLSN()))
				_redoLSN.pushBack(lsn);

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::SchemaModify:
		{
			// スキーマの更新操作を表す論理ログである

			const Schema::LogData& tmp =
				_SYDNEY_DYNAMIC_CAST(const Schema::LogData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()) &&
				!noUndoLSN.isFound(tmp.getEndStatementLSN()) &&
				!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

				// UNDO 済でないスキーマ操作は UNDO する

				Transaction::undoReorg(
					trans, *data, _database,
					!tmp.isUndoable() ||
					_committedLSN.isFound(tmp.getBeginTransactionLSN()));

			switch (tmp.getSubCategory()) {
			case Schema::LogData::Category::Unmount:
			case Schema::LogData::Category::DropDatabase:

				// データベースをアンマウントまたは破棄を表す論理ログである
				//
				//【注意】	現状ではデータベースの破棄を表す論理ログは
				//			システム用の論理ログファイルにしか記録されない

				if (!tmp.isUndoable() ||
					(_committedLSN.isFound(tmp.getBeginTransactionLSN()) &&
					 !_noRedoLSN.isFound(tmp.getEndStatementLSN()))) {

					// これらの操作は REDO されるので、
					// これまで覚えてきた情報をすべて忘れる

					_undoLSN.clear();
					_redoLSN.clear();
					_committedLSN.clear();
					_noRedoLSN.clear();
					rollbackedLSN.clear();
					noUndoLSN.clear();

					if (!tmp.isUndoable())
						_redoLSN.pushBack(lsn);
					else
						_committedLSN.pushBack(tmp.getBeginTransactionLSN());
				}
				break;

			case Schema::LogData::Category::AlterDatabase_SetToMaster:

				// 非同期レプリケーションのスレーブデータベースから
				// マスターデータベースへの変更を表す論理ログである

				// マスターデータベースなので、トランザクションの復元は必要ない

				_runningLSN.clear();
				_slave = false;

				// このログ自体は REDO する必要はない

				_noRedoLSN.pushBack(tmp.getBeginTransactionLSN());

				break;

			default:
				if (!tmp.isUndoable())

					// UNDO 不能なスキーマ操作は必ず REDO する

					_redoLSN.pushBack(lsn);
			}

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeBegin:
		{
			// バージョンファイルの同期の開始を表す論理ログである
#ifdef DEBUG
			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);

			// 常に UNDO 不可である

			; _SYDNEY_ASSERT(!tmp.isUndoable());
#endif
			if (synchronized) {

				// 同期処理によってデータベースは更新されているか、
				// (同期処理の終了を表す論理ログが記録されていないから)
				// 同期処理中に障害が発生しているので、REDO する

				_redoLSN.pushBack(lsn);

				_recovered = false;
			}

			// 同期処理でデータベースが更新されたか忘れる

			synchronized = Boolean::Unknown;
			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeEnd:
		{
			// バージョンファイルの同期の終了を表す論理ログである

			const Checkpoint::Log::FileSynchronizeEndData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::FileSynchronizeEndData&, *data);

			// 同期処理でデータベースが更新されたか覚えておく

			synchronized = (tmp.isModified()) ? Boolean::True : Boolean::False;
			break;
		}
		case Trans::Log::Data::Category::StartBatch:
		{
			// バッチ処理の開始を表す論理ログである

			_recovered = false;
			break;
		}
		case Trans::Log::Data::Category::ReplicationEnd:
		{
			// レプリケーション処理スレッドが終了する時の論理ログである

			if (_slave == false)

				// マスター昇格済みのデータベース

				break;

			if (replicationEnd)

				// 最新のものだけ参照すればよい

				break;

			const Admin::Log::ReplicationEndData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Admin::Log::ReplicationEndData&, *data);

			// 終了時のトランザクションの状態はこのログに記録されているので、
			// このログまで参照すればよい

			if (_lastMasterLSN <= tmp.getLastMasterLSN())
			{
				// このログが記録されてから、マスター側のログが記録されていない
				// 新しくReplicationEndのログを記録する必要はない
				//
				//【注意】	論理ログへの記録は必要なものだけ行われるため、
				//			レプリケーション終了のログより、
				//			論理ログに記録されているマスター側のLSNが、
				//			小さいことはあり得る

				_slave = false;
				_lastMasterLSN = tmp.getLastMasterLSN();
			}
			else
			{
				// このログが記録されてから、マスター側のログが記録されている
				// 新しくReplicationEndのログを記録する必要がる

				const ModVector<Trans::Log::LSN>& beginLSN = tmp.getBeginLSN();
				ModVector<Trans::Log::LSN>::ConstIterator i
					= beginLSN.begin();
				
				for (; i != beginLSN.end(); ++i)
				{
					if (!rollbackedLSN.isFound(*i))
						
						// ロールバックされていないトランザクションなので、
						// 復元する必要がある
						
						_runningLSN.pushBack(*i);
				}
			}

			replicationEnd = true;

			break;
		}
		}

		// 処理した論理ログのログシーケンス番号とログデータをおぼえておく

		_lsn = lsn;
		_logData = data;

		// 直前の論理ログのログシーケンス番号を得る
		{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		lsn = logFile->getPrevLSN(lsn);
		}
	}

	// 回復処理は論理ログファイルの先頭から行うことになる

unmount_logfile:

	if (_slave)
	{
		// スレーブデータベースの場合、
		// 必要ならレプリケーションスレッド終了のログを書き出す

		Log::ReplicationEndData data;

		data.setLastMasterLSN(_lastMasterLSN);
		data.setBeginLSN(_runningLSN);
		
		Trans::AutoLatch latch(trans, logFile->getLockName());
		logFile->store(data);

		_slave = false;
		_runningLSN.clear();
	}

	// 論理ログファイルをアンマウントし、バッファリング内容をすべて破棄する
	//
	//【注意】	データベース用の論理ログファイルは、
	//			回復処理開始時点のデータベースが
	//			読み書き可または読み取り専用かにあわせてマウントされ、
	//			論理ログ専用または読み取り専用のバッファに読み込まれる
	//
	//			データベースを読み書き可に変更するとき、
	//			データベース用の論理ログファイルには以下の論理ログが記録される
	//
	//			START TRANSACTION
	//			ALTER DATABASE READ WRITE
	//			CHECKPOINT DATABASE
	//			COMMIT
	//
	//			その結果、データベースに関するチェックポイント処理の
	//			終了時の状態から回復処理が行われることになる
	//
	//			データベースを構成する論理ログファイルを含めた全ファイルは
	//			ALTER DATABASE READ WRITE の REDO 時にアンマウントされるが、
	//			回復処理では ALTER DATABASE READ WRITE は REDO されることはない
	//
	//			そこで、回復処理開始時の状態に論理ログファイルを合わせるために
	//			アンマウントし、バッファリング内容を破棄することにする

	logFile->unmount();
}

//	FUNCTION public
//	Admin::Recovery::Database::findStartLSN --
//		データベースのマウント時の回復処理の基点となる
//		論理ログのログシーケンス番号を求める
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			マウントを行うトランザクションのトランザクション記述子
//		Admin::Mount::Option::Value	option
//			マウントオプションを表す値
//		Boolean::Value&		unmounted
//			マウントしようとしているデータベースが
//			アンマウントされたものかが設定される
//		Boolean::Value&		backup
//			マウントしようとしているデータベースが
//			バックアップされたものかが設定される
//		Trans::TimeStamp&	mostRecent
//			マウントしようとしているデータベースの
//			最後のチェックポイント処理時のタイムスタンプを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Recovery::
Database::findStartLSN(Trans::Transaction& trans,
					   Mount::Option::Value option,
					   Boolean::Value& unmounted, Boolean::Value& backup,
					   Trans::TimeStamp& mostRecent)
{
	option &= Mount::Option::Mask;

	; _SYDNEY_ASSERT(unmounted == Boolean::Unknown);
	; _SYDNEY_ASSERT(backup == Boolean::Unknown);
	; _SYDNEY_ASSERT(mostRecent.isIllegal());

	// 記憶している情報の検証

	; _SYDNEY_ASSERT(_lsn == Trans::Log::IllegalLSN);
	; _SYDNEY_ASSERT(!_logData.isOwner());
	; _SYDNEY_ASSERT(_starting.isIllegal());
	; _SYDNEY_ASSERT(_recovered);
	_first = false;
	; _SYDNEY_ASSERT(_undoLSN.isEmpty());
	; _SYDNEY_ASSERT(_redoLSN.isEmpty());
	; _SYDNEY_ASSERT(_committedLSN.isEmpty());
	; _SYDNEY_ASSERT(_noRedoLSN.isEmpty());
	; _SYDNEY_ASSERT(_finishedLSN.isEmpty());
	; _SYDNEY_ASSERT(_runningLSN.isEmpty());
	; _SYDNEY_ASSERT(_lastMasterLSN == Trans::Log::IllegalLSN);

	Trans::Log::LSN				lsn;
	ModVector<Trans::Log::LSN>	rollbackedLSN;
	ModVector<Trans::Log::LSN>	noUndoLSN;

	unsigned int rest = 0;
	Boolean::Value synchronized = Boolean::Unknown;
	LogicalLog::VersionNumber::Value v;

	// ロールフォワードリカバリかどうか
	// オンラインバックアップしたものからロールフォワードリカバリする場合、
	// オンラインバックアップまでのログは通常のロールフォワードと同じ処理を
	// 行うが、それより前のログに関してはバックアップと同じ処理を実施する
	// 必要がある
	// そのため、それをこのフラグで管理する
	
	bool backup_rollforward = false;

	// レプリケーション終了のログが現れたか否か
	
	bool replicationEnd = false;

	// マスター昇格のログが現れたか否か
	
	bool setToMaster = false;

	// 基本的にはリカバリは必要
	
	_recovered = false;

	// データベース用の論理ログファイルの末尾に記録されている
	// 論理ログのログシーケンス番号を得る

	Trans::Log::AutoFile logFile(_database.getLogFile());
	Trans::Log::LSN lastLSN;
	
	{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		logFile->recover();
		lsn = lastLSN = logFile->getLastLSN();

		// 回復処理の起点となるタイムスタンプ値を得る
		// 停止状態でバックアップされたものをマウントする場合には、
		// 以下のタイムスタンプが利用される
	
		_starting = logFile->getTimeStamp();

		// バージョンを得る

		v = logFile->getVersion();

		if (v >= LogicalLog::VersionNumber::Second)

			mostRecent = _starting;
	}

	if (_slave && v < LogicalLog::VersionNumber::Second)

		// スレーブデータベースとしてマウントきるのは、
		// v16.4 以降のデータベース
		
		_SYDNEY_THROW0(Exception::NotSupported);
	
	// 論理ログファイルの先頭に向かって
	// ひとつひとつ論理ログを読み出していく

	while (lsn != Trans::Log::IllegalLSN) {

		; _SYDNEY_ASSERT(unmounted != Boolean::True);

		// 論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data> data;
		{
			Trans::AutoLatch latch(trans, logFile->getLockName());
			data = logFile->load(lsn);
		}
		if (!data.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);

		if (data->getTimeStamp().isIllegal())

			// 不正なタイムスタンプが得られた

			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		if (data->getTimeStamp() < _starting)

			// バックアップされたデータベースをマウントするために
			// 回復処理を行う必要があるが、
			// 回復処理を開始する時点のタイムスタンプより
			// 前に記録された論理ログが読み出した
			//
			// この論理ログ(正確には、この論理ログの直後)から
			// 回復処理を開始すればよい

			goto unmount_logfile;

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::TransactionBegin:

			// トランザクションの開始を表す論理ログである

			if (_slave)
			{
				// トランザクションの終了を表す論理ログがなければ、
				// 再現するトランザクションとなる

				if (!_finishedLSN.isFound(lsn))

					_runningLSN.pushBack(lsn);
			}
			break;
			
		case Trans::Log::Data::Category::TransactionCommit:
		{
			// トランザクションのコミットを表す論理ログである

			const Trans::Log::TransactionCommitData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionCommitData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)

				// 不正なログシーケンス番号が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// コミットされたトランザクションの開始を表す
			// 論理ログのログシーケンス番号を覚えておく

			_committedLSN.pushBack(tmp.getBeginTransactionLSN());
			_finishedLSN.pushBack(tmp.getBeginTransactionLSN());

			break;
		}
		case Trans::Log::Data::Category::TransactionRollback:
		{
			// トランザクションのロールバックを表す論理ログである

			const Trans::Log::TransactionRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionRollbackData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// ロールバックされたトランザクションの開始を表す
			// 論理ログのログシーケンス番号を覚えておく

			rollbackedLSN.pushBack(tmp.getBeginTransactionLSN());
			_finishedLSN.pushBack(tmp.getBeginTransactionLSN());

			break;
		}
		case Trans::Log::Data::Category::TransactionPrepare:
		{
			// トランザクションのコミット準備完了を表す論理ログである

			// マウントしようとしているのはアンマウントされたデータベースでない
			//
			//【注意】	アンマウント後に実行できるのはマウントだけであり、
			//			MOUNT 文はトランザクションブランチで実行できないため

			unmounted = Boolean::False;

			const Trans::Log::TransactionPrepareData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionPrepareData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (!_committedLSN.isFound(tmp.getBeginTransactionLSN()) &&
				!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()))

				// バックアップを開始後に終了していない
				//『コミット準備済』のトランザクションは、ロールバックする
				//
				//【注意】	このトランザクションを
				//			ヒューリスティックに解決したことにしない

				rollbackedLSN.pushBack(tmp.getBeginTransactionLSN());

			break;
		}
		case Trans::Log::Data::Category::StatementRollback:
		{
			// SQL 文のロールバックを表す論理ログである

			const Trans::Log::StatementRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::StatementRollbackData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (_committedLSN.isFound(tmp.getBeginTransactionLSN())) {
				if (!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

					// コミットされたトランザクションで
					// 実行された SQL 文のうち、ロールバックされたものは
					// REDO しないようにする必要がある
					//
					//【注意】	この SQL 文は UNDO もしないことになる

					_noRedoLSN.pushBack(tmp.getEndStatementLSN());

			} else if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()))
				if (!noUndoLSN.isFound(tmp.getEndStatementLSN()))

					// バックアップ開始時か障害発生時までに
					// 終了しなかったトランザクションで
					// 実行された SQL 文のうち、ロールバックされたものは
					// UNDO しないようにする必要がある

					noUndoLSN.pushBack(tmp.getEndStatementLSN());

			break;
		}
		case Trans::Log::Data::Category::CheckpointDatabase:
		{
			// データベースに関する
			// チェックポイント処理の終了を表す論理ログである

			const Checkpoint::Log::CheckpointDatabaseData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::CheckpointDatabaseData&, *data);

			if (tmp.getFinishTimeStamp().isIllegal() ||
				tmp.getFinishTimeStamp() < _starting)

				// 不正なタイムスタンプが得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			// マウントしようとしているのは
			// アンマウントされたデータベースでない

			unmounted = Boolean::False;

			if (backup == Boolean::False &&
				!(option & Mount::Option::WithRecovery))

				// アンマウントされたデータベースをマウントしようとしているが
				// アンマウントを表す論理ログがまだ見つかっていない
				//
				//【注意】	アンマウント後に実行できるのはマウントだけである

				_SYDNEY_THROW1(
					Exception::DatabaseNotMountable, _database.getName());

			if (v < LogicalLog::VersionNumber::Second &&
				backup == Boolean::True) {

				//【注意】旧バージョンのみ
				
				// バックアップされたデータベースをマウントしようとしているとき

				; _SYDNEY_ASSERT(!mostRecent.isIllegal());
				; _SYDNEY_ASSERT(!_starting.isIllegal());
				; _SYDNEY_ASSERT(!_recovered);

				if (_starting != tmp.getFinishTimeStamp())
					if (tmp.isSynchronized())

						// バックアップの開始を表す論理ログに記録されている
						// 回復処理を開始する位置がおかしい

						_SYDNEY_THROW0(Exception::LogItemCorrupted);
					else 

						// さらに読み戻し続ける

						break;
				
			} else if (v < LogicalLog::VersionNumber::Second &&
					   (++rest > 1 || tmp.isSynchronized())) {

				//【注意】旧バージョンのみ

				// 2つ目のものか、
				// バッファとディスクの内容が完全に一致しているとき

				// この論理ログ(正確にはこの論理ログの直後)から
				// 回復処理を開始すればよい

				mostRecent = _starting = tmp.getFinishTimeStamp();

			} else {

				if (_starting != tmp.getFinishTimeStamp())
					
					// さらに読み戻し続ける

					break;
			}

			// マウントするデータベースを更新したトランザクションで、
			// バックアップの開始時点までに終了しなかったものの、
			// チェックポイント処理の直前に記録した
			// 論理ログのログシーケンス番号を求める

			const ModVector<Checkpoint::Log::CheckpointDatabaseData::
						TransactionInfo>& info = tmp.getTransactionInfo();
			const unsigned int n = info.getSize();

			for (unsigned int i = 0; i < n; ++i) {
				if (info[i]._beginLSN == Trans::Log::IllegalLSN ||
					info[i]._lastLSN == Trans::Log::IllegalLSN)

					// 不正なログシーケンス番号が得られた

					_SYDNEY_THROW0(Exception::LogItemCorrupted);

				if (!_committedLSN.isFound(info[i]._beginLSN))

					// コミットされてないので、Undoする必要あり
					
					_undoLSN.pushBack(info[i]._lastLSN);

				if (_slave && !_finishedLSN.isFound(info[i]._beginLSN))

					// トランザクションが終了していないので、
					// 再実行する必要あり
					
					_runningLSN.pushBack(info[i]._beginLSN);

			}

			if (_slave)

				_lastMasterLSN = lastLSN;

			goto unmount_logfile;
		}
		case Trans::Log::Data::Category::TupleModify:
		case Trans::Log::Data::Category::DriverModify:
		case Trans::Log::Data::Category::StartBatch:

			// タプル(またはドライバー)の更新を表す論理ログである
			// またはバッチ処理の開始を表す論理ログである

			// マウントしようとしているのは
			// アンマウントされたデータベースでない
			//
			//【注意】	アンマウント後に実行できるのはマウントだけである
			//
			//【注意】	タプルを更新する SQL 文がコミットされてようが、
			//			ロールバックされてようが関係ない

			unmounted = Boolean::False;

			if (backup == Boolean::False &&
				!(option & Mount::Option::WithRecovery))

				// マウントしようとしているのは
				// アンマウントされたデータベースでなければならないのに
				// アンマウントを表す論理ログがまだ見つかっていない

				_SYDNEY_THROW1(
					Exception::DatabaseNotMountable, _database.getName());

			break;

		case Trans::Log::Data::Category::SchemaModify:
		{
			// スキーマの更新操作を表す論理ログである

			const Schema::LogData& tmp =
				_SYDNEY_DYNAMIC_CAST(const Schema::LogData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			switch (tmp.getSubCategory()) {
			case Schema::LogData::Category::StartBackup:

				// バックアップの開始を表す論理ログである

				if (backup == Boolean::Unknown) {

					// バックアップの開始を表す論理ログを見つけた

					const bool usingSnapshot =
						option & Mount::Option::UsingSnapshot;

					const bool withRecovery =
						option & Mount::Option::WithRecovery;

					if (usingSnapshot && withRecovery)

						// usingSnapshot のマウントは、
						// ロールフォワードリカバリでは認めない

						_SYDNEY_THROW1(
							Exception::DatabaseNotMountable,
							_database.getName());

					if (usingSnapshot && _slave)

						// usingSnapshot のマウントは、
						// スレーブデータベースでは認めない

						_SYDNEY_THROW0(Exception::NotSupported);

					if (withRecovery &&
						v >= LogicalLog::VersionNumber::Second) {

						// 論理ログの最後のチェックポイント処理時
						// タイムスタンプと、タイムスタンプファイルの
						// タイムスタンプが同じかどうかチェックする

						Trans::TimeStamp t = Trans::TimeStamp(
							_SYDNEY_DYNAMIC_CAST(
								const Common::UnsignedInteger64Data&,
								*tmp[1]).getValue());

						if (t.isIllegal())

							// 不正なタイムスタンプが得られた

							_SYDNEY_THROW0(Exception::LogItemCorrupted);

						if (t != _starting) {

							// このバックアップ開始のログは、
							// マウントしようとしているデータベースを
							// バックアップしたときのものではない
							// -> 該当する論理ログの探索を続ける

							unmounted = Boolean::False;
				
							break;
						}
					}
					else if (_committedLSN.isFound(
								 tmp.getBeginTransactionLSN()) ||
							 rollbackedLSN.isFound(
								 tmp.getBeginTransactionLSN()) ||
							 _noRedoLSN.isFound(lsn))
					{
						// このデータベースはバックアップされたものではない

						backup = Boolean::False;

						break;
					}

					// このバックアップを開始した時点の状態に回復する

					// バックアップを開始したトランザクションの
					// 開始時の状態にマウントするときは、
					// 論理ログ中の配列の 2 番目の要素である
					// そのトランザクションの開始時タイムスタンプを、
					// バックアップの開始時点の状態にマウントするときは、
					// 論理ログ中の配列の 1 番目の要素である
					// この論理ログを記録した時点から
					// 前々回のチェックポイント処理の
					// 終了時タイムスタンプを、それぞれ得る

					_starting = _SYDNEY_DYNAMIC_CAST(
						const Common::UnsignedInteger64Data&,
						*tmp[1 + usingSnapshot]).getValue();

					if (_starting.isIllegal())

						// 不正なタイムスタンプが得られた

						if (usingSnapshot)

							// バックアップ時にスナップショットが破棄された
							// データベースをマウントしようとしている

							_SYDNEY_THROW0(Exception::SnapshotDiscarded);
						else
							_SYDNEY_THROW0(Exception::LogItemCorrupted);

					// マウントするデータベースの
					// 最後のチェックポイント処理時のタイムスタンプを得る

					mostRecent = _SYDNEY_DYNAMIC_CAST(
						const Common::UnsignedInteger64Data&,
						*tmp[!usingSnapshot]).getValue();

					if (mostRecent.isIllegal())

						// 不正なタイムスタンプが得られた

						_SYDNEY_THROW0(Exception::LogItemCorrupted);

					if (withRecovery && unmounted != Boolean::Unknown)

						// オンラインバックアップしたものを
						// ロールフォワードリカバリしている

						backup_rollforward = true;

					// バックアップされたデータベースをマウントしている

					backup = Boolean::True, unmounted = Boolean::False;

					if (backup_rollforward == false && _slave == false)

						// スレーブデータベースを作らない場合は、
						// バックアップを開始後の更新操作は REDO しない

						_committedLSN.clear();

					if (usingSnapshot) {

						// バックアップを開始したトランザクションの
						// 開始時点の状態にマウントするときは、
						// リカバリの必要はない

						_recovered = true;
						return;
					}
					
					// 回復処理を開始するチェックポイント処理の終了を表す
					// 論理ログまで読み戻し続ける

					if (_slave)
						_lastMasterLSN = lastLSN;
				}

				break;

			case Schema::LogData::Category::Unmount:
			case Schema::LogData::Category::AlterDatabase_ReadOnly:

				// データベースのアンマウントまたは
				// 読み取り専用への変更を表す論理ログである

				if (_committedLSN.isFound(tmp.getBeginTransactionLSN())) {

					// アンマウントまたは
					// 読み取り専用への変更はコミットされている
					//
					//【注意】	これらは暗黙のトランザクションで
					//			実行されることを仮定している

					if (unmounted == Boolean::False &&
						!(option & Mount::Option::WithRecovery))

						// ロールフォワードリカバリのためのマウントである
						// with recovery が必要

						_SYDNEY_THROW1(
							Exception::DatabaseNotMountable,
							_database.getName());

					if (v >= LogicalLog::VersionNumber::Second) {
						
						// データベースのアンマウントまたは
						// 読み取り専用への変更を表す論理ログをはじめて見つけた

						if ((_logData->getTimeStamp()).isIllegal())

							// 不正なタイムスタンプが得られた

							_SYDNEY_THROW0(Exception::LogItemCorrupted);

						if (tmp.getTimeStamp() != _starting) {

							// このアンマウントのログは、
							// マウントしようとしている
							// データベースをアンマウントしたときのものではない
							// -> 該当する論理ログの探索を続ける

							unmounted = Boolean::False;

							break;
						}

						if (unmounted == Boolean::Unknown) {
						
							// アンマウントまたは
							// 読み取り専用へ変更したデータをマウントしている
						
							unmounted = Boolean::True, backup = Boolean::False;

							// マウントされるデータベースは整合性のある
							// 状態なので、更新操作は REDO しないし、
							// リカバリの必要もない

							_committedLSN.clear();
							_recovered = true;

							if (_slave)
							{
								// アンマウントされているデータベースから
								// スレーブデータベースを作る場合は、
								// トランザクションの復元は必要ない
								// 論理ログファイルの末尾に記録されているログの
								// 次から受け取ればいい

								_runningLSN.clear();
								_lastMasterLSN = lastLSN;
							}
						}
					
					} else {

						// 正常にアンマウントされていないので、
						// スレーブデータベースは作成できない

						if (_slave)

							_SYDNEY_THROW0(Exception::NotSupported);

						// データベースのアンマウントまたは
						// 読み取り専用への変更を表す論理ログをはじめて見つけた
						
						if ((_starting = _logData->getTimeStamp()).isIllegal())
							
							// 不正なタイムスタンプが得られた

							_SYDNEY_THROW0(Exception::LogItemCorrupted);

						// マウントするデータベースの
						// 最後のチェックポイント処理時の
						// タイムスタンプを得る

						mostRecent = _SYDNEY_DYNAMIC_CAST(
							const Common::UnsignedInteger64Data&,
							*tmp[(tmp.getSubCategory() ==
								  Schema::LogData::Category::Unmount) ?
								 Schema::Database::Log::Unmount::MostRecent :
								 Schema::Database::Log::Alter::MostRecent]).
							getValue();

						if (mostRecent.isIllegal())

							// 不正なタイムスタンプが得られた

							_SYDNEY_THROW0(Exception::LogItemCorrupted);
						
						// アンマウントまたは
						// 読み取り専用へ変更したデータをマウントしている
						
						unmounted = Boolean::True, backup = Boolean::False;

						// マウントされるデータベースは整合性のある
						// 状態なので、更新操作は REDO しないし、
						// リカバリの必要もない

						_committedLSN.clear();
						_recovered = true;
					}

					return;
				}
				break;
			}

			switch (tmp.getSubCategory()) {
			case Schema::LogData::Category::Mount:

				// データベースのマウントを表す論理ログである

				if (backup == Boolean::Unknown ||
					unmounted == Boolean::Unknown) {

					; _SYDNEY_ASSERT(backup != Boolean::True &&
									 unmounted != Boolean::True);

					if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()))

						// ロールバックされていないので、
						// マウントしようとしているのは
						// アンマウントされたデータベースでも
						// バックアップされたデータベースでもない
						//
						//【注意】	暗黙のトランザクションで
						//			実行されることを仮定している

						unmounted = Boolean::False;
				}
				break;

			default:

				// それ以外のスキーマ操作を表す論理ログである

				if (unmounted == Boolean::Unknown)
					unmounted = Boolean::False;
			}

			if (backup == Boolean::False &&
				!(option & Mount::Option::WithRecovery))
				_SYDNEY_THROW1(
					Exception::DatabaseNotMountable, _database.getName());

			if (tmp.getSubCategory() ==
				Schema::LogData::Category::AlterDatabase_SetToMaster)
			{
				// マスターにするログは、マウント時はRedoしない

				setToMaster = true;

				_noRedoLSN.pushBack(tmp.getEndStatementLSN());
			}
				
			if (!rollbackedLSN.isFound(tmp.getBeginTransactionLSN()) &&
				!noUndoLSN.isFound(tmp.getEndStatementLSN()) &&
				!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

				// UNDO 済でないスキーマ操作は UNDO する
				// バックアップ以外でここにくるのは、ロールフォワードのみ

				Transaction::undoReorg(
					trans, *data, _database,
					!tmp.isUndoable() ||
					_committedLSN.isFound(tmp.getBeginTransactionLSN()),
					(backup == Boolean::True) ? false : true);

			if (!tmp.isUndoable())

				// UNDO 不能なスキーマ操作は必ず REDO する

				_redoLSN.pushBack(lsn);

			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeBegin:
		{
			// バージョンファイルの同期の開始を表す論理ログである

			// マウントしようとしているのは
			// アンマウントされたデータベースでない

			unmounted = Boolean::False;

			if (backup == Boolean::False &&
				!(option & Mount::Option::WithRecovery))
				_SYDNEY_THROW1(
					Exception::DatabaseNotMountable, _database.getName());
#ifdef DEBUG
			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);

			// 常に UNDO 不可である

			; _SYDNEY_ASSERT(!tmp.isUndoable());
#endif
			if (synchronized != Boolean::False) {

				// 同期処理によってデータベースは更新されているか、
				// (同期処理の終了を表す論理ログが記録されていないから)
				// 同期処理中に障害が発生しているので、REDO する

				_redoLSN.pushBack(lsn);

			}

			// 同期処理によってデータベースが更新されたかを忘れる

			synchronized = Boolean::Unknown;
			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeEnd:
		{
			// バージョンファイルの同期の終了を表す論理ログである

			// マウントしようとしているのは
			// アンマウントされたデータベースでない

			unmounted = Boolean::False;

			if (backup == Boolean::False &&
				!(option & Mount::Option::WithRecovery))
				_SYDNEY_THROW1(
					Exception::DatabaseNotMountable, _database.getName());

			const Checkpoint::Log::FileSynchronizeEndData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Checkpoint::Log::FileSynchronizeEndData&, *data);

			// 同期処理でデータベースが更新されたか覚えておく

			synchronized = (tmp.isModified()) ? Boolean::True : Boolean::False;
			break;
		}
		case Trans::Log::Data::Category::ReplicationEnd:
		{
			// レプリケーション処理スレッドが終了する時の論理ログである

			if (replicationEnd)

				// 最新のものだけ参照すればよい

				break;

			const Admin::Log::ReplicationEndData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Admin::Log::ReplicationEndData&, *data);

			// 終了時のトランザクションの状態はこのログに記録されている
			// マウントされるということは、マスターに昇格済みのはず

			if (setToMaster == false)
				
				// マスターに昇格されていないデータベースはマウントできない
				
				_SYDNEY_THROW0(Exception::NotSupported);

			// ここに記録されているトランザクションは
			// すべてロールバックされるべきもの

			const ModVector<Trans::Log::LSN>& beginLSN = tmp.getBeginLSN();
			ModVector<Trans::Log::LSN>::ConstIterator i
				= beginLSN.begin();

			for (; i != beginLSN.end(); ++i)
			{
				rollbackedLSN.pushBack(*i);
				_finishedLSN.pushBack(*i);
			}
		}
		}
		
		// 処理した論理ログのログシーケンス番号とログデータをおぼえておく

		_lsn = lsn;
		_logData = data;

		{
			// 直前の論理ログのログシーケンス番号を得る
			Trans::AutoLatch latch(trans, logFile->getLockName());
			lsn = logFile->getPrevLSN(lsn);
		}
	} while (lsn != Trans::Log::IllegalLSN) ;

	// 論理ログファイルを先頭まで読み出した

	if (backup != Boolean::True)
		if (option & Mount::Option::WithRecovery) {

			// 障害回復してでもマウントする指定がされている場合、
			// 最後のチェックポイント処理時のタイムスタンプと、
			// 回復処理を開始する位置を求める

			if (mostRecent.isIllegal() &&
				(mostRecent = logFile->getTimeStamp()).isIllegal())

					// 不正なタイムスタンプが得られた

					_SYDNEY_THROW0(Exception::LogItemCorrupted);
		} else 

			// マウントしようとしているデータベースは
			// バックアップされたものでもアンマウントされたものでもない

			_SYDNEY_THROW1(
				Exception::DatabaseNotMountable, _database.getName());

unmount_logfile:

	if (_slave && _lastMasterLSN == Trans::Log::IllegalLSN)
	{
		// スレーブデータベースを作成できるデータベースではない

		_SYDNEY_THROW1(Exception::DatabaseNotMountable, _database.getName());
	}

	// 論理ログファイルをアンマウントし、バッファリング内容をすべて破棄する

	logFile->unmount();

	if (backup_rollforward == true)

		// オンラインバックアップしたものからロールフォワードリカバリする場合、
		// ここから先の処理は、ロールフォワードリカバリと同じにする必要がある
		// よって、backup を false にする

		backup = Boolean::False;
}

//	FUNCTION public
//	Admin::Recovery::Database::undoAll --
//		スキーマおよび通常のデータベースの回復処理として、
//		すべての更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Recovery::
Database::undoAll(Trans::Transaction& trans)
{
	const unsigned int n = _undoLSN.getSize();
	if (!n)

		// UNDO すべき操作はない

		return;

	// 最初に UNDO する操作を表す論理ログの
	// ログシーケンス番号を降順にソートする

	ModMap<Trans::Log::LSN, Trans::Log::LSN, ModGreater<Trans::Log::LSN> >
		descMap;

	for (unsigned int i = 0; i < n; ++i)
		(void) descMap.insert(_undoLSN[i], _undoLSN[i]);

	// UNDO されていない操作がある限り、
	// 論理ログファイルの先頭に向かって
	// UNDO する操作を表す論理ログを読み出していく

	while (descMap.getSize()) {

		// UNDO の結果、論理ログファイルの属性が変更される可能性もあるので、
		// 一操作を UNDO するたびに論理ログファイル記述子を生成し、破棄する

		Trans::Log::AutoFile logFile(_database.getLogFile());

		// UNDO する操作を表す論理ログのうち、
		// 論理ログファイルの末尾に最も近い位置に
		// 記録されているもののログシーケンス番号を得る

		Trans::Log::LSN	lsn = (*descMap.begin()).first;
		; _SYDNEY_ASSERT(lsn != Trans::Log::IllegalLSN);

		// 論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data>	data;
		{
		Trans::AutoLatch latch(trans, logFile->getLockName());
		data = logFile->load(lsn);
		}
		if (!data.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::SchemaModify:
		{
			// スキーマの更新を表す論理ログである

			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);

			if (tmp.isUndoable())

				// スキーマの更新を UNDO する

				Transaction::undoReorg(trans, *data, _database,
									   false, _rollforward);
			else

				// UNDO 不能なスキーマの更新は REDO する

				Transaction::redoReorg(trans, *data, _database, _rollforward);

			//【注意】	あるデータベースに対して、
			//			トランザクション中に複数回実行可能な
			//			スキーマ操作がないことに依存している

			break;
		}
		case Trans::Log::Data::Category::FileSynchronizeBegin:
		case Trans::Log::Data::Category::FileSynchronizeEnd:
		{
			// バージョンファイルの同期の開始、終了を表す論理ログである
#ifdef DEBUG
			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);
			; _SYDNEY_ASSERT(!tmp.isUndoable());
#endif
			// なにもすることはない
			//
			//【注意】	バージョンファイルの同期処理中に
			//			チェックポイント処理は行われないので、
			//			実際はありえないはず

			break;
		}
		case Trans::Log::Data::Category::TupleModify:
		case Trans::Log::Data::Category::XATransaction:

			if (data->getCategory() == Trans::Log::Data::Category::TupleModify)
			{
				// タプルの更新を UNDO する

				Transaction::undoTuple(trans, *data, _database);
			}
			else
			{
				if (_mount == false)
				{
					// データベースのマウントでは
					// 子サーバのトランザクションブランチのケアは行わない
					
					const Trans::Log::InsideTransactionData& tmp =
						_SYDNEY_DYNAMIC_CAST(
							const Trans::Log::InsideTransactionData&, *data);
				
					if (!_finishedLSN.isFound(tmp.getBeginTransactionLSN()))
					{
						// 子サーバのトランザクションブランチを rollback する

						try
						{
							// 例外が発生しても無視する
						
							DServer::Branch::rollback(trans, *data, _database);
						}
						catch (...) {}
					}
				}
			}
			// thru

		case Trans::Log::Data::Category::StatementCommit:
		case Trans::Log::Data::Category::TransactionPrepare:
		{
			const Trans::Log::InsideTransactionData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::InsideTransactionData&, *data);

			// この論理ログを記録したトランザクションが
			// 直前に記録した論理ログのログシーケンス番号を得る

			lsn = tmp.getBackwardLSN();
			if (lsn == Trans::Log::IllegalLSN ||
				tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)

				// 不正なログシーケンス番号が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (lsn != tmp.getBeginTransactionLSN())

				// 直前に記録した論理ログはトランザクションの開始を
				// 表さないので、それが表す操作を後で UNDO できるように、
				// そのログシーケンス番号を記憶しておく
				//
				//【注意】	SQL 文のコミットを表す論理ログの直前に記録した
				//			論理ログがトランザクションの開始を表すことは、
				//			実際は有り得ないはず

				(void) descMap.insert(lsn, lsn);
			break;
		}
		case Trans::Log::Data::Category::StatementRollback:
		{
			// SQL 文のロールバックを表す論理ログである

			const Trans::Log::StatementRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::StatementRollbackData&, *data);

			// この論理ログを記録したトランザクションが
			// 直前に実行した SQL 文の終了を表す
			// 論理ログのログシーケンス番号を得る

			lsn = tmp.getEndStatementLSN();
			if (lsn == Trans::Log::IllegalLSN ||
				tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (lsn != tmp.getBeginTransactionLSN())

				// 直前に実行した SQL 文の終了を表す論理ログは
				// トランザクションの開始を表さないので、
				// それが表す操作を後で UNDO できるように
				// そのログシーケンス番号を記憶しておく

				(void) descMap.insert(lsn, lsn);
			break;
		}
		case Trans::Log::Data::Category::TransactionBegin:

			// トランザクションの開始を表す論理ログである

			// なにもすることはない
			//
			//【注意】	トランザクションの開始を表す
			//			論理ログのログシーケンス番号は、
			//			チェックポイントを表す論理ログの生成時や、
			//			UNDO 中の処理で除去しているので、実際はありえないはず

			break;

		case Trans::Log::Data::Category::DriverModify:

			// ドライバーの更新を UNDO する

			Transaction::undoDriver(trans, *data, _database);

			break;

		case Trans::Log::Data::Category::StartBatch:

			// バッチ処理の開始を表す論理ログである
			// -> 何もしない

			break;

		default:

			// トランザクション中に記録しない種類の論理ログが記録されている

			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		}

		// UNDO した操作を表す論理ログのログシーケンス番号を忘れる

		descMap.erase(descMap.begin());
	}

	// 記憶している最初に UNDO すべき操作をすべて忘れる

	_undoLSN.clear();
}

//	FUNCTION public
//	Admin::Recovery::Database::redoAll --
//		スキーマおよび通常のデータベースの回復処理として、
//		更新操作をすべて REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Recovery::
Database::redoAll(Trans::Transaction& trans)
{
	while (!isRedone())
		(void) redo(trans);
}

//	FUNCTION public
//	Admin::Recovery::Database::redo --
//		スキーマおよび通常のデータベースの回復処理として、更新操作を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//
//	RETURN
//		0 以外の値
//			生成またはマウントしたデータベースの
//			スキーマオブジェクトを格納する領域の先頭アドレス
//		0
//			データベースの生成またはマウント以外の操作を REDO した
//
//	EXCEPTIONS

Recovery::Database*
Recovery::
Database::redo(Trans::Transaction& trans)
{
	// 記憶している情報の検証

	; _SYDNEY_ASSERT(_undoLSN.isEmpty());

	ModAutoPointer<Recovery::Database>	dbRecovery;

	if (isRedone())

		// REDO すべきトランザクションまたは操作はない

		return dbRecovery.release();

	; _SYDNEY_ASSERT(_lsn != Trans::Log::IllegalLSN);
	; _SYDNEY_ASSERT(_logData.isOwner());

	switch (_logData->getCategory()) {
	case Trans::Log::Data::Category::TransactionCommit:
	{
		// トランザクションのコミットを表す論理ログである

		const Trans::Log::TransactionCommitData& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Trans::Log::TransactionCommitData&, *_logData);

		// コミットされたトランザクション中の操作は
		// すべて REDO し終えたので、
		// その開始を表す論理ログのログシーケンス番号を忘れる

		if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN)

			// 不正なログシーケンス番号が得られた

			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		ModVector<Trans::Log::LSN>::Iterator ite(
			_committedLSN.find(tmp.getBeginTransactionLSN()));
		; _SYDNEY_ASSERT(ite != _committedLSN.end());
		(void) _committedLSN.erase(ite);
		break;
	}
	case Trans::Log::Data::Category::TupleModify:
	{
		// タプルの更新を表す論理ログである

		const Trans::Log::ModificationData& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Trans::Log::ModificationData&, *_logData);

		if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
			tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		; _SYDNEY_ASSERT(tmp.isUndoable());

		if (_committedLSN.isFound(tmp.getBeginTransactionLSN()) &&
			!_noRedoLSN.isFound(tmp.getEndStatementLSN()))

			// コミットされているトランザクションでの
			// ロールバックされていない SQL 文でのタプルの更新は REDO する

			Transaction::redoTuple(trans, *_logData, _database);
		break;
	}
	case Trans::Log::Data::Category::SchemaModify:
	{
		// スキーマの更新を表す論理ログである

		const Schema::LogData& tmp =
			_SYDNEY_DYNAMIC_CAST(const Schema::LogData&, *_logData);

		if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
			tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		ModVector<Trans::Log::LSN>::Iterator ite(_redoLSN.find(_lsn));
		if (ite != _redoLSN.end()) {

			// UNDO 不能なスキーマの更新は REDO する

			; _SYDNEY_ASSERT(!tmp.isUndoable());

			dbRecovery = Transaction::redoReorg(
				trans, *_logData, _database, _rollforward);

			(void) _redoLSN.erase(ite);

		} else if (_committedLSN.isFound(tmp.getBeginTransactionLSN()) &&
				   !_noRedoLSN.isFound(tmp.getEndStatementLSN()))

			// コミットされているトランザクションでの
			// ロールバックされていない SQL 文でのスキーマの更新は REDO する

			dbRecovery = Transaction::redoReorg(
				trans, *_logData, _database, _rollforward);
		break;
	}
	case Trans::Log::Data::Category::FileSynchronizeBegin:
	{
		// バージョンファイルの同期の開始を表す論理ログである
#ifdef DEBUG
		const Trans::Log::ModificationData& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Trans::Log::ModificationData&, *_logData);
		; _SYDNEY_ASSERT(!tmp.isUndoable());
#endif
		ModVector<Trans::Log::LSN>::Iterator ite(_redoLSN.find(_lsn));
		if (ite != _redoLSN.end())

			// データベースを更新したバージョンファイルの同期は REDO する

			(void) _redoLSN.erase(ite);
		break;
	}
	case Trans::Log::Data::Category::BranchHeurDecide:
	{
		// トランザクションブランチを
		// ヒューリスティックに解決したことを表す論理ログである

		const Trans::Log::BranchHeurDecideData& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Trans::Log::BranchHeurDecideData&, *_logData);

		// ヒューリスティックに解決済のトランザクションブランチとして記憶する

		Trans::Branch::redo(tmp);

		ModVector<Trans::Log::LSN>::Iterator ite(_redoLSN.find(_lsn));
		; _SYDNEY_ASSERT(ite != _redoLSN.end());
		(void) _redoLSN.erase(ite);
		break;
	}
	case Trans::Log::Data::Category::BranchForget:
	{
		// ヒューリスティックに解決済なトランザクションブランチを
		// 抹消したことを表す論理ログである

		const Trans::Log::BranchForgetData& tmp =
			_SYDNEY_DYNAMIC_CAST(
				const Trans::Log::BranchForgetData&, *_logData);

		// ヒューリスティックに解決済のトランザクションブランチを抹消する

		Trans::Branch::redo(tmp);

		ModVector<Trans::Log::LSN>::Iterator ite(_redoLSN.find(_lsn));
		; _SYDNEY_ASSERT(ite != _redoLSN.end());
		(void) _redoLSN.erase(ite);
	}
	case Trans::Log::Data::Category::DriverModify:

		// ドライバーの更新を表す論理ログである

		Transaction::redoDriver(trans, *_logData, _database);
		break;

	case Trans::Log::Data::Category::StartBatch:

		// バッチ処理の開始を表す論理ログである
		// -> 何もしない

		break;
		
	case Trans::Log::Data::Category::XATransaction:

		// 子サーバのトランザクションブランチを開始した論理ログである
		
		if (_mount == false)
		{
			// データベースのマウントでは、
			// 子サーバのトランザクションブランチのケアを行わない

			const Trans::Log::XATransactionData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::XATransactionData&, *_logData);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (!_finishedLSN.isFound(tmp.getBeginTransactionLSN()))
			{
				// コミットまたはロールバックされていないトランザクションでの
				// 子サーバのトランザクションブランチはロールバックする

				try
				{
					// 例外が発生しても無視する
				
					DServer::Branch::rollback(trans, *_logData, _database);
				}
				catch (...) {}
			}
			
			// REDO から削除する
			ModVector<Trans::Log::LSN>::Iterator ite(_redoLSN.find(_lsn));
			if (ite != _redoLSN.end())
				(void) _redoLSN.erase(ite);
		}

		break;
	}

	if (!isRedone()) {

		// REDO の結果、論理ログファイルの属性が変更される可能性もあるので、
		// 一操作を REDO するたびに論理ログファイル記述子を生成し、破棄する

		Trans::Log::AutoFile logFile(_database.getLogFile());

		// 次の論理ログを読み出す

		Trans::AutoLatch latch(trans, logFile->getLockName());

		_lsn = logFile->getNextLSN(_lsn);
		if (_lsn == Trans::Log::IllegalLSN)

			// REDO し終えていない
			// コミットされたトランザクションが存在するのに、
			// 次の論理ログのログシーケンス番号が得られない

			_SYDNEY_THROW0(Exception::LogItemCorrupted);

		_logData = logFile->load(_lsn);
		if (!_logData.isOwner())
			_SYDNEY_THROW0(Exception::LogFileCorrupted);
	}

	return dbRecovery.release();
}

//
// Copyright (c) 2001, 2002, 2003, 2004, 2006, 2007, 2009, 2010, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
