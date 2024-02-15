// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogInfo.h --	論理ログファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2009, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_TRANS_LOGINFO_H
#define	__SYDNEY_TRANS_LOGINFO_H

#include "Trans/Module.h"
#include "Trans/AutoLogFile.h"
#include "Trans/TimeStamp.h"

#include "Common/Object.h"
#include "Schema/ObjectID.h"

_SYDNEY_BEGIN

namespace Admin
{
	class Database;
}
namespace Lock
{
	class Name;
}
namespace Schema
{
	class Database;
}

_SYDNEY_TRANS_BEGIN

class Transaction;

_SYDNEY_TRANS_LOG_BEGIN

//	CLASS
//	Trans::Log::Info --
//		あるトランザクションが操作する論理ログファイルの
//		トランザクションごとの情報を記憶するクラス
//
//	NOTES

class Info
	: public	Common::Object
{
	//【注意】	SUN C++ 5.3 では Trans:: で修飾しないとエラーになる
	friend class Trans::Transaction;
	friend class Admin::Database;
public:
	// デフォルトコンストラクター
	SYD_TRANS_FUNCTION
	Info();
	// コンストラクター
	SYD_TRANS_FUNCTION
	Info(Schema::Database& database,
		 LSN beginTransactionLSN = IllegalLSN,
		 LSN endStatementLSN = IllegalLSN,
		 LSN lastTransactionLSN = IllegalLSN);
	// コピーコンストラクター
	SYD_TRANS_FUNCTION
	Info(const Info& src);
	// デストラクター
	~Info();

	// = 演算子
	SYD_TRANS_FUNCTION
	Info&					operator =(const Info& src);

	// == 演算子
	bool					operator ==(const Info& r) const;
	// != 演算子
	bool					operator !=(const Info& r) const;

	// 論理ログを取り出す
	SYD_TRANS_FUNCTION
	Data*					load(LSN lsn) const;
#ifdef OBSOLETE
	// 先頭の論理ログの LSN を得る
	SYD_TRANS_FUNCTION
	LSN						getFirstLSN() const;
	// 末尾の論理ログの LSN を得る
	SYD_TRANS_FUNCTION
	LSN						getLastLSN() const;
	// ある論理ログの直後のものの LSN を得る
	SYD_TRANS_FUNCTION
	LSN						getNextLSN(LSN lsn) const;
	// ある論理ログの直前のものの LSN を得る
	SYD_TRANS_FUNCTION
	LSN						getPrevLSN(LSN lsn) const;
#endif

	// 論理ログファイルに記録する操作の対象である
	// データベースのオブジェクト識別子を得る
	SYD_TRANS_FUNCTION
	Schema::ObjectID::Value
	getDatabaseID() const;
	// 論理ログファイルに記録する操作の対象であるデータベースの名前を得る
	SYD_TRANS_FUNCTION
	ModUnicodeString
	getDatabaseName() const;
	// 操作する論理ログファイルの情報を表すクラスを得る
	SYD_TRANS_FUNCTION
	AutoFile
	getFile() const;
	// 論理ログファイルを表すロック名を得る
	SYD_TRANS_FUNCTION
	Lock::LogicalLogName
	getLockName() const;

	// トランザクションの開始を表す論理ログの LSN を得る
	LSN						getBeginTransactionLSN() const;
	// トランザクションで最後に実行した SQL 文の終了を表す論理ログの LSN を得る
	LSN						getEndStatementLSN() const;
	// トランザクションが最後に記録した論理ログの LSN を得る
	LSN						getLastTransactionLSN() const;

	// 同期処理が完了していることを設定する
	SYD_TRANS_FUNCTION
	void					setSyncDone();

	// コミット時に不要な論理ログを削除することを設定する
	SYD_TRANS_FUNCTION
	void					discardLog(bool bDiscardFull);

	// マスターサーバのトランザクションの開始を表す論理ログの LSN を設定する
	void					setMasterBeginTransactionLSN(LSN lsn_) const
		{ _masterBeginTransactionLSN = lsn_; }

private:
	// 論理ログを削除する方法
	struct DiscardLog {
		enum Value {
			None = 0,		// 削除しない
			
			Full,			// 最新以外すべて削除
			Checkpoint,		// 前々回のチェックポイント以前のものを削除

			ValueNum
		};
	};
	
	// 論理ログファイルを生成する
	SYD_TRANS_FUNCTION
	void					create();
	// 論理ログファイルを破棄する
	SYD_TRANS_FUNCTION
	void					destroy();
	// 論理ログファイルのマウントを指示する
	SYD_TRANS_FUNCTION
	void					mount();
	// 論理ログファイルのアンマウントを指示する
	SYD_TRANS_FUNCTION
	void					unmount();
	// 論理ログファイルの実体である OS ファイルの絶対パス名を変更する
	SYD_TRANS_FUNCTION
	void					rename(const Os::Path& path);
	// 論理ログファイルをフラッシュする
	SYD_TRANS_FUNCTION
	void					flush(LSN lsn);

	// 論理ログを記録する
	LSN						store(const Data& data,
								  Log::LSN masterLSN = Log::IllegalLSN);

	// 論理ログからトランザクションの状態を復元する
	void					restore(const Data& data, LSN lsn);

	// 操作する論理ログファイル
	AutoFile				_logFile;
	// トランザクションの開始を表す論理ログの LSN
	LSN						_beginTransactionLSN;
	// トランザクションで最後に実行した SQL 文の終了を表す論理ログの LSN
	LSN						_endStatementLSN;
	// トランザクション記述子を介して最後に挿入した論理ログの LSN
	LSN						_lastTransactionLSN;
	// コミットするまでアンマウントを遅らせるか
	AutoFile				_unmountDelayed;
	// UNMOUNT 文を実行するトランザクションか
	bool					_unmounting;
	// UNMOUNT 文を表す論理ログのタイムスタンプ値
	TimeStamp				_unmountTimeStamp;

	// 論理ログを削除するかどうか
	DiscardLog::Value		_discardLogicalLog;

	// マスターサーバのトランザクションの開始を表す論理ログのLSN
	mutable LSN				_masterBeginTransactionLSN;
};

