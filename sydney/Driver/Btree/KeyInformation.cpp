// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KeyInformation.cpp --
//		ノード／リーフページ内のキー情報クラスの実現ファイル
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#include "Common/Assert.h"

#include "FileCommon/ObjectID.h"

#include "Btree/NodePageHeader.h"
#include "Btree/KeyInformation.h"
#include "Btree/File.h"

_SYDNEY_USING
_SYDNEY_BTREE_USING

//////////////////////////////////////////////////
//
//	PUBLIC CONST
//
//////////////////////////////////////////////////

//
//	CONST public
//	Btree::KeyInformation::KeyTableAreaID --
//		キーテーブルエリアの物理エリア識別子
//
//	NOTES
//	キーテーブルエリアの物理エリア識別子。
//
// static
const PhysicalFile::AreaID
KeyInformation::KeyTableAreaID = NodePageHeader::AreaID + 1;

//////////////////////////////////////////////////
//
//	PRIVATE CONST
//
//////////////////////////////////////////////////

//
//	CONST private
//	Btree::KeyInformation::ChildNodePageIDOffset --
//		リーフページ以外のノードページのキー情報内での
//		「子ノードページの物理ページ識別子」の位置
//
//	NOTES
//	各キー情報の開始位置を基点とした場合の
//	「子ノードページの物理ページ識別子」が記録されている位置。[byte]
//	「子ノードページの物理ページ識別子」は、
//	リーフページ以外のノードページのキー情報に記録される。
//	また、キー値記録先がキー情報、キーオブジェクトいずれの場合も
//	キー情報に記録される。
//
// static
const Os::Memory::Offset
KeyInformation::ChildNodePageIDOffset = 0;

//
//	CONST private
//	Btree::KeyInformation::NodeKeyObjectIDOffset --
//		リーフページ以外のノードページのキー情報内での
//		「キーオブジェクトのオブジェクトID」の位置
//
//	NOTES
//	各キー情報の開始位置を基点とした場合の
//	「キーオブジェクトのオブジェクトID」が記録されている位置。[byte]
//	「キーオブジェクトのオブジェクトID」は、
//	キー値をキーオブジェクトに記録するタイプのファイルの
//	キー情報に記録される。
//	この場合、リーフページ、リーフページ以外のノードページ
//	いずれの場合もキー情報に記録されるが、
//	リーフページとリーフページ以外のノードページでは、
//	記録位置が異なる。
//
// static
const Os::Memory::Offset
KeyInformation::NodeKeyObjectIDOffset =
	KeyInformation::ChildNodePageIDOffset +
	File::PageIDArchiveSize;

//
//	CONST private
//	Btree::KeyInformation::ValueObjectIDOffset --
//		リーフページのキー情報内での
//		「バリューオブジェクトのオブジェクトID」の位置
//
//	NOTES
//	各キー情報の開始位置を基点とした場合の
//	「バリューオブジェクトのオブジェクトID」が記録されている位置。[byte]
//	「バリューオブジェクトのオブジェクトID」は、
//	リーフページのキー情報に記録される。
//	また、キー値記録先がキー情報、キーオブジェクトの場合も
//	キー情報に記録される。
//
// static
const Os::Memory::Offset
KeyInformation::ValueObjectIDOffset = 0;

//
//	CONST private
//	Btree::KeyInformation::LeafKeyObjectIDOffset --
//		リーフページのキー情報内での
//		「キーオブジェクトのオブジェクトID」の位置
//
//	NOTES
//	各キー情報の開始位置を基点とした場合の
//	「キーオブジェクトのオブジェクトID」が記録されている位置。[byte]
//	「キーオブジェクトのオブジェクトID」は、
//	キー値をキーオブジェクトに記録するタイプのファイルの
//	キー情報に記録される。
//	この場合、リーフページ、リーフページ以外のノードページ
//	いずれの場合もキー情報に記録されるが、
//	リーフページとリーフページ以外のノードページでは、
//	記録位置が異なる。
//
//	static
const Os::Memory::Offset
KeyInformation::LeafKeyObjectIDOffset =
	KeyInformation::ValueObjectIDOffset +
	File::ObjectIDArchiveSize;

//
//	CONST private
//	Btree::KeyInformation::SizeAtNode --
//		キー値をキーオブジェクトに記録するタイプのファイルの
//		リーフページ以外のノードページ内のキー情報のアーカイブサイズ
//
//	NOTES
//	キー値をキーオブジェクトに記録するタイプのファイルの
//	リーフページ以外のノードページ内のキー情報のアーカイブサイズ。[byte]
//
// static
const Os::Memory::Size
KeyInformation::SizeAtNode =
	File::PageIDArchiveSize + File::ObjectIDArchiveSize;

