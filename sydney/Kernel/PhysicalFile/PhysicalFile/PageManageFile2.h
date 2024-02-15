// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManageFile2.h --
//		物理ページ管理機能付き物理ファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_PAGEMANAGEFILE2_H
#define __SYDNEY_PHYSICALFILE_PAGEMANAGEFILE2_H

#include "PhysicalFile/File.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

//	CLASS
//	PhysicalFile::PageManageFile2 -- 物理ページ管理機能つき物理ファイルクラス
//
//	NOTES

class PageManageFile2 : public File {

	friend class Manager;
	friend class File;

public:

	// 物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPage(Page*&	Page_);

	// 生成されている全物理ページ記述子を破棄し、ページ内容を元に戻す
	void recoverPageAll();

	// 物理ページデータサイズを返す
	PageSize getPageDataSize(const AreaNum	DummyAreaNum_ = 1) const;

	// 先頭の使用中の物理ページの識別子を返す
	PageID getTopPageID(const Trans::Transaction&	Trans_);

	// 最後の使用中の物理ページの識別子を返す
	PageID getLastPageID(const Trans::Transaction&	Trans_);

	// 次の使用中の物理ページの識別子を返す
	PageID getNextPageID(const Trans::Transaction&	Trans_,
						 const PageID				PageID_);

	// 前の使用中の物理ページの識別子を返す
	PageID getPrevPageID(const Trans::Transaction&	Trans_,
						 const PageID				PageID_);

private:

	//	CLASS
	//	PhysicalFile::PageManageFile2::Tree -- 木構造を表すクラス
	//
	//	NOTES
	//	ひとつのノードと複数のリーフで構成される 2 段の木構造と
	//	その木構造で管理する物理ページの集合体

	class Tree {

	public:

		// コンストラクタ
		Tree();

		// デストラクタ
		~Tree();

		// ひとつの木構造あたりのページ数を設定する
		void setPageNum(const PageNum	LeafPerNode_,
						const PageNum	PagePerLeaf_);

		// ひとつの木構造あたりの物理ページ数を返す
		PageNum getPageNum() const;

		// ひとつの木構造あたりのバージョンページ数を返す
		PageNum getVersionPageNum() const;

	private:

		// ひとつの木構造あたりの物理ページ数
		PageNum	m_PageNum;

		// ひとつの木構造あたりのバージョンページ数
		PageNum	m_VersionPageNum;
	};

	class Leaf;

	//	CLASS
	//	PhysicalFile::PageManageFile2::Node -- ノードを表すクラス
	//
	//	NOTES
	//	ノードは複数のリーフをビットマップで管理していて、
	//	未使用の物理ページを管理しているリーフに対応するビットは ON 、
	//	未使用の物理ページを管理していない（つまり、管理している物理ページが
	//	すべて使用中である）リーフに対応するビットは OFF になっている。

	class Node {

	public:

		// コンストラクタ
		Node(const Tree&	Tree_,
			 const Leaf&	Leaf_);

		// デストラクタ
		~Node();

		// ビットマップサイズを設定する
		void setBitmapSize(const Os::Memory::Size	VersionPageDataSize_,
						   const Os::Memory::Size	FileHeaderSize_);

		// ビット ON/OFF
		void update(void*			Node_,
					const PageID	PageID_,
					const bool		BitON_) const;

		// 全ビット OFF
		void clear(void*	Node_) const;

		// ビット OFF
		void clear(const Trans::Transaction&	Trans_,
				   void*						Node_,
				   const PageID					PageID_,
				   const PageID					LastPageID_,
				   const PageNum				LeafUnusePageNum_) const;

		// ビット参照
		bool getBit(const void*		Node_,
					const PageID	PageID_) const;

		// 未使用物理ページを管理しているリーフを検索する
		int searchLeaf(const void*		Node_,
					   const PageNum	ManageLeafNum_) const;

		// 物理ページ以前に存在するノード数を返す
		PageNum getCount(const PageID	PageID_) const;

		// ノードのバージョンページ識別子を返す
		Version::Page::ID getVersionPageID(const PageID	PageID_) const;

		// ノードで管理可能なリーフ数を返す
		PageNum getManageLeafMax() const;

	private:

