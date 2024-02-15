// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectID.cpp -- オブジェクト ID 関連の関数定義
// 
// Copyright (c) 2000, 2001, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/ObjectID.h"
#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Schema/Recovery.h"
#include "Schema/Sequence.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::ObjectID::assign -- 新しいオブジェクト ID の値を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database* pDatabase_
//			オブジェクトが属するデータベース
//		Schema::ObjectID::Value iID_ = Invalid
//			Invalid以外が渡されたとき、Sequenceファイルの整合性を取る
//		Schema::ObjectID::Value iInitID_ = Invalid
//			未作成のときに使用する初期値
//
//	RETURN
//		生成された新しいオブジェクト ID の値
//
//	EXCEPTIONS

// static
ObjectID::Value
ObjectID::assign(Trans::Transaction& cTrans_, Database* pDatabase_, Value iID_ /* = Invalid */, Value iInitID_ /* = Invalid */)
{
	if (!pDatabase_ || pDatabase_->getID() == ObjectID::SystemTable) {
		// メタデータベースに属する -> データベースのオブジェクトIDを生成する

		// オブジェクト ID の値を生成するためのシーケンスを得て、
		// そのシーケンスに次のオブジェクト ID の値を生成させる

		if (iID_ == Invalid) {
			// 新規に割り当てられたIDを返す
			return Manager::ObjectTree::Sequence::get().getNextValue(cTrans_, iInitID_).getUnsigned();
		} else {
			// 与えられたIDで整合性を取る
			return Manager::ObjectTree::Sequence::get().getNextValue(iID_, cTrans_, iInitID_).getUnsigned();
		}
	}
	; _SYDNEY_ASSERT(pDatabase_ && pDatabase_->getID() != ObjectID::SystemTable);

	if (iID_ == Invalid) {
		// 新規に割り当てられたIDを返す
		return pDatabase_->getSequence().getNextValue(cTrans_, iInitID_).getUnsigned();

	} else {
		// 使用済みのうち最大のIDで整合性を取る
		Value iMaxID = Manager::RecoveryUtility::ID::getUsedIDMax(pDatabase_->getName());
		if (iMaxID > 0) {
			// 使用済みIDが登録されている場合のみ整合性を取る必要がある
			; _SYDNEY_ASSERT(iMaxID >= iID_);
			(void) pDatabase_->getSequence().getNextValue(iMaxID, cTrans_, iInitID_);
		}

		// 返り値は引数のID
		return iID_;
	}
}

// FUNCTION public
//	Schema::ObjectID::persist -- オブジェクトIDを永続化する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
ObjectID::
persist(Trans::Transaction& cTrans_, Database* pDatabase_)
{
	if (!pDatabase_ || pDatabase_->getID() == ObjectID::SystemTable)
		// メタデータベースに属する -> データベースのオブジェクトIDを永続化する
		Manager::ObjectTree::Sequence::get().persist(cTrans_);
	else
		// 通常のデータベース
		pDatabase_->getSequence().persist(cTrans_);
}

//	FUNCTION public
//	Schema::ObjectID::getLastValue -- 最後のオブジェクト ID の値を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database* pDatabase_
//			オブジェクトが属するデータベース
//
//	RETURN
//		最後のオブジェクト ID の値
//
//	EXCEPTIONS

// static
ObjectID::Value
ObjectID::
getLastValue(Trans::Transaction& cTrans_, Database* pDatabase_)
{
	if (!pDatabase_ || pDatabase_->getID() == ObjectID::SystemTable) {
		// メタデータベースに属する -> データベースのオブジェクトIDを生成する

		// オブジェクト ID の値を生成するためのシーケンスを得て、
		// そのシーケンスに次のオブジェクト ID の値を生成させる

		return Manager::ObjectTree::Sequence::get().getLastValue().getUnsigned();
	}

	return pDatabase_->getSequence().getLastValue().getUnsigned();
}

#ifdef OBSOLETE // serializeしない
//	FUNCTION public
//	Schema::ObjectID::serialize -- 
//		スキーマオブジェクトIDを表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ObjectID::
serialize(ModArchive& archiver)
{
	using namespace Common;

	// 親クラスとしてシリアル化する

	UnsignedIntegerData::serialize(archiver);
}
#endif

//
// Copyright (c) 2000, 2001, 2003, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
