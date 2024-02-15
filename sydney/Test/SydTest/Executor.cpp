// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.cpp -- テストを実行するクラスの実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
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
#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/ExceptionMessage.h"
#include "Common/Parameter.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"
#include "Server/Singleton.h"
#include "Client2/Singleton.h"
#include "Client2/Session.h"
#include "Client2/DataSource.h"
#include "Client2/ResultSet.h"
#include "Client2/PrepareStatement.h"
#include "Exception/Object.h"
#include "Exception/Cancel.h"
#include "Exception/ConnectionRanOut.h"
#include "Exception/SessionBusy.h"
#include "SydTest/Item.h"
#include "SydTest/Command.h"
#include "SydTest/String.h"
#include "SydTest/Number.h"
#include "SydTest/Parameter.h"
#include "SydTest/Parser.h"
#include "SydTest/CascadeConf.h"
#include "SydTest/Executor.h"
#include "SydTest/SydTestMessage.h"
#include "SydTest/SydTestException.h"
#include "SydTest/StopWatch.h"
#include "SydTest/Monitor.h"
#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModParameter.h"
#if MOD_CONF_REREGISTRY == 0
#include "ModParameterSource.h"
#endif
#include "ModThread.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "ModCharString.h"
#include "ModUnicodeOstrStream.h"
#include "ModVector.h"
#ifdef SYD_OS_LINUX
#include "Os/CriticalSectionManager.h"
#endif

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>

_SYDNEY_USING

namespace {

	ModSize getThreshold() {
		int result = Common::SystemParameter::getInteger(
			_TRMEISTER_U_STRING("SydTest_MessageDisplayThreshold"));
		return (result <= 0) ? ModSizeMax : static_cast<ModSize>(result);
	}

	ModUnicodeString tupleToString(Common::DataArrayData* pTuple)
	{
		ModUnicodeOstrStream result;
		result << "{";
		for (int i = 0; i < pTuple->getCount(); i++)
		{
			Common::Data* pData = pTuple->getElement(i).get();
			if (i > 0)
				result << ",";
			if (pData) {
				if (pData->getType() != Common::DataType::Binary) {
					result << pData->getString();
				} else {
					Common::BinaryData* pBin = dynamic_cast<Common::BinaryData*>(pData);
					const unsigned char* val = (const unsigned char*)pBin->getValue();
					int size = pBin->getSize();
					for (int j=0; j<size; j++) {
						result.setf(ModIos::right | ModIos::hex);
						result.width(2);
						result.fill('0');
						result << int(*(val+j));
					}
				}
			} else {
				result << "(0)";
			}
		}
		result << "}";
		return ModUnicodeString(result.getString());
	}

#ifdef SYD_OS_POSIX
	// 文字列を置換する
	bool
	commandReplace(std::string& str,
				   const std::string& subold, const std::string& subnew)
	{
		bool result = false;
		if (str.find(subold, 0) == 0)
		{
			str.replace(0, subold.size(), subnew);
			result = true;
		}
		return result;
	}
	// 文字列を置換する
	std::string&
	replace(std::string& str,
			const std::string& subold, const std::string& subnew)
	{
		std::string::size_type n;
		std::string::size_type nold = 0;
	
		while ((n = str.find(subold, nold)) != std::string::npos)
		{
			str.replace(n, subold.size(), subnew);
			nold = n + subnew.size();
		}
		return str;
	}
#endif

#if MOD_CONF_REGISTRY == 0
	ModParameter _param(ModParameterSource(0, "SYDPARAM", 0, "SYDSYSPARAM", 0), ModTrue);

	// Append the new parameter to the file.
	//
	// In Windows, set*() changes the registry with the new parameter.
	// So, in Linux, the new parameter should be appended to the file.
	void
	appendParameter(const char* path, std::string sParameter_, std::string sValue_)
	{
		// For Solaris, if temp includes "(xxx)", xxx is executed with subshell.
		//std::string temp =
		//	"echo " + sParameter_ + " " + sValue_ + " | cat >> $SYDPARAM";
		//system(temp.data());

		// char* path = std::getenv("SYDPARAM");
		std::ofstream file;
		file.open(path, std::ios::out | std::ios::app);
		file << sParameter_ << " " << sValue_ << std::endl;
		file.close();
	}
	void
	appendParameter(const char* path, const char* pszParameter_, const char* pszValue_)
	{
		std::string temp = "\"" + std::string(pszValue_) + "\"";
		appendParameter(path, std::string(pszParameter_), temp);
	}
	void
	appendParameter(const char* path, const char* pszParameter_, int iValue_)
	{
		std::stringstream temp;
		temp << iValue_;
		appendParameter(path, std::string(pszParameter_), temp.str());
	}

#endif

}

using namespace SydTest;

//staticな変数の宣言
Map<int, Client2::DataSource*> Executor::m_mapDataSource;
Map<int, Client2::Session*> Executor::m_mapClient;
Map<int, ModUnicodeString> Executor::m_mapDBName;
Map<ModUnicodeString, Executor::AsyncCommandPair> Executor::m_mapAsyncCommand;
Map<ModUnicodeString, Executor::PreparedCommandPair> Executor::m_mapPreparedCommand;
Map<ModUnicodeString, Executor::SyncPair> Executor::m_mapConditionVariable;
StopWatch Executor::m_stopWatch;
Map<int, ModCriticalSection*> Executor::m_mapCSForClient;
ModCriticalSection Executor::m_cCSForClient;
ModCriticalSection Executor::m_cCSForParameter;
ModCriticalSection Executor::m_cCSForFile;

namespace {
	//
	//	_$$::_cDataSource
	//


	Map<int, int> _mapResultCount;

	void _setResultCount(int iSession_, int iCount_)
	{
		_mapResultCount[iSession_] = iCount_;
	}
	int _getResultCount(int iSession_)
	{
		return _mapResultCount[iSession_];
	}
}

//
//	FUNCTION public
//	SydTest::Executor::Executor -- コンストラクタ
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
Executor::Executor(const char* pszFileName_,
				   const char* pszLabel_, int iOption_,
				   Monitor& rMonitor_,
				   const ModUnicodeString& cstrHostName_, int iPortNumber_,
				   const ModUnicodeString& cstrRegPath_,
				   const ModVector<CascadeInfo>& cascadeInfoVector_,
				   const Communication::CryptMode::Value cryptMode_,
				   const ModUnicodeString& cstrUserName_,
				   const ModUnicodeString& cstrPassword_)
: m_strFileName(pszFileName_),
  m_strLabel(pszLabel_),
  m_iOption(iOption_), m_rMonitor(rMonitor_),
  m_cstrHostName(cstrHostName_), m_iPortNumber(iPortNumber_),
  m_cstrUserName(cstrUserName_), m_cstrPassword(cstrPassword_),
  m_iSleepingRate(Common::SystemParameter::getInteger("SydTest_SleepingRate")),
  m_cstrRegPath(cstrRegPath_),
  m_cryptMode(cryptMode_),
  m_bReplaceBackSlash(false),
  m_bSydneyInitializedFlg(false),
  m_cascadeInfoVector(cascadeInfoVector_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCSForParameter);
	cAuto.lock();

	// 0や負の数が設定されていた場合や設定されていなかった場合は100とする
	if (m_iSleepingRate <= 0) 
	{
		m_iSleepingRate = 100;
	}

	if ((m_iOption & ExecuteOption::ReplaceBackSlash) != 0)
	{
		m_bReplaceBackSlash = true;
	}
}

//
//	FUNCTION public
//	SydTest::Executor::~Executor -- デストラクタ
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
Executor::~Executor()
{
}

//
//	FUNCTION public
//	  Executor::getNextCommandWithoutTiming -- 時計を止めてコマンドを実行する
//
//	NOTES
//    「時計を止める→getNextCommand()→時計を再開」
//    を行うwrapper関数
//
//	ARGUMENTS
//	  Parser& rParser_
//    パーザを表すオブジェクト
//
//	RETURN
//    Item* pItem
//    次に実行するコマンド
//
//	EXCEPTIONS
//    なし
//
Item*
Executor::getNextCommandWithoutTiming(Parser& rParser_)
{
	Item* pItem=0;
	m_stopWatch.stopAll();
	pItem = rParser_.getNextCommand(m_bReplaceBackSlash);
	m_stopWatch.startAll();
	return pItem;
}