		// ビットマップ開始オフセット [byte]
		Os::Memory::Offset	m_BitmapOffset;

		// ビットマップサイズ [byte]
		Os::Memory::Size	m_BitmapSize;

		// 木構造オブジェクトへの参照
		const Tree&			m_Tree;

		// リーフオブジェクトへの参照
		const Leaf&			m_Leaf;

		// ノードで管理可能なリーフ数
		PageNum				m_ManageLeafMax;
	};

	//	CLASS
	//	PhysicalFile::PageManageFile2::Leaf -- リーフを表すクラス
	//
	//	NOTES
	//	リーフは複数の物理ページをビットマップで管理していて、
	//	使用中の物理ページに対応するビットは ON 、
	//	未使用の物理ー得時に対応するビットは OFF になっている。

	class Leaf {

	public:

		//	STRUCT
		//	PhysicalFile::PageManageFile2::Leaf::Header --
		//		リーフヘッダ
		//
		//	NOTES

		struct Header {

			// 使用中物理ページ数
			PageNum	m_UsedPageNum;

			// 未使用物理ページ数
			PageNum	m_UnusePageNum;
		};

		// コンストラクタ
		Leaf(const Tree&	Tree_,
			   const Node&	Node_);

		// デストラクタ
		~Leaf();

		// ビットマップサイズを設定する
		void setBitmapSize(const Os::Memory::Size	VersionPageDataSize_);

		// ビット ON/OFF
		void update(void*			Leaf_,
					const PageID	PageID_,
					const bool		BitON_,
					const bool		AppendPage_) const;

		// ビット参照
		bool getBit(const void*		Leaf_,
					const PageID	PageID_) const;

		// リーフから使用中の物理ページを検索する
		int searchUsedPage(const void*	Leaf_,
						   const int	StartIndex_ = -1) const;

		// リーフから使用中の物理ページを検索する
		int searchUsedPageBehind(const void*	Leaf_,
								 const int		StartIndex_ = -1) const;

		// リーフから未使用物理ページを検索する
		int searchUnusePage(const void*	Leaf_) const;

		// 物理ページ以前に存在するリーフ数を返す
		PageNum getCount(const PageID	PageID_) const;

		// リーフのバージョンページ識別子を返す
		Version::Page::ID getVersionPageID(const PageID	PageID_) const;

		// リーフで管理可能な物理ページ数を返す
		PageNum getManagePageMax() const;

		// リーフで管理している使用中の物理ページ数を更新する
		void updateUsedPageNum(void*		Leaf_,
							   const int	AddNum_) const;

		// リーフで管理している未使用の物理ページ数を更新する
		void updateUnusePageNum(void*		Leaf_,
								const int	AddNum_) const;

		// リーフで管理している使用中の物理ページ数を返す
		PageNum getUsedPageNum(const void*	Leaf_) const;

		// リーフで管理している未使用の物理ページ数を返す
		PageNum getUnusePageNum(const void*	Leaf_) const;

	private:

		// ビットマップ開始オフセット [byte]
		Os::Memory::Offset	m_BitmapOffset;

		// ビットマップサイズ [byte]
		Os::Memory::Size	m_BitmapSize;

		// 木構造オブジェクトへの参照
		const Tree&			m_Tree;

		// ノードオブジェクトへの参照
		const Node&			m_Node;

		// リーフで管理可能な物理ページ最大数
		PageNum				m_ManagePageMax;
	};

	// コンストラクタ
	PageManageFile2(const File::StorageStrategy&	FileStorageStrategy_,
					const File::BufferingStrategy&	BufferingStrategy_,
					const Lock::FileName*			LockName_,
					bool							Batch_);

	// デストラクタ
	virtual ~PageManageFile2();

	// Allocate Page instance.
	// File's pure virtual function
	Page* allocatePageInstance(
		const Trans::Transaction&			cTransaction_,
		PageID								uiPageID_,
		Buffer::Page::FixMode::Value		eFixMode_,
		Admin::Verification::Progress*		pProgress_ = 0,
		Buffer::ReplacementPriority::Value	eReplacementPriority_
											= Buffer::ReplacementPriority::Low);
	
	// 物理ファイル生成時の初期化
	void initialize(const Trans::Transaction&	Trans_,
					void*						FileHeader_);

