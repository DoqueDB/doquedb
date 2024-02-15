// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchListMap.cpp --
// 
// Copyright (c) 2002, 2003, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/BatchListMap.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::BatchListMap::BatchListMap -- コンストラクタ
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
//	なし
//
BatchListMap::BatchListMap()
	: m_uiListSize(0), m_uiLastDocumentID(0), m_uiMaxRowID(0)
{
}

//
//	FUNCTION public
//	Inverted::BatchListMap::~BatchListMap -- デストラクタ
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
//	なし
//
BatchListMap::~BatchListMap()
{
	Iterator i = begin();
	for (; i != end(); ++i)
	{
		ModList<BatchBaseList*>::Iterator j = (*i).second.begin();
		for (; j != (*i).second.end(); ++j)
			delete (*j);
	}
}

//
//	FUNCTION public
//	Inverted::BatchListMap::insertEntry -- リストを挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrKey_
//		キー
//	const ModList<Inverted::BatchBaseList*>& cList_
//		リスト
//
//	RETURN
//	ModPair<Inverted::BatchListMap::Iterator, ModBoolean>
//		エントリのイテレータ
//
//	EXCEPTIONS
//
ModPair<BatchListMap::Iterator, ModBoolean>
BatchListMap::insertEntry(const ModUnicodeString& cstrKey_,
						  const ModList<BatchBaseList*>& cList_)
{
	ModPair<Iterator, ModBoolean> r = Super::insert(cstrKey_, cList_);
	if (r.second == ModTrue)
	{
		addListSize(cstrKey_.getBufferSize());
		addListSize(sizeof(ModValueNode<
						   ModPair<
						   ModUnicodeString, ModList<BatchBaseList*> > >));
	}
	return r;
}

//
//	Copyright (c) 2002, 2003, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
