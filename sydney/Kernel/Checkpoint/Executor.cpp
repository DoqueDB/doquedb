// -*-Mode1: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.cpp -- チェックポイント処理実行スレッド処理関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Buffer.h"
#include "Checkpoint/Daemon.h"
#include "Checkpoint/Database.h"
#include "Checkpoint/Debug.h"
#include "Checkpoint/Executor.h"
#include "Checkpoint/FileDestroyer.h"
#ifdef OBSOLETE
#include "Checkpoint/FileMover.h"
#endif
#include "Checkpoint/FileSynchronizer.h"
#include "Checkpoint/LogicalLog.h"
#include "Checkpoint/TimeStamp.h"

#include "Buffer/Daemon.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/Thread.h"
#include "Schema/Database.h"
#include "Server/Manager.h"
#include "Trans/AutoLogFile.h"
#include "Trans/AutoTransaction.h"
#include "Trans/TimeStamp.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace 
{
}

//	FUNCTION private
//	Checkpoint::Executor::repeatable -- チェックポイント処理を行う
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
Executor::repeatable()
{
	// 状態報告

	Daemon::AutoState cState;

	//【注意】	この関数の実行中は Checkpoint::Daemon::disable() できない

	if (!(getStatus() != Aborting && isEnabled() &&
		  Server::Manager::isAvailable()))
	{
		// スレッドが終了処理中、実行不可、
		// サーバーが利用不可なので、なにもしない

		return;
	}

	// システム全体のチェックポイント処理を行う

	cause(false);

	// この時点で、チェックポイント処理はすべて終了した

	try {
		// バージョンファイル同期スレッドを起こして、
		// 可能であれば、バージョンファイルの同期を取る

		Checkpoint::Daemon::wakeup(
			Checkpoint::Daemon::Category::FileSynchronizer);

		// 次に同期処理を起動するので、状態報告はそのままにする
	
		cState.release();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		SydErrorMessage << "Database synchronizer wakeup failed."
						<< ModEndl;
		
		// 今回はあきらめる

		Common::Thread::resetErrorCondition();
	}
}

//	FUNCTION public
//	Checkpoint::Executor::cause --
//		システム全体のチェックポイント処理を行う
//
//	NOTES
//		呼び出し側はほかのスレッドがチェックポイント処理や
//		同期処理を行わないことを保障しなければならない
//
//	ARGUMENTS
//		bool				aborting
//			true
//				最後のチェックポイント処理である
//			false
//				最後のチェックポイント処理でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Executor::cause(bool aborting)
{
	_SYDNEY_CHECKPOINT_EXECUTION_MESSAGE << "Process is started" << ModEndl;

	try {
		// バッファ関連のすべてのデーモンの処理を実行不可にする

		_SYDNEY::Buffer::Daemon::AutoDisabler	disabler;

		if (!aborting)
			// 同期処理候補を列挙する
			FileSynchronizer::prepare();

		// チェックポイント処理で使用するトランザクションを開始する

		Trans::AutoTransaction	trans(Trans::Transaction::attach());
		trans->begin(Schema::ObjectID::SystemTable,
					 Trans::Transaction::Mode(
						 Trans::Transaction::Category::ReadWrite,
						 Trans::Transaction::IsolationLevel::Serializable,
						 Boolean::False),
					 true, true);

		// 前回のチェックポイント処理の終了時に
		// ダーティだったバッファのうち、
		// いまだにダーティなものをすべてフラッシュする

		const bool persisted = Checkpoint::Buffer::flush(!aborting);
#ifdef OBSOLETE
		// これまでに移動が依頼されたオブジェクトのうち、
		// 移動可能なものがあれば、実際に移動する

		FileMover::execute(aborting);
#endif

		// これまでに破棄が依頼されたオブジェクトのうち、
		// 破棄可能なものがあれば、実際に破棄する

		FileDestroyer::execute(aborting);

		// 利用不可なデータベースを回復するときの
		// 開始時点のタイムスタンプを求め、設定する

		Database::setStartRecoveryTime(*trans);

		// チェックポイント処理で使用したトランザクションをコミットする

		trans->commit();

		// 新たに求めたタイムスタンプを
		// チェックポイント処理終了時のタイムスタンプとして設定する

		const Trans::TimeStamp& finish = Trans::TimeStamp::assign();
		TimeStamp::assign(finish, false);

		// チェックポイント処理の終了を表す論理ログを記録する
		//
		//【注意】	論理ログを記録する前に可能であれば、
		//			論理ログファイルはトランケートされる

		LogicalLog::store(persisted, aborting);

		if (persisted)

			// バッファとディスクの内容が完全に一致したので、
			// もう一度、先ほど求めたタイムスタンプを
			// チェックポイント処理終了時のタイムスタンプとして設定することで、
			// 前回と今回のチェックポイント処理終了時のタイムスタンプをあわせる

			TimeStamp::assign(finish, false);

		// チェックポイント処理はすべて終了した
		// バッファ関連のすべてのデーモンの処理を実行可能にする

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// チェックポイント処理に失敗したので、
		// サーバー全体を利用不可にする

		SydErrorMessage << "Checkpoint failed. Server is not available."
						<< ModEndl;

		Server::Manager::setAvailability(false);
		
		throw;
	}

	SydMessage << "Checkpoint occurred" << ModEndl;
}

