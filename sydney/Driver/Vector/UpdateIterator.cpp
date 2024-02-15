// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateIterator.cpp -- Updateモード専用オブジェクト反復子の実装ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

#include "Checkpoint/Database.h"
#include "Common/Assert.h"
#include "Common/Message.h"
#include "Exception/BadArgument.h"

#include "Vector/FileInformation.h"
#include "Vector/FileParameter.h"
#include "Vector/Object.h"
#include "Vector/PageManager.h"
#include "Vector/UpdateIterator.h"

_SYDNEY_USING
_SYDNEY_VECTOR_USING

//
//	FUNCTION
//	Vector::UpdateIterator::UpdateIterator -- コンストラクタ
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
//		const Trans::Transaction&	rTransaction_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//			
//
UpdateIterator::UpdateIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_,
		const Trans::Transaction&	rTransaction_) :
	ObjectIterator(rFileParameter_,
				   rOpenParameter_,
				   rPageManager_),
	m_rTransaction(rTransaction_)
{
}

//
//	FUNCTION
//	Vector::UpdateIterator::~UpdateIterator -- デストラクタ
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
UpdateIterator::~UpdateIterator()
{
}

//
//	FUNCTION
//	Vector::UpdateIterator::update -- オブジェクトを更新する
//
//	NOTE
//      オブジェクトを更新する
//
//	ARGUMENTS
//		ModUInt32 ulVectorKey_
//			
//		const Common::DataArrayData& rArrayData_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		"Fatal"
//			
void
UpdateIterator::update(ModUInt32 ulVectorKey_,
					   const Common::DataArrayData& rArrayData_)
{
	PageManager::AutoPageObject obj(m_rPageManager);//スコープを抜けると自動的にdetachする。

	obj.attach(ulVectorKey_, Buffer::Page::FixMode::Write);
	if (obj == 0) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}		

	// バックアップを取る
	Common::DataArrayData	cBackArrayData(rArrayData_);
	int element = 0;

	obj->read(cBackArrayData, element);

	try {
		// updateの本体
		obj->update(rArrayData_);
	} catch(...) { // 失敗した
		try {
			// エラー処理 : フィールドの内容を元に戻す
			obj->update(cBackArrayData);
		} catch (...) {

			// 元に戻せなかったので、利用不可にする

			SydErrorMessage << "fatal error" << ModEndl;

			Checkpoint::Database::setAvailability(
				m_rFileParameter.getLockName(), false);

			obj.dirty();
		}
		_SYDNEY_RETHROW;
	}

	obj.dirty();
}
						
//
//	FUNCTION
//	Vector::UpdateIterator::insert -- オブジェクトを挿入する
//
//	NOTE
//		オブジェクトを挿入する
//
//	ARGUMENTS
//		ModUInt32 ulVectorKey_
//			
//		const Common::DataArrayData& rArrayData_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		BadArgument
//		"Fatal"
//
void
UpdateIterator::insert(ModUInt32 ulVectorKey_,
					   const Common::DataArrayData& rArrayData_)
{
	PageManager::AutoPageObject obj(m_rPageManager);//スコープを抜けると自動的にdetachする。

	obj.attach(ulVectorKey_);
	if (obj == 0) {	// nullなのでdetachObjectの必要なし
#ifdef DEBUG
		SydDebugMessage << "no such vkey " << ulVectorKey_ << ModEndl;
#endif
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
		return;
	}

	{
	AutoFileInformation fileinfo(m_rPageManager);
	fileinfo.attach(Buffer::Page::FixMode::Write);//自動的に、detachする。

	ModUInt32 ulObjs = fileinfo->getObjectCount();
	
	try {
		obj->insert(rArrayData_);
	} catch(...) {
		try {
			// エラー処理 : オブジェクトの内容を元に戻す
			obj->unsetBit();
		} catch (...) {

			// 元に戻せなかったので、利用不可にする

			SydErrorMessage << "fatal error" << ModEndl;

			Checkpoint::Database::setAvailability(
				m_rFileParameter.getLockName(), false);

			obj.dirty();
		}
		_SYDNEY_RETHROW;
	}
	obj.dirty();
	obj.detach();
	
	// 実際のinsertより後でないと値が食い違う
	++ulObjs;
	fileinfo->setObjectCount(ulObjs);
	if (ulObjs == 1) {
		fileinfo->setFirstVectorKey(ulVectorKey_);
		fileinfo->setLastVectorKey(ulVectorKey_);
	} else { // ulObjs >= 2
		if (ulVectorKey_ < fileinfo->getFirstVectorKey()) {
			fileinfo->setFirstVectorKey(ulVectorKey_);
		} else if (ulVectorKey_ > fileinfo->getLastVectorKey()) {
			fileinfo->setLastVectorKey(ulVectorKey_);
		}
	}
	}
}

//
//	FUNCTION
//	Vector::UpdateIterator::expunge -- オブジェクトを削除する
//
//	NOTE
//		オブジェクトを削除する
//
//	ARGUMENTS
//		ModUInt32 ulVectorKey_
//			
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			
//
void
UpdateIterator::expunge(ModUInt32 ulVectorKey_)
{
	PageManager::AutoPageObject obj(m_rPageManager);//スコープを抜けると自動的にdetachする。

	obj.attach(ulVectorKey_, Buffer::Page::FixMode::Write);
	if (obj == 0) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	{
	AutoFileInformation fileinfo(m_rPageManager);
	fileinfo.attach(Buffer::Page::FixMode::Write);//自動的に、detachする。

	try {
		obj->expunge();
	} catch(...) {
		try {
			// エラー処理 : フィールドの内容を元に戻す
			obj->setBit();
		} catch (...) {

			// 元に戻せなかったので、利用不可にする

			SydErrorMessage << "Fatal error." << ModEndl;

			Checkpoint::Database::setAvailability(
				m_rFileParameter.getLockName(), false);

			obj.dirty();
		}
		_SYDNEY_RETHROW;
	}
	obj.dirty();
	obj.detach();
	
	ModUInt32 ulObjs = fileinfo->getObjectCount();

	switch (ulObjs) {
	case 0:
#ifdef DEBUG
		SydDebugMessage << "File is already empty."  << ModEndl;
#endif
		SydAssert(false); // ありえない(はず)
		break;
	// {First, Last}VectorKeyの変更
	case 1:
		fileinfo->setFirstVectorKey(FileCommon::VectorKey::Undefined);
		fileinfo->setLastVectorKey(FileCommon::VectorKey::Undefined);
		break;
	default: // >= 2
		if (ulVectorKey_ == fileinfo->getFirstVectorKey()) {
			fileinfo->setFirstVectorKey(m_rPageManager.nextVectorKey(ulVectorKey_, false));
		} else if (ulVectorKey_ == fileinfo->getLastVectorKey()) {
			fileinfo->setLastVectorKey(m_rPageManager.nextVectorKey(ulVectorKey_, true));
		}
		break;
	}

	fileinfo->setObjectCount(ulObjs-1);
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
