// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaManageNode.h --
//		物理エリア管理ノードのクラス定義、関数宣言
// 
// Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PHYSICALFILE_DIRECTAREAMANAGENODE_H
#define __SYDNEY_PHYSICALFILE_DIRECTAREAMANAGENODE_H

#include "PhysicalFile/DirectArea.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_PHYSICALFILE_BEGIN

//	CLASS
//	PhysicalFile::DirectAreaManageNode --
//		物理エリア管理ノードクラス
//
//	NOTES
//	※ リーフノードを除く。
//	物理エリア管理ノード内には空き領域率ビットマップが並んでいる。
//	空き領域率ビットマップは、ひとつの子ノード（リーフノード含む）につき、
//	1 バイトが割り当てられ、各ビットは下図のような意味合いをもち、
//	任意のビットが ON の状態である。
//	例えば、子ノードが 53 ％、25 ％、7 ％の空き領域をもつ物理ページ
//	（計 3 ページ）を管理している場合、その子ノードに対応する 1 バイトは、
//	ビット 2, 4, 7 が ON となる。
//	また、子ノードが管理しているいずれの物理ページも 5 ％以上の空き領域を
//	もっていない場合、その子ノードに対応する 1 バイトは、
//	すべてのビットが OFF となる。
//	※ 空き領域によりノードをたどり物理ページを検索する場合、
//	　 4 ％以下の空き領域に関しては無視されるためリーフノード以外のノードでは
//	　 管理の必要がない。
//
//	┌─┬─┬─┬─┬─┬─┬─┬─┐
//	│７│６│５│４│３│２│１│０│
//	└┼┴┼┴┼┴┼┴┼┴┼┴┼┴┼┘
//	　│　│　│　│　│　│　│　│
//	　│　│　│　│　│　│　│　└─ 該当子ノードが 80 ％以上の空き領域をもつ
//	　│　│　│　│　│　│　└─── 　　　〃　　　 60 〜 79 ％　　　〃
//	　│　│　│　│　│　└───── 　　　〃　　　 40 〜 59 ％　　　〃
//	　│　│　│　│　└─────── 　　　〃　　　 30 〜 39 ％　　　〃
//	　│　│　│　└───────── 　　　〃　　　 20 〜 29 ％　　　〃
//	　│　│　└─────────── 　　　〃　　　 15 〜 19 ％　　　〃
//	　│　└───────────── 　　　〃　　　 10 〜 14 ％　　　〃
//	　└─────────────── 　　　〃　　　  5 〜  9 ％　　　〃

class DirectAreaManageNode {

	friend class DirectAreaManageLeaf;

public:

	//	TYPEDEF public
	//	PhysicalFile::DirectAreaManageNode::AreaRate --
	//		領域率を表す型
	//
	//	NOTES

	typedef unsigned char	AreaRate;

	//	TYPEDEF public
	//	PhysicalFile::DirectAreaManageNode::Index --
	//		ノードインデックスを示す型
	//
	//	NOTES
	//	同一段ノードの通し番号

	typedef unsigned int	Index;

	//	CONST public
	//	PhysicalFile::DirectAreaManageNode::UnknownIndex --
	//		未知のノードインデックス
	//
	//	NOTES

	static const Index	UnknownIndex;

	//	STRUCT public
	//	PhysicalFile::DirectAreaManageNode::Info --
	//		ノード情報構造体
	//
	//	NOTES

	struct Info {

		Info() : _VersionPageID(0),
				 _NumManageChildren(0),
				 _Index(UnknownIndex),
				 _ChildIndex(UnknownIndex) {};

		// リーフノードを含むノードのバージョンページ識別子
		Version::Page::ID	_VersionPageID;

		// ノード内で管理している子ノード数
		// （リーフの場合は、リーフで管理している物理ページ数）
		PageNum				_NumManageChildren;

		// リーフノードを含むノードのインデックス（同一段での通し番号）
		Index				_Index;

		// 子ノードのインデックス（リーフの場合は、物理ページのインデックス）
		// （同一段での通し番号）
		Index				_ChildIndex;
	};

	//	TYPEDEF public
	//	PhysicalFile::DirectAreaManageNode::Route --
	//		リーフノードからトップノードまでのノード情報を積むベクター
	//
	//	NOTES

