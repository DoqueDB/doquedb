// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Exec.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Sqli";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include <iostream>
#include <signal.h>
#include "Sqli/Exec.h"
#include "Client2/Session.h"
#include "Client2/ResultSet.h"
#include "Common/DataArrayData.h"
#include "Common/UnicodeString.h"

#include "Communication/AuthorizeMode.h"

#include "Exception/Message.h"
#include "Exception/NumberAuthorizationFailed.h"
#include "Exception/NumberUserNotFound.h"
#include "Exception/NumberUserRequired.h"

#include "ModUnicodeOstrStream.h"
#include "ModCharTrait.h"
#include "ModAutoPointer.h"
#include "ModOsDriver.h"
#include "ModTime.h"

using namespace std;

_TRMEISTER_USING

namespace {
	//
	//	VARIABLE local
	//
	Client2::ResultSet* _pResultSet = 0;

	//
	//	FUNCTION local
	//
	extern "C" void _handler(int sig)
	{
		if (_pResultSet) _pResultSet->cancel();
	}

#ifdef SYD_OS_WINDOWS
	//
	//	LOCAL
	//	コードページ
	//
	unsigned int _codePage = CP_ACP;
#else
	// Encoding type
	ModKanjiCode::KanjiCodeType _encodingType =
									Common::LiteralCode;
#endif

	// エコーバックするか
	bool _doEcho = false;

	namespace _Password
	{
		// retry count
		const int _iRetryMax = 3;
	}

	//
	//	VARIABLE local
	//
	bool _bTime = false;

	//
	//	CLASS
	//	_$$::_AutoPrintTime -- 時間出力のためのクラス
	//
	class _AutoPrintTime
	{
	public:
		// コンストラクター
		_AutoPrintTime()
			{
				if (_bTime == true)
					_start = ModTime::getCurrentTime();
			}

		// デストラクター
		~_AutoPrintTime()
			{
				if (_bTime == true) {
					ModTimeSpan interval(ModTime::getCurrentTime() - _start);
					cout << "TIME: "
						 << interval.getTotalMilliSeconds()
						 << " ms"
						 << endl;
				}
			}

	private:
		// 処理の開始時刻
		ModTime						_start;
	};

	// sqli の終了ステータス
	int _exitStatus = 0;
}

//
//	FUNCTION public
//	Exec::Exec -- コンストラクタ
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
Exec::Exec(Client2::DataSource& cDataSource_, const Option& cOption_)
	: m_cDataSource(cDataSource_),
	  m_cOption(cOption_)
{
}

//
//	FUNCTION public
//	Exec::~Exec -- デストラクタ
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
Exec::~Exec()
{
}

//
//	FUNCTION public
//	Exec::replace -- 置換する
//
//	NOTES
//
//	ARGUMENS
//	const ModUnicodeString& cstrSQL_
//		SQL文
//
//	RETURN
//	ModUnicodeString
//		置換された文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Exec::replace(const ModUnicodeString& cstrSQL_)
{
	ModUnicodeString cstrResult;
	const ModUnicodeChar* p = cstrSQL_;
	while (*p)
	{
		Option::Vector::ConstIterator i = m_cOption.getReplaceString().begin();
		const Option::Vector::ConstIterator last = m_cOption.getReplaceString().end();
		for (; i != last; ++i)
		{
			if ((*i).first[0] == *p)
			{
				// 先頭文字がマッチしたので、compareする
				if (ModUnicodeCharTrait::compare((*i).first, p, (*i).first.getLength()) == 0)
				{
					// 同じなので、入れ替える
					cstrResult += (*i).second;
					p += (*i).first.getLength();
					break;
				}
			}
		}
		if (i == last)
		{
			cstrResult += *p++;
		}
	}

	return cstrResult;
}

