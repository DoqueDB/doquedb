// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MessageStreamBuffer.cpp -- メッセージストリームバッファ
// 
// Copyright (c) 2000, 2001, 2003, 2005, 2007, 2009, 2013, 2014, 2023, 2024 Ricoh Company, Ltd.
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
const char moduleName[] = "";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/MessageStreamBuffer.h"
#include "Common/SystemParameter.h"

#include "Os/AutoCriticalSection.h"

#include "ModOsDriver.h"
#include "ModUnicodeString.h"

#include <stdio.h>

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace {
}

//
//	FUNCTION public
//	Common::MessageStreamData -- コンストラクタ
//
//	NOTES
//	コンストラクタ。データメンバーの初期化を行う
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
MessageStreamData::MessageStreamData()
	: m_nFlushCnt(0), m_iIsUse(0),
		m_iFlag(MessageStreamBuffer::LEVEL_DEBUG
				| MessageStreamBuffer::WRITE_HEADER
				| MessageStreamBuffer::WRITE_PROCESSID
				| MessageStreamBuffer::WRITE_THREADID)
{
}

//
//	FUNCTION public
//	Common::MessageStreamOutput -- コンストラクタ
//
//	NOTES
//	コンストラクタ
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
MessageStreamOutput::MessageStreamOutput()
	: m_pFp(0), m_bClose(false), m_eType(Normal)
{
}

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_cBufferCriticalSection
//								-- バッファ用のクリティカルセクション
//
//	NOTES
//	バッファの排他制御用のクリティカルセクション。
//
Os::CriticalSection
MessageStreamBuffer::m_cBufferCriticalSection;

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_cOutputCriticalSection
//								-- 出力用のクリティカルセクション
//
//	NOTES
//	出力の排他制御用のクリティカルセクション。
//
Os::CriticalSection
MessageStreamBuffer::m_cOutputCriticalSection;

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_cBufferEvent
//								-- バッファが解放されたかどうかの条件変数
//
//	NOTES
//	バッファがいっぱいの時に使用する条件変数
//
ModConditionVariable
MessageStreamBuffer::m_cBufferEvent(ModFalse, ModFalse);

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_iWaitThreadCount
//								-- バッファの解放を待っているスレッド数
//
//	NOTES
//	バッファの解放を待っているスレッド。
//
int
MessageStreamBuffer::m_iWaitThreadCount = 0;

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_mapStreamData
//								-- 使用中のバッファを格納するマップ
//
//	NOTES
//	メッセージストリームをため込むために使用しているバッファを格納するマップ。
//	スレッドIDがキーとなる。
//
MessageStreamBuffer::DataMap
MessageStreamBuffer::m_mapStreamData;

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_mapStreamOutput
//								-- ストリーム出力先を格納するマップ
//
//	NOTES
//	メッセージストリームの出力先を格納するマップ。パラメータ名がキーとなる。
//
MessageStreamBuffer::OutputMap
MessageStreamBuffer::m_mapStreamOutput;

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_mapParamter
//								-- パラメータ名とファイル名を格納するマップ
//
//	NOTES
//	キーにパラメータ名、値にファイル名を格納するマップ。
//
MessageStreamBuffer::ParameterMap
MessageStreamBuffer::m_mapParameter;

//
//	VARIABLE private
//	Common::MessageStreamBuffer::m_pMessageStreamData
//										-- メッセージバッファのプール
//
//	NOTES
//	メッセージストリームを一時的にためるバッファをプールする。
//
MessageStreamData
MessageStreamBuffer::m_pMessageStreamData[POOL_SIZE];

//
//	FUNCTION public
//	Common::MessageStreamBuffer::MessageStreamBuffer -- コンストラクタ
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
MessageStreamBuffer::MessageStreamBuffer()
{
}

//
//	FUNCTION public
//	MessageStreamBuffer::~MessageStreamBuffer -- デストラクタ
//
//	NOTES
//	デストラクタ
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
MessageStreamBuffer::~MessageStreamBuffer()
{
}

//
//	FUNCTION public
//	Common::MessageStreamBuffer::putChar -- 文字を加える
//
//	NOTES
//	バッファに文字を加える。
//	書き込み範囲は ( バッファサイズ - 1 ) である。
//	最後に '\n' を挿入するためである。
//
//	ARGUMENTS
//	char cChar_
//		文字
//
//	RETURN
//	int
//		加えた文字数
//
//	EXCEPTIONS
//	なし
//
int
MessageStreamBuffer::putChar(char cChar_)
{
	int iResult = 0;
	
	//バッファを得る
	MessageStreamData* pData = getData();
	if (pData->m_pszBuffer <
		(&pData->m_szBuffer[0] + MessageStreamData::BUFFER_SIZE - 2))
	{
		//バッファに余裕があるので書く

		*pData->m_pszBuffer++ = cChar_;
		*pData->m_pszBuffer = 0;
		iResult = 1;
	}
	else
	{
		//バッファに余裕が無い場合は改行コードを書く

		*pData->m_pszBuffer++ = '\n';
		*pData->m_pszBuffer = 0;
	}
	
	return iResult;
}

