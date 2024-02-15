// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoPool.h -- オートバッファプール関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_AUTOPOOL_H
#define	__SYDNEY_BUFFER_AUTOPOOL_H

#include "Buffer/Module.h"
#include "Buffer/Pool.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	CLASS
//	Buffer::AutoPool -- オートバッファプール記述子を表すクラス
//
//	NOTES

class AutoPool
	: public	ModAutoPointer<Pool>
{
public:
	// コンストラクター
	AutoPool(Pool* pool);
	// デストラクター
	~AutoPool();

	// バッファプール記述子を破棄する
	virtual void			free();
};

//	FUNCTION public
//	Buffer::AutoPool::AutoPool --
//		オートバッファプール記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Buffer::Pool*		pool
//			オートバッファプール記述子が保持する
//			バッファプール記述子を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoPool::AutoPool(Pool* pool)
	: ModAutoPointer<Pool>(pool)
{}

//	FUNCTION public
//	Buffer::AutoPool::~AutoPool --
//		オートバッファプール記述子を表すクラスのデストラクター
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
AutoPool::~AutoPool()
{
	free();
}

//	FUNCTION public
//	Buffer::AutoPool::free -- 保持するバッファプール記述子を破棄する
//
//	NOTES
//		オートバッファプール記述子の破棄時などに呼び出される
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
void
AutoPool::free()
{
	if (isOwner())
		if (Pool* pool = release())
			Pool::detach(pool);
}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_AUTOPOOL_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
