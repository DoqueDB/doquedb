// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TermResourceManager.cpp -- 
// 
// Copyright (c) 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Utility/TermResourceManager.h"
#include "Utility/ModTerm.h"

#include "Common/Assert.h"
#include "Common/SystemParameter.h"
#include "Common/UnicodeString.h"

#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Path.h"

#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModOstrStream.h"

_TRMEISTER_USING
_TRMEISTER_UTILITY_USING

namespace
{
	//
	//	STRUCT
	//	_$$::_Resource -- 質問処理リソースを表す構造体
	//
	struct _Resource
	{
		// コンストラクタ
		_Resource(ModSize id)
		{
			ModOstrStream stream;
			stream << "Inverted_TermResource_" << id;
			Os::Path path;
			Common::SystemParameter::getValue(
				_TRMEISTER_U_STRING(stream.getString()), path);

			_p = new ModTermResource(path);
			; _SYDNEY_ASSERT(_p);
		}
		
		// デストラクタ
		~_Resource() { delete _p; }
		
		mutable ModTermResource* _p;
	};

	// 排他制御用のラッチ
	Os::CriticalSection _latch;

	// リソースを管理するためのハッシュマップ
	typedef ModHashMap<ModSize, _Resource*, ModHasher<ModSize> > _ResourceMap;
	_ResourceMap _resourceMap;

}

//
//	FUNCTION public
//	FullText2::TermResourceManager::get -- 質問処理リソースを得る
//
//	NOTES
//
//	ARGUMETNS
//	ModSize uiResourceID_
//		リソース番号
//
//	RETUEN
//	Utility::ModTermResource*
//		質問処理リソース
//
//	EXCEPTIONS
//
const ModTermResource*
TermResourceManager::get(ModSize uiResourceID_)
{
	Os::AutoCriticalSection latch(_latch);

	// すでに生成済みのリソースがあるか調べる
	_ResourceMap::Iterator i = _resourceMap.find(uiResourceID_);
	if (i == _resourceMap.end())
	{
		// ないので読み込む
		ModAutoPointer<_Resource> r(new _Resource(uiResourceID_));
		ModPair<_ResourceMap::Iterator, ModBoolean> p
			= _resourceMap.insert(uiResourceID_, r.get());
		i = p.first;
		r.release();
	}
	return (*i).second->_p;
}

//
//	FUNCTION public
//	FullText2::TermResourceManager::terminate -- 質問処理リソースを開放する
//
//	NOTES
//
//	ARGUMETNS
//
//	RETUEN
//
//	EXCEPTIONS
//
void
TermResourceManager::terminate()
{
//#ifdef PURIFY
	Os::AutoCriticalSection latch(_latch);

	// 生成済みのリソースを削除する
	_ResourceMap::Iterator i = _resourceMap.begin();
	for (; i != _resourceMap.end(); ++i)
	{
		delete (*i).second;
	}
	_resourceMap.erase(_resourceMap.begin(), _resourceMap.end());
//#endif
}

//
//	Copyright (c) 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