//
//	FUNCTION public
//	SydTest::Executor::executeSydneyCommand -- コマンドの解釈と実行
//
//	NOTES
//	渡されたItemクラスより、コマンドの内容を判断してSydneyに対するコマンドを
//  実行するメソッドを呼び出す。
//  また、適切なエラー処理を行う
//
//	ARGUMENTS
//	Item* pItem_
//		コマンドを表すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Executor::executeSydneyCommand(Command* pCommand_)
{
	switch (pCommand_->getCommandType())
	{
	case Command::CommandType::Pause:
	{
		if (!((m_iOption & ExecuteOption::IgnorePause)
		   || (m_iOption & ExecuteOption::DebugMode)))
		{
			// 実行の一時停止をする
			char c;
			std::cout << "SydTest: Press RETURN key to continue." << std::endl;
			while (1)
			{
				std::cin.get(c);
				if (c == '\n')
					break;
			}
		}
		break;
	}
	case Command::CommandType::InitSydney:
		// Sydneyの初期化
		if (!(m_iOption & ExecuteOption::DebugMode))
		{
			try {
				initialize(pCommand_);
			} catch (Exception::Object& err) {
				if (m_iOption & ExecuteOption::AbortWhenInitFail)
				{
					SydAssert(false);
				}
				SydTestErrorMessage << err << ModEndl;
				throw SydTestException(SydTestException::Fatal);
			} catch (...) {
				if (m_iOption & ExecuteOption::AbortWhenInitFail)
				{
					SydAssert(false);
				}
				SydTestErrorMessage << "Failed in initializing. Stop." << ModEndl;
				throw SydTestException(SydTestException::Fatal);
			}
		}
		break;
	case Command::CommandType::TermSydney:
		// Sydneyの終了
		if (!(m_iOption & ExecuteOption::DebugMode))
		{
			try {
				terminate(pCommand_);
			} catch (Exception::Object& err) {
				// SydTestに起因しないエラーはrethrowしない(以下同じ)
				SydTestErrorMessage << err << ModEndl;
			}
		}
		break;
	case Command::CommandType::InitSession:
		initializeSession(pCommand_);
		break;
	case Command::CommandType::TermSession:
		try {
			terminateSession(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::Command:
		try{
			executeSQLCommand(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::AsyncCommand:
		try{
			executeAsyncSQLCommand(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::GetAsyncResult:
		try{
			if (!(m_iOption & ExecuteOption::DebugMode))
			{
				getAsyncSQLResult(pCommand_);
			}
		} catch (SydTestException& err) {
			if (err.getDescription() == SydTestException::TagNotFound)
				SydTestErrorMessage << "Tag not found. " << ModEndl;
			else throw;
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::CancelAsync:
		try{
			if (!(m_iOption & ExecuteOption::DebugMode))
			{
				cancelAsyncSQLCommand(pCommand_);
			}
		} catch (SydTestException& err) {
			if (err.getDescription() == SydTestException::TagNotFound)
				SydTestErrorMessage << "Tag not found. " << ModEndl;
			else throw;
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::CreatePreparedCommand:
		try{
			prepareSQLCommand(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::PreparedCommand:
		try{
			executePreparedSQLCommand(pCommand_);
		} catch (SydTestException& err) {
			if (err.getDescription() == SydTestException::TagNotFound)
				SydTestErrorMessage << "Tag not found. " << ModEndl;
			else throw;
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::AsyncPreparedCommand:
		try{
			executeAsyncPreparedSQLCommand(pCommand_);
		} catch (SydTestException& err) {
			if (err.getDescription() == SydTestException::TagNotFound)
				SydTestErrorMessage << "Tag not found. " << ModEndl;
			else throw;
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::ErasePreparedCommand:
		try{
			erasePreparedSQLCommand(pCommand_);
		} catch (SydTestException& err) {
			if (err.getDescription() == SydTestException::TagNotFound)
				SydTestErrorMessage << "Tag not found. " << ModEndl;
			else throw;
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::Sleep:
		if (!(m_iOption & ExecuteOption::DebugMode))
		{
			sleep(pCommand_);
		}
		break;
	case Command::CommandType::ScaledSleep:
		if (!(m_iOption & ExecuteOption::DebugMode))
		{
			scaledSleep(pCommand_);
		}
		break;
	case Command::CommandType::Time:
		if (!(m_iOption & ExecuteOption::DebugMode))
		{
			displayCurrentTime();
		}
		break;
	case Command::CommandType::BeginTimespan:
		beginTimespan(pCommand_);
		break;
	case Command::CommandType::EndTimespan:
		endTimespan(pCommand_);
		break;
	case Command::CommandType::CreateThread:
		createThread(pCommand_);
		break;
	case Command::CommandType::JoinThread:
		joinThread(pCommand_);
		break;
	case Command::CommandType::Include:
		try {
			fileInclude(pCommand_);
		} catch (SydTestException& err) {
			if (err.getDescription() == SydTestException::FileNotFound)
				SydTestErrorMessage << "File not found. " << ModEndl;
			else throw;
		}
		break;
	case Command::CommandType::Exists:
		exists(pCommand_);
		break;
	case Command::CommandType::NotExists:
		notExists(pCommand_);
		break;
	case Command::CommandType::System:
		if (!(m_iOption & ExecuteOption::DebugMode))
		{
			doOsCommand(pCommand_);
		}
		break;
	case Command::CommandType::EchoOn:
		m_iOption |= ExecuteOption::ShowCommand;
		break;
	case Command::CommandType::EchoOff:
		m_iOption &= ~ExecuteOption::ShowCommand;
		break;
	case Command::CommandType::ParameterEchoOn:
		m_iOption |= ExecuteOption::ShowParameter;
		break;
	case Command::CommandType::ParameterEchoOff:
		m_iOption &= ~ExecuteOption::ShowParameter;
		break;
	case Command::CommandType::SessionNumberEchoOn:
		m_iOption |= ExecuteOption::ShowSessionNumber;
		break;
	case Command::CommandType::SessionNumberEchoOff:
		m_iOption &= ~ExecuteOption::ShowSessionNumber;
		break;
	case Command::CommandType::SetSystemParameter:
		setSystemParameter(pCommand_);
		break;
	case Command::CommandType::AssureCount:
		assureCount(pCommand_);
		break;
	case Command::CommandType::SetEncodingType:
		setEncodingType(pCommand_);
		break;
	case Command::CommandType::Synchronize:
		synchronize(pCommand_);
		break;
	case Command::CommandType::IsServerAvailable:
		isServerAvailable(pCommand_);
		break;
	case Command::CommandType::IsDatabaseAvailable:
		isDatabaseAvailable(pCommand_);
		break;
	case Command::CommandType::CreateUser:
		try {
			createUser(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::DropUser:
		try {
			dropUser(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::ChangePassword:
		try {
			changePassword(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::ChangeOwnPassword:
		try {
			changeOwnPassword(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::AddCascade:
		try {
			addCascade(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::AlterCascade:
		try {
			alterCascade(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		} 
		break;
	case Command::CommandType::DropCascade:
		try {
			dropCascade(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		} 
		break;
	case Command::CommandType::StartCascade:
		try {
			startCascade(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::TerminateCascade:
		try {
			terminateCascade(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::ForceTerminateCascade:
		try {
			forceTerminateCascade(pCommand_);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::ReloadSydney:
		try {
			if (!(m_iOption & ExecuteOption::DebugMode))
			{
				if (m_iPortNumber == -1)
					// InProcessの場合だけ実行する
					Server::Singleton::InProcess::reload();
			}
		} catch (Exception::Object& err) {
			SydTestErrorMessage << err << ModEndl;
		}
		break;
	case Command::CommandType::PrintCriticalSection:
#ifdef SYD_OS_LINUX
		// Linux 環境のみで有効
		// 実際は SIGUSR1 により実行される機能だが SydTest には ServiceModule がないのでコマンドとして実装
		try {
			Os::CriticalSectionManager::printOut();
		} catch (...) {
			// シグナルハンドラ同様、例外が発生しても握りつぶす
		}
#else
		SydTestErrorMessage << "PrintCriticalSection is only available on Linux" << ModEndl;
		throw SydTestException();
#endif		
		break;
	default:
		break;
	}
	// モニタに触る
	m_rMonitor.reset();
	pCommand_->release();
}

//
//	FUNCTION public
//	Executor::ExecuteMain -- スクリプトの本体部分を実行する
//
//	NOTES
//	  スクリプトの本体部分を実行する
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void 
Executor::executeMain()
{
	Parser cParser;
	if (!cParser.openFile(m_strFileName.getString())) {
		SydErrorMessage << m_strFileName.getString()
		           << ": File Open Error." << ModEndl;
		throw SydTestException(SydTestException::FileNotFound);
	}

	int iBeginFlag = 0;
	int iSaveFlag = 0;
	while (1)
	{
		ModAutoPointer<Item> pItem = getNextCommandWithoutTiming(cParser);
		if (pItem.get() == 0) {
			break;
		}

		if (pItem->getType() != Item::Type::Command)
		{
			SydTestErrorMessage << "Not a Command." << ModEndl;
			throw SydTestException(cParser.getLine());
		}
		else 
		{
			Command* pCommand = dynamic_cast<Command*>(pItem.get());
			Command::CommandType cType = pCommand->getCommandType();
			if (iBeginFlag == 0)
			{
				if (cType == Command::CommandType::BeginCommand)
				{
					iBeginFlag = 1;
					if (isSydTestInfoMessage())
					{
						SydTestInfoMessage << m_strLabel << " Start." << ModEndl;
					}
				}
				else if (cType == Command::CommandType::EndCommand)
				{
					if (isSydTestInfoMessage())
					{
						SydTestInfoMessage << m_strLabel << " End." << ModEndl;
					}

					break;
				}
			}
			else
			{
				if (cType == Command::CommandType::Other) // スレッド名かもしれない
				{
					Command* pNextCommand = dynamic_cast<Command*>(pItem->getNext());
					if (pNextCommand && pNextCommand->getCommandType() 
						== Command::CommandType::BeginBrace)
					{
						iSaveFlag = iBeginFlag;
						iBeginFlag = -1;
					}
					else
					{
						SydTestErrorMessage << "No such command '" 
							<< pCommand->getCommand() << "'."<< ModEndl;
						throw SydTestException(cParser.getLine());
					}
				}
				else if (cType == Command::CommandType::BeginBrace)
				{
					iSaveFlag = iBeginFlag;
					iBeginFlag = -1;
				}
				else if (cType == Command::CommandType::EndBrace)
				{
					iBeginFlag = iSaveFlag;
				}
				else if (cType == Command::CommandType::EndCommand && iBeginFlag == 1)
				{
					if (isSydTestInfoMessage())
					{
						SydTestInfoMessage << m_strLabel << " End." << ModEndl;
					}
					break;
				}
				else if (iBeginFlag == 1)
				{
					if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
					{
						SydTestInfoMessage << "[" << m_strLabel
							<< ":" << cParser.getLine() << "] " 
							<< pCommand->getCommand() << ModEndl;
					}
					try {
						executeSydneyCommand(pCommand);
					} catch (SydTestException& err) {
						err.setLine(cParser.getLine());
						throw;
					}
				}
			}
		}
	}
}

//
//	FUNCTION public
//	Executor::ExecuteMain -- スクリプトのスレッド部分を実行する
//
//	NOTES
//	  スクリプトのスレッド部分を実行する
//
//	ARGUMENTS
//    const char* pszLabel_  スレッド名
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void 
Executor::executeThread(const char* pszLabel_)
{
	Parser cParser;
	cParser.openFile(m_strFileName.getString());
	if (!cParser.getLabel(pszLabel_))
	{
		SydTestErrorMessage << "No clause for " << pszLabel_ << "." << ModEndl;
		throw SydTestException(cParser.getLine());
	}

	if (isSydTestInfoMessage())
	{
		SydTestInfoMessage << pszLabel_ << " begin." << ModEndl;
	}

	int iBeginFlag = 0;
	int iSaveFlag = 0;
	while (1)
	{
		ModAutoPointer<Item> pItem = getNextCommandWithoutTiming(cParser);
		if (pItem.get() == 0) {
			break;
		}

		if (pItem->getType() == Item::Type::Command)
		{
			Command* pCommand = dynamic_cast<Command*>(pItem.get());
			Command::CommandType cType = pCommand->getCommandType();
			// Braceの外側にいる場合
			if (iBeginFlag == 0)
			{
				if (cType == Command::CommandType::BeginBrace)
				{
					iBeginFlag = 1;
				}
				// Braceの更に内側のBraceから抜けた可能性もあり、Flagを元の状態に戻す
				else if (cType == Command::CommandType::EndBrace)
				{
					iBeginFlag = iSaveFlag;
				}
			}
			// Braceの内側にいる場合
			else if (iBeginFlag == 1)
			{
				// 更に内側のBraceに突入（Threadが更にThreadを呼ぶ場合)
				if (cType == Command::CommandType::BeginBrace)
				{
					iSaveFlag = iBeginFlag;
					iBeginFlag = 0;
				}
				// Threadの実行範囲終了
				else if (cType == Command::CommandType::EndBrace)
				{
					// さらに後ろにある同名のClauseを探す
					if (cParser.getLabel(pszLabel_))
					{
						iBeginFlag = 0;
					}
					else
					{
						if (isSydTestInfoMessage())
						{
							SydTestInfoMessage << pszLabel_ << " end." << ModEndl;
						}
						break;
					}
				}
				// 命令実行
				else
				{
					if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
					{
						SydTestInfoMessage << "[" << pszLabel_ << ":" << cParser.getLine() 
						<< "] " << pCommand->getCommand() << ModEndl;
					}
					try {
						executeSydneyCommand(pCommand);
					} catch (SydTestException& err) {
						err.setLine(cParser.getLine());
						throw err;
					}
				}
			}
		}
		else
		{
			SydTestErrorMessage << "Not a Command." << ModEndl;
			throw SydTestException(cParser.getLine());
		}
	}
}

//  private functions

//
//	FUNCTION private
//	SydTest::Executor::initialize -- Sydneyの初期化
//
//	NOTES
//	Sydneyを初期化する
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
void
Executor::initialize(Item* pItem_)
{
	Client2::DataSource* pDataSource = 0;
	int iConnectionID = 0;
	int iCascadeID = -1;
	Item* pNext = pItem_->getNext();

	// initialize のパラメータが省略されていない場合
	if (pNext != 0) {
		// オプション名が指定されていた場合は適切なオプション名か判定
		if (pNext->getType() == Item::Type::Option) {
			Option* pOpt = dynamic_cast<Option*> (pNext);
			if (pOpt->getOptionType() != Option::OptionType::ConnectionID) {
				SydTestErrorMessage << "Wrong option name: " << pOpt->getOptionName() << ModEndl;
				throw SydTestException();
			}
			// パラメータ値が未設定
			if ((pNext = pNext->getNext()) == 0) 
			{
				SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
				throw SydTestException();
			}
		}
		// ConnectionID の指定
		if (pNext->getType() != Item::Type::Number) {
			SydTestErrorMessage << "Invalid parameter type. " << ModEndl;
			throw SydTestException();
		}
		Number* pNum = dynamic_cast<Number*> (pNext);
		iConnectionID = pNum->getNumber();
		pNext = pNum->getNext();

		// CascadeID が省略されていない場合
		if (pNext != 0) {
			// オプション名が指定されていた場合は適切なオプション名か判定
			if (pNext->getType() == Item::Type::Option) {
				Option* pOpt = dynamic_cast<Option*> (pNext);
				if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
					SydTestErrorMessage << "Improper Option Name: " << pOpt->getOptionName() << ModEndl;
					throw SydTestException();
				}
				// パラメータ値が未設定
				if ((pNext = pNext->getNext()) == 0) 
				{
					SydTestErrorMessage << "No Parameter Value. " << ModEndl;
					throw SydTestException();
				}
			}
			// CascadeID の指定
			if (pNext->getType() != Item::Type::Number) {
				SydTestErrorMessage << "Invalid parameter Type. " << ModEndl;
				throw SydTestException();
			}
			pNum = dynamic_cast<Number*> (pNext);
			iCascadeID = pNum->getNumber();
			
			if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
			{
				SydTestInfoMessage << "[SydTest Option] " 
								   << "ConnectionID: " << iConnectionID 
								   << ", CascadeID: " << iCascadeID << ModEndl;
			}

			// CascadeID < 0 ならエラー
			if (iCascadeID < 0) {
				SydTestErrorMessage << "Invalid Cascade ID (<0)." << ModEndl;
				throw SydTestException();
			}
			
			// CascadeID の範囲外エラー
			if ((unsigned) iCascadeID >= m_cascadeInfoVector.getSize()) {
				SydTestErrorMessage << "Undefined Cascade ID (" << iCascadeID << ")." << ModEndl;
				throw SydTestException();
			}
		}
	}

	if(m_mapDataSource.exists(iConnectionID)) {
		SydTestErrorMessage<<"Connection " << iConnectionID << " already exists." << ModEndl;
		throw SydTestException();
	}

	if (iCascadeID == -1 && m_iPortNumber == -1) {
		if(!m_bSydneyInitializedFlg) {
			// Sydneyの初期化
			Server::Singleton::InProcess::initialize(false, m_cstrRegPath);
			m_bSydneyInitializedFlg = true;
		}

		pDataSource = Client2::DataSource::createDataSource();
	} else {
		if(!m_bSydneyInitializedFlg) {
			// Sydneyの初期化

			// 分散マネージャ(分散環境でない場合を含む)または /dist で指定された子サーバの場合はデフォルトパスで初期化
			if (iCascadeID == -1 || m_cascadeInfoVector[iCascadeID].cstrConfPath.getLength() == 0) {
				Client2::Singleton::RemoteServer::initialize(m_cstrRegPath);
			}
			// /distfile で指定された子サーバの場合は対応するレジストリパスで初期化
			else 
			{
				Client2::Singleton::RemoteServer::initialize(m_cascadeInfoVector[iCascadeID].cstrConfPath);
			}
			m_bSydneyInitializedFlg = true;
		}	
		
		pDataSource = Client2::DataSource::createDataSource( (iCascadeID == -1)? m_cstrHostName : m_cascadeInfoVector[iCascadeID].cstrHostName,
															 (iCascadeID == -1)? m_iPortNumber  : m_cascadeInfoVector[iCascadeID].iPortNumber);
	}

	if (m_iOption & ExecuteOption::Explain) {
		pDataSource->open(Client2::DataSource::Protocol::Version3 | m_cryptMode);
	} else if (m_iOption & ExecuteOption::Prepared) {
		pDataSource->open(Client2::DataSource::Protocol::Version3 | m_cryptMode);
	} else {
		pDataSource->open(Client2::DataSource::Protocol::Version1 | m_cryptMode);
	}

	m_mapDataSource.insert(iConnectionID, pDataSource);

//#ifdef SYD_OS_POSIX
//	m_bUnreplaceBackSlash =
//		Common::SystemParameter::getBoolean("SydTest_UnreplaceBackSlash");
//#endif
}

//
//	FUNCTION private
//	SydTest::Executor::terminate -- Sydneyの終了
//
//	NOTES
//	Sydneyを終了する
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
void
Executor::terminate(Item* pItem_ )
{
	int iConnectionID = 0;
	// デフォルトの CascadeID -1 は分散環境では分散マネージャを表す
	int iCascadeID = -1;
	Item* pNext = pItem_->getNext();

	// terminate <num> の場合は、指定した番号をクライアントIDをする
	if (pNext != 0) {
		// オプション名が指定されていた場合は適切なオプション名か判定
		if (pNext->getType() == Item::Type::Option) {
			Option* pOpt = dynamic_cast<Option*> (pNext);
			if (pOpt->getOptionType() != Option::OptionType::ConnectionID) {
				SydTestErrorMessage << "Wrong Option name for initialize: " << ModEndl;
				throw SydTestException();
			}
			// パラメータ値が未設定
			if ((pNext = pNext->getNext()) == 0) 
			{
				SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
				throw SydTestException();
			}
		}

		if (pNext->getType() != Item::Type::Number) {
			// 数字以外のタイプの場合は、Exceptionとする
			SydTestErrorMessage << "Invalid Parameter Type. " << ModEndl;
			throw SydTestException();
		}
		Number* pNum = dynamic_cast<Number*> (pNext);
		iConnectionID = pNum->getNumber();
		pNext = pNum->getNext();
		
		// CascadeID が省略されていない場合
		if (pNext != 0) {
			// オプション名が指定されていた場合は適切なオプション名か判定
			if (pNext->getType() == Item::Type::Option) {
				Option* pOpt = dynamic_cast<Option*> (pNext);
				if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
					SydTestErrorMessage << "Wrong Option name for initialize: " << pOpt->getOptionName() << ModEndl;
					throw SydTestException();
				}
				// パラメータ値が未設定
				if ((pNext = pNext->getNext()) == 0) 
				{
					SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
					throw SydTestException();
				}
			}
			// CascadeID の指定
			if (pNext->getType() != Item::Type::Number) {
				SydTestErrorMessage << "Invalid Parameter Type. " << ModEndl;
				throw SydTestException();
			}
			pNum = dynamic_cast<Number*> (pNext);
			iCascadeID = pNum->getNumber();
			
			// CascadeID < 0 ならエラー
			if (iCascadeID < 0) {
				SydTestErrorMessage << "Invalid Cascade ID (<0)." << ModEndl;
				throw SydTestException();
			}
			
			// /dist または /distfile オプションで指定した個数を超える CascadeID が指定されていたらエラー
			if ((unsigned) iCascadeID >= m_cascadeInfoVector.getSize()) {
				SydTestErrorMessage << "Undefined Cascade ID (" << iCascadeID << ")." << ModEndl;
				throw SydTestException();
			}
		}
		
	}
	
	// 対応するデータソースが存在しない場合は、Exceptionとする
	if(!m_mapDataSource.exists(iConnectionID)){
		SydTestErrorMessage<<"Connection " << iConnectionID << " no exists." << ModEndl;
		throw SydTestException();
	}
	
	Client2::DataSource* pDataSource = m_mapDataSource[iConnectionID];

	if (pDataSource)
	{
		pDataSource->close();
		pDataSource->release();
		pDataSource = 0;
	}
	m_mapDataSource.erase(iConnectionID);

	if (m_mapDataSource.isEmpty()) {
		if (iCascadeID == -1 && m_iPortNumber == -1)
		{
			// Sydneyの終了
			Server::Singleton::InProcess::terminate();
		}
		else
		{
			// Sydneyの終了
			Client2::Singleton::RemoteServer::terminate();
		}
		m_bSydneyInitializedFlg = false;
	}
}

//
//	FUNCTION private
//	SydTest::Executor::initializeSession -- セッションの初期化
//
//	NOTES
//	DB名、またはセッション番号を元にセッションの初期化を行う
//
//	ARGUMENTS
//	Item* pItem_
//		コマンドを表すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Executor::initializeSession(Item* pItem_)
{
	int iSessionNumber = -1;

	Item* pNext = pItem_->getNext();

	// パラメーターがなければエラー
	if (pNext == 0) 
	{
		SydTestErrorMessage << "No Parameter. " << ModEndl;
		throw SydTestException();
	}
	// オプション名が指定されていた場合は適切なオプション名か判定
    /**
	   TODO: 引数1個の場合と2個の場合で第一引数のパラメータが異なることに対応する
	 */
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::ConnectionID && 
			pOpt->getOptionType() != Option::OptionType::SessionID) {
			SydTestErrorMessage << "Wrong Option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}
	// コマンドの引数で整数が来た場合はセッション番号
	// i.e. この実装は"InitializeSession Num DBName"という語順も許している
	int iConnectionNumber = 0;
	if (pNext->getType() == Item::Type::Number)
	{
		Number* pNum = dynamic_cast<Number*> (pNext);
		iSessionNumber = pNum->getNumber();
		pNext = pNum->getNext();

		if (pNext == 0) {
			SydTestErrorMessage << "Second Parameter empty. " << ModEndl;
			throw SydTestException();
		}

		if (pNext->getType() == Item::Type::Option) {
			Option* pOpt = dynamic_cast<Option*> (pNext);
			// SessionID 以外にも DBName の場合もあるため、ここでは例外処理をしない
			if (pOpt->getOptionType() == Option::OptionType::SessionID) {
				// パラメータ値が未設定
				if ((pNext = pNext->getNext()) == 0) 
				{
					SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
					throw SydTestException();
				}
			}
		}

		// 整数が二つ並んだ場合は、"InitializeSession ConnectionID SessionID DBName"の語順となる.
		if(pNext->getType() == Item::Type::Number)
		{
			iConnectionNumber = iSessionNumber;
			Number* pNum = dynamic_cast<Number*> (pNext);
			iSessionNumber = pNum->getNumber();
			pNext = pNum->getNext();
		}
	}

	// DBName オプション名が指定されている場合（省略可能）
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::DBName) {
			SydTestErrorMessage << "Wrong Option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}

	// コマンドの引数が文字列でなければエラー
	if (pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "Database Name Empty. " << ModEndl;
		throw SydTestException();
	}

	String* pDatabase = dynamic_cast<String*> (pNext);
	ModUnicodeString cstrDatabaseName = pDatabase->getUnicodeString();

	ModUnicodeString cstrUserName;
	ModUnicodeString cstrPassword;
	pNext = pNext->getNext();
			
	if (iSessionNumber == -1)
	{
		if (pNext && pNext->getType() == Item::Type::Number)
		{
			Number* pNum = dynamic_cast<Number*> (pNext);
			iSessionNumber = pNum->getNumber();
			pNext = pNext->getNext();
			if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
			{
				SydTestInfoMessage << "[SydTest Option] " 
					<< cstrDatabaseName << ", " << iSessionNumber << ModEndl;
			}
		} else {
			// 番号指定がない場合はセッション番号0
			if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
			{
				SydTestInfoMessage << "[SydTest Option] " << cstrDatabaseName << ModEndl;
			}
			iSessionNumber = 0;
		}
	} else {
		if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
		{
			SydTestInfoMessage << "[SydTest Option] " << iSessionNumber
				<< ", " << cstrDatabaseName << ModEndl;
		}
	}

	if (pNext == 0) {
		// use default user and password passed by command line options
		cstrUserName = m_cstrUserName;
		cstrPassword = m_cstrPassword;
	} else {
		if (pNext->getType() != Item::Type::String) {
			SydTestInfoMessage << "Invalid user name specification." << ModEndl;
			throw SydTestException();
		}
		cstrUserName = dynamic_cast<String*>(pNext)->getUnicodeString();
		pNext = pNext->getNext();
		if (pNext && pNext->getType() == Item::Type::String) {
			cstrPassword = dynamic_cast<String*>(pNext)->getUnicodeString();
			pNext = pNext->getNext();
		}
	}

	if (m_mapClient.exists(iSessionNumber))
	{
		SydTestErrorMessage << "Session " << iSessionNumber << " already exists." << ModEndl;
		throw SydTestException();
	}

	Client2::Session* pClient = 0;
	if (!(m_iOption & ExecuteOption::DebugMode))
	{

		if(!m_mapDataSource.exists(iConnectionNumber)){
			SydTestErrorMessage<<"Client " << iConnectionNumber << " no exists." << ModEndl;
			throw SydTestException();
		}
		Client2::DataSource* pDataSource = m_mapDataSource[iConnectionNumber];
		
		try {
			pClient = pDataSource->createSession(cstrDatabaseName, cstrUserName, cstrPassword);
			if (isSydTestInfoMessage())
			{
				SydTestInfoMessage << "Session Initialize : " << iSessionNumber << ModEndl;
			}
		} catch (Exception::Object& err) {
			SydTestErrorMessage << getSessionNumberString(iSessionNumber)
								<< err << ModEndl;
		}
	}
	if (pClient) {
		m_mapClient.insert(iSessionNumber, pClient);
		m_mapDBName.insert(iSessionNumber, cstrDatabaseName);
		m_mapCSForClient.insert(iSessionNumber, new ModCriticalSection);

		if (m_iOption & ExecuteOption::Explain) {
			startExplain(iSessionNumber, pClient);
		}
	}
}

//
//	FUNCTION private
//	SydTest::Executor::terminateSession -- セッションの終了
//
//	NOTES
//	セッション番号をもとにセッションの終了処理を行う
//
//	ARGUMENTS
//	Item* pItem_
//		コマンドを表すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
Executor::terminateSession(Item* pItem_)
{
	int iNumber = 0;
	Item* pNext = pItem_->getNext();
	// コマンドの引数として整数がある場合はセッション番号
	if (pNext != 0)
	{
		if (pNext->getType() == Item::Type::Option)
		{
			Option* pOpt = dynamic_cast<Option*> (pNext);
			if (pOpt->getOptionType() != Option::OptionType::SessionID) {
				SydTestErrorMessage << "Wrong Option name: " << pOpt->getOptionName() << ModEndl;
				throw SydTestException();
			}
			
			// パラメータ値が未設定
			if ((pNext = pNext->getNext()) == 0) 
			{
				SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
				throw SydTestException();
			}
		}

		if (pNext->getType() == Item::Type::Number)
		{
			Number* pNum = dynamic_cast<Number*> (pNext);
			iNumber = pNum->getNumber();
			if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
			{
				SydTestInfoMessage << "[SydTest Option] " 
								   << iNumber << ModEndl;
			}
		}
	}

	if (m_mapClient.exists(iNumber) == false)
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not initialized." << ModEndl;
		throw SydTestException();
	}

	if (m_iOption & ExecuteOption::Explain) {
		endExplain(iNumber, m_mapClient[iNumber]);
	}

	if (!(m_iOption & ExecuteOption::DebugMode))
	{
		m_mapClient[iNumber]->close();
		m_mapClient[iNumber]->release();
		if (isSydTestInfoMessage())
		{
			SydTestInfoMessage << "Session Terminate : " << iNumber << ModEndl;
		}
	}
	m_mapClient.erase(iNumber);
	m_mapDBName.erase(iNumber);
	delete m_mapCSForClient[iNumber];
	m_mapCSForClient.erase(iNumber);
}

//
//	FUNCTION private
//	SydTest::executeSQLCommand -- コマンドの発行
//
//	NOTES
//	SQL文パラメータを読み込んでコマンドを発行する
//	タプルが得られた場合は内容を表示する
//
//	ARGUMENT
//	Item* pItem
//		コマンドを表すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::executeSQLCommand(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	if (pNext == 0)
	{
		SydTestErrorMessage << "'Command'/'Rowset' Needs a SQL String." << ModEndl;
		throw SydTestException();
	}

	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && !m_mapClient.exists(iNumber))
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not Initialized." << ModEndl;
		throw SydTestException();
	}

	// コマンドは引数としてSQL文である文字列を取らなければならない
	String* pSQLString = testStringPointer(pNext, ModUnicodeString("No SQL Command String."));
	if (m_iOption & ExecuteOption::ShowCommand  && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
		SydTestInfoMessage << getSessionNumberString(iNumber)
				<< "[[SQL Query]] " << pSQLString->getString(code) << ModEndl;
	}

	// パラメータを取得する
	Common::DataArrayData* pSQLParameter = getSQLParameter(pSQLString);
	showParameter(pSQLParameter, iNumber);

	// 解析終了、ここから「本体」

	if (m_iOption & ExecuteOption::DebugMode) return;

	bool bPrepared = false;
	bool bSkipResult = false;
	if (m_iOption & ExecuteOption::Prepared) {
		const ModUnicodeString& cstrSQL = pSQLString->getUnicodeString();
		if (cstrSQL.compare(ModUnicodeString("select"), ModFalse, 6) == 0
			|| cstrSQL.compare(ModUnicodeString("values"), ModFalse, 6) == 0
			|| cstrSQL.compare(ModUnicodeString("declare"), ModFalse, 7) == 0) {
			// use prepared
			bPrepared = true;

		} else if (cstrSQL.compare(ModUnicodeString("insert"), ModFalse, 6) == 0
				   || cstrSQL.compare(ModUnicodeString("update"), ModFalse, 6) == 0
				   || cstrSQL.compare(ModUnicodeString("delete"), ModFalse, 6) == 0) {
			// use prepared
			bPrepared = true;
			// skip result
			bSkipResult = true;
		}
	}
	_setResultCount(iNumber, 0);

	Client2::ResultSet* pSQLCommand = 0;
	Client2::PrepareStatement* pPrepareStatement = 0;

	Common::DataArrayData cParameter;
	if (pSQLParameter == 0)
		pSQLParameter = &cParameter;

	if (bPrepared) {
		try {
			pPrepareStatement = createPrepareSQLCommand(iNumber,
														pSQLString->getUnicodeString());
			pSQLCommand = createPreparedSQLCommand(iNumber,
												   *pPrepareStatement,
												   pSQLParameter);
		} catch (Exception::Object& err) {
			SydTestErrorMessage << getSessionNumberString(iNumber)
								<< err << ModEndl;
			; // pass through
		}
	} else {
		try {
			pSQLCommand = createSQLCommand(iNumber,
										   pSQLString->getUnicodeString(),
										   pSQLParameter);
		}catch(Exception::Object& err){
			SydTestErrorMessage << err << ModEndl;
			throw;
		}
	}
	
	if (pSQLCommand) {
		ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber]);
		cAuto.lock();
		getSQLResult(iNumber, pSQLCommand, bSkipResult);
	}
	if (pPrepareStatement) {
		pPrepareStatement->close();		
		pPrepareStatement->release();		
	}
}

//
//	FUNCTION private
//	  Executor::executeAsyncSQLCommand -- 非同期なSQLコマンドの発行
//
//	NOTES
//	  非同期なSQLコマンドを実行する
//
//	ARGUMENTS
//    Item* pItem_
//      コマンドを表すオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::executeAsyncSQLCommand(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	// ラベルを取得する
	String* pLabel = testStringPointer(pNext, ModUnicodeString("No Label."));
	ModUnicodeString cstrLabel = pLabel->getString();
	if (m_mapAsyncCommand.exists(cstrLabel)) {
		SydTestErrorMessage << "Label '" << cstrLabel << "' already exists." << ModEndl;
		throw SydTestException();
	}

	pNext = pNext->getNext();
	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && m_mapClient.exists(iNumber) != true)
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not Initialized." << ModEndl;
		throw SydTestException();
	}

	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber) << "[[Label]] "
						   << cstrLabel << ModEndl;
	}

	// コマンドは引数としてSQL文である文字列を取らなければならない
	String* pSQLString = testStringPointer(pNext, ModUnicodeString("No SQL Command String."));
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
		SydTestInfoMessage << getSessionNumberString(iNumber)
			<< "[[SQL Query]] " << pSQLString->getString(code) << ModEndl;
	}

	// パラメータを取得する
	Common::DataArrayData* pSQLParameter = getSQLParameter(pSQLString);
	showParameter(pSQLParameter, iNumber);

	if (m_iOption & ExecuteOption::DebugMode) return;

	bool bPrepared = false;
	bool bSkipResult = false;
	if (m_iOption & ExecuteOption::Prepared) {
		const ModUnicodeString& cstrSQL = pSQLString->getUnicodeString();
		if (cstrSQL.compare(ModUnicodeString("select"), ModFalse, 6) == 0
			|| cstrSQL.compare(ModUnicodeString("values"), ModFalse, 6) == 0) {
			// use prepared
			bPrepared = true;

		} else if (cstrSQL.compare(ModUnicodeString("insert"), ModFalse, 6) == 0
				   || cstrSQL.compare(ModUnicodeString("update"), ModFalse, 6) == 0
				   || cstrSQL.compare(ModUnicodeString("delete"), ModFalse, 6) == 0) {
			// use prepared
			bPrepared = true;
			// skip result
			bSkipResult = true;
		}
	}
	_setResultCount(iNumber, 0);

	Client2::ResultSet* pSQLCommand;
	Client2::PrepareStatement* pPrepareStatement = 0;

	Common::DataArrayData cParameter;
	if (pSQLParameter == 0)
		pSQLParameter = &cParameter;

	if (bPrepared) {
		pPrepareStatement = createPrepareSQLCommand(iNumber,
													pSQLString->getUnicodeString());
		pSQLCommand = createPreparedSQLCommand(iNumber,
											   *pPrepareStatement,
											   pSQLParameter);
	} else {
		try {
			pSQLCommand = createSQLCommand
				(iNumber, pSQLString->getUnicodeString(), pSQLParameter);
		}catch(Exception::Object& err){
			SydTestErrorMessage << err << ModEndl;
			throw;
		}
	}
	m_mapAsyncCommand.insert(cstrLabel, AsyncCommandPair(iNumber, pSQLCommand,
														 bSkipResult, pPrepareStatement));
}

//
//	FUNCTION private
//	Executor::getAsyncSQLResult -- 非同期なコマンドの結果を取得する
//
//	NOTES
//	非同期なコマンドの結果を取得する
//
//	ARGUMENTS
//    Item* pItem_
//      コマンドを表すオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::getAsyncSQLResult(Item* pItem_)
{
	int iNumber = -1;

	// ラベルを取得する
	String* pLabel = testStringPointer
		(pItem_->getNext(), ModUnicodeString("No Label."));

	ModUnicodeString cstrLabel = pLabel->getString();
	if (!(m_mapAsyncCommand.exists(cstrLabel))) {
		SydTestErrorMessage << "Label '" << cstrLabel << "' does not exist." << ModEndl;
		throw SydTestException(SydTestException::TagNotFound);
	}

	iNumber = m_mapAsyncCommand[cstrLabel].first;
	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber)
						   << "[[Label]] " << cstrLabel << ModEndl;
	}
	
	{
		ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber]);
		cAuto.lock();
		getSQLResult(iNumber, m_mapAsyncCommand[cstrLabel].second, m_mapAsyncCommand[cstrLabel].m_bSkipResult);
	}
	if (m_mapAsyncCommand[cstrLabel].m_pPrepareStatement) {
		m_mapAsyncCommand[cstrLabel].m_pPrepareStatement->close();
		m_mapAsyncCommand[cstrLabel].m_pPrepareStatement->release();
	}
	m_mapAsyncCommand.erase(cstrLabel);
}

//
//	FUNCTION private
//	  Executor::cancelAsyncSQLCommand -- 非同期なコマンドをキャンセルする
//
//	NOTES
//	  非同期なコマンドをキャンセルする
//
//	ARGUMENTS
//    Item* pItem_
//      コマンドを表すオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::cancelAsyncSQLCommand(Item* pItem_)
{
	// ラベルを取得する
	String* pLabel = testStringPointer(pItem_->getNext(), ModUnicodeString("No Label."));

	ModUnicodeString cstrLabel = pLabel->getString();
	if (!(m_mapAsyncCommand.exists(cstrLabel))) {
		SydTestErrorMessage << "Label '" << cstrLabel << "' does not exist." << ModEndl;
		throw SydTestException(SydTestException::TagNotFound);
	}

	int iNumber = m_mapAsyncCommand[cstrLabel].first;
	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber) << "[[Label]] " << cstrLabel << ModEndl;
	}
	ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber]);
	cAuto.lock();
	m_mapAsyncCommand[cstrLabel].second->cancel();
}

//
//	FUNCTION private
//	SydTest::prepareSQLCommand -- 最適化済コマンドの作成
//
//	NOTES
//	「?」を含むSQL文を読み込んで最適化済コマンドを作成する
//
//	ARGUMENT
//	Item* pItem
//		コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::prepareSQLCommand(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	if (pNext == 0)
	{
		SydTestErrorMessage << "'CreatePreparedCommand' Needs a SQL String." << ModEndl;
		throw SydTestException();
	}

	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && !m_mapClient.exists(iNumber))
	{
		SydTestErrorMessage << "Session #" << iNumber << " has not been initialized yet." << ModEndl;
		throw SydTestException();
	}

	// ラベルを取得する
	String* pLabel = testStringPointer(pNext, ModUnicodeString("No Label."));
	ModUnicodeString cstrLabel = pLabel->getString();
	pNext = pNext->getNext();

	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber)
			<< "[[Label]] " << cstrLabel << ModEndl;
	}

	// SQL文である文字列の取得
	String* pSQLString = testStringPointer(pNext, ModUnicodeString("No SQL Command String."));
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
		SydTestInfoMessage << getSessionNumberString(iNumber)
				<< "[[SQL Query]] " << pSQLString->getString(code) << ModEndl;
	}
	
	if (m_mapPreparedCommand.exists(cstrLabel)) {
		SydTestErrorMessage << "Label '" << cstrLabel << "' already exists." << ModEndl;
		throw SydTestException();
	}

	bool bSkipResult = false;
	if (m_iOption & ExecuteOption::Prepared) {
		const ModUnicodeString& cstrSQL = pSQLString->getUnicodeString();
		if (cstrSQL.compare(ModUnicodeString("insert"), ModFalse, 6) == 0
			|| cstrSQL.compare(ModUnicodeString("update"), ModFalse, 6) == 0
			|| cstrSQL.compare(ModUnicodeString("delete"), ModFalse, 6) == 0) {
			// skip result
			bSkipResult = true;
		}
	}

	//Client2::PrepareStatement* pPrepareStatement = _pDataSource->createPrepareStatement
	//	(m_mapDBName[iNumber], pSQLString->getUnicodeString());
	Client2::PrepareStatement*	pPrepareStatement = m_mapClient[iNumber]->createPrepareStatement(pSQLString->getUnicodeString());
	m_mapPreparedCommand.insert(pLabel->getString(), PreparedCommandPair(iNumber, pPrepareStatement, bSkipResult));
}

//
//	FUNCTION private
//	SydTest::executePreparedSQLCommand -- 最適化済コマンドの実行
//
//	NOTES
//	  最適化済コマンドにパラメータを補って実行する
//
//	ARGUMENT
//	  Item* pItem
//		コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::executePreparedSQLCommand(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	// ラベルを取得する
	String* pLabel = testStringPointer(pNext, ModUnicodeString("No Label for a prepared command."));
	ModUnicodeString cstrLabel = pLabel->getString();
	if (!(m_mapPreparedCommand.exists(cstrLabel))) {
		SydTestErrorMessage << "Label '" << cstrLabel << "' for a prepared command does not exist." << ModEndl;
		throw SydTestException(SydTestException::TagNotFound);
	}		

	int iNumber = m_mapPreparedCommand[cstrLabel].first;
	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber)
						   << "[[Label]] " << cstrLabel << ModEndl;
	}
	
	// パラメータを取得する
	Common::DataArrayData* pSQLParameter = getSQLParameter(pLabel);
	showParameter(pSQLParameter, iNumber);

	if (m_iOption & ExecuteOption::DebugMode) return;

	// パラメータがない時はアドレスを適当に設定する
	// OutputArchive::writeObjectでsegvが発生するから
	Common::DataArrayData cParameter;
	if (pSQLParameter == 0) pSQLParameter = &cParameter;

	//Client2::ResultSet* pSQLCommand = createPreparedSQLCommand
	//	(iNumber, *(m_mapPreparedCommand[cstrLabel].second), pSQLParameter);
	Client2::ResultSet*	pSQLCommand = m_mapClient[iNumber]->executePrepareStatement(*(m_mapPreparedCommand[cstrLabel].second), *pSQLParameter);
	
	ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber]);
	cAuto.lock();
	getSQLResult(iNumber, pSQLCommand, m_mapPreparedCommand[cstrLabel].m_bSkipResult);
}

