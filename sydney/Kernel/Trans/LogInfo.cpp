// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogInfo.cpp -- 論理ログファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Trans";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Trans/LogInfo.h"
#include "Trans/Configuration.h"
#include "Trans/LogData.h"
#include "Trans/AutoLogFile.h"

#include "Common/Assert.h"
#include "Schema/Database.h"
#include "Schema/LogData.h"

_SYDNEY_USING
_SYDNEY_TRANS_USING
_SYDNEY_TRANS_LOG_USING

namespace
{
}

//	FUNCTION public
//	Trans::Log::Info::Info --
//		あるトランザクションが操作する論理ログファイルの
//		トランザクションごとの情報を記憶するクラスのデフォルトコンストラクター
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

Info::Info()
	: _logFile(0),
	  _beginTransactionLSN(IllegalLSN),
	  _endStatementLSN(IllegalLSN),
	  _lastTransactionLSN(IllegalLSN),
	  _unmountDelayed(0),
	  _unmounting(false),
	  _discardLogicalLog(DiscardLog::None),
	  _masterBeginTransactionLSN(IllegalLSN)
{}

//	FUNCTION public
//	Trans::Log::Info::Info --
//		あるトランザクションが操作する論理ログファイルの
//		トランザクションごとの情報を記憶するクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベース用の論理ログファイルを操作する
//		Trans::Log::LSN		beginTransactionLSN
//			トランザクションの開始を表す論理ログのログシーケンス番号
//		Trans::Log::LSN		endStatementLSN
//			トランザクションが直前に実行した SQL 文の
//			終わりを表す論理ログのログシーケンス番号
//		Trans::Log::LSN		lastTransactionLSN
//			トランザクションが直前に記録した論理ログのログシーケンス番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Info::Info(Schema::Database& database,
		   LSN beginTransactionLSN, LSN endStatementLSN,
		   LSN lastTransactionLSN)
	: _logFile(database.getLogFile()),
	  _beginTransactionLSN(beginTransactionLSN),
	  _endStatementLSN(endStatementLSN),
	  _lastTransactionLSN(lastTransactionLSN),
	  _unmountDelayed(0),
	  _unmounting(false),
	  _discardLogicalLog(DiscardLog::None),
	  _masterBeginTransactionLSN(IllegalLSN)
{}

//	FUNCTION public
//	Trans::Log::Info::Info --
//		あるトランザクションが操作する論理ログファイルの
//		トランザクションごとの情報を記憶するクラスのコピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Info&	src
//			自分自身にコピーする操作する
//			トランザクションごとの論理ログファイルの情報を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Info::Info(const Info& src)
	: _logFile(src._logFile),
	  _beginTransactionLSN(IllegalLSN),
	  _endStatementLSN(IllegalLSN),
	  _lastTransactionLSN(IllegalLSN),
	  _unmountDelayed(0),
	  _unmounting(false),
	  _discardLogicalLog(DiscardLog::None),
	  _masterBeginTransactionLSN(IllegalLSN)
{}

//	FUNCTION public
//	Trans::Log::Info::operator = -- = 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Info&	src
//			自分自身に代入する操作する
//			トランザクションごとの論理ログファイルの情報を表すクラス
//
//	RETURN
//		代入後の自分自身
//
//	EXCEPTIONS

Info&
Info::operator =(const Info& src)
{
	if (this != &src) {
		_logFile = src._logFile;
		_beginTransactionLSN = IllegalLSN;
		_endStatementLSN = IllegalLSN;
		_lastTransactionLSN = IllegalLSN;
		_unmountDelayed.free();
		_unmounting = false;
		_unmountTimeStamp = IllegalTimeStamp;
		_discardLogicalLog = DiscardLog::None;

		// _masterBeginTransactionLSN はコピーしない
	}
	return *this;
}

//	FUNCTION private
//	Trans::Log::Info::create -- 論理ログファイルを生成する
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
Info::create()
{
	AutoFile file(getFile());
	if (file.isOwner())

		// 論理ログファイルを生成する

		file->create();
}

