// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.h --	論理ログデータ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_LOGDATA_H
#define	__SYDNEY_CHECKPOINT_LOGDATA_H

#include "Checkpoint/Module.h"
#include "Checkpoint/Database.h"
#include "Checkpoint/Externalizable.h"

#include "Schema/Object.h"
#include "Trans/LogData.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_CHECKPOINT_BEGIN
_SYDNEY_CHECKPOINT_LOG_BEGIN

//	CLASS
//	Checkpoint::Log::CheckpointData --
//		チェックポイント処理の終了の論理ログデータを表すクラス
//
//	NOTES

class CheckpointData
	: public	Trans::Log::Data
{
public:
	// コンストラクター
	CheckpointData(Trans::Log::Data::Category::Value category =
						Trans::Log::Data::Category::Unknown);
	CheckpointData(Trans::Log::Data::Category::Value category,
				   Trans::Log::Data::VersionNumber::Value version);
	CheckpointData(Trans::Log::Data::Category::Value category,
				   Trans::Log::Data::VersionNumber::Value version,
				   const Trans::TimeStamp& finish,
				   const Trans::TimeStamp& mostRecent,
				   bool synchronized);
	// デストラクター
	~CheckpointData();

	// このクラスをシリアル化する
	SYD_CHECKPOINT_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int				getClassID() const;

	// チェックポイント処理の終了直前に取得したタイムスタンプを得る
	const Trans::TimeStamp&
	getFinishTimeStamp() const;
	// 直前のチェックポイント処理の終了時のタイムスタンプを得る
	const Trans::TimeStamp&
	getMostRecentTimeStamp() const;

	// チェックポイント処理の終了時に
	// バッファとディスクの内容が完全に一致しているか調べる
	bool					isSynchronized() const;

private:
	// 直前のチェックポイント処理の終了時のタイムスタンプ
	Trans::TimeStamp		_mostRecent;
	// チェックポイント処理の終了時に
	// バッファとディスクの内容が完全に一致しているか
	bool					_synchronized;
};

//	CLASS
//	Checkpoint::Log::CheckpointDatabaseData --
//		データベースに関するチェックポイント処理の終了の
//		論理ログデータを表すクラス
//
//	NOTES

class CheckpointDatabaseData
	: public	CheckpointData
{
public:
	//	STRUCT
	//	Checkpoint::Log::CheckpointDatabaseData::TransactionInfo --
	//		あるデータベースに関するチェックポイント処理の終了時に、
	//		このデータベースを操作中のトランザクションに関する情報を表すクラス
	//
	//	NOTES

	struct TransactionInfo
	{
		// デフォルトコンストラクター
		TransactionInfo();
		// コンストラクター
		TransactionInfo(Trans::Log::LSN beginLSN, Trans::Log::LSN lastLSN,
						const Trans::XID& preparedXID);

		// トランザクションの開始を表す論理ログの LSN
		Trans::Log::LSN			_beginLSN;
		// トランザクションが直前に記録した論理ログの LSN
		Trans::Log::LSN			_lastLSN;
		// トランザクションがコミット準備済のトランザクションブランチの
		// 実体であれば、そのトランザクションブランチ識別子
		Trans::XID				_preparedXID;
	};

	// コンストラクター
	CheckpointDatabaseData(Trans::Log::Data::VersionNumber::Value version);
	CheckpointDatabaseData(
		Trans::Log::Data::VersionNumber::Value version,
		const Trans::TimeStamp& finish,
		const Trans::TimeStamp& mostRecent,
		bool synchronized, bool terminated,
		const ModVector<TransactionInfo>& transInfo);
	// デストラクター
	~CheckpointDatabaseData();

	// このクラスをシリアル化する
	SYD_CHECKPOINT_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	SYD_CHECKPOINT_FUNCTION
	virtual int
	getClassID() const;

	// チェックポイント処理の終了時に、あるデータベースを操作中の
	// トランザクションに関する情報を表すクラスを得る
	const ModVector<TransactionInfo>&
	getTransactionInfo() const;
	// 記録直後にチェックポイントスレッドは終了したか
	bool					isTerminated() const;

	// チェックポイント処理の終了時に、あるデータベースを操作中の
	// トランザクションのログシーケンス番号の最小値を得る
	Trans::Log::LSN			getBeginTransactionLSN() const;

private:
	// チェックポイント処理の終了時に、あるデータベースを操作中の
	// トランザクションに関する情報を表すクラス
	ModVector<TransactionInfo>	_transInfo;
	// 記録直後にチェックポイントスレッドは終了したか
	bool						_terminated;
};

