// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Deterrent.cpp -- バッファページのフラッシュの抑止関連の関数定義
// 
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Buffer";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyInclude.h"

#include "Buffer/Deterrent.h"

#include "Os/AutoRWLock.h"

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{

namespace _Deterrent
{
	// 以下のメンバの排他制御用の読み取り書き込みロック
	Os::RWLock				_rwlock;
#ifdef OBSOLETE
	// バッファページのフラッシュの抑止の開始の入れ子数
	unsigned int			_count = 0;
#endif
}

}

#ifdef OBSOLETE
//	FUNCTION
//	Buffer::Deterrent::start -- バッファページのフラッシュの抑止を開始する
//
//	NOTES
//		バッファページのフラッシュの抑止が開始されると、
//		フラッシュが抑止可能なバッファページがダーティになっても、
//		強制的にフラッシュしない限り、フラッシュされなくなる
//
//		排他制御の実装が面倒なので、現状ではすべての
//		バッファファイルについて影響を及ぼしてしまう
//
//		システムの起動直後は、バッファページのフラッシュは抑止されていない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Deterrent::start()
{
	Os::AutoRWLock	rwlock(getRWLock(), Os::RWLock::Mode::Write);

	++_Deterrent::_count;
}

//	FUNCTION
//	Buffer::Deterrent::end -- バッファページのフラッシュの抑止を終了する
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

void
Deterrent::end()
{
	Os::AutoRWLock	rwlock(getRWLock(), Os::RWLock::Mode::Write);

	if (_Deterrent::_count)
		--_Deterrent::_count;
}
#endif

//	FUNCTION
//	Buffer::Deterrent::getRWLock --
//		バッファページのフラッシュの抑止の排他制御をするための
//		読み取り書き込みロックを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		読み取り書き込みロックへのリファレンス
//
//	EXCEPTIONS
//		なし

Os::RWLock&
Deterrent::getRWLock()
{
	return _Deterrent::_rwlock;
}

#ifdef OBSOLETE
//	FUNCTION
//	Buffer::Deterrent::isStarted --
//		バッファページのフラッシュが抑止されているか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			抑止されている
//		false
//			抑止されていない
//
//	EXCEPTIONS
//		なし

bool
Deterrent::isStarted()
{
	return _Deterrent::_count;
}
#endif

//
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
