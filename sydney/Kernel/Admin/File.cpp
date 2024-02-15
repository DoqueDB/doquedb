// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.cpp -- 再構成ファイルへの論理ログ反映関連の関数定義
// 
// Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#include "Admin/File.h"

#include "Common/Assert.h"
#include "Exception/LogItemCorrupted.h"
#include "Schema/Database.h"
#include "Schema/File.h"
#include "Trans/AutoLatch.h"
#include "Trans/AutoLogFile.h"
#include "Trans/LogData.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING
_SYDNEY_ADMIN_REORGANIZATION_USING

//	FUNCTION public
//	Admin::Reorganization::File::reflect --
//		再構成中の索引ファイルへ直前の反映中に実行された操作を反映する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			再構成を行うトランザクションのトランザクション記述子
//		Schema::File&		database
//			再構成中のデータベースを表すクラス
//		Schema::File&		file
//			再構成で作成された索引ファイルを表すクラス
//		Trans::Log::LSN		startLSN
//			直前の反映を開始したときの末尾の論理ログのログシーケンス番号
//
//	RETURN
//		反映を開始したときの末尾の論理ログのログシーケンス番号
//
//	EXCEPTIONS

// static
Trans::Log::LSN
File::reflect(Trans::Transaction& trans,
			  const Schema::Database& database, Schema::File& file,
			  Trans::Log::LSN startLSN)
{
	// データベース用の論理ログファイルを操作するための情報を得る

	Trans::Log::AutoFile logFile(database.getLogFile());

	// 直前の反映中に実行されたトランザクションのうち、
	// REDO, UNDO する操作に関連する論理ログのログシーケンス番号を求める

	ModMap<Trans::Log::LSN, Trans::Log::LSN,
		   ModLess<Trans::Log::LSN> >	rollbackedLSN;
	ModVector<Trans::Log::LSN>	noRedoLSN;

	Trans::Log::LSN lastLSN =
		findRelatedLSN(trans, *logFile, startLSN, rollbackedLSN, noRedoLSN);

	// 直前の反映中に実行されたトランザクションのうち、
	// 直前の反映を開始したときに実行中でその後ロールバックされたものの、
	// 直前の反映を開始する以前に実行した操作をすべて UNDO する

	undo(trans, file, *logFile, rollbackedLSN);

	// 直前の反映中に実行されたトランザクションのうち、
	// コミットされたもの、または直前の反映の終了時に
	// 実行中のものをすべて REDO する

	redo(trans, file, *logFile, startLSN, lastLSN, rollbackedLSN, noRedoLSN);

	return lastLSN;
}

//	FUNCTION private
//	Admin::Reorganization::File::findRelatedLSN --
//		直前の反映中に実行されたトランザクションの操作のうち、
//		REDO, UNDO する操作に関連する論理ログのログシーケンス番号を求める
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			再構成を行うトランザクションのトランザクション記述子
//		Trans::Log::File&	logFile
//			再構成するデータベースに対する更新操作を表す論理ログが
//			記録されている論理ログファイルを操作するための情報
//		Trans::Log::LSN		startLSN
//			直前の反映を開始したときの末尾の論理ログのログシーケンス番号
//		ModMap<Trans::Log::LSN, Trans::Log::LSN,
//			   ModLess<Trans::Log::LSN> >& rollbackedLSN
//			再構成するデータベースを更新したトランザクションのうち、
//			直前の反映を開始したときに実行中でその後ロールバックされたものの、
//			直前の反映を開始する時点の直前に記録した
//			論理ログのログシーケンス番号が記録されるマップ
//		ModVector<Trans::Log::LSN>& noRedoLSN
//			再構成するデータベースを更新したトランザクションのうち、
//			反映中にロールバックされなかったもので、
//			ロールバックされた SQL 文の直前に実行された
//			SQL 文の終了を表す論理ログのログシーケンス番号が格納されるベクタ
//			
//	RETURN
//		反映を開始したときの末尾の論理ログのログシーケンス番号
//
//	EXCEPTIONS

