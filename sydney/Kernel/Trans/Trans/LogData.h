// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.h --	論理ログデータ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_LOGDATA_H
#define	__SYDNEY_TRANS_LOGDATA_H

#include "Trans/Module.h"
#include "Trans/Branch.h"
#include "Trans/Externalizable.h"
#include "Trans/LogFile.h"
#include "Trans/TimeStamp.h"

#include "Common/Externalizable.h"
#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN
_SYDNEY_TRANS_LOG_BEGIN

//	CLASS
//	Trans::Log::Data -- 論理ログデータを表すクラス
//
//	NOTES

class Data
	: public	Common::Object,				//【注意】	最初に継承すること
	  public	Common::Externalizable
{
	friend class File;
public:
	//	CLASS
	//	Trans::Log::Data::Category -- 論理ログデータの種別を表すクラス
	//
	//	NOTES

	struct Category
	{
		//	ENUM
		//	Trans::Log::Data::Category::Value --
		//		論理ログデータの種別を表す値の列挙型
		//
		//	NOTES

		//【注意】	論理ログファイルに永続化されているので、
		//			互換性を保つため、新しい値を加えるときは末尾に加えること
		//【注意】	ログ出力のため、新しい値を加えたときは、
		//			Log::Data::toString() 内の category にも加えること

		enum Value
		{
			// 不明
			Unknown =		0,

			// タイムスタンプの上位 32 ビットを割り当てる
			TimeStampAssign,

			// トランザクションの開始
			TransactionBegin,
			// トランザクションのコミット
			TransactionCommit,
			// トランザクションのロールバック
			TransactionRollback,
			// SQL 文のコミット
			StatementCommit,
			// SQL 文のロールバック
			StatementRollback,

			// データベースに対するチェックポイント処理の終了
			CheckpointDatabase,
			// システムに対するチェックポイント処理の終了
			CheckpointSystem,

			// タプルの更新
			TupleModify,
			// スキーマデータベースの更新
			SchemaModify,
			// バージョンファイルの同期の開始
			FileSynchronizeBegin,
			// バージョンファイルの同期の終了
			FileSynchronizeEnd,

			// トランザクションブランチのコミット準備完了
			TransactionPrepare,
			// トランザクションブランチのヒューリスティックな解決
			BranchHeurDecide,
			// ヒューリスティックな解決済のトランザクションブランチの抹消
			BranchForget,

			// ファイルドライバ内の更新処理
			DriverModify,

			// バッチモードの開始処理
			StartBatch,

			// 分散トランザクション
			XATransaction,

			// レプリケーションの終了
			ReplicationEnd,

			// 値の数
			Count
		};
	};

	struct VersionNumber
	{
		//	ENUM
		//	Trans::Log::Data::VersionNumber::Value --
		//		論理ログデータのバージョンを表す値の列挙型
		//
		//	NOTES

		//【注意】	この値は論理ログファイルに永続化されない。
		//			永続化するクラス IDをバージョンによって変えることにより、
		//			バージョンごとに読み書きするメンバを変更する

		enum Value
		{
			// 不明
			Unknown =			-1,

			First =				0,
			Second,
			Third,
			
			// 値の数
			Count
		};
	};

	// コンストラクター
	Data(Category::Value category = Category::Unknown);
	Data(Category::Value category, const TimeStamp& timeStamp);
	Data(Category::Value category, VersionNumber::Value version);
	Data(Category::Value category, VersionNumber::Value version,
		 const TimeStamp& timeStamp);
	// デストラクター
	virtual ~Data();

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;

	// 種別を得る
	Category::Value			getCategory() const;
	// バージョン番号を得る
	VersionNumber::Value
	getVersionNumber() const;
	// 記録する直前に取得したタイムスタンプを得る
	const TimeStamp&		getTimeStamp() const;
	// タイムスタンプをクリアする
	void					clearTimeStamp();

	// ログデータが Trans::Log::InsideTransactionData を継承しているか
	virtual bool
	isInsideTransactionData() const
	{
		return false;
	}

#ifndef SYD_COVERAGE
	// オブジェクトを表す文字列を得る
	SYD_TRANS_FUNCTION
	virtual ModUnicodeString toString() const;
#endif
private:
	// 種別
	mutable Category::Value	_category;
	// バージョン番号
	mutable VersionNumber::Value _version;
	// 記録する直前に取得したタイムスタンプ
	mutable TimeStamp		_timeStamp;
};

