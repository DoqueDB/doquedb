// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Request.h --
// 
// Copyright (c) 2002, 2005, 2007, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_REQUEST_H
#define __TRMEISTER_COMMON_REQUEST_H

#include "Common/Object.h"
#include "Common/Externalizable.h"
#include "ModUnicodeString.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::Request -- リクエストをあらわすクラス
//
//	NOTES
//	リクエストをあらわすクラス
//
class SYD_COMMON_FUNCTION Request : public Common::Object,
							 public Common::Externalizable
{
public:
	//
	//	ENUM
	//	Session::Request::XXX -- リクエスト番号
	//
	//	NOTES
	//	クライアントからセッションに渡されるリクエスト番号
	//
	enum
	{
		BeginConnection = 1,	//コネクション開始
		EndConnection,			//コネクションの終了
		BeginSession,			//セッション開始
		EndSession,				//セッション終了
		BeginWorker,			//Workerの開始
		CancelWorker,			//Workerの中断

		Shutdown,				//サーバの終了

		ExecuteStatement,		//SQL文実行
		PrepareStatement,		//SQL文のコンパイル
		ExecutePrepare,			//コンパイル結果の実行
		ErasePrepareStatement,	//最適化結果を削除する

		ReuseConnection,		//コネクションを再利用する
		NoReuseConnection,		//コネクションを再利用しない

		CheckAvailability,		// 利用可能性チェック

		PrepareStatement2,		//SQL文のコンパイル
		ErasePrepareStatement2,	//最適化結果を削除する

		BeginSession2,			// Starting session with user management
		EndSession2,			// Ending session with user management

		CreateUser,				// Creating new user
		DropUser,				// Deleting a user
		ChangeOwnPassword,		// Changing own password
		ChangePassword,			// Changing others password

		Shutdown2,				//サーバの終了

		CheckReplication,		// レプリケーションを確認する
		TransferLogicalLog,		// スレーブサーバへ論理ログを転送する
		StopTransferLogicalLog,	// スレーブサーバへの転送を停止する
		StartReplication,		// レプリケーションを開始する

		Sync = 101,				//sync

		QueryProductVersion = 201, // Investigating product version(JDBC)

		Undefined = -1
	};

	// 利用可能性のチェック対象
	struct AvailabilityTarget
	{
		enum Value
		{
			Server = 0,
			Database
		};
	};

	//コンストラクタ(1)
	Request();
	//コンストラクタ(2)
	explicit Request(int iCommand_);
	//デストラクタ
	virtual ~Request();

	//コマンドを設定する
	void setCommand(int iCommand_);
	//コマンドを取り出す
	int getCommand() const;

	//シリアル化
	void serialize(ModArchive& cArchiver_);
	//クラスIDを得る
	int getClassID() const;

	//文字列で取り出す(for Debug)
	ModUnicodeString toString() const;

private:
	//リクエスト番号
	int m_iCommand;
};

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_REQUEST_H

//
//	Copyright (c) 2002, 2005, 2007, 2009, 2014, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
