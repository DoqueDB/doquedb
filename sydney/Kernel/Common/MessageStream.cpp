// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MessageStream.cpp -- ログメッセージ出力用クラス
// 
// Copyright (c) 2000, 2002, 2003, 2005, 2006, 2007, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Common/Message.h"
#include "Common/MessageStream.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Process.h"
#include "Os/Thread.h"

#include "ModKanjiCode.h"
#include "ModCharTrait.h"
#include "ModMessage.h"
#include "ModTime.h"

#include <stdio.h>

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
	//
	//	VARIABLE
	//	_$::_pszOutputDate
	//
	const ModUnicodeString _pszOutputDate("Common_MessageIncludeDate");

	//
	//	VARIABLE
	//	_$::_pszOutputCode
	//
	const ModUnicodeString _pszOutputCode("Common_MessageOutputCode");

	const ModUnicodeChar _pszUTF8[]		= {'u','t','f','8',0};
	const ModUnicodeChar _pszUTF_8[]	= {'u','t','f','-','8',0};
	const ModUnicodeChar _pszSJIS[]		= {'s','j','i','s',0};
	const ModUnicodeChar _pszEUC[]		= {'e','u','c',0};

	//
	//	VARIABLE
	//	_$::_iCode -- メッセージの出力コード
	//
	ModKanjiCode::KanjiCodeType _eCode = ModKanjiCode::unknown;

	//
	//	VARIABLE
	//	_$::_Latch -- 出力コード用のラッチ
	//
	Os::CriticalSection	_latch;

	//
	//	FUNCTION
	//	_$::_getOutputCode -- 出力コードを得る
	//
	ModKanjiCode::KanjiCodeType _getOutputCode()
	{
		if (_eCode == ModKanjiCode::unknown)
		{
			Os::AutoCriticalSection cAuto(_latch);

			if (_eCode == ModKanjiCode::unknown)
			{
				// パラメータを取得する
				ModUnicodeString code
					= SystemParameter::getString(_pszOutputCode);
				if (code.compare(_pszUTF8, ModFalse) == 0 ||
					code.compare(_pszUTF_8, ModFalse) == 0)
					_eCode = ModKanjiCode::utf8;
				else if (code.compare(_pszEUC, ModFalse) == 0)
					_eCode = ModKanjiCode::euc;
				else if (code.compare(_pszSJIS, ModFalse) == 0)
					_eCode = ModKanjiCode::shiftJis;
				else
					_eCode = Common::LiteralCode;
			}
		}
		return _eCode;
	}

	// Os::CriticalSection を unlock するクラス
	class _AutoUnlock
	{
	public:
		_AutoUnlock(Os::CriticalSection& latch_)
			: m_cLatch(latch_), m_bLock(true) {}
		~_AutoUnlock()
			{ if (m_bLock) m_cLatch.unlock(); }

		void unlock()
			{
				if (m_bLock) m_cLatch.unlock();
				m_bLock = false;
			}

	private:
		Os::CriticalSection& m_cLatch;
		bool m_bLock;
	};
	
}

//	FUNCTION public
//	Common::MessageStream::MessageStream -- コンストラクタ
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

MessageStream::MessageStream()
{}

//	FUNCTION public
//	Common::MessageStream::~MessageStream -- デストラクタ
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

MessageStream::~MessageStream()
{}

//
//	FUNCTION public
//	Common::MessageStream::put -- 文字を追加する
//
//	NOTES
//	文字を追加する。
//
//	ARGUMENTS
//	const char cChar_
//		追加する文字
//
//	RETURN
//	ModOstream&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
ModOstream&
MessageStream::put(const char cChar_)
{
	if ( m_cStreamBuffer.putChar(cChar_) == 0 )
	{
		//書き込めなかった == buffer is fullならプロパティを保存
		MessageStreamData cData;
		m_cStreamBuffer.copyData(cData);

		//一度フラッシュする
		flush();

		//使用していた Buffer は消えたので再登録
		cData.m_nFlushCnt++;
		setData(cData);

		//もう一度チャレンジ
		m_cStreamBuffer.putChar(cChar_);
	}
	return *this;
}