	// トランケートする
	void truncate(const Trans::Transaction&	Trans_,
				  bool&						Modified_);

	// トランケートが必要かどうかを調べる
	bool needTruncate(const Trans::Transaction&	Trans_,
					  bool&						Modified_,
					  PageID&					LastManagePageID_,
					  PageID&					LastUsedPageID_);

	// 最後の使用中の物理ページの識別子を返す
	PageID getLastPageID(const Trans::Transaction&	Trans_,
						 const PageID				LastManagePageID_);

	// 物理ページデータサイズを返す
	static PageSize getPageDataSize(const Os::Memory::Size	VersionPageSize_,
									const AreaNum			DummyAreaNum_);

	// 物理ページ以前に存在する管理表数を返す
	PageNum getManageTableNum(const PageID	PageID_) const;

	// 管理表のバージョンページ識別子を返す
	Version::Page::ID getManageTableVersionPageID(const PageID	PageID_) const;

	// 木構造を更新する
	void updateTree(const Trans::Transaction&	Trans_,
					void*						FileHeader_,
					void*						Leaf_,
					const PageID				PageID_,
					const bool					IsUse_,
					const bool					AppendPage_,
					const bool					Discardable_);

	// ノードの全ビット OFF
	void clearNode(void*	Node_) const;

	// ノードのビット ON/OFF
	void updateNode(const Trans::Transaction&	Trans_,
					void*						FileHeader_,
					const PageID				PageID_,
					const bool					BitON_,
					const bool					Discardable_);

	// ノードとリーフをクリアする
	void clearTree(const Trans::Transaction&	Trans_,
				   const PageNum				TotalPageNum_);

	//	物理ページ追加のための準備を行う
	void
	prepareAppendPage(const Trans::Transaction&				Trans_,
					  void*									FileHeader_,
					  const PageID							LastPageID_,
					  const Buffer::Page::FixMode::Value	AllocateFixMode_);

	//	物理ページ追加のための準備を行う
	void
	prepareAppendPage(const Trans::Transaction&				Trans_,
					  void*									FileHeader_,
					  const PageID							LastPageID_,
					  const PageID							PageID_,
					  const Buffer::Page::FixMode::Value	AllocateFixMode_,
					  const Buffer::Page::FixMode::Value	WriteFixMode_);


	// 先頭物理ページを確保する
	void createTopPage(const Trans::Transaction&	Trans_,
					   void*						FileHeader_,
					   void*						TopLeaf_);

	// 物理ページ識別子からバージョンページ識別子へ変換する
	Version::Page::ID convertToVersionPageID(const PageID	PageID_) const;

	// 使用中の物理ページかどうかをチェックする
	bool isUsedPage(const void*		Leaf_,
					const PageID	PageID_) const;

	// 次に割り当てる物理ページを検索する
	PageID searchNextAssignPage(const Trans::Transaction&	Trans_,
								const void*					FileHeader_,
								const PageNum				ManagePageNum_);

	//
	// For checking consistency
	//

	// 整合性検査を行う
	void checkPhysicalFile(const Trans::Transaction&		Trans_,
						   Admin::Verification::Progress&	Progress_);

	// 管理物理ページ総数一致検査
	bool checkManagePageNumInFile(const Trans::Transaction&			Trans_,
								  Admin::Verification::Progress&	Progress_);

	// 使用中物理ページ総数一致検査
	bool checkUsedPageNumInFile(const Trans::Transaction&		Trans_,
								Admin::Verification::Progress&	Progress_);

	// ノードビットマップ検査
	bool checkNodeBitmap(const Trans::Transaction&		Trans_,
						 Admin::Verification::Progress&	Progress_);

	// 整合性検査のために管理表を fix する
	void verifyAllTable(const Trans::Transaction&		Trans_,
						Admin::Verification::Progress&	Progress_);

	// 利用者と自身の物理ページの使用状況が一致するかどうかをチェックする
	void correspondUsePage(const Trans::Transaction&		Trans_,
						   Admin::Verification::Progress&	Progress_);

	// 物理ページの使用状況を修復する
	void correctUsePage(const Trans::Transaction&		Trans_,
						const PageID					PageID_,
						void*							Leaf_,
						Admin::Verification::Progress&	Progress_);

