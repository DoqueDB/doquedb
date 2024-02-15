// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MessageStream.h -- ログメッセージ出力用クラス
// 
// Copyright (c) 2000, 2004, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_MESSAGESTREAM_H
#define __TRMEISTER_COMMON_MESSAGESTREAM_H

#include "Common/Module.h"
#include "Common/ExceptionMessage.h"
#include "Common/MessageStreamBuffer.h"

#include "ModKanjiCode.h"
#include "ModOstream.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Common::MessageStream -- ログメッセージ出力用クラス
//
//	NOTES
//	ログメッセージを出力するクラス。
//
class SYD_COMMON_FUNCTION MessageStream : public ModOstream
{
public:
	//コンストラクタ
	MessageStream();
	//デストラクタ
	virtual ~MessageStream();

	//文字を追加
	ModOstream& put(const char cChar_);
	ModOstream& put(const ModUnicodeChar cChar_);

	//文字列を追加
	ModOstream& write(const char* pszStr_);
	ModOstream& write(const ModUnicodeChar* pszStr_);

	//フラッシュする
	void flush();

	//モジュール名を設定する
	void setModuleName(const char* pszModuleName_);
	//ファイル名を設定する
	void setFileName(const char* pszFileName_);
	//行番号を設定する
	void setLineNumber(int iLineNumber_);
	//パラメータ名を設定する
	void setParameterName(const char* pszParameterName_);

	//フラグの値を得る
	int getFlag();

	//フラグを設定する
	int setFlag(int iFlag_);

	//出力するかどうか
	bool isOutput();

	//MessageStreamData のプロパティを設定する
	void setData(const MessageStreamData& cData_);

	// スレッドIDを表示するようにする
	MessageStream& onThread();

	// スレッドIDを表示しないようにする
	MessageStream& offThread();

	// 時刻を表示するようにする
	MessageStream& onDate();

	// 時刻を表示しないようにする
	MessageStream& offDate();

private:
	// 書き出す
	ModOstream& write(const char* pszStr_, ModKanjiCode::KanjiCodeType code_);
	
	//ディレクトリ部分をのぞいたファイル名を得る
	const char* getBaseName(const char* pszSrcName_);

	//ログメッセージのバッファ
	MessageStreamBuffer m_cStreamBuffer;
};

//
//	CLASS
//	Common::MessageStreamSelection -- メッセージストリームのインスタンスを得る
//
//	NOTES
//	メッセージストリームのインスタンスを得る。
//
class MessageStreamSelection
{
public:
	//メッセージストリームのインスタンスを得る
	SYD_COMMON_FUNCTION
	static MessageStream& getInstance(const char* pszModuleName_,
									  const char* pszFileName_,
									  int iLineNumber_,
									  const char* pszParameterName_,
									  int iFlag = 0);

private:
	//唯一のインスタンス
	static MessageStream m_cMessageStream;
	// 日時を出力するか？
	static int m_iIsDate;
};

_TRMEISTER_COMMON_END
_TRMEISTER_END

//
//	DEFINE
//	_TRMEISTER_MESSAGE --メッセージストリームのインスタンスを得る
//
//	NOTES
//	ソースファイル中に moduleName, srcFile が定義されていることが前提
//
//	ARGUMENTS
//	param
//			パラメータ名
//
//	level
//			ログレベル
//
#ifdef DEBUG
#define _TRMEISTER_MESSAGE(param, level)	\
			Common::MessageStreamSelection::getInstance(\
									moduleName, \
									srcFile, \
									__LINE__, \
									param, \
									level)
#else
#define _TRMEISTER_MESSAGE(param, level)	\
			if (Common::MessageStreamSelection::getInstance(\
									moduleName, \
									srcFile, \
									__LINE__, \
									param, \
									level).isOutput() == false) \
				; \
			else \
				Common::MessageStreamSelection::getInstance(\
									moduleName, \
									srcFile, \
									__LINE__, \
									param, \
									level)
#endif

#define _SYDNEY_MESSAGE _TRMEISTER_MESSAGE

#endif //__TRMEISTER_COMMON_MESSAGESTREAM_H

//
//	Copyright (c) 2000, 2004, 2007, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
