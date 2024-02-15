// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaObject.cpp -- エリアオブジェクトの基本クラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
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
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Btree/AreaObject.h"

#include "Common/Assert.h"
#include "PhysicalFile/File.h"
#include "PhysicalFile/Page.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::AreaObject::AreaObject -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	物理ページをアタッチする。
//
//	ARGUMENTS
//	const Trans::Transaction*			pTransaction_,
//		トランザクション記述子
//	PhysicalFile::File*					pFile_,
//		物理ファイル記述子
//	const PhysicalFile::PageID			uiPageID_,
//		エリアオブジェクトが記録されている
//		物理エリアが存在する物理ページの識別子
//	const PhysicalFile::AreaID			uiAreaID_
//		エリアオブジェクトが記録されている物理エリアの識別子
//	const Buffer::Page::FixMode::Value	eFixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//		バッファリング内容の破棄されにくさ
//	const bool							SavePage_
//		物理ファイルマネージャでアタッチした物理ページを
//		キャッシュしておくかどうか
//			true  : キャッシュしておく
//			false : キャッシュせず、デタッチ時にアンフィックスする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
AreaObject::AreaObject(
	const Trans::Transaction*					pTransaction_,
	PhysicalFile::File*							pFile_,
	const PhysicalFile::PageID					uiPageID_,
	const PhysicalFile::AreaID					uiAreaID_,
	const Buffer::Page::FixMode::Value			eFixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_,
	const bool									SavePage_)
	: Common::Object(),
	  m_pTransaction(pTransaction_),
	  m_pFile(pFile_),
	  m_pPage(0),
	  m_AreaTop(0),
	  m_ConstAreaTop(0),
	  m_uiAreaID(uiAreaID_),
	  m_eFixMode(eFixMode_),
	  m_bAttachPageBySelf(true),
	  m_SavePage(SavePage_),
	  m_RecoverPage(false)
{
	// ページをアタッチする
	this->m_pPage = m_pFile->attachPage(*this->m_pTransaction,
										uiPageID_,
										eFixMode_,
										ReplacementPriority_);

	if (eFixMode_ == Buffer::Page::FixMode::ReadOnly)
	{
		const char*	pageTop = (*this->m_pPage).operator const char*();

		this->m_ConstAreaTop =
			pageTop + this->m_pPage->getAreaOffset(this->m_uiAreaID);
	}
	else
	{
		char*	pageTop = (*this->m_pPage).operator char*();

		this->m_AreaTop =
			pageTop + this->m_pPage->getAreaOffset(this->m_uiAreaID);
	}
}

//
//	FUNCTION public
//	Btree::AreaObject::AreaObject -- コンストラクタ
//
//	NOTES
//	コンストラクタ
//	物理ページをアタッチしない。
//
//	ARGUMENTS
//	const Trans::Transaction*	pTransaction_,
//		トランザクション記述子
//	PhysicalFile::Page*			pPage_,
//		エリアオブジェクトが記録されている
//		物理エリアが存在する物理ページの記述子
//	const PhysicalFile::AreaID	uiAreaID_
//		エリアオブジェクトが記録されている物理エリアの識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
AreaObject::AreaObject(
	const Trans::Transaction*	pTransaction_,
	PhysicalFile::Page*			pPage_,
	const PhysicalFile::AreaID	uiAreaID_)
	: Common::Object(),
	  m_pTransaction(pTransaction_),
	  m_pFile(pPage_->getFile()),
	  m_pPage(pPage_),
	  m_AreaTop(0),
	  m_ConstAreaTop(0),
	  m_uiAreaID(uiAreaID_),
	  m_eFixMode(pPage_->getFixMode()),
	  m_bAttachPageBySelf(false),
	  m_SavePage(false),
	  m_RecoverPage(false)
{
	if (this->m_eFixMode == Buffer::Page::FixMode::ReadOnly)
	{
		const char*	pageTop = (*this->m_pPage).operator const char*();

		this->m_ConstAreaTop =
			pageTop + this->m_pPage->getAreaOffset(this->m_uiAreaID);
	}
	else
	{
		char*	pageTop = (*this->m_pPage).operator char*();

		this->m_AreaTop =
			pageTop + this->m_pPage->getAreaOffset(this->m_uiAreaID);
	}
}

