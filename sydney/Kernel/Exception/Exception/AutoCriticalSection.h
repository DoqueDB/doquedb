// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoCriticalSection.h --
//		Os::AutoCriticalSectionと同等だがOsモジュールに依存できないためここで定義する
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_EXCEPTION_AUTOCRITICALSECTION_H
#define	__TRMEISTER_EXCEPTION_AUTOCRITICALSECTION_H

#include "Exception/Module.h"
#include "ModCriticalSection.h"

_TRMEISTER_BEGIN
_TRMEISTER_EXCEPTION_BEGIN

//	TYPEDEF
//	Exception::AutoCriticalSection --
//		オブジェクト生成時に自動的にクリティカルセクションをロックし、
//		破棄時に自動的にロックをはずすクラス
//
//	NOTES
//		ModAutoMutex でも同様のことはできるが、
//		Linux版では gcc のバグにより正しく動作しないことがあるため
// 		ここで別途定義する。

class AutoCriticalSection
{
public:
	AutoCriticalSection(ModCriticalSection& cObject_)
		: m_cObject(cObject_)
	{
		m_cObject.lock();
	}
	~AutoCriticalSection()
	{
		m_cObject.unlock();
	}
private:
	ModCriticalSection&	m_cObject;
};

_TRMEISTER_EXCEPTION_END
_TRMEISTER_END

#endif	// __TRMEISTER_EXCEPTION_AUTOCRITICALSECTION_H

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