//
//	CONST private
//	Btree::KeyInformation::SizeAtLeaf --
//		キー値をキーオブジェクトに記録するタイプのファイルの
//		リーフページ内のキー情報のアーカイブサイズ
//
//	NOTES
//	キー値をキーオブジェクトに記録するタイプのファイルの
//	リーフページ内のキー情報のアーカイブサイズ。[byte]
//
// static
const Os::Memory::Size
KeyInformation::SizeAtLeaf = File::ObjectIDArchiveSize << 1;
//                                                     ~~~~ 「掛ける２」

//////////////////////////////////////////////////
//
//	PUBLIC METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION public
//	Btree::KeyInformation::KeyInformation -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	親クラスであるBtree::AreaObjectのコンストラクタ内で
//	物理ページをアタッチする。
//
//	ARGUMENTS
//	const Trans::Transaction*			Transaction_
//		トランザクション記述子
//	PhysicalFile::File*					File_
//		ツリーファイルの物理ファイル記述子
//	const PhysicalFile::PageID			NodePageID_
//		ノード／リーフページの物理ページ識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	const ModUInt32						KeyInfoIndex_
//		キーテーブル内でのキー情報インデックス
//	const bool							IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページではないノードページ
//	const bool							SavePage_
//		物理ファイルマネージャでアタッチした物理ページを
//		キャッシュしておくかどうか
//			true  : キャッシュしておく
//			false : キャッシュせず、デタッチ時にアンフィックスする
//	const int							KeyNum_ = 0
//		キーフィールド数
//		（キー値をキー情報に記録する場合に指定する。）
//	const Os::Memory::Size				KeySize_ = 0
//		キー値の記録サイズ
//		（キー値をキー情報に記録する場合に指定する。）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
KeyInformation::KeyInformation(
	const Trans::Transaction*			Transaction_,
	PhysicalFile::File*					File_,
	const PhysicalFile::PageID			NodePageID_,
	const Buffer::Page::FixMode::Value	FixMode_,
	const ModUInt32						KeyInfoIndex_,
	const bool							IsLeafPage_,
	const bool							SavePage_,
	const int							KeyNum_, // = 0
	const Os::Memory::Size				KeySize_ // = 0
	)
	: Btree::AreaObject(Transaction_,
						File_,
						NodePageID_,
						KeyInformation::KeyTableAreaID,
						FixMode_,
						Buffer::ReplacementPriority::Low,
						SavePage_),
	  m_IsLeafPage(IsLeafPage_),
	  m_CharPointer(0),
	  m_ConstCharPointer(0),
	  m_KeyNum(KeyNum_),
	  m_KeySize(KeySize_)
{
	this->setKeyPosType(KeySize_);

	this->m_Size = KeyInformation::getSize(this->m_IsLeafPage,
										   this->m_KeyPosType,
										   this->m_KeyNum);

	this->setStartOffsetByIndex(KeyInfoIndex_);
}

//
//	FUNCTION public
//	Btree::KeyInformation::KeyInformation -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	親クラスであるBtree::AreaObjectのコンストラクタ内で
//	物理ページをアタッチしない。
//
//	ARGUMENTS
//	const Trans::Transaction*	Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*			NodePage_
//		キー情報が記録されている物理ページの記述子
//	const ModUInt32				KeyInfoIndex_
//		キーテーブル内でのキー情報インデックス
//	const bool					IsLeafPage_
//		リーフページかどうか
//			true  : リーフページ
//			false : リーフページではないノードページ
//	const int					KeyNum_ = 0
//		キーフィールド数
//		（キー値をキー情報に記録する場合に指定する。）
//	const Os::Memory::Size		KeySize_ = 0
//		キー値の記録サイズ
//		（キー値をキー情報に記録する場合に指定する。）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
KeyInformation::KeyInformation(
	const Trans::Transaction*	Transaction_,
	PhysicalFile::Page*			NodePage_,
	const ModUInt32				KeyInfoIndex_,
	const bool					IsLeafPage_,
	const int					KeyNum_, // = 0
	const Os::Memory::Size		KeySize_ // = 0
	)
	: Btree::AreaObject(Transaction_,
						NodePage_,
						KeyInformation::KeyTableAreaID),
	  m_IsLeafPage(IsLeafPage_),
	  m_CharPointer(0),
	  m_ConstCharPointer(0),
	  m_KeyNum(KeyNum_),
	  m_KeySize(KeySize_)
{
	this->setKeyPosType(KeySize_);

	this->m_Size = KeyInformation::getSize(this->m_IsLeafPage,
										   this->m_KeyPosType,
										   this->m_KeyNum);

	this->setStartOffsetByIndex(KeyInfoIndex_);
}