//	CLASS
//	Checkpoint::Log::CheckpointSystemData --
//		システムに関するチェックポイント処理の終了の論理ログデータを表すクラス
//
//	NOTES

class CheckpointSystemData
	: public	CheckpointData
{
public:
	// コンストラクター
	CheckpointSystemData(Trans::Log::Data::VersionNumber::Value version);
	CheckpointSystemData(
		Trans::Log::Data::VersionNumber::Value version,
		const Trans::TimeStamp& finish,
		const Trans::TimeStamp& mostRecent,
		bool synchronized, bool terminated, bool unavailable,
		const Database::UnavailableMap& unavailableDatabase,
		const ModVector<Trans::Branch::HeurCompletionInfo>& heurCompletionInfo);
	// デストラクター
	~CheckpointSystemData();

	// このクラスをシリアル化する
	SYD_CHECKPOINT_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	SYD_CHECKPOINT_FUNCTION
	virtual int
	getClassID() const;

	// 記録直後にチェックポイントスレッドは終了したか
	bool					isTerminated() const;
	// メタデータベースは利用不可か
	bool					isUnavailable() const;
	// 利用不可なデータベースの一覧を得る
	const Database::UnavailableMap&
	getUnavailableDatabase() const;
	// ヒューリスティックに解決済のすべてのトランザクションブランチの情報を得る
	const ModVector<Trans::Branch::HeurCompletionInfo>&
	getHeurCompletionInfo() const;

private:
	// 記録直後にチェックポイントスレッドは終了したか
	bool					_terminated;
	// メタデータベースは利用不可か
	bool					_unavailable;
	// 利用不可なデータベースの一覧
	Database::UnavailableMap _unavailableDatabase;
	// ヒューリスティックに解決済のすべてのトランザクションブランチの情報
	ModVector<Trans::Branch::HeurCompletionInfo> _heurCompletionInfo;
};

//	CLASS
//	Checkpoint::Log::FileSynchronizeBeginData --
//		あるデータベースに関するバージョンファイルの同期の開始を表す
//		論理ログデータを表すクラス
//
//	NOTES

class FileSynchronizeBeginData
	: public	Trans::Log::ModificationData
{
public:
	// コンストラクター
	FileSynchronizeBeginData();
	// デストラクター
	~FileSynchronizeBeginData();

	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;
};

//	CLASS
//	Checkpoint::Log::FileSynchronizeEndData --
//		あるデータベースに関するバージョンファイルの同期の終了を表す
//		論理ログデータを表すクラス
//
//	NOTES

class FileSynchronizeEndData
	: public	Trans::Log::ModificationData
{
public:
	// コンストラクター
	FileSynchronizeEndData(bool modified = true);
	// デストラクター
	~FileSynchronizeEndData();

	// このクラスをシリアル化する
	SYD_CHECKPOINT_FUNCTION
	virtual void
	serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;

	// 同期処理でデータベースが更新されたか
	bool
	isModified() const;

private:
	// 同期処理でデータベースが更新されたか
	bool					_modified;
};

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::CheckpointData --
//		チェックポイント処理の終了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data::Category::Value	category
//			指定されたとき
//				論理ログデータの種別
//			指定されないとき
//				Trans::Log::Data::Category::Unknown が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointData::CheckpointData(Trans::Log::Data::Category::Value category)
	: Trans::Log::Data(category),
	  _synchronized(false)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::CheckpointData --
//		チェックポイント処理の終了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data::Category::Value	category
//			論理ログデータの種別
//		Trans::Log::Data::VersionNumber::Value	version
//			category で与えられた種別の論理ログデータのバージョン番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointData::CheckpointData(Trans::Log::Data::Category::Value category,
							   Trans::Log::Data::VersionNumber::Value version)
	: Trans::Log::Data(category, version),
	  _synchronized(false)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::CheckpointData --