// static
Trans::Log::LSN
File::findRelatedLSN(
	Trans::Transaction& trans, Trans::Log::File& logFile,
	Trans::Log::LSN startLSN,
	ModMap<Trans::Log::LSN, Trans::Log::LSN,
		   ModLess<Trans::Log::LSN> >& rollbackedLSN,
	ModVector<Trans::Log::LSN>& noRedoLSN)
{
	; _SYDNEY_ASSERT(startLSN != Trans::Log::IllegalLSN);
	; _SYDNEY_ASSERT(!rollbackedLSN.getSize());
	; _SYDNEY_ASSERT(noRedoLSN.isEmpty());

	// データベース用の論理ログファイルの末尾に記録されている
	// 論理ログのログシーケンス番号を得る

	Trans::Log::LSN	lastLSN;
	{
	Trans::AutoLatch latch(trans, logFile.getLockName());
	lastLSN = logFile.getLastLSN();
	}
	// 少なくとも、索引の作成を表す論理ログは記録されているはず

	; _SYDNEY_ASSERT(lastLSN != Trans::Log::IllegalLSN);

	Trans::Log::LSN lsn = lastLSN;
	while (lsn > startLSN) {

		// 論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data>	data;
		{
		Trans::AutoLatch latch(trans, logFile.getLockName());
		data = logFile.load(lsn);
		}
		; _SYDNEY_ASSERT(data.isOwner());

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::TransactionRollback:
		{
			// トランザクションのロールバックを表す論理ログである

			const Trans::Log::TransactionRollbackData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::TransactionRollbackData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getBackwardLSN() == Trans::Log::IllegalLSN)

				// 不正なログシーケンス番号が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (tmp.getBeginTransactionLSN() != tmp.getBackwardLSN())

				// トランザクションのロールバックを表す論理ログなので、
				// その開始を表す論理ログのログシーケンス番号をキーにして、
				// それが直前に記録した論理ログのログシーケンス番号を記録する
				// ★注意★
				// 以前の実装では再構成の開始前に始まったトランザクションのみを
				// 対象にしていたが再構成中のものはすべてUNDOが必要になる可能性があるので
				// その条件は除いた

				(void) rollbackedLSN.insert(tmp.getBeginTransactionLSN(),
											tmp.getBackwardLSN());
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

			if (rollbackedLSN.find(
					tmp.getBeginTransactionLSN()) == rollbackedLSN.end() &&
				!noRedoLSN.isFound(tmp.getEndStatementLSN()))

				// ロールバックされていないトランザクションで
				// 実行された SQL 文のうち、ロールバックされた
				// SQL 文は REDO しないようにする必要がある

				noRedoLSN.pushBack(tmp.getEndStatementLSN());
			break;
		}
		case Trans::Log::Data::Category::TupleModify:
		{
			// タプルの更新を表す論理ログである

			const Trans::Log::InsideTransactionData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::InsideTransactionData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getBackwardLSN() == Trans::Log::IllegalLSN)
				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			ModMap<Trans::Log::LSN, Trans::Log::LSN,
				   ModLess<Trans::Log::LSN> >::Iterator
				ite(rollbackedLSN.find(tmp.getBeginTransactionLSN()));

			if (ite != rollbackedLSN.end())

				// タプルを更新したトランザクションが
				// 反映を開始する前から実行中でロールバックされていれば、
				// それが直前に記録した論理ログのログシーケンス番号を記録する

				(*ite).second = tmp.getBackwardLSN();
		}
		}

		// 直前の論理ログのログシーケンス番号を得る
		{
		Trans::AutoLatch latch(trans, logFile.getLockName());
		lsn = logFile.getPrevLSN(lsn);
		}
	}

	// 反映を開始したときの末尾の論理ログのログシーケンス番号を返す

	return lastLSN;
}