//
//	FUNCTION public
//	Btree::KeyInformation::~KeyInformation -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
KeyInformation::~KeyInformation()
{
}

//
//	FUNCTION public
//	Btree::KeyInformation::getSize --
//		キー情報の書き込みサイズを返す
//
//	NOTES
//	キー情報の書き込みサイズを返す。
//	※ リーフページとそれ以外のノードページとでは、
//	   キー情報に記録されている内容が異なる。
//
//	ARGUMENTS
//	const bool						IsLeafPage_
//		リーフページ内記録されているキー情報の
//		書き込みサイズを返すかどうか
//			true  : リーフページの〜
//			false : リーフページではないノードページの〜
//	const Btree::KeyPosType::Value	KeyPosType_ = KeyObject
//		キー値記録先
//	const int						KeyNum_ = 0
//		キーフィールド数
//
//	RETURN
//	Os::Memory::Size
//		キー情報の書き込みサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
// static
Os::Memory::Size
KeyInformation::getSize(const bool				IsLeafPage_,
						const KeyPosType::Value	KeyPosType_, // = KeyObject
						const int				KeyNum_      // = 0
						)
{
	if (KeyPosType_ == KeyPosType::KeyObject)
	{
		// キー値をキーオブジェクトに記録するタイプのファイル…

		return
			IsLeafPage_ ?
				KeyInformation::SizeAtLeaf : KeyInformation::SizeAtNode;
	}
	else
	{
		// キー値をキー情報内に記録するタイプのファイル…

		return
			(IsLeafPage_ ?
				File::ObjectIDArchiveSize :
				File::PageIDArchiveSize) +
			NullBitmap::getSize(KeyNum_);
	}
}

//
//	FUNCTION public
//	Btree::KeyInformation::resetPhysicalPage --
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
KeyInformation::resetPhysicalPage(PhysicalFile::Page*	NewPhysicalPage_)
{
	AreaObject::resetPhysicalPage(NewPhysicalPage_);

	// とりあえず、先頭キー情報を参照できるように設定しておく
	this->setStartOffsetByIndex(0);
}

//
//	FUNCTION public
//	Btree::KeyInformation::setStartOffsetByIndex --
//		キーテーブルエリア内のオフセットを設定する
//
//	NOTES
//	キーテーブルエリア内のオフセットを設定する。
//
//	ARGUMENTS
//	const ModUInt32	KeyInfoIndex_
//		キーテーブル内でのキー情報インデックス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
KeyInformation::setStartOffsetByIndex(const ModUInt32	KeyInfoIndex_)
{
	this->m_Index = KeyInfoIndex_;

	Os::Memory::Size	keyInfoSize = this->m_Size + this->m_KeySize;

	Os::Memory::Offset	startOffset = keyInfoSize * KeyInfoIndex_;

	if (this->m_eFixMode == Buffer::Page::FixMode::ReadOnly)
	{
		this->m_CharPointer = 0;

		this->m_ConstCharPointer = this->getConstAreaTop() + startOffset;
	}
	else
	{
		; _SYDNEY_ASSERT(this->m_AreaTop != 0);

		this->m_CharPointer = this->m_AreaTop + startOffset;

		this->m_ConstCharPointer = this->m_CharPointer;
	}
}

//
//	FUNCTION public
//	Btree::KeyInformation::next --
//		キーテーブル内の次のキー情報へ移動する
//
//	NOTES
//	キーテーブル内の次のキー情報へ移動する。
//	キー情報がキーテーブル内で最後のキー情報の場合には、
//	移動できない。
//
//	ARGUMENTS
//	ModUInt32	LastIndex_
//		キーテーブル内の最後のキー情報のインデックス
//
//	RETURN
//	bool
//		次のキー情報へ移動できたかどうか
//			true  : 移動できた
//			false : 移動できなかった
//
//	EXCEPTIONS
//	なし
//
bool
KeyInformation::next(ModUInt32	LastIndex_)
{
	if (this->m_Index == LastIndex_)
	{
		return false;
	}

	this->setStartOffsetByIndex(this->m_Index + 1);

	return true;
}

