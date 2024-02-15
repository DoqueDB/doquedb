// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedUnicodeCharBlocker.h -- 転置のための日本語処理器の定義
// 
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedUnicodeCharBlocker_H__
#define __ModInvertedUnicodeCharBlocker_H__

#include "ModUnicodeChar.h"
#include "ModInvertedManager.h"

class ModCharString;

//
// CLASS
// ModInvertedUnicodeCharBlocker -- 転置のための日本語処理器
//
// NOTES
// 分割器の日本語化のために用いる。
// JIS コードの区分と、日常感覚による文字種の判定のずれを補正する。
// 
class
ModInvertedUnicodeCharBlocker : public ModInvertedObject {
public:
	static ModInvertedUnicodeCharBlocker* create();
	static ModInvertedUnicodeCharBlocker* create(const ModCharString&);

	virtual ~ModInvertedUnicodeCharBlocker() {}

	virtual ModSize getBlockNum() const = 0;
	virtual void getBlockRegion(const ModSize,
								ModUnicodeChar&, ModUnicodeChar&) const = 0;
	virtual ModSize getBlock(const ModUnicodeChar) const = 0;
};

#endif	__ModInvertedUnicodeCharBlocker_H__

//
// Copyright (c) 2000, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