	//
	// メンバ変数
	//

	// 木構造オブジェクト
	Tree	m_Tree;

	// ノードオブジェクト
	Node	m_Node;

	// リーフオブジェクト
	Leaf	m_Leaf;

}; // end of class PhysicalFile::PageManageFile2

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2 クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::AreaNum	DummyAreaNum_ = 1
//		[IN]		ダミーの物理エリア数
//					※ 空き領域管理機能付き物理ファイルと
//					　 インタフェースを共通化したために存在する。
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS

inline
PageSize
PageManageFile2::getPageDataSize(const AreaNum	DummyAreaNum/* = 1 */) const
{
	return this->m_UserAreaSizeMax;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2::Tree クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Tree::getPageNum --
//		ひとつの木構造あたりの物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	PhysicalFile::PageNum
//		ひとつの木構造あたりの物理ページ数
//
//	EXCEPTIONS

inline
PageNum
PageManageFile2::Tree::getPageNum() const
{
	return this->m_PageNum;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Tree::getVersionPageNum --
//		ひとつの木構造あたりのバージョンページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	PhysicalFile::PageNum
//		ひとつの木構造あたりのバージョンページ数
//
//	EXCEPTIONS

inline
PageNum
PageManageFile2::Tree::getVersionPageNum() const
{
	return this->m_VersionPageNum;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2::Node クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::getManageLeafMax --
//		ノードで管理可能なリーフ数を返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	PhysicalFile::PageNum
//		ノードで管理可能なリーフ数
//
//	EXCEPTIONS

inline
PageNum
PageManageFile2::Node::getManageLeafMax() const
{
	return this->m_ManageLeafMax;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFile2::Leaf クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::getManagePageMax --
//		リーフで管理可能な物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	PhysicalFile::PageNum
//		リーフで管理可能な物理ページ数
//
//	EXCEPTIONS

inline
PageNum
PageManageFile2::Leaf::getManagePageMax() const
{
	return this->m_ManagePageMax;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::updateUsedPageNum --
//		リーフで管理している使用中の物理ページ数を更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*		Leaf_
//		[IN]		リーフオブジェクト
//	const int	AddNum_
//		[IN]		物理ページ加算数
//
//	RETURN
//
//	EXCEPTIONS

inline
void
PageManageFile2::Leaf::updateUsedPageNum(void*		Leaf_,
										 const int	AddNum_) const
{
	(static_cast<Header*>(Leaf_))->m_UsedPageNum += AddNum_;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::updateUnusePageNum --
//		リーフで管理している未使用の物理ページ数を更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*		Leaf_
//		[IN]		リーフオブジェクト
//	const int	AddNum_
//		[IN]		物理ページ加算数
//
//	RETURN
//
//	EXCEPTIONS

inline
void
PageManageFile2::Leaf::updateUnusePageNum(void*		Leaf_,
										  const int	AddNum_) const
{
	(static_cast<Header*>(Leaf_))->m_UnusePageNum += AddNum_;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::getUsedPageNum --
//		リーフで管理している使用中の物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const void*	Leaf_
//		[IN]		リーフオブジェクト
//
//	RETURN
//	PhysicalFile::PageNum
//		リーフで管理している使用中の物理ページ数
//
//	EXCEPTIONS

inline
PageNum
PageManageFile2::Leaf::getUsedPageNum(const void*	Leaf_) const
{
	return (static_cast<const Header*>(Leaf_))->m_UsedPageNum;
}

//	FUNCTION public
//	PhysicalFile::PageManageFile2::Leaf::getUnusePageNum --
//		リーフで管理している未使用の物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const void*	Leaf_
//		[IN]		リーフオブジェクト
//
//	RETURN
//	PhysicalFile::PageNum
//		リーフで管理している未使用の物理ページ数
//
//	EXCEPTIONS

inline
PageNum
PageManageFile2::Leaf::getUnusePageNum(const void*	Leaf_) const
{
	return (static_cast<const Header*>(Leaf_))->m_UnusePageNum;
}

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_PHYSICALFILE_PAGEMANAGEFILE2_H

//
//	Copyright (c) 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
