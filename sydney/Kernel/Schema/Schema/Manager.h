// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- マネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_MANAGER_H
#define	__SYDNEY_SCHEMA_MANAGER_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Object.h"

#include "Lock/Mode.h"
#include "Lock/Duration.h"

#include "Server/SessionID.h"

#include "Trans/AutoLogFile.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Admin
{
	namespace Recovery
	{
		class Database;
	}
}
namespace Communication
{
	class Connection;
}
namespace Os
{
	class CriticalSection;
	class Path;
}
namespace Server
{
	class Session;
}
namespace Statement
{
	class Object;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Area;
class AreaContent;
class Database;
class LogData;
class ObjectSnapshot;
class Sequence;

namespace Manager
{
	SYD_SCHEMA_FUNCTION
	void					initialize();		// マネージャーを初期化する
	SYD_SCHEMA_FUNCTION
	void					terminate();		// マネージャーを終了する

	SYD_SCHEMA_FUNCTION
	void					install(Trans::Transaction& cTrans_);
												// マネージャーを
												// インストールする
#ifdef OBSOLETE // uninstallは使用されない
	SYD_SCHEMA_FUNCTION
	void					uninstall(Trans::Transaction& cTrans_);
												// マネージャーを
												// アンインストールする
#endif

	SYD_SCHEMA_FUNCTION
	void					recover(Trans::Transaction& cTransaction_,
						 			const Trans::TimeStamp& cPoint_);
												// メタデータベースを
												// 障害から回復する
	SYD_SCHEMA_FUNCTION
	void					restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする
	SYD_SCHEMA_FUNCTION
	void					sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
												// 不要な版を破棄する

	SYD_SCHEMA_FUNCTION
	void					checkCanceled(Trans::Transaction& cTrans_);
												// 中断要求があるか調べる

	namespace Configuration
	{
		void				initialize();		// マネージャー設定の
												// 初期化を行う
		void				terminate();		// マネージャー設定の
												// 終了処理を行う

		const Os::Path&		getDefaultAreaPath();
												// デフォルトエリアの
												// パス名を得る
		const Os::Path&		getSystemAreaPath();
												// システム表エリアの
												// パス名を得る
		bool				isSchemaPreloaded();
												// 全データベースの
												// スキーマ情報をマネージャーの
												// 初期化時に読み込むかを得る
#ifdef OBSOLETE // 常にすぐに削除するのでこのConfigurationは使用されない
		bool				isImmediatelyDestroyed();
												// ファイルやディレクトリーの
												// 削除をすぐに実行するか
#endif
		bool				isAlwaysTemporary();
												// ファイルを常に
												// 一時ファイルとして作るか
		bool				isCanceledWhenDuplicated();
												// 存在する名前でcreateされる
												// ときに例外を投げるか、
												// 単純に処理をキャンセルするか
		bool				isNoEncodingForm();	// FileIDにEncodingFormを設定しないか
	}

	namespace ObjectTree
	{
		void				initialize();		// オブジェクト木関連の
												// 初期化を行う
		void				terminate();		// オブジェクト木関連の
												// 終了処理を行う

		void				install(Trans::Transaction& cTrans_);
												// オブジェクト木関連を
												// インストールする
#ifdef OBSOLETE // uninstallは使用されない
		void				uninstall(Trans::Transaction& cTrans_);
												// オブジェクト木関連を
												// アンインストールする
#endif

		void				recover(Trans::Transaction& cTransaction_,
						 			const Trans::TimeStamp& cPoint_);
												// データベースIDの値を
												// 生成するためのファイルを
												// 障害から回復する
		void				restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする
		void				sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
												// 不要な版を破棄する

		// データベースIDを生成するためのシーケンスに関する操作
		namespace Sequence
		{
			void			initialize();		// シーケンスを初期化する
			void			terminate();		// シーケンスの終了処理を行う
			void			install(Trans::Transaction& cTrans_);
												// シーケンスをインストールする
#ifdef OBSOLETE // uninstallは使用されない
			void			uninstall(Trans::Transaction& cTrans_);
												// シーケンスをインストールする
#endif

