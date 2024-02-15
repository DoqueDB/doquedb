// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Deterrent.h -- バッファページのフラッシュの抑止関連の関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_DETERRENT_H
#define	__SYDNEY_BUFFER_DETERRENT_H

#include "Buffer/Module.h"

_SYDNEY_BEGIN

namespace Os
{
	class RWLock;
}

_SYDNEY_BUFFER_BEGIN

//	NAMESPACE
//	Buffer::Deterrent -- バッファページのフラッシュの抑止に関する名前空間
//
//	NOTES

namespace Deterrent
{
#ifdef OBSOLETE
	// バッファページのフラッシュの抑止を開始する
	SYD_BUFFER_FUNCTION
	void					start();
	// バッファページのフラッシュの抑止をやめる
	SYD_BUFFER_FUNCTION
	void					end();
#endif
	// 排他制御用の読み取り書き込みロックを得る
	SYD_BUFFER_FUNCTION
	Os::RWLock&				getRWLock();
#ifdef OBSOLETE
	// バッファページのフラッシュが抑止されているか
	SYD_BUFFER_FUNCTION
	bool					isStarted();
#endif
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_DETERRENT_H

//
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
