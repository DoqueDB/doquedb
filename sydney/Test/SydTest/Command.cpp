// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Command.cpp -- Sydneyに対するコマンドを表すクラス
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2015, 2017, 2018, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "SydTest";

}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SydTest/Command.h"
#include "ModCriticalSection.h"
#include "ModAutoMutex.h"

_SYDNEY_USING
using namespace SydTest;

namespace {

	// コマンド表のアイテム
	struct CommandItem
	{
		const char* m_pszCommand;
		Command::CommandType::Value m_eType;
	};

	CommandItem _pCommandItem[] = {
		{"Begin",				 Command::CommandType::BeginCommand},
		{"End",					 Command::CommandType::EndCommand},
		{"BeginBrace",			 Command::CommandType::BeginBrace},
		{"EndBrace",			 Command::CommandType::EndBrace},
		{"Pause",				 Command::CommandType::Pause},
		{"Initialize",			 Command::CommandType::InitSydney},
		{"Terminate",			 Command::CommandType::TermSydney},
		{"InitializeSession",	 Command::CommandType::InitSession},
		{"TerminateSession",	 Command::CommandType::TermSession},
		{"BeginTimeSpan",		 Command::CommandType::BeginTimespan},
		{"EndTimeSpan",			 Command::CommandType::EndTimespan},
		{"CurrentTime",			 Command::CommandType::Time},
		{"Command",				 Command::CommandType::Command},
		{"Rowset",				 Command::CommandType::Command},
		{"AsyncCommand",		 Command::CommandType::AsyncCommand},
		{"AsyncRowset",			 Command::CommandType::AsyncCommand},
		{"GetAsyncResult",		 Command::CommandType::GetAsyncResult},
		{"CancelAsync",			 Command::CommandType::CancelAsync},
		{"Sleep",				 Command::CommandType::Sleep},
		{"ScaledSleep",			 Command::CommandType::ScaledSleep},
		{"CreateThread",		 Command::CommandType::CreateThread},
		{"JoinThread",			 Command::CommandType::JoinThread},
		{"Exists",				 Command::CommandType::Exists},
		{"NotExists",			 Command::CommandType::NotExists},
		{"System",				 Command::CommandType::System}, 
		{"Include",				 Command::CommandType::Include}, 
		{"EchoOn",				 Command::CommandType::EchoOn},
		{"EchoOff",				 Command::CommandType::EchoOff},
		{"ParameterEchoOn",		 Command::CommandType::ParameterEchoOn},
		{"ParameterEchoOff",	 Command::CommandType::ParameterEchoOff},
		{"SessionNumberEchoOn",	 Command::CommandType::SessionNumberEchoOn},
		{"SessionNumberEchoOff", Command::CommandType::SessionNumberEchoOff},
		{"SetSystemParameter",	 Command::CommandType::SetSystemParameter},
		{"CreatePreparedCommand",Command::CommandType::CreatePreparedCommand},
		{"PreparedCommand",      Command::CommandType::PreparedCommand},
		{"AsyncPreparedCommand", Command::CommandType::AsyncPreparedCommand},
		{"ErasePreparedCommand", Command::CommandType::ErasePreparedCommand},
		{"AssureCount",			 Command::CommandType::AssureCount},
		{"SetEncodingType",		 Command::CommandType::SetEncodingType},
		{"Sync",				 Command::CommandType::Synchronize},
		{"IsServerAvailable",	 Command::CommandType::IsServerAvailable},
		{"IsDatabaseAvailable",	 Command::CommandType::IsDatabaseAvailable},
		{"CreateUser",			 Command::CommandType::CreateUser},
		{"DropUser",			 Command::CommandType::DropUser},
		{"ChangePassword",		 Command::CommandType::ChangePassword},
		{"ChangeOwnPassword",	 Command::CommandType::ChangeOwnPassword},
		{"AddCascade",			 Command::CommandType::AddCascade},
		{"AlterCascade",		 Command::CommandType::AlterCascade},
		{"DropCascade",			 Command::CommandType::DropCascade},
		{"StartCascade",		 Command::CommandType::StartCascade},
		{"TerminateCascade",	 Command::CommandType::TerminateCascade},
		{"ForceTerminateCascade",Command::CommandType::ForceTerminateCascade},
		{"Reload",				 Command::CommandType::ReloadSydney},
		{"PrintCriticalSection", Command::CommandType::PrintCriticalSection},
		{0,						 Command::CommandType::Other}
	};
}

_SYDNEY_USING

using namespace SydTest;

//static変数はここに宣言が必要

std::map<std::string, Command::CommandType::Value>
Command::m_mapCommandItem;

ModCriticalSection
Command::m_cCriticalSection;

//
//	FUNCTION public
//	SydTest::Command::Command -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const char* pszString_
//		コマンドの文字列
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Command::Command(const char* pszString_)
: Item(Type::Command), 
  m_cstrCommand(pszString_, 0,  ModOs::Process::getEncodingType())
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCriticalSection);
	cAuto.lock();

	if (m_mapCommandItem.size() == 0)
	{
		// マップにはまだ要素が入っていない
		CommandItem* pItem = _pCommandItem;
		while (pItem->m_pszCommand != 0)
		{
			m_mapCommandItem.insert(std::map<std::string, Command::CommandType::Value>::value_type(pItem->m_pszCommand, pItem->m_eType));
			pItem++;
		}
	}
	
	// マップの検索
	std::map<std::string, Command::CommandType::Value>::iterator i = m_mapCommandItem.find(pszString_);
	if (i == m_mapCommandItem.end())
	{
		m_eCType = CommandType::Other;
	}
	else 
	{
		m_eCType = (*i).second;
	}
}

//
//	FUNCTION public
//	SydTest::Command::~Command -- デストラクタ
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
Command::~Command()
{
}

//
//	FUNCTION public
//	SydTest::Command::getCommand -- コマンド文字列を取り出す
//
//	NOTES
//	コマンドを表す文字列を取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	char*
//		コマンドを表す文字列
//
//	EXCEPTIONS
//	なし
//
//
const ModUnicodeString&
Command::getCommand() const
{
	return m_cstrCommand;
}

//
//	FUNCTION public
//	SydTest::Command::getCommandType -- コマンドの種類を得る
//
//	NOTES
//	コマンドのタイプを取り出す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Command::CommandType
//		コマンドの種類
//
//	EXCEPTIONS
//	なし
//
//

Command::CommandType
Command::getCommandType() const
{
	return m_eCType;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2015, 2017, 2018, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
