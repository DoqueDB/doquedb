// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Log.h -- 論理ログ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2003, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALLOG_LOG_H
#define __SYDNEY_LOGICALLOG_LOG_H

#include "LogicalLog/LSN.h"
#include "LogicalLog/Module.h"

#include "Common/Object.h"
#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALLOG_BEGIN

//	CLASS
//	LogicalLog::Log -- 論理ログを表すクラス
//
//	NOTES

class Log
	: public	Common::Object
{
	friend class SubFile;
public:
	// コンストラクタ
	SYD_LOGICALLOG_FUNCTION
	Log(Os::Memory::Size size, const void* data = 0);
	// デストラクタ
	SYD_LOGICALLOG_FUNCTION
	virtual ~Log();

	// データ領域を指すポインタを得る
	operator void*() const;
	operator char*() const;

	// サイズを得る
	Os::Memory::Size		getSize() const;

#ifdef OBSOLETE
	// ログシーケンス番号を得る
	LSN
	getLSN() const;
	// ログシーケンス番号へのキャスト演算子
	operator LSN() const;
#endif

private:
	// コンストラクタ
	Log();
	// LSNを設定する
	void	setLSN(LSN lsn_);

	// 論理ログの一部分をある領域に書き込む
	Os::Memory::Size
	serialize(LSN prevLSN, void* p, Os::Memory::Size size);
	Os::Memory::Size
	serialize(void* p, Os::Memory::Size size, Os::Memory::Size offset) const;

	// メンバ変数

	// LSN
	//   readまたはappendされた後に有効
	LSN		m_lsn;
	// m_pDataの指すデータの長さ[bytes]
	Os::Memory::Size		m_uSize;
	// LogHeaderおよびログデータを保持する領域
	void*	m_pSerialData;
	// クライアントがログを書き込むアドレス
	// (ただし領域がallocされていなければNULLポインタ)
	void*	m_pData;
};

//	FUNCTION private
//	LogicalLog::Log::Log -- 論理ログを表すクラスのデフォルトコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Log::Log()
	: m_lsn(NoLSN),
	  m_uSize(0),
	  m_pSerialData(0),
	  m_pData(0)
{}

//
//	FUNCTION public
//	LogicalLog::Log::operator void* -- データ領域を返す
//
//	NOTES
//	データ領域の先頭アドレスを返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	データ領域の先頭アドレス
//	確保されていなければNULLポインタ
//
//	EXCEPTIONS
//	なし
//
inline
Log::operator void*() const
{
	// m_pDataの先頭HeaderSize分にはディスク上のヘッダが入っている
	return m_pData;
}

//
//	FUNCTION public
//	LogicalLog::Log::operator char* -- データ領域を返す
//
//	NOTES
//	データ領域の先頭アドレスを返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	データ領域の先頭アドレス
//	確保されていなければNULLポインタ
//
//	EXCEPTIONS
//	なし
//
inline
Log::operator char*() const
{
	return static_cast<char*>(operator void*());
}

//
//	FUNCTION public
//	LogicalLog::Log::getSize -- 大きさを返す
//
//	NOTES
//	ログの大きさを返す
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	保持しているログの大きさを返す
//	確保されていなければ0
//
//	EXCEPTIONS
//	なし
//

inline
Os::Memory::Size
Log::getSize() const
{
	return m_uSize;
}

#ifdef OBSOLETE
//	FUNCTION public
//	LogicalLog::Log::getLSN -- ログシーケンス番号を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		保持しているログシーケンス番号
//
//	EXCEPTIONS
//		なし

inline
LSN
Log::getLSN() const
{
	return m_lsn;
}

//	FUNCTION public
//	LogicalLog::Log::operator LSN -- ログシーケンス番号へのキャスト演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		保持しているログシーケンス番号
//
//	EXCEPTIONS
//		なし

inline
Log::operator LSN() const
{
	return getLSN();
}
#endif

//
//	FUNCTION private
//	LogicalLog::Log::setLSN -- LSNを設定する
//
//	NOTES
//	LSNを設定する
//
//	ARGUMENTS
//	  LSN	lsn_	設定すべきlsn
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
inline
void
Log::setLSN(LSN lsn_)
{
	m_lsn = lsn_;
}

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALLOG_LOG_H

//
//	Copyright (c) 2000, 2001, 2003, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
