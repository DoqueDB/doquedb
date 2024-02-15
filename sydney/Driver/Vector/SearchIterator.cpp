// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SearchIterator.cpp -- Searchモード専用オブジェクト反復子の実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
#include "Common/BitSet.h"
#include "Common/UnsignedIntegerData.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Vector/Object.h"
#include "Vector/OpenParameter.h"
#include "Vector/SearchIterator.h"

_SYDNEY_USING

using namespace Vector;

//
//	FUNCTION
//	Vector::SearchIterator::SearchIterator -- コンストラクタ
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
//		ModUInt32 ulVectorKey_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
SearchIterator::SearchIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_,
		ModUInt32			/*ulVectorKey_*/)
	: ReadIterator( rFileParameter_,
				    rOpenParameter_,
				    rPageManager_)
	, m_ulSearchingVectorKey(m_rOpenParameter.getSearchValue())
{
}

//
//	FUNCTION
//	Vector::SearchIterator::~SearchIterator -- デストラクタ
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
SearchIterator::~SearchIterator()
{
}

//
//	FUNCTION
//	Vector::SearchIterator::get -- オブジェクトをファイルから取得する
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
SearchIterator::get(Common::DataArrayData* pTuple_)
{
	if (m_ulCursor == FileCommon::VectorKey::Undefined) {
		m_ulCursor = m_ulSearchingVectorKey;
		if (m_bGetByBitSet) {
			; _SYDNEY_ASSERT(pTuple_->getElement(0).get() != 0);
			; _SYDNEY_ASSERT(pTuple_->getElement(0)->getType() == Common::DataType::BitSet);
			Common::DataArrayData cData;
			cData.setElement(0, new Common::UnsignedIntegerData);
			if (read(&cData) == true) {
				Common::BitSet* pBitSet = _SYDNEY_DYNAMIC_CAST(Common::BitSet*, pTuple_->getElement(0).get());
				pBitSet->set(getUI32Value(&cData));
			}
			return true;
		} else {
			return read(pTuple_);
		}
	} else {
		return false;
	}
}

#if 0
//
//	FUNCTION
//	Vector::SearchIterator::fetch -- 検索するオブジェクトを指定する
//
//	NOTE
//		検索するオブジェクトを指定する
//
//	ARGUMENTS
//		const ModUInt32 ulVectorKey_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::NotSupported
//			
void
SearchIterator::fetch(const ModUInt32 ulVectorKey_)
{
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}
#endif

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