//
//	FUNCTION private
//	SydTest::executeAsyncPreparedSQLCommand -- 最適化済コマンドを非同期に実行
//
//	NOTES
//	最適化済コマンドにパラメータを補って非同期に実行する
//
//	ARGUMENT
//	Item* pItem
//		コマンドを表すオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::executeAsyncPreparedSQLCommand(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	// Asyncコマンド用のラベルを取得する
	String* pLabel = testStringPointer(pNext, ModUnicodeString("No Label for an async command."));
	ModUnicodeString cstrAsyncLabel = pLabel->getString();
	if (m_mapAsyncCommand.exists(cstrAsyncLabel)) {
		SydTestErrorMessage << "Label '" << cstrAsyncLabel
							<< "' for an async command already exists." << ModEndl;
		throw SydTestException();
	}		

	// PreparedCommandのラベルを取得する
	pNext = pNext->getNext();
	pLabel = testStringPointer(pNext, ModUnicodeString("No Label for a prepared command."));
	ModUnicodeString cstrPrepLabel = pLabel->getString();
	if (!(m_mapPreparedCommand.exists(cstrPrepLabel))) {
		SydTestErrorMessage << "Label '" << cstrPrepLabel << "' for a prepared command does not exist." << ModEndl;
		throw SydTestException(SydTestException::TagNotFound);
	}		

	int iNumber = m_mapPreparedCommand[cstrPrepLabel].first;
	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber)
						   << "[[AsyncLabel]] " << cstrAsyncLabel << ModEndl;
		SydTestInfoMessage << getSessionNumberString(iNumber)
						   << "[[PrepLabel]] " << cstrPrepLabel << ModEndl;
	}
	
	// パラメータを取得する
	Common::DataArrayData* pSQLParameter = getSQLParameter(pLabel);
	showParameter(pSQLParameter, iNumber);

	if (m_iOption & ExecuteOption::DebugMode) return;

	Client2::ResultSet* pSQLCommand = createPreparedSQLCommand
		(iNumber, *(m_mapPreparedCommand[cstrPrepLabel].second), pSQLParameter);
	m_mapAsyncCommand.insert(cstrAsyncLabel, AsyncCommandPair(iNumber, pSQLCommand,
															  m_mapPreparedCommand[cstrPrepLabel].m_bSkipResult));
}

