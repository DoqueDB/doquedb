// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirectAreaManageNode.cpp --
//		物理エリア管理ノードの関数定義
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "PhysicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "PhysicalFile/DirectAreaManageNode.h"

#include "Common/Assert.h"

#include "Os/Math.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

// static
const DirectAreaManageNode::Index
DirectAreaManageNode::UnknownIndex = 0xFFFFFFFF;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaManageNode クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::DirectAreaManageNode --
//		コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const PhysialFile::DirectArea::Size	DirectAreaMaxSize_
//		[IN]		物理エリア最大サイズ [byte]
//	const PhysicalFile::PageNum			MaxChildren_
//		[IN]		親ノードで管理可能な最大子ノード数
//
//	RETURN
//
//	EXCEPTIONS

DirectAreaManageNode::DirectAreaManageNode(
	const DirectArea::Size	DirectAreaMaxSize_,
	const PageNum			MaxChildren_)
	: m_DirectAreaMaxSize(DirectAreaMaxSize_),
	  m_MaxChildren(MaxChildren_)
{
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::~DirectAreaManageNode --
//		デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

DirectAreaManageNode::~DirectAreaManageNode()
{
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::searchChildNodeIndex --
//		親ノード内で子ノードのインデックスを検索する
//
//	NOTES
//	引数 SearchAreaSize_ で指定された物理エリアサイズ以上の
//	空き領域をもつ物理ページを管理しているリーフノードへたどる
//	子ノードを検索し、その子ノードの同一段での通し番号を返す。
//	（親ノード内だけでのインデックスではない。）
//	該当する子ノードが存在しない場合には、
//	PhysicalFile::DirectAreaManageNode::UnknownIndex を返す。
//
//	ARGUMENTS
//	const void*										ParentNodePointer_
//		[IN]		親ノードへのポインタ
//	const PhysicalFile::DirectAreaManageNode::Index	ParentNodeIndex_
//		[IN]		親ノードのインデックス
//					（親ノードの同一段での通し番号）
//	const PhysicalFile::PageNum						NumChildren_
//		[IN]		親ノードで管理している子ノード数
//	const PhysicalFile::DirectArea::Size			SearchAreaSize_
//		[IN]		検索する物理エリアサイズ [byte]
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::Index
//		子ノードのインデックス（子ノードの同一段での通し番号）
//		該当する子ノードが存在しない場合には、
//		PhysicalFile::DirectAreaManageNode::UnknownIndex
//
//	EXCEPTIONS

DirectAreaManageNode::Index
DirectAreaManageNode::searchChildNodeIndex(
	const void*				ParentNodePointer_,
	const Index				ParentNodeIndex_,
	const PageNum			NumChildren_,
	const DirectArea::Size	SearchAreaSize_) const
{
	// 例えば、4 ％の領域を探すのであれば 5 ％以上の空きを、
	// 10 ％の領域を探すのであれば 15 ％以上の空きを、
	// というように、少し余裕のあるページを管理している子ノードを検索する。
	// そのため、検索すべき領域率を 1 ランク上げる。
	AreaRate	searchAreaRate = this->convertToRate(SearchAreaSize_);
	if (searchAreaRate == Rate_0_4) {
		searchAreaRate = Rate_5_9;
	} else {
		searchAreaRate >>= 1;
	} //               ~~~~ 1 ランク上げている

	Index	childNodeIndex = UnknownIndex;

	// 検索すべき領域率よりももっと余裕のあるページを管理している子ノードの
	// インデックスを格納するための配列
	Index	spareIndices[6] = { UnknownIndex,
								UnknownIndex,
								UnknownIndex,
								UnknownIndex,
								UnknownIndex,
								UnknownIndex };

	const AreaRate*	parentNode =
		static_cast<const AreaRate*>(ParentNodePointer_);

	for (PageNum i = 0; i < NumChildren_; i++) {

		AreaRate	childNodeAreaRate = *(parentNode + i);

		if (childNodeAreaRate & searchAreaRate) {
			childNodeIndex = i;
			break;
		}
		// もし検索すべき領域率よりももっと余裕のあるページを
		// 管理している子ノードであるならば、
		// ストックしていないランクならばストックしておく。
		AreaRate	spareAreaRate = searchAreaRate >> 1;
		int	j;
		for (j = 0; spareAreaRate != Rate_0_4; j++, spareAreaRate >>= 1) {
			if ((childNodeAreaRate & spareAreaRate) &&
				(spareIndices[j] == UnknownIndex)) {
				spareIndices[j] = i;
				break;
			}
		}
	}

	if (childNodeIndex == UnknownIndex) {

		// わりとぴったりめの子ノードは見つからなかったが、
		// 余裕のあるページを管理している子ノードがあるならば、
		// その子ノードを検索結果として返してあげる。

		for (int i = 0; i < 6; i++) {

			if (spareIndices[i] != UnknownIndex) {
				childNodeIndex = spareIndices[i];
				break;
			}
		}
	}

	return
		(childNodeIndex == UnknownIndex) ?
		childNodeIndex :
		this->m_MaxChildren * ParentNodeIndex_ + childNodeIndex;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::getNumManageChildren --
//		ノード内で管理している子ノード数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageNum						NumTreePage_
//		[IN]		木構造で管理している物理ページ数
//					※ 物理ファイル内に存在する物理ページ数から 1 を引いた値
//	const PhysicalFile::PageNum						MaxChildren_
//		[IN]		ひとつのノードで管理可能な子ノード数
//	const PhysicalFile::Depth						Depth_
//		[IN]		物理ファイル内の木構造の段数（深さ）
//	const PhysicalFile::Depth						NodeDepth_
//		[IN]		ノードの段
//	const PhysicalFile::DirectAreaManageNode::Index	NodeIndex_
//		[IN]		ノードインデックス（同一段でのノードの通し番号）
//
//	RETURN
//	PhysicalFile::PageNum
//		親ノード内で管理している子ノード数
//
//	EXCEPTIONS

// static
PageNum
DirectAreaManageNode::getNumManageChildren(const PageNum	NumTreePage_,
										   const PageNum	MaxChildren_,
										   //const Depth		Depth_,
										   //const Depth		NodeDepth_,
										   const Index		NodeIndex_)
{
	// リーフで管理しているページ数は
	// DirectAreaManageLeaf::getNumManagePage() で取得すべし

	// コンパイルのため
	int Depth_ = 0;
	int NodeDepth_ = 0;
	// 以上

	; _SYDNEY_ASSERT(Depth_ > 1);

	PageNum	numManageChildren = 0;

	// （親ノードの段での）同一段の最後のノードかどうかをチェック
	Index	lastNodeIndex = 0;
	if (Depth_ == 3 && NodeDepth_ == 2) {
		lastNodeIndex = (NumTreePage_ - 1) / (MaxChildren_ * MaxChildren_);
	}
	if (NodeIndex_ == lastNodeIndex) {

		// 同一段で最後のノード

		if (Depth_ == 2) {

			// 2 段

			; _SYDNEY_ASSERT(NodeDepth_ == 1);

			numManageChildren = (NumTreePage_ - 1) % MaxChildren_ + 1;

		} else {

			// 3 段

			; _SYDNEY_ASSERT(Depth_ == 3);

			if (NodeDepth_ == 1) {

				numManageChildren =
					(NumTreePage_ - 1) / (MaxChildren_ * MaxChildren_) + 1;

			} else {

				; _SYDNEY_ASSERT(NodeDepth_ == 2);

				numManageChildren =
					((NumTreePage_ - 1) % (MaxChildren_ * MaxChildren_)) / MaxChildren_ + 1;
			}
		}

	} else {

		// 同一段で最後のノードではない
		numManageChildren = MaxChildren_;
	}

	; _SYDNEY_ASSERT(numManageChildren > 0);

	return numManageChildren;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::convertToRate --
//		物理エリアサイズを領域率に変換する
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea::Size	AreaSize_
//		[IN]		物理エリアサイズ [byte]
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::AreaRate
//		領域率を表す値
//
//	EXCEPTIONS

DirectAreaManageNode::AreaRate
DirectAreaManageNode::convertToRate(const DirectArea::Size	AreaSize_) const
{
	unsigned char	areaPercentage =
		static_cast<unsigned char>(
			static_cast<double>(AreaSize_) / this->m_DirectAreaMaxSize * 100);
	return ToAreaRate[areaPercentage];
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::get --
//		子ノードが管理している物理ページの空き領域率を返す
//
//	NOTES
//	返される値は、子ノードでどの程度の空き領域率をもつ物理ページを
//	管理しているかを示す値である。
//
//	ARGUMENTS
//	const void*										NodePointer_
//		[IN]		ノードへのポインタ
//	const PhysicalFile::DirectAreaManageNode::Index	ChildNodeIndex_
//		[IN]		親ノード内での子ノードのインデックス
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::AreaRate
//		子ノードが管理している物理ページの空き領域率
//
//	EXCEPTIONS

// static
DirectAreaManageNode::AreaRate
DirectAreaManageNode::get(const void*	NodePointer_,
						  const Index	ChildNodeIndex_)
{
	const AreaRate*	childNodeFreeAreaRate =
		static_cast<const AreaRate*>(NodePointer_) + ChildNodeIndex_;
	return *childNodeFreeAreaRate;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::total --
//		ノードで管理している子ノードの空き領域率を集計する
//
//	NOTES
//	ノードで管理している子ノードの空き領域率を集計し、
//	ノードの親ノードへ記録するための空き領域率を表すビットマップを返す。
//
//	ARGUMENTS
//	const void*					NodePointer_
//		[IN]		ノードへのポインタ
//	const PhysicalFile::PageNum	NumManageChildren_
//		[IN]		ノード内で管理している子ノード数
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::AreaRate
//		親ノードへ記録するための空き領域率を表すビットマップ
//
//	EXCEPTIONS

// static
DirectAreaManageNode::AreaRate
DirectAreaManageNode::total(const void*		NodePointer_,
							const PageNum	NumManageChildren_)
{
	AreaRate	totalRate = Rate_0_4;

	const AreaRate*	nodePointer = static_cast<const AreaRate*>(NodePointer_);

	for (PageNum i = 0; i < NumManageChildren_ && totalRate != 0xFF; i++) {

		totalRate |= *(nodePointer + i);
	}

	return totalRate;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::update -- ノードを更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*												NodePointer_
//		[IN]		ノードへのポインタ
//	const PhysicalFile::PageNum							NumManageChildren_
//		[IN]		ノード内で管理している子ノード数
//	const PhysicalFile::DirectAreaManageNode::Index		ChildNodeIndex_
//		[IN]		親ノード内での子ノードのインデックス
//	const PhysicalFile::DirectAreaManageNode::AreaRate	ChildNodeAreaRate_
//		[IN]		子ノードで管理している空き領域率を表すビットマップ
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::AreaRate
//		更新後の空き領域率をあらわすビットマップ

// static
DirectAreaManageNode::AreaRate
DirectAreaManageNode::update(void*			NodePointer_,
							 const PageNum	NumManageChildren_,
							 const Index	ChildNodeIndex_,
							 const AreaRate	ChildNodeAreaRate_)
{
	update(NodePointer_, ChildNodeIndex_, ChildNodeAreaRate_);

	return total(NodePointer_, NumManageChildren_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageNode::update -- ノードを更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*	NodePointer_
//		[IN]		ノードへのポインタ
//	const PhysicalFile::DirectAreaManageNode::Index	ChildNodeIndex_
//		[IN]		親ノード内での子ノードのインデックス
//	const PhysicalFile::DirectAreaManageNode::AreaRate	ChildNodeAreaRate_
//		[IN]		子ノードで管理している空き領域率を表すビットマップ
//
//	RETURN
//
//	EXCEPTIONS

// static
void
DirectAreaManageNode::update(void*			NodePointer_,
							 const Index	ChildNodeIndex_,
							 const AreaRate	ChildNodeAreaRate_)
{
	AreaRate*	childNodeAreaRate =
		static_cast<AreaRate*>(NodePointer_) + ChildNodeIndex_;
	*childNodeAreaRate = ChildNodeAreaRate_;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaManageNode クラスの private 定数など
//
///////////////////////////////////////////////////////////////////////////////

//	CONST private
//	PhysicalFile::DirectAreaManageNode::Rate_**_** -- 領域率表す値
//
//	NOTES
//	下記参照

//	80 ％以上の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_80_100 = 0x01;

//	60 〜 79 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_60_79 = 0x02;

//	40 〜 59 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_40_59 = 0x04;

//	30 〜 39 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_30_39 = 0x08;

//	20 〜 29 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_20_29 = 0x10;

//	15 〜 19 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_15_19 = 0x20;

//	10 〜 14 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_10_14 = 0x40;

//	5 〜 9 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_5_9 = 0x80;

//	0 〜 4 ％の領域
// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::Rate_0_4 = 0x00;

//	CONST private
//	PhysicalFile::DirectAreaManageNode::ToAreaRate --
//		物理エリアの物理ページ内での 100 分率から
//		領域率 DirectAreaManageNode::AreaRate への変換表
//
//	NOTES

// static
const DirectAreaManageNode::AreaRate
DirectAreaManageNode::ToAreaRate[101] = {
	DirectAreaManageNode::Rate_0_4,		//   0 [%]
	DirectAreaManageNode::Rate_0_4,		//   1
	DirectAreaManageNode::Rate_0_4,		//   2
	DirectAreaManageNode::Rate_0_4,		//   3
	DirectAreaManageNode::Rate_0_4,		//   4
	DirectAreaManageNode::Rate_5_9,		//   5
	DirectAreaManageNode::Rate_5_9,		//   6
	DirectAreaManageNode::Rate_5_9,		//   7
	DirectAreaManageNode::Rate_5_9,		//   8
	DirectAreaManageNode::Rate_5_9,		//   9
	DirectAreaManageNode::Rate_10_14,	//  10
	DirectAreaManageNode::Rate_10_14,	//  11
	DirectAreaManageNode::Rate_10_14,	//  12
	DirectAreaManageNode::Rate_10_14,	//  13
	DirectAreaManageNode::Rate_10_14,	//  14
	DirectAreaManageNode::Rate_15_19,	//  15
	DirectAreaManageNode::Rate_15_19,	//  16
	DirectAreaManageNode::Rate_15_19,	//  17
	DirectAreaManageNode::Rate_15_19,	//  18
	DirectAreaManageNode::Rate_15_19,	//  19
	DirectAreaManageNode::Rate_20_29,	//  20
	DirectAreaManageNode::Rate_20_29,	//  21
	DirectAreaManageNode::Rate_20_29,	//  22
	DirectAreaManageNode::Rate_20_29,	//  23
	DirectAreaManageNode::Rate_20_29,	//  24
	DirectAreaManageNode::Rate_20_29,	//  25
	DirectAreaManageNode::Rate_20_29,	//  26
	DirectAreaManageNode::Rate_20_29,	//  27
	DirectAreaManageNode::Rate_20_29,	//  28
	DirectAreaManageNode::Rate_20_29,	//  29
	DirectAreaManageNode::Rate_30_39,	//  30
	DirectAreaManageNode::Rate_30_39,	//  31
	DirectAreaManageNode::Rate_30_39,	//  32
	DirectAreaManageNode::Rate_30_39,	//  33
	DirectAreaManageNode::Rate_30_39,	//  34
	DirectAreaManageNode::Rate_30_39,	//  35
	DirectAreaManageNode::Rate_30_39,	//  36
	DirectAreaManageNode::Rate_30_39,	//  37
	DirectAreaManageNode::Rate_30_39,	//  38
	DirectAreaManageNode::Rate_30_39,	//  39
	DirectAreaManageNode::Rate_40_59,	//  40
	DirectAreaManageNode::Rate_40_59,	//  41
	DirectAreaManageNode::Rate_40_59,	//  42
	DirectAreaManageNode::Rate_40_59,	//  43
	DirectAreaManageNode::Rate_40_59,	//  44
	DirectAreaManageNode::Rate_40_59,	//  45
	DirectAreaManageNode::Rate_40_59,	//  46
	DirectAreaManageNode::Rate_40_59,	//  47
	DirectAreaManageNode::Rate_40_59,	//  48
	DirectAreaManageNode::Rate_40_59,	//  49
	DirectAreaManageNode::Rate_40_59,	//  50
	DirectAreaManageNode::Rate_40_59,	//  51
	DirectAreaManageNode::Rate_40_59,	//  52
	DirectAreaManageNode::Rate_40_59,	//  53
	DirectAreaManageNode::Rate_40_59,	//  54
	DirectAreaManageNode::Rate_40_59,	//  55
	DirectAreaManageNode::Rate_40_59,	//  56
	DirectAreaManageNode::Rate_40_59,	//  57
	DirectAreaManageNode::Rate_40_59,	//  58
	DirectAreaManageNode::Rate_40_59,	//  59
	DirectAreaManageNode::Rate_60_79,	//  60
	DirectAreaManageNode::Rate_60_79,	//  61
	DirectAreaManageNode::Rate_60_79,	//  62
	DirectAreaManageNode::Rate_60_79,	//  63
	DirectAreaManageNode::Rate_60_79,	//  64
	DirectAreaManageNode::Rate_60_79,	//  65
	DirectAreaManageNode::Rate_60_79,	//  66
	DirectAreaManageNode::Rate_60_79,	//  67
	DirectAreaManageNode::Rate_60_79,	//  68
	DirectAreaManageNode::Rate_60_79,	//  69
	DirectAreaManageNode::Rate_60_79,	//  70
	DirectAreaManageNode::Rate_60_79,	//  71
	DirectAreaManageNode::Rate_60_79,	//  72
	DirectAreaManageNode::Rate_60_79,	//  73
	DirectAreaManageNode::Rate_60_79,	//  74
	DirectAreaManageNode::Rate_60_79,	//  75
	DirectAreaManageNode::Rate_60_79,	//  76
	DirectAreaManageNode::Rate_60_79,	//  77
	DirectAreaManageNode::Rate_60_79,	//  78
	DirectAreaManageNode::Rate_60_79,	//  79
	DirectAreaManageNode::Rate_80_100,	//  80
	DirectAreaManageNode::Rate_80_100,	//  81
	DirectAreaManageNode::Rate_80_100,	//  82
	DirectAreaManageNode::Rate_80_100,	//  83
	DirectAreaManageNode::Rate_80_100,	//  84
	DirectAreaManageNode::Rate_80_100,	//  85
	DirectAreaManageNode::Rate_80_100,	//  86
	DirectAreaManageNode::Rate_80_100,	//  87
	DirectAreaManageNode::Rate_80_100,	//  88
	DirectAreaManageNode::Rate_80_100,	//  89
	DirectAreaManageNode::Rate_80_100,	//  90
	DirectAreaManageNode::Rate_80_100,	//  91
	DirectAreaManageNode::Rate_80_100,	//  92
	DirectAreaManageNode::Rate_80_100,	//  93
	DirectAreaManageNode::Rate_80_100,	//  94
	DirectAreaManageNode::Rate_80_100,	//  95
	DirectAreaManageNode::Rate_80_100,	//  96
	DirectAreaManageNode::Rate_80_100,	//  97
	DirectAreaManageNode::Rate_80_100,	//  98
	DirectAreaManageNode::Rate_80_100,	//  99
	DirectAreaManageNode::Rate_80_100	// 100
};

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaManageLeaf クラスの public メンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::DirectAreaManageLeaf --
//		コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea::Size	DirectAreaMaxSize_
//		[IN]		物理エリア最大サイズ [byte]
//	const PhysicalFile::PageNum				MaxPage_
//		[IN]		リーフノードで管理可能な最大物理ページ数
//
//	RETURN
//
//	EXCEPTIONS

DirectAreaManageLeaf::DirectAreaManageLeaf(
	const DirectArea::Size	DirectAreaMaxSize_,
	const PageNum			MaxPage_)
	: m_DirectAreaMaxSize(DirectAreaMaxSize_),
	  m_MaxPage(MaxPage_)
{
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::~DirectAreaManageLeaf --
//		デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

DirectAreaManageLeaf::~DirectAreaManageLeaf()
{
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::searchPageIndex --
//		リーフノード内で物理ページのインデックスを検索する
//
//	NOTES
//	引数 SearchAreaSize_ で指定された物理エリアサイズ以上の
//	空き領域をもつ物理ページを管理しているリーフノードを検索し、
//	その物理ページの通し番号を返す。
//	（リーフノード内だけでのインデックスではない。）
//	該当する物理ページが存在しない場合には、
//	PhysicalFile::DirectAreaManageNode::UnknownIndex を返す。
//
//	ARGUMENTS
//	const void*										LeafPointer_
//		[IN]		リーフノードへのポインタ
//	const PhysicalFile::DirectAreaManageNode::Index	LeafIndex_
//		[IN]		リーフノードのインデックス
//	const PhysicalFile::PageNum						NumPage_
//		[IN]		リーフノードで管理している物理ページ数
//	const PhysicalFile::DirectArea::Size			SearchAreaSize_
//		[IN]		検索する物理エリアサイズ [byte]
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::Index
//		物理ページのインデックス
//		該当する物理ページが存在しない場合には、
//		PhysicalFile::DirectAreaManageNode::UnknownIndex
//
//	EXCEPTIONS

DirectAreaManageNode::Index
DirectAreaManageLeaf::searchPageIndex(
	const void*							LeafPointer_,
	const DirectAreaManageNode::Index	LeafIndex_,
	const PageNum						NumPage_,
	const DirectArea::Size				SearchAreaSize_) const
{
	// 例えば、4 ％の領域を探すのであれば 5 ％以上の空きを、
	// 10 ％の領域を探すのであれば 15 ％以上の空きを、
	// というように、少し余裕のあるページを管理している子ノードを検索する。
	// そのため、検索すべき領域率を 1 ランク上げる。
	AreaRate	searchAreaRate = this->convertToRate(SearchAreaSize_) >> 1;

	DirectAreaManageNode::Index	pageIndex = DirectAreaManageNode::UnknownIndex;

	// 検索すべき領域率よりももっと余裕のあるページの
	// インデックスを格納するための配列
	DirectAreaManageNode::Index	spareIndices[7] =
		{ DirectAreaManageNode::UnknownIndex,
		  DirectAreaManageNode::UnknownIndex,
		  DirectAreaManageNode::UnknownIndex,
		  DirectAreaManageNode::UnknownIndex,
		  DirectAreaManageNode::UnknownIndex,
		  DirectAreaManageNode::UnknownIndex,
		  DirectAreaManageNode::UnknownIndex };
	bool	useSpare = false;

	const AreaRate*	leaf = static_cast<const AreaRate*>(LeafPointer_);

	for (PageNum i = 0; i < NumPage_; i++) {

		AreaRate	pageRate = *(leaf + i);

		if (pageRate == searchAreaRate) {
			pageIndex = i;
			break;
		} else if (pageRate < searchAreaRate) {
			// 検索すべき領域率よりももっと余裕のあるページを管理している
			// リーフノードなので、
			// ストックしていないランクならばストックしておく。
			int	j;
			for (j = 0; pageRate < searchAreaRate; j++) {
				if (pageRate == 0) {
					pageRate = 1;
				} else {
					pageRate <<= 1;
				}
			}
			if (spareIndices[j] == DirectAreaManageNode::UnknownIndex) {
				useSpare = true;
				spareIndices[j] = i;
			}
		}
	}

	if ((pageIndex == DirectAreaManageNode::UnknownIndex) && useSpare) {

		// わりとぴったりめのページは見つからなかったが、
		// 余裕のあるページを管理していたらしいので、
		// そのページのインデックスを検索結果として返してあげる。

		for (int i = 0; i < 7; i++) {

			if (spareIndices[i] != DirectAreaManageNode::UnknownIndex) {
				pageIndex = spareIndices[i];
				break;
			}
		}
	}

	return
		(pageIndex == DirectAreaManageNode::UnknownIndex) ?
		pageIndex : this->m_MaxPage * LeafIndex_ + pageIndex;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::getNumManagePage --
//		リーフノード内で管理している物理ページ数を返す
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::PageNum						NumTreePage_
//		[IN]		木構造で管理している物理ページ数
//					※ 物理ファイル内に存在する物理ページ数から 1 を引いた値
//	const PhysicalFile::PageNum						MaxPage_
//		[IN]		ひとつのリーフノードで管理可能な物理ページ数
//	const PhysicalFile::DirectAreaManageNode::Index	LeafIndex_
//		[IN]		リーフノードインデックス
//					（物理ファイル内でのリーフノードの通し番号）
//
//	RETURN
//	PhysicalFile::PageNum
//		リーフノード内で管理している物理ページ数
//
//	EXCEPTIONS

// static
PageNum
DirectAreaManageLeaf::getNumManagePage(
	const PageNum						NumTreePage_,
	const PageNum						MaxPage_,
	const DirectAreaManageNode::Index	LeafIndex_)
{
	PageNum	numManagePage = 0;

	// 物理ファイル内で最後のリーフノードか？
	PageNum	numLeaf = (NumTreePage_ - 1) / MaxPage_ + 1;
	bool	isLastLeaf = (LeafIndex_ == (numLeaf - 1));

	if (isLastLeaf) {

		// 最後のリーフノード

		numManagePage = (NumTreePage_ - 1) % MaxPage_ + 1;

	} else {

		// 最後ではないリーフノード
		numManagePage = MaxPage_;
	}

	; _SYDNEY_ASSERT(numManagePage > 0);

	return numManagePage;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::convertToRate --
//		物理エリアサイズを領域率ビットマップに変換する
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea::Size	AreaSize_
//		[IN]		物理エリアサイズ [byte]
//
//	RETURN
//	PhysicalFile::DirectAreaManageLeaf::AreaRate
//		領域率を表すビットマップ
//
//	EXCEPTIONS

DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::convertToRate(const DirectArea::Size	AreaSize_) const
{
	unsigned char	areaPercentage =
		static_cast<unsigned char>(
			static_cast<double>(AreaSize_) / this->m_DirectAreaMaxSize * 100);
	return ToAreaRate[areaPercentage];
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::get --
//		物理ページの空き領域率を返す
//
//	NOTES
//
//	ARGUMENTS
//	const void*										LeafPointer_
//		[IN]		リーフノードへのポインタ
//	const PhysicalFile::DirectAreaManageNode::Index	PageIndex_
//		[IN]		リーフノード内での物理ページのインデックス
//
//	RETURN
//	PhysicalFile::DirectAreaManageLeaf::AreaRate
//		物理ページの空き領域率
//
//	EXCEPTIONS

// static
DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::get(const void*						LeafPointer_,
						  const DirectAreaManageNode::Index	PageIndex_)
{
	const AreaRate*	pageFreeAreaRate =
		static_cast<const AreaRate*>(LeafPointer_) + PageIndex_;
	return *pageFreeAreaRate;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::total --
//		リーフノードで管理している物理ページの空き領域率を集計する
//
//	NOTES
//	リーフノードで管理している複数の物理ページの空き領域率を集計し、
//	リーフノードの親ノードへ記録するための空き領域率を表すビットマップを返す。
//
//	ARGUMENTS
//	const void*					LeafPointer_
//		[IN]		リーフノードへのポインタ
//	const PhysicalFile::PageNum	NumManagePage_
//		[IN]		リーフノードで管理している物理ページ数
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::AreaRate
//		親ノードへ記録するための、
//		リーフノードに記録されている複数の物理ページの空き領域率
//		※ リーフノードの親ノードに記録する領域率なので、
//		　 PhysicalFile::DirectAreaManageLeaf::AreaRate ではない。
//		　                               ~~~~
//
//	EXCEPTIONS

// static
DirectAreaManageNode::AreaRate
DirectAreaManageLeaf::total(const void*		LeafPointer_,
							const PageNum	NumManagePage_)
{
	DirectAreaManageNode::AreaRate	totalRate = DirectAreaManageNode::Rate_0_4;

	const AreaRate*	leafPointer = static_cast<const AreaRate*>(LeafPointer_);

	for (PageNum i = 0; i < NumManagePage_ && totalRate != 0xFF; i++) {

		AreaRate	rate = *(leafPointer + i);
		if (rate == Rate_80_100)
			totalRate |= DirectAreaManageNode::Rate_80_100;
		else if (rate == Rate_60_79)
			totalRate |= DirectAreaManageNode::Rate_60_79;
		else if (rate == Rate_40_59)
			totalRate |= DirectAreaManageNode::Rate_40_59;
		else if (rate == Rate_30_39)
			totalRate |= DirectAreaManageNode::Rate_30_39;
		else if (rate == Rate_20_29)
			totalRate |= DirectAreaManageNode::Rate_20_29;
		else if (rate == Rate_15_19)
			totalRate |= DirectAreaManageNode::Rate_15_19;
		else if (rate == Rate_10_14)
			totalRate |= DirectAreaManageNode::Rate_10_14;
		else if (rate == Rate_5_9)
			totalRate |= DirectAreaManageNode::Rate_5_9;
	}

	return totalRate;
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::update -- リーフノードを更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*												LeafPointer_
//		[IN]		リーフノードへのポインタ
//	const PhysicalFile::PageNum							NumManagePage_
//		[IN]		リーフノードで管理している物理ページ数
//	const PhysicalFile::DirectAreaManageNode::Index		PageIndex_
//		[IN]		リーフノード内での物理ページのインデックス
//	const PhysicalFile::DirectAreaManageLeaf::AreaRate	PageFreeAreaRate_
//		[IN]		物理ページ内の空き領域率を表すビットマップ
//
//	RETURN
//	PhysicalFile::DirectAreaManageNode::AreaRate
//		親ノードに記録すべき、更新後のリーフノードの領域率を表すビットマップ
//		※ リーフノードの親ノードに記録する領域率なので、
//		　 PhysicalFile::DirectAreaManageLeaf::AreaRate ではない。
//		　                               ~~~~
//	EXCEPTIONS

// static
DirectAreaManageNode::AreaRate
DirectAreaManageLeaf::update(
	void*								LeafPointer_,
	const PageNum						NumManagePage_,
	const DirectAreaManageNode::Index	PageIndex_,
	const AreaRate						PageFreeAreaRate_)
{
	; _SYDNEY_ASSERT(LeafPointer_ != 0);

	update(LeafPointer_, PageIndex_, PageFreeAreaRate_);

	return total(LeafPointer_, NumManagePage_);
}

//	FUNCTION public
//	PhysicalFile::DirectAreaManageLeaf::update -- リーフノードを更新する
//
//	NOTES
//
//	ARGUMENTS
//	void*												LeafPointer_
//		[IN]		リーフノードへのポインタ
//	const PhysicalFile::DirectAreaManageNode::Index		PageIndex_
//		[IN]		リーフノード内での物理ページのインデックス
//	const PhysicalFile::DirectAreaManageLeaf::AreaRate	PageFreeAreaRate_
//		[IN]		物理ページ内の空き領域率を表すビットマップ
//
//	RETURN
//
//	EXCEPTIONS

// static
void
DirectAreaManageLeaf::update(
	void*								LeafPointer_,
	const DirectAreaManageNode::Index	PageIndex_,
	const AreaRate						PageFreeAreaRate_)
{
	; _SYDNEY_ASSERT(LeafPointer_ != 0);

	AreaRate*	pageFreeAreaRate =
		static_cast<AreaRate*>(LeafPointer_) + PageIndex_;
	*pageFreeAreaRate = PageFreeAreaRate_;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::DirectAreaManageLeaf クラスの private 定数など
//
///////////////////////////////////////////////////////////////////////////////

//	CONST private
//	PhysicalFile::DirectAreaManageLeaf::Rate_**_** -- 領域率を表す値
//
//	NOTES
//	下記参照

//	80 ％以上の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_80_100 = 0x00;

//	60 〜 79 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_60_79 = 0x01;

//	40 〜 59 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_40_59 = 0x02;

//	30 〜 39 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_30_39 = 0x04;

//	20 〜 29 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_20_29 = 0x08;

//	15 〜 19 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_15_19 = 0x10;

//	10 〜 14 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_10_14 = 0x20;

//	5 〜 9 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_5_9 = 0x40;

//	0 〜 4 ％の領域
// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::Rate_0_4 = 0x80;

//	CONST private
//	PhysicalFile::DirectAreaManageLeaf::ToAreaRate --
//		物理エリアの物理ページ内での 100 分率から
//		領域率 DirectAreaManageLeaf::AreaRate への変換表
//
//	NOTES

// static
const DirectAreaManageLeaf::AreaRate
DirectAreaManageLeaf::ToAreaRate[101] = {
	DirectAreaManageLeaf::Rate_0_4,		//   0 [%]
	DirectAreaManageLeaf::Rate_0_4,		//   1
	DirectAreaManageLeaf::Rate_0_4,		//   2
	DirectAreaManageLeaf::Rate_0_4,		//   3
	DirectAreaManageLeaf::Rate_0_4,		//   4
	DirectAreaManageLeaf::Rate_5_9,		//   5
	DirectAreaManageLeaf::Rate_5_9,		//   6
	DirectAreaManageLeaf::Rate_5_9,		//   7
	DirectAreaManageLeaf::Rate_5_9,		//   8
	DirectAreaManageLeaf::Rate_5_9,		//   9
	DirectAreaManageLeaf::Rate_10_14,	//  10
	DirectAreaManageLeaf::Rate_10_14,	//  11
	DirectAreaManageLeaf::Rate_10_14,	//  12
	DirectAreaManageLeaf::Rate_10_14,	//  13
	DirectAreaManageLeaf::Rate_10_14,	//  14
	DirectAreaManageLeaf::Rate_15_19,	//  15
	DirectAreaManageLeaf::Rate_15_19,	//  16
	DirectAreaManageLeaf::Rate_15_19,	//  17
	DirectAreaManageLeaf::Rate_15_19,	//  18
	DirectAreaManageLeaf::Rate_15_19,	//  19
	DirectAreaManageLeaf::Rate_20_29,	//  20
	DirectAreaManageLeaf::Rate_20_29,	//  21
	DirectAreaManageLeaf::Rate_20_29,	//  22
	DirectAreaManageLeaf::Rate_20_29,	//  23
	DirectAreaManageLeaf::Rate_20_29,	//  24
	DirectAreaManageLeaf::Rate_20_29,	//  25
	DirectAreaManageLeaf::Rate_20_29,	//  26
	DirectAreaManageLeaf::Rate_20_29,	//  27
	DirectAreaManageLeaf::Rate_20_29,	//  28
	DirectAreaManageLeaf::Rate_20_29,	//  29
	DirectAreaManageLeaf::Rate_30_39,	//  30
	DirectAreaManageLeaf::Rate_30_39,	//  31
	DirectAreaManageLeaf::Rate_30_39,	//  32
	DirectAreaManageLeaf::Rate_30_39,	//  33
	DirectAreaManageLeaf::Rate_30_39,	//  34
	DirectAreaManageLeaf::Rate_30_39,	//  35
	DirectAreaManageLeaf::Rate_30_39,	//  36
	DirectAreaManageLeaf::Rate_30_39,	//  37
	DirectAreaManageLeaf::Rate_30_39,	//  38
	DirectAreaManageLeaf::Rate_30_39,	//  39
	DirectAreaManageLeaf::Rate_40_59,	//  40
	DirectAreaManageLeaf::Rate_40_59,	//  41
	DirectAreaManageLeaf::Rate_40_59,	//  42
	DirectAreaManageLeaf::Rate_40_59,	//  43
	DirectAreaManageLeaf::Rate_40_59,	//  44
	DirectAreaManageLeaf::Rate_40_59,	//  45
	DirectAreaManageLeaf::Rate_40_59,	//  46
	DirectAreaManageLeaf::Rate_40_59,	//  47
	DirectAreaManageLeaf::Rate_40_59,	//  48
	DirectAreaManageLeaf::Rate_40_59,	//  49
	DirectAreaManageLeaf::Rate_40_59,	//  50
	DirectAreaManageLeaf::Rate_40_59,	//  51
	DirectAreaManageLeaf::Rate_40_59,	//  52
	DirectAreaManageLeaf::Rate_40_59,	//  53
	DirectAreaManageLeaf::Rate_40_59,	//  54
	DirectAreaManageLeaf::Rate_40_59,	//  55
	DirectAreaManageLeaf::Rate_40_59,	//  56
	DirectAreaManageLeaf::Rate_40_59,	//  57
	DirectAreaManageLeaf::Rate_40_59,	//  58
	DirectAreaManageLeaf::Rate_40_59,	//  59
	DirectAreaManageLeaf::Rate_60_79,	//  60
	DirectAreaManageLeaf::Rate_60_79,	//  61
	DirectAreaManageLeaf::Rate_60_79,	//  62
	DirectAreaManageLeaf::Rate_60_79,	//  63
	DirectAreaManageLeaf::Rate_60_79,	//  64
	DirectAreaManageLeaf::Rate_60_79,	//  65
	DirectAreaManageLeaf::Rate_60_79,	//  66
	DirectAreaManageLeaf::Rate_60_79,	//  67
	DirectAreaManageLeaf::Rate_60_79,	//  68
	DirectAreaManageLeaf::Rate_60_79,	//  69
	DirectAreaManageLeaf::Rate_60_79,	//  70
	DirectAreaManageLeaf::Rate_60_79,	//  71
	DirectAreaManageLeaf::Rate_60_79,	//  72
	DirectAreaManageLeaf::Rate_60_79,	//  73
	DirectAreaManageLeaf::Rate_60_79,	//  74
	DirectAreaManageLeaf::Rate_60_79,	//  75
	DirectAreaManageLeaf::Rate_60_79,	//  76
	DirectAreaManageLeaf::Rate_60_79,	//  77
	DirectAreaManageLeaf::Rate_60_79,	//  78
	DirectAreaManageLeaf::Rate_60_79,	//  79
	DirectAreaManageLeaf::Rate_80_100,	//  80
	DirectAreaManageLeaf::Rate_80_100,	//  81
	DirectAreaManageLeaf::Rate_80_100,	//  82
	DirectAreaManageLeaf::Rate_80_100,	//  83
	DirectAreaManageLeaf::Rate_80_100,	//  84
	DirectAreaManageLeaf::Rate_80_100,	//  85
	DirectAreaManageLeaf::Rate_80_100,	//  86
	DirectAreaManageLeaf::Rate_80_100,	//  87
	DirectAreaManageLeaf::Rate_80_100,	//  88
	DirectAreaManageLeaf::Rate_80_100,	//  89
	DirectAreaManageLeaf::Rate_80_100,	//  90
	DirectAreaManageLeaf::Rate_80_100,	//  91
	DirectAreaManageLeaf::Rate_80_100,	//  92
	DirectAreaManageLeaf::Rate_80_100,	//  93
	DirectAreaManageLeaf::Rate_80_100,	//  94
	DirectAreaManageLeaf::Rate_80_100,	//  95
	DirectAreaManageLeaf::Rate_80_100,	//  96
	DirectAreaManageLeaf::Rate_80_100,	//  97
	DirectAreaManageLeaf::Rate_80_100,	//  98
	DirectAreaManageLeaf::Rate_80_100,	//  99
	DirectAreaManageLeaf::Rate_80_100	// 100
};

//
//	Copyright (c) 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
