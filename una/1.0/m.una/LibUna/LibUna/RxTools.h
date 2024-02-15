// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RxTools.h -- Definition file of RxTools class
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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

#ifndef __RXTOOLS__HEADER__
#define __RXTOOLS__HEADER__

#include "Morph.h"
#include "ModUnicodeString.h"
#include "ModMap.h"
#include "Module.h"

_UNA_BEGIN

//
//	NAMESPACE
//		RxTools -- 品詞を正規表現で比較する際のヘルパ関数群
//
	namespace RxTools
	{
		// 品詞番号に対応する UNICODE 文字を得る
		ModUnicodeChar		getUnicodeChar(Morph::Type::Value type_);

		// 形態素列から品詞化された文字列を取得する
		void				getTypeString(ModVector<Morph>::ConstIterator srcIt_,
							  ModVector<Morph>::ConstIterator srcFin_,
							  ModVector<ModUnicodeChar>& type_);

		// 日本語の正規表現の品詞文字列から品詞番号用の正規表現文字列を作成する
		// For Japanese
		void				convertJpString(const ModUnicodeString& src_,
								ModUnicodeString& dst_);
		// 文字列を置き換える
		void				findReplaceString(const ModUnicodeString& src_,
								const ModMap<ModUnicodeString, ModUnicodeString,
								ModLess<ModUnicodeString> >& list_,
								ModUnicodeChar delim_,
								ModUnicodeString& dst_);
	} // end of namepsace RxTools

_UNA_END

#endif // __RXTOOLS__HEADER__

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