//
//	FUNCTION public
//	Common::MessageStream::put -- 文字を追加する
//
//	NOTES
//	文字を追加する。
//
//	ARGUMENTS
//	const ModUnicodeChar cChar_
//		追加する文字
//
//	RETURN
//	ModOstream&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
ModOstream&
MessageStream::put(const ModUnicodeChar cChar_)
{
	ModUnicodeString str(cChar_);
	ModKanjiCode::KanjiCodeType code = _getOutputCode();

	return write(str.getString(code), code);
}

//
//	FUNCTION public
//	Common::MessageStream::write -- 文字列を追加する
//
//	NOTES
//	文字列を追加する。
//
//	ARGUMENTS
//	const char* pszStr_
//		追加する文字列
//
//	RETURN
//	ModOstream&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
ModOstream&
MessageStream::write(const char* pszStr_)
{
	ModUnicodeString str(pszStr_, Common::LiteralCode);
	ModKanjiCode::KanjiCodeType code = _getOutputCode();
	
	return write(str.getString(code), code);
}

//
//	FUNCTION public
//	Common::MessageStream::write -- 文字列を追加する
//
//	NOTES
//	文字列を追加する
//
//	ARGUMENTS
//	const ModUnicodeChar* pszStr_
//		追加する文字列
//
//	RETURN
//	ModOstream&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
ModOstream&
MessageStream::write(const ModUnicodeChar* pszStr_)
{
	ModUnicodeString str(pszStr_);
	ModKanjiCode::KanjiCodeType code = _getOutputCode();

	return write(str.getString(code), code);
}

//
//	FUNCTION public
//	Common::MessageStream::flush -- メッセージを書き出す
//
//	NOTES
//	メッセージを書き出す。ModEndlやModFlushと呼ばれる実行されるメソッド。
//	ここでは、指定のファイルにログメッセージを出力する。
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
MessageStream::flush()
{
	//書き出すファイルを取得する
	MessageStreamData* pData = m_cStreamBuffer.getData();
	MessageStreamOutput* pOutput = m_cStreamBuffer.getOutput(
		pData->m_szParameterName);

	// 自動的にunlockする
	_AutoUnlock cAuto(pOutput->m_cCriticalSection);

	FILE* fp = static_cast<FILE*>(pOutput->m_pFp);

	if (fp == 0 && pOutput->m_eType == MessageStreamOutput::Normal)
	{
		// 先に出力先をunlockする
		cAuto.unlock();
		//書き出す先がないのでバッファを解放して終了する
		m_cStreamBuffer.releaseData();
		return;
	}

	if (pOutput->m_eType == MessageStreamOutput::Normal)
	{

		//日時
		if (pData->m_iFlag & MessageStreamBuffer::WRITE_DATE)
		{
			ModCharString cString
				= ModTime::getCurrentTime().getString(ModTrue);
			fprintf(fp, "%s ", cString.getString());
		}

		if (pData->m_iFlag & MessageStreamBuffer::WRITE_PROCESSID
			|| pData->m_iFlag & MessageStreamBuffer::WRITE_THREADID)
		{
			fprintf(fp, "(");
			
			//プロセスID
			if (pData->m_iFlag & MessageStreamBuffer::WRITE_PROCESSID)
			{
				fprintf(fp, "%u", Os::Process::self());
			}

			if (pData->m_iFlag & MessageStreamBuffer::WRITE_PROCESSID
				&& pData->m_iFlag & MessageStreamBuffer::WRITE_THREADID)
			{
				fprintf(fp, ":");
			}

			//スレッドID
			if (pData->m_iFlag & MessageStreamBuffer::WRITE_THREADID)
			{
				fprintf(fp, "%u", Message::getThreadID());
			}

			fprintf(fp, ") ");
		}

		//モジュール名、ファイル名、行番号
		if (pData->m_iFlag & MessageStreamBuffer::WRITE_HEADER)
		{
			fprintf(fp, "%s::%s %d: ",
					pData->m_szModuleName, pData->m_szFileName,
					pData->m_iLineNumber);
		}

		//フラッシュカウンタ
		ModCharString strFlushCnt;
		if ( pData->m_nFlushCnt > 0 )
		{
			//０以上の時に作成する
			strFlushCnt.format(" %d", pData->m_nFlushCnt);
		}

		//ログレベル
		if (pData->m_iFlag & MessageStreamBuffer::LEVEL_ERROR)
		{
			fprintf(fp, "[ERROR");
			fprintf(fp, strFlushCnt);
			fprintf(fp, "] ");
		}
		else if (pData->m_iFlag & MessageStreamBuffer::LEVEL_INFO)
		{
			fprintf(fp, "[INFO");
			fprintf(fp, strFlushCnt);
			fprintf(fp, "] ");
		}
		else
		{
			fprintf(fp, "[DEBUG");
			fprintf(fp, strFlushCnt);
			fprintf(fp, "] ");
		}

		//内容
		fprintf(fp, "%s", pData->m_szBuffer);

		fflush(fp);
	}
	else
	{
		switch (pOutput->m_eType)
		{
		case MessageStreamOutput::ModInfo:
			{
				ModMessageSelection::normal(pData->m_szFileName,
											pData->m_iLineNumber).getStream()
					<< "(" << pData->m_szModuleName << ") "
					<< pData->m_szBuffer << ModFlush;
			}
			break;
		case MessageStreamOutput::ModError:
			{
				ModMessageSelection::error(pData->m_szFileName,
										   pData->m_iLineNumber).getStream()
					<< "(" << pData->m_szModuleName << ") "
					<< pData->m_szBuffer << ModFlush;
			}
			break;
		case MessageStreamOutput::ModDebug:
			{
				ModMessageSelection::debug(pData->m_szFileName,
										   pData->m_iLineNumber).getStream()
					<< "(" << pData->m_szModuleName << ") "
					<< pData->m_szBuffer << ModFlush;
			}
			break;
		}
	}

	// 先に出力先をunlockする
	cAuto.unlock();
		
	//バッファを解放する
	m_cStreamBuffer.releaseData();
}

