// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLatch.h -- オートラッチ関連のクラス定義、関数宣言
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

#ifndef __SYDNEY_TRANS_AUTOLATCH_H
#define	__SYDNEY_TRANS_AUTOLATCH_H

#include "Trans/Module.h"
#include "Trans/Transaction.h"

#include "Lock/AutoLatch.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

//	CLASS
//	Trans::AutoLatch -- オートラッチを表すクラス
//
//	NOTES

class AutoLatch
	: public	Lock::AutoLatch
{
public:
	// コンストラクター
	AutoLatch(Transaction& trans, const Lock::Name& name);
};

//	CLASS
//	Trans::AutoUnlatch -- オートアンラッチを表すクラス
//
//	NOTES

class AutoUnlatch
	: public	Lock::AutoUnlatch
{
public:
	// コンストラクター
	AutoUnlatch(Transaction& trans, const Lock::Name& name);
};

//	FUNCTION public
//	Trans::AutoLatch::AutoLatch --
//		オートラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ラッチするトランザクションのトランザクション記述子
//		Lock::Name&			name
//			ラッチされるロック項目のロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoLatch::AutoLatch(Transaction& trans, const Lock::Name& name)
	: Lock::AutoLatch(trans._client, name)
{}

//	FUNCTION public
//	Trans::AutoUnlatch::AutoUnlatch --
//		オートアンラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			アンラッチするトランザクションのトランザクション記述子
//		Lock::Name&			name
//			アンラッチされるロック項目のロック名
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
AutoUnlatch::AutoUnlatch(Transaction& trans, const Lock::Name& name)
	: Lock::AutoUnlatch(trans._client, name)
{}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_AUTOLATCH_H

//
// Copyright (c) 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
