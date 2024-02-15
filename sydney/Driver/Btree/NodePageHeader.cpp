// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	NodePageHeader.cpp -- ノードページヘッダクラスの実現ファイル
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

namespace
{
const char	srcFile[] = __FILE__;
const char	moduleName[] = "Btree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Btree/File.h"
#include "Btree/NodePageHeader.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

namespace
{
	struct _NodePageHeader
	{
		PhysicalFile::PageID	_parentNodePageID;
		PhysicalFile::PageID	_prevPageID;
		PhysicalFile::PageID	_nextPageID;
		ModUInt32				_keyInfoNumber;
		ModUInt32				_useKeyInfoNumber;
	};

	struct _LeafPageHeader
		: public _NodePageHeader
	{
		PhysicalFile::PageID	_prevLeafPageID;
		PhysicalFile::PageID	_nextLeafPageID;
	};
}

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	CONST public
//	Btree::NodePageHeader::AreaID --
//		ノードページヘッダエリアの物理エリア識別子
//
//	NOTES
//	ノードページヘッダエリアの物理エリア識別子。
//
// static
const PhysicalFile::AreaID
NodePageHeader::AreaID = 0;

#ifdef OBSOLETE
//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	CONST private
//	Btree::NodePageHeader::m_lParentNodePageIDOffset --
//		「親ノードページの物理ページ識別子」の
//		ノードページヘッダエリア内での位置
//
//	NOTES
//	「親ノードページの物理ページ識別子」の
//	ノードページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lParentNodePageIDOffset = 0;

//
//	CONST private
//	Btree::NodePageHeader::m_lPrevPhysicalPageIDOffset --
//		「前の物理ページの物理ページ識別子」の
//		ノードページヘッダエリア内での位置
//
//	NOTES
//	「前の物理ページの物理ページ識別子」の
//	ノードページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lPrevPhysicalPageIDOffset =
	NodePageHeader::m_lParentNodePageIDOffset +
	File::PageIDArchiveSize;

//
//	CONST private
//	Btree::NodePageHeader::m_lNextPhysicalPageIDOffset --
//		「次の物理ページの物理ページ識別子」の
//		ノードページヘッダエリア内での位置
//
//	NOTES
//	「次の物理ページの物理ページ識別子」の
//	ノードページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lNextPhysicalPageIDOffset =
	NodePageHeader::m_lPrevPhysicalPageIDOffset +
	File::PageIDArchiveSize;

//
//	CONST private
//	Btree::NodePageHeader::m_lKeyInformationNumberOffset --
//		「物理ページ内のキー情報数」の
//		ノードページヘッダエリア内での位置
//
//	NOTES
//	「物理ページ内のキー情報数」の
//	ノードページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lKeyInformationNumberOffset =
	NodePageHeader::m_lNextPhysicalPageIDOffset +
	File::PageIDArchiveSize;

//
//	CONST private
//	Btree::NodePageHeader::m_lUseKeyInformationNumberOffset --
//		「使用中の物理ページ内のキー情報数」の
//		ノードページヘッダエリア内での位置
//		
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」の
//	ノードページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lUseKeyInformationNumberOffset =
	NodePageHeader::m_lKeyInformationNumberOffset +
	File::ModUInt32ArchiveSize;

//
//	CONST private
//	Btree::NodePageHeader::m_lPrevLeafPageIDOffset --
//		「前のリーフページの物理ページ識別子」の
//		リーフページヘッダエリア内での位置
//
//	NOTES
//	「前のリーフページの物理ページ識別子」の
//	リーフページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lPrevLeafPageIDOffset =
	NodePageHeader::m_lUseKeyInformationNumberOffset +
	File::ModUInt32ArchiveSize;

//
//	CONST private
//	Btree::NodePageHeader::m_lNextLeafPageIDOffset --
//		「次のリーフページの物理ページ識別子」の
//		リーフページヘッダエリア内での位置
//
//	NOTES
//	「次のリーフページの物理ページ識別子」の
//	リーフページヘッダエリア内での位置。[byte]
//
// static
const Os::Memory::Offset
NodePageHeader::m_lNextLeafPageIDOffset =
	NodePageHeader::m_lPrevLeafPageIDOffset +
	File::PageIDArchiveSize;

//
//	CONST private
//	Btree::NodePageHeader::m_ulNodePageHeaderArchiveSize --
//		リーフページ以外のノードページのヘッダのアーカイブサイズ
//
//	NOTES
//	リーフページ以外のノードページのヘッダのアーカイブサイズ。[byte]
//
// static
const Os::Memory::Size
NodePageHeader::m_ulNodePageHeaderArchiveSize =
	File::PageIDArchiveSize * 3 +
	(File::ModUInt32ArchiveSize << 1);
	//                          ~~~~ 「掛ける２」

//
//	CONST private
//	Btree::NodePageHeader::m_ulLeafPageHeaderArchiveSize --
//		リーフページヘッダのアーカイブサイズ
//
//	NOTES
//	リーフページヘッダのアーカイブサイズ。[byte]
//
// static
const Os::Memory::Size
NodePageHeader::m_ulLeafPageHeaderArchiveSize =
	NodePageHeader::m_ulNodePageHeaderArchiveSize +
	(File::PageIDArchiveSize << 1);
	//                       ~~~~ 「掛ける２」
#endif	// OBSOLETE

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::NodePageHeader::NodePageHeader -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	親クラスであるBtree::AreaObjectのコンストラクタ内で
//	物理ページをアタッチする。
//
//	ARGUMENTS
//	const Trans::Transaction*			pTransaction_
//		トランザクション記述子
//	PhysicalFile::File*					pFile_
//		物理ファイル記述子
//	ModPhysicalPageID					ulNodePageID_
//		ノード／リーフページの物理ページ識別子
//	const Buffer::Page::FixMode::Value	eFixMode_
//		フィックスモード
//	const bool							bIsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
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
NodePageHeader::NodePageHeader(
	const Trans::Transaction*			pTransaction_,
	PhysicalFile::File*					pFile_,
	const PhysicalFile::PageID			uiNodePageID_,
	const Buffer::Page::FixMode::Value	eFixMode_,
	const bool							bIsLeafPage_,
	const bool							SavePage_)
	: Btree::AreaObject(pTransaction_,
						pFile_,
						uiNodePageID_,
						NodePageHeader::AreaID,
						eFixMode_,
						Buffer::ReplacementPriority::Low,
						SavePage_),
	  m_bIsLeafPage(bIsLeafPage_)
{
}

//
//	FUNCTION public
//	Btree::NodePageHeader::NodePageHeader -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	親クラスであるBtree::AreaObjectのコンストラクタ内で
//	物理ページをアタッチしない。
//
//	ARGUMENTS
//	const Trans::Transaction*	pTransaction_
//		トランザクション記述子
//	PhysicalFile::Page*			pPage_
//		ノードページの物理ページ記述子
//	const bool					bIsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
NodePageHeader::NodePageHeader(
	const Trans::Transaction*	pTransaction_,
	PhysicalFile::Page*			pPage_,
	const bool					bIsLeafPage_)
	: Btree::AreaObject(pTransaction_,
						pPage_,
						NodePageHeader::AreaID),
	  m_bIsLeafPage(bIsLeafPage_)
{
}

//
//	FUNCTION public
//	Btree::NodePageHeader::~NodePageHeader -- デストラクタ
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
NodePageHeader::~NodePageHeader()
{
}

//	FUNCTION public
//	Btree::NodePageHeader::getArchiveSize --
//		ノードページヘッダのアーカイブサイズを返す
//
//	NOTES
//	ノードページヘッダのアーカイブサイズを返す。
//
//	ARGUMENTS
//	const bool	bIsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページ以外のノードページ
//
//	RETURN
//	Os::Memory::Size
//		ノードページヘッダのアーカイブサイズ [byte]
//
//	EXCEPTIONS
//	なし

// static
Os::Memory::Size
NodePageHeader::getArchiveSize(const bool	bIsLeafPage_)
{
	return (bIsLeafPage_) ? sizeof(_LeafPageHeader) : sizeof(_NodePageHeader);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::resetPhysicalPage --
//		物理ページをアタッチし直す
//
//	NOTES
//	物理ページをアタッチし直す。
//
//	ARGUMENTS
//	PhysicalFile::Page*	NewPhysicalPage_
//		物理ページ記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
void
NodePageHeader::resetPhysicalPage(PhysicalFile::Page*	NewPhysicalPage_)
{
	AreaObject::resetPhysicalPage(NewPhysicalPage_);
}

//
//	書き込みメソッド
//

//	FUNCTION public
//	Btree::NodePageHeader::write --
//		ノード／リーフページヘッダを書き込む
//
//	NOTES
//	ノード／リーフページヘッダを書き込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ParentNodePageID_
//		親ノードページの物理ページ識別子
//	const PhysicalFile::PageID	PrevPhysicalPageID_
//		前の物理ページの物理ページ識別子
//	const PhysicalFile::PageID	NextPhysicalPageID_
//		次の物理ページの物理ページ識別子
//	const ModUInt32				KeyInformationNumber_
//		物理ページ内のキー情報数
//	const ModUInt32				UseKeyInformationNumber_
//		使用中の物理ページ内のキー情報数
//	const PhysicalFile::PageID	PrevLeafPageID_ = UndefinedPageID
//		前のリーフページの物理ページ識別子
//	const PhysicalFile::PageID	NextLeafPageID_ = UndefinedPageID
//		次のリーフページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::write(
	const PhysicalFile::PageID ParentNodePageID_,
	const PhysicalFile::PageID PrevPhysicalPageID_,
	const PhysicalFile::PageID NextPhysicalPageID_,
	const ModUInt32	KeyInformationNumber_,
	const ModUInt32	UseKeyInformationNumber_,
	const PhysicalFile::PageID PrevLeafPageID_,
	const PhysicalFile::PageID NextLeafPageID_)
{
	; _SYDNEY_ASSERT(m_AreaTop);

	_LeafPageHeader header;
	header._parentNodePageID = ParentNodePageID_;
	header._prevPageID = PrevPhysicalPageID_;
	header._nextPageID = NextPhysicalPageID_;
	header._keyInfoNumber = KeyInformationNumber_;
	header._useKeyInfoNumber = UseKeyInformationNumber_;
	if (m_bIsLeafPage) {
		header._prevLeafPageID = PrevLeafPageID_;
		header._nextLeafPageID = NextLeafPageID_;
	}

	(void) Os::Memory::copy(m_AreaTop, &header, getArchiveSize(m_bIsLeafPage));
}

//	FUNCTION public
//	Btree::NodePageHeader::writeParentNodePageID --
//		「親ノードページの物理ページ識別子」を書き込む
//
//	NOTES
//	「親ノードページの物理ページ識別子」を書き込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ulParentNodePageID_
//		親ノードページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writeParentNodePageID(
	const PhysicalFile::PageID uiParentNodePageID_)
{
	; _SYDNEY_ASSERT(m_AreaTop);

	(void) Os::Memory::copy(
		m_AreaTop, &uiParentNodePageID_, sizeof(uiParentNodePageID_));
}

