// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalLog.cpp -- 論理ログファイルに関する処理を行うクラス関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Configuration.h"
#include "Checkpoint/Debug.h"
#include "Checkpoint/FileSynchronizer.h"
#include "Checkpoint/LogData.h"
#include "Checkpoint/LogicalLog.h"
#include "Checkpoint/TimeStamp.h"

#include "Common/Assert.h"
#include "Os/AutoCriticalSection.h"
#include "Schema/Manager.h"
#include "Trans/AutoLogFile.h"
#include "Trans/List.h"
#include "Trans/Transaction.h"

#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{

namespace _Transaction
{
	// 現在実行中の更新トランザクションの有無を調べる
	bool
	isAnyInProgress();

	// あるデータベースを操作中の更新トランザクションに関する
	// 論理ログ用の情報を取得する
	void
	getLogInfo(
		Schema::ObjectID::Value dbID,
		ModVector<Log::CheckpointDatabaseData::TransactionInfo>& v);
}

//	FUNCTION
//	$$$::_Transaction::isAnyInProgress --
//		現在実行中の更新トランザクションの有無を調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			存在する
//		false
//			存在しない
//
//	EXCEPTIONS

bool
_Transaction::isAnyInProgress()
{
	return Trans::Transaction::isAnyInProgress(
		Trans::Transaction::Category::ReadWrite);
}

//	FUNCTION
//	$$$::_Transaction::getLogInfo --
//		あるデータベースを操作中の更新トランザクションに関する
//		論理ログ用の情報を取得する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::ObjectID::Value	dbID
//			このスキーマオブジェクト識別子の表すデータベースを操作中の
//			更新トランザクションについて処理する
//		ModVector<Log::CheckpointDatabaseData::TransactionInfo>&	v
//			得られた情報が要素として追加される配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_Transaction::getLogInfo(
	Schema::ObjectID::Value dbID,
	ModVector<Log::CheckpointDatabaseData::TransactionInfo>& v)
{
	// 現在実行中の更新トランザクションの
	// トランザクション記述子をひとつひとつ調べていく

	const Trans::List<Trans::Transaction>& list =
		Trans::Transaction::getInProgressList(
			dbID, Trans::Transaction::Category::ReadWrite);

	Os::AutoCriticalSection latch(list.getLatch());

	if (list.getSize()) {
		Trans::List<Trans::Transaction>::ConstIterator ite(list.begin());
		const Trans::List<Trans::Transaction>::ConstIterator& end = list.end();

		do {
			// 更新トランザクション中で行われた
			// データベースに対する操作を記録する
			// 論理ログファイルを操作するための情報を得る

			const Trans::Log::Info& logInfo =
				(*ite).getLogInfo(Trans::Log::File::Category::Database);

			if (logInfo.getDatabaseID() == dbID &&
				logInfo.getBeginTransactionLSN() !=	Trans::Log::IllegalLSN &&
				logInfo.getBeginTransactionLSN() !=
					logInfo.getLastTransactionLSN()) {

				// 今調べている更新トランザクションは
				// 指定された論理ログファイルを使用しており、
				// トランザクション開始以外の論理ログを記録している
				//
				//【注意】	トランザクション開始の論理ログしか
				//			記録していないのは、実際はありえないはず

				v.pushBack(Log::CheckpointDatabaseData::TransactionInfo(
					logInfo.getBeginTransactionLSN(),
					logInfo.getLastTransactionLSN(),
					(*ite).getStatus() == Trans::Transaction::Status::Prepared ?
						(*ite).getXID() : Trans::XID()));
			}
		} while (++ite != end) ;
	}
}

}