//
//	FUNCTION private
//	SydTest::erasePrepareSQLCommand -- 最適化済コマンドの抹消
//
//	NOTES
//	最適化済コマンドを抹消する
//
//	ARGUMENT
//	Item* pItem
//		コマンドを表すオブジェクト
//
//	RETURN
//	なし
//
void
Executor::erasePreparedSQLCommand(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	// ラベルを取得する
	String* pLabel = testStringPointer(pNext, ModUnicodeString("No Label."));

	ModUnicodeString cstrLabel = pLabel->getString();
	if (!(m_mapPreparedCommand.exists(cstrLabel))) {
		SydTestErrorMessage << "Label '" << cstrLabel << "' for a prepared command does not exist." << ModEndl;
		throw SydTestException(SydTestException::TagNotFound);
	}		

	int iNumber = m_mapPreparedCommand[cstrLabel].first;

	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << getSessionNumberString(iNumber) << "[[Label]] " << cstrLabel << ModEndl;
	}

	// 自らを抹消する
	m_mapPreparedCommand[cstrLabel].second->close();
	m_mapPreparedCommand[cstrLabel].second->release();
	// mapからも消す
	m_mapPreparedCommand.erase(cstrLabel);
}


//
//	FUNCTION private
//  Executor::createSQLCommand -- SQL文を表すオブジェクトを生成する
//	
//	NOTES
//	  SQL文を表すオブジェクトを生成する
//
//	ARGUMENTS
//    int iNumber_
//      セッション番号
//    const ModUnicodeString& rstrCommand_
//      SQL文を表す文字列
//    Common::DataArrayData* pParameter_
//      SQL文のパラメーター
//
//	RETURN
//    Client2::ResultSet*
//      SQL文を表すオブジェクト
//
//	EXCEPTIONS
//    Exception::Object& 
//      下位ルーチンからの持ち上がり。そのままthrowしている。
//
Client2::ResultSet*
Executor::createSQLCommand(int iNumber_,
						   const ModUnicodeString& rstrCommand_, 
						   Common::DataArrayData* pParameter_)
{
	Client2::ResultSet* pResultSet = 0;
	while(true) {
		try{
			ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber_]);
			cAuto.lock();
			pResultSet = m_mapClient[iNumber_]->executeStatement(rstrCommand_, *pParameter_);
			break;
		} catch (Exception::SessionBusy&) {
			// しばらく待ってからretry
			ModThisThread::sleep(rand()%1000);
			continue;
		} catch (Exception::Object&) {
			throw;
		}
	}
	return pResultSet;
}