//	FUNCTION public
//	Btree::NodePageHeader::writePrevPhysicalPageID --
//		「前の物理ページの物理ページ識別子」を書き込む
//
//	NOTES
//	「前の物理ページの物理ページ識別子」を書き込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ulPrevPhysicalPageID_
//		前の物理ページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writePrevPhysicalPageID(
	const PhysicalFile::PageID uiPrevPhysicalPageID_)
{
	; _SYDNEY_ASSERT(m_AreaTop);
	; _SYDNEY_ASSERT(m_pPage);
	; _SYDNEY_ASSERT(m_pPage->getID() != uiPrevPhysicalPageID_);

	(void) Os::Memory::copy(
		m_AreaTop + sizeof(PhysicalFile::PageID),
		&uiPrevPhysicalPageID_, sizeof(uiPrevPhysicalPageID_));
}

//	FUNCTION public
//	Btree::NodePageHeader::writeNextPhysicalPageID --
//		「次の物理ページの物理ページ識別子」を書き込む
//
//	NOTES
//	「次の物理ページの物理ページ識別子」を書き込む。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ulNextPhysicalPageID_
//		次の物理ページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writeNextPhysicalPageID(
	const PhysicalFile::PageID uiNextPhysicalPageID_)
{
	; _SYDNEY_ASSERT(m_AreaTop);
	; _SYDNEY_ASSERT(m_pPage);
	; _SYDNEY_ASSERT(m_pPage->getID() != uiNextPhysicalPageID_);

	(void) Os::Memory::copy(
		m_AreaTop + sizeof(PhysicalFile::PageID) * 2,
		&uiNextPhysicalPageID_, sizeof(uiNextPhysicalPageID_));
}