//
//	FUNCTION public
//	Common::MessageStreamBuffer::putString -- 文字列を加える
//
//	NOTES
//	バッファに文字列を加える
//	書き込み範囲は ( バッファサイズ - 1 ) である。
//	最後に '\n' を挿入するためである。
//
//	ARGUMENTS
//	const char* pszStr_
//		文字列
//	ModKanjiCode::KanjiCodeType code_
//		漢字コード
//
//	RETURN
//	int
//		加えた文字数
//
//	EXCEPTIONS
//	なし
//
int
MessageStreamBuffer::putString(const char* pszStr_,
							   ModKanjiCode::KanjiCodeType code_)
{
	int iResult = 0;
	
	//バッファを得る
	MessageStreamData* pData = getData();
	const char* p = pszStr_;
	int iBufSize = 0;
	
	while (*p != 0)
	{
		iBufSize = static_cast<int>(pData->m_pszBuffer - &pData->m_szBuffer[0]);

		// 何バイトで1文字が構成されているかチェックし、
		// 文字の途中で改行しないようにする

		int size = ModKanjiCode::getCharacterSize(static_cast<const unsigned char>(*p), code_);

		// 最後の null と次のバッファのための改行も加えて書けるかチェック

		if (iBufSize < (MessageStreamData::BUFFER_SIZE - (size + 1)))
		{
			// 書き出せるので書く

			for (int i = 0; i < size; ++i)
			{
				if (*p == 0)
					break;

				*pData->m_pszBuffer++ = *p++;
				iResult += 1;
			}
		}
		else
		{
			// もう書けないので、改行コードを入れて次のバッファにする

			*pData->m_pszBuffer++ = '\n';

			break;
		}
	}
	
	*pData->m_pszBuffer = 0;
	return iResult;
}

//
//	FUNCTION public
//	Common::MessageStreamBuffer::getData -- バッファを得る
//
//	NOTES
//	バッファを得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::MessageStreamData*
//		バッファ
//
//	EXCEPTIONS
//	なし
//
MessageStreamData*
MessageStreamBuffer::getData()
{
	MessageStreamData* pData = 0;
	if ((pData = getDataFromMap()) == 0)
	{
		//バッファを使用していないので、プールから得る
		pData = getDataFromPool();
		//使用中マップに格納する
		setDataToMap(pData);
	}
	return pData;
}

//
//	FUNCTION public
//	Common::MessageStreamBuffer::copyData -- バッファのコピーを得る
//
//	NOTES
//	バッファのコピーを得る
//	ただし Common::MessageStreamData のプロパティを得る為だけに存在する。
//	取得した Common::MessageStreamData を利用することはできない
//
//	ARGUMENTS
//	Common::MessageStreamData& cData
//		バッファデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
MessageStreamBuffer::copyData(MessageStreamData& cData)
{
	MessageStreamData* pData = 0;
	if ((pData = getDataFromMap()) == 0)
	{
		//バッファを使用していないので、プールから得る
		pData = getDataFromPool();
		//使用中マップに格納する
		setDataToMap(pData);
	}
	cData = *pData;
	{
		//使うな！！
		cData.m_iIsUse = false;
		cData.m_pszBuffer = 0;
	}
	return;
}

//
//	FUNCTION public
//	Common::MessageStreamBuffer::release -- バッファを解放する
//
//	NOTES
//	バッファを解放する
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
MessageStreamBuffer::releaseData()
{
	MessageStreamData* pData = getDataFromMap();
	if (pData)
	{
		ModThreadId id = ModOsDriver::Thread::self();	//スレッドID
		Os::AutoCriticalSection cAuto(m_cBufferCriticalSection);
		pData->m_iIsUse = 0;
		m_mapStreamData.erase(id);
		if (m_iWaitThreadCount)
		{
			//待っているスレッドがあるので、イベントを発生させる
			m_cBufferEvent.signal();
		}
	}
}

