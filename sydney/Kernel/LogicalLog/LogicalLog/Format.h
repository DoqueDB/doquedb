// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Format.h -- ディスクに格納するデータの情報
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2009, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALLOG_FORMAT_H
#define __SYDNEY_LOGICALLOG_FORMAT_H

#include "LogicalLog/LSN.h"
#include "LogicalLog/Module.h"
#include "LogicalLog/VersionNumber.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALLOG_BEGIN

//	NAMESPACE
//	LogicalLog::Format -- 論理ログファイルのフォーマットに関する名前空間
//
//	NOTES

namespace Format 
{
	//	STRUCT
	//  LogicalLog::Format::FileHeader -- 論理ログファイルのヘッダ
	//
	//	NOTES
	//	サブファイルのヘッダーと構造体は共通

	struct FileHeader
	{
		// バージョン番号
		VersionNumber::Value	_versionNumber;
#ifdef SYD_C_GCC
		// Microsoft Visual C++ のデフォルトのアライメントに合わせる
		char					_dummy[4];
#endif
		// 末尾の論理ログのログシーケンス番号
		LSN						_last;
		// 次に割り当てるべきログシーケンス番号
		LSN						_next;
		// 同期処理が完了したかどうか
		bool					_fileSyncDone;
#ifdef SYD_C_GCC
		// Microsoft Visual C++ のデフォルトのアライメントに合わせる
		char					_dummy2[3];
#endif
		// 末尾のサブファイル番号
		int						_subfile;
		// このファイルの先頭のログシーケンス番号
		LSN						_top;
		// マスターのログシーケンス番号
		LSN						_masterLSN;
	};

	//	STRUCT
	//  LogicalLog::Format::LogHeader -- 論理ログのヘッダ
	//
	//	NOTES

	struct LogHeader
	{
		// 直前の論理ログのログシーケンス番号
		LSN						_prevLSN;
		// 論理ログデータのサイズ(B 単位)
		Os::Memory::Size		_size;
#ifdef SYD_C_GCC
		// Microsoft Visual C++ のデフォルトのアライメントに合わせる
		char					_dummy[4];
#endif
	};
}

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALLOG_FORMAT_H

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2009, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