//
//	FUNCTION public
//	Btree::KeyInformation::prev --
//		キーテーブル内の前のキー情報へ移動する
//
//	NOTES
//	キーテーブル内の前のキー情報へ移動する。
//	キー情報がキーテーブル内で先頭のキー情報の場合には、
//	移動できない。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		前のキー情報へ移動できたかどうか
//			true  : 移動できた
//			false : 移動できなかった
//
//	EXCEPTIONS
//	なし
//
bool
KeyInformation::prev()
{
	if (this->m_Index == 0)
	{
		return false;
	}

	this->setStartOffsetByIndex(this->m_Index - 1);

	return true;
}

//
//	書き込みメソッド
//

//	FUNCTION public
//	Btree::KeyInformation::writeKeyObjectID --
//		「キーオブジェクトのオブジェクトID」を書き込む
//
//	NOTES
//	「キーオブジェクトのオブジェクトID」を書き込む。
//	※ キー値をキーオブジェクトに記録するタイプのファイルのみ
//	　 呼び出し可能。
//
//	ARGUMENTS
//	const ModUInt64	KeyObjectID_
//		キーオブジェクトのオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
KeyInformation::writeKeyObjectID(const ModUInt64 KeyObjectID_)
{
	; _SYDNEY_ASSERT(m_KeyPosType == KeyPosType::KeyObject);
	; _SYDNEY_ASSERT(m_CharPointer);

	const Os::Memory::Offset offset =
		(m_IsLeafPage) ? LeafKeyObjectIDOffset : NodeKeyObjectIDOffset;

	(void) File::writeObjectID(m_CharPointer + offset, KeyObjectID_);
}

#ifdef OBSOLETE // 将来に対する予約
//	FUNCTION public
//	Btree::KeyInformation::write --
//		リーフページ以外のノードページのキー情報を書き込む
//
//	NOTES
//	リーフページ以外のノードページのキー情報を書き込む。
//	※ リーフページ以外のノードページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ChildNodePageID_
//		子ノードページの物理ページ識別子
//	const ModUInt64				KeyObjectID_
//		キーオブジェクトのオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
KeyInformation::write(const PhysicalFile::PageID	ChildNodePageID_,
					  const ModUInt64				KeyObjectID_)
{
	; _SYDNEY_ASSERT(!m_IsLeafPage);
	; _SYDNEY_ASSERT(m_CharPointer);

	char* p = m_CharPointer;
	Os::Memory::copy(p, &ChildNodePageID_, sizeof(ChildNodePageID_));
	p += sizeof(ChildNodePageID_);

	(void) File::writeObjectID(p, KeyObjectID_);
}
#endif

//	FUNCTION public
//	Btree::KeyInformation::writeChildNodePageID --
//		「子ノードページの物理ページ識別子」を書き込む
//
//	NOTES
//	「子ノードページの物理ページ識別子」を書き込む。
//	※ リーフページ以外のノードページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	ChildNodePageID_
//		子ノードページの物理ページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
KeyInformation::writeChildNodePageID(
	const PhysicalFile::PageID	ChildNodePageID_)
{
	; _SYDNEY_ASSERT(!m_IsLeafPage);
	; _SYDNEY_ASSERT(m_CharPointer);

	(void) Os::Memory::copy(
		m_CharPointer, &ChildNodePageID_, sizeof(ChildNodePageID_));
}

#ifdef OBSOLETE // 将来に対する予約
//	FUNCTION public
//	Btree::KeyInformation::write --
//		リーフページのキー情報を書き込む
//
//	NOTES
//	リーフページのキー情報を書き込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	const ModUInt64	ValueObjectID_
//		バリューオブジェクトのオブジェクトID
//	const ModUInt64	KeyObjectID_
//		キーオブジェクトのオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
KeyInformation::write(
	const ModUInt64 ValueObjectID_, const ModUInt64 KeyObjectID_)
{
	; _SYDNEY_ASSERT(m_IsLeafPage);
	; _SYDNEY_ASSERT(m_CharPointer);

	(void) File::writeObjectID(
		File::writeObjectID(m_CharPointer, ValueObjectID_), KeyObjectID_);
}
#endif

//	FUNCTION public
//	Btree::KeyInformation::writeValueObjectID --
//		「バリューオブジェクトのオブジェクトID」を書き込む
//
//	NOTES
//	「バリューオブジェクトのオブジェクトID」を書き込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	const ModUInt64	ValueObjectID_
//		バリューオブジェクトのオブジェクトID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし

