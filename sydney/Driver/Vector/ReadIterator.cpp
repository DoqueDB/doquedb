// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReadIterator.cpp -- 読み込み専用オブジェクト反復子の実装ファイル
// 
// Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
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
#include "FileCommon/VectorKey.h"
#include "Vector/FileParameter.h"
#include "Vector/OpenParameter.h"
#include "Vector/Object.h"
#include "Vector/ReadIterator.h"

_SYDNEY_USING

using namespace Vector;

// コンストラクタはprotected

//
//	FUNCTION
//	Vector::ReadIterator::~ReadIterator -- デストラクタ
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
ReadIterator::~ReadIterator()
{
}

// PROTECTED FUNCTIONS

//
//	FUNCTION
//	Vector::ReadIterator::ReadIterator -- コンストラクタ
//
//	NOTE
//		コンストラクタ
//
//	ARGUMENTS
//		FileParameter&	rFileParameter_
//			
//		OpenParameter&	rOpenParameter_
//			
//		PageManager&	rPageManager_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
ReadIterator::ReadIterator(
		FileParameter&	rFileParameter_,
		OpenParameter&	rOpenParameter_,
		PageManager&	rPageManager_) :
	ObjectIterator(rFileParameter_,
				   rOpenParameter_,
				   rPageManager_), 
	m_ulCursor(FileCommon::VectorKey::Undefined),
	m_ulMark(FileCommon::VectorKey::Undefined),
	m_bGetByBitSet(m_rOpenParameter.getsByBitSet())
{
}

//
//	FUNCTION
//	Vector::ReadIterator::read -- 物理ファイルからオブジェクトの値を読み出す
//
//	NOTE
//		物理ファイルからオブジェクトの値を読み出す
//
//	ARGUMENTS
//		なし
//			
//	RETURN
//		Common::Data*
//
//	EXCEPTIONS
//		Exception::Unexpected
//			
bool
ReadIterator::read(Common::DataArrayData* pTuple_)
{
	PageManager::AutoPageObject obj(m_rPageManager);//スコープを抜けると自動的にdetachする。
	return read(obj, pTuple_);
}

bool
ReadIterator::read(PageManager::AutoPageObject& obj_,
				   Common::DataArrayData* pTuple_)
{
	obj_.attach(m_ulCursor, Buffer::Page::FixMode::ReadOnly);
	if (obj_ == 0) {
		// 対応するオブジェクトが存在しなければnullを返す
		return false;
	}

	int i = 0;
	//	外部フィールドIDがプロジェクションの対象に含まれている場合、
	//	返り値の先頭に含める
	if (m_rOpenParameter.getOuterMaskAt(0)) {
		Common::Data::Pointer p = pTuple_->getElement(i++);
		; _SYDNEY_ASSERT(p->getType() == Common::DataType::UnsignedInteger);
		_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, p.get())
			->setValue(m_ulCursor);
	}

	try {
		obj_->read(*pTuple_, i);
	} catch (Exception::BadArgument&) { // from Object::Read
		return false;
	}

	return true;
}

//
//	FUNCTION
//	Vector::ReadIterator::mark -- オブジェクトをマークする
//
//	NOTE
//		オブジェクトをマークする。
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
void
ReadIterator::mark()
{
	m_ulMark = m_ulCursor;
}

//
//	FUNCTION
//	Vector::ReadIterator::rewind -- オブジェクトの読み込みを巻き戻す
//
//	NOTE
//		オブジェクトの読み込みを巻き戻す。
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
void
ReadIterator::rewind()
{
	m_ulCursor = m_ulMark;
	m_ulMark = FileCommon::VectorKey::Undefined;
	// mark されていないのに rewind を呼ぶとreset()と同等になる。
}

//
//	FUNCTION
//	Vector::ReadIterator::reset -- 反復子を初期状態に戻す
//
//	NOTE
//		反復子を初期状態に戻す
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
void
ReadIterator::reset()
{
	m_ulCursor = FileCommon::VectorKey::Undefined;
	m_ulMark = FileCommon::VectorKey::Undefined;
}

//
ModUInt32
ReadIterator::getUI32Value(Common::Data* pData_)
{
	Common::DataArrayData* pTuple = 
		_SYDNEY_DYNAMIC_CAST(Common::DataArrayData* ,pData_);
	_SYDNEY_ASSERT(pTuple);
	Common::UnsignedIntegerData* pValue = 
		_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData* ,pTuple->getElement(0).get());
	_SYDNEY_ASSERT(pValue);
	return pValue->getValue();
}

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