			void			recover(Trans::Transaction& cTransaction_,
						 			const Trans::TimeStamp& cPoint_);
												// データベースIDの値を
												// 生成するためのファイルを
												// 障害から回復する
			void			restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする
			void			sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
												// 不要な版を破棄する
			
			Schema::Sequence&
							get();				// データベース ID の値を
												// 生成するための
												// シーケンスを得る
		}

		// データベースに関する操作
		namespace Database
		{
			SYD_SCHEMA_FUNCTION
			bool			hold(Trans::Transaction& cTrans_,
								 Lock::Name::Category::Value eManipulate_,
								 Hold::Operation::Value eOperation_,
								 Object::ID::Value iDatabaseID_,
								 Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
												// 適切なロックモードで
												// データベースをロックする
			SYD_SCHEMA_FUNCTION
			void			release(Trans::Transaction& cTrans_,
									Lock::Name::Category::Value eManipulate_,
									Hold::Operation::Value eOperation_,
									Object::ID::Value iDatabaseID_);
												// 必要ならアンロックする

			// システムに登録されるデータベースを表すクラスに関する操作

			SYD_SCHEMA_FUNCTION
			const ModVector<Schema::Database*>&
							get(Trans::Transaction& cTrans_);
												// すべての登録の取得
			SYD_SCHEMA_FUNCTION
			Schema::Database*
							get(Schema::Object::ID::Value databaseID,
								Trans::Transaction& cTrans_);
			Schema::Object::ID
							getID(const Object::Name& databaseName,
								  Trans::Transaction& cTrans_);
												// ある登録の取得

			void			clearCache();		// すべてのデータベースのうちクリアが可能な
												// データベースオブジェクトのキャッシュをクリアする
			void			freazeCache(Schema::Object::ID::Value databaseID);
												// 指定されたIDを持つデータベースのキャッシュをfreezeする

			// データベースのキャッシュ数を管理する
			void			incrementCacheSize(int size = 1);
			void			decrementCacheSize(int size = 1);
			bool			checkCacheSize();

			// call revokeAll for all databases
			SYD_SCHEMA_FUNCTION
			void			revokeAll(int iUserID_);
		}

		// 表に関する操作
		namespace Table
		{
			SYD_SCHEMA_FUNCTION
			bool			hold(Trans::Transaction& cTrans_,
								 Lock::Name::Category::Value eManipulate_,
								 Hold::Operation::Value eOperation_,
								 Object::ID::Value iDatabaseID_,
								 Object::ID::Value iTableID_,
								 Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
												// 適切なロックモードで
												// 表をロックする
			SYD_SCHEMA_FUNCTION
			void			release(Trans::Transaction& cTrans_,
									Lock::Name::Category::Value eManipulate_,
									Hold::Operation::Value eOperation_,
									Object::ID::Value iDatabaseID_,
									Object::ID::Value iTableID_);
												// 必要ならアンロックする
		}
	}

	namespace ObjectName
	{
		void				initialize();
												// 初期化する
		void				terminate();
												// 後処理をする
		bool				reserve(const Schema::Object* pObj_);
												// 作成中のオブジェクトとして
												// 登録する
		void				withdraw(const Schema::Object* pObj_);
												// 作成中のオブジェクトとしての
												// 登録を解除する

		// 名称の長さの上限
		ModSize				getMaxLength();

		// 自動的にwithdrawを呼ぶためのクラス
		class SYD_SCHEMA_FUNCTION AutoWithdraw
		{
		public:
			AutoWithdraw(const Schema::Object* pObj_)
				: m_pObject(pObj_)
			{}
			// inlineにするとwithdrawも公開する必要があるのでcppで定義する
			~AutoWithdraw();

		private:
			const Schema::Object* m_pObject;
		};
	}