//	FUNCTION private
//	Checkpoint::LogicalLog::store --
//		既存の論理ログファイルのうち、
//		必要なものにチェックポイント処理の終了を表す論理ログを記録する
//
//	NOTES
//		チェックポイントスレッドから
//		今回のチェックポイント処理終了時のタイムスタンプが設定後、呼び出される
//
//		ただし、もし、ディスクとバッファの内容が一致したとしても、
//		前回のチェックポイント処理処理終了時のタイムスタンプは
//		今回のもので上書きされる前に呼び出される
//
//	ARGUMENTS
//		bool				persisted
//			true
//				バッファにはダーティな内容は存在せず、
//				バッファとディスクは完全に一致している
//			false
//				バッファにダーティな内容が存在する
//		bool				terminating
//			true
//				論理ログを記録直後、チェックポイントスレッドは終了する
//			false
//				論理ログを記録後も、チェックポイントスレッドは
//				継続して実行される
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Checkpoint::LogicalLog::store(bool persisted, bool terminating)
{
	// 現在使用中の論理ログファイルに関する情報を得る

	ModVector<Trans::Log::File*> fileList(Trans::Log::File::getInUseList());

	if (fileList.getSize()) {

		// 得られた論理ログファイルに関する情報ごとに処理する
		//
		//【注意】	同時に操作するものはいないはずなので、
		//			論理ログファイルをラッチしない

		ModVector<Trans::Log::File*>::Iterator			ite(fileList.begin());
		const ModVector<Trans::Log::File*>::Iterator&	end = fileList.end();

		do {
			if (static_cast<Schema::ObjectID::Value>(
					(*ite)->getLockName().getDatabasePart()) !=
				Schema::ObjectID::SystemTable) {

				// システムデータベース以外のデータベースについて処理する

				Trans::Log::AutoFile logFile((*ite)->attach());

				// データベースに対するチェックポイント処理の終了を表す
				// 論理ログを記録する

				const bool stored = store(*logFile, persisted, terminating);

				// 論理ログを記録しなかったときは
				// 論理ログファイルに関する情報を破棄する

				logFile.free(stored);
			}
		} while (++ite != end) ;
	}

	// システム用の論理ログファイルに
	// システム全体に対するチェックポイント処理の終了を表す論理ログを記録する
	//
	//【注意】	同時に操作するものはいないはずなので、
	//			論理ログファイルをラッチしない

	Trans::Log::AutoFile logFile(Schema::Manager::SystemTable::getLogFile());
	store(*logFile, persisted, terminating);

	_SYDNEY_CHECKPOINT_EXECUTION_MESSAGE
		<< "Checkpoint Log is stored" << ModEndl;
}

//	FUNCTION private
//	Checkpoint::LogicalLog::store --
//		ある論理ログファイルに
//		チェックポイント処理の終了を表す論理ログを必要であれば記録する
//
//	NOTES
//		指定された論理ログファイルを使用する
//		データベースを同時に操作するものはいない
//
//		今回のチェックポイント処理終了時のタイムスタンプが設定後、呼び出される
//
//		ただし、もし、ディスクとバッファの内容が一致したとしても、
//		前回のチェックポイント処理処理終了時のタイムスタンプは
//		今回のもので上書きされる前に呼び出される
//
//		論理ログを記録する前に可能であれば、論理ログファイルをトランケートする
//
//	ARGUMENTS
//		Trans::Log::File&	logFile
//			論理ログを記録する論理ログファイルを操作するための情報
//		bool				persisted
//			true
//				バッファにはダーティな内容は存在せず、
//				バッファとディスクは完全に一致している
//			false
//				バッファにダーティな内容が存在する
//		bool				terminating
//			true
//				論理ログを記録直後、チェックポイントスレッドは終了する
//			false
//				論理ログを記録後も、チェックポイントスレッドは
//				継続して実行される
//
//	RETURN
//		true
//			論理ログを記録した
//		false
//			論理ログを記録しなかった
//
//	EXCEPTIONS