//
//	FUNCTION public
//	Exec::execute -- 実行する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Exec::execute()
{
	//セッションを作成する
	Client2::Session* pSession = createSession();
	if (pSession == 0) {
		return;
	}

	if (m_cOption.isVersion() == false)
	{
		// prompt表示
		prompt();
	}

	try 
	{
		if (m_cOption.isVersion())
		{
			try
			{
				// サーバーバージョンの表示
				ModUnicodeString v = pSession->queryProductVersion();

				// 出力
				cout << (const char*)unicodeToMultiByte(v)
					 << endl;
				
				setExitStatus(0);
			}
			catch (Exception::Object& e)
			{
				ModUnicodeOstrStream stream;
				stream << e;
				cerr << "[ERROR] "
					 << (const char*)unicodeToMultiByte(stream.getString())
					 << endl;
				setExitStatus(1);
			}
			catch (ModException& e)
			{
				cerr << "[ERROR] ModException occurred. "
					 << e.setMessage() << endl;
				setExitStatus(1);
			}
		}
		else
		{
			ModUnicodeString cstrSQL;

			while (getNext(cstrSQL) == true)
			{
				if (m_cOption.getReplaceString().getSize())
				{
					//置換文字列がある
					cstrSQL = replace(cstrSQL);
				}

				if (_doEcho)
					cerr << (const char*) unicodeToMultiByte(cstrSQL) << endl;

				if (cstrSQL.compare(ModUnicodeString("exit"), ModFalse) == 0)
				{
					// 終了
					break;
				}

				_AutoPrintTime cTime;
			
				//SQL文を実行する
				_pResultSet = pSession->executeStatement(cstrSQL);

				signal(SIGINT, _handler);
			
				try
				{
					//実行結果を得る
					for (;;)
					{
						ModAutoPointer<Common::DataArrayData> pTuple
							= new Common::DataArrayData();
						Client2::ResultSet::Status::Value eStatus
							= _pResultSet->getNextTuple(pTuple.get());

						if (eStatus == Client2::ResultSet::Status::Data)
						{
							//タプルデータ
							ModUnicodeString cstrTuple = pTuple->getString();
							//出力
							cout << (const char*)unicodeToMultiByte(cstrTuple)
								 << endl;
						}
						else if (eStatus ==
								 Client2::ResultSet::Status::EndOfData)
						{
						}
						else if (eStatus ==
								 Client2::ResultSet::Status::Success)
						{
							break;
						}
						else if (eStatus ==
								 Client2::ResultSet::Status::Canceled)
						{
							cout << "CANCEL" << endl;
							break;
						}
						else if (eStatus == Client2::ResultSet::Status::MetaData)
						{
							//メタデータ
							ModUnicodeString cstrMetaData
								= _pResultSet->getMetaData()->getString();
							//出力
							cout << (const char*)unicodeToMultiByte(cstrMetaData) << endl;
						}
					}
					setExitStatus(0);
				}
				catch (Exception::Object& e)
				{
					ModUnicodeOstrStream stream;
					stream << e;
					cerr << "[ERROR] "
						 << (const char*)unicodeToMultiByte(stream.getString())
						 << endl;
					setExitStatus(1);
				}
				catch (ModException& e)
				{
					cerr << "[ERROR] ModException occurred. "
						 << e.setMessage() << endl;
					setExitStatus(1);
				}

				_pResultSet->close();
				_pResultSet->release();

				signal(SIGINT, SIG_IGN);
			}
		}
	}
	catch (...)
	{
		pSession->close();
		pSession->release();
		throw;
	}

	pSession->close();
	pSession->release();
}

//
//	FUNCTION public
//	Exec::prompt -- プロンプトを表示する
//
//	NOTES
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
Exec::prompt()
{
}

//
//	FUNCTION public static
//	Exec::multiByteToUnicode -- マルチバイト文字列をUnicode文字列に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const char* pszBuffer_
//		変換するマルチバイト文字列
//
//	RETURN
//	ModUnicodeString
//		変換したUnicode文字列
//
//	EXCEPTIONS
//
ModUnicodeString
Exec::multiByteToUnicode(const char* pszBuffer_)
{
#ifdef SYD_OS_WINDOWS
	int len = ::MultiByteToWideChar(_codePage, 0, pszBuffer_, -1, 0, 0);
	ModUnicodeChar* p = new ModUnicodeChar[len];
	::MultiByteToWideChar(_codePage, 0, pszBuffer_, -1, p, len*sizeof(ModUnicodeChar));
	ModUnicodeString str(p);
	delete [] p;
	return str;
#else
	return ModUnicodeString(pszBuffer_, _encodingType);
#endif
}

