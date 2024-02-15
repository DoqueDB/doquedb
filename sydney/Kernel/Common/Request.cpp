// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Request.cpp -- リクエストをあわらすクラス
// 
// Copyright (c) 2002, 2007, 2009, 2010, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Common/Request.h"
#include "Common/ClassID.h"

#include "ModUnicodeString.h"

_TRMEISTER_USING

using namespace Common;

//
//	FUNCTION public
//	Common::Request::Request -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Request::Request()
: m_iCommand(Undefined)
{
}

//
//	FUNCTION public
//	Common::Request::Request -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	int iCommand_
//		リクエスト番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Request::Request(int iCommand_)
: m_iCommand(iCommand_)
{
}

//
//	FUNCTION public
//	Common::Request::~Request -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Request::~Request()
{
}

//
//	FUNCTION public
//	Common::Request::setCommand -- リクエスト番号を設定する
//
//	NOTES
//	リクエスト番号を設定する。
//
//	ARGUMENTS
//	int iCommand_
//		リクエスト番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Request::setCommand(int iCommand_)
{
	m_iCommand = iCommand_;
}

//
//	FUNCTION public
//	Common::Request::getCommand -- リクエスト番号を得る
//
//	NOTES
//	リクエスト番号を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		リクエスト番号
//
//	EXCEPTIONS
//	なし
//
int
Request::getCommand() const
{
	return m_iCommand;
}

//
//	FUNCTION public
//	Common::Request::serialize -- シリアル化を行う
//
//	NOTES
//	シリアル化を行う。
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		アーカイバ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Request::serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore())
	{
		//書出し
		cArchiver_ << m_iCommand;
	}
	else
	{
		//読出し
		cArchiver_ >> m_iCommand;
	}
}

//
//	FUNCTION public
//	Common::Request::getClassID -- クラスIDを得る
//
//	NOTES
//	クラスIDを得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		クラスID
//
//	EXCEPTIONS
//	なし
//
int
Request::getClassID() const
{
	return ClassID::RequestClass;
}

//
//	FUNCTION public
//	Common::Request::toString -- 文字列で取り出す(for Debug)
//
//	NOTES
//	デバッグ用に文字列で取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModString
//		デバッグ文字列
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ModUnicodeString
Request::toString() const
{
	ModUnicodeString s("Request:");

	switch(m_iCommand)
	{
	case BeginConnection:	s += "BeginConnection";		break;	//コネクション開始
	case EndConnection:		s += "EndConnection";		break;	//コネクション終了
	case BeginSession:		s += "BeginSession";		break;	//セッション開始
	case EndSession:		s += "EndSession";			break;	//セッション終了
	case BeginWorker:		s += "BeginWorker";			break;	//ワーカ開始
	case CancelWorker:		s += "CancelWorker";		break;	//ワーカ中断

	case Shutdown:			s += "Shutdown";			break;	//シャットダウン

	case ExecuteStatement:	s += "ExecuteStatement";	break;	//SQL文の実行
	case PrepareStatement:	s += "PrepareStatement";	break;	//SQL文のコンパイル
	case ExecutePrepare:	s += "ExecutePrepare";		break;	//コンパイル結果の実行
	case ErasePrepareStatement:	s += "ErasePrepareStatement";	break;	//コンパイル結果の削除

	case ReuseConnection:	s += "ReuseConnection";		break;	//コネクションを再利用する
	case NoReuseConnection:	s += "NoReuseConnection";	break;	//コネクションを再利用しない

	case CheckAvailability:	s += "CheckAvailability";	break;	// 利用可能性チェック

	case PrepareStatement2:	s += "PrepareStatement2";	break;	//SQL文のコンパイル
	case ErasePrepareStatement2: s += "ErasePrepareStatement2"; break;//最適化結果を削除する

	case BeginSession2:		s += "BeginSession2";		break;	// Starting session with user management
	case EndSession2:		s += "EndSession2";			break;	// Ending session with user management

	case CreateUser:		s += "CreateUser";			break;	// Creating new user
	case DropUser:			s += "DropUser";			break;	// Deleting a user
	case ChangeOwnPassword:	s += "ChangeOwnPassword";	break;	// Changing own password
	case ChangePassword:	s += "ChangePassword";		break;	// Changing others password

	case Shutdown2:			s += "Shutdown2";			break;	//サーバの終了

	case Sync:				s += "Sync";				break;	//同期

	case QueryProductVersion: s += "QueryProductVersion"; break; // Investigating product version(JDBC)

	default:				s += "Undefine";			break;	//未定義
	}
	return s;
}

//
//	Copyright (c) 2002, 2007, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
