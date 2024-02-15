// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaManagePage.cpp --
//		空き領域管理機能付き物理ファイルの物理ページ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "Exception/IllegalFileAccess.h"
#include "Exception/BadArgument.h"
#include "Exception/FileManipulateError.h"

#include "Common/Assert.h"
#include "Common/Thread.h"

#include "PhysicalFile/AreaManagePage.h"
#include "PhysicalFile/File.h"

#include "PhysicalFile/Message_DiscordAreaUseSituation1.h"
#include "PhysicalFile/Message_DiscordAreaUseSituation2.h"
#include "PhysicalFile/Message_CorrectedAreaUseSituation.h"
#include "PhysicalFile/Message_ExistProtrusiveArea.h"
#include "PhysicalFile/Message_ExistDuplicateArea.h"

#include "ModDefaultManager.h"
#include "ModOsDriver.h"

_SYDNEY_USING

using namespace PhysicalFile;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManagePageHeaderクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST public
//	PhysicalFile::AreaManagePageHeader::SmallSize --
//		物理ページヘッダサイズ
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	物理ページヘッダサイズ。[byte]
//
// static
const PageSize
AreaManagePageHeader::SmallSize =
	(sizeof(ShortPageSize) << 1) +
	sizeof(ShortPageOffset) +
	sizeof(ShortAreaNum);

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::SmallUnuseAreaSizeOffset --
//		「未使用領域サイズ」のオフセット
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「未使用領域サイズ」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::SmallUnuseAreaSizeOffset = 0;

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::SmallFreeAreaSizeOffset --
//		「空き領域サイズ」のオフセット
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「空き領域サイズ」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::SmallFreeAreaSizeOffset =
	AreaManagePageHeader::SmallUnuseAreaSizeOffset +
	sizeof(ShortPageSize);

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::SmallFreeAreaSizeOffset --
//		「空き領域オフセット」のオフセット
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「空き領域オフセット」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::SmallFreeAreaOffsetOffset =
	AreaManagePageHeader::SmallFreeAreaSizeOffset +
	sizeof(ShortPageSize);

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::SmallFreeAreaSizeOffset --
//		「管理している物理エリア数」のオフセット
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「管理している物理エリア数」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::SmallManageAreaNumOffset =
	AreaManagePageHeader::SmallFreeAreaOffsetOffset +
	sizeof(ShortPageOffset);

//
//	CONST public
//	PhysicalFile::AreaManagePageHeader::LargeSize --
//		物理ページヘッダサイズ
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	物理ページヘッダサイズ。[byte]
//
// static
const PageSize
AreaManagePageHeader::LargeSize =
	(sizeof(PageSize) << 1) +
	sizeof(PageOffset) +
	sizeof(ShortAreaNum); // ※ 「管理している物理エリア数」は、
	                      // 　 どんなファイルでも2バイトで記録する。

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::LargeUnuseAreaSizeOffset --
//		「未使用領域サイズ」のオフセット
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「未使用領域サイズ」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::LargeUnuseAreaSizeOffset = 0;

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::LargeFreeAreaSizeOffset --
//		「空き領域サイズ」のオフセット
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「空き領域サイズ」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::LargeFreeAreaSizeOffset =
	AreaManagePageHeader::LargeUnuseAreaSizeOffset +
	sizeof(PageSize);

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::LargeFreeAreaSizeOffset --
//		「空き領域オフセット」のオフセット
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「空き領域オフセット」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::LargeFreeAreaOffsetOffset =
	AreaManagePageHeader::LargeFreeAreaSizeOffset +
	sizeof(PageSize);

//
//	CONST private
//	PhysicalFile::AreaManagePageHeader::LargeFreeAreaSizeOffset --
//		「管理している物理エリア数」のオフセット
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	物理ページヘッダに記録されている
//	「管理している物理エリア数」のオフセット。 [byte]
//
// static
const PageOffset
AreaManagePageHeader::LargeManageAreaNumOffset =
	AreaManagePageHeader::LargeFreeAreaOffsetOffset +
	sizeof(PageOffset);

namespace
{

namespace _SmallAreaManagePageHeader
{

//
//	FUNCTION
//	_SmallAreaManagePageHeader::overwrite --
//		物理ページヘッダを上書きする
//
//	NOTES
//	物理ページヘッダを上書きする。
//
//	ARGUMENTS
//	void*											HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::AreaManagePageHeader::Item&	Item_
//		物理ページヘッダに記録されている項目の構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwrite(void*								HeaderTop_,
		  const AreaManagePageHeader::Item&	Item_)
{
	ShortPageSize*	pageSizeWritePos =
		static_cast<ShortPageSize*>(HeaderTop_);

	*pageSizeWritePos++ =
		static_cast<ShortPageSize>(Item_.m_UnuseAreaSize);

	*pageSizeWritePos =
		static_cast<ShortPageSize>(Item_.m_FreeAreaSize);

	ShortPageOffset*	pageOffsetWritePos =
		syd_reinterpret_cast<ShortPageOffset*>(pageSizeWritePos + 1);

	*pageOffsetWritePos = Item_.m_FreeAreaOffset;

	ShortAreaNum*	areaNumWritePos =
		syd_reinterpret_cast<ShortAreaNum*>(pageOffsetWritePos + 1);

	*areaNumWritePos = Item_.m_ManageAreaNum;
}

//
//	FUNCTION
//	_SmallAreaManagePageHeader::overwriteUnuseAreaSize --
//		未使用領域サイズを上書きする
//
//	NOTES
//	未使用領域サイズを上書きする。
//
//	ARGUMENTS
//	void*							HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::PageSize	UnuseAreaSize_
//		未使用領域サイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwriteUnuseAreaSize(void*			HeaderTop_,
					   const PageSize	UnuseAreaSize_)
{
	ShortPageSize*	pageSizeWritePos =
		static_cast<ShortPageSize*>(HeaderTop_);

	*pageSizeWritePos = static_cast<ShortPageSize>(UnuseAreaSize_);
}

//
//	FUNCTION
//	_SmallAreaManagePageHeader::overwriteFreeAreaSize --
//		空き領域サイズを上書きする
//
//	NOTES
//	空き領域サイズを上書きする。
//
//	ARGUMENTS
//	void*							HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::PageSize	FreeAreaSize_
//		空き領域サイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
void
overwriteFreeAreaSize(void*				HeaderTop_,
					  const PageSize	FreeAreaSize_)
{
	ShortPageSize*	pageSizeWritePos =
		syd_reinterpret_cast<ShortPageSize*>(
			static_cast<char*>(HeaderTop_) +
			AreaManagePageHeader::SmallFreeAreaSizeOffset);

	*pageSizeWritePos = static_cast<ShortPageSize>(FreeAreaSize_);
}
#endif // OBSOLETE

//
//	FUNCTION
//	_SmallAreaManagePageHeader::overwriteFreeAreaOffset --
//		空き領域オフセットを上書きする
//
//	NOTES
//	空き領域オフセットを上書きする。
//
//	ARGUMENTS
//	void*							HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::PageOffset	FreeAreaOffset_
//			空き領域オフセット [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
void
overwriteFreeAreaOffset(void*				HeaderTop_,
						const PageOffset	FreeAreaOffset_)
{
	ShortPageOffset*	pageOffsetWritePos =
		syd_reinterpret_cast<ShortPageOffset*>(
			static_cast<char*>(HeaderTop_) +
			AreaManagePageHeader::SmallFreeAreaOffsetOffset);

	*pageOffsetWritePos = FreeAreaOffset_;
}
#endif // OBSOLETE

//
//	FUNCTION
//	_SmallAreaManagePageHeader::updateManageAreaNum --
//		管理している物理エリア数を更新する
//
//	NOTES
//	管理している物理エリア数を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const bool	Increment_
//		インクリメントするか、デクリメントするか
//			true  : インクリメント
//			false : デクリメント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
void
updateManageAreaNum(void*		HeaderTop_,
					const bool	Increment_)
{
	ShortAreaNum*	areaNumWritePos =
		syd_reinterpret_cast<ShortAreaNum*>(
			static_cast<char*>(HeaderTop_) +
			AreaManagePageHeader::SmallManageAreaNumOffset);

	if (Increment_)
	{
		(*areaNumWritePos)++;
	}
	else
	{
		; _SYDNEY_ASSERT(*areaNumWritePos > 0);

		(*areaNumWritePos)--;
	}
}
#endif // OBSOLETE

//
//	FUNCTION
//	_SmallAreaManagePageHeader::fetchOut --
//		物理ページヘッダに記録されているすべての項目を取り出す
//
//	NOTES
//	物理ページヘッダに記録されているすべての項目を取り出す。
//
//	ARGUMENTS
//	const void*									HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	PhysicalFile::AreaManagePageHeader::Item&	Item_
//		物理ページヘッダに記録されている項目の構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOut(const void*					HeaderTop_,
		 AreaManagePageHeader::Item&	Item_)
{
	const ShortPageSize*	pageSizeReadPos =
		static_cast<const ShortPageSize*>(HeaderTop_);

	Item_.m_UnuseAreaSize = *pageSizeReadPos++;

	Item_.m_FreeAreaSize = *pageSizeReadPos;

	const ShortPageOffset*	pageOffsetReadPos =
		syd_reinterpret_cast<const ShortPageOffset*>(
			pageSizeReadPos + 1);

	Item_.m_FreeAreaOffset = *pageOffsetReadPos;

	const ShortAreaNum*	areaNumReadPos =
		syd_reinterpret_cast<const ShortAreaNum*>(
			pageOffsetReadPos + 1);

	Item_.m_ManageAreaNum = *areaNumReadPos;
}

//
//	FUNCTION
//	_SmallAreaManagePageHeader::fetchOutAreaSize --
//		未使用領域サイズ・空き領域サイズを取り出す
//
//	NOTES
//	未使用領域サイズ・空き領域サイズを取り出す。
//
//	ARGUMENTS
//	const void*				HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	PhysicalFile::PageSize&	UnuseAreaSize_
//		未使用領域サイズへの参照 [byte]
//	PhysicalFile::PageSize&	FreeAreaSize_
//		空き領域サイズへの参照 [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutAreaSize(const void*	HeaderTop_,
				 PageSize&		UnuseAreaSize_,
				 PageSize&		FreeAreaSize_)
{
	const ShortPageSize*	pageSizeReadPos =
		static_cast<const ShortPageSize*>(HeaderTop_);

	UnuseAreaSize_ = *pageSizeReadPos++;

	FreeAreaSize_ = *pageSizeReadPos;
}

//
//	FUNCTION
//	_SmallAreaManagePageHeader::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	未使用領域サイズを返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageSize
//		未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
getUnuseAreaSize(const void*	HeaderTop_)
{
	const ShortPageSize*	pageSizeReadPos =
		static_cast<const ShortPageSize*>(HeaderTop_);

	return *pageSizeReadPos;
}

//
//	FUNCTION
//	_SmallAreaManagePageHeader::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	空き領域サイズを返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageSize
//		空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
getFreeAreaSize(const void*	HeaderTop_)
{
	const ShortPageSize*	pageSizeReadPos =
		syd_reinterpret_cast<const ShortPageSize*>(
			static_cast<const char*>(HeaderTop_) +
			AreaManagePageHeader::SmallFreeAreaSizeOffset);

	return *pageSizeReadPos;
}

//
//	FUNCTION
//	_SmallAreaManagePageHeader::getFreeAreaOffset --
//		空き領域オフセットを返す
//
//	NOTES
//	空き領域オフセットを返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageOffset
//		空き領域オフセット [byte]
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
PageOffset
getFreeAreaOffset(const void*	HeaderTop_)
{
	const ShortPageOffset*	pageOffsetReadPos =
		syd_reinterpret_cast<const ShortPageOffset*>(
			static_cast<const char*>(HeaderTop_) +
			AreaManagePageHeader::SmallFreeAreaOffsetOffset);

	return *pageOffsetReadPos;
}
#endif // OBSOLETE

//
//	FUNCTION
//	_SmallAreaManagePageHeader::getManageAreaNum --
//		管理している物理エリア数を返す
//
//	NOTES
//	管理している物理エリア数を返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::AreaNum
//		管理している物理エリア数
//
//	EXCEPTIONS
//	なし
//
AreaNum
getManageAreaNum(const void*	HeaderTop_)
{
	const ShortAreaNum*	areaNumReadPos =
		syd_reinterpret_cast<const ShortAreaNum*>(
			static_cast<const char*>(HeaderTop_) +
			AreaManagePageHeader::SmallManageAreaNumOffset);

	return *areaNumReadPos;
}

} // end of namespace _SmallAreaManagePageHeader

namespace _LargeAreaManagePageHeader
{

//
//	FUNCTION
//	_LargeAreaManagePageHeader::overwrite --
//		物理ページヘッダを上書きする
//
//	NOTES
//	物理ページヘッダを上書きする。
//
//	ARGUMENTS
//	void*											HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::AreaManagePageHeader::Item&	Item_
//		物理ページヘッダに記録されている項目の構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwrite(void*								HeaderTop_,
		  const AreaManagePageHeader::Item&	Item_)
{
	PageSize*	pageSizeWritePos =
		static_cast<PageSize*>(HeaderTop_);

	*pageSizeWritePos++ = Item_.m_UnuseAreaSize;

	*pageSizeWritePos = Item_.m_FreeAreaSize;

	PageOffset*	pageOffsetWritePos =
		syd_reinterpret_cast<PageOffset*>(pageSizeWritePos + 1);

	*pageOffsetWritePos = Item_.m_FreeAreaOffset;

	ShortAreaNum*	areaNumWritePos =
		syd_reinterpret_cast<ShortAreaNum*>(pageOffsetWritePos + 1);

	*areaNumWritePos = Item_.m_ManageAreaNum;
}

//
//	FUNCTION
//	_LargeAreaManagePageHeader::overwriteUnuseAreaSize --
//		未使用領域サイズを上書きする
//
//	NOTES
//	未使用領域サイズを上書きする。
//
//	ARGUMENTS
//	void*							HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::PageSize	UnuseAreaSize_
//		未使用領域サイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
overwriteUnuseAreaSize(void*			HeaderTop_,
					   const PageSize	UnuseAreaSize_)
{
	PageSize*	pageSizeWritePos =
		static_cast<PageSize*>(HeaderTop_);

	*pageSizeWritePos = UnuseAreaSize_;
}

