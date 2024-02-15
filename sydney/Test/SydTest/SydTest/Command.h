// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Command.h -- コマンドクラス
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2007, 2015, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_COMMAND_H
#define __SYDNEY_SYDTEST_COMMAND_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "SydTest/Item.h"
#include "ModUnicodeString.h"
#include "ModCriticalSection.h"
#include <map>
#include <string>


_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Command -- コマンドクラス。
//
//	NOTES
//
//
//##ModelId=3A9B4746034D
class  Command : public Item
{
public:
	// コマンドタイプ
	struct CommandType
	{
		enum Value
		{
			Other,
			BeginCommand,
			EndCommand,
			Pause,
			InitSydney,
			TermSydney,
			InitSession,
			TermSession,
			Command,
			Rowset,
			AsyncCommand, 
			AsyncRowset,
			GetAsyncResult,
			CancelAsync,
			Sleep,
			ScaledSleep,
			Time,
			BeginTimespan,
			EndTimespan,
			CreateThread,
			JoinThread,
			BeginBrace,
			EndBrace,
			Exists,
			NotExists,
			System,
			EchoOn,
			EchoOff,
			ParameterEchoOn,
			ParameterEchoOff,
			SessionNumberEchoOn,
			SessionNumberEchoOff,
			SetSystemParameter,
			CreatePreparedCommand,
			PreparedCommand,
			AsyncPreparedCommand,
			ErasePreparedCommand,
			Include,
			AssureCount,
			SetEncodingType,
			Synchronize,
			IsServerAvailable,
			IsDatabaseAvailable,
			CreateUser,
			DropUser,
			ChangePassword,
			ChangeOwnPassword,
			AddCascade,
			AlterCascade,
			DropCascade,
			StartCascade,
			TerminateCascade,
			ForceTerminateCascade,
			ReloadSydney,
			PrintCriticalSection
		};

		operator Value()
		{
			return m_eCommandType;
		}

		CommandType& operator=(Value eType_)
		{
			m_eCommandType = eType_;
			return *this;
		}

		Value m_eCommandType;

	};

	//コンストラクタ
	//##ModelId=3A9B4746036E
	explicit Command(const char* pszString_);
	//デストラクタ
	//##ModelId=3A9B4746036D
	virtual ~Command();
	// コマンドの文字列を得る
	//##ModelId=3A9B4746036C
	const ModUnicodeString& getCommand() const;
	// コマンドタイプを得る
	//##ModelId=3A9B47460367
	CommandType getCommandType() const;

private:
	// コマンド文字列
	//##ModelId=3A9B47460364
	ModUnicodeString m_cstrCommand;
	// コマンドタイプ
	//##ModelId=3A9B4746035F
	CommandType m_eCType;

	// コマンドを格納するマップ
	//##ModelId=3A9B4746035A
	static std::map<std::string, CommandType::Value> m_mapCommandItem;
	//
	static ModCriticalSection m_cCriticalSection;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_COMMAND_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2005, 2007, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