//	FUNCTION private
//	Trans::Log::Info::destroy -- 論理ログファイルを破棄する
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
Info::destroy()
{
	AutoFile file(getFile());
	if (file.isOwner()) {

		// 論理ログファイルを破棄する

		file->destroy();

		// 操作する論理ログファイルを忘れる

		_logFile.free();
		_unmountDelayed.free();

		// おぼえているログシーケンス番号をすべて忘れる

		_beginTransactionLSN =
		_endStatementLSN = _lastTransactionLSN = IllegalLSN;

		// 論理ログ削除のデータを忘れる
		
		_discardLogicalLog = DiscardLog::None;
	}
}

//	FUNCTION private
//	Trans::Log::Info::mount -- 論理ログファイルをマウントする
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
Info::mount()
{
	AutoFile file(getFile());
	if (file.isOwner())

		// 論理ログファイルをマウントする

		file->mount();
}

//	FUNCTION private
//	Trans::Log::Info::unmount -- 論理ログファイルをアンマウントを指示する
//
//	NOTES
//		Trans::Log::Info::store によって、
//		トランザクションの開始を表す論理ログを記録してから、
//		その終了を表す論理ログを記録するまでは、実際のアンマウントは遅延される
//
//		マウントされていない
//		論理ログファイルをアンマウントしてもエラーにならない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Info::unmount()
{
	if (getBeginTransactionLSN() != IllegalLSN)

		// トランザクション中なので、アンマウントを遅らせる

		_unmountDelayed = _logFile, _logFile.free();
	else {
		AutoFile file(getFile());
		if (file.isOwner()) {

			// 論理ログファイルを実際にアンマウントする

			file->unmount();

			// 操作する論理ログファイルを忘れる

			_logFile.free();
			_unmountDelayed.free();

			// おぼえているログシーケンス番号をすべて忘れる

			_beginTransactionLSN =
			_endStatementLSN = _lastTransactionLSN = IllegalLSN;

			// 論理ログ削除のデータを忘れる
		
			_discardLogicalLog = DiscardLog::None;
		}
	}
}

//	FUNCTION private
//	Trans::Log::Info::rename --
//		論理ログファイルの実体である OS ファイルの絶対パス名を変更する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Path&			path
//			論理ログファイルの実体である OS ファイルの変更後の絶対パス名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Info::rename(const Os::Path& path)
{
	AutoFile file(getFile());
	if (file.isOwner())

		// 論理ログファイルの実体である OS ファイルの絶対パス名を変更する

		file->rename(path);
}

//	FUNCTION private
//	Trans::Log::Info::flush -- 論理ログファイルをフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::LSN		lsn
//			Trans::Log::IllegalLSN 以外のとき
//				論理ログファイルの先頭から
//				このログシーケンス番号の表す論理ログまでフラッシュする
//			Trans::Log::IllegalLSN
//				論理ログファイル全体をフラッシュする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Info::flush(LSN lsn)
{
	AutoFile file(getFile());
	if (file.isOwner())

		// 論理ログファイルをフラッシュする

		file->flush(lsn);
}

//	FUNCTION public
//	Trans::Log::Info::load -- 論理ログを取り出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::LSN		lsn
//			取り出す論理ログのログシーケンス番号
//
//	RETURN
//		0 以外の値
//			取り出された論理ログのデータを記憶する領域の先頭アドレス
//		0
//			論理ログを取り出すべき論理ログファイルがないので、
//			論理ログを取り出せない
//
//	EXCEPTIONS

Data*
Info::load(LSN lsn) const
{
	// 指定されたログシーケンス番号の論理ログを
	// 論理ログファイルから取り出す

	AutoFile file(getFile());
	return (file.isOwner()) ? file->load(lsn) : 0;
}

//	FUNCTION private
//	Trans::Log::Info::store -- 論理ログを記録する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data&	data
//			論理ログとして記録するデータ
//
//	RETURN
//		Trans::Log::IllegalLSN 以外の値
//			記録された論理ログのログシーケンス番号
//		Trans::Log::IllegalLSN
//			必要なかったので、論理ログを記録しなかった
//
//	EXCEPTIONS

