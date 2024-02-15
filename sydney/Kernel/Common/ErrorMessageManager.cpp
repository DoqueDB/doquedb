// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorMessageManager.cpp -- 各国語のエラーメッセージを管理する
// 
// Copyright (c) 2000, 2003, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Common/ErrorMessageManager.h"
#include "Common/ErrorMessage.h"
#include "Common/UnicodeString.h"

#include "Exception/Object.h"

#include "Os/AutoCriticalSection.h"

_TRMEISTER_USING

using namespace Common;

namespace
{
//
//	言語とライブラリ名との表
//	
class LanguageItem
{
public:
	const char* m_pszLanguage;	//言語名
	const char* m_pszLibrary;	//関数名
};

#define DLLNAME(_x_) _x_ CLIENTSUFFIX
LanguageItem cLanguageTable[] = {
	{"English",		DLLNAME("Eng")},		//英語(default)
	{"Japanese",	DLLNAME("Jpn")},		//日本語
	{0,				0}
#undef DLLNAME
};

}

//
//	VARIABLE private
//	Common::ErrorMessageManager::m_mapManager -- 各国語のメッセージのマップ
//
//	NOTES
//	各国語に対応したCommon::ErrorMessageのマップ。
//	ロードしたものを格納する
//
ErrorMessageManager::MessageMap
ErrorMessageManager::m_mapMessage;

//
//	VARIABLE private
//	Common::ErrorMessageManager::m_pMessage -- 今設定されているメッセージ
//
//	NOTES
//	今設定されているメッセージを設定する。
//
ErrorMessage*
ErrorMessageManager::m_pMessage = 0;

//
//	VARIABLE private
//	Common::ErrorMessageManager::m_cCriticalSection -- クリティカルセクション
//
//	NOTES
//	排他制御用のクリティカルセクション
//
Os::CriticalSection
ErrorMessageManager::m_cCriticalSection;

//
//	FUNCTION public static
//	Common::ErrorMessageManager::setLanguage -- 言語を設定する
//
//	NOTES
//	言語を設定する。次にまたこの関数が呼ばれるまで makeErrorMessage の
//	返すメッセージはその言語のものとなる。
//	存在しないものが設定された場合は、defaultの言語になる。
//	defaultの言語はcLanguageTableの最初の要素のものである。
//
//	ARGUMENTS
//	const ModCharString& cstrLanguage_
//		言語名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ErrorMessageManager::setLanguage(const ModCharString& cstrLanguage_)
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);

	//指定の言語のライブラリ名を得る
	LanguageItem* p = &cLanguageTable[0];
	while (p->m_pszLanguage)
	{
		if (p->m_pszLanguage == cstrLanguage_)
		{
			//存在したのでマップを検索する
			ErrorMessage* pMessage = 0;
			MessageMap::Iterator i = m_mapMessage.find(cstrLanguage_);
			if (i == m_mapMessage.end())
			{
				//ないのでロードする
				pMessage = new ErrorMessage(p->m_pszLibrary);
				try
				{
					pMessage->initialize();
				}
				catch (Exception::Object&)
				{
					//ロードできなかった
					delete pMessage;
					pMessage = 0;
				}
				//マップに格納する
				//ロードできなくても何回もロードを試みないようにマップに入れる
				m_mapMessage.insert(cstrLanguage_, pMessage);
			}
			else
			{
				//マップにあるのでそれを取り出す
				pMessage = (*i).second;
			}

			if (pMessage)
			{
				m_pMessage = pMessage;
			}

			break;
		}
		p++;
	}
}

//
//	FUNCTION public static
//	Common::ErrorMessageManager::getMessage -- メッセージクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::ErrorMessage*
//		メッセージクラスへのポインタ
//
//	EXCEPTIONS
//
ErrorMessage*
ErrorMessageManager::getMessage()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);

	if (m_pMessage == 0)
	{
		// まだ言語が設定されていないので、デフォルトを設定する
		setLanguage(cLanguageTable[0].m_pszLanguage);
	}

	return m_pMessage;
}

//
//	FUNCTION public static
//	Common::ErrorMessageManager::makeErrorMesssage
//									-- エラーメッセージを作成する
//
//	NOTES
//	今設定されている言語でエラーメッセージを作成する。
//
//	ARGUMENTS
//	ModUnicodeChar* pszBuffer_
//		エラーメッセージを格納するバッファ
//	unsigned int uiErrorNumber_
//		エラー番号
//	const ModUnicodeChar* pszArgument_
//		メッセージ引数(Exceptionが持っている)
//
//	RETURN
//	ModUnicodeChar*
//		エラーメッセージ
//
//	EXCEPTIONS
//	なし
//
ModUnicodeChar*
ErrorMessageManager::makeErrorMessage(ModUnicodeChar* pszBuffer_,
									  unsigned int uiErrorNumber_,
									  const ModUnicodeChar* pszArgument_)
{
	*pszBuffer_ = 0;
	//メッセージを作成する
	ErrorMessage* p = getMessage();
	if (p) p->makeMessage(pszBuffer_, uiErrorNumber_, pszArgument_);
	return pszBuffer_;
}

//
//	FUNCTION public static
//	Common::ErrorMessageManager::initialize -- 初期化
//
// 	NOTES
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
ErrorMessageManager::initialize()
{
	// DLLをロードする
	setLanguage(cLanguageTable[0].m_pszLanguage);
}

//
//	FUNCTION public static
//	Common::ErrorMessageManager::terminate -- 後処理
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
Common::ErrorMessageManager::terminate()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);

	MessageMap::Iterator i = m_mapMessage.begin();
	const MessageMap::Iterator& end = m_mapMessage.end();

	for (; i != end; ++i) {
		delete (*i).second;
	}
	m_mapMessage.erase(m_mapMessage.begin(), m_mapMessage.end());

	m_pMessage = 0;
}

//
// Copyright (c) 2000, 2003, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
