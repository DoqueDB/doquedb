// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PageManageFile.cpp --
//		物理ページ管理機能付き物理ファイル関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Exception/BadArgument.h"
#include "Exception/FileManipulateError.h"
#include "Exception/IllegalFileAccess.h"
#include "Exception/MemoryExhaust.h"
#include "Exception/Unexpected.h"

#include "Common/Assert.h"

#include "Version/File.h"

#include "PhysicalFile/PageManageFile.h"
#include "PhysicalFile/PageManagePage.h"
#include "PhysicalFile/Message_DiscordManagePageNum.h"
#include "PhysicalFile/Message_DiscordUsePageNum.h"
#include "PhysicalFile/Message_DiscordUsePageNumInTable.h"
#include "PhysicalFile/Message_DiscordUnusePageNumInTable.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_USING
_SYDNEY_PHYSICALFILE_USING

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageTableクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::PageTable::getVersionPageID --
//		物理ページを管理している物理ページ表のバージョンページ識別子を返す
//
//	NOTES
//	引数PageID_が示す物理ページを管理している
//	物理ページ表のバージョンページ識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//	const PhysicalFile::PageNum	PagePerManageTable_
//		1つの物理ページ表が管理可能な物理ページ数
//
//	RETURN
//	Version::Page::ID
//		物理ページを管理している物理ページ表のバージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
// static
Version::Page::ID
PageTable::getVersionPageID(const PageID	PageID_,
							const PageNum	PagePerManageTable_)
{
	// 物理ファイル先頭から、引数PageID_で示される物理ページまでに
	// 物理ページ表がいくつ存在するかを求める
	PageNum	pageTableNum = PageID_ / PagePerManageTable_ + 1;

	// 物理ページ表のバージョンページ識別子を求めて返す
	return (pageTableNum - 1) * PagePerManageTable_ + pageTableNum;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageTable::Headerクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST public
//	PhysicalFile::PageTable::Header::SmallSize --
//		物理ページ表ヘッダサイズ
//
//	NOTES
//	各項目を2バイトで記録するタイプの
//	物理ページ表ヘッダのサイズ。[byte]
//
// static
const PageSize
PageTable::Header::SmallSize = sizeof(ShortPageNum) << 1;
//                                                  ~~~~ 「掛ける２」

//
//	CONST public
//	PhysicalFile::PageTable::Header::LargeSize --
//		物理ページ表ヘッダサイズ
//
//	NOTES
//	各項目を4バイトで記録するタイプの
//	物理ページ表ヘッダのサイズ。 [byte]
//
// static
const PageSize
PageTable::Header::LargeSize = sizeof(PageNum) << 1;

namespace
{

namespace _SmallPageTableHeader
{

//
//	FUNCTION
//	_SmallPageTableHeader::updateUsedPageNum --
//		使用中の物理ページ数を更新する
//
//	NOTES
//	物理ページ表ヘッダに記録されている
//	「使用中の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		使用中の物理ページ数への加算値
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
//	_SmallPageTableHeader::updateUnusePageNum --
//		未使用の物理ページ数を更新する
//
//	NOTES
//	物理ページ表ヘッダに記録されている
//	「未使用の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		未使用の物理ページ数への加算値
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
//	_SmallPageTableHeader::fetchOut --
//		物理ページ表ヘッダに記録されているすべての項目を取り出す
//
//	NOTES
//	物理ページ表ヘッダに記録されているすべての項目を取り出す。
//
//	ARGUMENTS
//	const void*								HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
//	PhysicalFile::PageTable::Header::Item&	Item_
//		物理ページ表ヘッダ項目構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOut(const void*				HeaderTop_,
		 PageTable::Header::Item&	Item_)
{
	const ShortPageNum*	pageNumReadPos =
		static_cast<const ShortPageNum*>(HeaderTop_);

	Item_.m_UsedPageNum = *pageNumReadPos++;

	Item_.m_UnusePageNum = *pageNumReadPos;
}

//
//	FUNCTION
//	_SmallPageTableHeader::getUsedPageNum --
//		使用中の物理ページ数を返す
//
//	NOTES
//	使用中の物理ページ数を取り出し、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
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
//	_SmallPageTableHeader::getUnusePageNum --
//		未使用の物理ページ数を返す
//
//	NOTES
//	未使用の物理ページ数を取り出し、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
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

} // end of namespace _SmallPageTableHeader

namespace _LargePageTableHeader
{

//
//	FUNCTION
//	_LargePageTableHeader::updateUsedPageNum --
//		使用中の物理ページ数を更新する
//
//	NOTES
//	物理ページ表ヘッダに記録されている
//	「使用中の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		使用中の物理ページ数への加算値
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
//	_LargePageTableHeader::updateUnusePageNum --
//		未使用の物理ページ数を更新する
//
//	NOTES
//	物理ページ表ヘッダに記録されている
//	「未使用の物理ページ数」を更新する。
//
//	ARGUMENTS
//	void*		HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
//	const int	AddNum_
//		未使用の物理ページ数への加算値
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
//	_LargePageTableHeader::fetchOut --
//		物理ページ表ヘッダに記録されているすべての項目を取り出す
//
//	NOTES
//	物理ページ表ヘッダに記録されているすべての項目を取り出す。
//
//	ARGUMENTS
//	const void*								HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
//	PhysicalFile::PageTable::Header::Item&	Item_
//		物理ページ表ヘッダ項目構造体への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
fetchOut(const void*				HeaderTop_,
		 PageTable::Header::Item&	Item_)
{
	const PageNum*	pageNumReadPos =
		static_cast<const PageNum*>(HeaderTop_);

	Item_.m_UsedPageNum = *pageNumReadPos++;

	Item_.m_UnusePageNum = *pageNumReadPos;
}

//
//	FUNCTION
//	_LargePageTableHeader::getUsedPageNum --
//		使用中の物理ページ数を返す
//
//	NOTES
//	使用中の物理ページ数を取り出し、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
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
//	_LargePageTableHeader::getUnusePageNum --
//		未使用の物理ページ数を返す
//
//	NOTES
//	未使用の物理ページ数を取り出し、返す。
//
//	ARGUMENTS
//	const void*	HeaderTop_
//		物理ページ表ヘッダ先頭へのポインタ
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

} // end of namespace _LargePageTableHeader

} // end of global namespace

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageTable::Headerクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::PageTable::Header::Header -- コンストラクタ
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
PageTable::Header::Header()
	: Common::Object(),
	m_Type(PageTable::Header::UnknownType)
{
}

//
//	FUNCTION public
//	PhysicalFile::PageTable::Header::~Header -- デストラクタ
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
PageTable::Header::~Header()
{
}