//	FUNCTION public
//	Checkpoint::Executor::cause --
//		あるデータベースに関するチェックポイント処理を行う
//
//	NOTES
//		呼び出し側はほかのスレッドがチェックポイント処理や
//		同期処理を行わないことを保障しなければならない
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			データベースのチェックポイント処理を行う
//			トランザクションのトランザクション記述子
//		Schema::Database&	database
//			チェックポイント処理を行うデータベースのスキーマオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Executor::cause(Trans::Transaction& trans, Schema::Database& database)
{
	; _SYDNEY_ASSERT(trans.getCategory() ==
					 Trans::Transaction::Category::ReadWrite);

	// 状態報告

	Daemon::AutoState cState(true);

	try {
		// バッファ関連のすべてのデーモンの処理を実行不可にする

		_SYDNEY::Buffer::Daemon::AutoDisabler	disabler;

		// 指定されたデータベースを構成する
		// すべてのファイルを完全にフラッシュする

		database.flush(trans);

		Trans::Log::AutoFile logFile(database.getLogFile());

		// 新たに求めたタイムスタンプを
		// 指定されたデータベースに関する
		// チェックポイント処理終了時のタイムスタンプとして設定する

		const Trans::TimeStamp& finish = Trans::TimeStamp::assign();
		TimeStamp::assign(logFile->getLockName(), finish, true /* メモリとファイルが一致している */);

		// 指定されたデータベースに関する
		// チェックポイント処理の終了を表す論理ログを記録する
		//
		//【注意】	論理ログを記録する前に可能であれば、
		//			論理ログファイルはトランケートされる

		const bool stored = LogicalLog::store(*logFile, true, false);

		// 論理ログを記録しなかったときは
		// 論理ログファイルに関する情報を破棄する

		logFile.free(stored);

		// 指定されたデータベースについては、
		// バッファとディスクの内容が完全に一致しているので、
		// もう一度、先ほど求めたタイムスタンプを
		// チェックポイント処理終了時のタイムスタンプとして設定することで、
		// 前回と今回のチェックポイント処理終了時のタイムスタンプをあわせる
		//
		// -> 上で第三引数をtrueにしたので不要

		//TimeStamp::assign(logFile->getLockName(), finish, false);

		// チェックポイント処理はすべて終了した
		// バッファ関連のすべてのデーモンの処理を実行可能にする

	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// チェックポイント処理に失敗したので、
		// データベースを利用不可にする

		SydErrorMessage << "Checkpoint failed. Database '"
						<< database.getName()
						<< "' is not available." << ModEndl;

		Database::setAvailability(database.getID(), false);
		
		throw;
	}
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2009, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