LSN
Info::store(const Data& data, Log::LSN masterLSN)
{
	if (!Configuration::NoLogicalLog::get()) {
		if (data.isInsideTransactionData()) {

			// 新しい論理ログはトランザクション中でないと記録できない

			switch (data.getCategory()) {
			case Data::Category::StatementCommit:
			case Data::Category::StatementRollback:
				if (getBeginTransactionLSN() == IllegalLSN ||
					getEndStatementLSN() == getLastTransactionLSN())

					// コミットまたはロールバックしようとしている
					// SQL 文で行った操作の論理ログを、
					// トランザクションを開始してから、
					// または直前の SQL 文を実行し終えてから
					// ひとつも記録していないので、
					// SQL 文のコミットまたはロールバックを表す
					// 論理ログを記録する必要はない

					return IllegalLSN;
				break;

			case Data::Category::TransactionPrepare:
			case Data::Category::TransactionCommit:
			case Data::Category::TransactionRollback:
				
				if (getBeginTransactionLSN() == IllegalLSN)
				{
					// コミット準備・コミット・ロールバックしようとしている
					// トランザクションを開始してから、
					// このトランザクションで行った操作の論理ログを
					// ひとつも記録していないので、トランザクションの
					// コミット準備・コミット・ロールバックを表す
					// 論理ログすら記録する必要はない
					//
					// ただし、スレーブデータベースの場合、
					// マスター側のLSNは論理ログに記録する必要がある

					if (masterLSN != Log::IllegalLSN)
					{
						AutoFile file(getFile());
						file->setMasterLSN(masterLSN);
					}

					return IllegalLSN;
				}
				break;

			default:

				// トランザクションの開始を表す論理ログを
				// 記録していなければ、まず、記録する

				if (getBeginTransactionLSN() == IllegalLSN)
				{
					if (_masterBeginTransactionLSN == IllegalLSN)
					{
						if (store(TransactionBeginData(),
								  _masterBeginTransactionLSN) == IllegalLSN)
							return IllegalLSN;
					}
					else
					{
						if (store(TransactionBeginForSlaveData(
									  _masterBeginTransactionLSN),
								  _masterBeginTransactionLSN) == IllegalLSN)
							return IllegalLSN;
					}
				}
			}

			// トランザクション中でないと記録できない論理ログに、
			//
			// * トランザクションの開始を表す論理ログ
			// * トランザクションが直前に実行した SQL 文の終了を表す論理ログ
			// * トランザクションが最後に記録した論理ログ
			//
			// のログシーケンス番号を含める

			const InsideTransactionData& tmp =
				_SYDNEY_DYNAMIC_CAST(const InsideTransactionData&, data);

			; _SYDNEY_ASSERT(getBeginTransactionLSN() != IllegalLSN);
			tmp._beginTransactionLSN = getBeginTransactionLSN();
			; _SYDNEY_ASSERT(getEndStatementLSN() != IllegalLSN);
			tmp._endStatementLSN = getEndStatementLSN();
			; _SYDNEY_ASSERT(getLastTransactionLSN() != IllegalLSN);
			tmp._backwardLSN = getLastTransactionLSN();
		}

		AutoFile file(getFile());
		if (file.isOwner()) {

			// 指定されたデータを論理ログファイルの末尾に記録し、
			// 得られたログシーケンス番号を記憶する

			_lastTransactionLSN = file->store(data, masterLSN);

			switch (data.getCategory()) {
			case Data::Category::TransactionRollback:

				// ロールバックされたので、論理ログ削除はクリアする
				
				_discardLogicalLog = DiscardLog::None;

				if (_unmounting) {

					// UNMOUNT 文を実行したトランザクションが
					// ロールバックされたので、遅延していた
					// 論理ログファイルのアンマウントは取りやめる

					_unmountDelayed.free();
					_unmounting = false;
				}
				// thru

			case Data::Category::TransactionCommit:

				// 論理ログの破棄を遅延しているか調べる

				if (_discardLogicalLog != DiscardLog::None) {

					// トランザクションがコミットされたので、
					// 不要な論理ログを削除する

					file->discardLog((_discardLogicalLog == DiscardLog::Full) ? true : false);

					// 不要な論理ログを削除したので、
					// 設定されていた情報を忘れる

					_discardLogicalLog = DiscardLog::None;
				}

				// 論理ログファイルのアンマウントを遅延しているか調べる

				if (_unmountDelayed.isOwner()) {

					if (_unmountTimeStamp != IllegalTimeStamp)
						
						// トランザクションがコミットされたので、
						// UNMOUNT文を表す論理ログのタイムスタンプを
						// タイムスタンプファイルに書く

						_unmountDelayed->setTimeStamp(_unmountTimeStamp);

					// トランザクションをコミットしたので、
					// 実際にアンマウントする

					_unmountDelayed->unmount();
					; _SYDNEY_ASSERT(!_logFile.isOwner());
					_unmountDelayed.free();
					_unmounting = false;
					_unmountTimeStamp = IllegalTimeStamp;

					// トランザクションが最後に記録した
					// 論理ログのログシーケンス番号を忘れる

					_lastTransactionLSN = IllegalLSN;
				}

				// トランザクションが終了したので、
				// トランザクションの開始を表す論理ログと
				// 最後に実行した SQL 文の終了を表す論理ログの
				// ログシーケンス番号を忘れる

				_beginTransactionLSN = _endStatementLSN = IllegalLSN;

				// トランザクションが終了したので、
				// マスターサーバのトランザクションの開始を表す論理ログの
				// ログシーケンス番号を忘れる
				
				_masterBeginTransactionLSN = IllegalLSN;
				break;

			case Data::Category::TransactionBegin:

				// トランザクションが開始したので、
				// トランザクションの開始を表す論理ログのログシーケンス番号を
				// 記録した論理ログのログシーケンス番号とする

				_beginTransactionLSN = getLastTransactionLSN();
				// thru

			case Data::Category::StatementCommit:
			case Data::Category::StatementRollback:
			case Data::Category::TransactionPrepare:

				// SQL 文の実行を終了したので、
				// それを表す論理ログのログシーケンス番号を記憶する

				_endStatementLSN = getLastTransactionLSN();
				break;

			case Data::Category::SchemaModify:
				{
					const Schema::LogData& tmp
						=_SYDNEY_DYNAMIC_CAST(const Schema::LogData&, data);
					
					if (tmp.getSubCategory() == 
						Schema::LogData::Category::Unmount ||
						tmp.getSubCategory() == 
						Schema::LogData::Category::AlterDatabase_ReadOnly) {

						// UNMOUNT 文を表す論理ログを記録したときは、
						// それを実行したトランザクションがロールバックされた
						// ときに論理ログファイルのアンマウントを取りやめる

						_unmounting = true;
					
						// トランザクションがコミットされたときに
						// UNMOUNT 文を表す論理ログをタイムタンプファイルに
						// 格納するために、タイムスタンプ値を保存する

						_unmountTimeStamp = data.getTimeStamp();
					}
					else if (tmp.getSubCategory() ==
							 Schema::LogData::Category::StartBackup) {

						// START BACKUP 文を表す論理ログを記録した時は、
						// タイムスタンプファイルに前々回のチェックポイント時の
						// タイムスタンプを書き出す

						file->storeSecondMostRecent();

					}
				}
			}

			// そのデータを保持する論理ログのログシーケンス番号を返す

			return getLastTransactionLSN();
		}
	}

	// 論理ログは記録されなかった

	return IllegalLSN;
}

