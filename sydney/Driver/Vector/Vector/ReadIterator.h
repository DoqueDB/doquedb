// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReadIterator.h -- 読み込み専用オブジェクト反復子のヘッダファイル
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

#ifndef __SYDNEY_VECTOR_READITERATOR_H
#define __SYDNEY_VECTOR_READITERATOR_H

#include "Vector/ObjectIterator.h"
#include "Vector/PageManager.h"

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
//	Vector::ReadIterator -- 読み込み専用オブジェクト反復子。
//
//	NOTES
//	さらに{Scan, Search, Fetch}Iteratorを派生する。
//	

class ReadIterator : public ObjectIterator
{
public:
	// コンストラクタはprotected

	//デストラクタ
	virtual ~ReadIterator();

	// オブジェクトをマークする
	void mark();
	// オブジェクトの読み込みを巻き戻す
	void rewind();
	// 反復子を初期状態に戻す
	void reset();

protected:
	// コンストラクタ
	ReadIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_);

	// 物理ファイルからオブジェクトの値を読み出す
	bool read(Common::DataArrayData* pTuple_);
	bool read(PageManager::AutoPageObject& obj_,
			  Common::DataArrayData* pTuple_);

	// Common::Data*からModUInt32を得る
	ModUInt32	getUI32Value(Common::Data* pData_);

	// 反復子が指しているオブジェクトのベクタキー(2001-01-15追加)
	ModUInt32	m_ulCursor;

	// 巻き戻しのために記録されるベクタキー
	ModUInt32	m_ulMark;

	// GetByBitSetモードかどうか
	const bool	m_bGetByBitSet;
};

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_READITERATOR_H

//
//	Copyright (c) 2000, 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