//
//	FUNCTION public
//	Common::MessageStream::setModuleName -- モジュール名を設定する
//
//	NOTES
//	モジュール名をバッファに設定する。
//
//	ARGUMENTS
//	const char* pszModuleName
//		モジュール名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
MessageStream::setModuleName(const char* pszModuleName_)
{
	MessageStreamData* pData = m_cStreamBuffer.getData();
	ModCharTrait::copy(pData->m_szModuleName, pszModuleName_);
}

//
//	FUNCTION public
//	Common::MessageStream::setFileName -- ファイル名を設定する
//
//	NOTES
//	ファイル名をバッファに設定する。
//
//	ARGUMENTS
//	const char* pszFileName
//		ファイル名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
MessageStream::setFileName(const char* pszFileName_)
{
	MessageStreamData* pData = m_cStreamBuffer.getData();
	const char* p = getBaseName(pszFileName_);
	ModCharTrait::copy(pData->m_szFileName, p);
}

//
//	FUNCTION public
//	Common::MessageStream::setLineNumber -- 行番号を設定する
//
//	NOTES
//	行番号をバッファに設定する。
//
//	ARGUMENTS
//	int iLineNumber_
//		行番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
MessageStream::setLineNumber(int iLineNumber_)
{
	MessageStreamData* pData = m_cStreamBuffer.getData();
	pData->m_iLineNumber = iLineNumber_;
}

//
//	FUNCTION public
//	Common::MessageStream::setParameterName -- パラメータ名を設定する
//
//	NOTES
//	パラメータ名名をバッファに設定する。
//
//	ARGUMENTS
//	const char* pszParameterName
//		パラメータ名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
MessageStream::setParameterName(const char* pszParameterName_)
{
	MessageStreamData* pData = m_cStreamBuffer.getData();
	ModCharTrait::copy(pData->m_szParameterName, pszParameterName_);
}

//
//	FUNCTION public
//	Common::MessageStream::getFlag -- フラグの値を得る
//
//	NOTES
//	フラグの値を得る。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		設定されていたフラグ
//
//	EXCEPTIONS
//	なし
//
int
MessageStream::getFlag()
{
	MessageStreamData* pData = m_cStreamBuffer.getData();
	return pData->m_iFlag;
}