//
//	FUNCTION public
//	PhysicalFile::PageTable::Header::setType --
//		物理ページ表ヘッダタイプを設定する
//
//	NOTES
//	物理ページ表ヘッダタイプを設定する。
//
//	ARGUMENTS
//	const PhysicalFile::PageTable::Header::Type	Type_
//		物理ページ表ヘッダタイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
PageTable::Header::setType(const PageTable::Header::Type	Type_)
{
	this->m_Type = Type_;

	this->setFunction();
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageTable::Headerクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::PageTable::Header::setFunction -- 
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
PageTable::Header::setFunction()
{
	; _SYDNEY_ASSERT(this->m_Type != PageTable::Header::UnknownType);

	if (this->m_Type == PageTable::Header::SmallType)
	{
		// 各項目を2バイトで記録するタイプ…

		this->UpdateUsedPageNum = _SmallPageTableHeader::updateUsedPageNum;

		this->UpdateUnusePageNum = _SmallPageTableHeader::updateUnusePageNum;

		this->FetchOut = _SmallPageTableHeader::fetchOut;

		this->GetUsedPageNum = _SmallPageTableHeader::getUsedPageNum;

		this->GetUnusePageNum = _SmallPageTableHeader::getUnusePageNum;
	}
	else
	{
		// 各項目を4バイトで記録するタイプ…

		this->UpdateUsedPageNum = _LargePageTableHeader::updateUsedPageNum;

		this->UpdateUnusePageNum = _LargePageTableHeader::updateUnusePageNum;

		this->FetchOut = _LargePageTableHeader::fetchOut;

		this->GetUsedPageNum = _LargePageTableHeader::getUsedPageNum;

		this->GetUnusePageNum = _LargePageTableHeader::getUnusePageNum;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageTable::Bitmapクラスの定数など
//
///////////////////////////////////////////////////////////////////////////////

//
//	CONST private
//	PhysicalFile::PageTable::Bitmap::StartCaseBySmallHeader --
//		物理ページ使用状態ビットマップ開始位置
//
//	NOTES
//	各項目を2バイトで記録するタイプの物理ページ表ヘッダに続く
//	物理ページ使用状態ビットマップ開始位置。[byte]
//
// static
const PageOffset
PageTable::Bitmap::StartCaseBySmallHeader = PageTable::Header::SmallSize;

//
//	CONST private
//	PhysicalFile::PageTable::Bitmap::StartCaseByLargeHeader --
//		物理ページ使用状態ビットマップ開始位置
//
//	NOTES
//	各項目を4バイトで記録するタイプの物理ページ表ヘッダに続く
//	物理ページ使用状態ビットマップ開始位置。[byte]
//
// static
const PageOffset
PageTable::Bitmap::StartCaseByLargeHeader = PageTable::Header::LargeSize;

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageTable::Bitmapクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::PageTable::Bitmap::getOffset --
//		物理ページ使用状態ビットマップの開始位置を返す
//
//	NOTES
//	物理ページ表内の物理ページ使用状態ビットマップの開始位置を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageTable::Header::Type	HeaderType_
//		物理ページ表ヘッダタイプ
//
//	RETURN
//	PhysicalFile::PageOffset
//		物理ページ使用状態ビットマップの開始位置 [byte]
//
//	EXCEPTIONS
//	なし
//
// static
PageOffset
PageTable::Bitmap::getOffset(const PageTable::Header::Type	HeaderType_)
{
	return
		(HeaderType_ == PageTable::Header::SmallType) ?
			PhysicalFile::PageTable::Bitmap::StartCaseBySmallHeader :
			PhysicalFile::PageTable::Bitmap::StartCaseByLargeHeader;
}

//
//	FUNCTION public
//	PhysicalFile::PageTable::Bitmap::getBitPosition --
//		物理ページ使用状態ビットマップのビット位置を設定する
//
//	NOTES
//	引数PageID_で示される物理ページに対応する
//	物理ページ使用状態ビットマップのオフセットとビット番号
//	および物理ページ表のバージョンページ識別子を設定する。
//	オフセットは物理ページ使用状態ビットマップ先頭から
//	32ビット単位で設定され、
//	ビット番号には0〜31のいずれかの値が設定される。
//
//	ARGUMENTS
//	const PhysicalFile::PageTable::Header::Type	HeaderType_
//		物理ページ表ヘッダタイプ
//	const PhysicalFile::PageID					PageID_
//		物理ページ識別子
//	const PhysicalFile::PageNum					PagePerManageTable_
//		1つの物理ページ表で管理可能な物理ページ数
//	Version::Page::ID&							TableVersionPageID_
//		物理ページ表のバージョンページ識別子への参照
//	PhysicalFile::PageOffset&					Offset_
//		物理ページ使用状態ビットマップのオフセットへの参照 [byte]
//	unsigned int&								BitNumber_
//		ビット番号への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
PageTable::Bitmap::getBitPosition(
	const PageTable::Header::Type	HeaderType_,
	const PageID					PageID_,
	const PageNum					PagePerManageTable_,
	Version::Page::ID&				TableVersionPageID_,
	PageOffset&						Offset_,
	unsigned int&					BitNumber_)
{
	// 物理ページ表のバージョンページ識別子を設定する
	TableVersionPageID_ =
		PageTable::getVersionPageID(PageID_, PagePerManageTable_);

	// 物理ページ表内でのビット番号を求める
	unsigned int	bitNumberInTable = PageID_ % PagePerManageTable_;

	// 物理ページ使用状態ビットマップのオフセットを求める
	Offset_ =
		PageTable::Bitmap::getOffset(HeaderType_) +
			(((bitNumberInTable % PagePerManageTable_) >> 5) << 2);
	//                                                 ~~~~ 「割る３２」

	// ビット番号を求める
	BitNumber_ = bitNumberInTable % 32;
}

//
//	FUNCTION public
//	PhysicalFile::PageTable::Bitmap::overwriteValue --
//		物理ページ使用状態ビットマップのビットを上書きする
//
//	NOTES
//	引数PageID_で示される物理ページの使用状態を表す
//	物理ページ使用状態ビットマップ内の1ビットを上書きする。
//
//	ARGUMENTS
//	void*										TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageTable::Header::Type	HeaderType_
//		物理ページ表ヘッダタイプ
//	const PhysicalFile::PageID					PageID_
//		物理ページ識別子
//	const PhysicalFile::PageNum					PagePerManageTable_
//		1つの物理ページ表で管理可能な物理ページ数
//	const bool									BitON_
//		ビットをONにするのかOFFにするのか
//			true  : ビットをONにする
//			false : ビットをOFFにする
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
// static
void
PageTable::Bitmap::overwriteValue(
	void*							TablePointer_,
	const PageTable::Header::Type	HeaderType_,
	const PageID					PageID_,
	const PageNum					PagePerManageTable_,
	const bool						BitON_)
{
	// 物理ページ使用状態ビットマップ位置を得る
	Version::Page::ID	dummyTableVersionPageID = 0;
	PageOffset			offset = 0;
	unsigned int		bitNumber = 0;
	PageTable::Bitmap::getBitPosition(HeaderType_,
									  PageID_,
									  PagePerManageTable_,
									  dummyTableVersionPageID,
									  offset,
									  bitNumber);

	// 物理ページ表のバッファリング内容へのポインタを
	// 物理ページ使用状態ビットマップ位置まで進める
	char*	tablePointer = static_cast<char*>(TablePointer_);
	tablePointer += offset;

	// 物理ページ使用状態ビットマップから32ビットを取り出す
	unsigned int*	bitmapPointer =
		syd_reinterpret_cast<unsigned int*>(tablePointer);
	unsigned int	bitmapValue = *bitmapPointer;

	// 物理ページ使用状態ビットマップとマスクするデータを設定する
	unsigned int	maskData = 1;
	if (BitON_)
	{
		// ビットをONにする
		bitmapValue |= maskData << bitNumber;
	}
	else
	{
		// ビットをOFFにする
		bitmapValue &= ~(maskData << bitNumber);
	}

	// 物理ページ使用状態ビットマップのビットを上書きする
	*bitmapPointer = bitmapValue;
}

//
//	FUNCTION public
//	PhysicalFile::PageTable::Bitmap::isUsedPage--
//		使用中の物理ページかどうかをチェックする
//
//	NOTES
//	引数PageID_で示される物理ページが
//	使用中かどうかをチェックする。
//
//	ARGUMENTS
//	const void*									TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageTable::Header::Type	HeaderType_
//		物理ページ表ヘッダタイプ
//	const PhysicalFile::PageID					PageID_
//		物理ページ識別子
//	const PhysicalFile::PageNum					PagePerManageTable_
//		1つの物理ページ表で管理可能な物理ページ数
//
//	RETURN
//	bool
//		使用中の物理ページかどうか
//			true  : 引数PageID_が使用中の物理ページの識別子である
//			false : 引数PageID_が未使用の物理ページの識別子である
//
//	EXCEPTIONS
//	なし
//
// static
bool
PageTable::Bitmap::isUsedPage(
	const void*						TablePointer_,
	const PageTable::Header::Type	HeaderType_,
	const PageID					PageID_,
	const PageNum					PagePerManageTable_)
{
	// 物理ページ使用状態ビットマップ位置を得る
	Version::Page::ID	dummyTableVersionPageID = 0;
	PageOffset			offset = 0;
	unsigned int		bitNumber = 0;
	PageTable::Bitmap::getBitPosition(HeaderType_,
									  PageID_,
									  PagePerManageTable_,
									  dummyTableVersionPageID,
									  offset,
									  bitNumber);

	// 物理ページ表のバッファリング内容へのポインタを
	// 物理ページ使用状態ビットマップ位置まで進める
	const char*	tablePointer = static_cast<const char*>(TablePointer_);
	tablePointer += offset;

	// 物理ページ使用状態ビットマップから32ビットを取り出す
	const unsigned int*	bitmapPointer =
		syd_reinterpret_cast<const unsigned int*>(tablePointer);
	unsigned int	bitmapValue = *bitmapPointer;
	
	// 物理ページ使用状態ビットマップとマスクするデータを設定する
	unsigned int	maskData = 1 << bitNumber;

	return (bitmapValue & maskData) != 0;
}

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFileクラスのpublicメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::recoverPage --
//		物理ページ記述子を破棄し、ページ内容を元に戻す
//
//	NOTES
//	物理ページ記述子を破棄し、ページ内容をアタッチ前の状態に戻す。
//	※ FixMode::WriteとFixMode::Discardableを
//	　 マスクしたモードでアタッチした物理ページにのみ有効。
//
//	ARGUMENTS
//	PhysicalFile::Page*&	Page_
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
PageManageFile::recoverPage(Page*&	Page_)
{
	this->detachPage(Page_, Page::UnfixMode::NotDirty);
}

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::recoverPageAll --
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
void
PageManageFile::recoverPageAll()
{
	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	while (this->m_Page != 0)
	{
		this->detachPage(this->m_Page, Page::UnfixMode::NotDirty);
	}

	File::recoverPageAll();
}

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::attachPage -- 物理ページ記述子を生成する
//
//	NOTES
//	物理ページ記述子を生成し、返す。
//　FixMode が ReadOnly のときスレッドセーフ。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	const PhysicalFile::PageID					PageID_
//		記述子を生成する物理ページの識別子
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//																= Low
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::IllegalFileAccess
//		異なるフィックスモードで既にアタッチ中の物理ページの
//		記述子を生成しようとした
//		または、
//		整合性検査中に呼び出された
//	Exception::MemoryExhaust
//		メモリ不足のため、物理ページ記述子を生成できなかった
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
// virtual
Page*
PageManageFile::attachPage(
	const Trans::Transaction&					Transaction_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_/* = Low */)
{
	if (this->m_Check)
	{
		//
		// 整合性検査中は、attachPageによる物理ページアタッチを
		// 禁止する。物理ページをアタッチしたいのであれば、
		// verifyPageを呼び出すこと。
		//

		throw Exception::IllegalFileAccess(moduleName,
										   srcFile,
										   __LINE__);
	}

	Page*	page = 0;

	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	if (FixMode_ == Buffer::Page::FixMode::ReadOnly)
	{
		// FixMode が ReadOnly ならスレッドセーフ化のためにキャッシュを利用しない
		// detachPage() で毎回 delete するのでページの参照カウンタもインクリメントしない
		
		// 物理ページ記述子を生成する
		page = this->attachPage(Transaction_,
								this,
								PageID_,
								FixMode_,
								ReplacementPriority_);
	}
	else
	{
		if (this->m_Page != 0)
		{
			// 既にアタッチ中の物理ページが存在する…

			page = this->getAttachedPage(Transaction_,
										 PageID_,
										 FixMode_,
										 &ReplacementPriority_,
										 0);

			if (page == 0)
			{
				// 既にアタッチ中の物理ページには、
				// 識別子が等しいものはなかった…

				// 物理ページ記述子を生成する
				page = this->attachPage(Transaction_,
										this,
										PageID_,
										FixMode_,
										ReplacementPriority_);

				// アタッチ中の物理ページのリンクを張り替える

				//
				// ※ 上で this->m_Page が 0 かどうかをチェックしているが、
				// 　 this->attachPage()で下位モジュールから
				// 　 MemoryExhaustがthrowされた場合、
				// 　 リトライのためにthis->m_Pageにキャッシュされている
				// 　 物理ページのうち、不要なものに関しては
				// 　 デタッチされてしまい、結果として、
				// 　 this->m_Page が 0 になってしまうこともある。
				// 　 したがって、再度 this->m_Page が 0 かどうかの
				// 　 チェックが必要となる。
				//

				if (this->m_Page != 0) {
					page->m_Next = this->m_Page->m_Next;
					page->m_Prev = this->m_Page;

					this->m_Page->m_Next->m_Prev = page;
					this->m_Page->m_Next = page;
				} else {
					page->m_Next = page;
					page->m_Prev = page;
				}
			}
		}
		else
		{
			// アタッチ中の物理ページが存在しない…

			// 物理ページ記述子を生成する
			page = this->attachPage(Transaction_,
									this,
									PageID_,
									FixMode_,
									ReplacementPriority_);

			// アタッチ中の物理ページのリンクを初期化する

			page->m_Next = page;
			page->m_Prev = page;
		}

		// 物理ページ記述子の参照カウンタを更新する
		page->m_ReferenceCounter++;

		// 生成した物理ページ記述子を保持する
		this->m_Page = page;
	}
	
	return page;
}

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::detachPage --
//		物理ページ記述子を破棄する
//
//	NOTES
//	物理ページ記述子を破棄する。
//
//	ARGUMENTS
//	PhysicalFile::Page*&					Page_
//		物理ページ記述子への参照
//	PhysicalFile::Page::UnfixMode::Value	UnfixMode_ = Omit
//		アンフィックスモード
//	const bool								SavePage_ = false
//		利用者からは破棄したように見えても、実はキャッシュしておくか
//			true  : キャッシュしておく
//			false : 本当に破棄してしまう
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
PageManageFile::detachPage(Page*&					Page_,
						   Page::UnfixMode::Value	UnfixMode_/* = Omit */,
						   const bool				SavePage_/* = false */)
{
	; _SYDNEY_ASSERT(Page_ != 0);

	// 物理ページ記述子のリンクを保護するためにラッチする

//	Os::AutoCriticalSection	latch(_latch);

	// 物理ページ記述子の参照カウンタを更新する
	// ReadOnly でアタッチしたページは参照カウンタを利用しないため常に 0 になる
	if (Page_->m_ReferenceCounter > 0)
	{
		Page_->m_ReferenceCounter--;
	}

	if (Page_->m_ReferenceCounter == 0)
	{
		// 物理ページ記述子がどこからも参照されなくなった…

		if (SavePage_ == false)
		{
			// 本当にデタッチしても構わない…

			if (UnfixMode_ != Page::UnfixMode::Omit)
			{
				// 明示的にアンフィックスモードが指定された…

				// 物理ページのバッファリング内容をアンフィックスする
				Page_->m_Memory.unfix(
					UnfixMode_ == Page::UnfixMode::Dirty);

			} else {

				// 物理ページのバッファリング内容をアンフィックスする
				Page_->m_Memory.unfix(
					Page_->m_UnfixMode == Page::UnfixMode::Dirty);
			}

			if(Page_->m_Prev == 0 && Page_->m_Next == 0)
			{
				// ReadOnly でアタッチしたページ
				
				delete Page_, Page_ = 0;
			}
			else
			{
				// アタッチ中の物理ページのリンクを張り替える

				Page_->m_Prev->m_Next = Page_->m_Next;

				Page_->m_Next->m_Prev = Page_->m_Prev;

				bool	isLastPage = false;
				if (Page_ == Page_->m_Next)
				{
					// 他にアタッチ中の物理ページが存在しない…

					; _SYDNEY_ASSERT(Page_ == Page_->m_Prev);

					isLastPage = true;
				}

				Page*	prevPage = isLastPage ? 0 : Page_->m_Prev;

				// デタッチしたページはファイルがdetachされるまで保存する
				Page_->m_Free = m_DetachPage;
				m_DetachPage = Page_;
				Page_ = 0;

				this->m_Page = prevPage;
			}
		}
		else
		{
			// ReadOnly でページをアタッチした場合
			// ページを delete するタイミングがなくなるので save を認めない
			; _SYDNEY_ASSERT(!(Page_->m_Prev == 0 && Page_->m_Next == 0));

			if (Page_->m_UnfixMode != Page::UnfixMode::Dirty)
			{
				Page_->m_UnfixMode = UnfixMode_;
			}

			// ページ内容を更新する
			this->savePage(Page_,
						   Page_->m_UnfixMode == Page::UnfixMode::Dirty);
		}
	}
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	PhysicalFile::PageManageFile::getAllocatedSize --
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
PageManageFile::getAllocatedSize(
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
	PageNum usedPageNum = 0;
	PageNum unusePageNum = 0;
	FileHeader::fetchOutPageNum(fileHeaderPointer,
								usedPageNum,
								unusePageNum);

	// 物理ファイルに含まれているすべての物理ページ数を求める
	totalPageNum = usedPageNum + unusePageNum;

	PageNum	allocatedPageNum = 0;

	; _SYDNEY_ASSERT(totalPageNum > 0);

	Version::Page::ID	tableVersionPageID = 1;
	Version::Page::ID	lastTableVersionPageID =
		this->getManageTableVersionPageID(totalPageNum - 1);

	PageNum	skipNum = this->m_PagePerManageTable + 1;

	while (tableVersionPageID <= lastTableVersionPageID)
	{
		PagePointer tablePage =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	pageTablePointer = tablePage->operator const void*();
                 const void*	pageTablePointer = static_cast<const VersionPage&>(*tablePage).operator const void*();

		allocatedPageNum +=
			(*(this->m_TableHeader.GetUsedPageNum))(pageTablePointer);

		tableVersionPageID += skipNum;
	}

	FileSize	allocatedSize = static_cast<FileSize>(allocatedPageNum) * this->m_UserAreaSizeMax;

	return allocatedSize;
}
#endif // OBSOLETE

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::getTopPageID --
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
PageManageFile::getTopPageID(const Trans::Transaction&	Transaction_)
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
		PagePointer tablePage =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	pageTablePointer = tablePage->operator const void*();
                 const void*	pageTablePointer = static_cast<const VersionPage&>(*tablePage).operator const void*();

		usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(pageTablePointer);

		unusePageNum =
			(*(this->m_TableHeader.GetUnusePageNum))(pageTablePointer);


		if (usedPageNum > 0)
		{
			totalPageNum = usedPageNum + unusePageNum;

			for (PageID pid = 0; pid < totalPageNum; pid++, pageID++)
			{
				if (PageTable::Bitmap::isUsedPage(
						pageTablePointer,
						this->m_TableHeader.m_Type,
						pageID,
						this->m_PagePerManageTable))
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
//	PhysicalFile::PageManageFile::getLastPageID --
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
PageManageFile::getLastPageID(const Trans::Transaction&	Transaction_)
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

	return getLastPageID(Transaction_,
						 usedPageNum,
						 unusePageNum);
}

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::getNextPageID --
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
PageManageFile::getNextPageID(const Trans::Transaction&	Transaction_,
							  const PageID				PageID_)
{
	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	// 物理ファイルヘッダのバッファリング内容を得る（フィックスする）
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
		PagePointer tablePage =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	pageTablePointer = tablePage->operator const void*();
                 const void*	pageTablePointer = static_cast<const VersionPage&>(*tablePage).operator const void*();

		usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(pageTablePointer);

		unusePageNum =
			(*(this->m_TableHeader.GetUnusePageNum))(pageTablePointer);

		totalPageNum = usedPageNum + unusePageNum;

		if (usedPageNum > 0)
		{

			PageID	pid =
				firstTable ? (pageID % this->m_PagePerManageTable) : 0;

			for (; pid < totalPageNum; pid++, pageID++)
			{
				if (PageTable::Bitmap::isUsedPage(
						pageTablePointer,
						this->m_TableHeader.m_Type,
						pageID,
						this->m_PagePerManageTable))
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
			pageID +=
				(totalPageNum - (pageID % this->m_PagePerManageTable));
		}

		tableVersionPageID += skipNum;

		firstTable = false;
	}
	
	return nextPageID;
}

//
//	FUNCTION public
//	PhysicalFile::PageManageFile::getPrevPageID --
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
PageManageFile::getPrevPageID(const Trans::Transaction&	Transaction_,
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

	PageNum usedPageNum = 0;
	PageNum unusePageNum = 0;
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
		PagePointer tablePage =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	pageTablePointer = tablePage->operator const void*();
                const void*	pageTablePointer = static_cast<const VersionPage&>(*tablePage).operator const void*();

		usedPageNum =
			(*(this->m_TableHeader.GetUsedPageNum))(pageTablePointer);

		unusePageNum =
			(*(this->m_TableHeader.GetUnusePageNum))(pageTablePointer);

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
				if (PageTable::Bitmap::isUsedPage(
						pageTablePointer,
						this->m_TableHeader.m_Type,
						pageID,
						this->m_PagePerManageTable))
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
PageManageFile::getTableID(const PageID	PageID_)
{
	return this->getManageTableVersionPageID(PageID_);
}

void
PageManageFile::getTableHeader(
	const Trans::Transaction&	Transaction_,
	const Version::Page::ID		TableVersionPageID_,
	PageNum&					UsedPageNum_,
	PageNum&					UnusePageNum_,
	PageNum*					DummyPageNumByUnuseAreaRate_,
	PageNum*					DummyPageNumByFreeAreaRate_)
{
	const Version::Page::Memory&	table =
		Version::Page::fix(Transaction_,
						   *m_VersionFile,
						   TableVersionPageID_,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);

	const void*	tableTop = (const void*)table;

	PageTable::Header::Item	tableHeaderItem;

	(*(this->m_TableHeader.FetchOut))(tableTop, tableHeaderItem);

	UsedPageNum_ = tableHeaderItem.m_UsedPageNum;

	UnusePageNum_ = tableHeaderItem.m_UnusePageNum;
}

void
PageManageFile::getTableBitmap(
	const Trans::Transaction&	Transaction_,
	const Version::Page::ID		TableVersionPageID_,
	unsigned char*				BitmapBuffer_)
{
	const Version::Page::Memory&	table =
		Version::Page::fix(Transaction_,
						   *m_VersionFile,
						   TableVersionPageID_,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Low);

	const void*	tableTop = (const void*)table;

	PageTable::Header::Item	tableHeaderItem;

	(*(this->m_TableHeader.FetchOut))(tableTop, tableHeaderItem);

	const char*	tablePointer = static_cast<const char*>(tableTop);
	tablePointer +=
		PageTable::Bitmap::getOffset(this->m_TableHeader.m_Type);

	PageNum	managePageNum =
		tableHeaderItem.m_UsedPageNum + tableHeaderItem.m_UnusePageNum;
	ModOsDriver::Memory::copy(BitmapBuffer_,
							  tablePointer,
							  sizeof(unsigned int) *
							  (((managePageNum - 1) >> 5) + 1));
}

#endif // DEBUG

///////////////////////////////////////////////////////////////////////////////
//
//	PhysicalFile::PageManageFileクラスのprivateメンバ関数
//
///////////////////////////////////////////////////////////////////////////////

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::PageManageFile --
//		コンストラクタ
//
//	NOTES
//	コンストラクタ。
//	物理ページ管理機能付き物理ファイルの記述子を生成する。
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
//	Exception::BadArgument
//		不正な引数
//
PageManageFile::PageManageFile(
	const File::StorageStrategy&	FileStorageStrategy_,
	const File::BufferingStrategy&	BufferingStrategy_,
	const Lock::FileName*			LockName_,
	bool							batch_)
	: File(FileStorageStrategy_,
		   BufferingStrategy_,
		   LockName_,
		   batch_),
	  m_TableHeader()
{
	// バージョンファイル記述子は、
	// PageManageFileクラスの親クラスであるFileクラスのコンストラクタ内で
	// 生成している。

	//
	// 以下が、物理ページ管理機能付き物理ファイル記述子を生成するための
	// 固有の処理
	//

	// バージョンページデータサイズと
	// 1つの物理ページ表で管理可能な物理ページ数を求める
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
		(this->m_VersionPageDataSize - PageTable::Header::SmallSize) << 3;
	//                                                               ~~~~
	//                                 1バイトで8つの物理ページを
	//                                 管理する（1物理ページ=1ビット）ので
	//                                「8倍する」という意味

	//
	// 物理ページ表ヘッダタイプおよびアクセスする関数を設定する。
	//

	PageTable::Header::Type	tableHeaderType =
		PageTable::Header::UnknownType;

	if (this->m_PagePerManageTable <= 0xFFFF)
	{
		tableHeaderType = PageTable::Header::SmallType;
	}
	else
	{
		this->m_PagePerManageTable =
			(this->m_VersionPageDataSize - PageTable::Header::LargeSize)
			<< 3;

		tableHeaderType = PageTable::Header::LargeType;
	}

	this->m_TableHeader.setType(tableHeaderType);

	// 公開領域最大サイズを設定する
	this->m_UserAreaSizeMax = this->m_VersionPageDataSize;
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::~PageManageFile --
//		デストラクタ
//
//	NOTES
//	デストラクタ。
//	物理ページ管理機能付き物理ファイルの記述子を破棄する。
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
PageManageFile::~PageManageFile()
{
	// バージョンファイル記述子は、
	// PageManageFileクラスの親クラスである
	// Fileクラスのデストラクタ内で破棄している。
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::attachPage --
//		物理ページ記述子を生成する
//
//	NOTES
//	物理ページ記述子を生成し、返す。
//  FixMode が ReadOnly 時にスレッドセーフ。
//
//	ARGUMENTS
//	const Trans::Transaction&					Transaction_
//		トランザクション記述子への参照
//	PhysicalFile::File*							File_
//		物理ファイル記述子
//	const PhysicalFile::PageID					PageID_
//		記述子を生成する物理ページの識別子
//	const Buffer::Page::FixMode::Value			FixMode_
//		フィックスモード
//	const Buffer::ReplacementPriority::Value	ReplacementPriority_
//		バッファリング内容の破棄されにくさ
//
//	RETURN
//	PhysicalFile::Page*
//		物理ページ記述子
//
//	EXCEPTIONS
//	Exception::MemoryExhaust
//		メモリ不足
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる可能性がある
//
Page*
PageManageFile::attachPage(
	const Trans::Transaction&					Transaction_,
	File*										File_,
	const PageID								PageID_,
	const Buffer::Page::FixMode::Value			FixMode_,
	const Buffer::ReplacementPriority::Value	ReplacementPriority_)
{
	Page*	page = 0;

	int	retryCnt = 0;

#ifdef OBSOLETE
	while (retryCnt < 3)
#else
	while (retryCnt < 2)
#endif
	{
		try
		{
			// ReadOnly の場合はスレッドセーフ化のためにキャッシュを利用しない
			if (FixMode_ != Buffer::Page::FixMode::ReadOnly)
			{
				
				if (m_DetachPage)
				{
					// キャッシュがある
				
					page = m_DetachPage;
					m_DetachPage = m_DetachPage->m_Free;

					try
					{

						// バージョンページをfixする
				
						page->reset(Transaction_,
									PageID_,
									FixMode_,
									ReplacementPriority_);

					}
#ifdef NO_CATCH_ALL
					catch (Exception::Object&)
#else
					catch (...)
#endif
					{
						// 何か例外が発生したので戻す
						page->m_Free = m_DetachPage;
						m_DetachPage = page;

						_SYDNEY_RETHROW;
					}

                    break;
				}
			}
			
			// 物理ページ記述子を生成する
			page = allocatePageInstance(Transaction_, PageID_, FixMode_);
			break;
		}
		catch (Exception::MemoryExhaust&)
		{
			switch (retryCnt)
			{
			case 0:
				if (retryStepSelf() == false) _SYDNEY_RETHROW;
				break;
#ifdef OBSOLETE
			case 1:
				if (retryStepOthers() == false) _SYDNEY_RETHROW;
				break;
#endif
			default:
				_SYDNEY_RETHROW;
			}

			retryCnt++;
		}
	}

	return page;
}

//	FUNCTION private
//	PhysicalFile::PageManageFile::allocatePageInstance -- Allocate Page instance
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
PageManageFile::allocatePageInstance(
	const Trans::Transaction&			cTransaction_,
	PageID								uiPageID_,
	Buffer::Page::FixMode::Value		eFixMode_,
	Admin::Verification::Progress*		pProgress_,
	Buffer::ReplacementPriority::Value	eReplacementPriority_)
{
	if (pProgress_ == 0)
	{
		return new PageManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, eReplacementPriority_);
	}
	else
	{
		// For verify
		return new PageManagePage(
			cTransaction_, this, uiPageID_, eFixMode_, *pProgress_);
	}
}

//	FUNCTION private
//	PhysicalFile::PageManageFile::initialize -- 物理ファイル生成時の初期化
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
PageManageFile::initialize(const Trans::Transaction&	Trans_,
						   void*						FileHeader_)
{
	// 先頭物理ページ表を確保し、初期化
	// ※ 物理ファイル生成時には先頭物理ページを“未使用”とするので
	// 　 物理ページ表ヘッダに記録されている“未使用の物理ページ数”を
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

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::getLastPageID --
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
PageManageFile::getLastPageID(const Trans::Transaction&	Transaction_,
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
		PagePointer tablePage =
			fixVersionPage(Transaction_,
						   tableVersionPageID,
						   Buffer::Page::FixMode::ReadOnly,
						   Buffer::ReplacementPriority::Middle);

		//const void*	pageTablePointer = tablePage->operator const void*();
                 const void*	pageTablePointer = static_cast<const VersionPage&>(*tablePage).operator const void*(); 

		usedPageNum_ =
			(*(this->m_TableHeader.GetUsedPageNum))(pageTablePointer);

		unusePageNum_ =
			(*(this->m_TableHeader.GetUnusePageNum))(pageTablePointer);

		bool	exist = false;

		totalPageNum = usedPageNum_ + unusePageNum_;

		if (usedPageNum_ > 0)
		{
			for (PageID pid = totalPageNum - 1; pid >= 0; pid--, pageID--)
			{
				if (PageTable::Bitmap::isUsedPage(
						pageTablePointer,
						this->m_TableHeader.m_Type,
						pageID,
						this->m_PagePerManageTable))
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
//	PhysicalFile::PageManageFile::fetchOutPageNumFromManageTable --
//		物理ページ表から使用中の物理ページ数・未使用の
//		物理ページ数を取り出す
//
//	NOTES
//	物理ページ表から使用中の物理ページ数・未使用の物理ページ数を取り出す。
//
//	ARGUMENTS
//	const void*	TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
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
PageManageFile::fetchOutPageNumFromManageTable(
	const void*	TablePointer_,
	PageNum&	UsedPageNum_,
	PageNum&	UnusePageNum_) const
{
	// 物理ページ表ヘッダから
	// 「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す

	PageTable::Header::Item	headerItem;

	(*(this->m_TableHeader.FetchOut))(TablePointer_, headerItem);

	UsedPageNum_ = headerItem.m_UsedPageNum;
	UnusePageNum_ = headerItem.m_UnusePageNum;
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::updateUsedPageNum --
//		物理ページ表に記録されている使用中の物理ページ数を更新する
//
//	NOTES
//	物理ページ表に記録されている使用中の物理ページ数を更新する。
//
//	ARGUMENTS
//	void*		TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
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
PageManageFile::updateUsedPageNum(void*		TablePointer_,
								  const int	AddNum_)
{
	// 物理ページ表ヘッダの「使用中の物理ページ数」を更新する
	(*(this->m_TableHeader.UpdateUsedPageNum))(TablePointer_, AddNum_);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::updateUnusePageNum --
//		物理ページ表に記録されている未使用の物理ページ数を更新する
//
//	NOTES
//	物理ページ表に記録されている未使用の物理ページ数を更新する。
//
//	ARGUMENTS
//	void*		TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
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
PageManageFile::updateUnusePageNum(void*		TablePointer_,
								   const int	AddNum_)
{
	// 物理ページ表ヘッダの「未使用の物理ページ数」を更新する
	(*(this->m_TableHeader.UpdateUnusePageNum))(TablePointer_, AddNum_);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::isUsedPage --
//		使用中の物理ページかどうかをチェックする
//
//	NOTES
//	引数PageID_が使用中の物理ページの識別子かどうかを
//	チェックする。
//
//	ARGUMENTS
//	const void*					TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		チェックする物理ページの識別子
//
//	RETURN
//	bool
//		使用中の物理ページかどうか
//			true  : 引数PageID_が使用中の物理ページの識別子である
//			false : 引数PageID_が未使用の物理ページの識別子である
//
//	EXCEPTIONS
//	なし
//
bool
PageManageFile::isUsedPage(const void*	TablePointer_,
						   const PageID	PageID_) const
{
	; _SYDNEY_ASSERT(PageID_ != ConstValue::UndefinedPageID);

	return PageTable::Bitmap::isUsedPage(TablePointer_,
										 this->m_TableHeader.m_Type,
										 PageID_,
										 this->m_PagePerManageTable);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::searchNextAssignPage --
//		次に割り当てる物理ページを検索する
//
//	NOTES
//	次に割り当てる物理ページを検索し、識別子を返す。
//	指定された物理ページ表で未使用の物理ページを
//	管理していない場合にはPhysicalFile::ConstValue::UndefinedPageIDを返す。
//
//	ARGUMENTS
//	const void*	TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//
//	RETURN
//	PhysicalFile::PageID
//		次に割り当てる物理ページの識別子
//
//	EXCEPTIONS
//	なし
//
PageID
PageManageFile::searchNextAssignPage(const void*	TablePointer_) const
{
	// 物理ページ表に記録されている
	// 「使用中の物理ページ数」と「未使用の物理ページ数」を取り出す

	PageTable::Header::Item	headerItem;

	(*(this->m_TableHeader.FetchOut))(TablePointer_, headerItem);

	// 物理ページ表で管理している物理ページ数を求める
	PageNum	totalPageNum =
		headerItem.m_UsedPageNum + headerItem.m_UnusePageNum;

	// 未使用の物理ページを管理していない物理ページ表の場合には
	// PhysicalFile::ConstValue::UndefinedPageIDを返す
	if (headerItem.m_UnusePageNum == 0)
	{
		return ConstValue::UndefinedPageID;
	}

	// 物理ページ使用状態ビットマップポインタを設定する
	// ※ 物理ページ表先頭には使用中／未使用の物理ページ数が
	// 　 記録されているので、その分、ポインタを進める
	const char*	tablePointer = static_cast<const char*>(TablePointer_);
	tablePointer +=
		PageTable::Bitmap::getOffset(this->m_TableHeader.m_Type);

	PageID	pageID = ConstValue::UndefinedPageID;
	PageID	allocatePageID = 0;

	// 物理ページ使用状態ビットマップを32ビットずつ、なめる
	const unsigned int*	bitmapPointer =
		syd_reinterpret_cast<const unsigned int*>(tablePointer);
	for (unsigned int i = 0;
		 i < totalPageNum && pageID == ConstValue::UndefinedPageID;
		 i += 32, allocatePageID += 32, bitmapPointer++)
	{
		if (*bitmapPointer != 0xFFFFFFFF)
		{
			unsigned int	maskData = 1;

			for (int j = 0; j < 32; j++, maskData <<= 1)
			{
				if (((~(*bitmapPointer) & maskData) != 0) &&
					(i + j < totalPageNum))
				{
					// 未使用の物理ページが存在したので、
					// その物理ページの識別子を設定して、
					// 検索終了
					pageID = allocatePageID + j;
					break;
				}
			}
		}
	}

	// 物理ページ表ヘッダに記録されている未使用の物理ページ数を参照し、
	// 未使用の物理ページを管理している物理ページ表だということを
	// 確認したのに、該当する物理ページが存在しないのはおかしい
	; _SYDNEY_ASSERT(pageID != ConstValue::UndefinedPageID);

	return pageID;
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::getManageTableVersionPageID --
//		物理ページ表のバージョンページ識別子を返す
//
//	NOTES
//	引数PageID_が示す物理ページを管理している
//	物理ページ表のバージョンページ識別子を返す。
//
//	ARGUMENTS
//	const PhysicalFile::PageID	PageID_
//		物理ページ識別子
//
//	RETURN
//	Version::Page::ID
//		物理ページ表のバージョンページ識別子
//
//	EXCEPTIONS
//	なし
//
Version::Page::ID
PageManageFile::getManageTableVersionPageID(const PageID	PageID_) const
{
	return PageTable::getVersionPageID(PageID_,
									   this->m_PagePerManageTable);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::updateManageTable --
//		物理ページ表を更新する
//
//	NOTES
//	（物理ページ解放時または再利用時に呼び出され）
//	物理ページ表を更新する。
//
//	ARGUMENTS
//	void*						TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		確保／解放した物理ページの識別子
//	const PhysicalFile::PageNum	PageNum_
//		物理ファイルが管理している物理ページ数
//	const bool					ForReuse_
//		物理ページ再利用のために物理ページ表を更新するかどうか
//			true  : 物理ページ再利用のために更新する
//			false : 物理ページ解放のために更新する
//	const void*					DummyPagePointer_ = 0
//		物理ページのバッファリング内容へのポインタ
//		※ 物理ページ管理機能付き物理ファイルでは参照しない
//		　 （空き領域管理機能付き物理ファイルのための引数）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
PageManageFile::updateManageTable(void*			TablePointer_,
								  const PageID	PageID_,
								  const PageNum	PageNum_,
								  const bool	ForReuse_,
								  const void*	DummyPagePointer_ // = 0
								  )
{
	// 物理ページ使用状態ビットマップを更新する
	PageTable::Bitmap::overwriteValue(TablePointer_,
									  this->m_TableHeader.m_Type,
									  PageID_,
									  this->m_PagePerManageTable,
									  ForReuse_);

	int	addNum = ForReuse_ ? +1 : -1;

	// 使用中の物理ページ数を更新する
	(*(this->m_TableHeader.UpdateUsedPageNum))(TablePointer_, addNum);

	// 未使用の物理ページ数を更新する
	(*(this->m_TableHeader.UpdateUnusePageNum))(TablePointer_, -addNum);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::updatePageBitmap --
//		物理ページ使用状態ビットマップを更新する
//
//	NOTES
//	物理ページ表内の物理ページ使用状態ビットマップを更新する。
//
//	ARGUMENTS
//	void*						TablePointer_
//		物理ページ表のバッファリング内容へのポインタ
//	const PhysicalFile::PageID	PageID_
//		使用状態を更新する物理ページの識別子
//	const bool					BitON_
//		ビットをONにするかどうか
//			true  : ビットをONにする（ページを使用状態にする）
//			false : ビットをOFFにする（ページを未使用状態にする）
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
PageManageFile::updatePageBitmap(void*			TablePointer_,
								 const PageID	PageID_,
								 const bool		BitON_)
{
	PageTable::Bitmap::overwriteValue(TablePointer_,
									  this->m_TableHeader.m_Type,
									  PageID_,
									  this->m_PagePerManageTable,
									  BitON_);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::getPageDataSize --
//		物理ページデータサイズを返す
//
//	NOTES
//	物理ページデータサイズを返す。
//
//	ARGUMENTS
//	const Os::Memory::Size		VersionPageSize_
//		バージョンページサイズ [byte]
//	const PhysicalFile::AreaNum	DummyAreaNum_
//		ダミーの物理エリア数
//		※ public版のgetPageDataSizeと
//		　 インタフェースを分けるために必要。
//
//	RETURN
//	PhysicalFile::PageSize
//		物理ページデータサイズ [byte]
//
//	EXCEPTIONS
//	なし
//	※ 版管理マネージャ以下のモジュールからの例外は
//	　 そのまま利用者に投げられる
//
// static
PageSize
PageManageFile::getPageDataSize(const Os::Memory::Size	VersionPageSize_,
								const AreaNum			DummyAreaNum_)
{
	// バージョンページに格納可能な内容のサイズを得て、返す
	Os::Memory::Size	verifyVersionPageSize =
		Version::File::verifyPageSize(VersionPageSize_);
	return Version::Page::getContentSize(verifyVersionPageSize);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::checkPhysicalFile --
//		物理ページ管理機能付き物理ファイルの整合性検査を行う
//
//	NOTES
//	物理ページ管理機能付き物理ファイルの整合性検査を行う。
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
PageManageFile::checkPhysicalFile(
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
	// 2. 使用中物理ページ数一致検査
	//

	this->checkPageNumInFile(Transaction_,
							 false, // 使用中の物理ページ数
							 Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 3. 物理ページ表ごとの使用中物理ページ数一致検査
	//

	this->checkPageNumInTable(Transaction_,
							  true, // 使用中
							  Progress_);

	if (Progress_.isGood() == false)
	{
		return;
	}

	//
	// 4. 物理ページ表ごとの未使用物理ページ数一致検査
	//

	this->checkPageNumInTable(Transaction_,
							  false, // 未使用
							  Progress_);
}

//
//	FUNCTION private
//	PhysicalFile::PageManageFile::checkPageNumInFile --
//		管理物理ページ総数一致検査および使用中物理ページ数一致検査
//
//	NOTES
//	管理物理ページ総数一致検査…
//	物理ファイルヘッダに記録されている
//	「使用中の物理ページ数」と「未使用の物理ページ数」の和を
//	その物理ファイルの管理物理ページ数とし、
//	物理ファイル内に存在するすべての物理ページ表で管理している
//	物理ページ数の総和と一致するかどうかを検査する。
//	各物理ページ表にはヘッダがあり、
//	ヘッダにその物理ページ表が管理している
//	「使用中の物理ページ数」と「未使用の物理ページ数」が記録されている。
//
//	使用中物理ページ数一致検査…
//	物理ファイルヘッダに記録されている「使用中の物理ページ数」と、
//	物理ファイル内に存在するすべての物理ページ表（ヘッダ）に
//	記録されている「使用中の物理ページ数」の総和が
//	一致するかどうかを検査する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const bool						IsManage_
//		管理物理ページ数一致検査、使用中物理ページ数一致検査いずれの検査か
//			true  : 管理物理ページ総数一致検査
//			false : 使用中物理ページ数一致検査
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
PageManageFile::checkPageNumInFile(
	const Trans::Transaction&		Transaction_,
	const bool						IsManage_,
	Admin::Verification::Progress&	Progress_)
{
	PagePointer fileHeader =
		fixVersionPage(Transaction_,
					   FileHeader::VersionPageID,
					   Buffer::Page::FixMode::ReadOnly,
					   Buffer::ReplacementPriority::Low);

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
						   Buffer::ReplacementPriority::Low);

		//const void*	tableTop = table->operator const void*();
                const void*	tableTop = static_cast<const VersionPage&>(*table).operator const void*();
          
		usedPageNum = (*(this->m_TableHeader.GetUsedPageNum))(tableTop);

		unusePageNum =
			IsManage_ ?
				(*(this->m_TableHeader.GetUnusePageNum))(tableTop) : 0;

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
//	PhysicalFile::PageManageFile::checkPageNumInTable --
//		物理ページ表ごとの使用中物理ページ数一致検査および
//		物理ページ表ごとの未使用物理ページ数一致検査
//
//	NOTES
//	物理ページ表ごとの使用中物理ページ数一致検査…
//	物理ファイル内に存在するすべての物理ページ表について、
//	物理ページ表内の物理ページ使用状態ビットマップを参照し、
//	ビットマップ中の“使用中”を示すビット数が、
//	物理ページ表ヘッダに記録されている「使用中の物理ページ数」と
//	一致するかどうかを検査する。
//
//	物理ページ表ごとの未使用物理ページ数一致検査…
//	物理ファイル内に存在するすべての物理ページ表について、
//	物理ページ表内の物理ページ使用状態ビットマップを参照し、
//	ビットマップ中の“未使用”を示すビット数が、
//	物理ページ表ヘッダに記録されている「未使用の物理ページ数」と
//	一致するかどうかを検査する。
//
//	ARGUMENTS
//	const Trans::Transaction&		Transaction_
//		トランザクション記述子への参照
//	const bool						IsUsed_
//		使用中物理ページ数一致検査、未使用物理ページ数一致検査いずれか
//			true  : 物理ページ表ごとの使用中物理ページ数一致検査
//			false : 物理ページ表ごとの未使用物理ページ数一致検査
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
PageManageFile::checkPageNumInTable(
	const Trans::Transaction&		Transaction_,
	const bool						IsUsed_,
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
						   Buffer::ReplacementPriority::Low);

		//const void*	tableTop = table->operator const void*();
                  const void*	tableTop = static_cast<const VersionPage&>(*table).operator const void*();

		PageNum	pageNumInTable = 0;

		if (IsUsed_)
		{
			pageNumInTable =
				(*(this->m_TableHeader.GetUsedPageNum))(tableTop);
		}
		else
		{
			pageNumInTable =
				this->m_PagePerManageTable -
				(*(this->m_TableHeader.GetUsedPageNum))(tableTop);
		}

		PageID	lastPageID = pageID + this->m_PagePerManageTable - 1;

		PageNum	pageNum = 0;

		for (; pageID <= lastPageID; pageID++)
		{
			bool	used =
				PageTable::Bitmap::isUsedPage(tableTop,
											  this->m_TableHeader.m_Type,
											  pageID,
											  this->m_PagePerManageTable);

			if (IsUsed_ == used)
			{
				pageNum++;
			}
		}

		if (pageNumInTable != pageNum)
		{
			if (IsUsed_)
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					this->m_FilePath,
					Message::DiscordUsePageNumInTable(pageNumInTable,
													  pageNum));
			}
			else
			{
				_SYDNEY_VERIFY_INCONSISTENT(
					Progress_,
					this->m_FilePath,
					Message::DiscordUnusePageNumInTable(pageNumInTable,
														pageNum));
			}

			break;
		}

		tableVersionPageID += this->m_PagePerManageTable + 1;
	}
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
