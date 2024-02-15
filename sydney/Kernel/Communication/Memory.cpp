// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Memory.cpp -- メモリーIOクラス
// 
// Copyright (c) 1999, 2000, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Communication";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Communication/Memory.h"
#include "Exception/ConnectionRanOut.h"
#include "Os/AutoCriticalSection.h"

#include "ModOsDriver.h"

_TRMEISTER_USING

using namespace Communication;

//
//	FUNCTION public
//	Communication::Memory::Memory -- コンストラクタ(1)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iSize_
//		メモリ領域のサイズ(default 16*1024)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Memory::Memory(int iSize_)
: m_iTotalSize(iSize_),
  m_iCurrentReadPosition(0), m_iCurrentWritePosition(0),
  m_bReadWait(false), m_bWriteWait(false), m_bOwner(true), m_iRelease(0)
{
	m_pHeadAddress = new char[m_iTotalSize];
}

//
//	FUNCTION public
//	Communication::Memory::Memory -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	int iSize_
//		メモリ領域のサイズ(default 16*1024)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Memory::Memory(void* pAddress_, int iSize_)
: m_pHeadAddress(pAddress_), m_iTotalSize(iSize_),
  m_iCurrentReadPosition(0), m_iCurrentWritePosition(0),
  m_bReadWait(false), m_bWriteWait(false), m_bOwner(false)
{
}

//
//	FUNCTION public
//	Communication::Memory::~Memory -- デストラクタ
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
Memory::~Memory()
{
	if (m_bOwner) delete[] m_pHeadAddress;
}

//
//	FUNCTION public
//	Communication::Memory::readSerial -- メモリーからの読み込み
//
//	NOTES
//	メモリーからの読み込み
//
//	ARGUMENTS
//	voud pBuffer_
//		読み込んだ内容を書き出すバッファ
//	ModSize msByte_
//		読み込むサイズ
//	ModSerialIO::DataType eType_
//		データ型(ここでは使わない)
//
//	RETURN
//	読み込んだサイズ
//
//	EXCEPTIONS
//	Exception::ConnectionRanOut
//		通信相手にコネクションを切断された
//
int
Memory::readSerial(void* pBuffer_, ModSize msByte_,
								  ModSerialIO::DataType eType_)
{
	int iByte = static_cast<int>(msByte_);
	Os::AutoTryCriticalSection cAuto(m_cCriticalSection, false);
	char* d = static_cast<char*>(pBuffer_);
	cAuto.lock();
	while (iByte)
	{
		int iSize;	//今回読み込めるサイズ
		if (m_iCurrentReadPosition + iByte <= m_iCurrentWritePosition)
			iSize = iByte;
		else
			iSize = m_iCurrentWritePosition - m_iCurrentReadPosition;
		if (iSize)
		{
			int iPosition = m_iCurrentReadPosition % m_iTotalSize;
			int iRest = iPosition + iSize - m_iTotalSize;
			if (iRest < 0) iRest = 0;
			const char* s = static_cast<const char*>(m_pHeadAddress);
			ModOsDriver::Memory::copy(d, s + iPosition, iSize - iRest);
			d += (iSize - iRest);
			iByte -= (iSize - iRest);
			m_iCurrentReadPosition += (iSize - iRest);
			if (iRest)
			{
				//残りを先頭から読む
				ModOsDriver::Memory::copy(d, s, iRest);
				d += iRest;
				iByte -= iRest;
				m_iCurrentReadPosition += iRest;
			}
			if (m_iCurrentReadPosition == m_iCurrentWritePosition)
			{
				//同じ位置なので0にする
				m_iCurrentReadPosition = 0;
				m_iCurrentWritePosition = 0;
			}
			int iTimes = m_iCurrentReadPosition / m_iTotalSize;
			if (iTimes >= 2)
			{
				//2周目なのでもどす
				m_iCurrentReadPosition -= m_iTotalSize;
				m_iCurrentWritePosition -= m_iTotalSize;
			}
		}
		else
		{
			if (m_bWriteWait == true)
			{
				//待っているのでイベントを発生させる
				m_cWriteConditionVariable.signal();
				m_bWriteWait = false;
			}
			if (m_iRelease)
			{
				//終了なので、例外を投げる
				_TRMEISTER_THROW0(Exception::ConnectionRanOut);
			}
			//1バイトも読めないので待つ
			m_bReadWait = true;
			cAuto.unlock();
			m_cReadConditionVariable.wait();
			//何か書き込まれたので読み込む
			cAuto.lock();
		}
	}

	return (static_cast<int>(msByte_) - iByte);
}