	namespace ObjectPath
	{
		// 初期化する
		void				initialize();
		// 後処理する
		void				terminate();

		// 使用するパスを登録する
		bool				reserve(const Schema::Object* pObject_,
									const ModUnicodeString& cstrPath_);
		bool				reserve(const Schema::Object* pObject_,
									const ModVector<ModUnicodeString>& vecPath_);

		void				withdraw(const Schema::Object* pObject_);

		// パス名の長さの上限
		ModSize				getMaxLength();

		// 自動的にwithdrawを呼ぶためのクラス
		class SYD_SCHEMA_FUNCTION AutoWithdraw
		{
		public:
			AutoWithdraw(const Schema::Object* pObj_)
				: m_pObject(pObj_)
			{}
			// inlineにするとwithdrawも公開する必要があるのでcppで定義する
			~AutoWithdraw();

		private:
			const Schema::Object* m_pObject;
		};
	}

	namespace SystemTable
	{
		//	TYPEDEF
		//	Schema::Manager::SystemTable::Timestamp --
		//
		//	NOTES

		typedef ModSize		Timestamp;

		//	ENUM
		//	Schema::Manager::SystemTable::Result::Value --
		//
		//	NOTES
		//	a flag denoting actions which should be done after commit

		namespace Result
		{
			typedef unsigned int Value;
			enum _Value
			{
				None = 0,
				NeedCheckpoint = 1,
				NeedReCache = 1 << 1,
			};
		}

		// ↓以下のメソッドの実装はReorganize.cppにある

		void				initialize();		// システム表関連の
												// 初期化を行う
		void				terminate();		// システム表関連の
												// 終了処理を行う

		Timestamp			getCurrentTimestamp();
												// 現在のタイムスタンプを得る
		SYD_SCHEMA_FUNCTION
		void				addTimestamp(Trans::Transaction& cTrans_);
												// スキーマ情報に変化があったとき
												// タイムスタンプを進める
		SYD_SCHEMA_FUNCTION
		void				reCache();
												// スキーマ情報に変化があったとき
												// コミット後にキャッシュの更新をする

		void				install(Trans::Transaction& cTrans_);
												// システム表関連を
												// インストールする
#ifdef OBSOLETE // uninstallは使用されない
		void				uninstall(Trans::Transaction& cTrans_);
												// システム表関連を
												// アンインストールする
#endif
		SYD_SCHEMA_FUNCTION
		void				recover(Trans::Transaction& cTransaction_,
						 			const Trans::TimeStamp& cPoint_);
												// データベースのシステム表を
												// 障害から回復する
		SYD_SCHEMA_FUNCTION
		void				restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする
		SYD_SCHEMA_FUNCTION
		void				sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
												// 不要な版を破棄する