//	CLASS
//	Trans::Log::TransactionBeginData --
//		トランザクションの開始の論理ログデータを表すクラス
//
//	NOTES

class TransactionBeginData
	: public	Data
{
public:
	// コンストラクター
	TransactionBeginData();
	// デストラクター
	~TransactionBeginData();

	// このクラスのクラス ID を得る
	virtual int				getClassID() const;
};

//	CLASS
//	Trans::Log::InsideTransactionData --
//		トランザクション中であるときのみ記録可能な論理ログデータを表すクラス
//
//	NOTES

class InsideTransactionData
	: public	Data
{
	friend class Info;
public:
	// コンストラクター
	InsideTransactionData(Category::Value category = Category::Unknown);
	// デストラクター
	~InsideTransactionData();

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int				getClassID() const;

	// ログデータが Trans::Log::InsideTransactionData を継承しているか
	virtual bool
	isInsideTransactionData() const
	{
		return true;
	}

	// トランザクションの開始を表す論理ログのログシーケンス番号を得る
	LSN						getBeginTransactionLSN() const;
	// トランザクションが直前に実行した SQL 文の終了を表す
	// 論理ログのログシーケンス番号を得る
	LSN						getEndStatementLSN() const;
	// トランザクションが直前に挿入した論理ログのログシーケンス番号を得る
	LSN						getBackwardLSN() const;

private:
	// トランザクションの開始を表す論理ログのログシーケンス番号
	mutable LSN				_beginTransactionLSN;
	// トランザクションが直前に実行した SQL 文の終了を表す
	// 論理ログのログシーケンス番号
	mutable LSN				_endStatementLSN;
	// トランザクションが直前に挿入した論理ログのログシーケンス番号
	mutable LSN				_backwardLSN;
};

//	CLASS
//	Trans::Log::ModificationData --
//		トランザクション中で実行される更新操作の論理ログデータを表すクラス
//
//	NOTES

class ModificationData
	: public	InsideTransactionData
{
public:
	// コンストラクター
	ModificationData(Category::Value category = Category::Unknown,
					 bool undoable = true);
	// デストラクター
	~ModificationData();

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int				getClassID() const;

	// UNDO 可能な更新操作を表すか
	bool					isUndoable() const;

private:
	// UNDO 可能な更新操作を表すか
	bool					_undoable;
};

//	CLASS
//	Trans::Log::TransactionCommitData --
//		トランザクションのコミットの論理ログデータを表すクラス
//
//	NOTES

class TransactionCommitData
	: public	InsideTransactionData
{
public:
	// コンストラクター
	TransactionCommitData();
	// デストラクター
	~TransactionCommitData();

	// このクラスのクラス ID を得る
	virtual int				getClassID() const;
};

//	CLASS
//	Trans::Log::TransactionRollbackData --
//		トランザクションのロールバックの論理ログデータを表すクラス
//
//	NOTES

class TransactionRollbackData
	: public	InsideTransactionData
{
public:
	// コンストラクター
	TransactionRollbackData();
	// デストラクター
	~TransactionRollbackData();

	// このクラスのクラス ID を得る
	virtual int				getClassID() const;
};

//	CLASS
//	Trans::Log::StatementCommitData --
//		SQL 文のコミットの論理ログデータを表すクラス
//
//	NOTES

class StatementCommitData
	: public	InsideTransactionData
{
public:
	// コンストラクター
	StatementCommitData();
	// デストラクター
	~StatementCommitData();

	// このクラスのクラス ID を得る
	virtual int				getClassID() const;
};

//	CLASS
//	Trans::Log::StatementRollbackData --
//		SQL 文のロールバックの論理ログデータを表すクラス
//
//	NOTES

class StatementRollbackData
	: public	InsideTransactionData
{
public:
	// コンストラクター
	StatementRollbackData();
	// デストラクター
	~StatementRollbackData();

	// このクラスのクラス ID を得る
	virtual int				getClassID() const;
};

//	CLASS
//	Trans::Log::TimeStampAssignData --
//		新たなタイムスタンプ値の上位 32 ビットの生成の
//		論理ログデータを表すクラス
//
//	NOTES