//		チェックポイント処理の終了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data::Category::Value	category
//			論理ログデータの種別
//		Trans::Log::Data::VersionNumber::Value	version
//			category で与えられた種別の論理ログデータのバージョン番号
//		Trans::TimeStamp&	finish
//			チェックポイント処理の終了直前に取得したタイムスタンプ
//		Trans::TimeStamp&	mostRecent
//			直前のチェックポイント処理の終了時のタイムスタンプ
//		bool				synchronized
//			true
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致している
//			false
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致していない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointData::
CheckpointData(Trans::Log::Data::Category::Value category,
			   Trans::Log::Data::VersionNumber::Value version,
			   const Trans::TimeStamp& finish,
			   const Trans::TimeStamp& mostRecent,
			   bool synchronized)
	: Trans::Log::Data(category, version, finish),
	  _mostRecent(mostRecent),
	  _synchronized(synchronized)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::~CheckpointData --
//		チェックポイント処理の終了の
//		論理ログデータを表すクラスのデストラクター
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

inline
CheckpointData::~CheckpointData()
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::getClassID --
//		このクラスのクラス ID を得る
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

inline
int
CheckpointData::getClassID() const
{
	return Checkpoint::Externalizable::Category::CheckpointLogData +
		Common::Externalizable::CheckpointClasses;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::getFinishTimeStamp --
//		チェックポイント処理の終了直前に取得したタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

inline
const Trans::TimeStamp&
CheckpointData::getFinishTimeStamp() const
{
	return getTimeStamp();
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::getMostRecentTimeStamp --
//		直前のチェックポイント処理の終了時のタイムスタンプを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたタイムスタンプ
//
//	EXCEPTIONS
//		なし

inline
const Trans::TimeStamp&
CheckpointData::getMostRecentTimeStamp() const
{
	return _mostRecent;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::isSynchronized --
//		チェックポイント処理の終了時にバッファとディスクの内容が
//		完全に一致しているかを調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			一致している
//		false
//			一致していない
//
//	EXCEPTIONS
//		なし

inline
bool
CheckpointData::isSynchronized() const
{
	return _synchronized;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::CheckpointDatabaseData --
//		データベースに関するチェックポイント処理の終了の
//		論理ログデータを表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans:::Log::Data::VersionNumber::Value	version
//			この論理ログデータのバージョン番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointDatabaseData::CheckpointDatabaseData(VersionNumber::Value version)
	: CheckpointData(Trans::Log::Data::Category::CheckpointDatabase, version)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::CheckpointDatabaseData --
//		データベースに関するチェックポイント処理の終了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans:::Log::Data::VersionNumber::Value	version
//			この論理ログデータのバージョン番号
//		Trans::TimeStamp&	finish
//			チェックポイント処理の終了直前に取得したタイムスタンプ
//		Trans::TimeStamp&	mostRecent
//			直前のチェックポイント処理の終了時のタイムスタンプ
//		bool				synchronized
//			true
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致している
//			false
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致していない
//		ModVector<Trans::Log::LSN>&	beginLSN
//			チェックポイント処理の終了時に、あるデータベースを操作中の
//			トランザクションの開始を表す論理ログのログシーケンス番号
//		ModVector<Trans::Log::LSN>&	lastLSN
//			チェックポイント処理の終了時に、あるデータベースを操作中の
//			トランザクションが直前に記録した論理ログのログシーケンス番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
CheckpointDatabaseData::CheckpointDatabaseData(
	Trans::Log::Data::VersionNumber::Value version,
	const Trans::TimeStamp& finish,
	const Trans::TimeStamp& mostRecent,
	bool synchronized,
	bool terminated,
	const ModVector<TransactionInfo>& transInfo)
	: CheckpointData(Trans::Log::Data::Category::CheckpointDatabase, version,
					 finish, mostRecent, synchronized),
	  _transInfo(transInfo), _terminated(terminated)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::~CheckpointDatabaseData --
//		データベースに関するチェックポイント処理の終了の
//		論理ログデータを表すクラスのデストラクター
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

inline
CheckpointDatabaseData::~CheckpointDatabaseData()
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::getTransactionInfo --
//		チェックポイント処理の終了時に、あるデータベースを操作中の
//		トランザクションに関する情報を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたチェックポイント処理の終了時に、あるデータベースを操作中の
//		トランザクションに関する情報を表すクラスを
//		要素とするベクタへのレファレンス
//
//	EXCEPTIONS
//		なし

inline
const ModVector<CheckpointDatabaseData::TransactionInfo>&
CheckpointDatabaseData::getTransactionInfo() const
{
	return _transInfo;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::isTerminated --
//		論理ログを記録直後にチェックポイントスレッドが終了したか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			終了した
//		false
//			終了しなかった
//
//	EXCEPTIONS
//		なし

inline
bool
CheckpointDatabaseData::isTerminated() const
{
	return _terminated;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::getBeginTransactionLSN --
//		チェックポイント処理終了時に、あるデータベースを操作中の
//		トランザクションのログシーケンス番号の最小値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		実行中のトランザクションのうち、開始時のログシーケンス番号が最小のもの
//
//	EXCEPTIONS

inline
Trans::Log::LSN
CheckpointDatabaseData::getBeginTransactionLSN() const
{
	Trans::Log::LSN lsn = Trans::Log::IllegalLSN;
	ModVector<TransactionInfo>::ConstIterator i = _transInfo.begin();
	for (; i != _transInfo.end(); ++i) {
		if ((*i)._beginLSN == Trans::Log::IllegalLSN)
			continue;
		if (lsn == Trans::Log::IllegalLSN || lsn > (*i)._beginLSN)
			lsn = (*i)._beginLSN;
	}
	return lsn;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::TransactionInfo::TransactionInfo --
//		あるデータベースに関するチェックポイント処理の終了時に、
//		このデータベースを操作中のトランザクションに関する
//		情報を表すクラスのデフォルトコンストラクター
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

inline
CheckpointDatabaseData::TransactionInfo::TransactionInfo()
	: _beginLSN(Trans::Log::IllegalLSN),
	  _lastLSN(Trans::Log::IllegalLSN)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::TransactionInfo::TransactionInfo --
//		あるデータベースに関するチェックポイント処理の終了時に、
//		このデータベースを操作中のトランザクションに関する
//		情報を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::LSN			_beginLSN;
//			トランザクションの開始を表す論理ログの LSN
//		Trans::Log::LSN			_lastLSN;
//			トランザクションが直前に記録した論理ログの LSN
//		Trans::XID				_preparedXID;
//			トランザクションがコミット準備済のトランザクションブランチの
//			実体であれば、そのトランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointDatabaseData::TransactionInfo::TransactionInfo(
	Trans::Log::LSN beginLSN, Trans::Log::LSN lastLSN,
	const Trans::XID& preparedXID)
	: _beginLSN(beginLSN),
	  _lastLSN(lastLSN),
	  _preparedXID(preparedXID)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::CheckpointSystemData --
//		システムに関するチェックポイント処理の終了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans:::Log::Data::VersionNumber::Value	version
//			この論理ログデータのバージョン番号
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointSystemData::CheckpointSystemData(VersionNumber::Value version)
	: CheckpointData(Trans::Log::Data::Category::CheckpointSystem, version)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::CheckpointSystemData --
//		システムに関するチェックポイント処理の終了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans:::Log::Data::VersionNumber::Value	version
//			この論理ログデータのバージョン番号
//		Trans::TimeStamp&	finish
//			チェックポイント処理の終了直前に取得したタイムスタンプ
//		Trans::TimeStamp&	mostRecent
//			直前のチェックポイント処理の終了時のタイムスタンプ
//		bool				synchronized
//			true
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致している
//			false
//				チェックポイント処理の終了時に
//				バッファとディスクの内容が完全に一致していない
//		bool				terminated
//			true
//				記録直後にチェックポイントスレッドは終了した
//			false
//				継続してチェックポイントスレッドは実行される
//		bool				unavailable
//			true
//				メタデータベースが利用可能である
//			false
//				メタデータベースは利用不可である
//		Checkpoint::Database::UnavailableMap&	unavailableDatabase
//			利用不可なデータベースの名前を管理するマップ
//		ModVector<Trans::Branch::HeurCompletionInfo>&	heurCompletionInfo
//			ヒューリスティックに解決済の
//			トランザクションブランチの情報を要素とする配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
CheckpointSystemData::CheckpointSystemData(
	Trans::Log::Data::VersionNumber::Value version,
	const Trans::TimeStamp& finish,
	const Trans::TimeStamp& mostRecent,
	bool synchronized, bool terminated, bool unavailable,
	const Database::UnavailableMap& unavailableDatabase,
	const ModVector<Trans::Branch::HeurCompletionInfo>& heurCompletionInfo)
	: CheckpointData(Trans::Log::Data::Category::CheckpointSystem, version,
					 finish, mostRecent, synchronized),
	  _terminated(terminated),
	  _unavailable(unavailable),
	  _unavailableDatabase(unavailableDatabase),
	  _heurCompletionInfo(heurCompletionInfo)
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::~CheckpointSystemData --
//		システムに関するチェックポイント処理の終了の
//		論理ログデータを表すクラスのデストラクター
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

inline
CheckpointSystemData::~CheckpointSystemData()
{}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::isTerminated --
//		論理ログを記録直後にチェックポイントスレッドが終了したか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			終了した
//		false
//			終了しなかった
//
//	EXCEPTIONS
//		なし

inline
bool
CheckpointSystemData::isTerminated() const
{
	return _terminated;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::isUnavailable --
//		チェックポイント処理の終了時にメタデータベースが利用不可か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			メタデータベースは利用不可である
//		false
//			メタデータベースは利用不可でない
//
//	EXCEPTIONS
//		なし

inline
bool
CheckpointSystemData::isUnavailable() const
{
	return _unavailable;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::getUnavailableDatabase --
//		チェックポイント処理の終了時に利用不可なデータベースの一覧を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		利用不可なデータベースの名前を管理するマップ
//
//	EXCEPTIONS
//		なし

inline
const Database::UnavailableMap&
CheckpointSystemData::getUnavailableDatabase() const
{
	return _unavailableDatabase;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpoingSystemData::getHeurCompletionInfo --
//		ヒューリスティックに解決済のすべてのトランザクションブランチの情報を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ヒューリスティックに解決済の
//		トランザクションブランチの情報を要素とする配列
//
//	EXCEPTIONS
//		なし

inline
const ModVector<Trans::Branch::HeurCompletionInfo>&
CheckpointSystemData::getHeurCompletionInfo() const
{
	return _heurCompletionInfo;
}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeBeginData::FileSynchronizeBeginData --
//		あるデータベースに関するバージョンファイルの同期の開始を表す
//		論理ログデータを表すクラスのコンストラクター
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

inline
FileSynchronizeBeginData::FileSynchronizeBeginData()
	: Trans::Log::ModificationData(
		Trans::Log::Data::Category::FileSynchronizeBegin, false)
{}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeBeginData::~FileSynchronizeBeginData --
//		あるデータベースに関するバージョンファイルの同期の開始を表す
//		論理ログデータを表すクラスのデストラクター
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

inline
FileSynchronizeBeginData::~FileSynchronizeBeginData()
{}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeBeginData::getClassID --
//		このクラスのクラス ID を得る
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

inline
int
FileSynchronizeBeginData::getClassID() const
{
	return Checkpoint::Externalizable::Category::FileSynchronizeBeginLogData +
		Common::Externalizable::CheckpointClasses;
}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeEndData::FileSynchronizeEndData --
//		あるデータベースに関するバージョンファイルの同期の終了を表す
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		bool				modified
//			true または指定されないとき
//				同期処理でデータベースは更新された
//			false
//				同期処理でデータベースは更新されなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
FileSynchronizeEndData::FileSynchronizeEndData(bool modified)
	: Trans::Log::ModificationData(
		Trans::Log::Data::Category::FileSynchronizeEnd, false),
	  _modified(modified)
{}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeEndData::~FileSynchronizeEndData --
//		あるデータベースに関するバージョンファイルの同期の終了を表す
//		論理ログデータを表すクラスのデストラクター
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

inline
FileSynchronizeEndData::~FileSynchronizeEndData()
{}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeEndData::getClassID --
//		このクラスのクラス ID を得る
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

inline
int
FileSynchronizeEndData::getClassID() const
{
	return Checkpoint::Externalizable::Category::FileSynchronizeEndLogData +
		Common::Externalizable::CheckpointClasses;
}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeEndData::isModified --
//		同期処理でデータベースが更新されたか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			更新された
//		false
//			更新されなかった
//
//	EXCEPTIONS
//		なし

inline
bool
FileSynchronizeEndData::isModified() const
{
	return _modified;
}

_SYDNEY_CHECKPOINT_LOG_END
_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_LOGDATA_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
