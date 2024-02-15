// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Log.cpp -- 論理ログ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalLog";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "LogicalLog/Format.h"
#include "LogicalLog/Log.h"

#include "Common/Assert.h"

#include "ModDefaultManager.h"

_SYDNEY_USING
_SYDNEY_LOGICALLOG_USING

//	FUNCTION public
//	LogicalLog::Log::Log -- 論理ログを表すクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Os::Memory::Size	size
//			論理ログデータを格納するための領域のサイズ(B 単位)
//		void*				data
//			指定されたとき
//				論理ログデータを格納する領域の先頭アドレス
//			指定されないとき
//				0 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Log::Log(Os::Memory::Size size, const void* data)
	: m_lsn(NoLSN),
	  m_uSize(size)
{
	m_pSerialData =
		ModDefaultManager::allocate(sizeof(Format::LogHeader) + getSize());
	Os::Memory::reset(m_pSerialData, sizeof(Format::LogHeader));

	m_pData = static_cast<char*>(m_pSerialData) + sizeof(Format::LogHeader);

	if (data)
		Os::Memory::copy(m_pData, data, size);
}

//	FUNCTION public
//	LogicalLog::Log::~Log -- 論理ログを表すクラスのデストラクタ
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

Log::~Log()
{
	if (m_pSerialData) {
		ModDefaultManager::free(
			m_pSerialData, sizeof(Format::LogHeader) + getSize());
		m_pSerialData = 0;
	}
}

//	FUNCTION private
//	LogicalLog::Log::serialize -- 論理ログの一部分をある領域に書き込む
//
//	NOTES
//
//	ARGUMENTS
//		LSN					prevLSN
//			直前の論理ログのログシーケンス番号
//		void*				p
//			論理ログの一部を書き込む領域の先頭アドレス
//		Os::Memory::Size	size
//			論理ログの一部を書き込む領域のサイズ(B 単位)
//
//	RETURN
//		書き込んだサイズ(B 単位)
//
//	EXCEPTIONS

Os::Memory::Size
Log::serialize(LSN prevLSN, void* p, Os::Memory::Size size)
{
	if (!p)
		return 0;

	Format::LogHeader& header =
		*static_cast<Format::LogHeader*>(m_pSerialData);
	header._prevLSN = prevLSN;
	header._size = getSize();

	if (size) {
		const Os::Memory::Size rest = sizeof(Format::LogHeader) + getSize();
		if (rest < size)
			size = rest;

		Os::Memory::copy(p, m_pSerialData, size);
	}

	return size;
}

//	FUNCTION private
//	LogicalLog::Log::serialize -- 論理ログの一部分をある領域に書き込む
//
//	NOTES
//
//	ARGUMENTS
//		void*				p
//			論理ログの一部を書き込む領域の先頭アドレス
//		Os::Memory::Size	size
//			論理ログの一部を書き込む領域のサイズ(B 単位)
//		Os::Memory::Size	offset
//			論理ログの先頭からなんバイト目から与えられた領域に書き込む
//
//	RETURN
//		書き込んだサイズ(B 単位)
//
//	EXCEPTIONS

Os::Memory::Size
Log::serialize(void* p, Os::Memory::Size size, Os::Memory::Size offset) const
{
	if (!(p && size))
		return 0;

	; _SYDNEY_ASSERT(offset >= 0 &&
					 sizeof(Format::LogHeader) + getSize() >= offset);
	Os::Memory::Size rest = sizeof(Format::LogHeader) + getSize() - offset;
	if (rest < size)
		size = rest;

	Os::Memory::copy(
		p, static_cast<const char*>(m_pSerialData) + offset, size);

	return size;
}

//
//	Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