//
//	FUNCTION public
//	Common::MessageStream::setFlag -- フラグを設定する
//
//	NOTES
//	フラグを設定する。
//
//	ARGUMENTS
//	int iFlag_
//		設定するフラグ
//
//	RETURN
//	int
//		設定されていたフラグ
//
//	EXCEPTIONS
//	なし
//
int
MessageStream::setFlag(int iFlag_)
{
	MessageStreamData* pData = m_cStreamBuffer.getData();
	int iFlag = pData->m_iFlag;
	pData->m_iFlag = iFlag_;

	return iFlag;
}

//
//	FUNCTION public
//	Common::MessageStream::isOutout -- メッセージを書き出すかどうかを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		メッセージを書き出す場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MessageStream::isOutput()
{
	//書き出すファイルを取得する
	MessageStreamData* pData = m_cStreamBuffer.getData();
	MessageStreamOutput* pOutput = m_cStreamBuffer.getOutput(
		pData->m_szParameterName);

	// 自動的にunlockする
	_AutoUnlock cAuto(pOutput->m_cCriticalSection);

	if (pOutput->m_pFp == 0 && pOutput->m_eType == MessageStreamOutput::Normal)
	{
		// 先に出力先をunlockする
		cAuto.unlock();
		//書き出す先がないのでバッファを解放する
		m_cStreamBuffer.releaseData();
		return false;
	}
	return true;
}

//
//	FUNCTION public
//	Common::MessageStream::setData -- MessageStreamData のプロパティを設定する
//
//	NOTES
//	MessageStreamData のプロパティを設定する。
//
//	ARGUMENTS
//	const MessageStareamData& cData_
//		設定する MessageStareamData
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
MessageStream::setData(const MessageStreamData& cData_)
{
	MessageStreamData* pData = m_cStreamBuffer.getData();

	pData->m_nFlushCnt = cData_.m_nFlushCnt;
	ModCharTrait::copy(pData->m_szModuleName,	  cData_.m_szModuleName);

	const char* p = getBaseName(cData_.m_szFileName);
	ModCharTrait::copy(pData->m_szFileName,		  p);

	ModCharTrait::copy(pData->m_szParameterName, cData_.m_szParameterName);
	pData->m_iLineNumber = cData_.m_iLineNumber;
	pData->m_iFlag |= cData_.m_iFlag;
}

//
//	FUNCTION public static
//	Common::MessageStreamSelection::onThread
//						-- スレッドIDを表示するようにする
//
//	NOTES
//		スレッドIDを表示するようにする。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	Common::MessageStream&
//		メッセージストリームのインスタンス
//
//	EXCEPTIONS
//		なし
//
MessageStream&
MessageStream::onThread()
{
	setFlag(getFlag() | MessageStreamBuffer::WRITE_THREADID);
	return *this;
}

//
//	FUNCTION public static
//	Common::MessageStreamSelection::offThread
//						-- スレッドIDを表示しないようにする
//
//	NOTES
//		スレッドIDを表示しないようにする。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	Common::MessageStream&
//		メッセージストリームのインスタンス
//
//	EXCEPTIONS
//		なし
//
MessageStream&
MessageStream::offThread()
{
	// not !, but ~.
	setFlag(getFlag() & ~(MessageStreamBuffer::WRITE_THREADID));
	return *this;
}

//
//	FUNCTION public static
//	Common::MessageStreamSelection::onDate
//						-- 時刻を表示するようにする
//
//	NOTES
//		時刻を表示するようにする。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	Common::MessageStream&
//		メッセージストリームのインスタンス
//
//	EXCEPTIONS
//		なし
//
MessageStream&
MessageStream::onDate()
{
	setFlag(getFlag() | MessageStreamBuffer::WRITE_DATE);
	return *this;
}

//
//	FUNCTION public static
//	Common::MessageStreamSelection::offDate
//						-- 時刻を表示しないようにする
//
//	NOTES
//		時刻を表示しないようにする。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	Common::MessageStream&
//		メッセージストリームのインスタンス
//
//	EXCEPTIONS
//		なし
//
MessageStream&
MessageStream::offDate()
{
	// not !, but ~.
	setFlag(getFlag() & ~(MessageStreamBuffer::WRITE_DATE));
	return *this;
}