		SYD_SCHEMA_FUNCTION
		bool
		hold(Trans::Transaction& cTrans_,
			 Hold::Target::Value eTarget_,
			 Lock::Name::Category::Value eManipulate_,
			 Hold::Operation::Value eOperation_,
			 Object::ID::Value iID_ = Object::ID::Invalid,
			 Trans::Log::File::Category::Value eLogCategory_ =
										Trans::Log::File::Category::System,
			 Schema::Database* pDatabase_ = 0,
			 Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
												// 適切なロックモードで
												// 指定された対象をロックする
		SYD_SCHEMA_FUNCTION
		bool				convert(Trans::Transaction& cTrans_,
									Hold::Target::Value iTargets_,
									Lock::Name::Category::Value eManipulateFrom_,
									Hold::Operation::Value eOperationForm_,
									Lock::Name::Category::Value eManipulateTo_,
									Hold::Operation::Value eOperationTo_,
									Object::ID::Value iID_ = Object::ID::Invalid,
									Trans::Log::File::Category::Value eLogCategory_
								 		= Trans::Log::File::Category::System,
									Schema::Database* pDatabase_ = 0,
									Lock::Timeout::Value iTimeout = Lock::Timeout::Unlimited);
												// 指定された対象のロックを
												// 適切なロックモードに変換する
		SYD_SCHEMA_FUNCTION
		void				release(Trans::Transaction& cTrans_,
									Hold::Target::Value eTarget_,
									Lock::Name::Category::Value eManipulate_,
									Hold::Operation::Value eOperation_,
									Object::ID::Value iID_ = Object::ID::Invalid,
									Trans::Log::File::Category::Value eLogCategory_
									 	= Trans::Log::File::Category::System,
									Schema::Database* pDatabase_ = 0);
												// 必要ならアンロックする

		// システム用の論理ログファイルの論理ログファイル記述子を得る
		SYD_SCHEMA_FUNCTION
		Trans::Log::AutoFile
		getLogFile();

		SYD_SCHEMA_FUNCTION
		Result::Value		reorganize(Trans::Transaction& cTrans_,
									   Server::Session* pSession_,
									   const Schema::Object::Name& cDatabase_,
									   const Statement::Object* pStatement_);
												// システム表の再構成を行う

		// ↑以上のメソッドの実装はReorganize.cppにある
		// 以下のメソッドの実装はRedo.cppにある

		// 再構成の再実行を行う
		SYD_SCHEMA_FUNCTION
		Admin::Recovery::Database*
							redo(Trans::Transaction& cTrans_,
								 const LogData& cLogData_,
								 const Schema::Object::Name& cDatabaseName_,
								 bool bRollforward_);

		// 以下のメソッドの実装はUndo.cppにある

		SYD_SCHEMA_FUNCTION
		void				undo(Trans::Transaction& cTrans_,
								 const Schema::LogData& cLogData_,
								 const Schema::Object::Name& cDatabaseName_,
								 bool redone, bool bRollforward_);
												// 再構成のゴミ掃除を行う
	}

	namespace ObjectSnapshot
	{
		void				initialize();		// スナップショット関連の
												// 初期化を行う
		void				terminate();		// スナップショット関連の
												// 終了処理を行う
		Schema::ObjectSnapshot*
							get(Trans::Transaction& cTrans_);
												// セッションに対応する
												// スナップショットを得る
		void				setDatabase(Trans::Transaction& cTrans_,
										Schema::Database* pDatabase_);
												// セッションが使用するデータベースを設定する
		void				erase(Server::SessionID iSessionID_);
												// セッションに対応する
												// スナップショットを消去する
		void				eraseCurrent();		// 消してもよいスナップショットをすべて消す

		void				eraseDatabase(const Schema::Object::ID& cDatabaseID_, Server::SessionID iSessionID_);
												// すべてのスナップショットからデータベースを消す
		void				eraseTable(const Schema::Object::ID& cDatabaseID_,
									   const Schema::Object::ID& cTableID_);
												// すべてのスナップショットから表を消す
		void				eraseIndex(const Schema::Object::ID& cDatabaseID_,
									   const Schema::Object::ID& cTableID_,
									   const Schema::Object::ID& cIndexID_);
												// すべてのスナップショットから索引を消す
		void				reCache();
												// スキーマ情報に変化があったとき
												// コミット後にキャッシュの更新をする
	}

	// リカバリー中に使用する Schema ID 管理ツール
	namespace Recovery
	{
		// 内部で使用する関数の宣言はRecovery.hにある

		SYD_SCHEMA_FUNCTION
		void				notifyDone();		// リカバリー処理の終了を教えてもらう

		// マウント中の回復処理の開始を通知する
		SYD_SCHEMA_FUNCTION
		void
		notifyBegin(Schema::Database& database);
		// マウント中の回復処理の終了を通知する
		SYD_SCHEMA_FUNCTION
		void
		notifyDone(Schema::Database& database);

	} // namespace Recovery

	Os::CriticalSection&	getCriticalSection();
												// 排他制御のための
												// クリティカルセクションを得る
												// (非公開関数)
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_MANAGER_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
