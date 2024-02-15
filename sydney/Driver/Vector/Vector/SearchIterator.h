// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchIterator.h -- Searchモード専用のオブジェクト反復子のヘッダファイル
// 
// Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_SEARCHITERATOR_H
#define __SYDNEY_VECTOR_SEARCHITERATOR_H

#include "FileCommon/VectorKey.h"
#include "Vector/ReadIterator.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
}

namespace PhysicalFile
{
class File;
class Page;
}

namespace Vector
{
class FileParameter;
class OpenParameter;
class FileInformation;
class Object;

//
//	CLASS
//	Vector::SearchIterator -- Searchモード専用のオブジェクト反復子。
//
//	NOTES
//
class SearchIterator : public ReadIterator
{
public:
	// コンストラクタ
	SearchIterator(
		FileParameter&	rFileParameter_,
		OpenParameter&	rOpenParameter_,
		PageManager&	rPageManager_,
		ModUInt32		ulVectorKey_
			= FileCommon::VectorKey::Undefined);
	//デストラクタ
	~SearchIterator();

	// アクセサ・マニピュレータ

	// オブジェクトをファイルから取得する
	bool get(Common::DataArrayData* pTuple_);
#if 0
	// 検索するオブジェクトを指定する
	void fetch(const ModUInt32 ulVectorKey_);
#endif

private:

	// サーチの対象であるベクタキー(固定)
	const ModUInt32	m_ulSearchingVectorKey;
};

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_SEARCHITERATOR_H

//
//	Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
