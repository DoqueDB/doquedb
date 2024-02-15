// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.h -- テストを実行するクラスのヘッダファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2011, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_EXECUTOR_H
#define __SYDNEY_SYDTEST_EXECUTOR_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "Communication/CryptMode.h"
#include "Client2/DataSource.h"
#include "Client2/Session.h"
#include "Client2/ResultSet.h"
#include "SydTest/Map.h"
#include "SydTest/Monitor.h"
#include "SydTest/Thread.h"
#include "SydTest/Item.h"
#include "SydTest/CascadeConf.h"
#include "SydTest/StopWatch.h"
#include "ModPair.h"
#include "ModTime.h"
#include "ModParameter.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace 
{
	class Rowset;
}

namespace SydTest
{
	class Command;
	class Thread;
	class Parser;
	class String;
	class Number;
	class Parameter;

struct ExecuteOption
{
	enum Value
	{
		// 何もしない
		Nothing           = 0x00,
		// スクリプト中のコマンドを表示する
		ShowCommand       = 0x01, 
		// スクリプト中のパラメータを表示する
		ShowParameter     = 0x02,
		// Pause文を無視する
		IgnorePause       = 0x04, 
		// デバッグモード(スクリプトの解析のみ行い、実行はしない)
		DebugMode	      = 0x8,
		// セッション番号を表示する
		ShowSessionNumber = 0x10,
		// (Not)Existsに失敗したときに実行を終える
		AbortWhenMissingPath = 0x20,
		// Initializeに失敗したときに実行を終える
		AbortWhenInitFail = 0x40,
		// Replace BackSlash to PathDelimiter.
		ReplaceBackSlash  = 0x80,
		// Explain all the statements
		Explain			  = 0x100,
		// Prepared for all commands
		Prepared		  = 0x200
		// マスク
//		Mask		      = 0x7ff
	};
};

//
//	CLASS
//	SydTest::Executor -- テストを実行する
//
//	NOTES
//
//
//##ModelId=3A9B474701BE
class Executor 
{
public:
	//コンストラクタ
	//##ModelId=3A9B47470210
	Executor(const char* pszFileName_, 
		     const char* pszLabel_, int iOption_, 
			 Monitor& rMonitor_,
			 const ModUnicodeString& cstrHostName_, int iPortNumber_,
			 const ModUnicodeString& cstrRegPath_,
			 const ModVector<CascadeInfo>& cascadeInfoVector_,
			 const Communication::CryptMode::Value cryptMode_ = Communication::CryptMode::Unknown,
			 const ModUnicodeString& cstrUserName_ = ModUnicodeString(),
			 const ModUnicodeString& cstrPassword_ = ModUnicodeString());
	//デストラクタ
	//##ModelId=3A9B4747020F
	virtual ~Executor();

	//時計を止める→getNextCommand→時計を再開
	Item* getNextCommandWithoutTiming(Parser& rParser);

	//一行分のコマンドを実行
	//##ModelId=3A9B47470208
	void executeSydneyCommand(Command* pCommand_);
	// メイン部分のコマンド群実行
	//##ModelId=3A9B47470206
	void executeMain();
	// スレッド部分のコマンド群実行
	//##ModelId=3A9B474701FE
	void executeThread(const char* pszLabel_);

    //初期化済みか
    bool isInitialized() {
        return m_bSydneyInitializedFlg;
	}

	// setEncodingTypeの下請(main()にも見せる)
	static void setEncodingType_sub(const char* str);

private:
	// Sydneyの初期化
	//##ModelId=3A9B474701FD
	void initialize(Item* pItem_);
	// Sydneyの終了
	//##ModelId=3A9B474701FC
	void terminate(Item* pItem_);
	// セッションの初期化
	//##ModelId=3A9B474701FA
	void initializeSession(Item* pItem_);
	// セッションの終了
	//##ModelId=3A9B474701F3
	void terminateSession(Item* pItem_);
	// 現在時刻の表示
	//##ModelId=3A9B474701F2
	void displayCurrentTime();
	// 時間測定の起点設定
	//##ModelId=3A9B474701F1
	void beginTimespan(Item* pItem_);
	// 時間測定の終点設定
	//##ModelId=3A9B474701F0
	void endTimespan(Item* pItem_);
	//##ModelId=3A9B474701E8

