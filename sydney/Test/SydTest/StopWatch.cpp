// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StopWatch.cpp -- コマンドの実行時間を計測するクラス
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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
#include "ModAutoMutex.h"
#include "SydTest/StopWatch.h"
#include "SydTest/SydTestMessage.h"

_SYDNEY_USING

using namespace SydTest;

//
//	FUNCTION public
//  StopWatch::start -- 特定の時計を開始させる
//
//	NOTES
//    特定の時計を開始させる
//
//	ARGUMENTS
//    ModUnicodeString pszLabel_
//      時計の名前
//
//	RETURN
//    bool
//      true: 開始成功、 false: 開始失敗、あるいは既に動いている
//
//	EXCEPTIONS
//    なし
//
bool
StopWatch::start(ModUnicodeString pszLabel_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	// まず該当の時計があるかどうか調べる
	if (m_cMap.exists(pszLabel_)) {
		// 止まっていれば再開
		if (m_cMap[pszLabel_].dStart == 0)
		{
			m_cMap[pszLabel_].dStart = ModTime::getCurrentTime();
			return true;
		} else {
			// そうでなければエラー
			return false;
		}
	} else { 
		// なければ作る
		bool bResult = m_cMap.insert(pszLabel_, _Watch());
		m_cMap[pszLabel_].dLap = 0;
		m_cMap[pszLabel_].dStart = ModTime::getCurrentTime();
		return bResult;
	}
}

//
//	FUNCTION public
//  StopWatch::stop -- 特定の時計を終了させる
//
//	NOTES
//    特定の時計を開始させる
//
//	ARGUMENTS
//    ModUnicodeString pszLabel_
//      時計の名前
//
//	RETURN
//    bool
//      true: 終了成功、 false: 終了失敗、あるいは既に止まっている
//
//	EXCEPTIONS
//    なし
//
bool
StopWatch::stop(ModUnicodeString pszLabel_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	if (m_cMap.exists(pszLabel_)) {
		// あれば止める
		if (m_cMap[pszLabel_].dStart != 0) {
			m_cMap[pszLabel_].dLap += 
				(ModTime::getCurrentTime() - m_cMap[pszLabel_].dStart);
			m_cMap[pszLabel_].dStart = 0;
			return true;
		} else {
#ifdef DEBUG
			if (isSydTestDebugMessage())
			{
				SydTestDebugMessage << "The watch " << pszLabel_
					<< " already stops." << ModEndl;
			}
#endif
			return false;
		}
	} else { 
			// なければエラー
#ifdef DEBUG
		if (isSydTestDebugMessage())
		{
			SydTestDebugMessage << "No such Watch "
				<< pszLabel_ << "." << ModEndl;
		}
#endif
		return false;
	}
}

//
//	FUNCTION public
//  StopWatch::showTotalTime -- 特定の時計の累計時間を返す
//
//	NOTES
//    特定の時計の累計時間を返す
//
//	ARGUMENTS
//    ModUnicodeString pszLabel_
//      時計の名前
//
//	RETURN
//    const ModTimeSpan
//      累計時間
//
//	EXCEPTIONS
//    なし
//
const ModTimeSpan
StopWatch::showTotalTime(ModUnicodeString pszLabel_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	return  m_cMap[pszLabel_].dLap + showCurrentLapTime(pszLabel_);
}

//
//	FUNCTION public
//  StopWatch::showCurrentLapTime -- 特定の時計の現在のラップを返す
//
//	NOTES
//    特定の時計の現在のラップを返す
//
//	ARGUMENTS
//    ModUnicodeString pszLabel_
//      時計の名前
//
//	RETURN
//    const ModTimeSpan
//      ラップタイム
//
//	EXCEPTIONS
//    なし
//
const ModTimeSpan
StopWatch::showCurrentLapTime(ModUnicodeString pszLabel_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	if (m_cMap[pszLabel_].dStart == 0) {
		return 0;
	} else {
		return ModTime::getCurrentTime() - m_cMap[pszLabel_].dStart;
	}
}

//
//	FUNCTION public
//  StopWatch::reset -- 特定の時計の現ラップをリセットする
//
//	NOTES
//    特定の時計の現ラップをリセットする
//
//	ARGUMENTS
//    ModUnicodeString pszLabel_
//      時計の名前
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void
StopWatch::reset(ModUnicodeString pszLabel_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	m_cMap[pszLabel_].dLap = 0;
}

//
//	FUNCTION public
//  StopWatch::clear -- 特定の時計を削除する
//
//	NOTES
//    特定の時計を削除
//
//	ARGUMENTS
//    ModUnicodeString pszLabel_
//      時計の名前
//
//	RETURN
//    bool
//      true: 削除成功、false: 削除失敗
//
//	EXCEPTIONS
//    なし
//
bool
StopWatch::clear(ModUnicodeString pszLabel_)
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	if (m_cMap.exists(pszLabel_)) {
		m_cMap.erase(pszLabel_);
		return true;
	} else { 
#ifdef DEBUG
		if (isSydTestDebugMessage())
		{
			SydTestDebugMessage << "No such timespan label "
				<< pszLabel_ << "." << ModEndl;
		}
#endif		
		return false;
	}
}

//
//	FUNCTION public
//  StopWatch::startAll -- 全ての時計を開始させる
//
//	NOTES
//    全ての時計を開始させる
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    bool
//      true: 成功、false: 失敗
//
//	EXCEPTIONS
//    なし
//
bool
StopWatch::startAll()
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	bool bResult;
	Map<ModUnicodeString, _Watch>::iterator i = m_cMap.begin();

	while(i != m_cMap.end()) {
		bResult = start((*i).first);
		if (bResult == false) return false;
		i++;
	}
	return true;
}

//
//	FUNCTION public
//  StopWatch::stopAll -- 全ての時計を停止させる
//
//	NOTES
//    全ての時計を停止させる
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    bool
//      true: 成功、false: 失敗
//
//	EXCEPTIONS
//    なし
//
bool
StopWatch::stopAll()
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	bool bResult;
	Map<ModUnicodeString, _Watch>::iterator i = m_cMap.begin();

	while(i != m_cMap.end()) {
		bResult = stop((*i).first);
		if (bResult == false) return false;
		i++;
	}
	return true;
}

#ifdef DEBUG
//
//	FUNCTION public
//  StopWatch::showAllWatches -- 全ての時計を表示する
//
//	NOTES
//    全ての時計のラップを表示する。
//    debug用の関数。
//
//	ARGUMENTS
//    なし
//
//	RETURN
//    なし
//
//	EXCEPTIONS
//    なし
//
void
StopWatch::showAllWatches()
{
	ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
	cAuto.lock();

	if (isSydTestDebugMessage())
	{
		SydTestDebugMessage << "-----" << ModEndl;
		Map<ModUnicodeString, _Watch>::iterator i = m_cMap.begin();
		while(i != m_cMap.end()) {
			SydTestDebugMessage << (*i).first << " : "
				<< showTotalTime((*i).first).getTotalSeconds() << ModEndl;
			i++;
		}
		SydTestDebugMessage << "-----" << ModEndl;
	}
}
#endif

//
//	Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