void
KeyInformation::writeValueObjectID(const ModUInt64 ValueObjectID_)
 {
	; _SYDNEY_ASSERT(m_IsLeafPage);
	; _SYDNEY_ASSERT(m_CharPointer);

	(void) File::writeObjectID(m_CharPointer, ValueObjectID_);
}

//
//	FUNCTION public
//	Btree::KeyInformation::copy -- キー情報をコピーする
//
//	NOTES
//	キー情報をコピーする。
//
//	ARGUMENTS
//	const Btree::KeyInformation&	SrcKeyInfo_
//		コピー元キー情報への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
KeyInformation::copy(const KeyInformation&	SrcKeyInfo_)
{
	if (this->m_IsLeafPage)
	{
		// リーフページ…

		this->writeValueObjectID(SrcKeyInfo_.readValueObjectID());
	}
	else
	{
		// リーフページ以外のノードページ…

		this->writeChildNodePageID(SrcKeyInfo_.readChildNodePageID());
	}

	if (this->m_KeyPosType == KeyPosType::KeyObject)
	{
		// キー値をキーオブジェクトに記録するタイプのファイル…

		this->writeKeyObjectID(SrcKeyInfo_.readKeyObjectID());
	}
	else
	{
		// キー値をキー情報に記録するタイプのファイル…

		; _SYDNEY_ASSERT(this->m_KeyNum > 0);
		; _SYDNEY_ASSERT(this->m_KeyNum == SrcKeyInfo_.m_KeyNum);

		; _SYDNEY_ASSERT(this->m_KeySize > 0);
		; _SYDNEY_ASSERT(this->m_KeySize == SrcKeyInfo_.m_KeySize);

		const NullBitmap::Value*	srcNullBitmapTop =
			SrcKeyInfo_.assignConstKeyNullBitmap();

		; _SYDNEY_ASSERT(srcNullBitmapTop != 0);

		NullBitmap::Value*	dstNullBitmapTop =
			this->assignKeyNullBitmap();

		; _SYDNEY_ASSERT(dstNullBitmapTop != 0);

		//
		// ヌルビットマップとキー値を記録している領域を
		// コピーする
		//

		ModSize	copySize =
			NullBitmap::getSize(this->m_KeyNum) + this->m_KeySize;

		ModOsDriver::Memory::copy(dstNullBitmapTop,
								  srcNullBitmapTop,
 								  copySize);
	}
}

//
//	読み込みメソッド
//

//	FUNCTION public
//	Btree::KeyInformation::readChildNodePageID --
//		「子ノードページの物理ページ識別子」を読み込む
//
//	NOTES
//	「子ノードページの物理ページ識別子」を読み込む。
//	※ リーフページ以外のノードページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageID
//		子ノードページの物理ページ識別子
//
//	EXCEPTIONS
//	なし

PhysicalFile::PageID
KeyInformation::readChildNodePageID() const
{
	; _SYDNEY_ASSERT(!m_IsLeafPage);
	; _SYDNEY_ASSERT(m_ConstCharPointer);

	PhysicalFile::PageID pageID;
	(void) Os::Memory::copy(&pageID, m_ConstCharPointer, sizeof(pageID));

	return pageID;
}

//	FUNCTION public
//	Btree::KeyInformation::readKeyObjectID --
//		「キーオブジェクトのオブジェクトID」を読み込む
//
//	NOTES
//	「キーオブジェクトのオブジェクトID」を読み込む。
//	※ キー値をキーオブジェクトに記録するタイプのファイルのみ
//	　 呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		キーオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	なし

ModUInt64
KeyInformation::readKeyObjectID() const
{
	; _SYDNEY_ASSERT(m_KeyPosType == KeyPosType::KeyObject);
	; _SYDNEY_ASSERT(m_ConstCharPointer);

	const Os::Memory::Offset offset =
		(m_IsLeafPage) ? LeafKeyObjectIDOffset : NodeKeyObjectIDOffset;

	ModUInt64 objectID;
	(void) File::readObjectID(m_ConstCharPointer + offset, objectID);

	return objectID;
}

//	FUNCTION public
//	Btree::KeyInformation::readValueObjectID --
//		「バリューオブジェクトのオブジェクトID」を読み込む
//
//	NOTES
//	「バリューオブジェクトのオブジェクトID」を読み込む。
//	※ リーフページに対してのみ呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt64
//		バリューオブジェクトのオブジェクトID
//
//	EXCEPTIONS
//	なし

