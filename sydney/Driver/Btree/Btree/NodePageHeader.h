// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NodePageHeader.h -- ノードページヘッダクラスのヘッダーファイル
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

#ifndef __SYDNEY_BTREE_NODEPAGEHEADER_H
#define __SYDNEY_BTREE_NODEPAGEHEADER_H

#include "Btree/AreaObject.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	Btree::NodePageHeader -- ノードページヘッダクラス
//
//	NOTES
//	リーフページを含むノードページヘッダクラス。
//	※ ただし、下図のようにリーフページとそれ以外の
//	   ノードページのヘッダは物理構成が異なる。
//
//		リーフページ以外のノードページのヘッダ
//		┌───────────────────────┐
//		│　親ノードページの物理ページ識別子 (32bit)　　│
//		├───────────────────────┤
//		│　前の物理ページの物理ページ識別子 (32bit)　　│
//		├───────────────────────┤
//		│　次の物理ページの物理ページ識別子 (32bit)　　│
//		├───────────────────────┤
//		│　　　物理ページ内のキー情報数 (32bit)　　　　│
//		├───────────────────────┤
//		│　使用中の物理ページ内のキー情報数 (32bit)　　│
//		└───────────────────────┘
//
//		リーフページのヘッダ
//		┌───────────────────────┐
//		│　親ノードページの物理ページ識別子 (32bit)　　│
//		├───────────────────────┤
//		│　前の物理ページの物理ページ識別子 (32bit)　　│
//		├───────────────────────┤
//		│　次の物理ページの物理ページ識別子 (32bit)　　│
//		├───────────────────────┤
//		│　　　物理ページ内のキー情報数 (32bit)　　　　│
//		├───────────────────────┤
//		│　使用中の物理ページ内のキー情報数 (32bit)　　│
//		├───────────────────────┤
//		│　前のリーフページの物理ページ識別子 (32bit)　│
//		├───────────────────────┤
//		│　次のリーフページの物理ページ識別子 (32bit)　│
//		└───────────────────────┘
//
class NodePageHeader : public Btree::AreaObject
{
public:

	// コンストラクタ
	NodePageHeader(
		const Trans::Transaction*			pTransaction_,
		PhysicalFile::File*					pFile_,
		const PhysicalFile::PageID			uiNodePageID_,
		const Buffer::Page::FixMode::Value	eFixMode_,
		const bool							bIsLeafPage_,
		const bool							SavePage_);

	// コンストラクタ
	NodePageHeader(
		const Trans::Transaction*	pTransaction_,
		PhysicalFile::Page*			pPage_,
		const bool					bIsLeafPage_);

	// デストラクタ
	~NodePageHeader();

	// ノードページヘッダのアーカイブサイズを返す
	static Os::Memory::Size getArchiveSize(const bool	bIsLeafPage_);

	// 物理ページをアタッチし直す
	void resetPhysicalPage(PhysicalFile::Page*	NewPhysicalPage_);

	//
	// 書き込みメソッド
	//

	//
	// リーフページを含むノードページのヘッダへの書き込みメソッド
	//

	// ノード／リーフページヘッダを書き込む
	void write(const PhysicalFile::PageID	ParentNodePageID_,
			   const PhysicalFile::PageID	PrevPhysialPageID_,
			   const PhysicalFile::PageID	NextPhysicalPageID_,
			   const ModUInt32				KeyInformationNumber_,
			   const ModUInt32				UseKeyInformationNumber_,
			   const PhysicalFile::PageID	PrevLeafPageID_ =
								PhysicalFile::ConstValue::UndefinedPageID,
			   const PhysicalFile::PageID	NextLeafPageID_ =
								PhysicalFile::ConstValue::UndefinedPageID);

	// 「親ノードページの物理ページ識別子」を書き込む
	void
		writeParentNodePageID(
			const PhysicalFile::PageID	uiParentNodePageID_);

	// 「前の物理ページの物理ページ識別子」を書き込む
	void
		writePrevPhysicalPageID(
			const PhysicalFile::PageID	uiPrevPhysicalPageID_);

	// 「次の物理ページの物理ページ識別子」を書き込む
	void
		writeNextPhysicalPageID(
			const PhysicalFile::PageID	uiNextPhysicalPageID_);

	// 「物理ページ内のキー情報数」を書き込む
	void
		writeKeyInformationNumber(
			const ModUInt32	ulKeyInformationNumber_);

	// 「使用中の物理ページ内のキー情報数」を書き込む
	void
		writeUseKeyInformationNumber(
			const ModUInt32	ulUseKeyInformationNumber_);

	// 「使用中の物理ページ内のキー情報数」をインクリメントする
	void incUseKeyInformationNumber();

	// 「使用中の物理ページ内のキー情報数」をデクリメントする
	void decUseKeyInformationNumber();

