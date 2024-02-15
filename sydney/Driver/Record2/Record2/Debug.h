// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Debug.h -- Macro definition for debugging
// 
// Copyright (c) 2001, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_DEBUG_H
#define __SYDNEY_RECORD2_DEBUG_H

#include "Record2/Module.h"
#include "Common/Message.h"

#define SydRecordInfoMessage _SYDNEY_MESSAGE( \
										"Record_MessageOutputInfo", \
										Common::MessageStreamBuffer::LEVEL_INFO)
#define SydRecordTraceMessage _SYDNEY_MESSAGE( \
										"Record_MessageOutputInfo", \
										Common::MessageStreamBuffer::LEVEL_INFO)
#define SydRecordDebugMessage _SYDNEY_MESSAGE( \
										"Record_MessageOutputDebug", \
										Common::MessageStreamBuffer::LEVEL_DEBUG)
#define SydRecordSizeMessage _SYDNEY_MESSAGE( \
										"Record_MessageOutputSize", \
										Common::MessageStreamBuffer::LEVEL_DEBUG)
_SYDNEY_BEGIN

namespace Record2
{

#ifdef DEBUG
	//
	//	CLASS
	//	AutoMessage -- Scope の入退出時点でメッセージを出力する
	//
	//	NOTES
	//	Scope の入退出時点でメッセージを出力するクラス
	//	ローカルスコープでインスタンスを定義して使用する。
	//
	class AutoMessage {
	public:
		AutoMessage( const char* s ,void* p = 0 )
			: m_szName(s)
			, m_pObj(p)
			{ SydRecordTraceMessage << m_szName << "(" << long(m_pObj) << ") begin." << ModEndl; }
		~AutoMessage() throw()
			{ try { SydRecordTraceMessage << m_szName << "(" << long(m_pObj) << ") is done." << ModEndl; } catch(...) {} }
	private:
		const char* const m_szName;
		const void* const m_pObj;

	private://auto のみで作成させる為の処置
		void* operator new(size_t);
		void operator delete(void*);
	};
#define TRACEMSG(s) AutoMessage _AutoMessage_(s,(void*)this)
#define TRACEMSG_(s) AutoMessage _AutoMessage_(s)
#else
#define TRACEMSG(s)
#define TRACEMSG_(s)
#endif

#ifdef DEBUG
#define MSGLIN(s) s
#else
#define MSGLIN(s)
#endif

}

_SYDNEY_END

#endif // __SYDNEY_RECORD2_DEBUG_H

//
// Copyright (c) 2001, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