//	FUNCTION public
//	Btree::NodePageHeader::writeKeyInformationNumber --
//		「物理ページ内のキー情報数」を書き込む
//
//	NOTES
//	「物理ページ内のキー情報数」を書き込む。
//
//	ARGUMENTS
//	const ModUInt32	ulKeyInformationNumber_
//		物理ページ内のキー情報数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writeKeyInformationNumber(
	const ModUInt32	ulKeyInformationNumber_)
{
	; _SYDNEY_ASSERT(m_AreaTop);

	(void) Os::Memory::copy(
		m_AreaTop + sizeof(PhysicalFile::PageID) * 3,
		&ulKeyInformationNumber_, sizeof(ulKeyInformationNumber_));
}

//	FUNCTION public
//	Btree::NodePageHeader::writeUseKeyInformationNumber --
//		「使用中の物理ページ内のキー情報数」を書き込む
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」を書き込む。
//
//	ARGUMENTS
//	const ModUInt32	ulUseKeyInformationNumber_
//		使用中の物理ページ内のキー情報数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writeUseKeyInformationNumber(
	const ModUInt32	ulUseKeyInformationNumber_)
{
	; _SYDNEY_ASSERT(m_AreaTop);

	(void) Os::Memory::copy(
		m_AreaTop + sizeof(PhysicalFile::PageID) * 3 + sizeof(ModUInt32),
		&ulUseKeyInformationNumber_, sizeof(ulUseKeyInformationNumber_));
}