	//
	// リーフページヘッダへの書き込みメソッド
	//

	// 「前のリーフページの物理ページ識別子」を書き込む
	void
		writePrevLeafPageID(
			const PhysicalFile::PageID	uiPrevLeafPageID_);

	// 「次のリーフページの物理ページ識別子」を書き込む
	void
		writeNextLeafPageID(
			const PhysicalFile::PageID	uiNextLeafPageID_);

	//
	// 読み込みメソッド
	//

	//
	// リーフページを含むノードページのヘッダからの読み込みメソッド
	//

	// 「親ノードページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readParentNodePageID() const;

#ifdef OBSOLETE
	// 「前の物理ページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readPrevPhysicalPageID() const;
#endif //OBSOLETE

	// 「次の物理ページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readNextPhysicalPageID() const;

	// 「物理ページ内のキー情報数」を読み込む
	ModUInt32 readKeyInformationNumber() const;

	// 「使用中の物理ページ内のキー情報数」を読み込む
	ModUInt32 readUseKeyInformationNumber() const;

	//
	// リーフページヘッダからの読み込みメソッド
	//

	// 「前のリーフページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readPrevLeafPageID() const;

	// 「次のリーフページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readNextLeafPageID() const;

#ifdef OBSOLETE
	//
	// 参照メソッド
	//

	//
	// リーフページを含むノードページのヘッダへの参照メソッド
	//

	// 「親ノードページの物理ページ識別子」へのポインタを返す
	PhysicalFile::PageID* assignParentNodePageID() const;
	// 
	const PhysicalFile::PageID* assignConstParentNodePageID() const;
	// 「前の物理ページの物理ページ識別子」へのポインタを返す
	PhysicalFile::PageID* assignPrevPhysicalPageID() const;
	// 
	const PhysicalFile::PageID* assignConstPrevPhysicalPageID() const;
	// 「次の物理ページの物理ページ識別子」へのポインタを返す
	PhysicalFile::PageID* assignNextPhysicalPageID() const;
	// 
	const PhysicalFile::PageID* assignConstNextPhysicalPageID() const;
	// 「物理ページ内のキー情報数」へのポインタを返す
	ModUInt32* assignKeyInformationNumber() const;
	// 
	const ModUInt32* assignConstKeyInformationNumber() const;
	// 「使用中の物理ページ内のキー情報数」へのポインタを返す
	ModUInt32* assignUseKeyInformationNumber() const;
	// 
	const ModUInt32* assignConstUseKeyInformationNumber() const;

	//
	// リーフページヘッダへの参照メソッド
	//

	// 「前のリーフページの物理ページ識別子」へのポインタを返す
	PhysicalFile::PageID* assignPrevLeafPageID() const;
	// 
	const PhysicalFile::PageID* assignConstPrevLeafPageID() const;
	// 「次のリーフページの物理ページ識別子」へのポインタを返す
	PhysicalFile::PageID* assignNextLeafPageID() const;
	// 
	const PhysicalFile::PageID* assignConstNextLeafPageID() const;
#endif	//OBSOLETE

	// ノードページヘッダエリアの物理エリア識別子
	static const PhysicalFile::AreaID	AreaID;

private:

	//
	// データメンバ
	//
#ifdef OBSOLETE
	// 「親ノードページの物理ページ識別子」の
	// ノードページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lParentNodePageIDOffset;

	// 「前の物理ページの物理ページ識別子」の
	// ノードページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lPrevPhysicalPageIDOffset;

	// 「次の物理ページの物理ページ識別子」の
	// ノードページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lNextPhysicalPageIDOffset;

	// 「物理ページ内のキー情報数」の
	// ノードページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lKeyInformationNumberOffset;

	// 「使用中の物理ページ内のキー情報数」の
	// ノードページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lUseKeyInformationNumberOffset;

	// 「前のリーフページの物理ページ識別子」の
	// リーフページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lPrevLeafPageIDOffset;

	// 「次のリーフページの物理ページ識別子」の
	// リーフページヘッダエリア内での位置 [byte]
	static const Os::Memory::Offset		m_lNextLeafPageIDOffset;

	// リーフページ以外のノードページのヘッダのアーカイブサイズ [byte]
	static const Os::Memory::Size		m_ulNodePageHeaderArchiveSize;

	// リーフページヘッダのアーカイブサイズ [byte]
	static const Os::Memory::Size		m_ulLeafPageHeaderArchiveSize;
#endif
	// リーフページかどうか
	bool								m_bIsLeafPage;

}; // end of class NodePageHeader


_SYDNEY_BTREE_END
_SYDNEY_END

#endif // __SYDNEY_BTREE_NODEPAGEHEADER_H

//
//	Copyright (c) 2000, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
