// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScanIterator.h -- Scanモード専用のオブジェクト反復子のヘッダファイル
// 
// Copyright (c) 2000, 2001, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_SCANITERATOR_H
#define __SYDNEY_VECTOR_SCANITERATOR_H

#include "Vector/ReadIterator.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
class BitSet;
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

//	CLASS
//	Vector::ScanIterator -- Scanモード専用のオブジェクト反復子。
//
//	NOTES
//
class ScanIterator : public ReadIterator
{
public:
	// コンストラクタ
	ScanIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_);

	//デストラクタ
	~ScanIterator();

	// アクセサ・マニピュレータ

	// オブジェクトをファイルから取得する
	bool get(Common::DataArrayData* pTuple_);
#if 0
	// 検索するオブジェクトを指定する
	void fetch(const ModUInt32 ulVectorKey_);
#endif

	// 順方向への移動。
	// ファイル末尾を指しているときには移動しない。
	bool next();

	// 逆方向への移動。
	// ファイル先頭を指しているときには移動しない。
	bool prev();

private:

	void getBitSet(Common::BitSet* pBitSet_);

	// next/prevの実体。返り値は動いたか動かなかったか。
	bool next(bool bDirection_);

	// 先頭のベクタキーを調べる
	ModUInt32 getStartVectorKey();
};

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_SCANITERATOR_H

//
//	Copyright (c) 2000, 2001, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