	// 子サーバの操作
	// 子サーバの追加
	void addCascade(Item* pItem_);
	// 子サーバの変更
	void alterCascade(Item* pItem_);
	// 子サーバの削除
	void dropCascade(Item* pItem_);
	// 子サーバの起動
	void startCascade(Item* pItem_);
	// 子サーバの正常終了
	void terminateCascade(Item* pItem_);
	// 子サーバの強制終了
	void forceTerminateCascade(Item* pItem_);
	
	// 同期的なSQLコマンドを発行する
	void executeSQLCommand(Item* pItem_);

	// 非同期なSQLコマンドを発行する
	void executeAsyncSQLCommand(Item* pItem_);
	// 非同期なSQLコマンドの結果を得る
	void getAsyncSQLResult(Item* pItem_);
	// 非同期なSQLコマンドの実行を中断する
	void cancelAsyncSQLCommand(Item* pItem_);

	// 最適化済コマンドを作成する
	void prepareSQLCommand(Item* pItem_);
	// 最適化済コマンドから同期的なSQLコマンドを発行する
	void executePreparedSQLCommand(Item* pItem_);
	// 最適化済コマンドから非同期的なSQLコマンドを発行する
	void executeAsyncPreparedSQLCommand(Item* pItem_);
	// 最適化済コマンドを抹消する
	void erasePreparedSQLCommand(Item* pItem_);

	//
	Client2::ResultSet* createSQLCommand(int iNumber_,
		const ModUnicodeString& rstrCommand_, 
		Common::DataArrayData* pParameter_);
	//
	Client2::PrepareStatement* createPrepareSQLCommand(int iNumber_,
													   const ModUnicodeString& rstrCommand_);
	//
	Client2::ResultSet* createPreparedSQLCommand(int iNumber_,
		const Client2::PrepareStatement& rPrep_,
		Common::DataArrayData* pParameter_);

	void getSQLResult(int iNumber_, Client2::ResultSet* pSQLCommand_,
					  bool bSkipResult = false);

	// 下請け
	Common::DataArrayData* getSQLParameter(Item* pString_);
	void showParameter(Common::DataArrayData* pParameter_, int iNumber_);

	// 引数ミリ秒だけsleepする
	void sleep(Item* pItem_);
	// 引数ミリ秒*レジストリで指定した割合だけsleepする
	void scaledSleep(Item* pItem_);

	// 直前の実行結果を確認する
	void assureCount(Command* pCommand_);

	// Encodingを切り替える
	void setEncodingType(Command* pCommand_);

	// Threadの生成
	// 実行はThreadクラスに任せる
	//##ModelId=3A9B474701DF
	void createThread(Command* pCommand_);
	// Thread終了を待つ
	//##ModelId=3A9B474701DD
	void joinThread(Command* pCommand_);
	// 他のファイルの包摂を行う
	void fileInclude(Command* pCommand_);
	// パスの存在を確認する
	void exists(Command* pCommand_);
	// パスの不在を確認する
	void notExists(Command* pCommand_);
	// OSのコマンドを実行する
	void doOsCommand(Command* pCommand_);
	// レジストリの変更
	void setSystemParameter(Command* pCommand_);

	// 条件変数を用いた待ち合わせをする
	void synchronize(Command* pCommand_);

	// isServerAvailable
	void isServerAvailable(Command* pCommand_);
	// isDatabaseAvailable
	void isDatabaseAvailable(Command* pCommand_);

	// user management
	void createUser(Command* pCommand_);
	void dropUser(Command* pCommand_);
	void changePassword(Command* pCommand_);
	void changeOwnPassword(Command* pCommand_);

	// ItemがStringか否かを検査
	String* testStringPointer(Item* pItem_, const ModUnicodeString& cstrErrorMessage_);
	// ItemがNumberか否かを検査
	Number* testNumberPointer(Item* pItem_, const ModUnicodeString& cstrErrorMessage_);
	// ItemがParameterか否かを検査
	Parameter* testParameterPointer(Item* pItem_, const ModUnicodeString& cstrErrorMessage_);

