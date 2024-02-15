// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FetchIterator.cpp -- Fetchモード専用オブジェクト反復子の実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Vector";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "ModAutoPointer.h"
#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Vector/Object.h"
#include "Vector/OpenParameter.h"
#include "Vector/FetchIterator.h"

_SYDNEY_USING

using namespace Vector;

//
//	FUNCTION
//	Vector::FetchIterator::FetchIterator -- コンストラクタ
//
//	NOTE
//		コンストラクタ
//
//	ARGUMENTS
//		FileParameter&		rFileParameter_
//			
//		OpenParameter&		rOpenParameter_
//			
//		PageManager&		rPageManager_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
FetchIterator::FetchIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_):
	ReadIterator(rFileParameter_,
				 rOpenParameter_,
				rPageManager_),
	m_ulFetchingVectorKey
		(FileCommon::VectorKey::Undefined)
{
}

//
//	FUNCTION
//	Vector::FetchIterator::~FetchIterator -- デストラクタ
//
//	NOTE
//		デストラクタ
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
FetchIterator::~FetchIterator()
{
}

//
//	FUNCTION
//	Vector::FetchIterator::get -- オブジェクトをファイルから取得する
//
//	NOTE
//		オブジェクトをファイルから取得する
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		Common::Data*
//
//	EXCEPTIONS
//		なし
//			
bool
FetchIterator::get(Common::DataArrayData* pTuple_)
{
	if (m_ulCursor == FileCommon::VectorKey::Undefined)
	{ // get前
		m_ulCursor = m_ulFetchingVectorKey;
		if (m_bGetByBitSet){
			Common::Data::Pointer pElement = pTuple_->getElement(0);
			if (pElement->getType() != Common::DataType::BitSet)
				_SYDNEY_THROW0(Exception::BadArgument);

			Common::BitSet* pBitSet =
				_SYDNEY_DYNAMIC_CAST(Common::BitSet*, pElement.get());
			pBitSet->reset();

			Common::DataArrayData cData;
			cData.setElement(0, new Common::UnsignedIntegerData);
			if (read(&cData) == true) {
				pBitSet->set(getUI32Value(&cData));
			}
			return true;
		} else {
			return read(pTuple_);
		}
	}
	else
	{  	// get後
		return false;
	}
}

//
//	FUNCTION
//	Vector::FetchIterator::fetch -- 検索するオブジェクトを指定する
//
//	NOTE
//		検索するオブジェクトを指定する。
//
//	ARGUMENTS
//		ModUInt32 ulVectorKey_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
void
FetchIterator::fetch(ModUInt32 ulVectorKey_)
{
	m_ulCursor = FileCommon::VectorKey::Undefined;
	m_ulFetchingVectorKey = ulVectorKey_;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