//	FUNCTION public
//	Btree::NodePageHeader::incUseKeyInformationNumber --
//		「使用中の物理ページ内のキー情報数」をインクリメントする
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」をインクリメントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::incUseKeyInformationNumber()
{
	; _SYDNEY_ASSERT(m_AreaTop);

	char* p = m_AreaTop + sizeof(PhysicalFile::PageID) * 3 + sizeof(ModUInt32);

	ModUInt32 n;
	(void) Os::Memory::copy(&n, p, sizeof(n));
	++n;
	(void) Os::Memory::copy(p, &n, sizeof(n));
}

//	FUNCTION public
//	Btree::NodePageHeader::decUseKeyInformationNumber --
//		「使用中の物理ページ内のキー情報数」をデクリメントする
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」をデクリメントする。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::decUseKeyInformationNumber()
{
	; _SYDNEY_ASSERT(m_AreaTop);

	char* p = m_AreaTop + sizeof(PhysicalFile::PageID) * 3 + sizeof(ModUInt32);

	ModUInt32 n;
	(void) Os::Memory::copy(&n, p, sizeof(n));
	--n;
	(void) Os::Memory::copy(p, &n, sizeof(n));
}

//	FUNCTION public
//	Btree::NodePageHeader::writePrevLeafPageID --
//		「前のリーフページの物理ページ識別子」を書き込む
//
//	NOTES
//	「前のリーフページの物理ページ識別子」を書き込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ulPrevLeafPageID_
//		前のリーフページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writePrevLeafPageID(
	const PhysicalFile::PageID uiPrevLeafPageID_)
{
	; _SYDNEY_ASSERT(m_bIsLeafPage);
	; _SYDNEY_ASSERT(m_AreaTop);
	; _SYDNEY_ASSERT(m_pPage);
	; _SYDNEY_ASSERT(m_pPage->getID() != uiPrevLeafPageID_);

	(void) Os::Memory::copy(
		m_AreaTop + sizeof(PhysicalFile::PageID) * 3 + sizeof(ModUInt32) * 2,
		&uiPrevLeafPageID_, sizeof(uiPrevLeafPageID_));
}