	typedef ModVector<Info>	Route;

	// コンストラクタ
	DirectAreaManageNode(const DirectArea::Size	DirectAreaMaxSize_,
						 const PageNum			MaxChildren_);

	// デストラクタ
	~DirectAreaManageNode();

	// 子ノードのインデックスを検索する
	Index searchChildNodeIndex(const void*				ParentNodePointer_,
							   const Index				ParentNodeIndex_,
							   const PageNum			NumChildren_,
							   const DirectArea::Size	SearchAreaSize_) const;

	// ノード内で管理している子ノード数を返す
	static PageNum getNumManageChildren(const PageNum	NumTreePage_,
										const PageNum	MaxChildren_,
										//const Depth		Depth_,
										//const Depth		NodeDepth_,
										const Index		NodeIndex_);

	// 物理エリアサイズを領域率ビットマップに変換する
	AreaRate convertToRate(const DirectArea::Size	AreaSize_) const;

	// 子ノードが管理している物理ページの空き領域率を返す
	static AreaRate get(const void*	NodePointer_,
						const Index	ChildNodeIndex_);

	// ノードで管理している子ノードの空き領域率を集計する
	static AreaRate total(const void*	NodePointer_,
						  const PageNum	NumManageChildren_);

	// ノードを更新する
	static AreaRate update(void*			NodePointer_,
						   const PageNum	NumManageChildren_,
						   const Index		ChildNodeIndex_,
						   const AreaRate	ChildNodeAreaRate_);
	static void update(void*			NodePointer_,
					   const Index		ChildNodeIndex_,
					   const AreaRate	ChildNodeAreaRate_);

private:

	//	CONST private
	//	PhysicalFile::DirectAreaManageNode::Rate_*_* --
	//		領域率を表す値
	//
	//	NOTES

	static const AreaRate	Rate_80_100;	// 80 ％以上の領域		= 0x01
	static const AreaRate	Rate_60_79;		// 60 〜 79 ％の領域	= 0x02
	static const AreaRate	Rate_40_59;		// 40 〜 59 ％の領域	= 0x04
	static const AreaRate	Rate_30_39;		// 30 〜 39 ％の領域	= 0x08
	static const AreaRate	Rate_20_29;		// 20 〜 29 ％の領域	= 0x10
	static const AreaRate	Rate_15_19;		// 15 〜 19 ％の領域	= 0x20
	static const AreaRate	Rate_10_14;		// 10 〜 14 ％の領域	= 0x40
	static const AreaRate	Rate_5_9;		//  5 〜  9 ％の領域	= 0x80
	static const AreaRate	Rate_0_4;		//  0 〜  4 ％の領域	= 0x00

	//	CONST private
	//	PhysicalFile::DirectAreaManageNode::ToAreaRate --
	//		物理エリアの物理ページ内での 100 分率から領域率 AreaRate への変換表
	//
	//	NOTES

	static const AreaRate	ToAreaRate[101];

	// 物理エリア最大サイズ
	DirectArea::Size	m_DirectAreaMaxSize;

	// 親ノードで管理可能な最大子ノード数
	PageNum				m_MaxChildren;

};	// end of class PhysicalFile::DirectAreaManageNode

//	CLASS
//	PhysicalFile::DirectAreaManageLeaf --
//		物理エリア管理リーフノードクラス
//
//	NOTES
//	物理エリア管理リーフノード内には空き領域率ビットマップが並んでいる。
//	空き領域率ビットマップは、1 物理ページにつき、1 バイトが割り当てられ、
//	各ビットは下図のような意味合いをもち、すべてのビットが OFF であるか、
//	いずれかひとつのビットが ON の状態である。
//	※ すべてのビットが OFF で 80％以上の空き領域をもつことを示す。
//
//	┌─┬─┬─┬─┬─┬─┬─┬─┐
//	│７│６│５│４│３│２│１│０│
//	└┼┴┼┴┼┴┼┴┼┴┼┴┼┴┼┘
//	　│　│　│　│　│　│　│　│
//	　│　│　│　│　│　│　│　└─ 物理ページが 60 〜 79 ％の空き領域をもつ
//	　│　│　│　│　│　│　└─── 　　〃　　　 40 〜 59 ％　　　〃
//	　│　│　│　│　│　└───── 　　〃　　　 30 〜 39 ％　　　〃
//	　│　│　│　│　└─────── 　　〃　　　 20 〜 29 ％　　　〃
//	　│　│　│　└───────── 　　〃　　　 15 〜 19 ％　　　〃
//	　│　│　└─────────── 　　〃　　　 10 〜 14 ％　　　〃
//	　│　└───────────── 　　〃　　　  5 〜  9 ％　　　〃
//	　└─────────────── 　　〃　　　  4 ％以下　　　　〃
//