class TimeStampAssignData
	: public	Data
{
	friend class Info;
public:
	// デフォルトコンストラクター
	TimeStampAssignData();
	// コンストラクター
	TimeStampAssignData(const TimeStamp& assigned);
	// デストラクター
	~TimeStampAssignData();

	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;

	// 生成されたタイムスタンプを得る
	const TimeStamp&
	getAssigned() const;
};

//	CLASS
//	Trans::Log::TransactionPrepareData --
//		トランザクションのコミット準備完了の論理ログデータを表すクラス
//
//	NOTES

class TransactionPrepareData
	: public	InsideTransactionData
{
public:
	// デフォルトコンストラクター
	TransactionPrepareData();
	// コンストラクター
	TransactionPrepareData(const XID& xid);
	// デストラクター
	~TransactionPrepareData();

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void
	serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;

	// コミット準備完了するトランザクションブランチの
	// トランザクションブランチ識別子を得る
	const XID&
	getXID() const;

private:
	// コミット準備解決する
	// トランザクションブランチのトランザクションブランチ識別子
	XID						_xid;
};

//	CLASS
//	Trans::Log::BranchHeurDecideData --
//		トランザクションブランチのヒューリスティックな解決の
//		論理ログデータを表すクラス
//
//	NOTES

class BranchHeurDecideData
	: public	Data
{
public:
	// デフォルトコンストラクター
	BranchHeurDecideData();
	// コンストラクター
	BranchHeurDecideData(const XID& xid, Branch::HeurDecision::Value desicion);
	// デストラクター
	~BranchHeurDecideData();

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void
	serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;

	// ヒューリスティックに解決するトランザクションブランチの
	// トランザクションブランチ識別子を得る
	const XID&
	getXID() const;
	// どうやってヒューリステックに解決するかを得る
	Branch::HeurDecision::Value
	getDecision() const;

private:
	// ヒューリスティックに解決する
	// トランザクションブランチのトランザクションブランチ識別子
	XID						_xid;
	// どうやってヒューリスティックに解決するか
	Branch::HeurDecision::Value	_decision;
};

//	CLASS
//	Trans::Log::BranchForgetData --
//		ヒューリスティックな解決済の
//		トランザクションブランチの抹消の論理ログデータを表すクラス
//
//	NOTES

class BranchForgetData
	: public	Data
{
public:
	// デフォルトコンストラクター
	BranchForgetData();
	// コンストラクター
	BranchForgetData(const XID& xid);
	// デストラクター
	~BranchForgetData();

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void
	serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;

	// 抹消したトランザクションブランチの
	// トランザクションブランチ識別子を得る
	const XID&
	getXID() const;

private:
	// 抹消したトランザクションブランチのトランザクションブランチ識別子
	XID						_xid;
};

//	CLASS
//	Trans::Log::StartBatchData --
//		バッチモード開始の論理ログデータを表すクラス
//
//	NOTES

class StartBatchData
	: public	InsideTransactionData
{
public:
	// デフォルトコンストラクター
	StartBatchData();
	// デストラクター
	~StartBatchData();

	// このクラスのクラス ID を得る
	virtual int
	getClassID() const;
private:
};

//	CLASS
//	Trans::Log::XATransactionData --
//		分散トランザクションの論理ログデータを表すクラス
//
//	NOTES

class XATransactionData
	: public	InsideTransactionData
{
public:
	// コンストラクター
	XATransactionData()
		: InsideTransactionData(Category::XATransaction) {}
	// コンストラクター
	XATransactionData(const XID& xid_)
		: InsideTransactionData(Category::XATransaction), _xid(xid_) {}
	// デストラクター
	~XATransactionData() {}

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void			serialize(ModArchive& archiver_);

	// 分散トランザクションが管理する
	// トランザクションブランチ識別子を得る
	const XID&				getXID() const { return _xid; }

	// このクラスのクラス ID を得る
	virtual int				getClassID() const
		{
			return Trans::Externalizable::Category::XATransactionLogData +
				Common::Externalizable::TransClasses;
		}
	
private:
	// 分散トランザクションが管理するトランザクションブランチ識別子
	XID						_xid;
};

