// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaManageFile.cpp --
//		空き領域管理機能付き物理ファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"

#include "Exception/BadArgument.h"

#include "Common/Assert.h"

#include "Version/File.h"

#include "PhysicalFile/AreaManageFile.h"
#include "PhysicalFile/AreaManagePage.h"
#include "PhysicalFile/Message_DiscordManagePageNum.h"
#include "PhysicalFile/Message_DiscordUsePageNum.h"
#include "PhysicalFile/Message_DiscordManagePageNumInTable.h"
#include "PhysicalFile/Message_DiscordUnuseAreaRate.h"
#include "PhysicalFile/Message_DiscordFreeAreaRate.h"
#include "PhysicalFile/Message_DiscordPageArray.h"

#include "Os/AutoCriticalSection.h"
#ifdef OBSOLETE
#include <algorithm>
#endif // OBSOLETE

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageTableクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::getVersionPageID --
//		物理ページを管理している空き領域管理表の
//		バージョンページ識別子を返す
//
//	NOTES
//	引数PageID_が示す物理ページを管理している
//	空き領域管理表のバージョンページ識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//	const PhysicalFile::PageNum	PagePerManageTable_
//		1つの空き領域管理表が管理可能な物理ページ数
//
//	RETURN
//	Version::Page::ID
//		物理ページを管理している空き領域管理表の
//		バージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
// static
Version::Page::ID
AreaManageTable::getVersionPageID(const PageID	PageID_,
								  const PageNum	PagePerManageTable_)
{
	// 物理ファイル先頭から、引数PageID_で示される物理ページまでに
	// 空き領域管理表がいくつ存在するかを求める
	PageNum	manageTableNum =
		PageID_ / PagePerManageTable_ + 1;

	// 空き領域管理表のバージョンページ識別子を求めて返す
	return (manageTableNum - 1) * PagePerManageTable_ + manageTableNum;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageTable::Headerクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::PageArrayElementNum --
//		物理ページ数配列の要素数
//
//	NOTES
//	空き領域管理表ヘッダ内の物理ページ数配列の要素数。
//	未使用領域率別物理ページ数配列、空き領域率別物理ページ数配列
//	それぞれが、この要素数分、要素をもつ。
//
// static
const unsigned int
AreaManageTable::Header::PageArrayElementNum = 8;

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::ToPageArrayIndex --
//		領域率から物理ページ数配列のインデックスへの変換表
//
//	NOTES
//	領域率から物理ページ数配列のインデックスへの変換表。
//
// static
const char
AreaManageTable::Header::ToPageArrayIndex[101] =
{
	0,	//   0 [%]
	0,	//   1
	0,	//   2
	0,	//   3
	0,	//   4
	1,	//   5
	1,	//   6
	1,	//   7
	1,	//   8
	1,	//   9
	2,	//  10
	2,	//  11
	2,	//  12
	2,	//  13
	2,	//  14
	3,	//  15
	3,	//  16
	3,	//  17
	3,	//  18
	3,	//  19
	4,	//  20
	4,	//  21
	4,	//  22
	4,	//  23
	4,	//  24
	4,	//  25
	4,	//  26
	4,	//  27
	4,	//  28
	4,	//  29
	4,	//  30
	4,	//  31
	4,	//  32
	4,	//  33
	4,	//  34
	4,	//  35
	4,	//  36
	4,	//  37
	4,	//  38
	4,	//  39
	5,	//  40
	5,	//  41
	5,	//  42
	5,	//  43
	5,	//  44
	5,	//  45
	5,	//  46
	5,	//  47
	5,	//  48
	5,	//  49
	5,	//  50
	5,	//  51
	5,	//  52
	5,	//  53
	5,	//  54
	5,	//  55
	5,	//  56
	5,	//  57
	5,	//  58
	5,	//  59
	6,	//  60
	6,	//  61
	6,	//  62
	6,	//  63
	6,	//  64
	6,	//  65
	6,	//  66
	6,	//  67
	6,	//  68
	6,	//  69
	6,	//  70
	6,	//  71
	6,	//  72
	6,	//  73
	6,	//  74
	6,	//  75
	6,	//  76
	6,	//  77
	6,	//  78
	6,	//  79
	7,	//  80
	7,	//  81
	7,	//  82
	7,	//  83
	7,	//  84
	7,	//  85
	7,	//  86
	7,	//  87
	7,	//  88
	7,	//  89
	7,	//  90
	7,	//  91
	7,	//  92
	7,	//  93
	7,	//  94
	7,	//  95
	7,	//  96
	7,	//  97
	7,	//  98
	7,	//  99
	7	// 100
};

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::SmallSize --
//		空き領域管理表ヘッダサイズ
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダのサイズ。[byte]
//
// static
const PageSize
AreaManageTable::Header::SmallSize =
	sizeof(ShortPageNum) *
	(2 + (AreaManageTable::Header::PageArrayElementNum << 1));
//   ~ 使用中／未使用物理ページ数の分                  ~~~~
//                                              2つのページ数配列のため

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::LargeSize --
//		空き領域管理表ヘッダサイズ
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダのサイズ。 [byte]
//
// static
const PageSize
AreaManageTable::Header::LargeSize =
	sizeof(PageNum) *
	(2 + (AreaManageTable::Header::PageArrayElementNum << 1));

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::SmallUnuseAreaRatePageArrayOffset --
//		未使用領域率別の物理ページ数配列開始位置
//
//	NOTES
//	各項目を2バイトで記録するタイプの空き領域管理表ヘッダの
//	未使用領域率別の物理ページ数配列開始位置。[byte]
//
// static
const PageOffset
AreaManageTable::Header::SmallUnuseAreaRatePageArrayOffset =
	sizeof(ShortPageNum) << 1;
//                               ~~~~ 「掛ける２」

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::LargeUnuseAreaRatePageArrayOffset --
//		未使用領域率別の物理ページ数配列開始位置
//
//	NOTES
//	各項目を4バイトで記録するタイプの空き領域管理表ヘッダの
//	未使用領域率別の物理ページ数配列開始位置。[byte]
//
// static
const PageOffset
AreaManageTable::Header::LargeUnuseAreaRatePageArrayOffset =
	sizeof(PageNum) << 1;

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::SmallUnuseAreaRatePageArrayOffset --
//		空き領域率別の物理ページ数配列開始位置
//
//	NOTES
//	各項目を2バイトで記録するタイプの空き領域管理表ヘッダの
//	空き領域率別の物理ページ数配列開始位置。[byte]
//
// static
const PageOffset
AreaManageTable::Header::SmallFreeAreaRatePageArrayOffset =
	AreaManageTable::Header::SmallUnuseAreaRatePageArrayOffset +
	sizeof(ShortPageNum) * AreaManageTable::Header::PageArrayElementNum;

//
//	CONST public
//	PhysicalFile::AreaManageTable::Header::LargeUnuseAreaRatePageArrayOffset --
//		空き領域率別の物理ページ数配列開始位置
//
//	NOTES
//	各項目を4バイトで記録するタイプの空き領域管理表ヘッダの
//	空き領域率別の物理ページ数配列開始位置。[byte]
//
// static
const PageOffset
AreaManageTable::Header::LargeFreeAreaRatePageArrayOffset =
	AreaManageTable::Header::LargeUnuseAreaRatePageArrayOffset +
	sizeof(PageNum) * AreaManageTable::Header::PageArrayElementNum;

namespace
{

namespace _SmallAreaManageTableHeader
{

//
//	FUNCTION
//	_SmallAreaManageTableHeader::getUsedPageNum --
//		使用中の物理ページ数を返す
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」を読み込み、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		使用中の物理ページ数
//
//	EXCEPTIONS
//	なし
//
PageNum
getUsedPageNum(const void*	HeaderTop_)
{
	const ShortPageNum*	pageNumReadPos =
		static_cast<const ShortPageNum*>(HeaderTop_);

	return *pageNumReadPos;
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::getUnusePageNum --
//		未使用の物理ページ数を返す
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「未使用の物理ページ数」を読み込み、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		未使用の物理ページ数
//
//	EXCEPTIONS
//	なし
//
PageNum
getUnusePageNum(const void*	HeaderTop_)
{
	const ShortPageNum*	pageNumReadPos =
		static_cast<const ShortPageNum*>(HeaderTop_);

	pageNumReadPos++;

	return *pageNumReadPos;
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader:: --
//		未使用領域率別／空き領域率別いずれかの
//		物理ページ数配列の開始位置を返す
//
//	NOTES
//	各項目を2バイトで記録するタイプの空き領域管理表ヘッダ内の
//	未使用領域率別／空き領域率別いずれかの
//	物理ページ数配列の開始位置を返す。
//
//	ARGUMENTS
//	const bool	ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//
//	RETURN
//	PhysicalFile::PageOffset
//		未使用領域率別／空き領域率別いずれかの物理ページ数配列の開始位置
//
//	EXCEPTIONS
//	なし
//
PageOffset
getPageArrayOffset(const bool	ByUnuseAreaRate_)
{
	return
		ByUnuseAreaRate_ ?
		AreaManageTable::Header::SmallUnuseAreaRatePageArrayOffset :
		AreaManageTable::Header::SmallFreeAreaRatePageArrayOffset;
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::existManagePage --
//		指定未使用領域率／空き領域率以上の領域をもつ物理ページを
//		管理しているかをチェックする
//
//	NOTES
//	ヘッダの各項目を2バイトで記録するタイプの空き領域管理表で
//	指定未使用領域率／空き領域率以上の領域をもつ物理ページを
//	管理しているかをチェックし、呼び出し側に知らせる。
//
//	ARGUMENTS
//	const void*			HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const unsigned int	AreaRateValue_
//		領域率 [%]
//	const bool			ByUnuseAreaRate_
//		引数AreaRateValue_を未使用領域率、空き領域率
//		どちらと解釈するか
//			true  : 引数AreaRateValue_を未使用領域率と解釈
//			        （未使用領域率でチェックする）
//			false : 引数AreaRateValue_を空き領域率と解釈
//					（空き領域率でチェックする）
//
//	RETURN
//	bool
//		指定未使用領域率／空き領域率以上の領域をもつ物理ページを
//		管理しているかどうか
//			true  : 管理している
//			false : 管理していない
//
//	EXCEPTIONS
//	なし
//
bool
existManagePage(const void*			HeaderTop_,
				const unsigned int	AreaRateValue_,
				const bool			ByUnuseAreaRate_)
{
	if (_SmallAreaManageTableHeader::getUsedPageNum(HeaderTop_) == 0)
	{
		return false;
	}

	const char*	headerPointer = static_cast<const char*>(HeaderTop_);

	headerPointer +=
		_SmallAreaManageTableHeader::getPageArrayOffset(ByUnuseAreaRate_);

	const ShortPageNum*	pageArrayPointer =
		syd_reinterpret_cast<const ShortPageNum*>(headerPointer);

	int	arrayIndex =
		AreaManageTable::Header::ToPageArrayIndex[AreaRateValue_] + 1;

	for (;
		 arrayIndex < AreaManageTable::Header::PageArrayElementNum;
		 arrayIndex++)
	{
		if (*(pageArrayPointer + arrayIndex) != 0)
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::updateUsedPageNum --
//		使用中の物理ページ数を更新する
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
updateUsedPageNum(void*		HeaderTop_,
				  const int	AddNum_)
{
	if (AddNum_ != 0)
	{
		ShortPageNum*	pageNumWritePos =
			static_cast<ShortPageNum*>(HeaderTop_);

#ifdef DEBUG

		if (AddNum_ < 0)
		{
			; _SYDNEY_ASSERT(static_cast<unsigned int>(-AddNum_) <=
							 *pageNumWritePos);
		}

#endif // DEBUG

		*pageNumWritePos += AddNum_;
	}
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::updateUnusePageNum --
//		未使用の物理ページ数を更新する
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「未使用の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
updateUnusePageNum(void*		HeaderTop_,
				   const int	AddNum_)
{
	if (AddNum_ != 0)
	{
		ShortPageNum*	pageNumWritePos =
			static_cast<ShortPageNum*>(HeaderTop_);

		pageNumWritePos++;

#ifdef DEBUG

		if (AddNum_ < 0)
		{
			; _SYDNEY_ASSERT(static_cast<unsigned int>(-AddNum_) <=
							 *pageNumWritePos);
		}

#endif // DEBUG

		*pageNumWritePos += AddNum_;
	}
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::updatePageArrayElement --
//		未使用領域率別／空き領域率別の物理ページ数配列の1要素を更新する
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別／空き領域率別の物理ページ数配列の
//	1要素を更新する。
//
//	ARGUMENTS
//	void*				HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const bool			ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	const unsigned int	AreaRateValue_
//		更新する要素に対応する領域率 [%]
//	const bool			Increment_
//		インクリメントするのかデクリメントするのか
//			true  : インクリメント
//			false : デクリメント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
updatePageArrayElement(void*				HeaderTop_,
					   const bool			ByUnuseAreaRate_,
					   const unsigned int	AreaRateValue_,
					   const bool			Increment_)
{
	ShortPageNum*	pageNumWritePos =
		static_cast<ShortPageNum*>(HeaderTop_);

	pageNumWritePos += 2;

	if (ByUnuseAreaRate_ == false)
	{
		pageNumWritePos +=
			AreaManageTable::Header::PageArrayElementNum;
	}

	int	arrayIndex =
		AreaManageTable::Header::ToPageArrayIndex[AreaRateValue_];

	pageNumWritePos += arrayIndex;

	if (Increment_)
	{
		(*pageNumWritePos)++;
	}
	else
	{
		; _SYDNEY_ASSERT(*pageNumWritePos > 0);

		(*pageNumWritePos)--;
	}
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::overwritePageArrayElement --
//		未使用領域率別／空き領域率別の物理ページ数配列の1要素を
//		上書きする。
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別／空き領域率別の物理ページ数配列の
//	1要素を上書きする。
//
//	ARGUMENTS
//	void*						HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const bool					ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	const unsigned int			ArrayIndex_
//		更新する要素のインデックス
//	const PhysicalFile::PageNum	PageNum_
//		ページ数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwritePageArrayElement(void*					HeaderTop_,
						  const bool			ByUnuseAreaRate_,
						  const unsigned int	ArrayIndex_,
						  const PageNum			PageNum_)
{
	ShortPageNum*	pageNumWritePos =
		static_cast<ShortPageNum*>(HeaderTop_);

	pageNumWritePos += 2;

	if (ByUnuseAreaRate_ == false)
	{
		pageNumWritePos +=
			AreaManageTable::Header::PageArrayElementNum;
	}

	pageNumWritePos += ArrayIndex_;

	*pageNumWritePos = static_cast<ShortPageNum>(PageNum_);
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::fetchOutPageNum --
//		使用中の物理ページ数と未使用の物理ページ数を取り出す
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	PageNum&	UsedPageNum_
//		使用中の物理ページ数への参照
//	PageNum&	UnusePageNum_
//		未使用の物理ページ数への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutPageNum(const void*	HeaderTop_,
				PageNum&	UsedPageNum_,
				PageNum&	UnusePageNum_)
{
	const ShortPageNum*	pageNumReadPos =
		static_cast<const ShortPageNum*>(HeaderTop_);

	UsedPageNum_ = *pageNumReadPos++;

	UnusePageNum_ = *pageNumReadPos;
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::fetchOutPageArrayBoth --
//		未使用領域率別・空き領域率別2つの物理ページ数配列を取り出す
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別・空き領域率別2つの物理ページ数配列を取り出す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	PageNum*	PageNumByUnuseAreaRate_
//		未使用領域率別物理ページ数配列読み込み先バッファへのポインタ
//	PageNum*	PageNumByFreeAreaRate_
//		空き領域率別物理ページ数配列読み込み先バッファへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutPageArrayBoth(const void*	HeaderTop_,
					  PageNum*		PageNumByUnuseAreaRate_,
					  PageNum*		PageNumByFreeAreaRate_)
{
	const ShortPageNum*	pageNumReadPos =
		static_cast<const ShortPageNum*>(HeaderTop_);

	pageNumReadPos += 2;

	unsigned int	i;
	for (i = 0; i < AreaManageTable::Header::PageArrayElementNum; i++)
	{
		*(PageNumByUnuseAreaRate_ + i) = *pageNumReadPos++;
	}

	for (i = 0; i < AreaManageTable::Header::PageArrayElementNum; i++)
	{
		*(PageNumByFreeAreaRate_ + i) = *pageNumReadPos++;
	}
}

//
//	FUNCTION
//	_SmallAreaManageTableHeader::fetchOutPageArray --
//		未使用領域率別／空き領域率別いずれかの物理ページ数配列を取り出す
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別／空き領域率別いずれかの
//	物理ページ数配列を取り出す。
//
//	ARGUMENTS
//	const void*	HeaderTop_,
//		空き領域管理表ヘッダ先頭へのポインタ
//	const bool	ByUnuseAreaRate_
//		未使用領域率別、空き領域率別
//		いずれの物理ページ数配列を取り出すのか
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	PageNum*	PageArray_
//		物理ページ数配列読み込み先バッファへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutPageArray(const void*	HeaderTop_,
				  const bool	ByUnuseAreaRate_,
				  PageNum*		PageArray_)
{
	const ShortPageNum*	pageNumReadPos =
		static_cast<const ShortPageNum*>(HeaderTop_);

	pageNumReadPos += 2;

	if (ByUnuseAreaRate_ == false)
	{
		pageNumReadPos +=
			AreaManageTable::Header::PageArrayElementNum;
	}

	unsigned int	i;
	for (i = 0; i < AreaManageTable::Header::PageArrayElementNum; i++)
	{
		*(PageArray_ + i) = *pageNumReadPos++;
	}
}

} // end of namespace _SmallAreaManageTableHeader

namespace _LargeAreaManageTableHeader
{

//
//	FUNCTION
//	_LargeAreaManageTableHeader::getUsedPageNum --
//		使用中の物理ページ数を返す
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」を読み込み、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		使用中の物理ページ数
//
//	EXCEPTIONS
//	なし
//
PageNum
getUsedPageNum(const void*	HeaderTop_)
{
	const PageNum*	pageNumReadPos =
		static_cast<const PageNum*>(HeaderTop_);

	return *pageNumReadPos;
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::getUnusePageNum --
//		未使用の物理ページ数を返す
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「未使用の物理ページ数」を読み込み、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageNum
//		未使用の物理ページ数
//
//	EXCEPTIONS
//	なし
//
PageNum
getUnusePageNum(const void*	HeaderTop_)
{
	const PageNum*	pageNumReadPos =
		static_cast<const PageNum*>(HeaderTop_);

	pageNumReadPos++;

	return *pageNumReadPos;
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::getPageArrayOffset --
//		未使用領域率別／空き領域率別いずれかの
//		物理ページ数配列の開始位置を返す
//
//	NOTES
//	各項目を4バイトで記録するタイプの空き領域管理表ヘッダ内の
//	未使用領域率別／空き領域率別いずれかの
//	物理ページ数配列の開始位置を返す。
//
//	ARGUMENTS
//	const bool	ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//
//	RETURN
//	PhysicalFile::PageOffset
//		未使用領域率別／空き領域率別いずれかの物理ページ数配列の開始位置
//
//	EXCEPTIONS
//	なし
//
PageOffset
getPageArrayOffset(const bool	ByUnuseAreaRate_)
{
	return
		ByUnuseAreaRate_ ?
		AreaManageTable::Header::LargeUnuseAreaRatePageArrayOffset :
		AreaManageTable::Header::LargeFreeAreaRatePageArrayOffset;
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::existManagePage --
//		指定未使用領域率／空き領域率以上の領域をもつ物理ページを
//		管理しているかをチェックする
//
//	NOTES
//	ヘッダの各項目を4バイトで記録するタイプの空き領域管理表で
//	指定未使用領域率／空き領域率以上の領域をもつ物理ページを
//	管理しているかをチェックし、呼び出し側に知らせる。
//
//	ARGUMENTS
//	const void*			HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const unsigned int	AreaRateValue_
//		領域率 [%]
//	const bool			ByUnuseAreaRate_
//		引数AreaRateValue_を未使用領域率、空き領域率
//		どちらと解釈するか
//			true  : 引数AreaRateValue_を未使用領域率と解釈
//			        （未使用領域率でチェックする）
//			false : 引数AreaRateValue_を空き領域率と解釈
//					（空き領域率でチェックする）
//
//	RETURN
//	bool
//		指定未使用領域率／空き領域率以上の領域をもつ物理ページを
//		管理しているかどうか
//			true  : 管理している
//			false : 管理していない
//
//	EXCEPTIONS
//	なし
//
bool
existManagePage(const void*			HeaderTop_,
				const unsigned int	AreaRateValue_,
				const bool			ByUnuseAreaRate_)
{
	if (_LargeAreaManageTableHeader::getUsedPageNum(HeaderTop_) == 0)
	{
		return false;
	}

	const char*	headerPointer = static_cast<const char*>(HeaderTop_);

	headerPointer +=
		_LargeAreaManageTableHeader::getPageArrayOffset(
			ByUnuseAreaRate_);

	const PageNum*	pageArrayPointer =
		syd_reinterpret_cast<const PageNum*>(headerPointer);

	int	arrayIndex =
		AreaManageTable::Header::ToPageArrayIndex[AreaRateValue_] + 1;

	for (;
		 arrayIndex < AreaManageTable::Header::PageArrayElementNum;
		 arrayIndex++)
	{
		if (*(pageArrayPointer + arrayIndex) != 0)
		{
			return true;
		}
	}

	return false;
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::updateUsedPageNum --
//		使用中の物理ページ数を更新する
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
updateUsedPageNum(void*		HeaderTop_,
				  const int	AddNum_)
{
	if (AddNum_ != 0)
	{
		PageNum*	pageNumWritePos =
			static_cast<PageNum*>(HeaderTop_);

#ifdef DEBUG

		if (AddNum_ < 0)
		{
			; _SYDNEY_ASSERT(static_cast<unsigned int>(-AddNum_) <=
							 *pageNumWritePos);
		}

#endif // DEBUG

		*pageNumWritePos += AddNum_;
	}
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::updateUnusePageNum --
//		未使用の物理ページ数を更新する
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「未使用の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
updateUnusePageNum(void*		HeaderTop_,
				   const int	AddNum_)
{
	if (AddNum_ != 0)
	{
		PageNum*	pageNumWritePos =
			static_cast<PageNum*>(HeaderTop_);

		pageNumWritePos++;

#ifdef DEBUG

		if (AddNum_ < 0)
		{
			; _SYDNEY_ASSERT(static_cast<unsigned int>(-AddNum_) <=
							 *pageNumWritePos);
		}

#endif // DEBUG

		*pageNumWritePos += AddNum_;
	}
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::updatePageArrayElement --
//		未使用領域率別／空き領域率別の物理ページ数配列の1要素を更新する
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別／空き領域率別の物理ページ数配列の
//	1要素を更新する。
//
//	ARGUMENTS
//	void*				HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const bool			ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	const unsigned int	AreaRateValue_
//		更新する要素に対応する領域率 [%]
//	const bool			Increment_
//		インクリメントするのかデクリメントするのか
//			true  : インクリメント
//			false : デクリメント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
updatePageArrayElement(void*				HeaderTop_,
					   const bool			ByUnuseAreaRate_,
					   const unsigned int	AreaRateValue_,
					   const bool			Increment_)
{
	PageNum*	pageNumWritePos =
		static_cast<PageNum*>(HeaderTop_);

	pageNumWritePos += 2;

	if (ByUnuseAreaRate_ == false)
	{
		pageNumWritePos +=
			AreaManageTable::Header::PageArrayElementNum;
	}

	int	arrayIndex =
		AreaManageTable::Header::ToPageArrayIndex[AreaRateValue_];

	pageNumWritePos += arrayIndex;

	if (Increment_)
	{
		(*pageNumWritePos)++;
	}
	else
	{
		; _SYDNEY_ASSERT(*pageNumWritePos > 0);

		(*pageNumWritePos)--;
	}
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::overwritePageArrayElement --
//		未使用領域率別／空き領域率別の物理ページ数配列の1要素を
//		上書きする。
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別／空き領域率別の物理ページ数配列の
//	1要素を上書きする。
//
//	ARGUMENTS
//	void*						HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	const bool					ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	const unsigned int			ArrayIndex_
//		更新する要素のインデックス
//	const PhysicalFile::PageNum	PageNum_
//		ページ数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwritePageArrayElement(void*					HeaderTop_,
						  const bool			ByUnuseAreaRate_,
						  const unsigned int	ArrayIndex_,
						  const PageNum			PageNum_)
{
	PageNum*	pageNumWritePos =
		static_cast<PageNum*>(HeaderTop_);

	pageNumWritePos += 2;

	if (ByUnuseAreaRate_ == false)
	{
		pageNumWritePos +=
			AreaManageTable::Header::PageArrayElementNum;
	}

	pageNumWritePos += ArrayIndex_;

	*pageNumWritePos = PageNum_;
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::fetchOutPageNum --
//		使用中の物理ページ数と未使用の物理ページ数を取り出す
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	PageNum&	UsedPageNum_
//		使用中の物理ページ数への参照
//	PageNum&	UnusePageNum_
//		未使用の物理ページ数への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutPageNum(const void*	HeaderTop_,
				PageNum&	UsedPageNum_,
				PageNum&	UnusePageNum_)
{
	const PageNum*	pageNumReadPos =
		static_cast<const PageNum*>(HeaderTop_);

	UsedPageNum_ = *pageNumReadPos++;

	UnusePageNum_ = *pageNumReadPos;
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::fetchOutPageArrayBoth --
//		未使用領域率別・空き領域率別2つの物理ページ数配列を取り出す
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別・空き領域率別2つの物理ページ数配列を取り出す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		空き領域管理表ヘッダ先頭へのポインタ
//	PageNum*	PageNumByUnuseAreaRate_
//		未使用領域率別物理ページ数配列読み込み先バッファへのポインタ
//	PageNum*	PageNumByFreeAreaRate_
//		空き領域率別物理ページ数配列読み込み先バッファへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutPageArrayBoth(const void*	HeaderTop_,
					  PageNum*		PageNumByUnuseAreaRate_,
					  PageNum*		PageNumByFreeAreaRate_)
{
	const PageNum*	pageNumReadPos =
		static_cast<const PageNum*>(HeaderTop_);

	pageNumReadPos += 2;

	unsigned int	i;
	for (i = 0; i < AreaManageTable::Header::PageArrayElementNum; i++)
	{
		*(PageNumByUnuseAreaRate_ + i) = *pageNumReadPos++;
	}

	for (i = 0; i < AreaManageTable::Header::PageArrayElementNum; i++)
	{
		*(PageNumByFreeAreaRate_ + i) = *pageNumReadPos++;
	}
}

//
//	FUNCTION
//	_LargeAreaManageTableHeader::fetchOutPageArray --
//		未使用領域率別／空き領域率別いずれかの物理ページ数配列を取り出す
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	空き領域管理表ヘッダに記録されている
//	未使用領域率別／空き領域率別いずれかの
//	物理ページ数配列を取り出す。
//
//	ARGUMENTS
//	const void*	HeaderTop_,
//		空き領域管理表ヘッダ先頭へのポインタ
//	const bool	ByUnuseAreaRate_
//		未使用領域率別、空き領域率別
//		いずれの物理ページ数配列を取り出すのか
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	PageNum*	PageArray_
//		物理ページ数配列読み込み先バッファへのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutPageArray(const void*	HeaderTop_,
				  const bool	ByUnuseAreaRate_,
				  PageNum*		PageArray_)
{
	const PageNum*	pageNumReadPos =
		static_cast<const PageNum*>(HeaderTop_);

	pageNumReadPos += 2;

	if (ByUnuseAreaRate_ == false)
	{
		pageNumReadPos +=
			AreaManageTable::Header::PageArrayElementNum;
	}

	unsigned int	i;
	for (i = 0; i < AreaManageTable::Header::PageArrayElementNum; i++)
	{
		*(PageArray_ + i) = *pageNumReadPos++;
	}
}

} // end of namespace _LargeAreaManageTableHeader

} // end of global namespace

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageTable::Headerクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Header::Header -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
AreaManageTable::Header::Header()
	: Common::Object(),
	  m_Type(AreaManageTable::Header::UnknownType)
{
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Header::~Header -- デストラクタ
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
AreaManageTable::Header::~Header()
{
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Header::updatePageArray --
//		未使用領域率別／空き領域率別の物理ページ数配列を更新する
//
//	NOTES
//	未使用領域率別の物理ページ数配列または
//	空き領域率別の物理ページ数配列を更新する。
//
//	ARGUMENTS
//	void*				TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const bool			ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列を更新するか
//			true  : 未使用領域率別の物理ページ数配列を更新
//			false : 空き領域率別の物理ページ数配列を更新
//	const unsigned int	BeforeAreaRateValue_
//		更新前の未使用領域率／空き領域率 [%]
//	const unsigned int	AfterAreaRateValue_
//		更新後の未使用領域率／空き領域率 [%]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageTable::Header::updatePageArray(
	void*				TablePointer_,
	const bool			ByUnuseAreaRate_,
	const unsigned int	BeforeAreaRateValue_,
	const unsigned int	AfterAreaRateValue_)
{
	// 更新前の領域率に対応する要素の物理ページ数をデクリメント
	(*this->UpdatePageArrayElement)(TablePointer_,
									ByUnuseAreaRate_,
									BeforeAreaRateValue_,
									false); // デクリメント

	// 更新後の領域率に対応する要素の物理ページ数をインクリメント
	(*this->UpdatePageArrayElement)(TablePointer_,
									ByUnuseAreaRate_,
									AfterAreaRateValue_,
									true); // インクリメント
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Header::setType --
//		空き領域管理表ヘッダタイプを設定する
//
//	NOTES
//	空き領域管理表ヘッダタイプを設定する。
//
//	ARGUMENTS
//	const PhysicalFile::AreaManageTable::Header::Type	Type_
//		空き領域管理表ヘッダタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageTable::Header::setType(
	const AreaManageTable::Header::Type	Type_)
{
	this->m_Type = Type_;

	this->setFunction();
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageTable::Headerクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::AreaManageTable::Header::setFunction -- 
//		アクセスする関数を設定する
//
//	NOTES
//	アクセスする関数を設定する。
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
void
AreaManageTable::Header::setFunction()
{
	; _SYDNEY_ASSERT(this->m_Type != AreaManageTable::Header::UnknownType);

	if (this->m_Type == AreaManageTable::Header::SmallType)
	{
		// 各項目を2バイトで記録するタイプ…

		this->GetUsedPageNum =
			_SmallAreaManageTableHeader::getUsedPageNum;

		this->GetUnusePageNum =
			_SmallAreaManageTableHeader::getUnusePageNum;

		this->GetPageArrayOffset =
			_SmallAreaManageTableHeader::getPageArrayOffset;

		this->ExistManagePage =
			_SmallAreaManageTableHeader::existManagePage;

		this->UpdateUsedPageNum =
			_SmallAreaManageTableHeader::updateUsedPageNum;

		this->UpdateUnusePageNum =
			_SmallAreaManageTableHeader::updateUnusePageNum;

		this->UpdatePageArrayElement =
			_SmallAreaManageTableHeader::updatePageArrayElement;

		this->OverwritePageArrayElement =
			_SmallAreaManageTableHeader::overwritePageArrayElement;

		this->FetchOutPageNum =
			_SmallAreaManageTableHeader::fetchOutPageNum;

		this->FetchOutPageArrayBoth =
			_SmallAreaManageTableHeader::fetchOutPageArrayBoth;

		this->FetchOutPageArray =
			_SmallAreaManageTableHeader::fetchOutPageArray;
	}
	else
	{
		// 各項目を4バイトで記録するタイプ…

		this->GetUsedPageNum =
			_LargeAreaManageTableHeader::getUsedPageNum;

		this->GetUnusePageNum =
			_LargeAreaManageTableHeader::getUnusePageNum;

		this->GetPageArrayOffset =
			_LargeAreaManageTableHeader::getPageArrayOffset;

		this->ExistManagePage =
			_LargeAreaManageTableHeader::existManagePage;

		this->UpdateUsedPageNum =
			_LargeAreaManageTableHeader::updateUsedPageNum;

		this->UpdateUnusePageNum =
			_LargeAreaManageTableHeader::updateUnusePageNum;

		this->UpdatePageArrayElement =
			_LargeAreaManageTableHeader::updatePageArrayElement;

		this->OverwritePageArrayElement =
			_LargeAreaManageTableHeader::overwritePageArrayElement;

		this->FetchOutPageNum =
			_LargeAreaManageTableHeader::fetchOutPageNum;

		this->FetchOutPageArrayBoth =
			_LargeAreaManageTableHeader::fetchOutPageArrayBoth;

		this->FetchOutPageArray =
			_LargeAreaManageTableHeader::fetchOutPageArray;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageTable::Bitmapクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST private
//	PhysicalFile::AreaManageTable::Bitmap::StartCaseBySmallHeader --
//		領域率ビットマップ開始位置
//
//	NOTES
//	各項目を2バイトで記録するタイプの空き領域管理表に続く
//	領域率ビットマップの開始位置。[byte]
//
// static
const PageOffset
AreaManageTable::Bitmap::StartCaseBySmallHeader =
	AreaManageTable::Header::SmallSize;

//
//	CONST private
//	PhysicalFile::AreaManageTable::Bitmap::StartCaseByLargeHeader --
//		領域率ビットマップ開始位置
//
//	NOTES
//	各項目を4バイトで記録するタイプの空き領域管理表に続く
//	領域率ビットマップの開始位置。[byte]
//
// static
const PageOffset
AreaManageTable::Bitmap::StartCaseByLargeHeader =
	AreaManageTable::Header::LargeSize;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageTable::Bitmapクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Bitmap::getOffset --
//		領域率ビットマップの開始位置を返す
//
//	NOTES
//	空き領域管理表内の領域率ビットマップの開始位置を返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaManageTable::Header::Type	HeaderType_
//		空き領域管理表ヘッダタイプ
//
//	RETURN
//	PhysicalFile::PageOffset
//		領域率ビットマップの開始位置 [byte]
//
//	EXCEPTIONS
//	なし
//
// static
PageOffset
AreaManageTable::Bitmap::getOffset(
	const AreaManageTable::Header::Type	HeaderType_)
{
	return
		(HeaderType_ == AreaManageTable::Header::SmallType) ?
			PhysicalFile::AreaManageTable::Bitmap::StartCaseBySmallHeader :
			PhysicalFile::AreaManageTable::Bitmap::StartCaseByLargeHeader;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Bitmap::getOffset --
//		領域率ビットマップオフセットを返す
//
//	NOTES
//	引数PageID_が示す物理ページを管理するための領域率ビットマップの
//	バージョンページ内でのオフセットを返す。
//
//	ARGUMENTS
//	const PhysicalFile::AreaManageTable::Header::Type	HeaderType_
//		空き領域管理表ヘッダタイプ
//	const PhysicalFile::PageID							PageID_
//		物理ページ識別子
//	const PhysicalFile::PageNum							PagePerManageTable_
//		1つの空き領域管理表が管理可能な物理ページ数
//
//	RETURN
//	PhysicalFile::PageOffset
//		領域率ビットマップオフセット [byte]
//
//	EXCEPTIONS
//	なし
//
// static
PageOffset
AreaManageTable::Bitmap::getOffset(
	const AreaManageTable::Header::Type	HeaderType_,
	const PageID						PageID_,
	const PageNum						PagePerManageTable_)
{
	return
		AreaManageTable::Bitmap::getOffset(HeaderType_) +
			PageID_ % PagePerManageTable_;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Bitmap::searchPage --
//		指定未使用領域率／空き領域率以上の領域をもつ物理ページを検索する
//
//	NOTES
//	領域率ビットマップ内で、
//	引数AreaRate_以上の未使用領域／空き領域をもつ物理ページを検索する。
//
//	ARGUMENTS
//	const void*											TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PhysicalFile::AreaManageTable::Header::Type	HeaderType_
//		空き領域管理表ヘッダタイプ
//	const unsigned int									AreaRateValue_
//		未使用領域率／空き領域率 [%]
//	PhysicalFile::PageID&								PageIDInTable_
//		空き領域管理表で管理している先頭物理ページの識別子を0とした場合の
//		物理ページ識別子への参照
//	const PhysicalFile::PageID							LastPageIDInTable_
//		空き領域管理表で管理している先頭物理ページの識別子を0とした場合の
//		最終物理ページの識別子
//	const bool											ByUnuseAreaRate_
//		未使用領域率、空き領域率いずれで物理ページを検索するか
//			true  : 引数AreaRate_以上の未使用領域をもつ物理ページを検索
//			false : 引数AreaRate_以上の空き領域をもつ物理ページを検索
//
//	RETURN
//	bool
//		該当する物理ページが見つかったかどうか
//			true  : 引数AreaRateValue_以上の未使用領域／空き領域をもつ
//			        物理ページが見つかった
//			false : 引数AreaRateValue_以上の未使用領域／空き領域をもつ
//			        物理ページが見つからなかった
//
//	EXCEPTIONS
//	なし
//
// static
bool
AreaManageTable::Bitmap::searchPage(
	const void*							TablePointer_,
	const AreaManageTable::Header::Type	HeaderType_,
	const unsigned int					AreaRateValue_,
	PageID&								PageIDInTable_,
	const PageID						LastPageIDInTable_,
	const bool							ByUnuseAreaRate_)
{
	// 領域率ビットマップへのポインタを設定する

	const unsigned char*	bitmapPointer =
		static_cast<const unsigned char*>(TablePointer_) +
		PhysicalFile::AreaManageTable::Bitmap::getOffset(HeaderType_) +
		PageIDInTable_;

	const unsigned char*	bitmapEndPointer =
		static_cast<const unsigned char*>(TablePointer_) +
		PhysicalFile::AreaManageTable::Bitmap::getOffset(HeaderType_) +
		LastPageIDInTable_;

	// 検索する領域率に対応する列挙子を得る
	BitmapTable::Rate	searchRate = BitmapTable::ToRate[AreaRateValue_];

	// 領域率ビットマップを走査して、
	// 引数AreaRateValue_以上の未使用領域／空き領域をもつ
	// 物理ページを検索する
	bool	exist = false;
	for (;
		 bitmapPointer <= bitmapEndPointer;
		 bitmapPointer++, PageIDInTable_++)
	{
		BitmapTable::Rate	rate =
			ByUnuseAreaRate_ ?
				BitmapTable::ToUnuseAreaRate[*bitmapPointer] :
				BitmapTable::ToFreeAreaRate[*bitmapPointer];

		if (rate > searchRate)
		{
			exist = true;
			break;
		}
	}

	return exist;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Bitmap::overwriteValue --
//		領域率ビットマップの値を上書きする
//
//	NOTES
//	領域率ビットマップの値を上書きする。
//
//	ARGUMENTS
//	void*												TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PhysicalFile::AreaManageTable::Header::Type	HeaderType_
//		空き領域管理表ヘッダタイプ
//	const PhysicalFile::PageID							PageID_
//		領域率ビットマップの値を上書きする物理ページの識別子
//	const PhysicalFile::PageNum							PagePerManageTable_
//		1つの空き領域管理表で管理可能な物理ページ数
//	const unsigned char									Value_
//		領域率ビットマップの値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
AreaManageTable::Bitmap::overwriteValue(
	void*								TablePointer_,
	const AreaManageTable::Header::Type	HeaderType_,
	const PageID						PageID_,
	const PageNum						PagePerManageTable_,
	const unsigned char					Value_)
{
	// 領域率ビットマップへのポインタを設定する
	unsigned char*	bitmapPointer =
		static_cast<unsigned char*>(TablePointer_);

	bitmapPointer += AreaManageTable::Bitmap::getOffset(HeaderType_,
														PageID_,
														PagePerManageTable_);

	// 領域率ビットマップを上書きする
	*bitmapPointer = Value_;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageTable::Bitmap::getValue --
//		領域率ビットマップの値を返す
//
//	NOTES
//	引数PageID_で示される物理ページに対応した
//	領域率ビットマップの値を返す。
//
//	ARGUMENTS
//	const void*											TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PhysicalFile::AreaManageTable::Header::Type	HeaderType_
//		空き領域管理表ヘッダタイプ
//	const PhysicalFile::PageID							PageID_
//		領域率ビットマップの値を得る物理ページ識別子
//	const PhysicalFile::PageNum							PagePerManageTable_
//		1つの空き領域管理表で管理可能な物理ページ数
//
//	RETURN
//	unsigned char
//		領域率ビットマップの値（8ビット）
//
//	EXCEPTIONS
//	なし
//
// static
unsigned char
AreaManageTable::Bitmap::getValue(
	const void*							TablePointer_,
	const AreaManageTable::Header::Type	HeaderType_,
	const PageID						PageID_,
	const PageNum						PagePerManageTable_)
{
	// 領域率ビットマップへのポインタを設定する
	const unsigned char*	bitmapPointer =
		static_cast<const unsigned char*>(TablePointer_);

	bitmapPointer +=
		AreaManageTable::Bitmap::getOffset(HeaderType_,
										   PageID_,
										   PagePerManageTable_);

	// 領域率ビットマップを取り出し、返す
	return *bitmapPointer;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFie::AreaManageFileクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST private
//	PhysicalFile::AreaManageFile::PageSearchableThreshold --
//		物理ページを高速検索可能な閾値となる検索基準領域率
//
//	NOTES
//	物理ページを高速検索可能な閾値となる検索基準領域率。 [%]
//
// static
const unsigned int
AreaManageFile::PageSearchableThreshold = 80;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFie::AreaManageFileクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::recoverPage --
//		物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	物理ページ記述子を破棄し、ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//	PhysicalFile::Page*&		Page_
//		破棄する物理ページの記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::recoverPage(const Trans::Transaction&	Transaction_,
							Page*&						Page_)
{
	AreaManagePage*	page = _SYDNEY_DYNAMIC_CAST(AreaManagePage*, Page_);

	; _SYDNEY_ASSERT(page != 0);

	bool	necessaryRecoverTable = page->m_NecessaryRecoverTable;
	PageID	pageID = page->m_ID;

	this->detachPage(Page_, Page::UnfixMode::NotDirty);

	if (necessaryRecoverTable)
	{
		Version::Page::ID	tableVersionPageID =
			this->getManageTableVersionPageID(pageID);

		this->recoverAreaManageTable(Transaction_, tableVersionPageID);
	}
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::recoverPageAll --
//		生成されている全物理ページ記述子を破棄し、
//		ページ内容を元に戻す
//
//	NOTES
//	生成されている全物理ページ記述子を破棄し、
//	ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::recoverPageAll(const Trans::Transaction&	Transaction_)
{
	ModVector<Version::Page::ID>	tableVersionPageIDs;

	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	while (this->m_Page != 0)
	{
		AreaManagePage*	page = _SYDNEY_DYNAMIC_CAST(AreaManagePage*,
													this->m_Page);

		bool	necessaryRecoverTable = page->m_NecessaryRecoverTable;
		PageID	pageID = page->m_ID;

		this->detachPage(this->m_Page, Page::UnfixMode::NotDirty);

		if (necessaryRecoverTable)
		{
			Version::Page::ID	tableVersionPageID =
				this->getManageTableVersionPageID(pageID);

			if (tableVersionPageIDs.find(tableVersionPageID) ==
				tableVersionPageIDs.end())
			{
				tableVersionPageIDs.pushBack(tableVersionPageID);
			}
		}
	}

	if (tableVersionPageIDs.getSize() > 0)
	{
		this->recoverAreaManageTables(Transaction_,
									  tableVersionPageIDs);
	}

	File::recoverPageAll();
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getPageSearchableThreshold --
//		物理ページを高速検索可能な閾値を返す
//
//	NOTES
//	物理ページを高速検索可能な閾値を返す。 [byte]
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページを高速検索可能な閾値 [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManageFile::getPageSearchableThreshold() const
{
	; _SYDNEY_ASSERT(this->m_UserAreaSizeMax > 0);

	return
		static_cast<PageSize>(
			static_cast<double>(this->m_UserAreaSizeMax) / 100.0 *
			(AreaManageFile::PageSearchableThreshold -
			 (100 - this->m_PageUseRate)));
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::searchFreePage --
//		物理ページを検索する
//
//	NOTES
//	引数Size_以上の未使用領域または空き領域をもつ
//	物理ページを検索し、
//	該当する物理ページが存在する場合には
//	その物理ページの識別子を、
//	存在しない場合には
//	PhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	検索は、引数PageID_で指定される識別子が示す
//	物理ページから物理ファイル終端に向かって行われる。
//	ただし、引数PageID_にPhysicalFile::ConstValue::UndefinedPageIDが
//	指定された場合には
//	物理ファイル先頭から物理ファイル終端に向かって
//	検索が行われる。
//
//	空き領域管理機能付き物理ファイルの場合、
//	物理ページ内の領域のうち利用者が利用可能な領域は、
//	その物理ページ内に存在する物理エリア数により変わる。
//	このため、利用者は物理ファイル記述子に対して
//		「引数AreaNum_個の物理エリアを生成するために、
//		　引数Size_分の未使用領域（または空き領域）が
//		　存在する物理ページを検索する」
//	というような指示を出す必要がある。
//
//	物理ファイル格納戦略で指定される
//	物理ページ内の使用率上限も考慮し、検索を行う。
//
//	検索高速化のため、実際には該当する物理ページが
//	物理ファイル内に存在するにもかかわらず、
//	PhysicalFile::ConstValue::UndefinedPageIDを返すこともある。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageSize	Size_
//		未使用領域サイズ／空き領域サイズ [byte]
//	const PhysicalFile::PageID		PageID_
//		検索開始物理ページの識別子
//		PhysicalFile::ConstValue::UndefinedPageIDが指定された場合には、
//		物理ファイル先頭から検索を行う。
//	const bool						IsUnuseArea_
//		true  : 引数Size_が未使用領域サイズ
//		false : 引数Size_が空き領域サイズ
//	const PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（見つかった物理ページ内に
//		　いくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageID
//		十分な未使用領域または空き領域をもつ
//		物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
PageID
AreaManageFile::searchFreePage(
	const Trans::Transaction&	Transaction_,
	const PageSize				Size_,
	const PageID				PageID_,
	const bool					IsUnuseArea_,
	const AreaNum				AreaNum_ // = 1
	)
{
	Page* pPage = searchFreePage2(Transaction_,
								  Size_,
								  Buffer::Page::FixMode::ReadOnly,
								  PageID_,
								  IsUnuseArea_,
								  AreaNum_);
	
	PageID id = (pPage) ? pPage->getID() : ConstValue::UndefinedPageID;
	
	if (m_bAttachedForSearchFreePage && pPage)
		// 内部でattachしたページなので、detachする
		detachPage(pPage, Page::UnfixMode::NotDirty);

	return id;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::searchFreePage2 --
//		物理ページを検索する
//
//	NOTES
//	引数Size_以上の未使用領域または空き領域をもつ
//	物理ページを検索し、
//	該当する物理ページが存在する場合には
//	その物理ページの識別子を、
//	存在しない場合には0を返す。
//
//	検索は、引数PageID_で指定される識別子が示す
//	物理ページから物理ファイル終端に向かって行われる。
//	ただし、引数PageID_にPhysicalFile::ConstValue::UndefinedPageIDが
//	指定された場合には
//	物理ファイル先頭から物理ファイル終端に向かって
//	検索が行われる。
//
//【仕様変更】
//	これだと、ファイルが大きくなった場合リニアにコストが増大してしまうので、
//	よろしくない。この不具合に対応するため、検索時に利用する管理テーブルページ
//	の上限を3とした。管理テーブルをランダムに3つ見て、空いていなかったら
//	あきらめる。
//	本来なら、管理テーブルを管理するページを作るべきだが、ファイルフォーマット
//	が変更されてしまうので、とりあえずこの修正で様子を見る。
//
//	空き領域管理機能付き物理ファイルの場合、
//	物理ページ内の領域のうち利用者が利用可能な領域は、
//	その物理ページ内に存在する物理エリア数により変わる。
//	このため、利用者は物理ファイル記述子に対して
//		「引数AreaNum_個の物理エリアを生成するために、
//		　引数Size_分の未使用領域（または空き領域）が
//		　存在する物理ページを検索する」
//	というような指示を出す必要がある。
//
//	物理ファイル格納戦略で指定される
//	物理ページ内の使用率上限も考慮し、検索を行う。
//
//	検索高速化のため、実際には該当する物理ページが
//	物理ファイル内に存在するにもかかわらず、0を返すこともある。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::PageSize	Size_
//		未使用領域サイズ／空き領域サイズ [byte]
//	Buffer::Page::FixMode::Value eFixMode
//		見つかったページをattachするときのFIXモード
//	PhysicalFile::PageID		PageID_
//		検索開始物理ページの識別子
//		PhysicalFile::ConstValue::UndefinedPageIDが指定された場合には、
//		物理ファイル先頭から検索を行う。
//	bool						IsUnuseArea_
//		true  : 引数Size_が未使用領域サイズ
//		false : 引数Size_が空き領域サイズ
//	PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（見つかった物理ページ内に
//		　いくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::Page*
//		十分な未使用領域または空き領域をもつ物理ページ
//
//	EXCEPTIONS
//	なし
//
Page*
AreaManageFile::searchFreePage2(const Trans::Transaction&		Transaction_,
								PageSize						Size_,
								Buffer::Page::FixMode::Value	eFixMode_,
								PageID							PageID_,
								bool							IsUnuseArea_,
								AreaNum 						AreaNum_)
{
	m_bAttachedForSearchFreePage = false;
	
	// 検索基準領域率を求める
	// 　検索基準領域率 ＝
	//       100 − 物理ページ内の使用率上限[％] ＋ 検索領域率
	unsigned int	searchRateValue =
		100 - this->m_PageUseRate + this->convertToAreaRate(Size_);

	if (searchRateValue >= AreaManageFile::PageSearchableThreshold)
	{
		// 高速検索ができないかもしれない…

		// 検索は行わずに利用者に
		// 「該当する物理ページは存在しない」と通知する
		return 0;
	}

	Page*	pPage = 0;

	bool	exist = false;

	try
	{
		// 物理ファイルヘッダのバッファリング内容を得る
		// （フィックスする）
		PagePointer fileHeader =
			fixVersionPage(Transaction_,
						   FileHeader::VersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		// 物理ファイルヘッダのバッファリング内容へのポインタを得る
		const void*	fileHeaderPointer =static_cast<const VersionPage&>(*fileHeader).operator const void*(); 

		// 物理ファイルヘッダに記録されている
		// 　1. 使用中の物理ページ数
		// 　2. 未使用の物理ページ数
		// を取り出す
		PageNum	usedPageNum = 0;
		PageNum	unusePageNum = 0;
		FileHeader::fetchOutPageNum(fileHeaderPointer,
									usedPageNum,
									unusePageNum);

		if (usedPageNum == 0)
		{
			// 物理ファイル内に使用中の物理ページが存在しない…

			// 検索は行わずに利用者に
			// 「該当する物理ページは存在しない」と通知する
			return 0;
		}

		// 空き領域管理表のバージョンページ識別子を設定する
		Version::Page::ID	tableVersionPageID =
			FileHeader::VersionPageID + 1;

		// 空き領域管理表で管理している
		// 先頭物理ページの識別子を0とした場合の
		// 物理ページ識別子を設定する
		PageID	pageIDInTable = 0;

		// 物理ファイルが管理している物理ページ数を求める
		PageNum	totalPageNum = usedPageNum + unusePageNum;

		// 最後の空き領域管理表のバージョンページ識別子を得る
		Version::Page::ID	lastTableVersionPageID =
			this->getManageTableVersionPageID(totalPageNum - 1);

		// チェックするべき空き領域管理表を格納する配列
		Version::Page::ID checkTable[3];
		ModSize checkCount = 3;
		
		if (PageID_ != 0 && PageID_ != ConstValue::UndefinedPageID)
		{
			// 検索開始物理ページの識別子が指定されている…

			// 検索開始空き領域管理表のバージョンページ識別子を得る
			tableVersionPageID =
				this->getManageTableVersionPageID(PageID_);

			// 空き領域管理表で管理している先頭物理ページの識別子を
			// 0とした場合の物理ページ識別子を設定する
			pageIDInTable =
				PageID_ % this->m_PagePerManageTable;

			// ページIDが指定されている場合は、指定されたページID
			// からチェックするので、それ以降のものを乱数で得る
			checkCount = File::getLookManageTable(m_PagePerManageTable,
												  tableVersionPageID + m_PagePerManageTable + 1,
												  lastTableVersionPageID,
												  &checkTable[1],
												  2);

			// 指定ページの空き領域管理表を先頭に設定する
			checkTable[0] = tableVersionPageID;
			checkCount++;
		}
		else
		{
			// 本当にチェックする空き領域管理表を乱数で得る
			checkCount = File::getLookManageTable(m_PagePerManageTable,
												  tableVersionPageID,
												  lastTableVersionPageID,
												  checkTable,
												  checkCount);
		}

		for (ModSize i = 0; i < checkCount && exist == false; ++i)
		{
			// チェックする空き領域管理表のページ番号
			tableVersionPageID = checkTable[i];

			// 上の空き領域管理表の先頭要素が示すページID
			ModSize basePageID =
				(tableVersionPageID - FileHeader::VersionPageID - 1) /
				(m_PagePerManageTable + 1) *
				m_PagePerManageTable;
			
			// 空き領域管理表のバッファリング内容を得る
			// （フィックスする）
			PagePointer table =
				fixVersionPage(Transaction_,
							   tableVersionPageID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Middle);

			// 空き領域管理表のバッファリング内容へのポインタを得る
			//const void*	tablePointer = table->operator const void*();
                        const void*	tablePointer = static_cast<const VersionPage&>(*table).operator const void*();

			// 空き領域管理表ヘッダに記録されている
			// 　1. 使用中の物理ページ数
			// 	 2. 未使用の物理ページ数
			// を取り出す
			(*(this->m_TableHeader.FetchOutPageNum))(tablePointer,
													 usedPageNum,
													 unusePageNum);

			// 空き領域管理表が管理している物理ページ数を求める
			totalPageNum = usedPageNum + unusePageNum;

			// 空き領域管理表で管理している先頭物理ページの識別子を
			// 0とした場合の最終物理ページの識別子を求める
			PageID	lastPageIDInTable = totalPageNum - 1;

			if ((*(this->m_TableHeader.ExistManagePage))(tablePointer,
														 searchRateValue,
														 IsUnuseArea_))
			{
				// 空き領域管理表で検索基準領域率以上の
				// 未使用領域／空き領域をもつ物理ページを管理している…

				do
				{
					if (AreaManageTable::Bitmap::searchPage(
							tablePointer,
							this->m_TableHeader.m_Type,
							searchRateValue,
							pageIDInTable,
							lastPageIDInTable,
							IsUnuseArea_))
					{
						PageID pageID = basePageID + pageIDInTable;

						AreaManagePageHeader::Item	headerItem;
						AreaNum	overwriteAreaNum;
//						{
						// 物理ページ記述子のリンクを保護するためにラッチする

//						Os::AutoCriticalSection latch(_latch);

						pPage = 0;

						if (eFixMode_ == Buffer::Page::FixMode::ReadOnly)
							pPage = this->getCachedPage(pageID);

						if (m_bAttachedForSearchFreePage = (pPage == 0))
						{
							// 物理ページのバッファリング内容を得る
							// （フィックスする）
							pPage = 
								this->attachPage(
									Transaction_,
									pageID,
									eFixMode_,
									Buffer::ReplacementPriority::Low);
						}

						; _SYDNEY_ASSERT(pPage != 0);

						// 物理ページのバッファリング内容への
						// ポインタを得る
						const void*	pagePointer = (const void*)*pPage;

						// 物理ページヘッダから
						// 　1. 未使用領域サイズ
						// 　2. 空き領域サイズ
						// 　3. 空き領域オフセット
						// 　4. 管理している物理エリア数
						// を取り出す

						// バージョンページ先頭へポインタを移動する
						pagePointer =
							static_cast<const char*>(pagePointer) -
							m_PageHeader->getSize();

						(*m_PageHeader->FetchOut)(pagePointer,
												  headerItem);

						// 物理エリア生成による、
						// 物理エリア管理ディレクトリの
						// 追加サイズを求める

						overwriteAreaNum =
							m_Directory.getOverwriteAreaNum(
								pagePointer,
								headerItem.m_ManageAreaNum);

						//【注意】	ここで物理ページ記述子のリンクを
						//			保護するラッチがはずれる
//						}

						if (overwriteAreaNum > AreaNum_)
						{
							overwriteAreaNum = AreaNum_;
						}

						PageSize	additionalSize =
							m_Directory.getSize(
								headerItem.m_ManageAreaNum +
								AreaNum_ -
								overwriteAreaNum) -
							m_Directory.getSize(
								headerItem.m_ManageAreaNum);

						PageSize*	areaSizePointer =
							IsUnuseArea_ ?
								&headerItem.m_UnuseAreaSize :
								&headerItem.m_FreeAreaSize;

						if (*areaSizePointer - additionalSize > Size_)
						{
							// 検索条件に該当する物理ページが
							// 見つかった…

							exist = true;
						}
						else
						{
							 // やっぱり足りなかった…

							if (m_bAttachedForSearchFreePage)
							{
								this->detachPage(pPage,
												 Page::UnfixMode::NotDirty);
							}

							pageIDInTable++;
						}
					}
				}
				while (exist == false &&
					   pageIDInTable < lastPageIDInTable);

			} // end if (AreaManageTable::Header::existManagePage(
			  //             tablePointer,
			  //             serachRateValue,
			  //             IsUnuseArea_))

			// 空き領域管理表で管理している
			// 先頭物理ページの識別子を0とした場合の
			// 物理ページ識別子を再初期化する
			pageIDInTable = 0;
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		exist = false;
	}

	return exist ? pPage : 0;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getAllocatedSize --
//		利用者が確保済のおおよその領域サイズを返す
//
//	NOTES
//	利用者が確保済のおおよその領域サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::FileSize
//		利用者が確保済のおおよその領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
FileSize
AreaManageFile::getAllocatedSize(
	const Trans::Transaction&	Transaction_)
{
	AutoUnfix cUnfix(this);
	cUnfix.success();
	
	PageNum	totalPageNum = 0;

	// 物理ファイルヘッダのバッファリング内容を得る
	// （フィックスする）
	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	// 物理ファイルヘッダのバッファリング内容へのポインタを得る
	//const void*	fileHeaderPointer = fileHeader->operator const void*();
         const void*	fileHeaderPointer =static_cast<const VersionPage&>(*fileHeader).operator const void*();

	// 物理ファイルヘッダから
	//     1. 使用中の物理ページ数
	//     2. 未使用の物理ページ数
	// を取り出す
	PageNum	usedPageNum = 0;
	PageNum	unusePageNum = 0;
	FileHeader::fetchOutPageNum(fileHeaderPointer,
								usedPageNum,
								unusePageNum);

	// 物理ファイルに含まれているすべての物理ページ数を求める
	totalPageNum = usedPageNum + unusePageNum;

	// 領域率別の物理ページ数を記録するための配列を確保し、初期化する
	ModSize	arraySize =
		sizeof(PageNum) * AreaManageTable::Header::PageArrayElementNum;
	PageNum*	totalArray =
		static_cast<PageNum*>(ModDefaultManager::allocate(arraySize));
	PageNum*	workArray =
		static_cast<PageNum*>(ModDefaultManager::allocate(arraySize));
	for (unsigned int i = 0;
		 i < AreaManageTable::Header::PageArrayElementNum;
		 i++)
	{
		*(totalArray + i) = 0;
	}

	//
	// 各空き領域管理表を参照し、
	// 領域率別の物理ページ数を読み込み、
	// 配列の各要素へ加算していく。
	//

	; _SYDNEY_ASSERT(totalPageNum > 0);

	Version::Page::ID	tableVersionPageID = 1;
	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(totalPageNum - 1);

	PageNum	skipNum = this->m_PagePerManageTable + 1;

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		PagePointer table =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	tablePointer = table->operator const void*();
                const void*	tablePointer = static_cast<const VersionPage&>(*table).operator const void*();

		(*(this->m_TableHeader.FetchOutPageArray))(tablePointer,
												   true, // 未使用
												   workArray);

		for (unsigned int i = 0;
			 i < AreaManageTable::Header::PageArrayElementNum;
			 i++)
		{
			*(totalArray + i) += *(workArray + i);
		}

		tableVersionPageID += skipNum;
	}

	FileSize	allocatedSize = 0;

	//
	// 領域率から領域サイズを求める
	//

	// 〜４％未使用。
	// ということは、使い切っている可能性がある。
	if (*(totalArray + 0) > 0)
	{
		allocatedSize +=
				(this->m_UserAreaSizeMax * *(totalArray + 0));
	}

	// ５〜９％未使用。
	// ということは、９５％使っている可能性がある。
	if (*(totalArray + 1) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.95 * *(totalArray + 1)));
	}

	// １０〜１４％未使用。
	// ということは、９０％使っている可能性がある。
	if (*(totalArray + 2) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.9 * *(totalArray + 2)));
	}

	// １５〜１９％未使用。
	// ということは、８５％使っている可能性がある。
	if (*(totalArray + 3) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.85 * *(totalArray + 3)));
	}

	// ２０〜３９％未使用。
	// ということは、８０％使っている可能性がある。
	if (*(totalArray + 4) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.8 * *(totalArray + 4)));
	}

	// ４０〜５９％未使用。
	// ということは、６０％使っている可能性がある。
	if (*(totalArray + 5) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.6 * *(totalArray + 5)));
	}

	// ６０〜７９％未使用。
	// ということは、４０％使っている可能性がある。
	if (*(totalArray + 6) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.4 * *(totalArray + 6)));
	}

	// ８０％〜未使用。
	// ということは、２０％使っている可能性がある。
	if (*(totalArray + 7) > 0)
	{
		allocatedSize +=
			static_cast<FileSize>(
				(this->m_UserAreaSizeMax * 0.2 * *(totalArray + 7)));
	}

	ModDefaultManager::free(totalArray, arraySize);
	ModDefaultManager::free(workArray, arraySize);

	return allocatedSize;
}
#endif // OBSOLETE

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getTopPageID --
//		先頭の使用中の物理ページの識別子を返す
//
//	NOTES
//	先頭の使用中の物理ページの識別子を返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::PageID
//		先頭の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
AreaManageFile::getTopPageID(const Trans::Transaction&	Transaction_)
{
	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	//const void*	fileHeaderPointer = fileHeader->operator const void*();
        const void*	fileHeaderPointer =static_cast<const VersionPage&>(*fileHeader).operator const void*();

	PageNum	usedPageNum = 0;
	PageNum	unusePageNum = 0;
	FileHeader::fetchOutPageNum(fileHeaderPointer,
								usedPageNum,
								unusePageNum);

	if (usedPageNum == 0)
	{
		// 物理ファイル内に使用中の物理ページが存在しない…

		// ならば、“先頭の使用中の物理ページ”など存在しない。
		return ConstValue::UndefinedPageID;
	}

	PageNum	totalPageNum = usedPageNum + unusePageNum;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	PageID	lastPageID = totalPageNum - 1;

	PageID	pageID = 0;

	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(pageID);
	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(lastPageID);

	PageNum	skipNum = this->m_PagePerManageTable + 1;

	PageID	topPageID = ConstValue::UndefinedPageID;

	bool	exist = false;

	while (tableVersionPageID <= lastTableVersionPageID && exist == false)
	{
		PagePointer manageTable =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	manageTablePointer = manageTable->operator const void*();
                const void*     manageTablePointer = static_cast<const VersionPage&>(*manageTable).operator const void*();

		usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(manageTablePointer);

		unusePageNum =
			(*(this->m_TableHeader.GetUnusePageNum))(manageTablePointer);

		if (usedPageNum > 0)
		{
			totalPageNum = usedPageNum + unusePageNum;

			for (PageID pid = 0; pid < totalPageNum; pid++, pageID++)
			{
				if (this->isUsedPage(manageTablePointer, pageID))
				{
					exist = true;

					topPageID = pageID;

					break;
				}

				; _SYDNEY_ASSERT(pageID != lastPageID);
			}
		}
		else
		{
			pageID += this->m_PagePerManageTable;
		}

		tableVersionPageID += skipNum;
	}

	return topPageID;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getLastPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//	最後の使用中の物理ページの識別子を返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::PageID
//		最後の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
AreaManageFile::getLastPageID(const Trans::Transaction&	Transaction_)
{
	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	//const void*	fileHeaderPointer = fileHeader->operator const void*();
        const void*	fileHeaderPointer =static_cast<const VersionPage&>(*fileHeader).operator const void*();

	PageNum usedPageNum = 0;
	PageNum unusePageNum = 0;
	FileHeader::fetchOutPageNum(fileHeaderPointer,
								usedPageNum,
								unusePageNum);
	
	return this->getLastPageID(Transaction_, usedPageNum, unusePageNum);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getNextPageID --
//		次の使用中の物理ページの識別子を返す
//
//	NOTES
//	次の使用中の物理ページの識別子を返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageID
//		次の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
AreaManageFile::getNextPageID(const Trans::Transaction&	Transaction_,
							  const PageID				PageID_)
{
	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	//const void*	fileHeaderPointer = fileHeader->operator const void*();
        const void*	fileHeaderPointer =static_cast<const VersionPage&>(*fileHeader).operator const void*();

	PageNum	usedPageNum = 0;
	PageNum	unusePageNum = 0;
	FileHeader::fetchOutPageNum(fileHeaderPointer,
								usedPageNum,
								unusePageNum);

	if (usedPageNum == 0)
	{
		// 物理ファイル内に使用中の物理ページが存在しない…

		// ならば、“次の使用中の物理ページ”など存在しない。
		return ConstValue::UndefinedPageID;
	}

	PageNum	totalPageNum = usedPageNum + unusePageNum;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	PageID	lastPageID = totalPageNum - 1;

	if (PageID_ > lastPageID)
	{
		// 管理していない物理ページの識別子が指定された…

		// ということは、“次の使用中の物理ページ”など存在しない。

		return ConstValue::UndefinedPageID;
	}

	PageID	pageID = PageID_ + 1;

	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(pageID);
	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(lastPageID);

	PageNum	skipNum = this->m_PagePerManageTable + 1;

	PageID	nextPageID = ConstValue::UndefinedPageID;

	bool	firstTable = true;

	bool	exist = false;

	while (tableVersionPageID <= lastTableVersionPageID && exist == false)
	{
		PagePointer manageTable =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	manageTablePointer = manageTable->operator const void*();
                 const void*     manageTablePointer = static_cast<const VersionPage&>(*manageTable).operator const void*();

		usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(manageTablePointer);

		unusePageNum =
			(*(this->m_TableHeader.GetUnusePageNum))(manageTablePointer);

		totalPageNum = usedPageNum + unusePageNum;

		if (usedPageNum > 0)
		{
			PageID	pid =
				firstTable ? (pageID % this->m_PagePerManageTable) : 0;

			for (; pid < totalPageNum; pid++, pageID++)
			{
				if (this->isUsedPage(manageTablePointer, pageID))
				{
					exist = true;

					nextPageID = pageID;

					break;
				}

				// 例えば、
				//     allocatePage(0)
				//     allocatePage(5)
				//     freePage(5)
				//     getNextPageID(0)
				// のような手順でgetNextPageID()することも
				// あるので、ここでアボートしてはいけない！
				//; _SYDNEY_ASSERT(pageID != lastPageID);
				if (pageID == lastPageID)
				{
					return nextPageID;
				}
			}
		}
		else
		{
			pageID += (totalPageNum - (pageID % this->m_PagePerManageTable));
		}

		tableVersionPageID += skipNum;

		firstTable = false;
	}

	return nextPageID;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManageFile::getPrevPageID --
//		前の使用中の物理ページの識別子を返す
//
//	NOTES
//	前の使用中の物理ページの識別子を返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	PhysicalFile::PageID
//		前の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
AreaManageFile::getPrevPageID(const Trans::Transaction&	Transaction_,
							  const PageID				PageID_)
{
	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	if (PageID_ == 0)
	{
		// 先頭物理ページの識別子が指定された…

		// ということは、“前の使用中の物理ページ”など存在しない。

		return ConstValue::UndefinedPageID;
	}

	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	//const void*	fileHeaderPointer = fileHeader->operator const void*();
         const void*	fileHeaderPointer =static_cast<const VersionPage&>(*fileHeader).operator const void*();

	PageNum	usedPageNum = 0;
	PageNum	unusePageNum = 0;
	FileHeader::fetchOutPageNum(fileHeaderPointer,
								usedPageNum,
								unusePageNum);

	if (usedPageNum == 0)
	{
		// 物理ファイル内に使用中の物理ページが存在しない…

		// ならば、“前の使用中の物理ページ”など存在しない。
		return ConstValue::UndefinedPageID;
	}

	PageNum	totalPageNum = usedPageNum + unusePageNum;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	PageID	lastPageID = totalPageNum - 1;

	PageID	pageID;

	if (PageID_ > lastPageID)
	{
		// 管理していない物理ページの識別子が指定された…

		// ならば、最後の物理ページから検索を開始する。

		pageID = lastPageID;
	}
	else
	{
		pageID = PageID_ - 1;
	}

	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(pageID);
	Version::Page::ID	topTableVersionPageID = 1;

	PageNum	skipNum = this->m_PagePerManageTable + 1;

	PageID	prevPageID = ConstValue::UndefinedPageID;

	bool	firstTable = true;

	while (true)
	{
		PagePointer manageTable =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	manageTablePointer = manageTable->operator const void*();
                const void*     manageTablePointer = static_cast<const VersionPage&>(*manageTable).operator const void*();

		usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(manageTablePointer);

		unusePageNum =
			(*(this->m_TableHeader.GetUnusePageNum))(manageTablePointer);

		bool	exist = false;

		totalPageNum = usedPageNum + unusePageNum;

		if (usedPageNum > 0)
		{
			PageID	pid =
				firstTable ?
					(pageID % this->m_PagePerManageTable) :
					(totalPageNum - 1);

			while (true)
			{
				if (this->isUsedPage(manageTablePointer, pageID))
				{
					exist = true;

					prevPageID = pageID;

					break;
				}

				// 例えば、
				//     allocatePage(0)
				//     allocatePage(5)
				//     freePage(0)
				//     getPrevPageID(5)
				// のような手順でgetNextPageID()することも
				// あるので、ここでアボートしてはいけない！
				//; _SYDNEY_ASSERT(pageID != 0);
				if (pageID == 0)
				{
					return prevPageID;
				}

				pageID--;

				if (pid == 0)
				{
					break;
				}

				pid--;
			}
		}
		else
		{
			if (((pageID + 1) % this->m_PagePerManageTable) > 0) {
				pageID -= ((pageID + 1) % this->m_PagePerManageTable);
			} else {
				pageID -= this->m_PagePerManageTable;
			}
		}

		if (exist || tableVersionPageID == 1)
		{
			break;
		}

		; _SYDNEY_ASSERT(tableVersionPageID > skipNum);

		tableVersionPageID -= skipNum;

		firstTable = false;
	}

	return prevPageID;
}

#ifdef DEBUG

Version::Page::ID
AreaManageFile::getTableID(const PageID	PageID_)
{
	return this->getManageTableVersionPageID(PageID_);
}

void
AreaManageFile::getTableHeader(
	const Trans::Transaction&	Transaction_,
	const Version::Page::ID		TableVersionPageID_,
	PageNum&					UsedPageNum_,
	PageNum&					UnusePageNum_,
	PageNum*					PageNumByUnuseAreaRate_,
	PageNum*					PageNumByFreeAreaRate_)
{
	const Version::Page::Memory&	table =
		Version::Page::fix(Transaction_,
						   *m_VersionFile,
						   TableVersionPageID_,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

	const void*	tableTop = (const void*)table;

	(*(this->m_TableHeader.FetchOutPageNum))(tableTop,
											 UsedPageNum_,
											 UnusePageNum_);

	(*(this->m_TableHeader.FetchOutPageArrayBoth))(tableTop,
												   PageNumByUnuseAreaRate_,
												   PageNumByFreeAreaRate_);
}

void
AreaManageFile::getTableBitmap(
	const Trans::Transaction&	Transaction_,
	const Version::Page::ID		TableVersionPageID_,
	unsigned char*				BitmapBuffer_)
{
	const Version::Page::Memory&	table =
		Version::Page::fix(Transaction_,
						   *m_VersionFile,
						   TableVersionPageID_,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

	const void*	tableTop = (const void*)table;

	PageNum	usedPageNum;
	PageNum	unusePageNum;

	(*(this->m_TableHeader.FetchOutPageNum))(tableTop,
											 usedPageNum,
											 unusePageNum);

	const char*	tablePointer = static_cast<const char*>(tableTop);

	tablePointer +=
		AreaManageTable::Bitmap::getOffset(this->m_TableHeader.m_Type);

	ModOsDriver::Memory::copy(BitmapBuffer_,
							  tablePointer,
							  usedPageNum + unusePageNum);
}

#endif // DEBUG

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManageFileクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::AreaManageFile --
//		コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	空き領域管理機能付き物理ファイルの記述子を生成する。
//
//	ARGUMENTS
//	const PhysicalFile::File::StorageStrategy&		FileStorageStrategy_
//		物理ファイル格納戦略への参照
//	const PhysicalFile::File::BufferingStrategy&	BufferingStrategy_
//		物理ファイルバッファリング戦略への参照
//	const Lock::FileName*							LockName_
//		物理ファイルが存在する論理ファイルのロック名へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	BadArgument
//		不正な引数
//
AreaManageFile::AreaManageFile(
	const File::StorageStrategy&	FileStorageStrategy_,
	const File::BufferingStrategy&	BufferingStrategy_,
	const Lock::FileName*			LockName_,
	bool							batch_)
   : File(FileStorageStrategy_,
		  BufferingStrategy_,
		  LockName_,
		  batch_),
	 m_AreaIDs(0),
	 m_AreaIDsSize(0),
	 m_LastAreaIDs(0),
	 m_LastAreaIDsSize(0),
	 m_TableHeader(),
	 m_PageHeader(0)
{
	// バージョンファイル記述子は、
	// AreaManageFileクラスの親クラスである
	// Fileクラスのコンストラクタ内で生成している。

	//
	// 以下が、空き領域管理機能付き物理ファイル記述子を
	// 生成するための固有の処理
	//

	// バージョンページデータサイズと
	// 1つの空き領域管理表で管理可能な
	// 物理ページ数を求める
	// ※ これは、空き領域管理表内の
	// 　 領域率ビットマップのバイト数と等しい
	try
	{
		this->m_VersionPageSize =
			Version::File::verifyPageSize(
				FileStorageStrategy_.m_VersionFileInfo._pageSize);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		// （多分、）不正な引数…

		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
	this->m_VersionPageDataSize =
		Version::Page::getContentSize(this->m_VersionPageSize);

	this->m_PagePerManageTable =
		this->m_VersionPageDataSize - AreaManageTable::Header::SmallSize;


	// AreaManagePageHeaderの初期化
	m_PageHeader = new AreaManagePageHeader(m_VersionPageSize);
	
	//
	// 空き領域管理表ヘッダタイプおよびアクセスする関数を設定する。
	//

	AreaManageTable::Header::Type	tableHeaderType =
		AreaManageTable::Header::UnknownType;

	if (this->m_PagePerManageTable <= 0xFFFF)
	{
		tableHeaderType = AreaManageTable::Header::SmallType;
	}
	else
	{
		this->m_PagePerManageTable =
			this->m_VersionPageDataSize -
			AreaManageTable::Header::LargeSize;

		tableHeaderType = AreaManageTable::Header::LargeType;
	}

	this->m_TableHeader.setType(tableHeaderType);

	// 公開領域最大サイズを求める
	this->m_UserAreaSizeMax =
		this->m_VersionPageDataSize - m_PageHeader->getSize();

	// 物理ページ内の使用率上限をチェックする
	if (FileStorageStrategy_.m_PageUseRate < 1 ||
		FileStorageStrategy_.m_PageUseRate > 100)
	{
		// 不正な引数…
		
		delete m_PageHeader, m_PageHeader = 0;
		
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	// 物理ページ内の使用率上限を設定する
	this->m_PageUseRate = FileStorageStrategy_.m_PageUseRate;

	// Area::Directoryの初期化
	m_Directory.initialize(m_VersionPageSize, m_VersionPageDataSize);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::~AreaManageFile --
//		デストラクタ
//
//	NOTES
//	デストラクタ。
//	空き領域管理機能付き物理ファイルの記述子を破棄する。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
AreaManageFile::~AreaManageFile()
{
	// バージョンファイル記述子は、
	// AreaManageFileクラスの親クラスである
	// Fileクラスのデストラクタ内で破棄している。

	delete m_PageHeader, m_PageHeader = 0;
}

//	FUNCTION private
//	PhysicalFile::AreaManageFile::initialize -- 物理ファイル生成時の初期化
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Trans_
//		[IN]		トランザクション記述子
//	void*						FileHeader_
//		[IN]		物理ファイルヘッダ
//
//	RETURN
//
//	EXCEPTIONS

void
AreaManageFile::initialize(const Trans::Transaction&	Trans_,
						   void*						FileHeader_)
{
	// 先頭空き領域管理表を確保し、初期化
	// ※ 物理ファイル生成時には先頭物理ページを“未使用”とするので
	// 　 空き領域管理表ヘッダに記録されている“未使用の物理ページ数”を
	// 　 インクリメントする
	Version::Page::ID	vpid = FileHeader::VersionPageID + 1;
	PagePointer	topTable = fixVersionPage(Trans_,
										  vpid++,
										  Buffer::Page::FixMode::Allocate,
										  Buffer::ReplacementPriority::Middle);
	this->updateUnusePageNum(topTable->operator void*(), 1);

	// 先頭物理ページを確保
	// ※ 先頭の物理ページはまだ“未使用”なので初期化不要
	fixVersionPage(Trans_,
				   vpid++,
				   Buffer::Page::FixMode::Allocate,
				   Buffer::ReplacementPriority::Low);

	// 物理ファイルヘッダに記録されている“未使用の物理ページ数”を更新
	// ※ 空き領域管理表ヘッダ同様
	FileHeader::Item_Type1*	fileHeader =
		static_cast<FileHeader::Item_Type1*>(FileHeader_);
	(fileHeader->m_UnusePageNum)++;
}

//	FUNCTION private
//	PhysicalFile::AreaManageFile::allocatePageInstance -- Allocate Page instance
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
Page*
AreaManageFile::allocatePageInstance(
	const Trans::Transaction&			cTransaction_,
	PageID								uiPageID_,
	Buffer::Page::FixMode::Value		eFixMode_,
	Admin::Verification::Progress*		pProgress_,
	Buffer::ReplacementPriority::Value	eReplacementPriority_)
{
	if (pProgress_ == 0)
	{
		return new AreaManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, eReplacementPriority_);
	}
	else
	{
		// For verify
		return new AreaManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, *pProgress_);
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::getLastPageID --
//		最後の使用中の物理ページの識別子を返す
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::PageNum		usedPageNum_
//		使用中ページ数
//	PhysicalFile::PageNum		unusePageNum_
//		未使用ページ数
//
//	RETURN
//	PhysicalFile::PageID
//		最後の使用中の物理ページの識別子
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
PageID
AreaManageFile::getLastPageID(const Trans::Transaction&	Transaction_,
							  PageNum					usedPageNum_,
							  PageNum					unusePageNum_)
{
	if (usedPageNum_ == 0)
	{
		// 物理ファイル内に使用中の物理ページが存在しない…

		// ならば、“最後の使用中の物理ページ”など存在しない。
		return ConstValue::UndefinedPageID;
	}

	PageNum	totalPageNum = usedPageNum_ + unusePageNum_;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	PageID	pageID = totalPageNum - 1;

	PageID	lastPageID = ConstValue::UndefinedPageID;

	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(pageID);
	Version::Page::ID	topTableVersionPageID = 1;

	PageNum	skipNum = this->m_PagePerManageTable + 1;

	while (true)
	{
		PagePointer manageTable =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	manageTablePointer = manageTable->operator const void*();
                const void*     manageTablePointer = static_cast<const VersionPage&>(*manageTable).operator const void*();

		usedPageNum_ =
			(*(this->m_TableHeader.GetUsedPageNum))(manageTablePointer);

		unusePageNum_ =
			(*(this->m_TableHeader.GetUnusePageNum))(manageTablePointer);

		bool	exist = false;

		totalPageNum = usedPageNum_ + unusePageNum_;

		if (usedPageNum_ > 0)
		{
			for (PageID pid = totalPageNum - 1; pid >= 0; pid--, pageID--)
			{
				if (this->isUsedPage(manageTablePointer, pageID))
				{
					exist = true;

					lastPageID = pageID;

					break;
				}

				; _SYDNEY_ASSERT(pid != 0);
			}
		}
		else
		{
			pageID -= totalPageNum;
		}

		if (exist)
		{
			break;
		}

		; _SYDNEY_ASSERT(tableVersionPageID > skipNum);

		tableVersionPageID -= skipNum;
	}

	return lastPageID;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::fetchOutPageNumFromManageTable --
//		空き領域管理表から
//		使用中の物理ページ数と未使用の物理ページ数を取り出す
//
//	NOTES
//	空き領域管理表から
//	使用中の物理ページ数と未使用の物理ページ数を取り出す。
//
//	ARGUMENTS
//	const void*	TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	PageNum&	UsedPageNum_
//		使用中の物理ページ数への参照
//	PageNum&	UnusePageNum_
//		未使用の物理ページ数への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::fetchOutPageNumFromManageTable(
	const void*	TablePointer_,
	PageNum&	UsedPageNum_,
	PageNum&	UnusePageNum_) const
{
	(*(this->m_TableHeader.FetchOutPageNum))(TablePointer_,
											 UsedPageNum_,
											 UnusePageNum_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::updateUsedPageNum --
//		空き領域管理表に記録されている使用中の物理ページ数を更新する
//
//	NOTES
//	空き領域管理表に記録されている使用中の物理ページ数を更新する。
//
//	ARGUMENTS
//	void*		TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::updateUsedPageNum(void*		TablePointer_,
								  const int	AddNum_)
{
	(*(this->m_TableHeader.UpdateUsedPageNum))(TablePointer_, AddNum_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::updateUnusePageNum --
//		空き領域管理表に記録されている未使用の物理ページ数を更新する
//
//	NOTES
//	空き領域管理表に記録されている未使用の物理ページ数を更新する。
//
//	ARGUMENTS
//	void*		TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const int	AddNum_
//		加算数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::updateUnusePageNum(void*		TablePointer_,
								   const int	AddNum_)
{
	(*(this->m_TableHeader.UpdateUnusePageNum))(TablePointer_, AddNum_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::isUsedPage --
//		使用中の物理ページかどうかをチェックする
//
//	NOTES
//	引数PageID_が使用中の物理ページの識別子かどうかをチェックする。
//	チェックは空き領域管理表内でのみ行われる。
//
//	ARGUMENTS
//	const void*					TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		チェックする物理ページの識別子
//
//	RETURN
//	bool
//		使用中の物理ページかどうか
//			true  : 引数PageID_が使用中の物理ページの識別子である
//			false : 引数PageID_が未使用の物理ページの識別子である
//
//	EXCEPTION
//	なし
//
bool
AreaManageFile::isUsedPage(const void*	TablePointer_,
						   const PageID	PageID_) const
{
	// 物理ページに対応した領域率ビットマップの値を得る
	unsigned char	bitmapValue =
		AreaManageTable::Bitmap::getValue(TablePointer_,
										  this->m_TableHeader.m_Type,
										  PageID_,
										  this->m_PagePerManageTable);

	// 領域率ビットマップ変換表を参照し、
	// 使用中の物理ページかどうかをチェックする
	return BitmapTable::ToUnuseAreaRate[bitmapValue] != BitmapTable::Unuse;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::searchNextAssignPage --
//		次に割り当てる物理ページを検索する
//
//	NOTES
//	次に割り当てる物理ページを検索し、識別子を返す。
//	指定された空き領域管理表で未使用の物理ページを
//	管理していない場合にはPhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	ARGUMENTS
//	const void*	TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
PageID
AreaManageFile::searchNextAssignPage(const void*	TablePointer_) const
{
	//
	// 空き領域管理表ヘッダに記録されている
	// 「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す
	//

	PageNum	usedPageNum = 0;
	PageNum	unusePageNum = 0;

	(*(this->m_TableHeader.FetchOutPageNum))(TablePointer_,
											 usedPageNum,
											 unusePageNum);

	// 空き領域管理表が管理している物理ページ数を求める
	PageNum	totalPageNum = usedPageNum + unusePageNum;

	// 未使用の物理ページを管理していない空き領域管理表の場合には
	// PhysicalFile::ConstValue::UndefinedPageIDを返す
	if (unusePageNum == 0)
	{
		return ConstValue::UndefinedPageID;
	}

	// 領域率ビットマップポインタを設定する
	// ※ 空き領域管理表先頭には
	// 　 空き領域管理表ヘッダが記録されているので
	// 　 その分、ポインタを進める
	const unsigned char*	bitmapPointer =
		static_cast<const unsigned char*>(TablePointer_);

	bitmapPointer +=
		AreaManageTable::Bitmap::getOffset(this->m_TableHeader.m_Type);

	PageID	pageID = ConstValue::UndefinedPageID;
	PageID	allocatePageID = 0;

	// 領域率ビットマップを1バイトずつ、なめる
	for (PageNum i = 0;
		 i < totalPageNum;
		 i++, allocatePageID++, bitmapPointer++)
	{
		// 未使用の物理ページが存在したら、
		// その物理ページの識別子を設定して、検索終了
		if (BitmapTable::ToUnuseAreaRate[*bitmapPointer] ==
			BitmapTable::Unuse)
		{
			pageID = allocatePageID;
			break;
		}
	}

	// 空き領域管理表ヘッダに記録されている
	// 未使用の物理ページ数を参照し、
	// 未使用の物理ページを管理している
	// 空き領域管理表だということを確認したのに、
	// 該当する物理ページが存在しないのはおかしい
	; _SYDNEY_ASSERT(pageID != ConstValue::UndefinedPageID);

	return pageID;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::initializePage --
//		物理ページを初期化する
//
//	NOTES
//	物理ページを初期化する。
//	空き領域管理機能付き物理ファイルの場合、
//	物理ページ先頭に「物理ページヘッダ」、
//	物理ページ末尾に「物理エリア管理ディレクトリ」
//	が存在する。
//	初期状態の物理ページには、物理エリア管理ディレクトリは
//	存在しないので、物理ページヘッダのみを初期化する。
//
//	ARGUMENTS
//	void*	PagePointer_
//		物理ページのバッファリング内容へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::initializePage(void*	PagePointer_)
{
	// 物理ページヘッダを初期化する
	AreaManagePageHeader::initialize(PagePointer_,
									 this->m_VersionPageSize,
									 this->m_VersionPageDataSize);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::getManageTableVersionPageID --
//		空き領域管理表のバージョンページ識別子を返す
//
//	NOTES
//	引数PageID_が示す物理ページを管理している
//	空き領域管理表のバージョンページ識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		空き領域管理表のバージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
Version::Page::ID
AreaManageFile::getManageTableVersionPageID(const PageID	PageID_) const
{
	return AreaManageTable::getVersionPageID(PageID_,
											 this->m_PagePerManageTable);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::updateManageTable --
//		空き領域管理表を更新する
//
//	NOTES
//	（物理ページ解放時または再利用時に呼び出され）
//	空き領域管理表を更新する。
//
//	ARGUMENTS
//	void*						TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		確保／解放した物理ページの識別子
//	const PhysicalFile::PageNum	PageNum_
//		物理ファイルが管理している物理ページ数
//	const bool					ForReuse_
//		true
//			物理ページ再利用のために更新する
//		false
//			物理ページ解放のために更新する
//	const void*					PagePointer_ = 0
//		物理ページのバッファリング内容へのポインタ
//		※ 物理ページを解放および再利用する場合に参照する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::updateManageTable(void*			TablePointer_,
								  const PageID	PageID_,
								  const PageNum	PageNum_,
								  const bool	ForReuse_,
								  const void*	PagePointer_ // = 0
								  )
{
	// 物理ページヘッダから
	// 未使用領域サイズと空き領域サイズを取り出す
	PageSize	unuseAreaSize = 0;
	PageSize	freeAreaSize = 0;
	(*m_PageHeader->FetchOutAreaSize)(PagePointer_,
									  unuseAreaSize,
									  freeAreaSize);

	// 読み込んだ領域サイズを領域率に変換する
	unsigned int	unuseAreaRateValue =
		this->convertToAreaRate(unuseAreaSize);
	unsigned int	freeAreaRateValue =
		this->convertToAreaRate(freeAreaSize);

	char	bitmapValue = 0;

	// 領域率ビットマップの値を設定する
	if (ForReuse_)
	{
		// 物理ページを再利用するためにこの関数が呼ばれた…

		// ※ 物理ページを解放するために
		// 　 この関数が呼ばれたのであれば、
		// 　 解放する（された）物理ページに関する
		// 　 領域率ビットマップの8ビットのビット列は
		// 　 全ビットをOFFにすればよいので、
		// 　 変数bitmapValueは0のままでよい

		BitmapTable::Rate	unuseAreaRate =
			BitmapTable::ToRate[unuseAreaRateValue];
		BitmapTable::Rate	freeAreaRate =
			BitmapTable::ToRate[freeAreaRateValue];

		// 領域率ビットマップ変換表を参照し、
		// 領域率ビットマップの値を設定する
		bitmapValue =
			BitmapTable::ToBitmapValue[unuseAreaRate][freeAreaRate];
	}

	// 領域率ビットマップを更新する
	AreaManageTable::Bitmap::overwriteValue(TablePointer_,
											this->m_TableHeader.m_Type,
											PageID_,
											this->m_PagePerManageTable,
											bitmapValue);

	int	addNum = ForReuse_ ? +1 : -1;

	// 空き領域管理表ヘッダに記録されている
	// 「使用中の物理ページ数」を更新する
	(*(this->m_TableHeader.UpdateUsedPageNum))(TablePointer_, addNum);

	// 空き領域管理表ヘッダに記録されている
	// 「未使用の物理ページ数」を更新する
	(*(this->m_TableHeader.UpdateUnusePageNum))(TablePointer_, -addNum);

	// 未使用領域率別の物理ページ数配列の要素を更新する
	(*(this->m_TableHeader.UpdatePageArrayElement))(TablePointer_,
													true, // 未使用
													unuseAreaRateValue,
													ForReuse_);

	// 空き領域率別の物理ページ数配列の要素を更新する
	(*(this->m_TableHeader.UpdatePageArrayElement))(TablePointer_,
													false, // 空き
													freeAreaRateValue,
													ForReuse_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::updateAreaBitmap --
//		空き領域管理表内の領域率ビットマップを更新する
//
//	NOTES
//	空き領域管理表内の領域率ビットマップを更新する。
//
//	ARGUMENTS
//	void*				TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const PageID		PageID_
//		領域率ビットマップを更新する物理ページの識別子
//	const unsigned char	BitmapValue_
//		領域率ビットマップの値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::updateAreaBitmap(void*					TablePointer_,
								 const PageID			PageID_,
								 const unsigned char	BitmapValue_)
{
	AreaManageTable::Bitmap::overwriteValue(TablePointer_,
											this->m_TableHeader.m_Type,
											PageID_,
											this->m_PagePerManageTable,
											BitmapValue_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::updatePageArray --
//		未使用領域率別／空き領域率別の物理ページ数配列を更新する
//
//	NOTES
//	未使用領域率別／空き領域率別の物理ページ数配列を更新する。
//
//	ARGUMENTS
//	void*				TablePointer_
//		空き領域管理表のバッファリング内容へのポインタ
//	const bool			ByUnuseAreaRate_
//		未使用領域率別、空き領域率別いずれの物理ページ数配列か
//			true  : 未使用領域率別の物理ページ数配列
//			false : 空き領域率別の物理ページ数配列
//	const unsigned int	AreaRateValue_
//		更新する要素に対応する領域率 [%]
//	const bool			Increment_
//		インクリメントするのかデクリメントするのか
//			true  : インクリメント
//			false : デクリメント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::updatePageArray(void*				TablePointer_,
								const bool			ByUnuseAreaRate_,
								const unsigned int	AreaRateValue_,
								const bool			Increment_)
{
	(*(this->m_TableHeader.UpdatePageArrayElement))(TablePointer_,
													ByUnuseAreaRate_,
													AreaRateValue_,
													Increment_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::recoverAreaManageTable --
//		空き領域管理表を修復する
//
//	NOTES
//	空き領域管理表を修復する。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子
//	const Version::Page::ID		TableVersionPageID_
//		空き領域管理表のバージョンページ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::recoverAreaManageTable(
	const Trans::Transaction&	Transaction_,
	const Version::Page::ID		TableVersionPageID_)
{
	//
	// 空き領域管理表ヘッダに記録されている
	// 「使用中の物理ページ数」と「未使用の物理ページ数」
	// を読み込む。
	//
	// ※ 「使用中の物理ページ数」と「未使用の物理ページ数」
	// 　 については、修復する必要はない。
	//

	PagePointer table =
		fixVersionPage(Transaction_,
					   TableVersionPageID_,
					   File::DiscardableWriteFixMode,
					   Buffer::ReplacementPriority::Middle);

	void*	tableTop = (void*)(*table);

	PageNum	usedPageNum =
		(*(this->m_TableHeader.GetUsedPageNum))(tableTop);

	PageNum	unusePageNum =
		(*(this->m_TableHeader.GetUnusePageNum))(tableTop);

	PageNum	managePageNum = usedPageNum + unusePageNum;

	//
	// 空き領域管理表で管理している物理ページのうち、
	// 先頭の物理ページの識別子を設定する。
	//

	PageID	pageID = 0;

	while (this->getManageTableVersionPageID(pageID) !=
		   TableVersionPageID_)
	{
		pageID += this->m_PagePerManageTable;
	}

	//
	// 未使用領域率別の物理ページ数配列と
	// 空き領域率別の物理ページ数配列のための
	// バッファを確保する。
	//

	ModSize	arraySize =
		sizeof(PageNum) * AreaManageTable::Header::PageArrayElementNum;

	PageNum*	unusePageNumArray =
		static_cast<PageNum*>(ModDefaultManager::allocate(arraySize));

	ModOsDriver::Memory::reset(unusePageNumArray, arraySize);

	PageNum*	freePageNumArray =
		static_cast<PageNum*>(ModDefaultManager::allocate(arraySize));

	ModOsDriver::Memory::reset(freePageNumArray, arraySize);

	for (PageNum i = 0; i < managePageNum; i++, pageID++)
	{
		if (this->isUsedPage(tableTop, pageID) == false)
		{
			continue;
		}

		// 物理ページ記述子のリンクを保護するためにラッチする

//		Os::AutoCriticalSection latch(_latch);

		// 既にアタッチしているページの場合は、
		// その記述子を使用する。
		// アタッチしていないのであれば、ここでアタッチする。

		Page*	page = this->getCachedPage(pageID);

		bool	attached;

		if (attached = (page == 0))
		{
			page = this->attachPage(Transaction_,
									pageID,
									Buffer::Page::FixMode::ReadOnly);
		}

		; _SYDNEY_ASSERT(page != 0);

		const void*	pageTop = page->m_VersionPageTop;

		//
		// 物理ページヘッダに記録されている
		// 「未使用領域サイズ」と「空き領域サイズ」を読み込む。
		//

		PageSize	unuseAreaSize =
			(*m_PageHeader->GetUnuseAreaSize)(pageTop);

		PageSize	freeAreaSize =
			(*m_PageHeader->GetFreeAreaSize)(pageTop);

		//
		// 読み込んだサイズをそれぞれパーセントに変換する。
		//

		unsigned int	unuseAreaRateValue =
			this->convertToAreaRate(unuseAreaSize);

		unsigned int	freeAreaRateValue =
			this->convertToAreaRate(freeAreaSize);

		//
		// 今度はパーセントをBitmapTable::Rateに変換する。
		//

		BitmapTable::Rate	unuseAreaRate =
			BitmapTable::ToRate[unuseAreaRateValue];

		BitmapTable::Rate	freeAreaRate =
			BitmapTable::ToRate[freeAreaRateValue];

		//
		// 空き領域管理表内の領域率ビットマップを上書きする。
		//

		unsigned char	bitmapValue =
			BitmapTable::ToBitmapValue[unuseAreaRate][freeAreaRate];

		AreaManageTable::Bitmap::overwriteValue(
			tableTop,
			this->m_TableHeader.m_Type,
			pageID,
			this->m_PagePerManageTable,
			bitmapValue);

		//
		// 未使用領域率別の物理ページ数配列と
		// 空き領域率別の物理ページ数配列を更新する。
		// ※ 空き領域管理表ヘッダに直接書き込むのは後でする。
		//

		int	unusePageNumArrayIndex =
			AreaManageTable::Header::ToPageArrayIndex[unuseAreaRateValue];

		(*(unusePageNumArray + unusePageNumArrayIndex))++;

		int	freePageNumArrayIndex =
			AreaManageTable::Header::ToPageArrayIndex[freeAreaRateValue];

		(*(freePageNumArray + freePageNumArrayIndex))++;

		if (attached)
		{
			this->detachPage(page, Page::UnfixMode::NotDirty);
		}
	}

	//
	// 未使用領域率別の物理ページ数配列と
	// 空き領域率別の物理ページ数配列を更新する。
	// ※ ここで、空き領域管理表ヘッダに直接書き込む。
	//

	for (unsigned int j = 0;
		 j < AreaManageTable::Header::PageArrayElementNum;
		 j++)
	{
		(*(this->m_TableHeader.OverwritePageArrayElement))
			(tableTop,
			 true, // by unuse area rate
			 j,
			 *(unusePageNumArray + j));

		(*(this->m_TableHeader.OverwritePageArrayElement))
			(tableTop,
			 false, // by free area rate
			 j,
			 *(freePageNumArray + j));
	}

	ModDefaultManager::free(unusePageNumArray, arraySize);

	ModDefaultManager::free(freePageNumArray, arraySize);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::recoverAreaManageTables --
//		空き領域管理表を修復する
//
//	NOTES
//	空き領域管理表を修復する。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子
//	const ModVector<Version::Page::ID>&	TableVersionPageIDs_
//		空き領域管理表のバージョンページ識別子が記録されている
//		ベクターへの参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::recoverAreaManageTables(
	const Trans::Transaction&		Transaction_,
	ModVector<Version::Page::ID>&	TableVersionPageIDs_)
{
	ModVector<Version::Page::ID>::Iterator	IDIterator =
		TableVersionPageIDs_.begin();

	ModVector<Version::Page::ID>::Iterator	IDsEnd =
		TableVersionPageIDs_.end();

	for (; IDIterator != IDsEnd; IDIterator++)
	{
		this->recoverAreaManageTable(Transaction_, *IDIterator);
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::convertToAreaRate --
//		未使用領域サイズ／空き領域サイズを領域率に変換する
//
//	NOTES
//	未使用領域サイズ／空き領域サイズを領域率に変換する。
//
//	ARGUMENTS
//	const PhysicalFile::PageSize	AreaSize_
//		未使用領域サイズ／空き領域サイズ [byte]
//
//	RETURN
//	unsigned int
//		未使用領域／空き領域率 [%]
//
//	EXCEPTIONS
//	なし
//
unsigned int
AreaManageFile::convertToAreaRate(const PageSize	AreaSize_) const
{
	return
		static_cast<unsigned int>((static_cast<double>(AreaSize_) /
								   this->m_UserAreaSizeMax) *
								  100);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//	物理ページデータサイズを返す。
//
//	ARGUMENTS
//	const Os::Memory::Size		VersionPageSize_
//		バージョンページサイズ [byte]
//	const PhysicalFile::AreaNum	AreaNum_
//		生成物理エリア数
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// static
PageSize
AreaManageFile::getPageDataSize(const Os::Memory::Size	VersionPageSize_,
								const AreaNum			AreaNum_)
{
	// バージョンページに格納可能な内容のサイズを得る
	Os::Memory::Size	verifyVersionPageSize =
		Version::File::verifyPageSize(VersionPageSize_);
	Os::Memory::Size	versionPageContentSize =
		Version::Page::getContentSize(verifyVersionPageSize);

	// 物理ページヘッダの分を引く
	PageSize	pageDataSize =
		versionPageContentSize
		- AreaManagePageHeader::getSize(VersionPageSize_);

	// 物理エリア管理ディレクトリの分を引いて返す
	return pageDataSize - Area::Directory::getSize(VersionPageSize_, AreaNum_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::initializeAreaIDBitmap --
//		物理エリア識別子を記録するためのビットマップの列を初期化する
//
//	NOTES
//	物理エリア識別子を記録するためのビットマップの列を初期化する。
//
//	ARGUMENTS
//	const PhysicalFile::PageNum	ManagePageNum_
//		物理ファイル内で管理している物理ページ数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::initializeAreaIDBitmap(const PageNum	ManagePageNum_)
{
	this->m_AreaIDsSize = sizeof(Common::BitSet*) * ManagePageNum_;

	this->m_AreaIDs =
		static_cast<Common::BitSet**>(
			ModDefaultManager::allocate(this->m_AreaIDsSize));

	this->m_LastAreaIDsSize = sizeof(AreaID) * ManagePageNum_;

	this->m_LastAreaIDs =
		static_cast<AreaID*>(
			ModDefaultManager::allocate(this->m_LastAreaIDsSize));

	for (PageNum i = 0; i < ManagePageNum_; i++)
	{
		*(this->m_AreaIDs + i) = 0;

		*(this->m_LastAreaIDs + i) = ConstValue::UndefinedAreaID;
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::terminateAreaIDBitmap --
//		物理エリア識別子を記録するためのビットマップの列を解放する
//
//	NOTES
//	物理エリア識別子を記録するためのビットマップの列を解放する。
//
//	ARGUMENTS
//	const PhysicalFile::PageNum	ManagePageNum_
//		物理ファイル内で管理している物理ページ数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::terminateAreaIDBitmap(const PageNum	ManagePageNum_)
{
	; _SYDNEY_ASSERT(
		sizeof(Common::BitSet*) * ManagePageNum_ == this->m_AreaIDsSize);

	for (PageNum i = 0; i < ManagePageNum_; i++)
	{
		if (*(this->m_AreaIDs + i) != 0)
		{
			delete *(this->m_AreaIDs + i);
		}
	}

	ModDefaultManager::free(this->m_AreaIDs, this->m_AreaIDsSize);

	this->m_AreaIDsSize = 0;
	this->m_AreaIDs = 0;

	; _SYDNEY_ASSERT(
		sizeof(AreaID) * ManagePageNum_ == this->m_LastAreaIDsSize);

	ModDefaultManager::free(this->m_LastAreaIDs,
							this->m_LastAreaIDsSize);

	this->m_LastAreaIDsSize = 0;
	this->m_LastAreaIDs = 0;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::setAreaIDBitmap --
//		物理エリア識別子を記録するためのビットマップを設定する
//
//	NOTES
//	任意の物理ページに対応する物理エリア識別子を
//	記録するためのビットマップを設定する。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//	const PhysicalFile::AreaNum	AreaNum_
//		引数AreaIDs_の要素数
//	const PhysicalFile::AreaID*	AreaIDs_
//		利用者が使用中とする物理エリアの識別子が
//		記録されている配列へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManageFile::setAreaIDBitmap(const PageID	PageID_,
								const AreaNum	AreaNum_,
								const AreaID*	AreaIDs_)
{
	if (*(this->m_AreaIDs + PageID_) == 0)
	{
		Common::BitSet*	areaIDs = new Common::BitSet();

		areaIDs->reset();

		*(this->m_AreaIDs + PageID_) = areaIDs;
	}

	AreaID	lastAreaID = 0;

	Common::BitSet*	areaIDs = *(this->m_AreaIDs + PageID_);

	for (AreaNum i = 0; i < AreaNum_; i++)
	{
		AreaID	areaID = *(AreaIDs_ + i);

		(*areaIDs).set(areaID, true);

		if (lastAreaID < areaID)
		{
			lastAreaID = areaID;
		}
	}

	if (AreaNum_ > 0)
	{
		*(this->m_LastAreaIDs + PageID_) = lastAreaID;
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::correspondUseArea --
//		利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする
//
//	NOTES
//	利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::correspondUseArea(
	const Trans::Transaction&		Transaction_,
	Admin::Verification::Progress&	Progress_)
{
	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	PageID	pageID = 0;

	//
	// 各空き領域管理表が管理している物理ページのうちで、
	// 使用中の物理ページについて、
	// 利用者と自身の物理エリアの使用状況が一致するかどうかを
	// チェックする。
	//

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		AutoUnfix cUnfix(this);
		
		Admin::Verification::Progress	tableProgress(Progress_.getConnection());

		//
		// 可能であれば修復するように指定されているのであれば、
		// 空き領域率別の物理ページ数配列の要素を
		// 更新する可能性があるため、
		// Writeモードでフィックスする。
		//

		Buffer::Page::FixMode::Value	fixMode =
			Buffer::Page::FixMode::Unknown;

		if (this->isCorrect())
		{
			// 可能であれば修復する…

			//
			// 何らかのエラー発生時に、
			// フィックスした版を破棄できるように
			// Discardableモードも指定する。
			//

			fixMode = File::DiscardableWriteFixMode;
		}
		else
		{
			// 修復しない…

			fixMode = Buffer::Page::FixMode::ReadOnly;
		}

		PagePointer table =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   fixMode,
						   Buffer::ReplacementPriority::Middle);

		const Version::Page::Memory&	tableConstRef = *table;

		void*	tableTop = 0;
		if (fixMode == Buffer::Page::FixMode::ReadOnly)
		{
			//
			// 修復しないので、const_castしても安全。
			//

			tableTop =
				const_cast<void*>((const void*)tableConstRef);
		}
		else
		{
			tableTop = (void*)(*table);
		}

		while (this->getManageTableVersionPageID(pageID) ==
			   tableVersionPageID)
		{
			if (this->m_PageIDs[pageID])
			{
				// 利用者は使用中と指定した物理ページ…

				//
				// 物理ページに関しても、
				// 可能であれば修復するように指定されているのであれば、
				// 更新する可能性があるため、
				// Writeモードでフィックスする。
				//
				Page*	page =
					this->attachPage(Transaction_,
									 pageID,
									 fixMode,
									 Buffer::ReplacementPriority::Low);

				Admin::Verification::Progress	pageProgress(Progress_.getConnection());

				page->correspondUseArea(
					Transaction_,
					tableTop,
					**(this->m_AreaIDs + pageID),
					*(this->m_LastAreaIDs + pageID),
					pageProgress);

				Page::UnfixMode::Value	unfixMode =
					(pageProgress.getStatus() ==
					 Admin::Verification::Status::Corrected) ?
						Page::UnfixMode::Dirty :
						Page::UnfixMode::NotDirty;

				this->detachPage(page, unfixMode);

				if (pageProgress.isGood() == false)
				{
					Progress_ += pageProgress;

					return;
				}

				if (pageProgress.getStatus() !=
					Admin::Verification::Status::Consistent)
				{
					tableProgress += pageProgress;
				}
			}

			pageID++;
		}

		// 修復したのであれば、表を更新したことを伝えなければ。
		if (tableProgress.getStatus() == Admin::Verification::Status::Corrected)
		{
			cUnfix.success();
		}

		if (tableProgress.getStatus() !=
			Admin::Verification::Status::Consistent)
		{
			Progress_ += tableProgress;
		}

		if (Progress_.isGood() == false)
		{
			break;
		}

		tableVersionPageID += this->m_PagePerManageTable + 1;
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::checkPhysicalFile --
//		空き領域管理機能付き物理ファイルの整合性検査を行う
//
//	NOTES
//	空き領域管理機能付き物理ファイルの整合性検査を行う。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::checkPhysicalFile(
	const Trans::Transaction&		Transaction_,
	Admin::Verification::Progress&	Progress_)
{
	AutoUnfix cUnfix(this);
	cUnfix.success();
	
	//
	// 1. 管理物理ページ総数一致検査
	//

	this->checkPageNumInFile(Transaction_,
							 true, // 管理している物理ページ数
							 Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 2. 使用中物理ページ総数一致検査
	//

	this->checkPageNumInFile(Transaction_,
							 false, // 使用中の物理ページ数
							 Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 3. 空き領域管理表ごとの物理ページ数一致検査
	//

	this->checkPageNumInTable(Transaction_, Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 4. 物理ページ未使用領域サイズ一致検査
	//

	this->checkAreaSize(Transaction_,
						true, // 未使用領域
						Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 5. 物理ページ空き領域サイズ一致検査
	//

	this->checkAreaSize(Transaction_,
						false, // 空き領域
						Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 6. 物理エリア情報検査
	//

	this->checkPhysicalArea(Transaction_, Progress_);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::checkPageNumInFile --
//		管理物理ページ総数一致検査および使用中物理ページ総数一致検査
//
//	NOTES
//	管理物理ページ総数一致検査…
//	物理ファイルヘッダに記録されている
//	「使用中の物理ページ数」と「未使用の物理ページ数」の和を
//	その物理ファイルの管理物理ページ数とし、
//	物理ファイル内に存在するすべての空き領域管理表で管理している
//	物理ページ数の総和と一致するかどうかを検査する。
//	各空き領域管理表にはヘッダがあり、
//	ヘッダにその空き領域管理表が管理している
//	「使用中の物理ページ数」と「未使用の物理ページ数」が
//	記録されている。
//	修復不能な検査項目。
//
//	使用中物理ページ総数一致検査…
//	物理ファイルヘッダに記録されている
//	「使用中の物理ページ数」と、
//	物理ファイル内に存在するすべての空き領域管理表（ヘッダ）に
//	記録されている「使用中の物理ページ数」の総和が
//	一致するかどうかを検査する。
//	修復不能な検査項目。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子
//	const bool						IsManage_
//		true  : 管理物理ページ総数一致検査
//		false : 使用中物理ページ総数一致検査
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::checkPageNumInFile(
	const Trans::Transaction&		Transaction_,
	const bool						IsManage_,
	Admin::Verification::Progress&	Progress_)
{
	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Middle);

	//const void*	fileHeaderTop = fileHeader->operator const void*();
        const void*	fileHeaderTop = static_cast<const VersionPage&>(*fileHeader).operator const void*();

	PageNum	usedPageNum = 0;
	PageNum	unusePageNum = 0;
	if (IsManage_)
	{
		FileHeader::fetchOutPageNum(fileHeaderTop,
									usedPageNum,
									unusePageNum);
	}
	else
	{
		usedPageNum = FileHeader::getUsedPageNum(fileHeaderTop);
	}

	PageNum	pageInFile = usedPageNum + unusePageNum;

	PageNum	pageInTables = 0;

	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		PagePointer table =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	tableTop = table->operator const void*();
                const void*	tableTop = static_cast<const VersionPage&>(*table).operator const void*();

		usedPageNum = 0;
		unusePageNum = 0;

		if (IsManage_)
		{
			(*(this->m_TableHeader.FetchOutPageNum))(tableTop,
													 usedPageNum,
													 unusePageNum);
		}
		else
		{
			usedPageNum =
				(*(this->m_TableHeader.GetUsedPageNum))(tableTop);
		}

		pageInTables += usedPageNum + unusePageNum;

		tableVersionPageID += this->m_PagePerManageTable + 1;
	}

	Admin::Verification::Status::Value	status =
		(pageInFile == pageInTables) ?
			Admin::Verification::Status::Consistent :
			Admin::Verification::Status::Inconsistent;

	if (status == Admin::Verification::Status::Inconsistent)
	{
		if (IsManage_)
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				this->m_FilePath,
				Message::DiscordManagePageNum(pageInFile,
											  pageInTables));
		}
		else
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				this->m_FilePath,
				Message::DiscordUsePageNum(pageInFile,
										   pageInTables));
		}
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::checkPageNumInTable --
//		空き領域管理表ごとの物理ページ数一致検査
//
//	NOTES
//	物理ファイル内に存在するすべての空き領域管理表について、
//	空き領域管理表内の未使用領域率別／空き領域率別2つの
//	物理ページ数配列を参照し、
//	それぞれの使用中を示す複数の要素に記録されている
//	物理ページ数の和が一致し、
//	しかも空き領域管理表ヘッダに記録されている
//	「使用中の物理ページ数」と「未使用の物理ページ数」の和とも
//	一致するかどうかを検査する。
//	修復不能な検査項目。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::checkPageNumInTable(
	const Trans::Transaction&		Transaction_,
	Admin::Verification::Progress&	Progress_)
{
	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		PagePointer table =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	tableTop = table->operator const void*();
                const void*	tableTop = static_cast<const VersionPage&>(*table).operator const void*();
 
		PageNum	usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(tableTop);

		ModSize	arraySize =
			sizeof(PageNum) *
			AreaManageTable::Header::PageArrayElementNum;

		PageNum*	pageNumByUnuseArray =
			static_cast<PageNum*>(
				ModDefaultManager::allocate(arraySize));

		PageNum*	pageNumByFreeArray =
			static_cast<PageNum*>(
				ModDefaultManager::allocate(arraySize));

		(*(this->m_TableHeader.FetchOutPageArrayBoth))(
			tableTop,
			pageNumByUnuseArray,
			pageNumByFreeArray);

		PageNum	pageNumByUnuse = 0;
		PageNum	pageNumByFree = 0;

		for (unsigned int i = 0;
			 i < AreaManageTable::Header::PageArrayElementNum;
			 i++)
		{
			pageNumByUnuse += *(pageNumByUnuseArray + i);
			pageNumByFree += *(pageNumByFreeArray + i);
		}

		ModDefaultManager::free(pageNumByUnuseArray, arraySize);
		ModDefaultManager::free(pageNumByFreeArray, arraySize);

		if (pageNumByUnuse != pageNumByFree ||
			pageNumByUnuse != usedPageNum)
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				this->m_FilePath,
				Message::DiscordManagePageNumInTable(usedPageNum,
													 pageNumByUnuse,
													 pageNumByFree));

			break;
		}

		tableVersionPageID += this->m_PagePerManageTable + 1;
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::checkAreaSize --
//		物理ページ未使用領域サイズ一致検査
//		および
//		物理ページ空き領域サイズ一致検査
//
//	NOTES
//	物理ページ未使用領域サイズ一致検査…
//	物理ファイル内に存在するすべての空き領域管理表について、
//	空き領域管理表が管理しているすべての物理ページのヘッダに
//	記録されている未使用領域サイズを参照して
//	未使用領域率別の物理ページ数配列を生成し、
//	空き領域管理表内の同配列とすべての要素が
//	一致するかどうかを検査する。
//	修復不能な検査項目。
//
//	物理ページ空き領域サイズ一致検査…
//	物理ファイル内に存在するすべての空き領域管理表について、
//	空き領域管理表が管理しているすべての物理ページのヘッダに
//	記録されている空き領域サイズを参照して
//	空き領域率別の物理ページ数配列を生成し、
//	空き領域管理表内の同配列とすべての要素が
//	一致するかどうかを検査する。
//	修復不能な検査項目。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子
//	const bool						IsUnuse_
//		true  : 物理ページ未使用領域サイズ一致検査
//		false : 物理ページ空き領域サイズ一致検査
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::checkAreaSize(
	const Trans::Transaction&		Transaction_,
	const bool						IsUnuse_,
	Admin::Verification::Progress&	Progress_)
{
	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	PageID	pageID = 0;

	ModSize	arraySize =
		sizeof(PageNum) * AreaManageTable::Header::PageArrayElementNum;

	// こちらの物理ページ数配列には、
	// 実際に各物理ページの未使用領域／空き領域サイズを参照して
	// 算出した領域率を記録する。
	PageNum*	realPageNumArray =
		static_cast<PageNum*>(ModDefaultManager::allocate(arraySize));

	// こちらの物理ページ数配列には、
	// 空き領域管理表に記録されている
	// 物理ページ数配列を読み込む。
	PageNum*	pageNumArray =
		static_cast<PageNum*>(ModDefaultManager::allocate(arraySize));

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		try
		{
			unsigned int	i;
			for (i = 0;
				 i < AreaManageTable::Header::PageArrayElementNum;
				 i++)
			{
				*(realPageNumArray + i) = 0;
			}

			PagePointer table =
				fixVersionPage(Transaction_,
							   tableVersionPageID,
							   Buffer::Page::FixMode::ReadOnly,
							   Buffer::ReplacementPriority::Middle);

			//const void*	tableTop = table->operator const void*();
                        const void*	tableTop = static_cast<const VersionPage&>(*table).operator const void*();

			while (this->getManageTableVersionPageID(pageID) ==
				   tableVersionPageID)
			{
				if (this->isUsedPage(tableTop, pageID))
				{
					Page*	page = 0;

					try
					{
						page =
							this->attachPage(
								Transaction_,
								pageID,
								Buffer::Page::FixMode::ReadOnly);

						const void*	pageTop = page->m_VersionPageTop;

						PageSize	realAreaSize =
							IsUnuse_ ?
							(*m_PageHeader->GetUnuseAreaSize)(pageTop) :
							(*m_PageHeader->GetFreeAreaSize)(pageTop);

						// [%]
						unsigned int	realAreaRateValue =
							this->convertToAreaRate(realAreaSize);

						BitmapTable::Rate	realAreaRate =
							BitmapTable::ToRate[realAreaRateValue];

						unsigned char	bitmapValue =
							AreaManageTable::Bitmap::getValue(
								tableTop,
								this->m_TableHeader.m_Type,
								pageID,
								this->m_PagePerManageTable);

						BitmapTable::Rate	areaRate =
							IsUnuse_ ?
							BitmapTable::ToUnuseAreaRate[bitmapValue] :
							BitmapTable::ToFreeAreaRate[bitmapValue];

						if (realAreaRate != areaRate)
						{
							if (IsUnuse_)
							{
								_SYDNEY_VERIFY_INCONSISTENT(
									Progress_,
									this->m_FilePath,
									Message::DiscordUnuseAreaRate(
										pageID,
										realAreaRate,
										areaRate));
							}
							else
							{
								_SYDNEY_VERIFY_INCONSISTENT(
									Progress_,
									this->m_FilePath,
									Message::DiscordFreeAreaRate(
										pageID,
										realAreaRate,
										areaRate));
							}

							this->detachPage(page,
											 Page::UnfixMode::NotDirty);

							ModDefaultManager::free(realPageNumArray,
													arraySize);
							ModDefaultManager::free(pageNumArray,
													arraySize);

							return;
						}
						else
						{
							; _SYDNEY_ASSERT(
								areaRate != BitmapTable::Unuse);

							int	arrayIndex =
								static_cast<int>(areaRate) - 1;

							(*(realPageNumArray + arrayIndex))++;
						}

						this->detachPage(page, Page::UnfixMode::NotDirty);
					}
#ifdef NO_CATCH_ALL
					catch (Exception::Object&)
#else
					catch (...)
#endif
					{
						if (page != 0)
						{
							this->detachPage(page,
											 Page::UnfixMode::NotDirty);
						}

						_SYDNEY_RETHROW;
					}
				}

				pageID++;
			}

			(*(this->m_TableHeader.FetchOutPageArray))(tableTop,
													   IsUnuse_,
													   pageNumArray);

			for (i = 0;
				 i < AreaManageTable::Header::PageArrayElementNum;
				 i++)
			{
				if (*(realPageNumArray + i) != *(pageNumArray + i))
				{
					_SYDNEY_VERIFY_INCONSISTENT(
						Progress_,
						this->m_FilePath,
						Message::DiscordPageArray(
							tableVersionPageID,
							i,
							*(pageNumArray + i),
							*(realPageNumArray + i)));

					ModDefaultManager::free(realPageNumArray, arraySize);
					ModDefaultManager::free(pageNumArray, arraySize);

					return;
				}
			}
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			ModDefaultManager::free(realPageNumArray, arraySize);
			ModDefaultManager::free(pageNumArray, arraySize);

			_SYDNEY_RETHROW;
		}

		tableVersionPageID += this->m_PagePerManageTable + 1;
	}

	ModDefaultManager::free(realPageNumArray, arraySize);
	ModDefaultManager::free(pageNumArray, arraySize);
}

//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::checkPhysicalArea --
//		物理エリア情報検査
//
//	NOTES
//	使用中の物理ページ内に存在する物理エリア管理ディレクトリ内の
//	各物理エリア情報を参照し、
//	記録されている物理エリアサイズ、物理エリアオフセットに
//	不整合がないかどうかを検査する。
//	（サイズ、オフセット共にページサイズを超えているものが
//	　ないかを検査する。また、物理エリアの重複検査も行う。）
//	修復不能な検査項目。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
void
AreaManageFile::checkPhysicalArea(
	const Trans::Transaction&		Transaction_,
	Admin::Verification::Progress&	Progress_)
{
	Version::Page::ID	tableVersionPageID =
		this->getManageTableVersionPageID(0);

	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(this->m_LastManagePageID);

	PageID	pageID = 0;

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		PagePointer table =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	tableTop = table->operator const void*();
               const void*	tableTop = static_cast<const VersionPage&>(*table).operator const void*();

		while (this->getManageTableVersionPageID(pageID) ==
			   tableVersionPageID)
		{
			if (this->isUsedPage(tableTop, pageID))
			{
				Page*	page = 0;

				try
				{
					page =
						this->attachPage(Transaction_,
										 pageID,
										 Buffer::Page::FixMode::ReadOnly);

					page->checkPhysicalArea(Progress_);

					this->detachPage(page, Page::UnfixMode::NotDirty);

					if (Progress_.isGood() == false)
					{
						break;
					}
				}
#ifdef NO_CATCH_ALL
				catch (Exception::Object&)
#else
				catch (...)
#endif
				{
					if (page != 0)
					{
						this->detachPage(page, Page::UnfixMode::NotDirty);
					}

					_SYDNEY_RETHROW;
				}
			}
			pageID++;
		}

		if (Progress_.isGood() == false)
		{
			break;
		}

		tableVersionPageID += this->m_PagePerManageTable + 1;
	}
}

#ifdef OBSOLETE
//
//	FUNCTION private
//	PhysicalFile::AreaManageFile::getLookManageTable
//		-- searchFreePageでチェックするべき管理テーブルを列挙する
//
//	NOTES
//
//	ARGUMENTS
//	PhysicalFile::PageID uiFirst_
//		先頭の管理テーブルページID
//	PhysicalFile::PageID uiLast_
//		最後の管理テーブルページID
//	PhysicalFile::PageID* pLookPage_
//		チェックするべき管理テーブルページID
//	int iSize_
//		pLookPage_の容量(2以上であること)
//
//	RETURN
//	int
//		pLookPage_に格納したページIDの数
//
//	EXCEPTIONS
//
ModSize
AreaManageFile::getLookManageTable(Version::Page::ID uiFirst_,
								   Version::Page::ID uiLast_,
								   Version::Page::ID* pLookPage_,
								   ModSize iSize_)
{
	if (uiFirst_ > uiLast_) return 0;
	
	// シャッフルするページの合計数を求める
	// 最終ページはシャッフルの対象ではない
	ModSize n = ((uiLast_ - uiFirst_) / (m_PagePerManageTable + 1));

	ModSize j = 0;

	if (n == 1)
	{
		// シャッフルの対象ページが1ページなので、シャッフルする必要がない
		pLookPage_[j++] = uiFirst_;
	}
	else if (n > 1)
	{
		// WindowsとLinux, Solaris共通で使用できる乱数ジェネレータがないので、
		// STLのrandom_shuffleを利用する。
		// そのため、まずすべてのエントリを配列に格納する
		// (必要以上に内部で乱数が発生するので、遅いかも...)
		ModVector<Version::Page::ID> v;
		v.reserve(n);
		for (ModSize i = 0; i < n; ++i)
			v.pushBack(uiFirst_ + (m_PagePerManageTable + 1) * i);

		// STLのrandom_shuffleで配列をシャッフルする
		std::random_shuffle(v.begin(), v.end());

		// 要求している数だけ返す
		ModSize count = (n < (iSize_ - 1)) ? n : (iSize_ - 1);
		while (j < count)
		{
			pLookPage_[j] = v[j];
			j++;
		}
	}
	// 最終ページを最後に加える
	pLookPage_[j++] = uiLast_;
	
	return j;
}
#endif // OBSOLETE

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