class DirectAreaManageLeaf {

public:

	//	TYPEDEF public
	//	PhysicalFile::DirectAreaManageLeaf::AreaRate --
	//		領域率を表す型
	//
	//	NOTES

	typedef unsigned char	AreaRate;

	// コンストラクタ
	DirectAreaManageLeaf(const DirectArea::Size	DirectAreaMaxSize_,
						 const PageNum			MaxPage_);

	// デストラクタ
	~DirectAreaManageLeaf();

	// 物理ページのインデックスを検索する
	DirectAreaManageNode::Index
	searchPageIndex(const void*							LeafPointer_,
					const DirectAreaManageNode::Index	LeafIndex_,
					const PageNum						NumPage_,
					const DirectArea::Size				SearchAreaSize_) const;

	// リーフノード内で管理している物理ページ数を返す
	static PageNum
	getNumManagePage(const PageNum						NumTreePage_,
					 const PageNum						MaxPage_,
					 const DirectAreaManageNode::Index	LeafIndex_);

	// 物理エリアサイズを領域率に変換する
	AreaRate convertToRate(const DirectArea::Size	AreaSize_) const;

	// 物理ページの空き領域率を返す
	static AreaRate get(const void*							LeafPointer_,
						const DirectAreaManageNode::Index	PageIndex_);

	// リーフノードで管理している物理ページの空き領域率を集計する
	static DirectAreaManageNode::AreaRate
	total(const void*	LeafPointer_,
		  const PageNum	NumManagePage_);

	// リーフノードを更新する
	static DirectAreaManageNode::AreaRate
	update(void*								LeafPointer_,
		   const PageNum						NumManagePage_,
		   const DirectAreaManageNode::Index	PageIndex_,
		   const AreaRate						PageFreeAreaRate_);
	static void update(void*								LeafPointer_,
					   const DirectAreaManageNode::Index	PageIndex_,
					   const AreaRate						PageFreeAreaRate_);

	//	CONST private
	//	PhysicalFile::DirectAreaManageLeaf::Rate_*_* --
	//		領域率を表す値
	//
	//	NOTES
	//	※ PhysicalFile::DirectAreaManageNode::Rate_*_* とは値が異なる。

	static const AreaRate	Rate_80_100;	// 80 ％以上の領域		= 0x00
	static const AreaRate	Rate_60_79;		// 60 〜 79 ％の領域	= 0x01
	static const AreaRate	Rate_40_59;		// 40 〜 59 ％の領域	= 0x02
	static const AreaRate	Rate_30_39;		// 30 〜 39 ％の領域	= 0x04
	static const AreaRate	Rate_20_29;		// 20 〜 29 ％の領域	= 0x08
	static const AreaRate	Rate_15_19;		// 15 〜 19 ％の領域	= 0x10
	static const AreaRate	Rate_10_14;		// 10 〜 14 ％の領域	= 0x20
	static const AreaRate	Rate_5_9;		//  5 〜  9 ％の領域	= 0x40
	static const AreaRate	Rate_0_4;		//  4 ％以下の領域		= 0x80

private:

	//	CONST private
	//	PhysicalFile::DirectAreaManageLeaf::ToAreaRate --
	//		物理エリアの物理ページ内での 100 分率から領域率 AreaRate への変換表
	//
	//	NOTES

	static const AreaRate	ToAreaRate[101];

	// 物理エリア最大サイズ
	DirectArea::Size	m_DirectAreaMaxSize;

	// リーフノードで管理可能な最大物理ページ数
	PageNum				m_MaxPage;

};	// end of class PhysicalFile::DirectAreaManageLeaf

_SYDNEY_PHYSICALFILE_END
_SYDNEY_END

#endif // __SYDNEY_PHYSICALFILE_DIRECTAREAMANAGENODE_H

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