//
//	FUNCTION
//	_LargeAreaManagePageHeader::overwriteFreeAreaSize --
//		空き領域サイズを上書きする
//
//	NOTES
//	空き領域サイズを上書きする。
//
//	ARGUMENTS
//	void*							HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::PageSize	FreeAreaSize_
//		空き領域サイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
void
overwriteFreeAreaSize(void*				HeaderTop_,
					  const PageSize	FreeAreaSize_)
{
	PageSize*	pageSizeWritePos =
		syd_reinterpret_cast<PageSize*>(
			static_cast<char*>(HeaderTop_) +
			AreaManagePageHeader::LargeFreeAreaSizeOffset);

	*pageSizeWritePos = FreeAreaSize_;
}
#endif // OBSOLETE

//
//	FUNCTION
//	_LargeAreaManagePageHeader::overwriteFreeAreaOffset --
//		空き領域オフセットを上書きする
//
//	NOTES
//	空き領域オフセットを上書きする。
//
//	ARGUMENTS
//	void*							HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const PhysicalFile::PageOffset	FreeAreaOffset_
//			空き領域オフセット [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
void
overwriteFreeAreaOffset(void*				HeaderTop_,
						const PageOffset	FreeAreaOffset_)
{
	PageOffset*	pageOffsetWritePos =
		syd_reinterpret_cast<PageOffset*>(
			static_cast<char*>(HeaderTop_) +
			AreaManagePageHeader::LargeFreeAreaOffsetOffset);

	*pageOffsetWritePos = FreeAreaOffset_;
}
#endif // OBSOLETE

//
//	FUNCTION
//	_LargeAreaManagePageHeader::updateManageAreaNum --
//		管理している物理エリア数を更新する
//
//	NOTES
//	管理している物理エリア数を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	const bool	Increment_
//		インクリメントするか、デクリメントするか
//			true  : インクリメント
//			false : デクリメント
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
void
updateManageAreaNum(void*		HeaderTop_,
					const bool	Increment_)
{
	ShortAreaNum*	areaNumWritePos =
		syd_reinterpret_cast<ShortAreaNum*>(
			static_cast<char*>(HeaderTop_) +
			AreaManagePageHeader::LargeManageAreaNumOffset);

	if (Increment_)
	{
		(*areaNumWritePos)++;
	}
	else
	{
		; _SYDNEY_ASSERT(*areaNumWritePos > 0);

		(*areaNumWritePos)--;
	}
}
#endif // OBSOLETE

//
//	FUNCTION
//	_LargeAreaManagePageHeader::fetchOut --
//		物理ページヘッダに記録されているすべての項目を取り出す
//
//	NOTES
//	物理ページヘッダに記録されているすべての項目を取り出す。
//
//	ARGUMENTS
//	const void*									HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	PhysicalFile::AreaManagePageHeader::Item&	Item_
//		物理ページヘッダに記録されている項目の構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOut(const void*					HeaderTop_,
		 AreaManagePageHeader::Item&	Item_)
{
	const PageSize*	pageSizeReadPos =
		static_cast<const PageSize*>(HeaderTop_);

	Item_.m_UnuseAreaSize = *pageSizeReadPos++;

	Item_.m_FreeAreaSize = *pageSizeReadPos;

	const PageOffset*	pageOffsetReadPos =
		syd_reinterpret_cast<const PageOffset*>(pageSizeReadPos + 1);

	Item_.m_FreeAreaOffset = *pageOffsetReadPos;

	const ShortAreaNum*	areaNumReadPos =
		syd_reinterpret_cast<const ShortAreaNum*>(
			pageOffsetReadPos + 1);

	Item_.m_ManageAreaNum = *areaNumReadPos;
}

//
//	FUNCTION
//	_LargeAreaManagePageHeader::fetchOutAreaSize --
//		未使用領域サイズ・空き領域サイズを取り出す
//
//	NOTES
//	未使用領域サイズ・空き領域サイズを取り出す。
//
//	ARGUMENTS
//	const void*				HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//	PhysicalFile::PageSize&	UnuseAreaSize_
//		未使用領域サイズへの参照 [byte]
//	PhysicalFile::PageSize&	FreeAreaSize_
//		空き領域サイズへの参照 [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOutAreaSize(const void*	HeaderTop_,
				 PageSize&		UnuseAreaSize_,
				 PageSize&		FreeAreaSize_)
{
	const PageSize*	pageSizeReadPos =
		static_cast<const PageSize*>(HeaderTop_);

	UnuseAreaSize_ = *pageSizeReadPos++;

	FreeAreaSize_ = *pageSizeReadPos;
}

//
//	FUNCTION
//	_LargeAreaManagePageHeader::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	未使用領域サイズを返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageSize
//		未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
getUnuseAreaSize(const void*	HeaderTop_)
{
	const PageSize*	pageSizeReadPos =
		static_cast<const PageSize*>(HeaderTop_);

	return *pageSizeReadPos;
}

//
//	FUNCTION
//	_LargeAreaManagePageHeader::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	空き領域サイズを返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageSize
//		空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
getFreeAreaSize(const void*	HeaderTop_)
{
	const PageSize*	pageSizeReadPos =
		syd_reinterpret_cast<const PageSize*>(
			static_cast<const char*>(HeaderTop_) +
			AreaManagePageHeader::LargeFreeAreaSizeOffset);

	return *pageSizeReadPos;
}

//
//	FUNCTION
//	_LargeAreaManagePageHeader::getFreeAreaOffset --
//		空き領域オフセットを返す
//
//	NOTES
//	空き領域オフセットを返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::PageOffset
//		空き領域オフセット [byte]
//
//	EXCEPTIONS
//	なし
//
#ifdef OBSOLETE
PageOffset
getFreeAreaOffset(const void*	HeaderTop_)
{
	const PageOffset*	pageOffsetReadPos =
		syd_reinterpret_cast<const PageOffset*>(
			static_cast<const char*>(HeaderTop_) +
			AreaManagePageHeader::LargeFreeAreaOffsetOffset);

	return *pageOffsetReadPos;
}
#endif // OBSOLETE