// FUNCTION public
//	SydTest::Executor::createPrepareSQLCommand -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	const ModUnicodeString& rstrCommand_
//	Common::DataArrayData* pParameter_
//	
// RETURN
//	Client2::PrepareStatement*
//
// EXCEPTIONS

Client2::PrepareStatement*
Executor::
createPrepareSQLCommand(int iNumber_,
						const ModUnicodeString& rstrCommand_)
{
	Client2::PrepareStatement*	pPrepareStatement = 0;
	while(true) {
		try{
			ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber_]);
			cAuto.lock();
			pPrepareStatement = m_mapClient[iNumber_]->createPrepareStatement(rstrCommand_);
			break;
		} catch (Exception::SessionBusy&) {
			// しばらく待ってからretry
			ModThisThread::sleep(rand()%1000);
			continue;
		} catch (Exception::Object&) {
			throw;
		}
	}
	return pPrepareStatement;
}

//
//	FUNCTION private
//  Executor::createPreparedSQLCommand -- 最適化済のSQL文のパラメータを埋める
//	
//	NOTES
//	  最適化済のSQL文とパラメータから完全なSQL文を表すオブジェクトを生成する
//
//	ARGUMENTS
//    int iNumber_
//      セッション番号
//    const Client2::PrepareStatement& rPrep_,
//      最適化済SQL文を表すオブジェクト
//    Common::DataArrayData* pParameter_
//      SQL文のパラメーター
//
//	RETURN
//    Client2::ResultSet*
//      最適化済のSQL文を表すオブジェクト
//
//	EXCEPTIONS
//    Exception::Object& 
//      下位ルーチンからの持ち上がり。そのままthrowしている。
//
Client2::ResultSet*
Executor::createPreparedSQLCommand(int iNumber_,
								   const Client2::PrepareStatement& rPrep_,
								   Common::DataArrayData* pParameter_)
{
	Client2::ResultSet* pResultSet = 0;
	while(true) {
		try{
			ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber_]);
			cAuto.lock();
			pResultSet = m_mapClient[iNumber_]->executePrepareStatement(rPrep_, *pParameter_);
			break;
		} catch (Exception::SessionBusy&) {
			// しばらく待ってからretry
			ModThisThread::sleep(rand()%1000);
			continue;
		} catch (Exception::Object&) {
			throw;
		}
	}
	return pResultSet;
}

//
//	FUNCTION private
//  Executor::getSQLResult -- SQL文の結果を取得する
//	
//	NOTES
//	  SQL文の結果を取得する
//
//	ARGUMENTS
//    int iNumber_
//      セッション番号
//    Client2::ResultSet* pSQLCommand_
//      SQL文を表すオブジェクト
//    bool bSkipResult_ = false
//      trueのときResultSetの中身を出力しない
//      
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void
Executor::getSQLResult(int iNumber_, Client2::ResultSet* pSQLCommand_, bool bSkipResult_)
{
	int resultCount = 0;
	int retryCount = 0;
	bool bExplain = false;

	while (true)
	{
		try
		{
			ModAutoPointer<Common::DataArrayData>	pTuple = new Common::DataArrayData();
			Client2::ResultSet::Status::Value eStatus;
			
			eStatus = pSQLCommand_->getNextTuple(pTuple.get());
			if (eStatus == Client2::ResultSet::Status::Data) 
			{
				if ((bExplain || !bSkipResult_) && isSydTestInfoMessage())
				{
					ModUnicodeString cstrRow = 
						Common::SystemParameter::getBoolean("SydTest_ShowBinary")
						? tupleToString(pTuple.get()) : pTuple->toString();
					if (bExplain || cstrRow.getLength() <= getThreshold())
					{
						SydTestInfoMessage << getSessionNumberString(iNumber_)
							<< cstrRow << ModEndl;
					}
					else						
					{
						SydTestInfoMessage  << getSessionNumberString(iNumber_)
							<< "<length = " << cstrRow.getLength() << ">"<< ModEndl;
					}
				}
				//pSQLCommand_->releaseTuple(pTuple);
				if (!bExplain) ++resultCount;
			}
			else if (eStatus == Client2::ResultSet::Status::EndOfData) {
				if ((bExplain || !bSkipResult_) && isSydTestInfoMessage()) {
					SydTestInfoMessage  << getSessionNumberString(iNumber_)
										<< "End Of Data." << ModEndl;
				}
				bExplain = false;
			}
			else if (eStatus == Client2::ResultSet::Status::Success)
			{
				if (isSydTestInfoMessage())
				{
					SydTestInfoMessage  << getSessionNumberString(iNumber_)
						<< "Success." << ModEndl;
				}
				break;
			}
			else if (eStatus == Client2::ResultSet::Status::Canceled)
			{
				if (isSydTestInfoMessage())
				{
					SydTestInfoMessage  << getSessionNumberString(iNumber_)
						<< "Canceled." << ModEndl;
				}
				break;
			}
			else if (eStatus == Client2::ResultSet::Status::MetaData)
			{
				//メタデータ
				ModUnicodeString cstrMeta = pSQLCommand_->getMetaData()->getString();
				if (cstrMeta.compare("{Plan}") == 0) {
					bExplain = true;
				} else {
					bExplain = false;
					resultCount = 0;
				}
			}
		}
		catch (Exception::SessionBusy&)
		{
			// しばらく待ってからretry
			ModThisThread::sleep(rand()%1000);
			if (isSydTestInfoMessage())
			{
				SydTestInfoMessage  << "retry #" << ++retryCount << ModEndl;
			}
			continue;
		}
		catch (Exception::ConnectionRanOut&)
		{
			if (isSydTestInfoMessage())
			{
				SydTestInfoMessage  << getSessionNumberString(iNumber_)
								<< "Connection Ran Out." << ModEndl;
			}
			break;
		}
		catch (Exception::Object& err)
		{
			SydTestErrorMessage  << getSessionNumberString(iNumber_)
								 << err << ModEndl;
			break;
		}
	}
	_setResultCount(iNumber_, resultCount);
	
	pSQLCommand_->close();
	pSQLCommand_->release();
}

//
//	FUNCTION private
//  Executor::getSQLParameter -- SQLパラメータを得る
//
//	NOTES
//    SQLパラメータを得る
//
//	ARGUMENTS
//	  Item* pSQLString_
//      パラメータを表す文字列を含むコマンド要素
//
//	RETURN
//    Common::DataArrayData* 
//      パラメータ
//
//	EXCEPTIONS
//    SydTestException
//
Common::DataArrayData* 
Executor::getSQLParameter(Item* pSQLString_)
{
	Common::DataArrayData* pResult = 0;
	Item* pNext = pSQLString_->getNext();

	if (pNext == 0) return 0;

	if (pNext->getType() != Item::Type::Parameter)
	{
		SydTestErrorMessage << "Wrong parameter type." << ModEndl;
			throw SydTestException();
	}

	pResult = (dynamic_cast<Parameter*>(pNext))->getParameter();
	return pResult;
}

//
//	FUNCTION private
//  Executor::showParameter
//
//	NOTES
//    SQLパラメータを表示する
//
//	ARGUMENTS
//	  Common::DataArrayData* pSQLParameter_
//      パラメータを表すオブジェクト
//	  int iNumber_
//      セッション番号
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void
Executor::showParameter(Common::DataArrayData* pSQLParameter_, int iNumber_)
{
	if (pSQLParameter_ != 0 && (m_iOption & ExecuteOption::ShowParameter) && isSydTestInfoMessage())
	{
		ModUnicodeString cstrPrm = pSQLParameter_->getString();
		if (cstrPrm.getLength() <= getThreshold())
		{				
			SydTestInfoMessage << getSessionNumberString(iNumber_)
				<< "[[SQL Parameter]] " << cstrPrm << ModEndl;
		} else {
			SydTestInfoMessage << getSessionNumberString(iNumber_)
				<< "[[SQL Parameter: length = "
				<< cstrPrm.getLength() << "]] " <<  ModEndl;
		}
	}
}

//
//	FUNCTION private
//	Executor::sleep -- 休止する
//
//	NOTES
//	引数にある時間だけ休止する
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::sleep(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	int iSpan = testNumberPointer(pNext, ModUnicodeString("Not a number."))->getNumber();

	if (m_iOption & ExecuteOption::ShowParameter && isSydTestTimeMessage())
	{
		SydTestTimeMessage << "[Sleep Time] " << iSpan << ModEndl;
	}

	ModThisThread::sleep(iSpan);
}

//
//	FUNCTION private
//	Executor::scaledSleep -- 休止する
//
//	NOTES
//	(引数 * SydTest_SleepingRate / 100) ミリ秒だけ休止する
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::scaledSleep(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	int iSpan = testNumberPointer(pNext, ModUnicodeString("Not a number."))->getNumber()
		* m_iSleepingRate / 100;

	if (m_iOption & ExecuteOption::ShowParameter && isSydTestTimeMessage())
	{
		SydTestTimeMessage << "[Sleep Time] " << iSpan << ModEndl;
	}

	ModThisThread::sleep(iSpan);
}

//
//	FUNCTION private
//	SydTest::Executor::displayCurrentTime -- 現在時刻の表示
//
//	NOTES
//  現在の時刻を表示する
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
void
Executor::displayCurrentTime()
{
	if (isSydTestTimeMessage())
	{
		SydTestTimeMessage << "[TIME] " << ModTime::getCurrentTime().getString() << ModEndl;
	}
}

//
//	FUNCTION private
//	SydTest::Executor::beginTimespan -- 時間測定の起点設定
//
//	NOTES
//  ある区間の時間を計るための起点を設定する
//
//	ARGUMENTS
//	Item* pItem_
//    引数を含むオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::beginTimespan(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	const char* str;

	// 時間ラベルの決定
	if (pNext == 0) {
		str = "DefaultTimeSpan";
	} else if (pNext->getType() == Item::Type::String) {
		str = (dynamic_cast<String*>(pNext))->getString();
	} else {
		SydTestErrorMessage << "Not a string." << ModEndl;
		throw SydTestException();
	}

	if (m_iOption & ExecuteOption::ShowCommand && isSydTestTimeMessage())
	{
		SydTestTimeMessage << "[Time Option] " << str << ModEndl;
	}

	// 計時を開始する
	if (!m_stopWatch.start(str))
	{
		SydTestErrorMessage << "Timespan label "
			<< str << " already exists." << ModEndl;
		throw SydTestException();
	}
}

//
//	FUNCTION private
//	SydTest::Executor::endTimespan -- 時間測定の終点設定
//
//	NOTES
//  ある区間の時間を計るための終点を設定し、測定結果を表示する
//	上限値より値が大きい場合はエラーメッセージで出力する。
//
//	ARGUMENTS
//	Item* pItem_
//    引数を含むオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::endTimespan(Item* pItem_)
{
	Item* pNext = pItem_->getNext();
	const char* str;
	ModTimeSpan upper;
	
	// 時間ラベルの決定
	if (pNext == 0 || pNext->getType() == Item::Type::Number) {
		str = "DefaultTimeSpan";
	} else if (pNext->getType() == Item::Type::String) {
		str = (dynamic_cast<String*>(pNext))->getString();
		pNext = pNext->getNext();
	} else {
		SydTestErrorMessage << "Not string." << ModEndl;
		throw SydTestException();
	}

	// TimeSpanの上限値の決定
	if (pNext == 0) {
		upper = ModTimeSpan(); // 0 millsecond
	} else if (pNext->getType() == Item::Type::Number) {
		const int num = (dynamic_cast<Number*>(pNext))->getNumber();
		upper = ModTimeSpan(num/1000, num%1000);
	} else {
		SydTestErrorMessage << "Not Number." << ModEndl;
		throw SydTestException();
	}

	// オプションの表示
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestTimeMessage())
	{
		SydTestTimeMessage << "[Time Option] " << str << ModEndl;

		// 上限値は追加機能なので、デフォルトの場合は出力しない。
		if (upper.getTotalMilliSeconds() != 0)
		{
			SydTestTimeMessage << "[Time Option] UpperTimeSpan: "
							   << upper.getTotalSeconds() << "."
							   << ModIosSetW(3) << ModIosSetFill('0')
							   << upper.getMilliSeconds() << "s"
							   << ModEndl;
		}
	}
	
	// 計測
	if (m_stopWatch.stop(str)) {
		ModTimeSpan cTimeSpan = m_stopWatch.showTotalTime(str);
		if (isSydTestTimeMessage())
		{
			SydTestTimeMessage << "[TIME] TimeSpan: "
							   << cTimeSpan.getTotalSeconds() << "."
							   << ModIosSetW(3) << ModIosSetFill('0')
							   << cTimeSpan.getMilliSeconds() << "s"
							   << ModEndl;
		}
		m_stopWatch.clear(str);

		// 上限値と比較。デフォルトの場合は比較しない。
		if (upper.getTotalMilliSeconds() != 0 && cTimeSpan > upper)
		{
			SydTestErrorMessage << "It took longer than UpperTimeSpan."
								<< ModEndl;
		}
	} else {
		SydTestErrorMessage << "No such timespan label "
			<< str << "." << ModEndl;
		throw SydTestException();
	}
}