ModUInt64
KeyInformation::readValueObjectID() const
{
	; _SYDNEY_ASSERT(m_IsLeafPage);
	; _SYDNEY_ASSERT(m_ConstCharPointer);

	ModUInt64 objectID;
	(void) File::readObjectID(m_ConstCharPointer, objectID);

	return objectID;
}

//
//	FUNCTION public
//	Btree::KeyInformation::assignKeyNullBitmap --
//		「キー値ヌルビットマップ」へのポインタを返す
//
//	NOTES
//	「キー値ヌルビットマップ」へのポインタを返す。
//	※ キー値をキー情報に記録するタイプのファイルのみ呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Btree::NullBitmap::Value*
//		「キー値ヌルビットマップ」へのポインタ
//
//	EXCEPTIONS
//	なし
//
NullBitmap::Value*
KeyInformation::assignKeyNullBitmap() const
{
	; _SYDNEY_ASSERT(this->m_KeyPosType == KeyPosType::KeyInfo);
	; _SYDNEY_ASSERT(this->m_KeySize > 0);
	; _SYDNEY_ASSERT(this->m_CharPointer != 0);

	Os::Memory::Offset	offset =
		this->m_IsLeafPage ?
			File::ObjectIDArchiveSize : File::PageIDArchiveSize;

	return
		syd_reinterpret_cast<NullBitmap::Value*>(this->m_CharPointer + offset);
}

//
//	FUNCTION public
//	Btree::KeyInformation::assignConstKeyNullBitmap --
//		「キー値ヌルビットマップ」へのポインタを返す
//
//	NOTES
//	「キー値ヌルビットマップ」へのポインタを返す。
//	※ キー値をキー情報に記録するタイプのファイルのみ呼び出し可能。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Btree::NullBitmap::Value*
//		「キー値ヌルビットマップ」へのポインタ
//
//	EXCEPTIONS
//	なし
//
const NullBitmap::Value*
KeyInformation::assignConstKeyNullBitmap() const
{
	; _SYDNEY_ASSERT(this->m_KeyPosType == KeyPosType::KeyInfo);
	; _SYDNEY_ASSERT(this->m_KeySize > 0);
	; _SYDNEY_ASSERT(this->m_ConstCharPointer != 0);

	Os::Memory::Offset	offset =
		this->m_IsLeafPage ?
			File::ObjectIDArchiveSize : File::PageIDArchiveSize;

	return
		syd_reinterpret_cast<const NullBitmap::Value*>(
			this->m_ConstCharPointer + offset);
}

//
//	FUNCTION public
//	Btree::KeyInformation::assignKeyTop --
//		キー値先頭へのポインタを返す
//
//	NOTES
//	キー値先頭へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	void*
//		キー値先頭へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
void*
KeyInformation::assignKeyTop() const
{
	NullBitmap::Value*	nullBitmapTop = this->assignKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	return NullBitmap::getTail(nullBitmapTop,
							   this->m_KeyNum);
}

//
//	FUNCTION public
//	Btree::KeyInformation::assignConstKeyTop --
//		キー値先頭へのポインタを返す
//
//	NOTES
//	キー値先頭へのポインタを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const void*
//		キー値先頭へのポインタ
//
//	EXCEPTIONS
//	[YET!]
//
const void*
KeyInformation::assignConstKeyTop() const
{
	const NullBitmap::Value*	nullBitmapTop =
		this->assignConstKeyNullBitmap();

	; _SYDNEY_ASSERT(nullBitmapTop != 0);

	return NullBitmap::getConstTail(nullBitmapTop,
									this->m_KeyNum);
}

//////////////////////////////////////////////////
//
//	PRIVATE METHOD
//
//////////////////////////////////////////////////

//
//	FUNCTION private
//	Btree::KeyInformation::setKeyPosType -- キー値記録先を設定する
//
//	NOTES
//	キー値記録先を設定する。
//
//	ARGUMENTS
//	const Os::Memory::Size	KeySize_
//		キー値の記録サイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	
void
KeyInformation::setKeyPosType(const Os::Memory::Size	KeySize_)
{
	if (this->m_KeySize == 0)
	{
		// キー値の記録サイズが指定されなかった…

		// ということは、キー値はキーオブジェクトに記録する。

		this->m_KeyPosType = KeyPosType::KeyObject;
	}
	else
	{
		// キー値の記録サイズが指定された…

		// ということは、キー値はキー情報に記録する。

		this->m_KeyPosType = KeyPosType::KeyInfo;
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
