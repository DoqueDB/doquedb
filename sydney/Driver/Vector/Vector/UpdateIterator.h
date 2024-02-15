// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateIterator.h -- 書き込み専用オブジェクト反復子のヘッダファイル
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_UPDATEITERATOR_H
#define __SYDNEY_VECTOR_UPDATEITERATOR_H

#include "Common/Common.h"
#include "Common/Object.h"
#include "FileCommon/VectorKey.h"

#include "Vector/ObjectIterator.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
}

namespace Trans
{
class Transaction;
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
//	Vector::UpdateIterator -- Updateモード専用のオブジェクト反復子。
//
//	NOTES
//
class UpdateIterator : public ObjectIterator
{
public:
	// コンストラクタ
	UpdateIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_,
		const Trans::Transaction& rTransaction_);
	//デストラクタ
	~UpdateIterator();

	// マニピュレータ
	void update(ModUInt32 ulVectorKey_,
				const Common::DataArrayData& rArrayData_);

	// 第二引数はpopFront済
	void insert(ModUInt32 ulVectorKey_,
				const Common::DataArrayData& rArrayData_);
	void expunge(ModUInt32 ulVectorKey_);

private:

	// setFatal()のために必要
	const Trans::Transaction&	m_rTransaction;
};

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_UPDATEITERATOR_H

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