//
//	FUNCTION private
//	SydTest::Executor::fileInclude
//
//	NOTES
//  別ファイルに記されたスクリプトを起動する。
//	Begin - End 間、および { - } 間に呼ばれた場合の動作は保証しない。
//
//	ARGUMENTS
//	Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::fileInclude(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "Missing File Name." << ModEndl;
		throw SydTestException(SydTestException::FileNotFound);
	}
	const char* str = (dynamic_cast<String*>(pNext))->getString();
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "Include " << str << ModEndl;
	}
	if (ModOsDriver::File::isNotFound(str)) 
	{
		SydTestErrorMessage << "No such file `" << str << "'." << ModEndl;
	}
	else 
	{
		Executor cExecutor(str, str, m_iOption, m_rMonitor, 
						   m_cstrHostName, m_iPortNumber, m_cstrRegPath, m_cascadeInfoVector);
		cExecutor.executeMain();
	}
}

//
//	FUNCTION private
//	SydTest::Executor::createThread -- Threadの作成
//
//	NOTES
//  一つのスレッドを起こし、スレッドごとの処理を実行する
//
//	ARGUMENTS
//	Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::createThread(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "Missing Thread Name." << ModEndl;
		throw SydTestException();
	}
	const char* str = (dynamic_cast<String*>(pNext))->getString();
	Thread::createStatic(m_strFileName.getString(), str, m_iOption, m_rMonitor, 
						 m_cstrHostName, m_iPortNumber, m_cstrRegPath, m_cascadeInfoVector,
						 m_cryptMode, m_cstrUserName, m_cstrPassword);
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Option] " << str << ModEndl;
	}
}

//
//	FUNCTION private
//	SydTest::Executor::joinThread -- Threadの終了
//
//	NOTES
//  スレッドを終了する
//
//	ARGUMENTS
//	Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::joinThread(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "No Thread Name." << ModEndl;
		throw SydTestException();
	}
	const char* str = (dynamic_cast<String*>(pNext))->getString();
	Thread::joinStatic(str);
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Option] " << str << ModEndl;
	}
}

//
//	FUNCTION private
//	SydTest::Executor::exists -- パスの存在確認
//
//	NOTES
//    パスの存在を確認する。なければ作る。
//
//	ARGUMENTS
//	Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::exists(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();

	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "No Path Name." << ModEndl;
		throw SydTestException();
	}

	const char* str = (dynamic_cast<String*>(pNext))->getString();

	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Option] " << str << ModEndl;
	}

	if (!(m_iOption & ExecuteOption::DebugMode))
	{
		try
		{
			// isDirectoryの返り値はMod[True|False]
			bool bResult = ModOsDriver::File::isNotFound(str) ? false: true;
			if (bResult && isSydTestInfoMessage()) {
				SydTestInfoMessage << "Success." << ModEndl;
			} else {
				if (m_iOption & ExecuteOption::AbortWhenMissingPath) {
					SydTestErrorMessage << "Directory " << str 
						<< " does not exist." << ModEndl;
					SydAssert(false);
					throw SydTestException(SydTestException::Requested);
				} else {
					SydTestErrorMessage << "Directory " << str 
						<< " has not created." << ModEndl;
					ModOsDriver::File::mkdir(str, ModOs::ownerModeMask, ModTrue);
					SydTestErrorMessage << "So created it." << ModEndl;
				}
			}
		}
		catch (ModException& err)
		{
			SydTestErrorMessage << "ModException occurred." << ModEndl;
			ModErrorMessage << err << ModEndl;
		}
		catch (Exception::Object& err)
		{
			SydTestErrorMessage << err << ModEndl;
		}
	}
}

//
//	FUNCTION private
//	SydTest::Executor::notExists -- パスの不在確認
//
//	NOTES
//    パスの不在を確認する。あれば消す(rmdirと同様の動作)。
//
//	ARGUMENTS
//	Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//	  なし
//
//	EXCEPTIONS
//	  SydTestException
//
void
Executor::notExists(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();

	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "No Path Name." << ModEndl;
		throw SydTestException();
	}

	const char* str = (dynamic_cast<String*>(pNext))->getString();

	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Option] " << str << ModEndl;
	}

	if (!(m_iOption & ExecuteOption::DebugMode))
	{
		try
		{
			// isDirectoryの返り値はMod[True|False]
			bool bResult = ModOsDriver::File::isNotFound(str) ? false: true;
			if (bResult) {
				SydTestErrorMessage << "Directory " << str 
					<< " remains." << ModEndl;
				if (m_iOption & ExecuteOption::AbortWhenMissingPath)
				{
					SydAssert(false);
					throw SydTestException(SydTestException::Requested);
				}
				// ディレクトリのみ消そうとする
				try {
					ModOsDriver::File::rmdir(str);
					SydTestErrorMessage << "So removed it." << ModEndl;
				} catch(ModException& err) {
					if (err.getErrorNumber() == ModOsErrorNotEmpty) {
						SydTestErrorMessage << "Could not remove " << str 
							<< " because of remaining file(s)." << ModEndl;
					} else {
						throw;
					}
				}
			} else if (isSydTestInfoMessage()){
				SydTestInfoMessage << "Success." << ModEndl;
			}
		}
		catch (ModException& err)
		{
			SydTestErrorMessage << "ModException occurred." << ModEndl;
			ModErrorMessage << err << ModEndl;
		}
		catch (Exception::Object& err)
		{
			SydTestErrorMessage << err << ModEndl;
		}
	}
}

//
//	FUNCTION private 
//	Executor::doOsCommand -- OSのコマンドラインを呼び出す
//
//	NOTES
//	  OSのコマンドラインを呼び出す
//
//	ARGUMENTS
//    Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void 
Executor::doOsCommand(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	String* pString = testStringPointer
		(pNext, ModUnicodeString("No Command."));

	std::string tmpString = pString->getString();
#ifdef SYD_OS_POSIX
	if (commandReplace(tmpString,
					   std::string("../../restore.bat"),
					   std::string("../../restore.sh")) == false)
	if (commandReplace(tmpString,
					   std::string("../../switchcopy.bat"),
					   std::string("cp -rp")) == false)
	if (commandReplace(tmpString,
					   std::string("move /Y"),
					   std::string("mv -f")) == false)
	if (commandReplace(tmpString,
					   std::string("dir "),
					   std::string("df ")) == false)
	commandReplace(tmpString,
				   std::string("md "),
				   std::string("mkdir "));
#endif
	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[System Parameter] " 
			<< tmpString.data() << ModEndl;
	}			
	system(tmpString.data());
}


// 直前の実行結果を確認する
void
Executor::
assureCount(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (!pNext || pNext->getType() != Item::Type::Number) {
		SydTestErrorMessage << "AssureCount [Session] <Count>" << ModEndl;
		throw SydTestException();
	}
	Number* pNumber = testNumberPointer
		(pNext, ModUnicodeString("AssureCount argument must be a number."));
	int iNumber = pNumber->getNumber();
	pNext = pNumber->getNext();

	int iSession = 0;
	if (pNext) {
		if (pNext->getType() != Item::Type::Number) {
			SydTestErrorMessage << "AssureCount [Session] <Count>" << ModEndl;
			throw SydTestException();
		}
		iSession = iNumber;
		pNumber = testNumberPointer
			(pNext, ModUnicodeString("AssureCount argument must be a number."));
		iNumber = pNumber->getNumber();
	}

	int iResult = _getResultCount(iSession);
	if (iResult != iNumber) {
		SydTestErrorMessage << "AssureCount failed. result = " << iResult << " expected = " << iNumber << ModEndl;
		SydAssert(false);
	}
}

// Encodingを変更する
void
Executor::setEncodingType(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "No Operand." << ModEndl;
		throw SydTestException();
	}

	const char* str = (dynamic_cast<String*>(pNext))->getString();
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Option] " << str << ModEndl;
	}

	if (!(m_iOption & ExecuteOption::DebugMode))
	{
		setEncodingType_sub(str);
	}
}

// スレッドどうしの同期を行う
void
Executor::synchronize(Command* pCommand_)
{
	// 第一引数(mandatory)の取得
	Item* pNext = pCommand_->getNext();
	if (pNext == 0 || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "No Operand." << ModEndl;
		throw SydTestException();
	}
	ModUnicodeString tag = (dynamic_cast<String*>(pNext))->getString();
	if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Option] " << tag << ModEndl;
	}

	// 第二引数(optional)の取得
	int waitCount = 0;
	pNext = pNext->getNext();
	if (pNext == 0) {
		waitCount = 2;
	}
	else if (pNext->getType() == Item::Type::Number)
	{
		waitCount = (dynamic_cast<Number*>(pNext))->getNumber();
		if (waitCount < 2) {
			SydTestErrorMessage << "Bad Argument." << ModEndl;
			throw SydTestException();
		}
	}
	else 
	{
		SydTestErrorMessage << "Bad Argument." << ModEndl;
		throw SydTestException();
	}

	// 処理開始
	static ModCriticalSection cs;
	ModAutoMutex<ModCriticalSection> m(&cs);
	m.lock();

	if (!m_mapConditionVariable.exists(tag)){
		// 最初に到達したスレッドならば新しい条件変数を作成
		// 第一引数の「(ModTrue)」は複数の待ちスレッドを同時に起こすために必要
		// 第二引数の「(ModTrue)」はsignal状態を保持し続けるために必要 #1470
		ModConditionVariable* pCV = new ModConditionVariable(ModTrue, ModTrue); 
		m_mapConditionVariable.insert(tag, SyncPair(0, pCV));
	}

	if (++(m_mapConditionVariable[tag].first) < waitCount) {
		// 最初または途中に到達したスレッドである
		m.unlock();
		if (isSydTestDebugMessage())
		{
			SydTestDebugMessage << m_strLabel << "/" << tag << ":"
								<< m_mapConditionVariable[tag].first << " (wait)" << ModEndl;
		}
		m_mapConditionVariable[tag].second->wait();
		if (isSydTestDebugMessage())
		{
			SydTestDebugMessage << m_strLabel << "/" << tag << ": signaled." << ModEndl;
		}
		m.lock();
		if (--(m_mapConditionVariable[tag].first) == 1) {
			// 条件変数の破棄
			delete m_mapConditionVariable[tag].second;
			m_mapConditionVariable.erase(tag);
			if (isSydTestDebugMessage())
			{
				SydTestDebugMessage << m_strLabel << "/" << tag << ": deleted." << ModEndl;
			}
		}
	} else {
		// 最後に到達したスレッドである
		if (isSydTestDebugMessage())
		{
			SydTestDebugMessage << m_strLabel << "/" << tag << ":"
							<< m_mapConditionVariable[tag].first << " (signal)" << ModEndl;
		}
		m_mapConditionVariable[tag].second->signal();
	}		
}

// isServerAvailable
void
Executor::isServerAvailable(Command* pCommand_)
{
	// 使用できるデータソースを使用する
	Client2::DataSource* pDataSource;
	Map<int, Client2::DataSource*>::iterator i = m_mapDataSource.begin();
	if (i != m_mapDataSource.end()) {
		pDataSource = (*i).second;
	}

	bool result = pDataSource->isServerAvailable();
	if (isSydTestInfoMessage())
	{
		SydTestInfoMessage << (result ? "true" : "false") << ModEndl;
	}
}

// isDatabaseAvailable
void
Executor::isDatabaseAvailable(Command* pCommand_)
{
	// 使用できるデータソースを使用する
	Client2::DataSource* pDataSource;
	Map<int, Client2::DataSource*>::iterator i = m_mapDataSource.begin();
	if (i != m_mapDataSource.end()) {
		pDataSource = (*i).second;
	}
	
	Item* pNext = pCommand_->getNext();
	bool result;
	if (pNext == 0)
	{
		result = pDataSource->isDatabaseAvailable();
	}
	else if (pNext->getType() == Item::Type::Number)
	{
		Number* pNum = dynamic_cast<Number*>(pNext);

		if (m_iOption & ExecuteOption::ShowCommand && isSydTestInfoMessage())
		{
			SydTestInfoMessage
				<< "[[Database ID]] "
				<< pNum->getNumber() << ModEndl;
		}
		
		result = pDataSource->isDatabaseAvailable(pNum->getNumber());
	}
	else
	{
		// エラー
		SydTestErrorMessage << "Invalid Parameter." << ModEndl;
		throw SydTestException();
	}
	if (isSydTestInfoMessage())
	{
		SydTestInfoMessage << (result ? "true" : "false") << ModEndl;
	}
}

void
Executor::
createUser(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0)
	{
		SydTestErrorMessage << "'CreateUser' needs user name and password." << ModEndl;
		throw SydTestException();
	}

	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && !m_mapClient.exists(iNumber))
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not Initialized." << ModEndl;
		throw SydTestException();
	}

	// ユーザー名とパスワード引数として文字列を取らなければならない
	String* pUserName = testStringPointer(pNext, ModUnicodeString("No user name String."));
	pNext = pNext->getNext();
	String* pPassword = testStringPointer(pNext, ModUnicodeString("No password String."));
	pNext = pNext->getNext();
	int iUserID = -1;
	bool bHasUserID = false;
	if (pNext && pNext->getType() == Item::Type::Number) {
		iUserID = (dynamic_cast<Number*>(pNext))->getNumber();
		bHasUserID = true;
		pNext = pNext->getNext();
	}
	if (m_iOption & ExecuteOption::ShowCommand  && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
		SydTestInfoMessage << getSessionNumberString(iNumber)
						   << "[SydTest Option] " << pUserName->getString(code)
						   << " " << pPassword->getString(code);
		if (bHasUserID) {
			SydTestInfoMessage << " " << iUserID;
		}
		SydTestInfoMessage << ModEndl;
	}

	if (m_iOption & ExecuteOption::DebugMode) return;

	if (bHasUserID) {
		m_mapClient[iNumber]->createUser(pUserName->getUnicodeString(),
										 pPassword->getUnicodeString(),
										 iUserID);
	} else {
		m_mapClient[iNumber]->createUser(pUserName->getUnicodeString(),
										 pPassword->getUnicodeString());
	}
}

