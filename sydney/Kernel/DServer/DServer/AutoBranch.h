// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoBranch.h --
//		オートトランザクションブランチ記述子関連のクラス定義、関数宣言
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DSERVER_AUTOBRANCH_H
#define	__SYDNEY_DSERVER_AUTOBRANCH_H

#include "DServer/Module.h"
#include "DServer/Branch.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_DSERVER_BEGIN

//
//	CLASS
//	DServer::AutoBranch -- オートトランザクションブランチ記述子を表すクラス
//
//	NOTES
//
class AutoBranch
	: public ModAutoPointer<Branch>
{
public:
	// コンストラクター
	AutoBranch(Branch* branch)
		: ModAutoPointer<Branch>(branch) {}
	// デストラクター
	~AutoBranch() { free(); }

	// トランザクションブランチ記述子を破棄する
	virtual void free()
	{
		if (isOwner())
			if (Branch* branch = release())
				Branch::detach(branch);
	}
};

_SYDNEY_DSERVER_END
_SYDNEY_END

#endif	// __SYDNEY_DSERVER_AUTOBRANCH_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
