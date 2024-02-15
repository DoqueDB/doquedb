// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NodePage.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/NodePage.h"
#include "Bitmap/BitmapFile.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::NodePage::NodePage -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Bitmap::File& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NodePage::NodePage(File& cFile_)
	: Page(cFile_)
{
}

//
//	FUNCTION public
//	Bitmap::NodePage::~NodePage -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NodePage::~NodePage()
{
}

//
//	FUNCTION public
//	Bitmap::NodePage::dirty -- ページをdirtyにする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NodePage::dirty()
{
	// 必要ならFixModeを変更する
	updateMode();
	// ページをdirtyにする
	Page::dirty();
}

//
//	FUNCTION protected
//	Bitmap::NodePage::updateMode -- 更新モードに変更する
//
//	NOTES
//	ReadOnlyでattachしている物理ページをWriteでattachしなおす。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NodePage::updateMode()
{
	if (!isReadOnly()) return;
	
	Page::setPhysicalPage(
		m_cFile.changeFixMode(
			getPhysicalPage()));
	
	reload();
}

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