//
//	FUNCTION private
//	Common::MessageStream::write -- 文字列を追加する
//
//	NOTES
//	文字列を追加する。
//
//	ARGUMENTS
//	const char* pszStr_
//		追加する文字列
//	ModKanjiCode::KanjiCodeType
//		追加する文字列の漢字コード
//
//	RETURN
//	ModOstream&
//		自分自身の参照
//
//	EXCEPTIONS
//	なし
//
ModOstream&
MessageStream::write(const char* pszStr_, ModKanjiCode::KanjiCodeType code_)
{
	int iStrLen = ModCharTrait::length(pszStr_);
	int iWriteLen = 0;

	const char* p = pszStr_;

	//全て処理するまで続ける
	while ( iWriteLen < iStrLen )
	{
		iWriteLen += m_cStreamBuffer.putString(p, code_);
		p = pszStr_ + iWriteLen;

		if ( iWriteLen < iStrLen )
		{
			//書き込めなかった == buffer is fullならプロパティを保存
			MessageStreamData cData;
			m_cStreamBuffer.copyData(cData);

			//一度フラッシュする
			flush();

			//使用していた Buffer は消えたので再登録
			cData.m_nFlushCnt++;
			setData(cData);
		}
	}
	return *this;
}

//
//	FUNCTION private
//	Common::MessageStream::getBaseName -- ファイル名からディレクトリ部分を除く
//
//	NOTES
//	ファイル名からディレクトリ部分を除く。
//
//	ARGUMENTS
//	const char* pszSrcName_
//		取除く対象の文字列
//
//	RETURN
//	const char*
//		ディレクトリ等を除いた部分の先頭のポインタ
//
//	EXCEPTIONS
//	なし
//
const char*
MessageStream::getBaseName(const char* pszSrcName_)
{
	return Message::getBaseName(pszSrcName_);
}

//
//	VARIABLE private
//	Common::MessageStreamSelection::m_cMessageStream
//				-- メッセージストリームのSydney唯一のインスタンス
//
//	NOTES
//	メッセージストリームのSydney唯一のインスタンス
//
MessageStream
MessageStreamSelection::m_cMessageStream;

//
//	VARIABLE private
//	Common::MessageStreamSelection::m_iIsDate
//				-- 日時を出力するか
//
//	NOTES
//
int
MessageStreamSelection::m_iIsDate = -1;

//
//	FUNCTION public static
//	Common::MessageStreamSelection::getInstance
//						-- メッセージストリームのインスタンスを得る
//
//	NOTES
//	メッセージストリームのSydney唯一のインスタンスを得る
//
//	ARGUMENTS
//	const char* pszModuleName_
//		モジュール名
//	const char* pszFileName_
//		ファイル名
//	int iLineNumber_
//		行番号
//	const char* pszParameterName_
//		パラメータ名
//	int iFlag
//		追加するフラグ(default 0)
//
//	RETURN
//	Common::MessageStream&
//		メッセージストリームのインスタンス
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
MessageStream&
MessageStreamSelection::getInstance(const char* pszModuleName_,
									const char* pszFileName_,
									int iLineNumber_,
									const char* pszParameterName_,
									int iFlag)
{
	m_cMessageStream.setModuleName(pszModuleName_);
	m_cMessageStream.setFileName(pszFileName_);
	m_cMessageStream.setLineNumber(iLineNumber_);
	m_cMessageStream.setParameterName(pszParameterName_);
	int flag = m_cMessageStream.setFlag(0);
	flag |= iFlag;
	m_cMessageStream.setFlag(flag);

	if (m_iIsDate == -1)
	{
		// パラメータを取得する
		if (SystemParameter::getBoolean(_pszOutputDate) == true)
			m_iIsDate = 1;
		else
			m_iIsDate = 0;
	}

	if (m_iIsDate) m_cMessageStream.onDate();

	return m_cMessageStream;
}

//
//	Copyright (c) 2000, 2002, 2003, 2005, 2006, 2007, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