void
Executor::
dropUser(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0)
	{
		SydTestErrorMessage << "'DropUser' needs user name." << ModEndl;
		throw SydTestException();
	}

	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && !m_mapClient.exists(iNumber))
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not Initialized." << ModEndl;
		throw SydTestException();
	}

	// ユーザー名として文字列を取らなければならない
	String* pUserName = testStringPointer(pNext, ModUnicodeString("No user name String."));
	pNext = pNext->getNext();
	int iBehavior = -1;
	bool bHasBehavior = false;
	if (pNext && pNext->getType() == Item::Type::Number) {
		iBehavior = (dynamic_cast<Number*>(pNext))->getNumber();
		bHasBehavior = true;
		pNext = pNext->getNext();
	}
	if (m_iOption & ExecuteOption::ShowCommand  && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
			SydTestInfoMessage << getSessionNumberString(iNumber)
							   << "[SydTest Option] " << pUserName->getString(code);
			if (bHasBehavior) {
				SydTestInfoMessage << " " << iBehavior;
			}
			SydTestInfoMessage << ModEndl;
	}

	if (m_iOption & ExecuteOption::DebugMode) return;

	if (bHasBehavior) {
		m_mapClient[iNumber]->dropUser(pUserName->getUnicodeString(), iBehavior);
	} else {
		m_mapClient[iNumber]->dropUser(pUserName->getUnicodeString());
	}
}

void
Executor::
changePassword(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0)
	{
		SydTestErrorMessage << "'ChangePassword' needs user name and password." << ModEndl;
		throw SydTestException();
	}

	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && !m_mapClient.exists(iNumber))
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not Initialized." << ModEndl;
		throw SydTestException();
	}

	// ユーザー名とパスワード引数として文字列を取らなければならない
	String* pUserName = testStringPointer(pNext, ModUnicodeString("No user name String."));
	pNext = pNext->getNext();
	String* pPassword = testStringPointer(pNext, ModUnicodeString("No password String."));
	if (m_iOption & ExecuteOption::ShowCommand  && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
			SydTestInfoMessage << getSessionNumberString(iNumber)
							   << "[SydTest Option] " << pUserName->getString(code)
							   << " " << pPassword->getString(code) << ModEndl;
	}

	if (m_iOption & ExecuteOption::DebugMode) return;

	m_mapClient[iNumber]->changePassword(pUserName->getUnicodeString(),
										 pPassword->getUnicodeString());
}

void
Executor::
changeOwnPassword(Command* pCommand_)
{
	Item* pNext = pCommand_->getNext();
	if (pNext == 0)
	{
		SydTestErrorMessage << "'ChangeOwnPassword' needs password." << ModEndl;
		throw SydTestException();
	}

	int iNumber = 0;
	// コマンドの引数として数値が来たらセッション番号
	if (pNext->getType() == Item::Type::Number)
	{
		iNumber = (dynamic_cast<Number*>(pNext))->getNumber();
		pNext = pNext->getNext();
	}

	// 与えられたセッション番号のクライアントオブジェクトが存在しない場合はエラー
	if (!(m_iOption & ExecuteOption::DebugMode) && !m_mapClient.exists(iNumber))
	{
		SydTestErrorMessage << "Session Number: " << iNumber
							<< " is not Initialized." << ModEndl;
		throw SydTestException();
	}

	// パスワード引数として文字列を取らなければならない
	String* pPassword = testStringPointer(pNext, ModUnicodeString("No password String."));
	if (m_iOption & ExecuteOption::ShowCommand  && isSydTestInfoMessage())
	{
		ModKanjiCode::KanjiCodeType code = ModKanjiCode::utf8;
			SydTestInfoMessage << getSessionNumberString(iNumber)
							   << "[SydTest Option] " << pPassword->getString(code)
							   << ModEndl;
	}

	if (m_iOption & ExecuteOption::DebugMode) return;

	m_mapClient[iNumber]->changeOwnPassword(pPassword->getUnicodeString());
}

//
//	FUNCTION private 
//	Executor::setSystemParameter -- システムパラメータを設定する
//
//	NOTES
//	  システムパラメータを設定する
//
//	ARGUMENTS
//    Command* pCommand_
//      コマンドを表すオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
setSystemParameter(Command* pCommand_)
{
#if MOD_CONF_REGISTRY == 1
	// 接続先によってはデフォルトパスから変更するため static をはずし、毎回初期化する
	/* static */ ModUnicodeString root
		("HKEY_LOCAL_MACHINE\\SOFTWARE\\Ricoh\\TRMeister");
#else
	ModUnicodeString cstr_confPath;
	ModUnicodeString cstr_systemConfPath;
#endif

	Item* pNext = pCommand_->getNext();
	if (pNext == 0)// || pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// オプション名が指定されていた場合は適切なオプション名か判定
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
			SydTestErrorMessage << "Wrong option Name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "No Parameter value." << ModEndl;
			throw SydTestException();
		}
	}

	// 第一パラメータが数字なら CascadeID (省略した場合は SydTest が Initialize した SydServer)
	if (pNext->getType() == Item::Type::Number)
	{
		int iCascadeID = static_cast<Number*>(pNext)->getNumber();
		if ( iCascadeID < 0 || (unsigned)iCascadeID >= m_cascadeInfoVector.getSize()) {
			SydTestErrorMessage << "Invalid Cascade ID (" << iCascadeID << ")." << ModEndl;
			throw SydTestException();
		}
#if MOD_CONF_REGISTRY == 1
		root = m_cascadeInfoVector[iCascadeID].cstrConfPath;
#else
		cstr_confPath = m_cascadeInfoVector[iCascadeID].cstrConfPath;
		cstr_systemConfPath = m_cascadeInfoVector[iCascadeID].cstrSystemConfPath;
#endif
		pNext = pNext->getNext();
	}

	if( pNext->getType() != Item::Type::String )
	{
		SydTestErrorMessage << "No Parameter Name." << ModEndl;
		throw SydTestException();
	}
	String* pString = static_cast<String*>(pNext);

	const char* pszParameter = pString->getString();
	if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
	{
		SydTestInfoMessage << "[SydTest Parameter] " 
						   << pszParameter << ModEndl;
	}			

	Item* pValue = pNext->getNext();
	if (pValue == 0)
	{
		SydTestErrorMessage << "No Parameter Value." << ModEndl;
		throw SydTestException();
	}

	switch (pValue->getType())
	{
	// 数値型と文字列型にのみ対応
	case Item::Type::String:
	{
		ModAutoMutex<ModCriticalSection> cAuto(&m_cCSForFile);
		cAuto.lock();
		const char* pString = 
			static_cast<String*>(pValue)->getString();
		if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
		{
			SydTestInfoMessage << "[SydTest Parameter and Value] " 
				<< pszParameter << ", " << pString << ModEndl;
		}
#if MOD_CONF_REGISTRY == 0
		if (cstr_confPath.getLength() == 0) {
			_param.setString(pszParameter, pString);
			appendParameter(std::getenv("SYDPARAM"), pszParameter, pString);
		} else {
/*
        TODO: 子サーバはこれする前に立ち上げ直す。たぶんMod使わない？
			ModParameter cascadeParam = ModParameter(ModParameterSource(0, 
														   0, 
														   cstr_confPath.getString(), 
														   0,
														   cstr_systemConfPath.getString()
											), 
										ModTrue);
			cascadeParam.setString(pszParameter, pString);
*/
			appendParameter(cstr_confPath.getString() ,pszParameter, pString);
		}
#endif
#if MOD_CONF_REGISTRY == 1
		ModParameter::setString(root, ModUnicodeString(pszParameter), 
								ModUnicodeString(pString));
#endif
	}
	break;
	case Item::Type::Number:
	{
		ModAutoMutex<ModCriticalSection> cAuto(&m_cCSForFile);
		cAuto.lock();
		int iNumber = static_cast<Number*>(pValue)->getNumber();
		if (m_iOption & ExecuteOption::ShowParameter && isSydTestInfoMessage())
		{
			SydTestInfoMessage << "[SydTest Parameter and Value] " 
				<< pszParameter << ", " << iNumber << ModEndl;
		}
#if MOD_CONF_REGISTRY == 0
		if (cstr_confPath.getLength() == 0) {
			_param.setInteger(pszParameter, iNumber);
			appendParameter(std::getenv("SYDPARAM"), pszParameter, iNumber);
		} else {
/*
			ModParameter cascadeParam = ModParameter(ModParameterSource(0, 
														   0, 
														   cstr_confPath.getString(), 
														   0,
														   cstr_systemConfPath.getString()
											), 
										ModTrue);
			cascadeParam.setInteger(pszParameter, iNumber);
*/
			appendParameter(cstr_confPath.getString(), pszParameter, iNumber);
		}
#endif
#if MOD_CONF_REGISTRY == 1
		ModUnicodeString musParameter(pszParameter); 
		ModParameter::setInteger(root, musParameter, iNumber);
#endif
	}
	break;
	default:
		SydTestErrorMessage << "Wrong value type." << ModEndl;
		throw SydTestException();
	}
}

//
//	FUNCTION private 
//	Executor::addCascade -- 子サーバを追加する
//
//	NOTES
//	  子サーバを追加する
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
addCascade(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	if (pNext == 0)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// オプション名が指定されていた場合は適切なオプション名か判定
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
			SydTestErrorMessage << "Wrong option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}

	// 第一引数は CascadeID なので整数以外の場合はエラー
	if (pNext->getType() != Item::Type::Number)
	{
		SydTestErrorMessage << "Cascade ID Empty." << ModEndl;
		throw SydTestException();
	}
	Number* pNum = dynamic_cast<Number*> (pNext);
	int iCascadeID = pNum->getNumber();

	// CascadeID < 0 ならエラー
	if (iCascadeID < 0)
	{
		SydTestErrorMessage << "Invalid Cascade ID. (<0)" << ModEndl;
		throw SydTestException();
	}

	// /dist または /distfile オプションで指定した個数を超える CascadeID が指定されていたらエラー
	if ((unsigned) iCascadeID >= m_cascadeInfoVector.getSize())
	{
		SydTestErrorMessage << "Undefined Cascade ID (" << iCascadeID << ")." << ModEndl;
		throw SydTestException();
	}

	pNext = pNum->getNext();
	// 第二引数は子サーバ名なので文字列以外の場合はエラー
	if (pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "Child Server Name Empty." << ModEndl;
		throw SydTestException();
	}

	String* pString = dynamic_cast<String*> (pNext);
	ModUnicodeString cstrChildServerName = pString->getUnicodeString();

	pNext = pNext->getNext();
	// 第二引数は子サーバ名なので文字列以外の場合はエラー

	ModUnicodeString cstrDBName = "";
	if (pNext &&
		pNext->getType() == Item::Type::String)
	{
		cstrDBName = (dynamic_cast<String*> (pNext))->getUnicodeString();
	}


	// SQL コマンドを発行する
	ModUnicodeOstrStream cstreamAddCascadeSQL;
	cstreamAddCascadeSQL << "create cascade " << cstrChildServerName
						 << " on '" << m_cascadeInfoVector[iCascadeID].cstrHostName << "' "
						 << m_cascadeInfoVector[iCascadeID].iPortNumber <<" "<<cstrDBName ;

	ModAutoPointer<Item> pCascadeItem = 0;
	pCascadeItem = new Command("Command");
	pCascadeItem->setNext(new String(cstreamAddCascadeSQL.getString()));
	Item* pNextItem = pCascadeItem->getNext();
	pNextItem->setNext(0);

	executeSQLCommand(pCascadeItem);
}

//
//	FUNCTION private 
//	Executor::alterCascade -- 子サーバを変更する
//
//	NOTES
//	  子サーバを変更する
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
alterCascade(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	if (pNext == 0)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// オプション名が指定されていた場合は適切なオプション名か判定
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
			SydTestErrorMessage << "Wrong option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}

	// 第一引数は CascadeID なので整数以外の場合はエラー
	if (pNext->getType() != Item::Type::Number)
	{
		SydTestErrorMessage << "Cascade ID Empty." << ModEndl;
		throw SydTestException();
	}
	Number* pNum = dynamic_cast<Number*> (pNext);
	int iCascadeID = pNum->getNumber();

	// CascadeID が 0 以下ならエラー ( 0 は分散マネージャなので子サーバには割り当てられない)
	if (iCascadeID <= 0)
	{
		SydTestErrorMessage << "Invalid Cascade ID (<=0)." << ModEndl;
		throw SydTestException();
	}

	// /dist または /distfile オプションで指定した個数を超える CascadeID が指定されていたらエラー
	// ※子サーバの CascadeID は 1 から始まる(0 は分散マネージャ用)ため、イコールはOK
	if ((unsigned) iCascadeID > m_cascadeInfoVector.getSize())
	{
		SydTestErrorMessage << "Undefined Cascade ID (" << iCascadeID << ")." << ModEndl;
		throw SydTestException();
	}

	pNext = pNum->getNext();
	// 第二引数は子サーバ名なので文字列以外の場合はエラー
	if (pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "Child Server Name Empty." << ModEndl;
		throw SydTestException();
	}
	String* pString = dynamic_cast<String*> (pNext);
	ModUnicodeString cstrChildServerName = pString->getUnicodeString();

	// SQL コマンドを発行する
	ModUnicodeOstrStream cstreamAddCascadeSQL;
	cstreamAddCascadeSQL << "alter cascade " << cstrChildServerName
						 << " to '" << m_cascadeInfoVector[iCascadeID].cstrHostName << "' "
						 << m_cascadeInfoVector[iCascadeID].iPortNumber;

	ModAutoPointer<Item> pCascadeItem = 0;
	pCascadeItem = new Command("Command");
	pCascadeItem->setNext(new String(cstreamAddCascadeSQL.getString()));
	Item* pNextItem = pCascadeItem->getNext();
	pNextItem->setNext(0);

	executeSQLCommand(pCascadeItem);	
}