//
//	FUNCTION public
//	Btree::AreaObject::~AreaObject -- デストラクタ
//
//	NOTES
//	デストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
AreaObject::~AreaObject()
{
	if (this->m_bAttachPageBySelf)
	{
		// 自身で物理ページをアタッチした…

		// ページをデタッチする
		PhysicalFile::Page::UnfixMode::Value	unfixMode =
			(this->m_eFixMode == Buffer::Page::FixMode::ReadOnly ||
			 this->m_RecoverPage) ?
				PhysicalFile::Page::UnfixMode::NotDirty :
				PhysicalFile::Page::UnfixMode::Dirty;

		this->m_pFile->detachPage(this->m_pPage,
								  unfixMode,
								  this->m_SavePage);
		this->m_pPage = 0;
	}
}

//
//	FUNCTION public
//	Btree::AreaObject::getFreeAreaSize --
//		エリアオブジェクトが記録されている物理ページの
//		空き領域サイズを返す
//
//	NOTES
//	エリアオブジェクトが記録されている物理ページの
//	空き領域サイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Os::Memory::Size
//		エリアオブジェクトが記録されている物理ページの
//		空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
Os::Memory::Size
AreaObject::getFreeAreaSize() const
{
	// allocateArea した後の空き領域を求める
	return this->m_pPage->getFreeAreaSize(*this->m_pTransaction, 1);
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Btree::AreaObject::getPageID --
//		エリアオブジェクトが記録されている物理ページの
//		物理ページ識別子を返す
//
//	NOTES
//	エリアオブジェクトが記録されている物理ページの
//	物理ページ識別子を返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		エリアオブジェクトが記録されている物理ページの
//		物理ページ識別子
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID
AreaObject::getPageID() const
{
	return this->m_pPage->getID();
}
#endif //OBSOLETE

//
//	FUNCTION public
//	Btree::AreaObject::setRecoverPage --
//		アタッチした物理ページをアタッチ前の状態にするように設定する
//
//	NOTES
//	アタッチした物理ページをアタッチ前の状態にするように設定する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
AreaObject::setRecoverPage()
{
	if (this->m_bAttachPageBySelf && this->m_RecoverPage == false)
	{
		this->m_RecoverPage = true;
	}
}

//////////////////////////////////////////////////
//
//	PROTECTED METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION protected
//	Btree::AreaObject::resetPhysicalFilePage --
//		物理ページを再設定する
//
//	NOTES
//	物理ページを再設定する。
//	保持している物理ページ記述子を自身でアタッチしたのであれば、
//	その物理ページ記述子をデタッチした後、再設定する。
//
//	ARGUMENTS
//	PhysicalFile::Page*	NewPhysicalFilePage_
//		物理ページ記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
AreaObject::resetPhysicalPage(PhysicalFile::Page*	NewPhysicalPage_)
{
	if (this->m_bAttachPageBySelf)
	{
		PhysicalFile::Page::UnfixMode::Value	unfixMode =
			(this->m_eFixMode == Buffer::Page::FixMode::ReadOnly ||
			 this->m_RecoverPage) ?
				PhysicalFile::Page::UnfixMode::NotDirty :
				PhysicalFile::Page::UnfixMode::Dirty;

		this->m_pFile->detachPage(this->m_pPage,
								  unfixMode,
								  this->m_SavePage);
	}

	this->m_bAttachPageBySelf = false;

	this->m_pPage = NewPhysicalPage_;

	; _SYDNEY_ASSERT(this->m_eFixMode == this->m_pPage->getFixMode());

	if (this->m_eFixMode == Buffer::Page::FixMode::ReadOnly)
	{
		const char*	pageTop = (*this->m_pPage).operator const char*();

		this->m_ConstAreaTop =
			pageTop + this->m_pPage->getAreaOffset(this->m_uiAreaID);
	}
	else
	{
		char*	pageTop = (*this->m_pPage).operator char*();

		this->m_AreaTop =
			pageTop + this->m_pPage->getAreaOffset(this->m_uiAreaID);
	}
}

//	FUNCTION protected
//	Btree::AreaObject::getConstAreaTop --
//		物理エリア先頭へのポインタを返す
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const char*
//		物理エリア先頭へのポインタ
//
//	EXCEPTIONS

const char*
AreaObject::getConstAreaTop() const
{
	const char* const constAreaTop = (m_eFixMode == Buffer::Page::FixMode::ReadOnly) ? m_ConstAreaTop : m_AreaTop;
	; _SYDNEY_ASSERT(constAreaTop);
	return constAreaTop;
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