//
//	FUNCTION private
//	Trans::Log::Info::restore -- 論理ログからトランザクションの状態を復元する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Log::Data&	data
//		論理ログに記録されていたデータ
//	Trans::Log::LSN lsn
//		論路ログが記録されていたログシーケンス番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Info::restore(const Data& data, LSN lsn)
{
	// 最後のログのログシーケンス番号
	
	_lastTransactionLSN = lsn;

	switch (data.getCategory()) {
	case Data::Category::TransactionRollback:
	case Data::Category::TransactionCommit:

		// トランザクションが終了したので、
		// トランザクションの開始を表す論理ログと
		// 最後に実行した SQL 文の終了を表す論理ログの
		// ログシーケンス番号を忘れる

		_beginTransactionLSN = _endStatementLSN = IllegalLSN;

		// トランザクションが終了したので、
		// マスターサーバのトランザクションの開始を表す論理ログの
		// ログシーケンス番号を忘れる
				
		_masterBeginTransactionLSN = IllegalLSN;
		break;

	case Data::Category::TransactionBegin:

		// トランザクションが開始したので、
		// トランザクションの開始を表す論理ログのログシーケンス番号を
		// 記録した論理ログのログシーケンス番号とする

		_beginTransactionLSN = getLastTransactionLSN();
		// thru

	case Data::Category::StatementCommit:
	case Data::Category::StatementRollback:
	case Data::Category::TransactionPrepare:

		// SQL 文の実行を終了したので、
		// それを表す論理ログのログシーケンス番号を記憶する

		_endStatementLSN = getLastTransactionLSN();
		break;

	default:
		break;
	}
}