bool
Checkpoint::LogicalLog::store(
	Trans::Log::File& logFile, bool persisted, bool terminating)
{
	bool result = false;
	
	//【注意】	指定された論理ログファイルを使用する
	//			データベースを同時に操作するものはいないこと

	const Schema::ObjectID::Value dbID =
		static_cast<Schema::ObjectID::Value>(
			logFile.getLockName().getDatabasePart());
	; _SYDNEY_ASSERT(dbID != Schema::ObjectID::Invalid);

	if (!logFile.isReadOnly()) {
		if (!terminating && !logFile.isSynchronized())

			// この論理ログファイルに最後に記録したのは、
			// バージョンファイルの同期を表す論理ログでないので、
			// この論理ログファイルを使用するデータベースを
			// 同期処理の候補として登録する

			FileSynchronizer::enter(dbID);

		if (dbID == Schema::ObjectID::SystemTable) {

			// 操作する論理ログファイルはシステム用である

			// ヒューリスティックに解決した
			// トランザクションブランチの情報を求める

			ModVector<Trans::Branch::HeurCompletionInfo> info;
			Trans::Branch::getHeurCompletionInfo(info);

			if (Configuration::TruncateLogicalLog::get() &&
				logFile.isTruncatable() &&
				Database::getUnavailable().isEmpty() &&
				info.isEmpty() &&
				persisted &&
				!_Transaction::isAnyInProgress())

				// 現在実行中の更新トランザクションがなく、
				// 利用不可なデータベースがなく、
				// ヒューリスティックに解決した
				// トランザクションブランチをひとつも覚えてなく、
				// 今回のチェックポイント処理で
				// 全データベースが完全に永続化されていれば、
				// システム用の論理ログファイルをトランケートする
				//
				//【注意】	この直後に異常終了すると、
				//			再起動時に回復処理が行われなくなるので、
				//			更新トランザクションがないことを確認している

				logFile.truncate();

			// システム全体に対する
			// チェックポイント処理の終了を表す論理ログを記録する
			//
			//【注意】	システムデータベースが利用不可でも、
			//			システム全体に対する
			//			チェックポイント処理の終了を表す論理ログは記録する
			//
			//【注意】	システム用の論理ログファイルには、
			//			データベースに対する
			//			チェックポイント処理の終了を表す論理ログは記録しない

			(void) logFile.store(
				Log::CheckpointSystemData(
					Trans::Log::Data::VersionNumber::Second,
					TimeStamp::getMostRecent(),
					TimeStamp::getSecondMostRecent(),
					persisted, terminating,
					!Database::isAvailable(Schema::ObjectID::SystemTable),
					Database::getUnavailable(), info));

			// 論理ログを記録したので
			
			result = true;

		} else if (Database::isAvailable(dbID)) {

			// 操作する論理ログファイルは
			// システムデータベース以外の利用可能なデータベース用である

			// 現在実行中の更新トランザクションのうち、
			// 指定された論理ログファイルを使用するものについて、
			// 記録する論理ログ用の情報を求める

			ModVector<Log::CheckpointDatabaseData::TransactionInfo> info;
			_Transaction::getLogInfo(dbID, info);

			// 前回のチェックポイント処理終了時のタイムスタンプを求める
			//
			//【注意】	すでに今回のチェックポイント処理の終了時の
			//			タイムスタンプを生成しているので、
			//			前回のチェックポイント処理終了時のタイムスタンプは
			//			TimeStamp::getSecondMostRecent で得る

			const Trans::TimeStamp& mostRecent =
				TimeStamp::getSecondMostRecent(logFile.getLockName());

			// この論理ログファイルが
			// 実行中のトランザクションにより使用されているか、
			// 直前のチェックポイント以降に論理ログが記録されるか調べる

			const bool used =
				info.getSize() || logFile.getLastModification() > mostRecent;

			if (Configuration::TruncateLogicalLog::get() &&
				logFile.isTruncatable() &&
				(terminating ||
				 logFile.getLastModification().isIllegal() || !used))

				// 論理ログファイルが実行中のトランザクションにより
				// 使用されていなければ、トランケートできる
				//
				// 新しいバージョンの場合は、何もしない
				//
				//【注意】	もし、前回のチェックポイント以降に
				//			あるトランザクションにより使用されていれば、
				//			そのトランザクションの操作を表す
				//			論理ログが記録されているはず
				//
				//			前回のチェックポイント時に
				//			実行中のトランザクションにより使用中のものは、
				//			そのとき、チェックポイント処理の終了を表す
				//			論理ログが記録されているはず
				//
				//			チェックポイント処理の終了を表す
				//			論理ログを記録したときのタイムスタンプは、
				//			チェックポイント終了時のタイムスタンプと等しいので、
				//			前回のチェックポイント処理より後に
				//			論理ログが記録されていないかで判定できる

				logFile.truncate();

			if (used) {
				if (!logFile.isSynchronized() &&
					Configuration::EnableFileSynchronizer::get() ==
						Configuration::EnableFileSynchronizer::SPEED)

					// 現在利用中なので、今回は同期処理を実行しない

					FileSynchronizer::skip(dbID);

				// データベースに対する
				// チェックポイント処理の終了を表す論理ログを記録する

				(void) logFile.store(
					Log::CheckpointDatabaseData(
						Trans::Log::Data::VersionNumber::Third,
						TimeStamp::getMostRecent(logFile.getLockName()),
						mostRecent, persisted, terminating, info));

				// 論理ログを記録したので

				result = true;

				// 下位でローテートが本当に必要かどうか判断して実行している
				// 必要なら、不要な論理ログの削除も行われる
				
				logFile.rotate(persisted);
			}
		}
	}

	return result;
}

//
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
