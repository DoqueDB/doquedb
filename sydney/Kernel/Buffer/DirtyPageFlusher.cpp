// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirtyPageFlusher.cpp --
//		ダーティバッファページ書き込みスレッドに関する関数定義
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#include "Buffer/AutoPool.h"
#include "Buffer/Configuration.h"
#include "Buffer/DirtyPageFlusher.h"

#include "Common/Message.h"

#include "Os/AutoCriticalSection.h"
#include "Os/AutoRWLock.h"

#include "Exception/ModLibraryError.h"

#define _MOD_EXCEPTION(e) \
	Exception::ModLibraryError(moduleName, srcFile, __LINE__, e)

_SYDNEY_USING
_SYDNEY_BUFFER_USING

namespace 
{
}

//	FUNCTION private
//	Buffer::DirtyPageFlusher::repeatable --
//		ダーティバッファページの書き込みを行う
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
DirtyPageFlusher::repeatable()
{
	try
	{
		unsigned int i = 0;
		do {
			switch (i) {
			case Pool::Category::Normal:
			case Pool::Category::LogicalLog:

				// 通常データと論理ログデータを格納するバッファプールを処理する
				//
				//【注意】	読取専用のデータは永続化する必要はなく、
				//			一時データはバッファプールがあふれたときしか
				//			フラッシュしない

				// バッファプール記述子を生成する
				//
				//【注意】	AutoPool による間接参照はコスト高なので
				//			可能な限り行わない

				AutoPool destructor(
					Pool::attach(static_cast<Pool::Category::Value>(i)));
				Pool& pool = *destructor;

				// ダーティリストを切り替えるので、書き込みロックしておく

				Os::AutoRWLock rwlock(pool.getRWLock(),
									  Os::RWLock::Mode::Write);
				{
					// バッファプールを保護するためにラッチする

					Os::AutoCriticalSection	latch(pool.getLatch());

					// フラッシュする必要があるか判定する
					//
					//【注意】	オーバーフローする可能性があるので、
					//			誤差が出るが、先に除算してから乗算する

					if (!(pool._dirtyList && pool._dirtyList->_total &&
						  pool._dirtyList->_total >= pool.getLimit() /
						  100 * Configuration::FlushPageCoefficient::get()))

						// フラッシュする必要がないので、
						// 次のバッファプールを処理する

						continue;
				}

				// このバッファプールのダーティなバッファページを
				// 管理するリストに登録されているバッファページの
				// バッファリング内容をできる限り書き込む
				//
				//【注意】	終了要請されているときは、
				//			すべてのダーティなバッファページをフラッシュする

				const bool aborting = getStatus() == Aborting;
				pool.flushDirtyPage(
					Pool::FlushablePageFilter::ForPool(false, aborting),
					aborting);
			}
		} while (++i < Pool::Category::Count);
	}
	catch (Exception::Object& e)
	{
		// Sydneyの例外が発生した場合にはログに記録するだけ
		
		SydErrorMessage << e << ModEndl;
	}
	catch (ModException& e)
	{
		// Modの例外が発生した場合にはログに記録するだけ
		
		SydErrorMessage << _MOD_EXCEPTION(e) << ModEndl;
	}
}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