#ifdef OBSOLETE
//	FUNCTION public
//	Trans::Log::Info::getFirstLSN -- 先頭の論理ログのログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

LSN
Info::getFirstLSN() const
{
	AutoFile file(getFile());
	return (file.isOwner()) ? file->getFirstLSN() : IllegalLSN;
}

//	FUNCTION public
//	Trans::Log::Info::getLastLSN -- 末尾の論理ログのログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

LSN
Info::getLastLSN() const
{
	AutoFile file(getFile());
	return (file.isOwner()) ? file->getLastLSN() : IllegalLSN;
}

//	FUNCTION public
//	Trans::Log::Info::getNextLSN --
//		ある論理ログの直後のもののログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::LSN		lsn
//			このログシーケンス番号の論理ログの
//			直後のもののログシーケンス番号を得る
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

LSN
Info::getNextLSN(LSN lsn) const
{
	AutoFile file(getFile());
	return (file.isOwner()) ? file->getNextLSN(lsn) : IllegalLSN;
}

//	FUNCTION public
//	Trans::Log::Info::getPrevLSN --
//		ある論理ログの直前のもののログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::LSN		lsn
//			このログシーケンス番号の論理ログの
//			直前のもののログシーケンス番号を得る
//
//	RETURN
//		得られたログシーケンス番号
//
//	EXCEPTIONS

LSN
Info::getPrevLSN(LSN lsn) const
{
	AutoFile file(getFile());
	return (file.isOwner()) ? file->getPrevLSN(lsn) : IllegalLSN;
}
#endif

//	FUNCTION public
//	Trans::Log::Info::getDatabaseID --
//		論理ログに記録する操作の対象であるデータベースのオブジェクト識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたオブジェクト識別子
//
//	EXCEPTIONS
//		なし

Schema::ObjectID::Value
Info::getDatabaseID() const
{
	const AutoFile file(getFile());
	return (file.isOwner()) ?
		static_cast<Schema::ObjectID::Value>(
			file->getLockName().getDatabasePart()) : Schema::ObjectID::Invalid;
}

//	FUNCTION
//	Trans::Log::Info::getDatabaseName --
//		論理ログに記録する操作の対象であるデータベースの名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた名前
//
//	EXCEPTIONS

ModUnicodeString
Info::getDatabaseName() const
{
	const AutoFile file(getFile());
	return (file.isOwner()) ? file->getDatabaseName() : ModUnicodeString();
}

//	FUNCTION public
//	Trans::Log::Info::getFile --
//		操作する論理ログファイルの情報を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた操作する論理ログファイルの情報を表すクラスを
//		格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

AutoFile
Info::getFile() const
{
	return (_unmountDelayed.isOwner()) ? _unmountDelayed :
		(_logFile.isOwner()) ? _logFile : AutoFile(0);
}

//	FUNCTION public
//	Trans::Log::Info::getLockName -- 論理ログファイルを表すロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたロック名
//
//	EXCEPTIONS
//		なし

Lock::LogicalLogName
Info::getLockName() const
{
	const AutoFile file(getFile());
	return (file.isOwner()) ?
		file->getLockName() : Lock::LogicalLogName(Schema::ObjectID::Invalid);
}

//
//	FUNCTION public
//	Trans::Log::Info::setSyncDone -- 同期処理が完了していることを設定する
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
Info::setSyncDone()
{
	AutoFile file(getFile());
	if (file.isOwner())
	{
		file->setSyncDone(true);
	}
}

//
//	FUNCTION public
//	Trans::Log::Info::discardLog
//		-- コミット時に不要な論理ログを削除することを設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool bDiscardFull_
//			最新以外すべて削除する場合はtrue、前々回のチェックポイント以前を
//			削除する場合はfalseを指定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Info::discardLog(bool bDiscardFull_)
{
	if (bDiscardFull_)

		// 最新のもの以外すべて削除する
		
		_discardLogicalLog = DiscardLog::Full;

	else

		// 前々回のチェックポイント以前のものを削除する

		_discardLogicalLog = DiscardLog::Checkpoint;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