//
//	FUNCTION public static
//	Exec::unicodeToMultiByte -- Unicode文字でつをマルチバイト文字列に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrBuffer_
//		変換するUnicode文字列
//
//	RETURN
//	ModCharString
//		変換したマルチバイト文字列
//
//	EXCEPTIONS
//
ModCharString
Exec::unicodeToMultiByte(const ModUnicodeString& cstrBuffer_)
{
#ifdef SYD_OS_WINDOWS
	int len = ::WideCharToMultiByte(_codePage, 0, cstrBuffer_, -1, 0, 0, 0, 0);
	char* p = new char[len];
	::WideCharToMultiByte(_codePage, 0, cstrBuffer_, -1, p, len, 0, 0);
	ModCharString str(p);
	delete [] p;
	return str;
#else
	ModUnicodeString tmp(cstrBuffer_);
	return ModCharString(tmp.getString(_encodingType));
#endif
}
	
//
//	FUNCTION public static
//	Exec::setCode -- 文字コードを設定する
//
//	NOTES
//
//	ARGUMENTS
//	const ModCharString& cstrCode_
//		チェックするコード
//
//	RETURN
//	bool
//		サポートしている場合はtrue、それ以外の場合はfalse
//
//	EXCETPIONS
//
bool
Exec::setCode(const ModCharString& cstrCode_)
{
#ifdef SYD_OS_WINDOWS
	if (cstrCode_.getLength() == 0)
	{
		_codePage = CP_ACP;
		return true;
	}
	else if (ModCharTrait::compare(cstrCode_, "utf-8", ModFalse) == 0)
	{
		_codePage = CP_UTF8;
		return true;
	}
#else
	if (cstrCode_.getLength() == 0) {
		_encodingType = ModOs::Process::getEncodingType();
		return true;
	} else if (ModCharTrait::compare(cstrCode_, "utf-8", ModFalse) == 0) {
		_encodingType = ModKanjiCode::utf8;
		return true;
	}
#endif
	return false;
}

//	FUNCTION public static
//	Exec::setEcho -- エコーバックするか設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool		v
//			true
//				エコーバックする
//			false
//				エコーバックしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Exec::setEcho(bool v)
{
	_doEcho = v;
}

// FUNCTION private
//	Sqli::Exec::createSession -- Sessionを新たに作る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Client2::Session*
//
// EXCEPTIONS

Client2::Session*
Exec::
createSession()
{
	if (m_cDataSource.getAuthorization() == Communication::AuthorizeMode::None) {
		// connecting to old version server
		return m_cDataSource.createSession(m_cOption.getDatabaseName());
	}

	// retry until valid user and password is specified
	int iRetry = (m_cOption.isUserPasswordSpecified())
		? 1
		: _Password::_iRetryMax + 1;

	Client2::Session* pSession = 0;
	for (;;) {
		try {
			pSession = m_cDataSource.createSession(m_cOption.getDatabaseName(),
												   m_cOption.getUserName(),
												   m_cOption.getPassword());
			break;
		} catch (Exception::Object& e) {
			switch (e.getErrorNumber()) {
			case Exception::ErrorNumber::AuthorizationFailed:
			case Exception::ErrorNumber::UserNotFound:
			case Exception::ErrorNumber::UserRequired:
				{
					if (--iRetry > 0) {
						if (m_cOption.isUserPasswordSpecified()) {
							ModUnicodeOstrStream stream;
							stream << e;
							cerr << "[ERROR] "
								 << (const char*)Exec::unicodeToMultiByte(stream.getString())
								 << endl;
							ModOsDriver::Thread::sleep(3000);
						}
						m_cOption.clearUserPassword();
						if (m_cOption.inputUserName() == false) break;
						if (m_cOption.inputPassword() == false) break;
						break;
					}
					throw;
				}
			default:
				{
					throw;
				}
			}
		}
	}
	return pSession;
}

//
//	FUNCTION public static
//	Exec::setTime -- 実行時間出力を行うかどうかを設定する
//
void
Exec::setTime(bool bTime_)
{
	_bTime = bTime_;
}

//
//	FUNCTION public static
//	Exec::getExitStatus -- sqliの終了ステータスを得る
//
int
Exec::getExitStatus()
{
	return _exitStatus;
}

//
//	FUNCTION pubklic static
//	Exec::setExitStatus -- sqliの終了ステータスを設定する
//
void
Exec::setExitStatus(int status_)
{
	_exitStatus = status_;
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2014, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