//
//	FUNCTION private 
//	Executor::dropCascade -- 子サーバを削除する
//
//	NOTES
//	  子サーバを削除する
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
dropCascade(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	if (pNext == 0)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// 第一引数は子サーバ名なので文字列以外の場合はエラー
	if (pNext->getType() != Item::Type::String)
	{
		SydTestErrorMessage << "Child Server Name Empty." << ModEndl;
		throw SydTestException();
	}
	String* pString = dynamic_cast<String*> (pNext);
	ModUnicodeString cstrChildServerName = pString->getUnicodeString();

	// SQL コマンドを発行する
	ModUnicodeOstrStream cstreamAddCascadeSQL;
	cstreamAddCascadeSQL << "drop cascade " << cstrChildServerName;

	ModAutoPointer<Item> pCascadeItem = 0;
	pCascadeItem = new Command("Command");
	pCascadeItem->setNext(new String(cstreamAddCascadeSQL.getString()));
	Item* pNextItem = pCascadeItem->getNext();
	pNextItem->setNext(0);

	executeSQLCommand(pCascadeItem);
}

//
//	FUNCTION private 
//	Executor::startCascade -- 子サーバを起動する
//
//	NOTES
//	  子サーバを起動する。起動中のSydServerに対する動作は保証しない。
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
startCascade(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	if (pNext == 0)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// オプション名が指定されていた場合は適切なオプション名か判定(省略可)
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
			SydTestErrorMessage << "Wrong option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}
	
	// 第一引数は起動するサーバのCascadeID
	if (pNext->getType() != Item::Type::Number)
	{
		SydTestErrorMessage << "Cascade Server Name Empty." << ModEndl;
		throw SydTestException();
	}
	Number* pNum = dynamic_cast<Number*> (pNext);
	int iCascadeID = pNum->getNumber();

	if (iCascadeID < 0 || iCascadeID >= m_cascadeInfoVector.getSize()) {
		SydTestErrorMessage << "Invalid CascadeID (=" << iCascadeID << ")" << ModEndl;
		throw SydTestException();
	}

	if (isSydTestInfoMessage())
	{
		SydTestInfoMessage << "Cascade ID: " << iCascadeID << ModEndl;
	}
	
	// 外部コマンドを組み立てる
	ModUnicodeOstrStream cCommandStream;
#ifdef SYD_OS_WINDOWS
	cCommandStream << "net start " << m_cascadeInfoVector[iCascadeID].cstrServiceName << " > nul";
#else
	cCommandStream << m_cascadeInfoVector[iCascadeID].cstrInstallPath
				   << "/bin/trmeister start > /dev/null";
#endif
	ModUnicodeString cCommand(cCommandStream.getString());

	int iStatus = system(cCommand.getString());
	if(iStatus != 0) {
		SydTestErrorMessage << "abnormal exit status to start cascade " << iCascadeID << ModEndl;
	}
}

//
//	FUNCTION private 
//	Executor::terminateCascade -- 子サーバを正常終了する
//
//	NOTES
//	  子サーバを正常終了する。停止中のSydServerに対する動作は保証しない。
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
terminateCascade(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	if (pNext == 0)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// オプション名が指定されていた場合は適切なオプション名か判定(省略可)
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
			SydTestErrorMessage << "Wrong option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}
	
	// 第一引数は停止するサーバのCascadeID
	if (pNext->getType() != Item::Type::Number)
	{
		SydTestErrorMessage << "Cascade Server Name Empty." << ModEndl;
		throw SydTestException();
	}
	Number* pNum = dynamic_cast<Number*> (pNext);
	int iCascadeID = pNum->getNumber();

	if (iCascadeID < 0 || iCascadeID >= m_cascadeInfoVector.getSize()) {
		SydTestErrorMessage << "Invalid CascadeID (=" << iCascadeID << ")" << ModEndl;
		throw SydTestException();
	}

	if (isSydTestInfoMessage())
	{
		SydTestInfoMessage << "Cascade ID: " << iCascadeID << ModEndl;
	}
	
	// 外部コマンドを組み立てる
	ModUnicodeOstrStream cCommandStream;
#ifdef SYD_OS_WINDOWS
	cCommandStream << "net stop " << m_cascadeInfoVector[iCascadeID].cstrServiceName << " > nul";
#else
	cCommandStream << m_cascadeInfoVector[iCascadeID].cstrInstallPath
				   << "/bin/trmeister stop > /dev/null";
#endif
	ModUnicodeString cCommand(cCommandStream.getString());

	int iStatus = system(cCommand.getString());
	if(iStatus != 0) {
		SydTestErrorMessage << "abnormal exit status to terminate cascade " << iCascadeID << ModEndl;
	}
}

//
//	FUNCTION private 
//	Executor::forceTerminateCascade -- 子サーバを強制終了する
//
//	NOTES
//	  子サーバを強制終了する。異常終了に相当。
//	  停止中のSydServerに対する動作は保証しない。
//
//	ARGUMENTS
//    Item* pItem_
//      引数を含むオブジェクト
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    SydTestException
//
void
Executor::
forceTerminateCascade(Item* pItem_)
{
	Item* pNext = pItem_->getNext();

	if (pNext == 0)
	{
		SydTestErrorMessage << "No Parameter." << ModEndl;
		throw SydTestException();
	}

	// オプション名が指定されていた場合は適切なオプション名か判定(省略可)
	if (pNext->getType() == Item::Type::Option) {
		Option* pOpt = dynamic_cast<Option*> (pNext);
		if (pOpt->getOptionType() != Option::OptionType::CascadeID) {
			SydTestErrorMessage << "Wrong option name: " << pOpt->getOptionName() << ModEndl;
			throw SydTestException();
		}
		// パラメータ値が未設定
		if ((pNext = pNext->getNext()) == 0) 
		{
			SydTestErrorMessage << "Parameter value is not set. " << ModEndl;
			throw SydTestException();
		}
	}

	// 第一引数は停止するサーバのCascadeID
	if (pNext->getType() != Item::Type::Number)
	{
		SydTestErrorMessage << "Cascade Server Name Empty." << ModEndl;
		throw SydTestException();
	}
	Number* pNum = dynamic_cast<Number*> (pNext);
	int iCascadeID = pNum->getNumber();

	if (iCascadeID < 0 || iCascadeID >= m_cascadeInfoVector.getSize()) {
		SydTestErrorMessage << "Invalid CascadeID (=" << iCascadeID << ")" << ModEndl;
		throw SydTestException();
	}

	if (isSydTestInfoMessage())
	{
		SydTestInfoMessage << "Cascade ID: " << iCascadeID << ModEndl;
	}
	
	// 外部コマンドを組み立てる
	ModUnicodeOstrStream cCommandStream;
#ifdef SYD_OS_WINDOWS
	cCommandStream << "taskkill /f /fi "
				   << "\"SERVICES eq " << m_cascadeInfoVector[iCascadeID].cstrServiceName << "\" "
				   << "/im SydServer.exe > nul";	// 実行ファイル名の情報を持ってないのでハードコーディング
#else
	cCommandStream << "pgrep -f " << m_cascadeInfoVector[iCascadeID].cstrInstallPath << "/bin/SydServer "
				   << "| xargs kill -9 > /dev/null";
#endif
	ModUnicodeString cCommand(cCommandStream.getString());

	int iStatus = system(cCommand.getString());
	if(iStatus != 0) {
		SydTestErrorMessage << "abnormal exit status to force-terminate cascade " << iCascadeID << ModEndl;
	}
}

// 以下は下請け関数

//
//	FUNCTION private 
//	Executor::testStringPointer -- コマンドの要素を文字列に変換する
//
//	NOTES
//	  コマンドの要素を文字列に変換する。変換できない場合は例外を出す。
//
//	ARGUMENTS
//    Item* pNext_
//      コマンドの要素
//    ModUnicodeString& cstrErrorMessage_
//      第一引数を文字列に変換できなかったときに表示するエラーメッセージ
//
//	RETURN
//    String*
//      変換結果である文字列
//
//	EXCEPTIONS
//    SydTestException
//
String* 
Executor::testStringPointer(Item* pNext_,
							const ModUnicodeString& cstrErrorMessage_)
{
	if (pNext_ == 0 || pNext_->getType() !=  Item::Type::String)
	{
		SydTestErrorMessage << cstrErrorMessage_ << ModEndl;
		throw SydTestException();
	}
	else
	{
		return static_cast<String*>(pNext_);
	}
}

//
//	FUNCTION private 
//	Executor::testNumberPointer -- コマンドの要素を数値に変換する
//
//	NOTES
//	  コマンドの要素を数値に変換する。変換できない場合は例外を出す。
//
//	ARGUMENTS
//    Item* pNext_
//      コマンドの要素
//    ModUnicodeString& cstrErrorMessage_
//      第一引数を数値に変換できなかったときに表示するエラーメッセージ
//
//	RETURN
//    Number*
//      変換結果である文字列
//
//	EXCEPTIONS
//    SydTestException
//
Number* 
Executor::testNumberPointer(Item* pNext_,
							const ModUnicodeString& cstrErrorMessage_)
{
	if (pNext_ == 0 || pNext_->getType() !=  Item::Type::Number)
	{
		SydTestErrorMessage << cstrErrorMessage_ << ModEndl;
		throw SydTestException();
	}
	else
	{
		return static_cast<Number*>(pNext_);
	}
}

//
//	FUNCTION private 
//	Executor::testParameterPointer -- コマンドの要素をパラメータ型に変換する
//
//	NOTES
//	  コマンドの要素をパラメータ型に変換する。変換できない場合は例外を出す。
//
//	ARGUMENTS
//    Item* pNext_
//      コマンドの要素
//    ModUnicodeString& cstrErrorMessage_
//      第一引数を数値に変換できなかったときに表示するエラーメッセージ
//
//	RETURN
//    Parameter*
//      変換結果であるパラメータ
//
//	EXCEPTIONS
//    SydTestException
//
Parameter* 
Executor::testParameterPointer(Item* pNext_,
							   const ModUnicodeString& cstrErrorMessage_)
{
	if (pNext_ == 0 || pNext_->getType() !=  Item::Type::Parameter)
	{
		SydTestErrorMessage << cstrErrorMessage_ << ModEndl;
		throw SydTestException();
	}
	else
	{
		return static_cast<Parameter*>(pNext_);
	}
}

//
//	FUNCTION private 
//	Executor::getSessionNumberString -- セッション番号を文字列に変換する
//
//	NOTES
//	  セッション番号を、「<<n>>」の形の文字列に変換する。
//
//	ARGUMENTS
//    int iNumber_
//      セッション番号
//
//	RETURN
//    ModCharString
//      変換結果の文字列。形式は「<<n>>」。
//
//	EXCEPTIONS
//    なし
//
ModCharString 
Executor::getSessionNumberString(int iNumber_)
{
	if (m_iOption & ExecuteOption::ShowSessionNumber)
	{
		ModCharString str;
		str.format("<<%d>> ", iNumber_);
		return str;
	} else {
		return "";
	}
}

// setEncodingTypeの下請
void
Executor::setEncodingType_sub(const char* str) 
{
	if (!strcmp(str, "utf8")) {
		ModOs::Process::setEncodingType(ModKanjiCode::utf8);
	}
	else if (!strcmp(str, "euc")) {
		ModOs::Process::setEncodingType(ModKanjiCode::euc);
	}
	else if (!strcmp(str, "shiftJis") || !strcmp(str, "ShiftJis") || !strcmp(str, "sjis")) {
	    ModOs::Process::setEncodingType(ModKanjiCode::shiftJis);
	}
#if 0
	else if (!strcmp(str, "jis")) {
		ModOs::Process::setEncodingType(ModKanjiCode::jis);
	}
	else if (!strcmp(str, "ucs2")) {
		ModOs::Process::setEncodingType(ModKanjiCode::ucs2);
	}
#endif
	else //default
		ModOs::Process::setEncodingType(Common::LiteralCode);
}
// FUNCTION private
//	SydTest::Executor::startExplain -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	Client2::Session* pClient_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Executor::
startExplain(int iNumber_, Client2::Session* pClient_)
{
	Client2::ResultSet* pResultSet;
	try {
		Common::DataArrayData cDummy;
		pResultSet = createSQLCommand(iNumber_, "start explain execute hint 'file data cost lock'", &cDummy);
	}catch(Exception::Object& err){
		SydTestErrorMessage << err << ModEndl;
		throw;
	}

	ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber_]);
	cAuto.lock();

	Common::DataArrayData cTuple;
	Client2::ResultSet::Status::Value eStatus;

	while (pResultSet->getNextTuple(&cTuple) != Client2::ResultSet::Status::Success);
	pResultSet->close();
	pResultSet->release();
}

// FUNCTION private
//	SydTest::Executor::endExplain -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	Client2::Session* pClient_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Executor::
endExplain(int iNumber_, Client2::Session* pClient_)
{
	Client2::ResultSet* pResultSet;
	try {
		Common::DataArrayData cDummy;
		pResultSet = createSQLCommand(iNumber_, "end explain", &cDummy);
	}catch(Exception::Object& err){
		SydTestErrorMessage << err << ModEndl;
		throw;
	}

	ModAutoMutex<ModCriticalSection> cAuto(m_mapCSForClient[iNumber_]);
	cAuto.lock();

	Common::DataArrayData cTuple;
	Client2::ResultSet::Status::Value eStatus;

	while (pResultSet->getNextTuple(&cTuple) != Client2::ResultSet::Status::Success);
	pResultSet->close();
	pResultSet->release();
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