	// セッション番号に対応する文字列を得る
	ModCharString getSessionNumberString(int iNumber_);

	// start explain
	void startExplain(int iNumber_, Client2::Session* pClient_);
	// end explain
	void endExplain(int iNumber_, Client2::Session* pClient_);

	// クライアントID→クライアントのmap
	static Map<int, Client2::DataSource*> m_mapDataSource;
	// セッションID→セッションのmap
	static Map<int, Client2::Session*> m_mapClient;
	// セッションID→DB名のmap
	static Map<int, ModUnicodeString> m_mapDBName;

	// 非同期コマンドとそのセッション番号のペアのmap
	struct AsyncCommandPair
		: public ModPair<int, Client2::ResultSet*>
	{
		bool m_bSkipResult;
		Client2::PrepareStatement* m_pPrepareStatement;

		AsyncCommandPair()
			: ModPair<int, Client2::ResultSet*>(),
			  m_bSkipResult(false),
			  m_pPrepareStatement(0)
		{}
		AsyncCommandPair(int id_, Client2::ResultSet* pResultSet_,
						 bool bSkipResult_,
						 Client2::PrepareStatement* pPrepareStatement_ = 0)
			: ModPair<int, Client2::ResultSet*>(id_, pResultSet_),
			  m_bSkipResult(bSkipResult_),
			  m_pPrepareStatement(pPrepareStatement_)
		{}
	};
	static Map<ModUnicodeString, AsyncCommandPair> m_mapAsyncCommand;

	// 最適化済コマンドとそのセッション番号のペアのmap
	struct PreparedCommandPair
		: public ModPair<int, Client2::PrepareStatement*>
	{
		bool m_bSkipResult;

		PreparedCommandPair()
			: ModPair<int, Client2::PrepareStatement*>(),
			  m_bSkipResult(false)
		{}
		PreparedCommandPair(int id_, Client2::PrepareStatement* pStatement_,
							bool bSkipResult_)
			: ModPair<int, Client2::PrepareStatement*>(id_, pStatement_),
			  m_bSkipResult(bSkipResult_)
		{}
	};
	static Map<ModUnicodeString, PreparedCommandPair> m_mapPreparedCommand;

	// <残りスレッド数, 条件変数>のmap
	typedef ModPair<int, ModConditionVariable*> SyncPair;
	static Map<ModUnicodeString, SyncPair> m_mapConditionVariable;

	// 時刻管理クラス
	static StopWatch m_stopWatch;

	// 排他制御用変数
	static ModCriticalSection m_cCSForParameter;
	static ModCriticalSection m_cCSForFile;
	static ModCriticalSection m_cCSForClient;
	static Map<int, ModCriticalSection*> Executor::m_mapCSForClient;

	// モニタへの参照
	Monitor&	m_rMonitor;

	// '{ }'の内部にいるかどうかのフラグ
	//##ModelId=3A9B474701DC
	bool m_bExecuteFlag;

	// 関連するスレッドの名前
	const char* m_strLabel;
	// 実行する内容記述ファイル名
	//##ModelId=3A9B474701C8
	ModUnicodeString m_strFileName;
	// 実行時オプション
	// 型がExecuteOption::Valueでないことに注意
	int m_iOption; 

	//リモートホスト名
	ModUnicodeString m_cstrHostName;
	//ポート番号
	int m_iPortNumber;

	// User name
	ModUnicodeString m_cstrUserName;
	// Password
	ModUnicodeString m_cstrPassword;

	//レジストリの親パス
	ModUnicodeString m_cstrRegPath;
	
	//子サーバ情報のベクター
	ModVector<CascadeInfo> m_cascadeInfoVector;

	//ScalableSleepの係数
	int m_iSleepingRate;

	// 暗号モード
	Communication::CryptMode::Value m_cryptMode;

	// Replacement of Backslash in the string
	bool m_bReplaceBackSlash;

	// Sydneyをinitialize済みかどうかのフラグ
	bool m_bSydneyInitializedFlg;
};
}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_EXECUTOR_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2011, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