//
//	FUNCTION public
//	Communication::Memory::writeSerial -- メモリーへの書き出し
//
//	NOTES
//	メモリーへの書き出し
//
//	ARGUMENTS
//	const void* pBuffer_
//		メモリーへ書き出す内容が格納されているバッファ
//	ModSize msByte_
//		書き出すサイズ
//	ModSerialIO::DataType eType_
//		データ型(ここでは使わない)
//
//	RETURN
//	書き込んだサイズ
//
//	EXCEPTIONS
//	Exception::ConnectionRanOut
//		通信相手にコネクションを切断された
//
int
Memory::writeSerial(const void* pBuffer_, ModSize msByte_,
								   ModSerialIO::DataType eType_)
{
	Os::AutoTryCriticalSection cAuto(m_cCriticalSection, false);
	const char* s = static_cast<const char*>(pBuffer_);
	cAuto.lock();
	if (m_iRelease)
	{
		//終了なので、例外を投げる
		_TRMEISTER_THROW0(Exception::ConnectionRanOut);
	}
	int iByte = static_cast<int>(msByte_);
	while (iByte)
	{
		int iSize;	//今回書き込めるサイズ
		if (m_iTotalSize - (m_iCurrentWritePosition - m_iCurrentReadPosition)
			>= iByte)
			iSize = iByte;
		else
			iSize = m_iTotalSize
				- (m_iCurrentWritePosition - m_iCurrentReadPosition);
		if (iSize)
		{
			int iPosition = m_iCurrentWritePosition % m_iTotalSize;
			int iRest = iPosition + iSize - m_iTotalSize;
			if (iRest < 0) iRest = 0;
			char* d = static_cast<char*>(m_pHeadAddress);
			ModOsDriver::Memory::copy(d + iPosition, s, iSize - iRest);
			s += (iSize - iRest);
			iByte -= (iSize - iRest);
			m_iCurrentWritePosition += (iSize - iRest);
			if (iRest)
			{
				//残りを先頭から書き込む
				ModOsDriver::Memory::copy(d, s, iRest);
				s += iRest;
				iByte -= iRest;
				m_iCurrentWritePosition += iRest;
			}
		}
		else
		{
			if (m_bReadWait == true)
			{
				//待っているのでイベントを発生させる
				m_cReadConditionVariable.signal();
				m_bReadWait = false;
			}
			//1バイトも書けないので待つ
			m_bWriteWait = true;
			cAuto.unlock();
			m_cWriteConditionVariable.wait();
			//何か読み込まれたので書き込む
			cAuto.lock();
			if (m_iRelease)
			{
				//終了なので、例外を投げる
				_TRMEISTER_THROW0(Exception::ConnectionRanOut);
			}
		}
	}

	return (static_cast<int>(msByte_) - iByte);
}

//
//	FUNCTION public
//	Communcation::Memory::writeFlushSerial -- 出力をフラッシュする
//
//	NOTES
//	出力をフラッシュする。
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
Memory::writeFlushSerial()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	if (m_bReadWait == true)
	{
		//待っているのでイベントを発生させる
		m_cReadConditionVariable.signal();
		m_bReadWait = false;
	}
}

//
//	FUNCTION public
//	Communication::Memory::wait -- 書き込みがあるまで待つ
//
//	NOTES
//	書き込みがあるまで待つ。
//
//	ARGUMENTS
//	int iMilliseconds_
//		待つ時間(ミリ秒)
//
//	RETURN
//	書き込みがあったらtrue、それ以外の場合はfalseを返す
//
//	EXCEPTIONS
//	なし
//
bool
Memory::wait(int iMilliseconds_)
{
	bool result = false;
	Os::AutoTryCriticalSection cAuto(m_cCriticalSection, false);
	//終了
	if (m_iRelease) return true;
	if (m_iCurrentReadPosition == m_iCurrentWritePosition)
	{
		//書き込み位置と読み込み位置が同じなので、何も書かれていない
		m_bReadWait = true;
		cAuto.unlock();
		if ( iMilliseconds_ == 0 ) {
			result = false;
		} else {
			if (iMilliseconds_ < 0)
			{
				//永遠に待つ
				m_cReadConditionVariable.wait();
			}
			else
			{
				m_cReadConditionVariable.wait(ModTimeSpan(iMilliseconds_ / 1000,
														  iMilliseconds_ % 1000));
			}
			cAuto.lock();
			//同じかどうかチェックする
			if (m_iCurrentReadPosition != m_iCurrentWritePosition || m_iRelease)
			{
				result = true;
			}
		}
	}
	else
	{
		result = true;
	}
	return result;
}

//
//	FUNCTION public
//	Communication::Memory::release -- 使用しなくなったことを宣言する
//
//	NOTES
//	使用しなくなったことを宣言する。Connection が close されたら、呼ばれる。
//	戻り値が2の場合は、コネクションの両方でクローズされた時であるので、
//	メモリーを解放することができる。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		リリースされた回数。
//
//	EXCEPTIONS
//	なし
//
int
Memory::release()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	++m_iRelease;
	if (m_bReadWait == true)
	{
		//反対側のスレッドが待っているので、イベントを発生する
		m_cReadConditionVariable.signal();
		m_bReadWait = false;
	}
	if (m_bWriteWait == true)
	{
		//反対側のスレッドが待っているので、イベントを発生する
		m_cWriteConditionVariable.signal();
		m_bWriteWait = false;
	}
	return m_iRelease;
}

//
//	Copyright (c) 1999, 2000, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