//
//	FUNCTION public
//	Common::MessageStreamBuffer::getOutput -- 出力先を得る
//
//	NOTES
//	得られる出力先はロックされている状態である
//	呼び出し元で、ロックを解除する必要がある
//
//	ARGUMENTS
//	const ModCharString& cstrParameterName_
//		パラメータ名
//
//	RETURN
//	Common::MessageStreamOutput*
//		出力先データ
//
//	EXCEPTIONS
//	なし
//
MessageStreamOutput*
MessageStreamBuffer::getOutput(const ModCharString& cstrParameterName_)
{
	ModCharString cstrFileName;
	Os::AutoCriticalSection cAuto(m_cOutputCriticalSection);

	//
	// パラメータ名 -> ファイル名
	//
	ParameterMap::Iterator i = m_mapParameter.find(cstrParameterName_);
	if (i == m_mapParameter.end())
	{
		ModUnicodeString u_cstrFileName;
		//見つからないのでパラメータをみる
		if (SystemParameter::getValue(cstrParameterName_,
									  u_cstrFileName) != true)
		{
			//パラメータがないので出力しない "0" を設定する
			cstrFileName = "0";

#ifdef DEBUG
			//DEBUG 時、 Error と Info は 出力する
			if ( (cstrParameterName_ == "Common_MessageOutputInfo") ||
				 (cstrParameterName_ == "Common_MessageOutputError") )
			{
				cstrFileName = "1";
			}
#endif

		}
		else if (u_cstrFileName.getLength() == 0)
		{
			//長さが0なので出力しない "0" を設定する
			cstrFileName = "0";

#ifdef DEBUG
			//DEBUG 時、 Error と Info は 出力する
			if ( (cstrParameterName_ == "Common_MessageOutputInfo") ||
				 (cstrParameterName_ == "Common_MessageOutputError") )
			{
				cstrFileName = "1";
			}
#endif

		}
		else
		{
			cstrFileName = u_cstrFileName.getString(Common::LiteralCode);
		}
		//マップに格納する
		m_mapParameter.insert(cstrParameterName_, cstrFileName);
	}
	else
	{
		//ファイル名を設定する
		cstrFileName = (*i).second;
	}

	//
	// ファイル名 -> MessageStreamOutput
	//
	MessageStreamOutput* pOutput = 0;
	OutputMap::Iterator j = m_mapStreamOutput.find(cstrFileName);
	if (j == m_mapStreamOutput.end())
	{
		//見つからないので作成する
		pOutput = new MessageStreamOutput;
		if (cstrFileName == "0")
		{
			//出力しない
		}
		else if (cstrFileName == "1")
		{
			//標準出力
			pOutput->m_pFp = stdout;
		}
		else if (cstrFileName == "2")
		{
			//標準エラー
			pOutput->m_pFp = stderr;
		}
		else if (cstrFileName == "ModMessage")
		{
			// ModMessage
			pOutput->m_eType = MessageStreamOutput::ModInfo;
		}
		else if (cstrFileName == "ModErrorMessage")
		{
			// ModErrorMessage
			pOutput->m_eType = MessageStreamOutput::ModError;
		}
		else if (cstrFileName == "ModDebugMessage")
		{
			// ModDebugMessage
			pOutput->m_eType = MessageStreamOutput::ModDebug;
		}
		else
		{
			// 通常のファイル
			pOutput->m_pFp = fopen(cstrFileName, "a");
			pOutput->m_bClose = true;
		}
		m_mapStreamOutput.insert(cstrFileName, pOutput);
	}
	else
	{
		pOutput = (*j).second;
	}

	// ロックしてから返す。呼び出し元で必ず unlock しなければならない
	pOutput->m_cCriticalSection.lock();

	return pOutput;
}

//
//	FUNCTION public static
//	MessageStreamBuffer::reset --　すべての出力先を再オープンする
//
//	NOTES
//	すべての出力先を再オープンする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
MessageStreamBuffer::reset()
{
	Os::AutoCriticalSection cAuto(m_cOutputCriticalSection);

	// パラメータ名 -> ファイル名のマップをクリアする
	// これにより、パラメータ値の変更に対応できるはず
	m_mapParameter.erase(m_mapParameter.begin(), m_mapParameter.end());

	// すべての通常ファイルをクローズ -> オープンする
	for (OutputMap::Iterator i = m_mapStreamOutput.begin();
		 i != m_mapStreamOutput.end(); ++i)
	{
		{
			// ロックする
			Os::AutoCriticalSection cAuto((*i).second->m_cCriticalSection);
		
			if ((*i).second->m_bClose == true)
			{
				// 通常ファイル
				
				if ((*i).second->m_pFp)
				{
					// オープンされているので、クローズする
					// FILE構造体をさわれるModがないので、
					// 標準ライブラリを直接呼び出す
					
					fclose(static_cast<FILE*>((*i).second->m_pFp));
				}

				// オープンする
				
				(*i).second->m_pFp = fopen((*i).first, "a");
			}
		}
	}
}

