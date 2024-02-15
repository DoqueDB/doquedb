// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MessageStreamBuffer.h -- メッセージストリーム用のバッファクラス
// 
// Copyright (c) 2000, 2004, 2005, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_MESSAGESTREAMBUFFER_H
#define __TRMEISTER_COMMON_MESSAGESTREAMBUFFER_H

#include "Common/Object.h"
#include "Common/UnicodeString.h"
#include "Os/CriticalSection.h"
#include "ModMap.h"
#include "ModConditionVariable.h"
#include "ModCharString.h"
#include "ModKanjiCode.h"

_TRMEISTER_BEGIN

namespace Common
{

//
//	CLASS
//	Common::MessageStreamData -- メッセージストリーム用バッファのデータ
//
//	NOTES
//	メッセージストリーム用バッファのデータ
//
class MessageStreamData
{
public:
	enum { BUFFER_SIZE = 1024 * 2 };
	enum { FILE_SIZE = 32 };
	enum { PARAMETER_SIZE = 128 };

	//コンストラクタ
	MessageStreamData();

	unsigned int m_nFlushCnt;				//バッファフラッシュカウンタ

	int m_iIsUse;							//使用中かどうか
	char m_szModuleName[FILE_SIZE];			//モジュール名
	char m_szFileName[FILE_SIZE];			//ファイル名
	int m_iLineNumber;						//行番号
	int m_iFlag;							//フラグ等
	char m_szParameterName[PARAMETER_SIZE];	//パラメータ名
	char m_szBuffer[BUFFER_SIZE];			//バッファ
	char* m_pszBuffer;						//書き込むバッファの位置
};

//
//	CLASS
//	Common::MessageStreamOutput -- メッセージストリームの出力先のデータ
//
//	NOTES
//	メッセージストリームの出力先のデータ
//
class MessageStreamOutput : public Common::Object
{
public:
	enum Type
	{
		Normal,
		ModInfo,
		ModError,
		ModDebug
	};

	//コンストラクタ
	MessageStreamOutput();

	//出力先のファイル構造体
	void* m_pFp;
	//クローズするべきかどうか
	bool m_bClose;
	//排他制御用のクリティカルセクション
	Os::CriticalSection m_cCriticalSection;
	// メッセージの種類
	Type m_eType;
};

//
//	CLASS
//	Common::MessageStreamBuffer -- メッセージストリーム用バッファクラス
//
//	NOTES
//	メッセージストリーム用のバッファクラス。
//
class MessageStreamBuffer : public Common::Object
{
public:
	//バッファをプールする数
	enum { POOL_SIZE = 64 };

	//フラグ類
	enum
	{
		//ログレベル
		LEVEL_DEBUG = (1 << 0),		//DEBUG
		LEVEL_INFO = (1 << 1),		//INFO
		LEVEL_ERROR = (1 << 2),		//ERROR

		//ヘッダー(モジュール名、ファイル名、行番号)を書き出す
		WRITE_HEADER = (1 << 8),
		//日時を書き出す
		WRITE_DATE = (1 << 9),
		//プロセスIDを書き出す
		WRITE_PROCESSID = (1 << 10),
		//スレッドIDを書き出す
		WRITE_THREADID = (1 << 11)
	};

	//コンストラクタ
	MessageStreamBuffer();
	//デストラクタ
	virtual ~MessageStreamBuffer();

	//文字を加える
	int putChar(char cChar_);
	//文字列を加える
	int putString(const char* pszStr_, ModKanjiCode::KanjiCodeType code_);

	//バッファを指すポインタを得る
	MessageStreamData* getData();
	//バッファのコピーを得る
	void copyData(MessageStreamData& cData);
	//バッファを解放する
	void releaseData();

	//出力先を得る
	MessageStreamOutput* getOutput(const ModCharString& cstrParameterName_);

	//すべての出力先を再オープンする
	static void reset();
	//すべての出力先をクローズする
	static void terminate();

	//
	//	TYPEDEF
	//	Common::MessageStreamBuffer::DataMap
	//
	//	NOTES
	//	使用中のバッファデータを格納するマップ。キーはスレッドID
	//
	typedef ModMap<ModThreadId, MessageStreamData*, ModLess<ModThreadId> >
		DataMap;

	//
	//	TYPEDEF
	//	Common::MessageStreamBuffer::OutputMap
	//
	//	NOTES
	//	使用中の出力先データを格納するマップ。キーはファイル名
	//	ただし、stdout は "1"、stderr は "2" になる。
	//
	typedef
		ModMap<ModCharString, MessageStreamOutput*, ModLess<ModCharString> >
			OutputMap;

	//
	//	TYPEDEF
	//	Common::MessageStreamBuffer::ParameterMap
	//
	//	NOTES
	//	使用中の出力先パラメータを格納するマップ。キーはパラメータ名
	//	値はファイル名。
	//
	typedef ModMap<ModCharString, ModCharString, ModLess<ModCharString> >
		ParameterMap;

private:
	//使用中のバッファデータをマップから取り出す
	MessageStreamData* getDataFromMap();
	//使用中のバッファデータをマップに挿入する
	void setDataToMap(MessageStreamData* pData_);
	//未使用のバッファデータをプールから取り出す
	MessageStreamData* getDataFromPool();
	//char に続くバイトが何バイトで１文字を得る
	static int getCharSize(const char ch_, ModKanjiCode::KanjiCodeType code_);

	//バッファ用のクリティカルセクション
	static Os::CriticalSection m_cBufferCriticalSection;
	//出力用のクリティカルセクション
	static Os::CriticalSection m_cOutputCriticalSection;
	//条件変数
	static ModConditionVariable m_cBufferEvent;
	//バッファの解放を待っているスレッド数
	static int m_iWaitThreadCount;

	//使用中のバッファデータを格納するマップ
	static DataMap m_mapStreamData;
	//出力先データを格納するマップ
	static OutputMap m_mapStreamOutput;
	//パラメータ名とファイル名を格納するマップ
	static ParameterMap m_mapParameter;

	//バッファデータのプール
	static MessageStreamData m_pMessageStreamData[POOL_SIZE];
};

}

_TRMEISTER_END

#endif //__TRMEISTER_COMMON_MESSAGESTREAMBUFFER_H

//
//	Copyright (c) 2000, 2004, 2005, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