//	FUNCTION public
//	Btree::NodePageHeader::writeNextLeafPageID --
//		「次のリーフページの物理ページ識別子」を書き込む
//
//	NOTES
//	「次のリーフページの物理ページ識別子」を書き込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ulNextLeafPageID_
//		次のリーフページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
NodePageHeader::writeNextLeafPageID(
	const PhysicalFile::PageID uiNextLeafPageID_)
{
	; _SYDNEY_ASSERT(m_bIsLeafPage);
	; _SYDNEY_ASSERT(m_AreaTop);
	; _SYDNEY_ASSERT(m_pPage);
	; _SYDNEY_ASSERT(m_pPage->getID() != uiNextLeafPageID_);

	(void) Os::Memory::copy(
		m_AreaTop + sizeof(PhysicalFile::PageID) * 3 +
		sizeof(ModUInt32) * 2 + sizeof(PhysicalFile::PageID),
		&uiNextLeafPageID_, sizeof(uiNextLeafPageID_));
}

//
//	読み込みメソッド
//

//	FUNCTION public
//	Btree::NodePageHeader::readParentNodePageID --
//		「親ノードページの物理ページ識別子」を読み込む
//
//	NOTES
//	「親ノードページの物理ページ識別子」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		親ノードページの物理ページ識別子
//
//	EXCEPTIONS
//	なし