//	CLASS
//	Trans::Log::TransactionBeginForSlaveData --
//		トランザクションの開始の論理ログデータを表すクラス(スレーブ用)
//
//	NOTES

class TransactionBeginForSlaveData
	: public	TransactionBeginData
{
	friend class Info;
public:
	// コンストラクター(1)
	TransactionBeginForSlaveData()
		: TransactionBeginData(),
		  m_uiMasterLSN(IllegalLSN) {}
	// コンストラクター(2)
	TransactionBeginForSlaveData(LSN uiMasterLSN_)
		: TransactionBeginData(),
		  m_uiMasterLSN(uiMasterLSN_) {}
	// デストラクター
	~TransactionBeginForSlaveData() {}

	// このクラスをシリアル化する
	SYD_TRANS_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	virtual int				getClassID() const
	{
		return
			Trans::Externalizable::Category::TransactionBeginForSlaveLogData +
			Common::Externalizable::TransClasses;
	}
	
	// マスター側の
	// トランザクションの開始を表す論理ログのログシーケンス番号を得る
	LSN						getMasterLSN() const
	{
		return m_uiMasterLSN;
	}

private:
	// マスター側の
	// トランザクションの開始を表す論理ログのログシーケンス番号
	mutable LSN				m_uiMasterLSN;
};

//【注意】	さまざまな論理ログデータを表すクラスの親子関係は以下のとおり
//
//	Data ┬TransactionBeginData
//       ├InsideTransactionData ┬ModificationData
//	     │                      ├TransactionCommitData
//		 │                      ├TransactionRollbackData
//		 │                      ├StatementCommitData
//		 │                      ├StatementRollbackData
//		 │                      ├TransactionPrepareData
//	     │                      ├StartBatchData
//	     │                      └XATransactionData
//		 ├TimeStampAssignData
//		 ├BranchHeurDecideData
//		 └BranchForgetData

//	FUNCTION public
//	Trans::Log::Data::Data --
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
Data::Data(Category::Value category)
	: _category(category),
	  _version(VersionNumber::Unknown)
{}

//	FUNCTION public
//	Trans::Log::Data::Data --
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data::Category::Value	category
//			論理ログデータの種別
//		Trans::TimeStamp&	timeStamp
//			論理ログデータを記録したときのタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Data::Data(Category::Value category, const TimeStamp& timeStamp)
	: _category(category),
	  _version(VersionNumber::Unknown),
	  _timeStamp(timeStamp)
{}

//	FUNCTION public
//	Trans::Log::Data::Data --
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Data::Category::Value	category
//			論理ログデータの種別
//		Trans::Log::Data::VersionNumber::Value	version
//			category で与えられた種別の論理ログデータのバージョン番号
//		Trans::TimeStamp&	timeStamp
//			指定されたとき
//				論理ログデータを記録したときのタイムスタンプ
//			指定されないとき
//				Trans::TimeStamp() が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Data::Data(Category::Value category, VersionNumber::Value version)
	: _category(category),
	  _version(version)
{}

inline
Data::Data(Category::Value category, VersionNumber::Value version,
		   const TimeStamp& timeStamp)
	: _category(category),
	  _version(version),
	  _timeStamp(timeStamp)
{}

//	FUNCTION public
//	Trans::Log::Data::~Data --
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
Data::~Data()
{}