//	FUNCTION private
//	Admin::Reorganization::File::undo --
//		再構成中の索引ファイルへの反映処理として、更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			再構成を行うトランザクションのトランザクション記述子
//		Schema::File&		file
//			再構成で作成された索引ファイルを表すクラス
//		Trans::Log::File&	logFile
//			再構成するデータベースに対する更新操作を表す論理ログが
//			記録されている論理ログファイルを操作するための情報
//		ModMap<Trans::Log::LSN, Trans::Log::LSN,
//			   ModLess<Trans::Log::LSN> >& rollbackedLSN
//			再構成するデータベースを更新したトランザクションのうち、
//			直前の反映を開始したときに実行中でその後ロールバックされたものの、
//			直前の反映を開始する時点の直前に記録した
//			論理ログのログシーケンス番号が記録されるマップ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
File::undo(Trans::Transaction& trans,
		   Schema::File& file, Trans::Log::File& logFile,
		   const ModMap<Trans::Log::LSN, Trans::Log::LSN,
						ModLess<Trans::Log::LSN> >& rollbackedLSN)
{
	if (!rollbackedLSN.getSize())

		// UNDO すべき操作はない

		return;

	// 最初に UNDO すべき操作を表す論理ログの
	// ログシーケンス番号を降順にソートする

	ModMap<Trans::Log::LSN, Trans::Log::LSN, ModGreater<Trans::Log::LSN> >
		descMap;
	{
	ModMap<Trans::Log::LSN, Trans::Log::LSN,
		   ModLess<Trans::Log::LSN> >::ConstIterator
		ite(rollbackedLSN.begin());
	const ModMap<Trans::Log::LSN, Trans::Log::LSN,
				 ModLess<Trans::Log::LSN> >::ConstIterator&
		end = rollbackedLSN.end();

	for (; ite != end; ++ite)
		(void) descMap.insert((*ite).second, (*ite).second);
	}
	// UNDO されていない操作がある限り、
	// 論理ログファイルの先頭に向かって
	// UNDO する操作を表す論理ログを読み出していく

	while (descMap.getSize()) {

		// UNDO する操作を表す論理ログのうち、
		// 論理ログファイルの末尾に最も近い位置に
		// 記録されているもののログシーケンス番号を得る

		Trans::Log::LSN	lsn = (*descMap.begin()).first;
		; _SYDNEY_ASSERT(lsn != Trans::Log::IllegalLSN);

		// 論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data>	data;
		{
		Trans::AutoLatch latch(trans, logFile.getLockName());
		data = logFile.load(lsn);
		}
		; _SYDNEY_ASSERT(data.isOwner());

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::TupleModify:

			// タプルの更新を UNDO する

			file.undoTuple(trans, *data);
			// thru

		case Trans::Log::Data::Category::SchemaModify:

			//【注意】	再構成中の索引に対して、
			//			実行されるスキーマ操作はないはず

		case Trans::Log::Data::Category::FileSynchronizeBegin:
		case Trans::Log::Data::Category::FileSynchronizeEnd:

			//【注意】	再構成中の索引が存在するデータベースに対して、
			//			実行されるバージョンファイルの同期はないはず

		case Trans::Log::Data::Category::StatementCommit:
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
			//			UNDO 中の処理で除去しているので、実際はありえないはず

			break;

		default:

			// トランザクション中に記録しない種類の論理ログが記録されている

			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		}

		// UNDO した操作を表す論理ログの LSN を忘れる

		descMap.erase(descMap.begin());
	}
}

