// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KeyPosType.h -- キー記録位置のヘッダーファイル
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

#ifndef __SYDNEY_BTREE_KEYPOSTYPE_H
#define __SYDNEY_BTREE_KEYPOSTYPE_H

#include "Btree/Module.h"

#include "Common/Object.h"

#include "Os/Memory.h"

_SYDNEY_BEGIN

namespace Btree
{

//
//	STRUCT public
//	Btree::KeyPosType -- キー値記録先
//
//	NOTES
//	キー値記録先を示すための構造体。
//	v4.0までは、キー値はキー情報とは別の領域に
//	“キーオブジェクト”として記録していた。
//	v5.0からは、これまでの“キーオブジェクト”として記録する
//	タイプに加え、キー情報内に記録するタイプのファイルも
//	サポートする。
//	ファイルがいずれのタイプになるかは、
//	キー値の記録サイズによる。
//	（極小さなキー値ならばキー情報内に記録する。）
//
struct KeyPosType
{
	//
	//	ENUM public
	//	Btree::KeyPosType::Value -- キー値記録先
	//
	//	NOTES
	//	キー値記録先を示す。
	//
	enum Value
	{
		KeyInfo = 0, // キー値はキー情報内に記録する
		KeyObject,   // キー値はキーオブジェクトに記録する
		Undefined
	};

	// キー値記録先を決める閾値 [byte]
	static const Os::Memory::Size	KeyPosThreshold; // = 12
};

} // end of name space Btree

_SYDNEY_END

#endif // __SYDNEY_BTREE_KEYPOSTYPE_H

//
//	Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
