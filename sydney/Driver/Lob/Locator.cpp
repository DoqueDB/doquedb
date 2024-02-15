// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Locator.cpp -- 
// 
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Lob";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Lob/Locator.h"
#include "Lob/LogicalInterface.h"
#include "Lob/ObjectID.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_LOB_USING

//
//	FUNCTION public
//	Lob::Locator::Locator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Lob::ObjectID& cObjectID_
//		オブジェクトID
//	Lob::LogicalInterface* pFile_
//		Lobファイルインターフェース
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Locator::Locator(const Trans::Transaction& cTransaction_,
				 const ObjectID& cObjectID_,
				 LogicalInterface* pFile_)
	: LogicalFileLocator(cTransaction_),
	  m_cObjectID(cObjectID_)
{
	m_pFile = LogicalInterface::attach(pFile_);
}

//
//	FUNCTION public
//	Lob::Locator::~Locator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Locator::~Locator()
{
	LogicalInterface::detach(m_pFile);
}

//
//	FUNCTION public
//	Lob::Locator::get -- 指定範囲のデータを取り出す
//
//	NOTES
//
//	ARGUMENTS
//	const Common::IntegerData* pPosition_
//		位置
//	const Common::IntegerData* pLength_
//		長さ
//	Common::Data* pResult_
//		[OUT]指定範囲のデータ
//
//	RETURN
//		true ... 正しく取り出せた
//		false... NULLだった(pResult_はNULLになる)
//
//	EXCEPTIONS
//
bool
Locator::get(const Common::UnsignedIntegerData* pPosition_,
			 const Common::UnsignedIntegerData* pLength_,
			 Common::Data* pResult_)
{
	return m_pFile->get(getTransaction(), m_cObjectID,
						pPosition_->getValue(), pLength_->getValue(),
						pResult_);
}

//
//	FUNCTION public
//	Lob::Locator::replace -- 指定範囲のデータを変更する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::UnsignedIntegerData* pPosition_
//		位置
//	const Common::Data* pData_
//		データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Locator::replace(const Common::UnsignedIntegerData* pPosition_,
				 const Common::Data* pData_)
{
	m_pFile->replace(getTransaction(), m_cObjectID,
					 pPosition_->getValue(), pData_);
}

//
//	FUNCTION public
//	Lob::Locator::append -- データを末尾に追加する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data* pData_
//		追加するデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Locator::append(const Common::Data* pData_)
{
	m_pFile->append(getTransaction(), m_cObjectID, pData_);
}

//
//	FUNCTION public
//	Lob::Locator::truncate -- データを末尾から指定サイズ分切り詰める
//
//	NOTES
//
//	ARGUMENTS
//	const Common::UnsignedIntegerData* pLength_
//		切り詰める長さ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Locator::truncate(const Common::UnsignedIntegerData* pLength_)
{
	m_pFile->truncate(getTransaction(), m_cObjectID, pLength_->getValue());
}

//
//	FUNCTION public
//	Lob::Locator::length -- データ長を得る
//
//	NOTES
//
//	ARGUMENTS
//	Common::UnsignedIntegerData* pResult_
//		[OUT]データ長
//
//	RETURN
//		なし	
//
//	EXCEPTIONS
//
void
Locator::length(Common::UnsignedIntegerData* pResult_)
{
	pResult_->setValue(m_pFile->getDataSize(getTransaction(), m_cObjectID));
}

//
//	Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