PhysicalFile::PageID
NodePageHeader::readParentNodePageID() const
{
	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	PhysicalFile::PageID v;
	(void) Os::Memory::copy(&v, p, sizeof(v));

	return v;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Btree::NodePageHeader::readPrevPhysicalPageID --
//		「前の物理ページの物理ページ識別子」を読み込む
//
//	NOTES
//	「前の物理ページの物理ページ識別子」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		前の物理ページの物理ページ識別子
//
//	EXCEPTIONS
//	なし

PhysicalFile::PageID
NodePageHeader::readPrevPhysicalPageID() const
{
	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	PhysicalFile::PageID v;
	(void) Os::Memory::copy(&v, p + sizeof(PhysicalFile::PageID), sizeof(v));

	return v;
}
#endif //OBSOLETE

//	FUNCTION public
//	Btree::NodePageHeader::readNextPhysicalPageID --
//		「次の物理ページの物理ページ識別子」を読み込む
//
//	NOTES
//	「次の物理ページの物理ページ識別子」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		次の物理ページの物理ページ識別子
//
//	EXCEPTIONS
//	なし

PhysicalFile::PageID
NodePageHeader::readNextPhysicalPageID() const
{
	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	PhysicalFile::PageID v;
	(void) Os::Memory::copy(
		&v, p + sizeof(PhysicalFile::PageID) * 2, sizeof(v));

	return v;
}

//	FUNCTION public
//	Btree::NodePageHeader::readKeyInformationNumber --
//		「物理ページ内のキー情報数」を読み込む
//
//	NOTES
//	「物理ページ内のキー情報数」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		物理ページ内のキー情報数
//
//	EXCEPTIONS
//	なし

ModUInt32
NodePageHeader::readKeyInformationNumber() const
{
	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	ModUInt32 v;
	(void) Os::Memory::copy(
		&v, p + sizeof(PhysicalFile::PageID) * 3, sizeof(v));

	return v;
}

//	FUNCTION public
//	Btree::NodePageHeader::readUseKeyInformationNumber --
//		「使用中の物理ページ内のキー情報数」を読み込む
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」を読み込む。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		使用中の物理ページ内のキー情報数
//
//	EXCEPTIONS
//	なし

ModUInt32
NodePageHeader::readUseKeyInformationNumber() const
{
	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	ModUInt32 v;
	(void) Os::Memory::copy(
		&v, p + sizeof(PhysicalFile::PageID) * 3 + sizeof(ModUInt32),
		sizeof(v));

	return v;
}

//	FUNCTION public
//	Btree::NodePageHeader::readPrevLeafPageID --
//		「前のリーフページの物理ページ識別子」を読み込む
//
//	NOTES
//	「前のリーフページの物理ページ識別子」を読み込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		前のリーフページの物理ページ識別子
//
//	EXCEPTIONS
//	なし

PhysicalFile::PageID
NodePageHeader::readPrevLeafPageID() const
{
	; _SYDNEY_ASSERT(m_bIsLeafPage);

	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	PhysicalFile::PageID v;
	(void) Os::Memory::copy(
		&v, p + sizeof(PhysicalFile::PageID) * 3 + sizeof(ModUInt32) * 2,
		sizeof(v));

	return v;
}

//	FUNCTION public
//	Btree::NodePageHeader::readNextLeafPageID --
//		「次のリーフページの物理ページ識別子」を読み込む
//
//	NOTES
//	「次のリーフページの物理ページ識別子」を読み込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		次のリーフページの物理ページ識別子
//
//	EXCEPTIONS
//	なし

PhysicalFile::PageID
NodePageHeader::readNextLeafPageID() const
{
	; _SYDNEY_ASSERT(m_bIsLeafPage);

	const char* p = getConstAreaTop();
	; _SYDNEY_ASSERT(p);

	PhysicalFile::PageID v;
	(void) Os::Memory::copy(
		&v, p + sizeof(PhysicalFile::PageID) * 3 +
		sizeof(ModUInt32) * 2 + sizeof(PhysicalFile::PageID),
		sizeof(v));

	return v;
}

#ifdef OBSOLETE
//
//	参照メソッド
//

//
//	FUNCTION public
//	Btree::NodePageHeader::assignParentNodePageID --
//		「親ノードページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「親ノードページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID*
//		「親ノードページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID*
NodePageHeader::assignParentNodePageID() const
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<PhysicalFile::PageID*>(this->m_AreaTop);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstParentNodePageID --
//		「親ノードページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「親ノードページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::PageID*
//		「親ノードページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::PageID*
NodePageHeader::assignConstParentNodePageID() const
{
	return
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop());
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignPrevPhysicalPageID --
//		「前の物理ページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「前の物理ページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID*
//		「前の物理ページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID*
NodePageHeader::assignPrevPhysicalPageID() const
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + NodePageHeader::m_lPrevPhysicalPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstPrevPhysicalPageID --
//		「前の物理ページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「前の物理ページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::PageID*
//		「前の物理ページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::PageID*
NodePageHeader::assignConstPrevPhysicalPageID() const
{
	return
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			NodePageHeader::m_lPrevPhysicalPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignNextPhysicalPageID --
//		「次の物理ページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「次の物理ページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID*
//		「次の物理ページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID*
NodePageHeader::assignNextPhysicalPageID() const
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + NodePageHeader::m_lNextPhysicalPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstNextPhysicalPageID --
//		「次の物理ページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「次の物理ページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::PageID*
//		「次の物理ページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::PageID*
NodePageHeader::assignConstNextPhysicalPageID() const
{
	return
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			NodePageHeader::m_lNextPhysicalPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignKeyInformationNumber --
//		「物理ページ内のキー情報数」へのポインタを返す
//
//	NOTES
//	「物理ページ内のキー情報数」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		「物理ページ内のキー情報数」へのポインタ
//
//	EXCEPTIONS
//	なし
//
ModUInt32*
NodePageHeader::assignKeyInformationNumber() const
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<ModUInt32*>(
			this->m_AreaTop +
			NodePageHeader::m_lKeyInformationNumberOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstKeyInformationNumber --
//		「物理ページ内のキー情報数」へのポインタを返す
//
//	NOTES
//	「物理ページ内のキー情報数」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUInt32*
//		「物理ページ内のキー情報数」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const ModUInt32*
NodePageHeader::assignConstKeyInformationNumber() const
{
	return
		syd_reinterpret_cast<const ModUInt32*>(
			this->getConstAreaTop() +
			NodePageHeader::m_lKeyInformationNumberOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignUseKeyInformationNumber --
//		「使用中の物理ページ内のキー情報数」へのポインタを返す
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		「使用中の物理ページ内のキー情報数」へのポインタ
//
//	EXCEPTIONS
//	なし
//
ModUInt32*
NodePageHeader::assignUseKeyInformationNumber() const
{
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<ModUInt32*>(
			this->m_AreaTop +
			NodePageHeader::m_lUseKeyInformationNumberOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstUseKeyInformationNumber --
//		「使用中の物理ページ内のキー情報数」へのポインタを返す
//
//	NOTES
//	「使用中の物理ページ内のキー情報数」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUInt32*
//		「使用中の物理ページ内のキー情報数」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const ModUInt32*
NodePageHeader::assignConstUseKeyInformationNumber() const
{
	return
		syd_reinterpret_cast<const ModUInt32*>(
			this->getConstAreaTop() +
			NodePageHeader::m_lUseKeyInformationNumberOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignPrevLeafPageID --
//		「前のリーフページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「前のリーフページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID*
//		「前のリーフページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID*
NodePageHeader::assignPrevLeafPageID() const
{
	; _SYDNEY_ASSERT(this->m_bIsLeafPage);
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + NodePageHeader::m_lPrevLeafPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstPrevLeafPageID --
//		「前のリーフページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「前のリーフページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::PageID*
//		「前のリーフページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::PageID*
NodePageHeader::assignConstPrevLeafPageID() const
{
	; _SYDNEY_ASSERT(this->m_bIsLeafPage);

	return
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			NodePageHeader::m_lPrevLeafPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignNextLeafPageID --
//		「次のリーフページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「次のリーフページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID*
//		「次のリーフページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
PhysicalFile::PageID*
NodePageHeader::assignNextLeafPageID() const
{
	; _SYDNEY_ASSERT(this->m_bIsLeafPage);
	; _SYDNEY_ASSERT(this->m_AreaTop != 0);

	return
		syd_reinterpret_cast<PhysicalFile::PageID*>(
			this->m_AreaTop + NodePageHeader::m_lNextLeafPageIDOffset);
}

//
//	FUNCTION public
//	Btree::NodePageHeader::assignConstNextLeafPageID --
//		「次のリーフページの物理ページ識別子」へのポインタを返す
//
//	NOTES
//	「次のリーフページの物理ページ識別子」へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const PhysicalFile::PageID*
//		「次のリーフページの物理ページ識別子」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const PhysicalFile::PageID*
NodePageHeader::assignConstNextLeafPageID() const
{
	; _SYDNEY_ASSERT(this->m_bIsLeafPage);

	return
		syd_reinterpret_cast<const PhysicalFile::PageID*>(
			this->getConstAreaTop() +
			NodePageHeader::m_lNextLeafPageIDOffset);
}
#endif	// OBSOLETE

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
