// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NodePage.h -- ビットマップが格納されていない途中のページ共通の基底クラス
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

#ifndef __SYDNEY_BITMAP_NODEPAGE_H
#define __SYDNEY_BITMAP_NODEPAGE_H

#include "Bitmap/Module.h"
#include "Bitmap/Page.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

class File;

//
//	CLASS
//	Bitmap::NodePage --
//
//	NOTES
//
class NodePage : public Page
{
public:
	// コンストラクタ
	NodePage(File& cFile_);
	// デストラクタ
	virtual ~NodePage();

	// ダーティにする(あわせて書き込み可能なメモリーにする
	void dirty();

protected:
	// 必要ならFixModeを変更する
	virtual void updateMode();

	// 物理ページが変更されたので読み直す
	virtual void reload() = 0;
};

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif // __SYDNEY_BITMAP_NODEPAGE_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

