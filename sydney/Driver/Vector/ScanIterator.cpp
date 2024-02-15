// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScanIterator.cpp -- Scanモード専用オブジェクト反復子の実装ファイル
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

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Exception/BadArgument.h"
#include "Exception/NotSupported.h"
#include "Vector/FileInformation.h"
#include "Vector/FileParameter.h"
#include "Vector/Object.h"
#include "Vector/OpenParameter.h"
#include "Vector/PageManager.h"
#include "Vector/ScanIterator.h"

_SYDNEY_USING

using namespace Vector;

//
//	FUNCTION
//	Vector::ScanIterator::ScanIterator -- コンストラクタ
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
ScanIterator::ScanIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_)
		: ReadIterator(rFileParameter_,
			rOpenParameter_,
			rPageManager_)
{
}

//
//	FUNCTION
//	Vector::ScanIterator::~ScanIterator -- デストラクタ
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
ScanIterator::~ScanIterator()
{
}

//
// オブジェクトをファイルから取得する
// Leakable!
//
//	FUNCTION
//	Vector::ScanIterator::get -- 
//
//	NOTE
//		
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
ScanIterator::get(Common::DataArrayData* pTuple_)
{
	if (m_bGetByBitSet)
	{
		Common::Data::Pointer pElement = pTuple_->getElement(0);
		if (pElement->getType() != Common::DataType::BitSet)
			_SYDNEY_THROW0(Exception::BadArgument);

		Common::BitSet* pBitSet =
			_SYDNEY_DYNAMIC_CAST(Common::BitSet*, pElement.get());
		pBitSet->reset();

		getBitSet(pBitSet);
		return true;
	}
	else if (next())
	{
		return read(pTuple_);
	}
	else // no more object
	{
		return false;
	}
}

#if 0
//
//	FUNCTION
//	Vector::ScanIterator::fetch -- 検索するオブジェクトを指定する
//
//	NOTE
//		検索するオブジェクトを指定する
//
//	ARGUMENTS
//		const ModUInt32 ulVectorKey_
//			
//	RETURN
//		void 
//
//	EXCEPTIONS
//		Exception::NotSupported
//			
void 
ScanIterator::fetch(const ModUInt32 ulVectorKey_)
{
	// scanではfetch()を用いないので例外とする
	throw Exception::NotSupported(moduleName, srcFile, __LINE__);
}
#endif

// 間接的に使用する関数

//
//	FUNCTION
//	Vector::ScanIterator::next -- 順方向への移動
//
//	NOTE
//		順方向への移動。
//		ファイル末尾を指しているときには移動しない。
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//			
bool
ScanIterator::next()
{
	return next(false);
}

//
//	FUNCTION
//	Vector::ScanIterator::prev -- 逆方向への移動
//
//	NOTE
//		逆方向への移動。
//		ファイル先頭を指しているときには移動しない。
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//			
bool
ScanIterator::prev()
{
	return next(true);
}

// PRIVATE FUNCTIONS

//
void
ScanIterator::getBitSet(Common::BitSet* pBitSet_)
{
	// read()するとしないとでは効率に大いに差あり
	PageManager::AutoPageObject obj(m_rPageManager);//スコープを抜けると自動的にdetachする。
	if (m_rOpenParameter.getOuterMaskAt(0)) {
		while (next())
			pBitSet_->set(m_ulCursor);
	} else {
		Common::DataArrayData cData;
		cData.setElement(0, new Common::UnsignedIntegerData);
		while (next()) {
			read(obj, &cData);
			pBitSet_->set(getUI32Value(&cData));
		}
	}
}

//
//	FUNCTION
//	Vector::ScanIterator::next -- 指定した方向に移動
//
//	NOTE
//		指定した方向に移動。
//		ファイルの端を指しているときには移動しない。
//
//	ARGUMENTS
//		bool bDirection_
//			
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//			
bool
ScanIterator::next(bool bDirection_)
{
	ModUInt32 ulNextCursor;
	if (m_ulCursor == FileCommon::VectorKey::Undefined)
	{
		ulNextCursor = getStartVectorKey();
	}
	else
	{
		ulNextCursor = m_rPageManager.nextVectorKey
			(m_ulCursor, m_rOpenParameter.getSortOrder() ^ bDirection_);
	}


	if (m_ulCursor == ulNextCursor)
	{
		return false;
	}
	else
	{
		m_ulCursor = ulNextCursor;
		return true;
	}
	// pageManager::nextVectorKeyにgetStartVectorKeyを統合できないか?
}

// 最新のファイル管理情報に基づいて先頭のベクタキーを得る。
ModUInt32
ScanIterator::getStartVectorKey()
{
	ModUInt32 ulStartVectorKey;

	AutoFileInformation fileinfo(m_rPageManager);
	fileinfo.attach();//自動的に、detachする。

	if (m_rOpenParameter.getSortOrder()) {	// true == backward
		ulStartVectorKey = fileinfo->getLastVectorKey();
	} else { // false == forward
		ulStartVectorKey = fileinfo->getFirstVectorKey();
	}

	return ulStartVectorKey;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
