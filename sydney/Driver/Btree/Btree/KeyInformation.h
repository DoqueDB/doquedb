// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KeyInformation.h --
//		ノード／リーフページ内のキー情報クラスのヘッダーファイル
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

#ifndef __SYDNEY_BTREE_KEYINFORMATION_H
#define __SYDNEY_BTREE_KEYINFORMATION_H

#include "Btree/Module.h"
#include "Btree/AreaObject.h"
#include "Btree/KeyPosType.h"
#include "Btree/NullBitmap.h"

_SYDNEY_BEGIN
_SYDNEY_BTREE_BEGIN

//
//	CLASS
//	ノード／リーフページ内のキー情報クラス
//
//	NOTES
//	ノード／リーフページ内のキー情報クラス。
//	キー情報に書き込まれるデータはリーフページとそれ以外の
//	ノードページとでは異なる。
//	キー情報の列のことをキーテーブルと呼ぶ。
//
//	<<<<< キー値をキーオブジェクトに記録するタイプのファイル >>>>>
//
//	リーフページ以外のノードページに記録されているキー情報は
//	下図のような物理構成である。
//
//		┌────────────────────────┐
//		│　　子ノードページの物理ページ識別子 (32bit)　　│
//		├────────────────────────┤
//		│　　キーオブジェクトのオブジェクトID (48bit)　　│
//		└────────────────────────┘
//
//	リーフページに記録されているキー情報は下図のような物理構成である。
//
//		┌────────────────────────┐
//		│　バリューオブジェクトのオブジェクトID (48bit)　│
//		├────────────────────────┤
//		│　　キーオブジェクトのオブジェクトID (48bit)　　│
//		└────────────────────────┘
//
//	<<<<< キー値をキー情報内に記録するタイプのファイル >>>>>
//
//	リーフページ以外のノードページに記録されているキー情報は
//	下図のような物理構成である。
//
//		┌────────────────────────┐
//		│　　子ノードページの物理ページ識別子 (32bit)　　│
//		├────────────────────────┤
//		│　　　　キー値ヌルビットマップ  (8bit)　　　　　│
//		├────────────────────────┤
//		│　　　　　　　キー値  (12byte以下)　　　　　　　│
//		└────────────────────────┘
//
//	リーフページに記録されているキー情報は下図のような物理構成である。
//
//		┌────────────────────────┐
//		│　バリューオブジェクトのオブジェクトID (48bit)　│
//		├────────────────────────┤
//		│　　　　キー値ヌルビットマップ  (8bit)　　　　　│
//		├────────────────────────┤
//		│　　　　　　　キー値  (12byte以下)　　　　　　　│
//		└────────────────────────┘
//
class KeyInformation : public Btree::AreaObject
{
public:

	// コンストラクタ
	KeyInformation(
		const Trans::Transaction*			Transaction_,
		PhysicalFile::File*					File_,
		const PhysicalFile::PageID			NodePageID_,
		const Buffer::Page::FixMode::Value	FixMode_,
		const ModUInt32						KeyInformationIndex_,
		const bool							IsLeafPage_,
		const bool							SavePage_,
		const int							KeyNum_ = 0,
		const Os::Memory::Size				KeySize_ = 0);

	// コンストラクタ
	KeyInformation(const Trans::Transaction*	Transaction_,
				   PhysicalFile::Page*			NodePage_,
				   const ModUInt32				KeyInformationIndex_,
				   const bool					IsLeafPage_,
				   const int					KeyNum_ = 0,
				   const Os::Memory::Size		KeySize_ = 0);

	// デストラクタ
	~KeyInformation();

	// キー情報の書き込みサイズを返す [byte]
	static Os::Memory::Size
		getSize(const bool				IsLeafPage_,
				const KeyPosType::Value	KeyPosType_ = KeyPosType::KeyObject,
				const int				KeyNum_ = 0);

	// 物理ページをアタッチし直す
	void resetPhysicalPage(PhysicalFile::Page*	NewPhysicalPage_);

	// キーテーブルエリア内のオフセットを設定する
	void setStartOffsetByIndex(const ModUInt32	KeyInfoIndex_);

	// キーテーブル内でのインデックスを返す
	ModUInt32 getIndex() const;

	// キーテーブル内の次のキー情報へ移動する
	bool next(ModUInt32	LastIndex_);

	// キーテーブル内の前のキー情報へ移動する
	bool prev();

	//
	// 書き込みメソッド
	//

	//
	// リーフページを含むノードページのキー情報への書き込みメソッド
	//

	// 「キーオブジェクトのオブジェクトID」を書き込む
	void writeKeyObjectID(const ModUInt64	KeyObjectID_);

	//
	// リーフページ以外のノードページのキー情報への書き込みメソッド
	//

#ifdef OBSOLETE // 将来に対する予約
	// リーフページ以外のノードページのキー情報を書き込む
	void write(const PhysicalFile::PageID	ChildNodePageID_,
			   const ModUInt64				KeyObjectID_);
#endif