//	FUNCTION private
//	Admin::Reorganization::File::redo --
//		再構成中の索引ファイルへの反映処理として、更新操作を REDO する
//		
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			再構成を行うトランザクションのトランザクション記述子
//		Schema::File&		file
//			再構成で作成された索引ファイルを表すクラス
//		Trans::Log::File&	logFile
//			再構成するデータベースに対する更新操作を表す論理ログが
//			記録されている論理ログファイルを操作するための情報
//		Trans::Log::LSN		startLSN
//			直前の反映を開始したときの末尾の論理ログのログシーケンス番号
//		Trans::Log::LSN		lastLSN
//			反映を開始したときの末尾の論理ログのログシーケンス番号
//		ModMap<Trans::Log::LSN, Trans::Log::LSN,
//			   ModLess<Trans::Log::LSN> >& rollbackedLSN
//			再構成するデータベースを更新したトランザクションのうち、
//			直前の反映を開始したときに実行中でその後ロールバックされたものの、
//			直前の反映を開始する時点の直前に記録した
//			論理ログのログシーケンス番号が記録されるマップ
//		ModVector<Trans::Log::LSN>& noRedoLSN
//			再構成するデータベースを更新したトランザクションのうち、
//			反映中にロールバックされなかったもので、
//			ロールバックされた SQL 文の直前に実行された
//			SQL 文の終了を表す論理ログのログシーケンス番号が格納されるベクタ
//			
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
File::redo(Trans::Transaction& trans,
		   Schema::File& file, Trans::Log::File& logFile,
		   Trans::Log::LSN startLSN, Trans::Log::LSN lastLSN,
		   const ModMap<Trans::Log::LSN, Trans::Log::LSN,
						ModLess<Trans::Log::LSN> >& rollbackedLSN,
		   const ModVector<Trans::Log::LSN>& noRedoLSN)
{
	; _SYDNEY_ASSERT(startLSN != Trans::Log::IllegalLSN);
	; _SYDNEY_ASSERT(lastLSN != Trans::Log::IllegalLSN);

	if (startLSN == lastLSN)

		// REDO すべき操作はない

		return;

	// 指定された論理ログまで、
	// ひとつひとつ論理ログデータを読み出しながら、処理していく

	Trans::Log::LSN	lsn = startLSN;
	do {
		// 次のログシーケンス番号を得て、その論理ログデータを読み出す

		ModAutoPointer<const Trans::Log::Data>	data;
		{
		Trans::AutoLatch latch(trans, logFile.getLockName());
		lsn = logFile.getNextLSN(lsn);
		data = logFile.load(lsn);
		}
		; _SYDNEY_ASSERT(data.isOwner());

		switch (data->getCategory()) {
		case Trans::Log::Data::Category::TupleModify:
		{
			// タプルの更新を表す論理ログである

			const Trans::Log::ModificationData& tmp =
				_SYDNEY_DYNAMIC_CAST(
					const Trans::Log::ModificationData&, *data);

			if (tmp.getBeginTransactionLSN() == Trans::Log::IllegalLSN ||
				tmp.getEndStatementLSN() == Trans::Log::IllegalLSN)

				// 不正なログシーケンス番号が得られた

				_SYDNEY_THROW0(Exception::LogItemCorrupted);

			if (rollbackedLSN.find(
					tmp.getBeginTransactionLSN()) == rollbackedLSN.end() &&
				!noRedoLSN.isFound(tmp.getEndStatementLSN()))

				// ロールバックされていないトランザクションでの
				// ロールバックされていない SQL 文でのタプルの更新は REDO する

				file.redoTuple(trans, *data);
			break;
		}
		case Trans::Log::Data::Category::SchemaModify:

			// スキーマ操作を表す論理ログである

			// なにもすることはない
			//
			//【注意】	再構成中の索引に対して
			//			実行されるスキーマ操作はないはず

		case Trans::Log::Data::Category::FileSynchronizeBegin:
		case Trans::Log::Data::Category::FileSynchronizeEnd:

			// バージョンファイルの同期を表す論理ログである

			// なにもすることはない
			//
			//【注意】	再構成中の索引が存在するデータベースに対して
			//			実行されるバージョンファイルの同期はないはず

			break;
		}
	} while (lsn < lastLSN) ;
}

//
// Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
