// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Japanese.cpp -- 日本語のメッセージテーブル
// 
// Copyright (c) 1999, 2001, 2002, 2003, 2005, 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
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
#include "Message/Message.h"
#include "Message/AutoCriticalSection.h"
#include "Common/UnicodeString.h"

// MessageFormatJapanese.h から参照される
#include "Exception/AllNumberFiles.h"

#include "PhysicalFile/MessageAll_Number.h"
#include "Schema/MessageAll_Number.h"
#include "Version/MessageAll_Number.h"

#ifndef SYD_CPU_SPARC
#include "Btree/MessageAll_Number.h"
#endif
#include "FullText/MessageAll_Number.h"
#include "Inverted/MessageAll_Number.h"
#include "Record/MessageAll_Number.h"
#ifndef SYD_CPU_SPARC
#include "Vector/MessageAll_Number.h"
#endif
#include "Lob/MessageAll_Number.h"
#include "Btree2/MessageAll_Number.h"
#include "Vector2/MessageAll_Number.h"
#include "Bitmap/MessageAll_Number.h"
#include "Record2/MessageAll_Number.h"
#include "Array/MessageAll_Number.h"
#include "FullText2/MessageAll_Number.h"

#include "ModMap.h"
#include "ModCriticalSection.h"

_SYDNEY_USING

namespace {

//
//	TYPEDEF
//	MessageFormatItemMap --
//	メッセージフォーマット文字列格納マップ
//
//	NOTES
//	メッセージフォーマットの UNICODE 文字列を格納するマップクラス
//
typedef ModMap<unsigned int, ModUnicodeString, ModLess<unsigned int> >
	        MessageFormatItemMap;

//
//	VARIABLE
//	MessageFormatTable -- メッセージフォーマットテーブル
//
//	NOTES
//	メッセージフォーマットのテーブル
//
Message::MessageFormatItem MessageFormatTable[] = {
#include "Exception/MessageFormatJapanese.h"
#include "PhysicalFile/MessageFormat_Japanese.h"
#include "Schema/MessageFormat_Japanese.h"
#include "Version/MessageFormat_Japanese.h"
#ifndef SYD_CPU_SPARC
#include "Btree/MessageFormat_Japanese.h"
#endif
#include "FullText/MessageFormat_Japanese.h"
#include "Inverted/MessageFormat_Japanese.h"
#include "Record/MessageFormat_Japanese.h"
#ifndef SYD_CPU_SPARC
#include "Vector/MessageFormat_Japanese.h"
#endif
#include "Lob/MessageFormat_Japanese.h"
#include "Btree2/MessageFormat_Japanese.h"
#include "Vector2/MessageFormat_Japanese.h"
#include "Bitmap/MessageFormat_Japanese.h"
#include "Record2/MessageFormat_Japanese.h"
#include "Array/MessageFormat_Japanese.h"
#include "FullText2/MessageFormat_Japanese.h"
	{0,	0}
};

}

//
//	FUNCTION global
//	DBGetMessageFormat -- メッセージフォーマットを得る
//
//	NOTES
//	メッセージ番号からメッセージフォーマットを得る。
//
//	ARGUMENTS
//	unsigned int uiMessageNumber_
//		メッセージ番号
//
//	RETURN
//	const char*
//		メッセージフォーマット 見つからない場合は0を返す
//
//	EXCEPTIONS
//	なし
//
const ModUnicodeChar*
DBGetMessageFormat(unsigned int uiMessageNumber_)
{
	const ModUnicodeChar * p = 0;

	static MessageFormatItemMap cMsgFrmtItmMap;
	static ModCriticalSection cCriticalSection;

	Message::AutoCriticalSection cAuto(cCriticalSection);

	if (cMsgFrmtItmMap.isEmpty() == ModTrue)
	{
		// マップへの登録
		Message::MessageFormatItem* i = MessageFormatTable;
		while (i->m_pszMessageFormat)
		{
			cMsgFrmtItmMap.insert(i->m_uiMessageNumber,
								  i->m_pszMessageFormat);
			++i;
		}
	}

	// マップ内の検索
	MessageFormatItemMap::Iterator iter = cMsgFrmtItmMap.find(uiMessageNumber_);
	if (iter != cMsgFrmtItmMap.end())
	{
		p = (*iter).second;
	}
	return p;
}

//
//	Copyright (c) 1999, 2001, 2002, 2003, 2005, 2007, 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
