// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoBranch.h --
//		オートトランザクションブランチ記述子関連のクラス定義、関数宣言
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_AUTOBRANCH_H
#define	__SYDNEY_TRANS_AUTOBRANCH_H

#include "Trans/Module.h"
#include "Trans/Branch.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

//	CLASS
//	Trans::AutoBranch -- オートトランザクションブランチ記述子を表すクラス
//
//	NOTES

class AutoBranch
	: public	ModAutoPointer<Branch>
{
public:
	// コンストラクター
	AutoBranch(Branch* branch);
	// デストラクター
	~AutoBranch();

	// トランザクションブランチ記述子を破棄する
	virtual void			free();
};

//	FUNCTION public
//	Trans::AutoBranch::AutoBranch --
//		オートトランザクションブランチ記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Branch*	branch
//			オートトランザクションブランチ記述子が保持する
//			トランザクションブランチ記述子を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoBranch::AutoBranch(Branch* branch)
	: ModAutoPointer<Branch>(branch)
{}

//	FUNCTION public
//	Trans::AutoBranch::~AutoBranch --
//		オートトランザクションブランチ記述子を表すクラスのデストラクター
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
AutoBranch::~AutoBranch()
{
	free();
}

//	FUNCTION public
//	Trans::AutoBranch::free -- 保持するトランザクションブランチ記述子を破棄する
//
//	NOTES
//		オートトランザクションブランチ記述子の破棄時などに呼び出される
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
AutoBranch::free()
{
	if (isOwner())
		if (Branch* branch = release())
			Branch::detach(branch);
}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_AUTOBRANCH_H

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
