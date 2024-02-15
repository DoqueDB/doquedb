// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KeyPosType.cpp --
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Btree/KeyPosType.h"

_SYDNEY_USING

using namespace Btree;

//
//	CONST public
//	Btree::KeyPosType::KeyThreashold
//		キー値記録先を決める閾値
//
//	NOTES
//	キー値をキー情報に記録するかどうかの閾値。
//	（キー値の記録サイズ[byte]）
//	キー値の記録サイズがこの閾値以下の場合には、
//	各ノード内のキー値はキー情報に記録され、
//	この閾値を超える場合には、キー値はキーオブジェクトに記録される。
//
// static
const Os::Memory::Size
KeyPosType::KeyPosThreshold = 12;

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
