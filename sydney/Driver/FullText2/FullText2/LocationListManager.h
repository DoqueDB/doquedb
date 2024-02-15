// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationListManager.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_LOCATIONLISTMANAGER_H
#define __SYDNEY_FULLTEXT2_LOCATIONLISTMANAGER_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class LocationListIterator;

//
//	CLASS
//	FullText2::LocationListManager
//		-- 位置情報走査クラスを管理する
//
//	NOTES
//
class LocationListManager
{
	friend class LocationListIterator;
	
public:
	// コンストラクタ
	LocationListManager();
	// デストラクタ
	virtual ~LocationListManager();

protected:
	// 解放された位置情報走査クラスをリストに追加する
	void addFree(LocationListIterator* pList_);
	// 解放された位置情報走査クラスを得る
	LocationListIterator* getFree();

	// 解放された位置情報走査クラスをフリーする
	void clearFree();

private:
	// フリーリスト
	LocationListIterator* m_pFree;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LOCATIONLISTMANAGER_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