//	FUNCTION public
//	Trans::Log::Data::getClassID -- このクラスのクラス ID を得る
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
Data::getClassID() const
{
	return Trans::Externalizable::Category::LogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::Data::getCategory -- 種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた種別
//
//	EXCEPTIONS
//		なし

inline
Data::Category::Value
Data::getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Trans::Log::Data::getVersionNumber -- バージョン番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたバージョン番号
//
//	EXCEPTIONS
//		なし

inline
Data::VersionNumber::Value
Data::getVersionNumber() const
{
	return _version;
}

//	FUNCTION public
//	Trans::Log::Data::getTimeStamp --
//		記録する直前に取得したタイムスタンプを得る
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
const TimeStamp&
Data::getTimeStamp() const
{
	return _timeStamp;
}

//	FUNCTION public
//	Trans::Log::Data::clearTimeStamp --
//		タイムスタンプをクリアする
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
void
Data::clearTimeStamp()
{
	_timeStamp = IllegalTimeStamp;
}

//	FUNCTION public
//	Trans::Log::TransactionBeginData::TransactionBeginData --
//		トランザクションの開始の論理ログデータを表すクラスのコンストラクター
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
TransactionBeginData::TransactionBeginData()
	: Data(Category::TransactionBegin)
{}

//	FUNCTION public
//	Trans::Log::TransactionBeginData::~TransactionBeginData --
//		トランザクションの開始の論理ログデータを表すクラスのデストラクター
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
TransactionBeginData::~TransactionBeginData()
{}

//	FUNCTION public
//	Trans::Log::TransactionBeginData::getClassID --
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
TransactionBeginData::getClassID() const
{
	return Trans::Externalizable::Category::TransactionBeginLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::InsideTransactionData::InsideTransactionData --
//		トランザクション中であるときのみ記録可能な
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
InsideTransactionData::InsideTransactionData(Category::Value category)
	: Data(category),
	  _beginTransactionLSN(IllegalLSN),
	  _endStatementLSN(IllegalLSN),
	  _backwardLSN(IllegalLSN)
{}

//	FUNCTION public
//	Trans::Log::InsideTransactionData::~InsideTransactionData --
//		トランザクション中であるときのみ記録可能な
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
InsideTransactionData::~InsideTransactionData()
{}

//	FUNCTION public
//	Trans::Log::InsideTransactionData::getClassID --
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
InsideTransactionData::getClassID() const
{
	return Trans::Externalizable::Category::InsideTransactionLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::InsideTransactionData::getBeginTransactionLSN --
//		トランザクションの開始を表す論理ログのログシーケンス番号を得る
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
//		なし

inline
LSN
InsideTransactionData::getBeginTransactionLSN() const
{
	return _beginTransactionLSN;
}

//	FUNCTION public
//	Trans::Log::InsideTransactionData::getEndStatementLSN --
//		トランザクションが直前に実行した SQL 文の終了を表す
//		論理ログのログシーケンス番号を得る
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
//		なし

inline
LSN
InsideTransactionData::getEndStatementLSN() const
{
	return _endStatementLSN;
}

//	FUNCTION public
//	Trans::Log::InsideTransactionData::getBackwardLSN --
//		トランザクションが直前に挿入した論理ログのログシーケンス番号を得る
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
//		なし

inline
LSN
InsideTransactionData::getBackwardLSN() const
{
	return _backwardLSN;
}

//	FUNCTION public
//	Trans::Log::ModificationData::ModificationData --
//		トランザクション中で実行される更新操作の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//		bool				undoable
//			true または指定されないとき
//				UNDO 可能な操作を表す論理ログデータである
//			false
//				REDO しかできない操作を表す論理ログデータである
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
ModificationData::ModificationData(Category::Value category, bool undoable)
	: InsideTransactionData(category),
	  _undoable(undoable)
{}

//	FUNCTION public
//	Trans::Log::ModificationData::~ModificationData --
//		トランザクション中で実行される更新操作の
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
ModificationData::~ModificationData()
{}

//	FUNCTION public
//	Trans::Log::ModificationData::getClassID --
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
ModificationData::getClassID() const
{
	return Trans::Externalizable::Category::ModificationLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::ModificationData::isUndoable -- UNDO 可能な更新操作を表すか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			UNDO 可能な更新操作を表す
//		false
//			UNDO 不可な更新操作を表す
//
//	EXCEPTIONS
//		なし

inline
bool
ModificationData::isUndoable() const
{
	return _undoable;
}

//	FUNCTION public
//	Trans::Log::TransactionCommitData::TransactionCommitData --
//		トランザクションのコミットの
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
TransactionCommitData::TransactionCommitData()
	: InsideTransactionData(Category::TransactionCommit)
{}

//	FUNCTION public
//	Trans::Log::TransactionCommitData::~TransactionCommitData --
//		トランザクションのコミットの
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
TransactionCommitData::~TransactionCommitData()
{}

//	FUNCTION public
//	Trans::Log::TransactionCommitData::getClassID --
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
TransactionCommitData::getClassID() const
{
	return Trans::Externalizable::Category::TransactionCommitLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::TransactionRollbackData::TransactionRollbackData --
//		トランザクションのロールバックの
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
TransactionRollbackData::TransactionRollbackData()
	: InsideTransactionData(Category::TransactionRollback)
{}

//	FUNCTION public
//	Trans::Log::TransactionRollbackData::~TransactionRollbackData --
//		トランザクションのロールバックの
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
TransactionRollbackData::~TransactionRollbackData()
{}

//	FUNCTION public
//	Trans::Log::TransactionRollbackData::getClassID --
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
TransactionRollbackData::getClassID() const
{
	return Trans::Externalizable::Category::TransactionRollbackLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::StatementCommitData::StatementCommitData --
//		SQL 文のコミットの論理ログデータを表すクラスのコンストラクター
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
StatementCommitData::StatementCommitData()
	: InsideTransactionData(Category::StatementCommit)
{}

//	FUNCTION public
//	Trans::Log::StatementCommitData::~StatementCommitData --
//		SQL 文のコミットの論理ログデータを表すクラスのデストラクター
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
StatementCommitData::~StatementCommitData()
{}

//	FUNCTION public
//	Trans::Log::StatementCommitData::getClassID --
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
StatementCommitData::getClassID() const
{
	return Trans::Externalizable::Category::StatementCommitLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::StatementRollbackData::StatementRollbackData --
//		SQL 文のロールバックの論理ログデータを表すクラスのコンストラクター
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
StatementRollbackData::StatementRollbackData()
	: InsideTransactionData(Category::StatementRollback)
{}

//	FUNCTION public
//	Trans::Log::StatementRollbackData::~StatementRollbackData --
//		SQL 文のロールバックの論理ログデータを表すクラスのデストラクター
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
StatementRollbackData::~StatementRollbackData()
{}

//	FUNCTION public
//	Trans::Log::StatementRollbackData::getClassID --
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
StatementRollbackData::getClassID() const
{
	return Trans::Externalizable::Category::StatementRollbackLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::TimeStampAssignData::TimeStampAssignData --
//		新たなタイムスタンプ値の上位 32 ビットの生成の
//		論理ログデータを表すクラスのデフォルトコンストラクター
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
TimeStampAssignData::TimeStampAssignData()
	: Data(Category::TimeStampAssign)
{}

//	FUNCTION public
//	Trans::Log::TimeStampAssignData::TimeStampAssignData --
//		新たなタイムスタンプ値の上位 32 ビットの生成の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::TimeStamp&	assigned
//			生成されたタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TimeStampAssignData::TimeStampAssignData(const TimeStamp& assigned)
	: Data(Category::TimeStampAssign, assigned)
{}

//	FUNCTION public
//	Trans::Log::TimeStampAssignData::~TimeStampAssignData --
//		新たなタイムスタンプ値の上位 32 ビットの生成の
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
TimeStampAssignData::~TimeStampAssignData()
{}

//	FUNCTION public
//	Trans::Log::TimeStampAssignData::getClassID -- このクラスのクラス ID を得る
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
TimeStampAssignData::getClassID() const
{
	return Trans::Externalizable::Category::TimeStampAssignLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::TimeStampAssignData::getAssigned --
//		生成されたタイムスタンプを得る
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
const TimeStamp&
TimeStampAssignData::getAssigned() const
{
	return getTimeStamp();
}

//	FUNCTION public
//	Trans::Log::TransactionPrepareData::TransactionPrepareData --
//		トランザクションのコミット準備完了の
//		論理ログデータを表すクラスのデフォルトコンストラクター
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
TransactionPrepareData::TransactionPrepareData()
	: InsideTransactionData(Category::TransactionPrepare)
{}

//	FUNCTION public
//	Trans::Log::TransactionPrepareData::TransactionPrepareData --
//		トランザクションのコミット準備完了の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID			xid
//			コミット準備完了する
//			トランザクションブランチのトランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
TransactionPrepareData::TransactionPrepareData(const XID& xid)
	: InsideTransactionData(Category::TransactionPrepare),
	  _xid(xid)
{}

//	FUNCTION public
//	Trans::Log::TransactionPrepareData::~TransactionPrepareData --
//		トランザクションのコミット準備完了の
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
TransactionPrepareData::~TransactionPrepareData()
{}

//	FUNCTION public
//	Trans::Log::TransactionPrepareData::getClassID --
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
TransactionPrepareData::getClassID() const
{
	return Trans::Externalizable::Category::TransactionPrepareLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::TransactionPrepareData::getXID --
//		コミット準備完了する
//		トランザクションブランチのトランザクションブランチ識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションブランチ識別子
//
//	EXCEPTIONS
//		なし

inline
const XID&
TransactionPrepareData::getXID() const
{
	return _xid;
}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::BranchHeurDecideData --
//		トランザクションブランチのヒューリスティックな解決の
//		論理ログデータを表すクラスのデフォルトコンストラクター
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
BranchHeurDecideData::BranchHeurDecideData()
	: Data(Category::BranchHeurDecide)
{}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::BranchHeurDecideData --
//		トランザクションブランチのヒューリスティックな解決の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID			xid
//			ヒューリスティックに解決する
//			トランザクションブランチのトランザクションブランチ識別子
//		Trans::Branch::HeurDecision::Value	decision
//			どうやってヒューリスティックに解決するかを表す値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
BranchHeurDecideData::BranchHeurDecideData(
	const XID& xid, Branch::HeurDecision::Value decision)
	: Data(Category::BranchHeurDecide),
	  _xid(xid),
	  _decision(decision)
{}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::~BranchHeurDecideData --
//		トランザクションブランチのヒューリスティックな解決の
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
BranchHeurDecideData::~BranchHeurDecideData()
{}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::getClassID --
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
BranchHeurDecideData::getClassID() const
{
	return Trans::Externalizable::Category::BranchHeurDecideLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::getXID --
//		ヒューリスティックに解決する
//		トランザクションブランチのトランザクションブランチ識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションブランチ識別子
//
//	EXCEPTIONS
//		なし

inline
const XID&
BranchHeurDecideData::getXID() const
{
	return _xid;
}

//	FUNCTION public
//	Trans::Log::BranchHeurDecideData::getDecision --
//		どうやってヒューリスティックに解決するかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたどうやってヒューリスティックに解決するかを表す値
//
//	EXCEPTIONS
//		なし

inline
Branch::HeurDecision::Value
BranchHeurDecideData::getDecision() const
{
	return _decision;
}

//	FUNCTION public
//	Trans::Log::BranchForgetData::BranchForgetData --
//		ヒューリスティックな解決済のトランザクションブランチの抹消の
//		論理ログデータを表すクラスのデフォルトコンストラクター
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
BranchForgetData::BranchForgetData()
	: Data(Category::BranchForget)
{}

//	FUNCTION public
//	Trans::Log::BranchForgetData::BranchForgetData --
//		ヒューリスティックな解決済のトランザクションブランチの抹消の
//		論理ログデータを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID			xid
//			抹消したトランザクションブランチのトランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
BranchForgetData::BranchForgetData(const XID& xid)
	: Data(Category::BranchForget),
	  _xid(xid)
{}

//	FUNCTION public
//	Trans::Log::BranchForgetData::~BranchForgetData --
//		ヒューリスティックな解決済のトランザクションブランチの抹消の
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
BranchForgetData::~BranchForgetData()
{}

//	FUNCTION public
//	Trans::Log::BranchHeurForgetData::getClassID --
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
BranchForgetData::getClassID() const
{
	return Trans::Externalizable::Category::BranchForgetLogData +
		Common::Externalizable::TransClasses;
}

//	FUNCTION public
//	Trans::Log::BranchForgetData::getXID --
//		抹消したトランザクションブランチのトランザクションブランチ識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクションブランチ識別子
//
//	EXCEPTIONS
//		なし

inline
const XID&
BranchForgetData::getXID() const
{
	return _xid;
}

//	FUNCTION public
//	Trans::Log::StartBatchData::StartBatchData --
//		トランザクションのコミットの
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
StartBatchData::StartBatchData()
	: InsideTransactionData(Category::StartBatch)
{}

//	FUNCTION public
//	Trans::Log::StartBatchData::~StartBatchData --
//		トランザクションのコミットの
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
StartBatchData::~StartBatchData()
{}

//	FUNCTION public
//	Trans::Log::StartBatchData::getClassID --
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
StartBatchData::getClassID() const
{
	return Trans::Externalizable::Category::StartBatchLogData +
		Common::Externalizable::TransClasses;
}

_SYDNEY_TRANS_LOG_END
_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_LOGDATA_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