//
//	FUNCTION public static
//	MessageStreamBuffer::terminate --　すべての出力先をクローズする
//
//	NOTES
//	すべての出力先をクローズする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
MessageStreamBuffer::terminate()
{
	Os::AutoCriticalSection cAuto(m_cOutputCriticalSection);
	for (OutputMap::Iterator i = m_mapStreamOutput.begin();
		 i != m_mapStreamOutput.end(); ++i)
	{
		{
			//ロックする
			Os::AutoCriticalSection cAuto((*i).second->m_cCriticalSection);
		
			//ファイルをクローズする
			if ((*i).second->m_bClose == true && (*i).second->m_pFp)
				// FILE構造体をさわれるModがないので、
				// 標準ライブラリを直接呼び出す
				fclose(static_cast<FILE*>((*i).second->m_pFp));
		}
		
		delete (*i).second;
	}
	m_mapStreamOutput.erase(m_mapStreamOutput.begin(),
							m_mapStreamOutput.end());
}

//
//	FUNCTION private
//	Common::MessageStreamBuffer::getDataFromMap
//							-- 使用中のバッファデータをマップから取り出す
//
//	NOTES
//	使用中のバッファデータをマップから取り出す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::MessageStreamData*
//		使用中のバッファデータ。存在しない場合はnullを返す。
//
//	EXCEPTIONS
//	なし
//
MessageStreamData*
MessageStreamBuffer::getDataFromMap()
{
	MessageStreamData* pResult = 0;

	//スレッドIDを得る
	ModThreadId id = ModOsDriver::Thread::self();

	//ロックする
	Os::AutoCriticalSection cAuto(m_cBufferCriticalSection);

	//マップをスレッドIDで検索する
	DataMap::Iterator i = m_mapStreamData.find(id);
	if (i != m_mapStreamData.end())
		//存在したので、バッファを取り出す。
		pResult = (*i).second;

	return pResult;
}

//
//	FUNCTION private
//	Common::MessageStreamBuffer::setDataToMap
//							-- 使用中のバッファデータをマップに挿入する
//
//	NOTES
//	使用中のバッファデータをマップに挿入する。
//
//	ARGUMENTS
//	Common::MessageStreamData* pData_
//		マップに挿入するバッファデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
MessageStreamBuffer::setDataToMap(MessageStreamData* pData_)
{
	//スレッドIDを得る
	ModThreadId id = ModOsDriver::Thread::self();

	//ロックする
	Os::AutoCriticalSection cAuto(m_cBufferCriticalSection);

	//マップに挿入する
	m_mapStreamData.insert(id, pData_);
}

//
//	FUNCTION private
//	Common::MessageStreamBuffer::getDataFromPool
//						-- プールからバッファデータを取り出す
//
//	NOTES
//	プールからバッファデータを取り出す。
//	すべて使用中の場合は、m_cBufferCriticalSection を unlock して空くまで待つ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Common::MessageStreamData*
//		バッファデータ
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
MessageStreamData*
MessageStreamBuffer::getDataFromPool()
{
	MessageStreamData* pData = 0;

	//ロックする
	Os::AutoTryCriticalSection cAuto(m_cBufferCriticalSection, false);
	cAuto.lock();

	while (pData == 0)
	{
		//プールを探す
		for (int i = 0; i < POOL_SIZE; ++i)
		{
			if (m_pMessageStreamData[i].m_iIsUse == 0)
			{
				//使っていないのが見つかった。
				pData = &m_pMessageStreamData[i];
				ModOsDriver::Memory::set(pData, 0, sizeof(MessageStreamData));
				pData->m_iIsUse = 1;
				pData->m_iFlag = (LEVEL_DEBUG |
								  WRITE_HEADER |
								  WRITE_PROCESSID |
								  WRITE_THREADID);
				pData->m_pszBuffer = &pData->m_szBuffer[0];
				break;
			}
		}
		if (pData == 0)
		{
			//見つからなかったので、解放されるまで待つ
			m_iWaitThreadCount++;
			cAuto.unlock();
			m_cBufferEvent.wait();
			cAuto.lock();
			m_iWaitThreadCount--;
		}
	}

	return pData;
}

//
//	Copyright (c) 2000, 2001, 2003, 2005, 2007, 2009, 2013, 2014, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