//
//	FUNCTION
//	_LargeAreaManagePageHeader::getManageAreaNum --
//		管理している物理エリア数を返す
//
//	NOTES
//	管理している物理エリア数を返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページヘッダ先頭へのポインタ
//
//	RETURN
//	PhysicalFile::AreaNum
//		管理している物理エリア数
//
//	EXCEPTIONS
//	なし
//
AreaNum
getManageAreaNum(const void*	HeaderTop_)
{
	const ShortAreaNum*	areaNumReadPos =
		syd_reinterpret_cast<const ShortAreaNum*>(
			static_cast<const char*>(HeaderTop_) +
			AreaManagePageHeader::LargeManageAreaNumOffset);

	return *areaNumReadPos;
}

} // end of namespace _LargeAreaManagePageHeader

} // end of global namespace

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManagePageHeaderクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::AreaManagePageHeader::AreaManagePageHeader --
//		コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const PhysicalFile::PageSize	VersionPageSize_
//		バージョンページサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
AreaManagePageHeader::AreaManagePageHeader(
	const PageSize VersionPageSize_)
{
	; _SYDNEY_ASSERT(VersionPageSize_ != 0);

	if ((VersionPageSize_ & ConstValue::PageSizeUpperMask) > 0)
	{
		// 指定サイズが65536バイトを超えた（ヘッダの要素は4バイト）

		this->m_Type = AreaManagePageHeader::LargeType;

		this->m_Size = AreaManagePageHeader::LargeSize;
	}
	else
	{
		// 指定サイズが65536バイトを超えなかった（ヘッダの要素は2バイト）

		this->m_Type = AreaManagePageHeader::SmallType;

		this->m_Size = AreaManagePageHeader::SmallSize;
	}

	this->setFunction();
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePageHeader::~AreaManagePageHeader --
//		デストラクタ
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
AreaManagePageHeader::~AreaManagePageHeader()
{
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePageHeader::initialize --
//		物理ページヘッダを初期化する
//
//	NOTES
//	空き領域管理機能付き物理ファイル内の物理ページヘッダを初期化する。
//
//	ARGUMENTS
//	void*							PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::PageSize	VersionPageSize_
//		バージョンページサイズ [byte]
//	const PhysicalFile::PageSize	VersionPageDataSize_
//		バージョンページデータサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
AreaManagePageHeader::initialize(void*			PagePointer_,
								 const PageSize	VersionPageSize_,
								 const PageSize	VersionPageDataSize_)
{
	PageSize	headerSize = 0;

	if ((VersionPageSize_ & ConstValue::PageSizeUpperMask) > 0)
	{
		headerSize = AreaManagePageHeader::LargeSize;
	}
	else
	{
		headerSize = AreaManagePageHeader::SmallSize;
	}

	AreaManagePageHeader::Item	headerItem;

	// 未使用領域サイズ
	headerItem.m_UnuseAreaSize = VersionPageDataSize_ - headerSize;

	// 空き領域サイズ
	// ※ 初期状態では、未使用領域サイズと等しい
	headerItem.m_FreeAreaSize = headerItem.m_UnuseAreaSize;

	// 空き領域オフセット
	headerItem.m_FreeAreaOffset = headerSize;

	// 管理している物理エリア数
	headerItem.m_ManageAreaNum = 0;

	AreaManagePageHeader	pageHeader(VersionPageSize_);

	(*(pageHeader.Overwrite))(PagePointer_, headerItem);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePageHeader::getSize --
//		物理ページヘッダサイズを返す
//
//	NOTES
//	物理ページヘッダサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページヘッダサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePageHeader::getSize() const
{
	return
		(this->m_Type == AreaManagePageHeader::SmallType) ?
			AreaManagePageHeader::SmallSize :
			AreaManagePageHeader::LargeSize;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePageHeader::getSize --
//		物理ページヘッダサイズを返す
//
//	NOTES
//	PhysicalFile::PageSize	VersionPageSize_
//		バージョンページサイズ [byte]
//	物理ページヘッダサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページヘッダサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePageHeader::getSize(PageSize VersionPageSize_)
{
	if ((VersionPageSize_ & ConstValue::PageSizeUpperMask) > 0)
	{
		return AreaManagePageHeader::LargeSize;
	}
	else
	{
		return AreaManagePageHeader::SmallSize;
	}
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePageHeader::getSize --
//		物理ページヘッダサイズを返す
//
//	NOTES
//	PhysicalFile::PageSize	VersionPageSize_
//		バージョンページサイズ [byte]
//	物理ページヘッダサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページヘッダサイズ [byte]
//
//	EXCEPTIONS
//	なし
//
AreaNum
AreaManagePageHeader::getManageAreaNum(PageSize VersionPageSize_,
									   const void* HeaderTop_)
{
	if ((VersionPageSize_ & ConstValue::PageSizeUpperMask) > 0)
	{
		return _LargeAreaManagePageHeader::getManageAreaNum(HeaderTop_);
	}
	else
	{
		return _SmallAreaManagePageHeader::getManageAreaNum(HeaderTop_);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManagePageHeaderクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::AreaManagePageHeader::setFunction --
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
AreaManagePageHeader::setFunction()
{
	if (this->m_Type == AreaManagePageHeader::SmallType)
	{
		// 各項目を2バイトで記録するタイプ…

		this->Overwrite =
			_SmallAreaManagePageHeader::overwrite;

		this->OverwriteUnuseAreaSize =
			_SmallAreaManagePageHeader::overwriteUnuseAreaSize;

#ifdef OBSOLETE
		this->OverwriteFreeAreaSize =
			_SmallAreaManagePageHeader::overwriteFreeAreaSize;
#endif // OBSOLETE

#ifdef OBSOLETE
		this->OverwriteFreeAreaOffset =
			_SmallAreaManagePageHeader::overwriteFreeAreaOffset;
#endif // OBSOLETE

#ifdef OBSOLETE
		this->UpdateManageAreaNum =
			_SmallAreaManagePageHeader::updateManageAreaNum;
#endif // OBSOLETE

		this->FetchOut =
			_SmallAreaManagePageHeader::fetchOut;

		this->FetchOutAreaSize =
			_SmallAreaManagePageHeader::fetchOutAreaSize;

		this->GetUnuseAreaSize =
			_SmallAreaManagePageHeader::getUnuseAreaSize;

		this->GetFreeAreaSize =
			_SmallAreaManagePageHeader::getFreeAreaSize;

#ifdef OBSOLETE
		this->GetFreeAreaOffset =
			_SmallAreaManagePageHeader::getFreeAreaOffset;
#endif // OBSOLETE

		this->GetManageAreaNum =
			_SmallAreaManagePageHeader::getManageAreaNum;
	}
	else
	{
		// 各項目を4バイトで記録するタイプ…

		this->Overwrite =
			_LargeAreaManagePageHeader::overwrite;

		this->OverwriteUnuseAreaSize =
			_LargeAreaManagePageHeader::overwriteUnuseAreaSize;

#ifdef OBSOLETE
		this->OverwriteFreeAreaSize =
			_LargeAreaManagePageHeader::overwriteFreeAreaSize;
#endif // OBSOLETE

#ifdef OBSOLETE
		this->OverwriteFreeAreaOffset =
			_LargeAreaManagePageHeader::overwriteFreeAreaOffset;
#endif // OBSOLETE

#ifdef OBSOLETE
		this->UpdateManageAreaNum =
			_LargeAreaManagePageHeader::updateManageAreaNum;
#endif // OBSOLETE

		this->FetchOut =
			_LargeAreaManagePageHeader::fetchOut;

		this->FetchOutAreaSize =
			_LargeAreaManagePageHeader::fetchOutAreaSize;

		this->GetUnuseAreaSize =
			_LargeAreaManagePageHeader::getUnuseAreaSize;

		this->GetFreeAreaSize =
			_LargeAreaManagePageHeader::getFreeAreaSize;

#ifdef OBSOLETE
		this->GetFreeAreaOffset =
			_LargeAreaManagePageHeader::getFreeAreaOffset;
#endif // OBSOLETE

		this->GetManageAreaNum =
			_LargeAreaManagePageHeader::getManageAreaNum;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManagePageクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::allocateArea --
//		物理エリアを確保する
//
//	NOTES
//	物理エリアを確保する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaSize	AreaSize_
//		確保する物理エリアのサイズ [byte]
//	const bool						WithCompaction_ = false
//		物理ページ内に物理エリアを確保するために十分な空き領域が
//		存在しない場合に、物理エリアの再配置(compaction)を
//		行うかどうか。
//			true  : 物理エリアの再配置を行う
//			false : 物理エリアの再配置を行わない
//
//	RETURN
//	PhysicalFile::AreaID
//		確保した物理エリアの識別子
//
//	EXCEPTIONS
//	BadArgument
//		物理ページ内に物理エリアを確保するために
//		十分な空き領域が存在しない
//	FileManipulateError
//		物理エリアを確保できなかった
//
AreaID
AreaManagePage::allocateArea(const Trans::Transaction&	Transaction_,
							 const AreaSize				AreaSize_,
							 const bool					WithCompaction_)
{
	//
	// 物理ページヘッダに記録されている
	// 　1. 未使用領域サイズ
	// 　2. 空き領域サイズ
	// 　3. 空き領域オフセット
	// 　4. 管理している物理エリア数
	// を取り出す
	//

	AreaManagePageHeader::Item	headerItem;

	(*(this->m_Header.FetchOut))(this->m_VersionPageTop, headerItem);

#ifdef DEBUG

	//
	// v5.0から、1つの物理ページ内には、物理エリアを65535個しか
	// 生成しないという仕様になった。（識別子は0〜65534(0xFFFE)）
	// そして、このことに違反しようとする場合はアサートとする。
	//

	if (headerItem.m_ManageAreaNum == 0xFFFF)
	{
		// 既に物理ページ内で65535個の物理エリアを管理している…

		//
		// しかし、未使用の物理エリアが存在するかもしれないので
		// 上のチェックだけではアボートしない。
		//

		//
		// 上書き可能な物理エリア情報が存在するかどうか
		// チェックする。
		// 上書き可能な物理エリア情報が存在しない場合に
		// アボートする。
		//

		; _SYDNEY_ASSERT(
			m_Directory.getOverwriteAreaID(this->m_VersionPageTop,
										   headerItem.m_ManageAreaNum)
			!= ConstValue::UndefinedAreaID);
	}

#endif


	// 物理エリア確保前の、物理ページ内の未使用領域率を得る
	unsigned int		beforeUnuseAreaRateValue = 0;
	BitmapTable::Rate	beforeUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_UnuseAreaSize,
							beforeUnuseAreaRateValue,
							beforeUnuseAreaRate);

	// 物理エリア確保前の、物理ページ内の空き領域率を得る
	unsigned int		beforeFreeAreaRateValue = 0;
	BitmapTable::Rate	beforeFreeAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_FreeAreaSize,
							beforeFreeAreaRateValue,
							beforeFreeAreaRate);

	//
	// 物理ページ内に含まれる物理エリア情報のうち、
	// 上書き可能な物理エリア情報が存在するのならば、
	// その物理エリア情報のインデックス（つまり物理エリア識別子）を
	// 取得する。
	// 上書き可能な物理エリア情報が存在しないのならば、
	// Area::Directory::getOverwriteAreaIDが
	// ConstValue::UndefinedAreaIDを返す。
	//
	// “上書き可能な物理エリア情報”とは、
	// 物理エリアの再配置(compaction)によって、
	// 再利用ができなくなった物理エリアに関する物理エリア情報のこと。
	//
	AreaID	overwriteAreaID =
		m_Directory.getOverwriteAreaID(this->m_VersionPageTop,
									   headerItem.m_ManageAreaNum);

	// 物理エリア管理ディレクトリに1つ物理エリア情報を追加した場合の
	// 差分サイズを求める
	PageSize	diffSize =
		(overwriteAreaID == ConstValue::UndefinedAreaID) ?
			m_Directory.getSize(headerItem.m_ManageAreaNum + 1) -
			m_Directory.getSize(headerItem.m_ManageAreaNum) :
			0;

	if (headerItem.m_FreeAreaSize < diffSize)
	{
		// 物理ページ内に物理エリアを確保するために
		// 十分な空き領域が存在しない…

		if (WithCompaction_ &&
			headerItem.m_UnuseAreaSize != headerItem.m_FreeAreaSize)
		{
			// ★
			// 必要ならば物理エリアの再配置をするように指示されて、
			// かつ、再配置することに意味のある物理ページ…
			// （物理ページ内の“未使用領域サイズ”と“空き領域サイズ”が
			// 　同じならば再配置は無意味である。）

			// まず、物理エリアの再配置をして
			this->compaction(Transaction_, this->m_VersionPageTop);

			// 再帰呼び出しをする
			return this->allocateArea(Transaction_,
									  AreaSize_,
									  false); // もう物理エリアの再配置は不要
		}

		// 物理ページ内に十分な領域が存在しない
		return PhysicalFile::ConstValue::UndefinedAreaID;
	}

	// 「未使用領域サイズ」と「空き領域サイズ」を差分サイズ分更新する
	headerItem.m_UnuseAreaSize -= diffSize;
	headerItem.m_FreeAreaSize -= diffSize;

	if (AreaSize_ > headerItem.m_FreeAreaSize)
	{
		// 物理ページ内に物理エリアを確保するために
		// 十分な空き領域が存在しない…

		if (WithCompaction_ &&
			headerItem.m_UnuseAreaSize != headerItem.m_FreeAreaSize)
		{
			// 上の“★”と同じ処理

			this->compaction(Transaction_, this->m_VersionPageTop);

			return this->allocateArea(Transaction_,
									  AreaSize_,
									  false); // もう物理エリアの再配置は不要
		}

		// 物理ページ内に十分な領域が存在しない
		return PhysicalFile::ConstValue::UndefinedAreaID;
	}

	// 物理エリア情報に確保する物理エリアのオフセットを設定する
	Area::Information	areaInformation;
	areaInformation.m_Offset = headerItem.m_FreeAreaOffset;

	// 物理エリア情報に物理エリアサイズを設定する
	areaInformation.m_Size = AreaSize_;

	// 確保する物理エリアの識別子を求める
	AreaID	areaID =
		(overwriteAreaID == ConstValue::UndefinedAreaID) ?
			headerItem.m_ManageAreaNum : overwriteAreaID;

	// 物理ページヘッダを更新する
	headerItem.m_UnuseAreaSize -= AreaSize_;
	headerItem.m_FreeAreaSize -= AreaSize_;
	headerItem.m_FreeAreaOffset += AreaSize_;
	if (overwriteAreaID == ConstValue::UndefinedAreaID)
	{
		// 上書き可能な物理エリア情報がなかった…

		headerItem.m_ManageAreaNum++;
	}

	//
	// 物理ページ内の更新は、ここではまだしない。
	//

	// 物理エリア確保後の、物理ページ内の未使用領域率を得る
	unsigned int		afterUnuseAreaRateValue = 0;
	BitmapTable::Rate	afterUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_UnuseAreaSize,
							afterUnuseAreaRateValue,
							afterUnuseAreaRate);

	// 物理エリア確保後の、物理ページ内の空き領域率を得る
	unsigned int		afterFreeAreaRateValue = 0;
	BitmapTable::Rate	afterFreeAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_FreeAreaSize,
							afterFreeAreaRateValue,
							afterFreeAreaRate);

	if (beforeUnuseAreaRate != afterUnuseAreaRate ||
		beforeFreeAreaRate != afterFreeAreaRate)
	{
		// 物理エリア確保により、
		// 空き領域管理表の更新も必要となった…

		// 物理ページ内を更新する前に
		// 空き領域管理表のバッファリング内容を得る（フィックスする）
		// ※ 物理ページ内を更新してしまった後で
		// 　 空き領域管理表をフィックスできなかった場合、
		// 　 物理ページ内を元に戻す処理が必要となるため。

		try
		{
			// 空き領域管理表のバッファリング内容を得る
			// （フィックスする）

			File::PagePointer table =
				m_File->fixVersionPage(
					Transaction_,
					this->m_File->getManageTableVersionPageID(this->m_ID),
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Middle);

			// 空き領域管理表のバッファリング内容へのポインタを得る
			void*	tablePointer = (void*)(*table);

			// 物理エリア確保後の領域率ビットマップの値を得る
			unsigned char	afterBitValue =
				BitmapTable::ToBitmapValue[afterUnuseAreaRate]
										  [afterFreeAreaRate];

			// 空き領域管理表内の領域率ビットマップを上書きする
			AreaManageTable::Bitmap::overwriteValue(
				tablePointer,
				this->m_TableHeader->m_Type,
				this->m_ID,
				this->m_File->getPagePerManageTable(),
				afterBitValue);

			if (beforeUnuseAreaRate != afterUnuseAreaRate)
			{
				// 物理ページ内の未使用領域率が変更された…

				// 物理エリア確保前の空き領域管理表ヘッダ内の
				// 「未使用領域率別の物理ページ数配列」を更新する
				this->m_TableHeader->updatePageArray(
					tablePointer,
					true, // 未使用領域率別の物理ページ数配列
					beforeUnuseAreaRateValue,
					afterUnuseAreaRateValue);
			}

			if (beforeFreeAreaRate != afterFreeAreaRate)
			{
				// 物理ページ内の空き領域率が変更された…

				// 物理エリア確保前の空き領域管理表ヘッダ内の
				// 「空き領域率別の物理ページ数配列」を更新する
				this->m_TableHeader->updatePageArray(
					tablePointer,
					false, // 空き領域率別の物理ページ数配列
					beforeFreeAreaRateValue,
					afterFreeAreaRateValue);
			}

			//
			// 空き領域管理表を更新したので、
			// 利用者側でエラーを検出した場合には
			// 空き領域管理表の修復が必要となった。
			//

			this->m_NecessaryRecoverTable = true;
		}
		catch (Exception::Object&)
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			_SYDNEY_RETHROW;
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			throw Exception::FileManipulateError(moduleName,
												 srcFile,
												 __LINE__);
		}
#endif
	}

	//
	// ここからが物理ページ内の更新
	//

	if (overwriteAreaID == ConstValue::UndefinedAreaID)
	{
		// 上書き可能な物理エリア情報がなかった…

		// 物理エリア管理ディレクトリに物理エリア情報を追加する
		m_Directory.appendInformation(this->m_VersionPageTop,
									  areaInformation);
	}
	else
	{
		// 上書き可能な物理エリア情報があった…

		; _SYDNEY_ASSERT(areaID == overwriteAreaID);

		m_Directory.overwriteInfo(this->m_VersionPageTop,
								  areaID,
								  areaInformation);
	}

	// 物理ページヘッダを上書きする
	(*(this->m_Header.Overwrite))(this->m_VersionPageTop,
								  headerItem);

	// 物理エリア使用状態ビットマップのビットをONにする
	m_Directory.overwriteBitValue(this->m_VersionPageTop,
								  areaID,
								  true);

	return areaID;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::freeArea --
//		物理エリアを解放する
//
//	NOTES
//	物理エリアを解放する。
//	解放された物理エリアの領域は、
//	関数compactionを呼び出して物理エリアの再配置を行わない限り、
//	利用されることはない。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション識別子への参照
//	PhysicalFile::AreaID		AreaID_
//		解放する物理エリアの識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileManipulateError
//		物理エリアを解放できなかった
//
void
AreaManagePage::freeArea(const Trans::Transaction&	Transaction_,
						 AreaID						AreaID_)
{
	// 物理ページヘッダに記録されている
	// 　1. 未使用領域サイズ
	// 　2. 空き領域サイズ
	// を取り出す
	PageSize	unuseAreaSize = 0;
	PageSize	freeAreaSize = 0;
	(*(this->m_Header.FetchOutAreaSize))(this->m_VersionPageTop,
										 unuseAreaSize,
										 freeAreaSize);

	// 物理エリア解放前の、物理ページ内の未使用領域率を得る
	unsigned int		beforeUnuseAreaRateValue = 0;
	BitmapTable::Rate	beforeUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							beforeUnuseAreaRateValue,
							beforeUnuseAreaRate);

	// 未使用領域サイズを更新する
	unuseAreaSize += m_Directory.getAreaSize(this->m_VersionPageTop,
											 AreaID_);

	//
	// 物理ページ内の更新は、ここではまだしない。
	//

	// 物理エリア解放後の、物理ページ内の未使用領域率を得る
	unsigned int	afterUnuseAreaRateValue = 0;
	BitmapTable::Rate	afterUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							afterUnuseAreaRateValue,
							afterUnuseAreaRate);

	if (beforeUnuseAreaRate != afterUnuseAreaRate)
	{
		// 物理エリア解放により、空き領域管理表の更新も必要となった…

		// 物理ページ内を更新する前に
		// 空き領域管理表のバッファリング内容を得る（フィックスする）
		// ※ 物理ページ内を更新してしまった後で
		// 　 空き領域管理表をフィックスできなかった場合、
		// 　 物理ページ内を元に戻す処理が必要となるため。

		try
		{
			// 空き領域管理表のバッファリング内容を得る
			// （フィックスする）

			File::PagePointer table =
				m_File->fixVersionPage(
					Transaction_,
					this->m_File->getManageTableVersionPageID(
						this->m_ID),
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Middle);

			// 空き領域管理表のバッファリング内容へのポインタを得る
			void*	tablePointer = (void*)(*table);

			// 物理エリア解放後の空き領域率を得る
			// ※ 物理エリア解放により、空き領域率は変わらないが、
			// 　 領域率ビットマップの値を得るために必要
			unsigned int		afterFreeAreaRateValue = 0;
			BitmapTable::Rate	afterFreeAreaRate =
				BitmapTable::InvalidRate;
			this->convertToAreaRate(freeAreaSize,
									afterFreeAreaRateValue,
									afterFreeAreaRate);

			// 物理エリア解放後の領域率ビットマップの値を得る
			unsigned char	afterBitValue =
				BitmapTable::ToBitmapValue[afterUnuseAreaRate]
										  [afterFreeAreaRate];

			// 空き領域管理表内の領域率ビットマップを上書きする
			AreaManageTable::Bitmap::overwriteValue(
				tablePointer,
				this->m_TableHeader->m_Type,
				this->m_ID,
				this->m_File->getPagePerManageTable(),
				afterBitValue);

			// 物理エリア解放前の空き領域管理表ヘッダ内の
			// 「未使用領域率別の物理ページ数配列」を更新する
			this->m_TableHeader->updatePageArray(
				tablePointer,
				true, // 未使用領域率別の物理ページ数配列
				beforeUnuseAreaRateValue,
				afterUnuseAreaRateValue);

			//
			// 空き領域管理表を更新したので、
			// 利用者側でエラーを検出した場合には
			// 空き領域管理表の修復が必要となった。
			//

			this->m_NecessaryRecoverTable = true;
		}
		catch (Exception::Object&)
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			_SYDNEY_RETHROW;
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			throw Exception::FileManipulateError(moduleName,
												 srcFile,
												 __LINE__);
		}
#endif
	}

	//
	// ここからが物理ページ内の更新
	//

	// 物理エリア管理ディレクトリ内の
	// 「物理エリア使用状態ビットマップ」のビットをOFFにする
	m_Directory.overwriteBitValue(this->m_VersionPageTop,
								  AreaID_,
								  false);

	// 物理ページヘッダ内の「未使用領域サイズ」を上書きする
	(*(this->m_Header.OverwriteUnuseAreaSize))(this->m_VersionPageTop,
											   unuseAreaSize);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::reuseArea --
//		物理エリアを再利用する
//
//	NOTES
//	関数PhysicalFile::Page::freeArea()により一度解放した物理エリアを
//	再利用する。
//	関数PhysicalFile::Page::compaction()を呼び出して
//	物理エリアの再配置を行うと、
//	物理エリアは再利用不可能となる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID	AreaID_
//		再利用する物理エリアの識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		再利用した物理エリアの識別子
//
//	EXCEPTIONS
//	BadArgument
//		再利用不可能な物理エリアの識別子が指定された
//	FileManipulateError
//		物理エリアを再利用できなかった
//
AreaID
AreaManagePage::reuseArea(const Trans::Transaction&	Transaction_,
						  const AreaID				AreaID_)
{
	// 物理エリア情報に記録されている
	// 　1. 物理エリアオフセット
	// 　2. 物理エリアサイズ
	// を取り出す
	Area::Information	areaInformation;
	m_Directory.fetchOutInfo(this->m_VersionPageTop,
							 AreaID_,
							 areaInformation);

	if (areaInformation.m_Offset == ConstValue::UndefinedAreaOffset ||
		areaInformation.m_Size == ConstValue::UndefinedAreaSize)
	{
		// 再利用不可能な物理エリアの識別子が指定された…

		// 不正な物理エリア識別子
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}

	// 物理ページヘッダに記録されている
	// 　1. 未使用領域サイズ
	// 　2. 空き領域サイズ
	// を取り出す
	PageSize	unuseAreaSize = 0;
	PageSize	freeAreaSize = 0;
	(*(this->m_Header.FetchOutAreaSize))(this->m_VersionPageTop,
										 unuseAreaSize,
										 freeAreaSize);

	// 物理エリア再利用前の、物理ページ内の未使用領域率を得る
	unsigned int		beforeUnuseAreaRateValue = 0;
	BitmapTable::Rate	beforeUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							beforeUnuseAreaRateValue,
							beforeUnuseAreaRate);

	; _SYDNEY_ASSERT(unuseAreaSize >= areaInformation.m_Size);

	// 未使用領域サイズを更新する
	unuseAreaSize -= areaInformation.m_Size;

	//
	// 物理ページ内の更新は、ここではまだしない。
	//

	// 物理エリア再利用後の、物理ページ内の未使用領域率を得る
	unsigned int		afterUnuseAreaRateValue = 0;
	BitmapTable::Rate	afterUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							afterUnuseAreaRateValue,
							afterUnuseAreaRate);

	if (beforeUnuseAreaRate != afterUnuseAreaRate)
	{
		// 物理エリア再利用により、空き領域管理表の更新も必要となった…

		// 物理ページ内を更新する前に
		// 空き領域管理表のバッファリング内容を得る（フィックスする）
		// ※ 物理ページ内を更新してしまった後で
		// 　 空き領域管理表をフィックスできなかった場合、
		// 　 物理ページ内を元に戻す処理が必要となるため。

		try
		{

			// 空き領域管理表のバッファリング内容を得る
			// （フィックスする）

			File::PagePointer table =
				m_File->fixVersionPage(
					Transaction_,
					this->m_File->getManageTableVersionPageID(this->m_ID),
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Middle);

			// 空き領域管理表のバッファリング内容へのポインタを得る
			void*	tablePointer = (void*)(*table);

			// 物理エリア再利用後の空き領域率を得る
			// ※ 物理エリア再利用により、空き領域率は変わらないが、
			// 　 領域率ビットマップの値を得るために必要
			unsigned int		afterFreeAreaRateValue = 0;
			BitmapTable::Rate	afterFreeAreaRate =
				BitmapTable::InvalidRate;
			this->convertToAreaRate(freeAreaSize,
									afterFreeAreaRateValue,
									afterFreeAreaRate);

			// 物理エリア再利用後の領域率ビットマップの値を得る
			unsigned char	afterBitValue =
				BitmapTable::ToBitmapValue[afterUnuseAreaRate]
										  [afterFreeAreaRate];

			// 空き領域管理表内の領域率ビットマップを上書きする
			AreaManageTable::Bitmap::overwriteValue(
				tablePointer,
				this->m_TableHeader->m_Type,
				this->m_ID,
				this->m_File->getPagePerManageTable(),
				afterBitValue);

			// 物理エリア再利用前の空き領域管理表ヘッダ内の
			// 「未使用領域率別の物理ページ数配列」を更新する
			this->m_TableHeader->updatePageArray(
				tablePointer,
				true, // 未使用領域率別の物理ページ数配列
				beforeUnuseAreaRateValue,
				afterUnuseAreaRateValue);

			//
			// 空き領域管理表を更新したので、
			// 利用者側でエラーを検出した場合には
			// 空き領域管理表の修復が必要となった。
			//

			this->m_NecessaryRecoverTable = true;
		}
		catch (Exception::Object&)
		{
			_SYDNEY_RETHROW;
		}
#ifndef NO_CATCH_ALL
		catch (...)
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			throw Exception::FileManipulateError(moduleName,
												 srcFile,
												 __LINE__);
		}
#endif
	}

	//
	// ここからが物理ページ内の更新
	//

	// 物理エリア管理ディレクトリ内の
	// 「物理エリア使用状態ビットマップ」のビットをONにする
	m_Directory.overwriteBitValue(this->m_VersionPageTop,
								  AreaID_,
								  true);

	// 物理ページヘッダ内の「未使用領域サイズ」を更新する
	(*(this->m_Header.OverwriteUnuseAreaSize))(this->m_VersionPageTop,
											   unuseAreaSize);

	return AreaID_;
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::writeArea --
//		物理エリアへデータを書き込む
//
//	NOTES
//	物理エリアへデータを書き込む。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_,
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID		AreaID_,
//		データを書き込む物理エリアの識別子
//	const void*						Buffer_,
//		書き込むデータが格納されているバッファへのポインタ
//	const PhysicalFile::AreaOffset	Offset_,
//		物理エリア内の書き込み開始位置 [byte]
//	const PhysicalFile::AreaSize	Size_
//		書き込みサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::writeArea(const Trans::Transaction&	Transaction_,
						  const AreaID				AreaID_,
						  const void*				Buffer_,
						  const AreaOffset			Offset_,
						  const AreaSize			Size_)
{
	// 物理エリア情報に記録されている内容を取り出す
	Area::Information	areaInformation;
	m_Directory.fetchOutInfo(this->m_VersionPageTop,
							 AreaID_,
							 areaInformation);

	; _SYDNEY_ASSERT(Offset_ >= 0 &&
					 (Offset_ + Size_ <= areaInformation.m_Size));

	// 物理エリアへのポインタを設定する
	char*	areaPointer =
		this->m_VersionPageTop + areaInformation.m_Offset + Offset_;

	// 物理エリアへデータを書き込む
	ModOsDriver::Memory::copy(areaPointer, Buffer_, Size_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::readArea --
//		物理エリアからデータを読み込む
//
//	NOTES
//	物理エリアからデータを読み込む。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_,
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID		AreaID_,
//		データを読み込む物理エリアの識別子
//	void*							Buffer_,
//		物理エリアから読み込んだデータを格納するバッファへのポインタ
//	const PhysicalFile::AreaOffset	Offset_,
//		物理エリア内の読み込み開始位置 [byte]
//	const PhysicalFile::AreaSize	Size_
//		読み込みサイズ [byte]
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::readArea(const Trans::Transaction&	Transaction_,
						 const AreaID				AreaID_,
						 void*						Buffer_,
						 const AreaOffset			Offset_,
						 const AreaSize				Size_)
{
	// 物理エリア情報に記録されている内容を取り出す
	Area::Information	areaInformation;
	m_Directory.fetchOutInfo(this->m_VersionPageTop,
							 AreaID_,
							 areaInformation);

	; _SYDNEY_ASSERT(Offset_ >= 0 &&
					 (Offset_ + Size_ <= areaInformation.m_Size));

	// 物理エリアへのポインタを設定する
	char*	areaPointer =
		this->m_VersionPageTop + areaInformation.m_Offset + Offset_;

	// 物理エリアからデータを読み込む
	ModOsDriver::Memory::copy(Buffer_, areaPointer, Size_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::fetchOutAreaInformation --
//		物理エリア情報を取り出す
//
//	NOTES
//	引数AreaID_で指定される物理エリアに関する情報を読み出して、
//	引数AreaInfo_へ設定する。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID			AreaID_
//		物理エリア識別子
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::fetchOutAreaInformation(
	const Trans::Transaction&	Transaction_,
	const AreaID				AreaID_,
	Area::Information&			AreaInfo_) const
{
	// 物理エリア情報を取り出す
	m_Directory.fetchOutInfo(this->m_VersionPageTop,
							 AreaID_,
							 AreaInfo_);

	// 物理エリアオフセットには「バージョンページ先頭からの位置」が
	// 記録されているが、
	// 利用者には「公開領域先頭からの位置」として見せる
	AreaInfo_.m_Offset -= this->m_Header.getSize();
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::fetchOutAreaInformation --
//		物理エリア情報を取り出す
//
//	NOTES
//	引数AreaID_で指定される物理エリアに関する情報を読み出して、
//	引数AreaInfo_へ設定する。
//
//	ARGUMENTS
//	const PhysicalFile::Content&		Content_,
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID			AreaID_,
//		物理エリア識別子
//	PhysicalFile::Area::Information&	AreaInfo_
//		物理エリア情報への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::fetchOutAreaInformation(
	const Content&		Content_,
	const AreaID		AreaID_,
	Area::Information&	AreaInfo_) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	char*	pagePointer = Content_.operator char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 物理エリア情報を取り出す
	m_Directory.fetchOutInfo(pagePointer,
							 AreaID_,
							 AreaInfo_);
	
	// 物理エリアオフセットには「バージョンページ先頭からの位置」が
	// 記録されているが、
	// 利用者には「公開領域先頭からの位置」として見せる
	AreaInfo_.m_Offset -= m_Header.getSize();
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getAreaSize --
//		物理エリア情報に記録されている物理エリアサイズを返す
//
//	NOTES
//	物理エリア情報に記録されている物理エリアサイズを返す
//
//	ARGUMENTS
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaSize
//		物理エリアサイズ
//
//	EXCEPTIONS
//	なし
//
AreaSize
AreaManagePage::getAreaSize(AreaID	AreaID_) const
{
	return m_Directory.getAreaSize(this->m_VersionPageTop, AreaID_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::changeAreaSize --
//		物理エリアを拡大／縮小する
//
//	NOTES
//	物理エリアを拡大／縮小する。
//	縮小時には戻り値は常にtrueとなるが、
//	拡大時には拡大が可能で実際に拡大を行った場合にtrueとなる。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::Content&			Content_,
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID		AreaID_,
//		拡大または縮小する物理エリアの識別子
//	const PhysicalFile::AreaSize	Size_,
//		拡大または縮小後の物理エリアのサイズ [byte]
//	const bool						DoCompaction_ = false
//		物理エリアの再配置も行うかどうか
//			true  : 必要に応じて、物理エリアの再配置も行う
//			false : 物理エリアの再配置は行わない
//
//	RETURN
//	bool
//		物理エリアを拡大／縮小したかどうか
//			true  : 物理エリアを拡大／縮小した
//			false : 物理エリアを拡大しなかった（できなかった）
//
//	EXCEPTIONS
//	なし
//
bool
AreaManagePage::changeAreaSize(
	const Trans::Transaction&	Transaction_,
	Content&					Content_,
	const AreaID				AreaID_,
	const AreaSize				Size_,
	const bool					DoCompaction_ // = false
	)
{
	// 物理ページのバッファリング内容へのポインタを得る
	char*	pagePointer = Content_.operator char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 物理エリアを拡大／縮小する
	return this->changeAreaSize(Transaction_,
								pagePointer,
								AreaID_,
								Size_,
								DoCompaction_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::changeAreaSize --
//		物理エリアを拡大／縮小する
//
//	NOTES
//	物理エリアを拡大／縮小する。
//	縮小時には戻り値は常にtrueとなるが、
//	拡大時には拡大が可能で実際に拡大を行った場合にtrueとなる。
//
//	ARGUMENTS
//	const Trans::Transaction&		cTransaction_
//		トランザクション記述子への参照
//	PhysicalFile::AreaID			uiAreaID_,
//		拡大または縮小する物理エリアの識別子
//	PhysicalFile::AreaSize			uiSize_,
//		拡大または縮小後の物理エリアのサイズ [byte]
//	bool							bDoCompaction_ = false
//		物理エリアの再配置も行うかどうか
//			true  : 必要に応じて、物理エリアの再配置も行う
//			false : 物理エリアの再配置は行わない
//
//	RETURN
//	bool
//		物理エリアを拡大／縮小したかどうか
//			true  : 物理エリアを拡大／縮小した
//			false : 物理エリアを拡大しなかった（できなかった）
//
//	EXCEPTIONS
//	なし
//
bool
AreaManagePage::changeAreaSize(
	const Trans::Transaction&	Transaction_,
	AreaID						AreaID_,
	AreaSize					Size_,
	bool						DoCompaction_)
{
	return this->changeAreaSize(Transaction_,
								this->m_VersionPageTop,
								AreaID_,
								Size_,
								DoCompaction_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::compaction --
//		物理エリアを再配置する
//
//	NOTES
//	物理エリアを再配置する。
//	この関数を呼び出して物理エリアの再配置を行うと、
//	以前に解放された物理エリアは再利用不可能となる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::Content&		Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileManipulateError
//		物理エリアの再配置に失敗した
//
void
AreaManagePage::compaction(const Trans::Transaction&	Transaction_,
						   Content&						Content_)
{
	char*	pagePointer = Content_.operator char*();
	pagePointer -= m_Header.getSize();

	// 物理エリアを再配置する
	this->compaction(Transaction_, pagePointer);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::compaction --
//		物理エリアを再配置する
//
//	NOTES
//	物理エリアを再配置する。
//	この関数を呼び出して物理エリアの再配置を行うと、
//	以前に解放された物理エリアは再利用不可能となる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileManipulateError
//		物理エリアの再配置に失敗した
//
void
AreaManagePage::compaction(const Trans::Transaction&	Transaction_)
{
	// 物理エリアを再配置する
	this->compaction(Transaction_, this->m_VersionPageTop);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getTopAreaID --
//		先頭の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での先頭物理エリアの識別子を返す。
//	（物理的に先頭の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での先頭の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getTopAreaID(const Content&	Content_) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	const char*	pagePointer = Content_.operator const char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 先頭の使用中物理エリアの識別子を得る
	return this->getTopAreaID(pagePointer);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getTopAreaID --
//		先頭の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での先頭物理エリアの識別子を返す。
//	（物理的に先頭の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での先頭の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getTopAreaID(
	const Trans::Transaction&	Transaction_) const
{
	// 先頭の使用中物理エリアの識別子を得る
	return this->getTopAreaID(this->m_VersionPageTop);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getLastAreaID --
//		最後の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での最終物理エリアの識別子を返す。
//	（物理的に最後の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での最後の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getLastAreaID(const Content&	Content_) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	const char*	pagePointer = Content_.operator const char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 最後の使用中物理エリアの識別子を得る
	return this->getLastAreaID(pagePointer);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getLastAreaID --
//		最後の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での最終物理エリアの識別子を返す。
//	（物理的に最後の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での最後の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getLastAreaID(
	const Trans::Transaction&	Transaction_) const
{
	// 最後の使用中物理エリアの識別子を得る
	return this->getLastAreaID(this->m_VersionPageTop);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getNextAreaID --
//		次の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での次の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID		AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での次の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getNextAreaID(const Content&	Content_,
							  const AreaID		AreaID_) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	const char*	pagePointer = Content_.operator const char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 次の使用中物理エリアの識別子を得る
	return this->getNextAreaID(pagePointer, AreaID_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getNextAreaID --
//		次の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での次の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での次の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getNextAreaID(const Trans::Transaction&	Transaction_,
							  const AreaID				AreaID_) const
{
	// 次の使用中物理エリアの識別子を得る
	return this->getNextAreaID(this->m_VersionPageTop, AreaID_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getPrevAreaID --
//		前の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での前の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaID		AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での前の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getPrevAreaID(const Content&	Content_,
							  const AreaID		AreaID_) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	const char*	pagePointer = Content_.operator const char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 前の使用中物理エリアの識別子を得る
	return this->getPrevAreaID(pagePointer, AreaID_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getPrevAreaID --
//		前の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での前の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での前の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getPrevAreaID(const Trans::Transaction&	Transaction_,
							  const AreaID				AreaID_) const
{
	// 前の使用中物理エリアの識別子を得る
	return this->getPrevAreaID(this->m_VersionPageTop, AreaID_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	物理ページ内の未使用領域サイズを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePage::getUnuseAreaSize(const Content&	Content_,
								 const AreaNum	AreaNum_ // = 1
								 ) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	const char*	pagePointer = Content_.operator const char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 物理ページ内の未使用領域サイズを返す
	return this->getUnuseAreaSize(pagePointer, AreaNum_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	物理ページ内の未使用領域サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaNum	AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePage::getUnuseAreaSize(
	const Trans::Transaction&	Transaction_,
	const AreaNum				AreaNum_ // = 1
	) const
{
	// 物理ページ内の未使用領域サイズを得る
	return this->getUnuseAreaSize(this->m_VersionPageTop, AreaNum_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	物理ページ内の空き領域サイズを返す。
//
//	ARGUMENTS
//	const PhysicalFile::Content&	Content_
//		関数PhysicalFile::Page::fixで得られる
//		物理ページのバッファリング内容への参照
//	const PhysicalFile::AreaNum		AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePage::getFreeAreaSize(const Content&	Content_,
								const AreaNum	AreaNum_ // = 1
								) const
{
	// 物理ページのバッファリング内容へのポインタを得る
	const char*	pagePointer = Content_.operator const char*();

	// PhysicalFile::Contentをchar*にキャストすると、
	// 公開領域先頭へのポインタが得られる
	// つまり、物理ページ先頭（物理ページヘッダの先頭）へのポインタが
	// 得られるわけではない
	// そのため、物理ページ先頭へポインタを移動する
	pagePointer -= m_Header.getSize();

	// 物理ページ内の空き領域サイズを得て、そのまま返す
	return this->getFreeAreaSize(pagePointer, AreaNum_);
}

//
//	FUNCTION public
//	PhysicalFile::AreaManagePage::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	物理ページ内の空き領域サイズを返す。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::AreaNum	AreaNum_ = 1
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePage::getFreeAreaSize(
	const Trans::Transaction&	Transaction_,
	const AreaNum				AreaNum_ // = 1
	) const
{
	// 物理ページ内の空き領域サイズを得る
	return this->getFreeAreaSize(this->m_VersionPageTop, AreaNum_);
}

//
//	FUNCTION private
//	AreaManagePage::correspondUseArea --
//		利用者と自身の物理エリアの使用状況が
//		一致するかどうかをチェックする
//
//	NOTES
//	利用者と自身の物理エリアの使用状況が一致するかどうかをチェックする。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	void*							TableTop_
//		空き領域管理表先頭へのポインタ
//	Common::BitSet&					AreaIDs_
//		物理エリア識別子が記録されているビットマップへの参照
//	const PhysicalFile::AreaID		LastAreaID_
//		利用者がその物理ページ内で最後の使用中とした物理エリアの識別子
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::correspondUseArea(
	const Trans::Transaction&		Transaction_,
	void*							TableTop_,
	Common::BitSet&					AreaIDs_,
	const AreaID					LastAreaID_,
	Admin::Verification::Progress&	Progress_)
{
	AreaID	realLastAreaID =
		this->getLastAreaID(this->m_VersionPageTop);

	//
	// ここでは、利用者が指定した最後の物理エリア識別子と、
	// 物理ファイルマネージャが認識しているそれが
	// 一致しているかはチェックしない。
	//

	AreaID	areaID;

	if (realLastAreaID != ConstValue::UndefinedAreaID)
	{
		for (areaID = 0; areaID <= realLastAreaID; areaID++)
		{
			Admin::Verification::Progress	areaProgress(Progress_.getConnection());

			bool	used = m_Directory.isUsedArea(this->m_VersionPageTop,
												  areaID);
			
			if ((LastAreaID_ == ConstValue::UndefinedAreaID && used) ||
				(areaID > LastAreaID_ && used))
			{
				// 物理ファイルマネージャとしては使用中と認識していたのに、
				// 利用者は未使用と指定していた…

				//
				// この場合は、直せる。
				// 修復指示をされていれば、直して、検査を続ける。
				//

				if ((this->m_File->m_Treatment &
					 Admin::Verification::Treatment::Correct)
					!= 0)
				{
					// 修復指示をされている…

					// では、直す。

					this->correctUseArea(areaID,
										 TableTop_,
										 areaProgress);
				}
				else
				{
					// 修復指示をされていない…

					_SYDNEY_VERIFY_INCONSISTENT(
						areaProgress,
						this->m_File->m_FilePath,
						Message::DiscordAreaUseSituation1(this->m_ID,
														  areaID));
				}
			}
			else
			{
				if (used != AreaIDs_[areaID])
				{
					if (used)
					{
						// 物理ファイルマネージャとしては、
						// 使用中と認識していたのに、
						// 利用者は未使用と指定していた…

						//
						// この場合は、直せる。
						// 修復指示をされていれば、直して、検査を続ける。
						//

						if ((this->m_File->m_Treatment &
							 Admin::Verification::Treatment::Correct)
							!= 0)
						{
							// 修復指示をされている…

							// では、直す。

							this->correctUseArea(areaID,
												 TableTop_,
												 areaProgress);
						}
						else
						{
							// 修復指示をされていない…

							_SYDNEY_VERIFY_INCONSISTENT(
								areaProgress,
								this->m_File->m_FilePath,
								Message::DiscordAreaUseSituation1(
									this->m_ID,
									areaID));
						}
					}
					else
					{
						// 物理ファイルマネージャとしては、
						// 未使用と認識していたのに、
						// 利用者は使用中と指定していた…

						// これは直せない。

						_SYDNEY_VERIFY_INCONSISTENT(
							areaProgress,
							this->m_File->m_FilePath,
							Message::DiscordAreaUseSituation2(
								this->m_ID,
								areaID));
					}
				}
			}

			if (areaProgress.getStatus() !=
				Admin::Verification::Status::Consistent)
			{
				Progress_ += areaProgress;
			}

			if (Progress_.isGood() == false)
			{
				break;
			}

		} // end for areaID
	}

	if (Progress_.isGood() &&
		LastAreaID_ != ConstValue::UndefinedAreaID)
	{
		if (realLastAreaID != ConstValue::UndefinedAreaID)
		{
			areaID = realLastAreaID + 1;
		}
		else
		{
			areaID = 0;
		}

		for (; areaID <= LastAreaID_; areaID++)
		{
			if (AreaIDs_[areaID])
			{
				// 物理ファイルマネージャとしては、
				// 未使用と認識していたのに、
				// 利用者は使用中と指定していた…

				// これは直せない。

				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					this->m_File->m_FilePath,
					Message::DiscordAreaUseSituation2(this->m_ID,
													  areaID));

				break;
			}
		}
	}
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::correctUseArea --
//		物理エリアの使用状態を修復する
//
//	NOTES
//	物理ファイルマネージャでは使用中と認識していた物理ページを
//	未使用状態に修復する。
//	※ どこからも例外が送出されることはないはずなので、
//	　 必ず修復できるはず。
//
//	ARGUMENTS
//	const PhysicalFile::AreaID		AreaID_
//		修復する物理エリアの識別子（この物理エリアを未使用状態にする）
//	void*							TableTop_
//		空き領域管理表先頭へのポインタ
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::correctUseArea(
	const AreaID					AreaID_,
	void*							TableTop_,
	Admin::Verification::Progress&	Progress_)
{
	// 物理ページヘッダに記録されている
	// 　1. 未使用領域サイズ
	// 　2. 空き領域サイズ
	// を取り出す
	PageSize	unuseAreaSize = 0;
	PageSize	freeAreaSize = 0;
	(*(this->m_Header.FetchOutAreaSize))(this->m_VersionPageTop,
										 unuseAreaSize,
										 freeAreaSize);

	// 修復前の、物理ページ内の未使用領域率を得る
	unsigned int		beforeUnuseAreaRateValue = 0;
	BitmapTable::Rate	beforeUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							beforeUnuseAreaRateValue,
							beforeUnuseAreaRate);

	// 未使用領域サイズを更新する
	unuseAreaSize +=
		m_Directory.getAreaSize(this->m_VersionPageTop, AreaID_);

	// 物理エリア管理ディレクトリ内の
	// 「物理エリア使用状態ビットマップ」のビットをOFFにする
	m_Directory.overwriteBitValue(this->m_VersionPageTop,
								  AreaID_,
								  false); // ビットOFF

	// 物理ページヘッダ内の「未使用領域サイズ」を上書きする
	(*(this->m_Header.OverwriteUnuseAreaSize))(this->m_VersionPageTop,
											   unuseAreaSize);

	// 修復後の、物理ページ内の未使用領域率を得る
	unsigned int		afterUnuseAreaRateValue = 0;
	BitmapTable::Rate	afterUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							afterUnuseAreaRateValue,
							afterUnuseAreaRate);

	if (beforeUnuseAreaRate != afterUnuseAreaRate)
	{
		// 修復により、空き領域管理表の更新も必要となった…

		// 修復後の空き領域率を得る
		// ※ 修復により、空き領域率は変わらないが、
		// 　 領域率ビットマップの値を得るために必要
		unsigned int		afterFreeAreaRateValue = 0;
		BitmapTable::Rate	afterFreeAreaRate = BitmapTable::InvalidRate;
		this->convertToAreaRate(freeAreaSize,
								afterFreeAreaRateValue,
								afterFreeAreaRate);

		// 修復後の領域率ビットマップの値を得る
		unsigned char	afterBitValue =
			BitmapTable::ToBitmapValue[afterUnuseAreaRate]
									  [afterFreeAreaRate];

		// 空き領域管理表内の領域率ビットマップを上書きする
		AreaManageTable::Bitmap::overwriteValue(
			TableTop_,
			this->m_TableHeader->m_Type,
			this->m_ID,
			this->m_File->getPagePerManageTable(),
			afterBitValue);

		// 物理エリア解放前の空き領域管理表ヘッダ内の
		// 「未使用領域率別の物理ページ数配列」を更新する
		this->m_TableHeader->updatePageArray(
			TableTop_,
			true, // 未使用領域率別の物理ページ数配列
			beforeUnuseAreaRateValue,
			afterUnuseAreaRateValue);
	}

	_SYDNEY_VERIFY_CORRECTED(Progress_,
							 this->m_File->m_FilePath,
							 Message::CorrectedAreaUseSituation(
								this->m_ID,
								AreaID_));
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::checkPhysicalArea --
//		物理エリア情報検査
//
//	NOTES
//	物理エリア情報の整合性検査および
//	物理エリアの重複検査を行う。
//
//	ARGUMENTS
//	Admin::Verification::Progress&	Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::checkPhysicalArea(
	Admin::Verification::Progress&	Progress_) const
{
	//
	// 物理エリア情報の整合性検査をする。
	//

	AreaID	areaID = this->getTopAreaID(this->m_VersionPageTop);

	while (areaID != ConstValue::UndefinedAreaID)
	{
		Area::Information	areaInfo;
		m_Directory.fetchOutInfo(this->m_VersionPageTop,
								 areaID,
								 areaInfo);

		PageOffset	pageLast = m_File->m_VersionPageDataSize - 1;

		if (areaInfo.m_Offset < 0 || areaInfo.m_Offset > pageLast)
		{
			_SYDNEY_VERIFY_INCONSISTENT(
				Progress_,
				this->m_File->m_FilePath,
				Message::ExistProtrusiveArea(this->m_ID,
											 areaID,
											 areaInfo.m_Offset,
											 areaInfo.m_Size));

			return;
		}
		else
		{
			PageOffset	areaLast = areaInfo.m_Offset + areaInfo.m_Size;

			if (areaLast > pageLast)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					this->m_File->m_FilePath,
					Message::ExistProtrusiveArea(this->m_ID,
												 areaID,
												 areaInfo.m_Offset,
												 areaInfo.m_Size));

				return;
			}
		}

		areaID = this->getNextAreaID(this->m_VersionPageTop, areaID);
	}

	//
	// 物理エリアの重複検査をする。
	//

	areaID = this->getTopAreaID(this->m_VersionPageTop);

	while (areaID != ConstValue::UndefinedAreaID)
	{
		Area::Information	srcAreaInfo;
		m_Directory.fetchOutInfo(this->m_VersionPageTop,
								 areaID,
								 srcAreaInfo);

		AreaID	dstAreaID = this->getNextAreaID(this->m_VersionPageTop,
												areaID);

		Admin::Verification::Progress	areaProgress(Progress_.getConnection());

		while (dstAreaID != ConstValue::UndefinedAreaID)
		{
			Area::Information	dstAreaInfo;
			m_Directory.fetchOutInfo(this->m_VersionPageTop,
									 dstAreaID,
									 dstAreaInfo);

			if (srcAreaInfo.m_Offset < dstAreaInfo.m_Offset)
			{
				PageOffset	srcEndOffset =
					srcAreaInfo.m_Offset + srcAreaInfo.m_Size - 1;

				if (srcEndOffset >= dstAreaInfo.m_Offset)
				{
					_SYDNEY_VERIFY_INCONSISTENT(
						Progress_,
						this->m_File->m_FilePath,
						Message::ExistDuplicateArea(
							this->m_ID,
							areaID,
							srcAreaInfo.m_Offset,
							srcAreaInfo.m_Size,
							dstAreaID,
							dstAreaInfo.m_Offset,
							dstAreaInfo.m_Size));

					return;
				}
			}
			else
			{
				PageOffset	dstEndOffset =
					dstAreaInfo.m_Offset + dstAreaInfo.m_Size - 1;

				if (dstEndOffset >= srcAreaInfo.m_Offset)
				{
					_SYDNEY_VERIFY_INCONSISTENT(
						Progress_,
						this->m_File->m_FilePath,
						Message::ExistDuplicateArea(
							this->m_ID,
							areaID,
							srcAreaInfo.m_Offset,
							srcAreaInfo.m_Size,
							dstAreaID,
							dstAreaInfo.m_Offset,
							dstAreaInfo.m_Size));

					return;
				}
			}

			dstAreaID = this->getNextAreaID(this->m_VersionPageTop,
											dstAreaID);
		}

		areaID = this->getNextAreaID(this->m_VersionPageTop, areaID);
	}
}

#ifdef DEBUG

void
AreaManagePage::getPageHeader(
	const Trans::Transaction&	Transaction_,
	PageSize&					UnuseAreaSize_,
	unsigned int&				UnuseAreaRate_,
	PageSize&					FreeAreaSize_,
	unsigned int&				FreeAreaRate_,
	PageOffset&					FreeAreaOffset_,
	AreaNum&					ManageAreaNum_)
{
	AreaManagePageHeader::Item	pageHeaderItem;

	(*(this->m_Header.FetchOut))(this->m_VersionPageTop,
								 pageHeaderItem);

	UnuseAreaSize_	= pageHeaderItem.m_UnuseAreaSize;
	FreeAreaSize_	= pageHeaderItem.m_FreeAreaSize;
	FreeAreaOffset_	= pageHeaderItem.m_FreeAreaOffset;
	ManageAreaNum_	= pageHeaderItem.m_ManageAreaNum;

	BitmapTable::Rate	dummyRate;
	this->convertToAreaRate(UnuseAreaSize_,
							UnuseAreaRate_,
							dummyRate);

	this->convertToAreaRate(FreeAreaSize_,
							FreeAreaRate_,
							dummyRate);
}

void
AreaManagePage::getAreaDirectory(
	const Trans::Transaction&	Transaction_,
	unsigned char*				AreaUseFlag_,
	Area::Information*			AreaInfo_)
{
	AreaNum	manageAreaNum =
		(*(this->m_Header.GetManageAreaNum))(this->m_VersionPageTop);

	if (manageAreaNum == 0)
	{
		return;
	}

	AreaID	lastAreaID = manageAreaNum - 1;

	for (AreaID areaID = 0; areaID <= lastAreaID; areaID++)
	{
		bool	use =
			m_Directory.isUsedArea(this->m_VersionPageTop, areaID);

		*(AreaUseFlag_ + areaID) = use ? 1 : 0;

		m_Directory.fetchOutInfo(this->m_VersionPageTop,
								 areaID,
								 *(AreaInfo_ + areaID));
	}
}

#endif

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::AreaManagePageクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::AreaManagePage -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::File*					File_
//		物理ファイル記述子
//	const PhysicalFile::PageID					PageID_
//		物理ページ識別子
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority
//																= Middle
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
AreaManagePage::AreaManagePage(
	const Trans::Transaction&					Transaction_,
	AreaManageFile*								File_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_)
	: Page(Transaction_,
		   File_,
		   PageID_,
		   FixMode_,
		   ReplacementPriority_),
	  m_File(File_),
	  m_Header(*File_->m_PageHeader),
	  m_NecessaryRecoverTable(false),
	  m_Directory(File_->getDirectory())
{
	this->m_VersionPageTop =
		this->m_PhysicalPageTop - m_Header.getSize();
	m_TableHeader = &m_File->m_TableHeader;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	const Trans::Transaction&			Transaction_
//		トランザクション記述子への参照
//	PhysicalFile*						File_
//		物理ファイル記述子
//	const PhysicalFile::PageID			PageID_
//		物理ページ識別子
//	const Buffer::Page::FixMode::Value	FixMode_
//		フィックスモード
//	Admin::Verification::Progress&		Progress_
//		整合性検査の途中経過への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	[YET!]
//
AreaManagePage::AreaManagePage(
	const Trans::Transaction&			Transaction_,
	AreaManageFile*						File_,
	const PageID						PageID_,
	const Buffer::Page::FixMode::Value	FixMode_,
	Admin::Verification::Progress&		Progress_)
	: Page(Transaction_,
		   File_,
		   PageID_,
		   FixMode_,
		   Progress_),
	  m_File(File_),
	  m_Header(*File_->m_PageHeader),
	  m_Directory(File_->getDirectory())
{
	this->m_VersionPageTop =
		this->m_PhysicalPageTop - m_Header.getSize();
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::~AreaManagePage --
//		デストラクタ
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
AreaManagePage::~AreaManagePage()
{
}

//
//	FUCNTION private
//	PhysicalFile::AreaManagePage::reset -- メモリー再利用
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AreaManagePage::reset(
	const Trans::Transaction&			Transaction_,
	PageID								PageID_,
	Buffer::Page::FixMode::Value		FixMode_,
	Buffer::ReplacementPriority::Value	ReplacementPriority_)
{
	Page::reset(Transaction_,
				m_File,
				PageID_,
				FixMode_,
				ReplacementPriority_);
	m_VersionPageTop = m_PhysicalPageTop - m_Header.getSize();
	m_TableHeader = &m_File->m_TableHeader;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getUserAreaSize --
//		利用者に公開する領域のサイズを返す
//
//	NOTES
//	利用者に公開する領域のサイズを返す。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::PageSize
//		利用者に公開する領域のサイズ [byte]
//
//	EXCEPTIONS
//	なし
//

#ifdef DEBUG

PageSize
AreaManagePage::getUserAreaSize() const
{
	// 物理ページヘッダ内に記録されている
	// 「管理している物理エリア数」を得る
	AreaNum	areaNum =
		(*(this->m_Header.GetManageAreaNum))(this->m_VersionPageTop);

	return this->m_File->m_UserAreaSizeMax - m_Directory.getSize(areaNum);
}

#endif

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::convertToAreaRate --
//		未使用領域サイズ／空き領域サイズを領域率に変換する
//
//	NOTES
//	未使用領域サイズ／空き領域サイズを領域率に変換する。
//
//	ARGUMENTS
//	const PhysicalFile::PageSize		AreaSize_
//		領域サイズ [byte]
//	unsigned int&						AreaRateValue_
//		領域率への参照 [%]
//	PhysicalFile::BitmapTable::Rate&	AreaRate_
//		領域率列挙子への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
AreaManagePage::convertToAreaRate(const PageSize		AreaSize_,
								  unsigned int&			AreaRateValue_,
								  BitmapTable::Rate&	AreaRate_) const
{
	AreaRateValue_ =
		static_cast<unsigned int>((static_cast<double>(AreaSize_) /
								  this->m_File->m_UserAreaSizeMax) *
								  100);
	AreaRate_ = BitmapTable::ToRate[AreaRateValue_];
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::changeAreaSize --
//		物理エリアを拡大／縮小する
//
//	NOTES
//	物理エリアを拡大／縮小する。
//	縮小時には戻り値は常にtrueとなるが、
//	拡大時には拡大が可能で実際に拡大を行った場合にtrueとなる。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	void*							PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID		AreaID_
//		拡大／縮小する物理エリアの識別子
//	const PhysicalFile::AreaSize	Size_
//		拡大／縮小後の物理エリアサイズ [byte]
//	const bool						DoCompaction_ = false
//		物理エリアの再配置も行うかどうか
//			true  : 必要に応じて、物理エリアの再配置も行う
//			false : 物理エリアの再配置は行わない
//
//	RETURN
//	bool
//		物理エリアを拡大／縮小したかどうか
//			true  : 物理エリアを拡大／縮小した
//			false : 物理エリアを拡大しなかった（できなかった）
//
//	EXCEPTIONS
//	なし
//
bool
AreaManagePage::changeAreaSize(
	const Trans::Transaction&	Transaction_,
	void*						PagePointer_,
	const AreaID				AreaID_,
	const AreaSize				Size_,
	const bool					DoCompaction_ // = false
	)
{
	// 物理エリア情報を取り出す
	Area::Information	areaInformation;
	m_Directory.fetchOutInfo(PagePointer_,
							 AreaID_,
							 areaInformation);

	bool	status = false;
	if (Size_ > areaInformation.m_Size)
	{
		// 物理エリアを拡大する
		status = this->expandArea(Transaction_,
								  PagePointer_,
								  AreaID_,
								  areaInformation.m_Size,
								  Size_,
								  DoCompaction_);
	}
	else
	{
		// 物理エリアを縮小する
		status = this->reduceArea(Transaction_, 
								  PagePointer_,
								  AreaID_,
								  Size_);
	}

	return status;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::expandArea --
//		物理エリアを拡大する
//
//	NOTES
//	物理エリアを拡大する。
//	現在、物理エリアに記録されているデータは
//	失われる。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	void*							PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID		AreaID_
//		拡大する物理エリアの識別子
//	const PhysicalFile::AreaSize	BeforeAreaSize_
//		拡大前の物理エリアサイズ [byte]
//	const PhysicalFile::AreaSize	AfterAreaSize_
//		拡大後の物理エリアサイズ [byte]
//	const bool						DoCompaction_
//		物理エリアの再配置も行うかどうか
//			true  : 必要に応じて、物理エリアの再配置も行う
//			false : 物理エリアの再配置は行わない
//
//	RETURN
//	bool
//		物理エリアを拡大したかどうか
//			true  : 物理エリアを拡大した
//			false : 物理エリアを拡大しなかった（できなかった）
//
//	EXCEPTIONS
//	なし
//
bool
AreaManagePage::expandArea(const Trans::Transaction&	Transaction_,
						   void*						PagePointer_,
						   const AreaID					AreaID_,
						   const AreaSize				BeforeAreaSize_,
						   const AreaSize				AfterAreaSize_,
						   const bool					DoCompaction_)
{
	// 物理ページヘッダに記録されている情報を取り出す
	AreaManagePageHeader::Item	headerItem;

	(*(this->m_Header.FetchOut))(PagePointer_, headerItem);

	bool	possible = false;
	bool	doCompaction = false;
	if (AfterAreaSize_ <= headerItem.m_FreeAreaSize)
	{
		// 物理エリアの再配置を行わなくても、空き領域に移動可能…

		possible = true;
	}
	else
	{
		// 物理エリアの再配置を行わなければ、空き領域に移動不可能…

		if (DoCompaction_ &&
			AfterAreaSize_ <=
			(headerItem.m_UnuseAreaSize + BeforeAreaSize_))
		{
			possible = true;
			doCompaction = true;
		}
	}

	if (possible == false)
	{
		return false;
	}

	if (doCompaction)
	{
		// 物理エリアの再配置を行えば、空き領域に移動可能…

		//
		// 物理ページ内で使用中の物理エリアのうち、
		// 最も大きな識別子の物理エリアを
		// freeArea→compactionしてしまうと、
		// その物理ページ内で管理している物理エリア数が
		// 減少してしまう。
		// これを防ぐために、このような場合には、
		// 物理ページヘッダに記録している“管理している物理エリア数”を
		// 元に戻してあげなくてはいけない。
		//
		bool	restoreManageAreaNum =
			(AreaID_ == this->getLastAreaID(this->m_VersionPageTop));

		try
		{
			// 引数AreaID_で指定された物理エリアを解放する
			this->freeArea(Transaction_, AreaID_);
		}
		catch (Exception::Object&)
		{
			// 物理エリアを解放できなかった…

			// ということは、物理エリアを拡大できない
			return false;
		}

		try
		{
			// 物理エリアの再配置を行う
			this->compaction(Transaction_, PagePointer_);
		}
		//catch (Exception::FileManipulateError&)
		catch (Exception::Object&)
		{
			// 物理エリアの再配置に失敗した…

			// ということは、物理エリアを拡大できない
			return false;
		}

		// 再度、物理ページヘッダに記録されている情報を取り出す
		(*(this->m_Header.FetchOut))(PagePointer_, headerItem);

		if (restoreManageAreaNum)
		{
			headerItem.m_ManageAreaNum = AreaID_ + 1;
		}
	}

	// 物理エリア拡大前の物理ページ内の未使用領域率を得る
	unsigned int		beforeUnuseAreaRateValue = 0;
	BitmapTable::Rate	beforeUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_UnuseAreaSize,
							beforeUnuseAreaRateValue,
							beforeUnuseAreaRate);

	// 物理エリア拡大前の物理ページ内の空き領域率を得る
	unsigned int		beforeFreeAreaRateValue = 0;
	BitmapTable::Rate	beforeFreeAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_FreeAreaSize,
							beforeFreeAreaRateValue,
							beforeFreeAreaRate);

	// 物理ページヘッダに記録されている「未使用領域サイズ」に
	// 今までの物理エリアサイズを加算する
	PageSize	unuseAreaSize = headerItem.m_UnuseAreaSize;
	if (doCompaction == false)
	{
		unuseAreaSize += BeforeAreaSize_;
	}

	// 物理エリア情報を更新する
	Area::Information	areaInformation;
	areaInformation.m_Offset = headerItem.m_FreeAreaOffset;
	areaInformation.m_Size = AfterAreaSize_;

	// 物理ページヘッダを更新する
	headerItem.m_UnuseAreaSize = unuseAreaSize - AfterAreaSize_;
	headerItem.m_FreeAreaSize -= AfterAreaSize_;
	headerItem.m_FreeAreaOffset += AfterAreaSize_;

	//
	// 物理ページ内の更新は、ここではまだしない。
	//

	// 物理エリア拡大後の未使用領域率を得る
	unsigned int		afterUnuseAreaRateValue = 0;
	BitmapTable::Rate	afterUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_UnuseAreaSize,
							afterUnuseAreaRateValue,
							afterUnuseAreaRate);

	// 物理エリア拡大後の空き領域率を得る
	unsigned int		afterFreeAreaRateValue = 0;
	BitmapTable::Rate	afterFreeAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(headerItem.m_FreeAreaSize,
							afterFreeAreaRateValue,
							afterFreeAreaRate);

	if (beforeUnuseAreaRate != afterUnuseAreaRate ||
		beforeFreeAreaRate != afterFreeAreaRate)
	{
		// 物理エリア拡大により、
		// 空き領域管理表の更新も必要となった…

		// 物理ページ内を更新する前に
		// 空き領域管理表のバッファリング内容を得る（フィックスする）
		// ※ 物理ページ内を更新してしまった後で
		// 　 空き領域管理表をフィックスできなかった場合、
		// 　 物理ページ内を元に戻す処理が必要となるため。

		try
		{
			// 空き領域管理表のバッファリング内容を得る
			// （フィックスする）

			File::PagePointer table =
				m_File->fixVersionPage(
					Transaction_,
					this->m_File->getManageTableVersionPageID(this->m_ID),
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Middle);

			// 空き領域管理表のバッファリング内容へのポインタを得る
			void*	tablePointer = (void*)(*table);

			// 物理エリア拡大後の領域率ビットマップの値を得る
			unsigned char	afterBitValue =
				BitmapTable::ToBitmapValue[afterUnuseAreaRate]
										  [afterFreeAreaRate];

			// 空き領域管理表内の領域率ビットマップを上書きする
			AreaManageTable::Bitmap::overwriteValue(
				tablePointer,
				this->m_TableHeader->m_Type,
				this->m_ID,
				this->m_File->getPagePerManageTable(),
				afterBitValue);

			if (beforeUnuseAreaRate != afterUnuseAreaRate)
			{
				// 物理エリア拡大前の空き領域管理表ヘッダ内の
				// 「未使用領域率別の物理ページ数配列」を更新する
				this->m_TableHeader->updatePageArray(
					tablePointer,
					true, // 未使用領域率別の物理ページ数配列
					beforeUnuseAreaRateValue,
					afterUnuseAreaRateValue);
			}

			if (beforeFreeAreaRate != afterFreeAreaRate)
			{
				// 物理エリア拡大前の空き領域管理表ヘッダ内の
				// 「空き領域率別の物理ページ数配列」を更新する
				this->m_TableHeader->updatePageArray(
					tablePointer,
					false, // 空き領域率別の物理ページ数配列
					beforeFreeAreaRateValue,
					afterFreeAreaRateValue);
			}

			//
			// 空き領域管理表を更新したので、
			// 利用者側でエラーを検出した場合には
			// 空き領域管理表の修復が必要となった。
			//

			this->m_NecessaryRecoverTable = true;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			return false;
		}
	}

	//
	// ここからが物理ページの更新
	//

	// 物理エリア情報を上書きする
	m_Directory.overwriteInfo(PagePointer_, AreaID_, areaInformation);

	// 物理ページヘッダを上書きする
	(*(this->m_Header.Overwrite))(PagePointer_, headerItem);

	// 物理エリア使用状態ビットマップのビットをONにする
	m_Directory.overwriteBitValue(PagePointer_, AreaID_, true);

	return true;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::reduceArea --
//		物理エリアを縮小する
//
//	NOTES
//	物理エリアを縮小する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	void*							PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID		AreaID_
//		縮小する物理エリアの識別子
//	const PhysicalFile::AreaSize	Size_
//		縮小後の物理エリアサイズ [byte]
//
//	RETURN
//	bool
//		（版管理マネージャ以下のモジュールで
//		エラーが発生せずに）縮小が正常に行われた場合には、
//		常にtrueを返す
//
//	EXCEPTIONS
//	なし
//
bool
AreaManagePage::reduceArea(const Trans::Transaction&	Transaction_,
						   void*						PagePointer_,
						   const AreaID					AreaID_,
						   const AreaSize				Size_)
{
	// 物理ページヘッダに記録されている「未使用領域サイズ」を取り出す
	PageSize	unuseAreaSize =
		(*(this->m_Header.GetUnuseAreaSize))(PagePointer_);

	// 物理エリア縮小前の物理ページ内の未使用領域率を得る
	unsigned int		beforeUnuseAreaRateValue = 0;
	BitmapTable::Rate	beforeUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							beforeUnuseAreaRateValue,
							beforeUnuseAreaRate);

	// 物理エリア情報を取り出す
	Area::Information	areaInformation;
	m_Directory.fetchOutInfo(PagePointer_, AreaID_, areaInformation);

	// 物理エリア縮小前後の差分サイズを求める
	PageSize	diffSize = areaInformation.m_Size - Size_;

	// 未使用領域サイズを更新する
	unuseAreaSize += diffSize;

	//
	// 物理ページ内の更新は、ここではまだしない。
	//

	// 物理エリア縮小後の物理ページ内の未使用領域率を得る
	unsigned int		afterUnuseAreaRateValue = 0;
	BitmapTable::Rate	afterUnuseAreaRate = BitmapTable::InvalidRate;
	this->convertToAreaRate(unuseAreaSize,
							afterUnuseAreaRateValue,
							afterUnuseAreaRate);

	if (beforeUnuseAreaRate != afterUnuseAreaRate)
	{
		// 物理エリア縮小により、
		// 空き領域管理表の更新も必要となった…

		// 物理ページ内を更新する前に
		// 空き領域管理表のバッファリング内容を得る（フィックスする）
		// ※ 物理ページ内を更新してしまった後で
		// 　 空き領域管理表をフィックスできなかった場合、
		// 　 物理ページ内を元に戻す処理が必要となるため。

		try
		{
			// 空き領域管理表のバッファリング内容を得る
			// （フィックスする）

			File::PagePointer table = 
				m_File->fixVersionPage(
					Transaction_,
					this->m_File->getManageTableVersionPageID(this->m_ID),
					File::DiscardableWriteFixMode,
					Buffer::ReplacementPriority::Middle);

			// 空き領域管理表のバッファリング内容へのポインタを得る
			void*	tablePointer = (void*)(*table);

			// 物理ページヘッダに記録されている「空き領域サイズ」を得る
			PageSize	freeAreaSize =
				(*(this->m_Header.GetFreeAreaSize))(PagePointer_);

			// 物理ページ内の空き領域率を得る
			unsigned int		freeAreaRateValue = 0;
			BitmapTable::Rate	freeAreaRate = BitmapTable::InvalidRate;
			this->convertToAreaRate(freeAreaSize,
									freeAreaRateValue,
									freeAreaRate);

			// 物理エリア縮小後の領域率ビットマップの値を得る
			unsigned char	afterBitValue =
				BitmapTable::ToBitmapValue[afterUnuseAreaRate]
										  [freeAreaRate];

			// 空き領域管理表内の領域率ビットマップを上書きする
			AreaManageTable::Bitmap::overwriteValue(
				tablePointer,
				this->m_TableHeader->m_Type,
				this->m_ID,
				this->m_File->getPagePerManageTable(),
				afterBitValue);

			// 物理エリア縮小前の空き領域管理表ヘッダ内の
			// 「未使用領域率別の物理ページ数配列」を更新する
			this->m_TableHeader->updatePageArray(
				tablePointer,
				true, // 未使用領域率別の物理ページ数配列
				beforeUnuseAreaRateValue,
				afterUnuseAreaRateValue);

			//
			// 空き領域管理表を更新したので、
			// 利用者側でエラーを検出した場合には
			// 空き領域管理表の修復が必要となった。
			//

			this->m_NecessaryRecoverTable = true;
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// ここにくるのは空き領域管理表の
			// バッファリング内容を取得できなかったときだけである。

			return false;
		}
	}

	//
	// ここからが物理ページの更新
	//

	// 物理エリア情報に記録されている「物理エリアサイズ」を上書きする
	m_Directory.overwriteAreaSize(PagePointer_, AreaID_, Size_);
	
	// 物理ページヘッダの「未使用領域サイズ」を上書きする
	(*(this->m_Header.OverwriteUnuseAreaSize))(PagePointer_,
											   unuseAreaSize);

	return true;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::compaction --
//		物理エリアを再配置する
//
//	NOTES
//	物理エリアを再配置する。
//	この関数を呼び出して物理エリアの再配置を行うと、
//	以前に解放された物理エリアは再利用不可能となる。
//
//	ARGUMENTS
//	const Trans::Transaction&	Transaction_
//		トランザクション記述子への参照
//	void*						PagePointer_
//		物理ページのバッファリング内容へのポインタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	FileManipulateError
//		物理エリアの再配置に失敗した
//
void
AreaManagePage::compaction(const Trans::Transaction&	Transaction_,
						   void*						PagePointer_)
{
	// 物理ページヘッダに記録されている情報を取り出す
	AreaManagePageHeader::Item	headerItem;

	(*(this->m_Header.FetchOut))(PagePointer_, headerItem);

	if (headerItem.m_UnuseAreaSize == headerItem.m_FreeAreaSize)
	{
		// 物理エリアの再配置は不要…

		return;
	}

	try
	{
		// 物理ページ内を更新する前に
		// 空き領域管理表のバッファリング内容を得る（フィックスする）
		// ※ 物理ページ内を更新してしまった後で
		// 　 空き領域管理表をフィックスできなかった場合、
		// 　 物理ページ内を元に戻す処理が必要となるため。
		// 　 再配置した物理エリアを元に戻すのは困難である。

		// 空き領域管理表のバッファリング内容を得る（フィックスする）

		//
		// 何らかのエラー発生時に、
		// フィックスした版を破棄できるように
		// Discardableモードも指定する。
		//

		File::PagePointer table = 
			m_File->fixVersionPage(
				Transaction_,
				this->m_File->getManageTableVersionPageID(this->m_ID),
				File::DiscardableWriteFixMode,
				Buffer::ReplacementPriority::Middle);

		// 空き領域管理表のバッファリング内容へのポインタを得る
		void*	tablePointer = (void*)(*table);

		// 物理エリア際はいつ前の物理ページ内の未使用領域率を得る
		unsigned int		beforeUnuseAreaRateValue = 0;
		BitmapTable::Rate	beforeUnuseAreaRate =
			BitmapTable::InvalidRate;
		this->convertToAreaRate(headerItem.m_UnuseAreaSize,
								beforeUnuseAreaRateValue,
								beforeUnuseAreaRate);

		// 物理エリア再配置前の物理ページ内の空き領域率を得る
		unsigned int		beforeFreeAreaRateValue = 0;
		BitmapTable::Rate	beforeFreeAreaRate =
			BitmapTable::InvalidRate;
		this->convertToAreaRate(headerItem.m_FreeAreaSize,
								beforeFreeAreaRateValue,
								beforeFreeAreaRate);

		// 利用者に公開する領域のオフセットを得る
		PageOffset	userAreaOffset = m_Header.getSize();

		char*	bufferPointer = 0;
		try
		{
			// 作業用バッファを確保する
			bufferPointer =
				static_cast<char*>(
					ModDefaultManager::allocate(
						m_File->m_VersionPageDataSize));
		}
		catch (ModException&)
		{
			// MOD側で何らかの例外が発生し、
			// 物理エリアの再配置に必要な
			// 作業用バッファを確保できなかった…
			
			Common::Thread::resetErrorCondition();

			// 物理エリアの再配置に失敗した
			throw Exception::FileManipulateError(moduleName,
												 srcFile,
												 __LINE__);
		}

		if (bufferPointer == 0)
		{
			// 物理エリアの再配置に必要な
			// 作業用バッファを確保できなかった…

			// 物理エリアの再配置に失敗した
			throw Exception::FileManipulateError(moduleName,
												 srcFile,
												 __LINE__);
		}

		// 作業用バッファオフセットを初期化する
		PageOffset	bufferOffset = userAreaOffset;

		// 物理エリアを管理していない、つまり、物理エリアを１つも
		// 確保していないのであれば、物理ページ内の
		// 未使用領域サイズと空き領域サイズは等しいので
		// ここは通らないはずである。
		// したがって、以下は必ず成り立つ。
		; _SYDNEY_ASSERT(headerItem.m_ManageAreaNum > 0);

		// 物理ページ内で管理している最終物理エリアの識別子を求める
		AreaID	lastAreaID = headerItem.m_ManageAreaNum - 1;

		// 各物理エリアごとに更新
		AreaID	areaID;

		for (areaID = 0; areaID <= lastAreaID; areaID++)
		{
			Area::Information	areaInformation;

			if (m_Directory.isUsedArea(PagePointer_, areaID))
			{
				// 使用中の物理エリア…

				// 物理エリア情報を取り出す
				m_Directory.fetchOutInfo(PagePointer_,
										 areaID,
										 areaInformation);

				char*	pagePointer = static_cast<char*>(PagePointer_);

				// 物理ページのバッファリング内容から作業用バッファへ
				// 物理エリアのデータをコピーする
				ModOsDriver::Memory::copy(
					bufferPointer + bufferOffset,
					pagePointer + areaInformation.m_Offset,
					areaInformation.m_Size);

				// 物理エリア情報の「物理エリアオフセット」を上書きする
				m_Directory.overwriteAreaOffset(PagePointer_,
												areaID,
												bufferOffset);

				// 作業用バッファオフセットを更新する
				bufferOffset += areaInformation.m_Size;
			}
			else
			{
				// 未使用の物理エリア…

				// 再利用されないように、
				// 物理エリア情報に無効な値を上書きする

				areaInformation.m_Offset =
					ConstValue::UndefinedAreaOffset;
				areaInformation.m_Size =
					ConstValue::UndefinedAreaSize;

				m_Directory.overwriteInfo(PagePointer_,
										  areaID,
										  areaInformation);
			}
		}

		AreaNum	beforeAreaNum = headerItem.m_ManageAreaNum;

		areaID = lastAreaID;

		while (true)
		{
			if (m_Directory.isUsedArea(PagePointer_, areaID))
			{
				// 使用中の物理エリア…
				break;
			}

			headerItem.m_ManageAreaNum--;

			if (areaID == 0)
			{
				break;
			}

			areaID--;
		}

		PageSize	diffSize =
			m_Directory.getSize(beforeAreaNum) -
			m_Directory.getSize(headerItem.m_ManageAreaNum);

		headerItem.m_UnuseAreaSize += diffSize;

		headerItem.m_FreeAreaSize = headerItem.m_UnuseAreaSize;

		headerItem.m_FreeAreaOffset = bufferOffset;

		// 物理ページヘッダを上書きする
		(*(this->m_Header.Overwrite))(PagePointer_, headerItem);

		// 作業用バッファの空き領域の部分を0埋めする
		ModOsDriver::Memory::reset(bufferPointer + bufferOffset,
								   headerItem.m_FreeAreaSize);

		// 作業用バッファから物理ページのバッファリング内容へ
		// 物理ページ内の利用者に公開する領域をコピーする
		PageSize	userAreaSize =
			m_File->m_VersionPageDataSize -
				m_Header.getSize() -
					m_Directory.getSize(headerItem.m_ManageAreaNum);
		char*	pagePointer = static_cast<char*>(PagePointer_);
		ModOsDriver::Memory::copy(pagePointer + userAreaOffset,
								  bufferPointer + userAreaOffset,
								  userAreaSize);

		// 作業用バッファを解放する
		ModDefaultManager::free(bufferPointer,
								m_File->m_VersionPageDataSize);

		// 物理エリア再配置後の物理ページ内の未使用領域率を得る
		unsigned int		afterUnuseAreaRateValue = 0;
		BitmapTable::Rate	afterUnuseAreaRate =
			BitmapTable::InvalidRate;
		this->convertToAreaRate(headerItem.m_UnuseAreaSize,
								afterUnuseAreaRateValue,
								afterUnuseAreaRate);

		// 物理エリア再配置後の物理ページ内の空き領域率を得る
		unsigned int		afterFreeAreaRateValue = 0;
		BitmapTable::Rate	afterFreeAreaRate =
			BitmapTable::InvalidRate;
		this->convertToAreaRate(headerItem.m_FreeAreaSize,
								afterFreeAreaRateValue,
								afterFreeAreaRate);

		if (beforeUnuseAreaRate != afterUnuseAreaRate ||
			beforeFreeAreaRate != afterFreeAreaRate)
		{
			// 物理エリア再配置により、
			// 空き領域管理表の更新も必要となった…

			// ※ 物理エリアの再配置により、物理ページ内の
			// 　 「未使用領域サイズ」と「空き領域サイズ」は等しくなる
			; _SYDNEY_ASSERT(afterUnuseAreaRate == afterFreeAreaRate);

			// 物理エリア再配置後の領域率ビットマップの値を得る
			unsigned char	afterBitValue =
				BitmapTable::ToBitmapValue[afterUnuseAreaRate]
										  [afterFreeAreaRate];

			// 空き領域管理表内の領域率ビットマップを上書きする
			AreaManageTable::Bitmap::overwriteValue(
				tablePointer,
				this->m_TableHeader->m_Type,
				this->m_ID,
				this->m_File->getPagePerManageTable(),
				afterBitValue);

			if (beforeUnuseAreaRate != afterUnuseAreaRate)
			{
				// 物理ページ内の未使用領域率が変更された…

				// 物理エリア再配置前の空き領域管理表ヘッダ内の
				// 「未使用領域率別の物理ページ数配列」を更新する
				this->m_TableHeader->updatePageArray(
					tablePointer,
					true, // 未使用領域率別の物理ページ数配列
					beforeUnuseAreaRateValue,
					afterUnuseAreaRateValue);
			}

			if (beforeFreeAreaRate != afterFreeAreaRate)
			{
				// 物理ページ内の空き領域率が変更された…

				// 物理エリア再配置前の、空き領域管理表ヘッダ内の
				// 「空き領域率別の物理ページ数配列」を更新する
				this->m_TableHeader->updatePageArray(
					tablePointer,
					false, // 空き領域率別の物理ページ数配列
					beforeFreeAreaRateValue,
					afterFreeAreaRateValue);
			}

			//
			// 空き領域管理表を更新したので、
			// 利用者側でエラーを検出した場合には
			// 空き領域管理表の修復が必要となった。
			//

			this->m_NecessaryRecoverTable = true;
		}

	}
	catch (Exception::Object&)
	{
		_SYDNEY_RETHROW;
	}
#ifndef NO_CATCH_ALL
	catch (...)
	{
		throw Exception::FileManipulateError(moduleName,
											 srcFile,
											 __LINE__);
	}
#endif
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getTopAreaID --
//		先頭の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での先頭物理エリアの識別子を返す。
//	（物理的に先頭の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const void*	PagePointer_
//		物理ページのバッファリング内容へのポインタ
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での先頭の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getTopAreaID(const void*	PagePointer_) const
{
	AreaID	topAreaID = ConstValue::UndefinedAreaID;

	// 物理ページヘッダに記録されている
	// 「管理している物理エリア数」を得る
	AreaNum	areaNum =
		(*(this->m_Header.GetManageAreaNum))(PagePointer_);

	if (areaNum > 0)
	{
		// 物理ページ内で管理している最終物理エリアの識別子を求める
		AreaID	lastAreaID = areaNum - 1;

		for (AreaID areaID = 0; areaID <= lastAreaID; areaID++)
		{
			if (m_Directory.isUsedArea(PagePointer_, areaID))
			{
				topAreaID = areaID;
				break;
			}
		}
	}

	return topAreaID;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getLastAreaID --
//		最後の使用中物理エリアの識別子を返す
//
//	NOTES
//	物理ページ内で（利用者が確保し、）使用中の物理エリアのうち、
//	物理エリア識別子順での最終物理エリアの識別子を返す。
//	（物理的に最後の物理エリアの識別子ではない。）
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const void*	PagePointer_
//		物理ページのバッファリング内容へのポインタ
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での最後の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getLastAreaID(const void*	PagePointer_) const
{
	AreaID	lastAreaID = ConstValue::UndefinedAreaID;

	// 物理ページヘッダに記録されている
	// 「管理している物理エリア数」を得る
	AreaNum	areaNum =
		(*(this->m_Header.GetManageAreaNum))(PagePointer_);

	if (areaNum > 0)
	{
		AreaID	areaID = areaNum - 1;

		while (1)
		{
			if (m_Directory.isUsedArea(PagePointer_, areaID))
			{
				lastAreaID = areaID;
				break;
			}

			if (areaID == 0)
			{
				break;
			}

			areaID--;
		}
	}

	return lastAreaID;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getNextAreaID --
//		次の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での次の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const void*					PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での次の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getNextAreaID(const void*	PagePointer_,
							  const AreaID	AreaID_) const
{
	AreaID	nextAreaID = ConstValue::UndefinedAreaID;

	// 物理ページヘッダに記録されている
	// 「管理している物理エリア数」を得る
	AreaNum	areaNum =
		(*(this->m_Header.GetManageAreaNum))(PagePointer_);

	if (areaNum > 0)
	{
		// 物理ページ内で管理している最終物理エリアの識別子を求める
		AreaID	lastAreaID = areaNum - 1;

		for (AreaID areaID = AreaID_ + 1; areaID <= lastAreaID; areaID++)
		{
			if (m_Directory.isUsedArea(PagePointer_, areaID))
			{
				nextAreaID = areaID;
				break;
			}
		}
	}

	return nextAreaID;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getPrevAreaID --
//		前の使用中物理エリアの識別子を返す
//
//	NOTES
//	引数AreaID_で指定される物理エリアの、
//	物理エリア識別子順での前の使用中物理エリアの識別子を返す。
//	該当する物理エリアが物理ページ内に存在しない場合には、
//	PhysicalFile::ConstValue::UndefinedAreaIDを返す。
//
//	ARGUMENTS
//	const void*					PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaID	AreaID_
//		物理エリア識別子
//
//	RETURN
//	PhysicalFile::AreaID
//		物理エリア識別子順での前の使用中物理エリアの識別子
//
//	EXCEPTIONS
//	なし
//
AreaID
AreaManagePage::getPrevAreaID(const void*	PagePointer_,
							  const AreaID	AreaID_) const
{
	AreaID	prevAreaID = ConstValue::UndefinedAreaID;

	if (AreaID_ > 0)
	{
		AreaID	areaID = AreaID_ - 1;

		// 物理ページヘッダに記録されている
		// 「管理している物理エリア数」を得る
		AreaNum	areaNum =
			(*(this->m_Header.GetManageAreaNum))(PagePointer_);

		// 物理ページ内で管理している最終物理エリアの識別子を求める
		AreaID	lastAreaID = areaNum - 1;

		if (areaID > lastAreaID)
		{
			// 物理ページ内で管理していない
			// 物理エリアの識別子が指定された…

			// 物理ページ内で管理している最終物理エリアから検索開始
			areaID = lastAreaID;
		}

		while (1)
		{
			if (m_Directory.isUsedArea(PagePointer_, areaID))
			{
				prevAreaID = areaID;
				break;
			}

			if (areaID == 0)
			{
				break;
			}

			areaID--;
		}
	}

	return prevAreaID;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getUnuseAreaSize --
//		未使用領域サイズを返す
//
//	NOTES
//	物理ページ内の未使用領域サイズを返す。
//	返される値は、現在の未使用領域サイズから、
//	引数AreaNum_で指定される物理エリア数での
//	物理エリア管理ディレクトリのサイズ分を引いた値である。
//
//	ARGUMENTS
//	const void*					PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaNum	AreaNum_
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の未使用領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePage::getUnuseAreaSize(const void*	PagePointer_,
								 const AreaNum	AreaNum_) const
{
	// 物理ページヘッダから「未使用領域サイズ」を読み込む
	PageSize	unuseAreaSize =
		(*(this->m_Header.GetUnuseAreaSize))(PagePointer_);

	// 物理ページヘッダから「管理している物理エリア数」を読み込む
	AreaNum	currentAreaNum =
		(*(this->m_Header.GetManageAreaNum))(PagePointer_);

	// 上書き可能な物理エリア情報数を取得する
	AreaNum	overwriteAreaNum =
		m_Directory.getOverwriteAreaNum(this->m_VersionPageTop,
										currentAreaNum);

	if (overwriteAreaNum > AreaNum_)
	{
		overwriteAreaNum = AreaNum_;
	}

	// 追加されるであろう物理エリア管理ディレクトリのサイズを求める
	PageSize	additionalDirectorySize =
		m_Directory.getSize(
			currentAreaNum + AreaNum_ - overwriteAreaNum) -
		m_Directory.getSize(currentAreaNum);

	// 物理エリア管理ディレクトリのサイズを考慮して、
	// 未使用領域サイズを返す
	if (unuseAreaSize < additionalDirectorySize)
	{
		return 0;
	}
	return unuseAreaSize - additionalDirectorySize;
}

//
//	FUNCTION private
//	PhysicalFile::AreaManagePage::getFreeAreaSize --
//		空き領域サイズを返す
//
//	NOTES
//	物理ページ内の空き領域サイズを返す。
//	返される値は、現在の未使用領域サイズから、
//	引数AreaNum_で指定される物理エリア数での
//	物理エリア管理ディレクトリのサイズ分を引いた値である。
//
//	ARGUMENTS
//	const void*					PagePointer_
//		物理ページのバッファリング内容へのポインタ
//	const PhysicalFile::AreaNum	AreaNum_
//		生成物理エリア数
//		（物理ページ内にいくつの物理エリアを生成するか）
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページ内の空き領域サイズ [byte]
//
//	EXCEPTIONS
//	なし
//
PageSize
AreaManagePage::getFreeAreaSize(const void*		PagePointer_,
								const AreaNum	AreaNum_) const
{
	// 物理ページヘッダから「空き領域サイズ」を読み込む
	PageSize	freeAreaSize =
		(*(this->m_Header.GetFreeAreaSize))(PagePointer_);

	// 物理ページヘッダから「管理している物理エリア数」を読み込む
	AreaNum	currentAreaNum =
		(*(this->m_Header.GetManageAreaNum))(PagePointer_);

	// 上書き可能な物理エリア情報数を取得する
	AreaNum	overwriteAreaNum =
		m_Directory.getOverwriteAreaNum(this->m_VersionPageTop,
										currentAreaNum);

	if (overwriteAreaNum > AreaNum_)
	{
		overwriteAreaNum = AreaNum_;
	}

	// 追加されるであろう物理エリア管理ディレクトリのサイズを求める
	PageSize	additionalDirectorySize =
		m_Directory.getSize(currentAreaNum + AreaNum_) -
			m_Directory.getSize(currentAreaNum);

	// 物理エリア管理ディレクトリのサイズを考慮して、
	// 空き領域サイズを返す
	if (freeAreaSize < additionalDirectorySize)
	{
		return 0;
	}
	return freeAreaSize - additionalDirectorySize;
}

//
//	Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
