// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Config.h -- Ｂ＋木ファイルドライバの設定関連の関数宣言
// 
// Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_CONFIG_H
#define __SYDNEY_BTREE_CONFIG_H

#include "Common/Common.h"
#include "Common/Internal.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	NAMESPACE
//	Btree::Config -- Ｂ＋木ファイルドライバの設定に関する名前空間
//
//	NOTES
//	Ｂ＋木ファイルドライバの設定に関する名前空間。
//
namespace Config
{
#ifdef OBSOLETE
	// 設定をすべてシステムパラメータから読み出す
	void get();
#endif //OBSOLETE

#ifdef OBSOLETE
	// 設定のリセットを行う
	void reset();
#endif //OBSOLETE

} // end of namespace Btree::Config

} // end of namespace Btree

_SYDNEY_END

#endif //__SYDNEY_BTREE_CONFIG_H

//
//	Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