//	FUNCTION public
//	Trans::Log::Info::~Info --
//		あるトランザクションが操作する論理ログファイルの
//		トランザクションごとの情報を記憶するクラスのデストラクター
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

inline
Info::~Info()
{}

//	FUNCTION public
//	Trans::Log::Info::operator == -- == 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Info&	r
//			自分自身と比較する操作する論理ログファイルの情報を表すクラス
//
//	RETURN
//		true
//			自分自身と等しい
//		false
//			自分自身と等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
Info::operator ==(const Info& r) const
{
	return getDatabaseID() == r.getDatabaseID();
}

//	FUNCTION public
//	Trans::Log::Info::operator != -- != 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Log::Info&	r
//			自分自身と比較する操作する論理ログファイルの情報を表すクラス
//
//	RETURN
//		true
//			自分自身と等しくない
//		false
//			自分自身と等しい
//
//	EXCEPTIONS
//		なし

inline
bool
Info::operator !=(const Info& r) const
{
	return getDatabaseID() != r.getDatabaseID();
}

//	FUNCTION public
//	Trans::Log::Info::getBeginTransactionLSN --
//		トランザクションの開始を表す論理ログの LSN を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた LSN
//
//	EXCEPTIONS
//		なし

inline
LSN
Info::getBeginTransactionLSN() const
{
	return _beginTransactionLSN;
}

//	FUNCTION public
//	Trans::Log::Info::getEndStatementLSN --
//		トランザクションで最後に実行した SQL 文の終了を表す
//		論理ログの LSN を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた LSN
//
//	EXCEPTIONS
//		なし

inline
LSN
Info::getEndStatementLSN() const
{
	return _endStatementLSN;
}

//	FUNCTION public
//	Trans::Log::Info::getLastTransactionLSN --
//		トランザクションが最後に記録した論理ログの LSN を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた LSN
//
//	EXCEPTIONS
//		なし

inline
LSN
Info::getLastTransactionLSN() const
{
	return _lastTransactionLSN;
}

_SYDNEY_TRANS_LOG_END
_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_LOGINFO_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2009, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