	// 「子ノードページの物理ページ識別子」を書き込む
	void
		writeChildNodePageID(
			const PhysicalFile::PageID	ChildNodePageID_);

	//
	// リーフページのキー情報への書き込みメソッド
	//

#ifdef OBSOLETE // 将来に対する予約
	// リーフページのキー情報を書き込む
	void write(const ModUInt64	ValueObjectID_,
			   const ModUInt64	KeyObjectID_);
#endif

	// 「バリューオブジェクトのオブジェクトID」を書き込む
	void writeValueObjectID(const ModUInt64	ValueObjectID_);

	// キー情報をコピーする
	void copy(const KeyInformation&	SrcKeyInfo_);

	//
	// 読み込みメソッド
	//

	//
	// リーフページを含むノードページのキー情報からの読み込みメソッド
	//

	// 「キーオブジェクトのオブジェクトID」を読み込む
	ModUInt64 readKeyObjectID() const;

	//
	// リーフページ以外のノードページのキー情報からの読み込みメソッド
	//

	// 「子ノードページの物理ページ識別子」を読み込む
	PhysicalFile::PageID readChildNodePageID() const;

	//
	// リーフページのキー情報からの読み込みメソッド
	//

	// 「バリューオブジェクトのオブジェクトID」を読み込む
	ModUInt64 readValueObjectID() const;

	//
	// キー値をキー情報内に記録するタイプのファイルの
	// 「キー値ヌルビットマップ」への参照メソッド
	//

	// 「キー値ヌルビットマップ」へのポインタを返す
	NullBitmap::Value* assignKeyNullBitmap() const;

	// 「キー値ヌルビットマップ」へのポインタを返す
	const NullBitmap::Value* assignConstKeyNullBitmap() const;

	// キー値先頭へのポインタを返す
	void* assignKeyTop() const;

	// キー値先頭へのポインタを返す
	const void* assignConstKeyTop() const;

	// キーテーブルエリアの物理エリア識別子
	static const PhysicalFile::AreaID	KeyTableAreaID;

private:

	// キー値記録先を設定する
	void setKeyPosType(const Os::Memory::Size	KeySize_);

	//
	// 静的データメンバ
	//

	// リーフページ以外のノードページのキー情報内での
	// 「子ノードページの物理ページ識別子」の位置 [byte]
	static const Os::Memory::Offset		ChildNodePageIDOffset;

	// リーフページ以外のノードページのキー情報内での
	// 「キーオブジェクトのオブジェクトID」の位置 [byte]
	static const Os::Memory::Offset		NodeKeyObjectIDOffset;

	// リーフページのキー情報内での
	// 「バリューオブジェクトのオブジェクトID」の位置 [byte]
	static const Os::Memory::Offset		ValueObjectIDOffset;

	// リーフページのキー情報内での
	// 「キーオブジェクトのオブジェクトID」の位置 [byte]
	static const Os::Memory::Offset		LeafKeyObjectIDOffset;

	// キー値をキーオブジェクトに記録するタイプのファイルの
	// リーフページ以外のノードページ内の
	// キー情報の書き込みサイズ [byte]
	static const Os::Memory::Size		SizeAtNode;

	// キー値をキーオブジェクトに記録するタイプのファイルの
	// リーフページ内のキー情報の書き込みサイズ [byte]
	static const Os::Memory::Size		SizeAtLeaf;

	//
	// 非静的データメンバ
	//

	// リーフページかどうか
	bool								m_IsLeafPage;

	// キーテーブル内のインデックス
	ModUInt32							m_Index;

	// キー情報へのポインタ
	char*								m_CharPointer;

	// キー情報へのポインタ
	const char*							m_ConstCharPointer;

	// キー値記録先
	KeyPosType::Value					m_KeyPosType;

	// キー値をキー情報内に記録するタイプのファイルのキーフィールド数
	int									m_KeyNum;

	// キー情報の記録サイズ
	// （キー値をキー情報内に記録するタイプのファイルの場合も
	// 　このサイズにはキー値の分は含まない。）
	Os::Memory::Size					m_Size;

	// キー値をキー情報内に記録するタイプのファイルのキー値の記録サイズ
	Os::Memory::Size					m_KeySize;

}; // end of class Btree::KeyInformation

//
//	FUNCTION public
//	Btree::KeyInformation::getIndex --
//		キーテーブル内のインデックスを返す
//
//	NOTES
//	キーテーブル内のインデックスを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		キーテーブル内のインデックス
//
//	EXCEPTIONS
//	なし
//
inline ModUInt32
KeyInformation::getIndex() const
{
	return this->m_Index;
}

_SYDNEY_BTREE_END
_SYDNEY_END

#endif // __SYDNEY_BTREE_KEYINFORMATION_H

//
//	Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
