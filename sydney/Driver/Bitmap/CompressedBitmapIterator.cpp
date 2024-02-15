// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CompressedBitmapIterator.cpp -- 
// 
// Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Bitmap";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Bitmap/CompressedBitmapIterator.h"
#include "Bitmap/CompressedBitmapFile.h"
#ifdef DEBUG
#include "Bitmap/Parameter.h"
#endif

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

namespace
{
#ifdef DEBUG
	//	ページサイズ -- デバッグ用に利用する
	ParameterInteger _cDebugPageSize("Bitmap_DebugPageSize", 0);
#endif
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::CompressedBitmapIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	CompressedBitmapFile& cFile_
//		ファイル
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CompressedBitmapIterator::CompressedBitmapIterator(CompressedBitmapFile& cFile_)
	: m_cFile(cFile_)
{
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::~CompressedBitmapIterator
//		-- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
CompressedBitmapIterator::~CompressedBitmapIterator()
{
}

//
//	FUNCTION public static
//	Bitmap::CompressedBitmapIterator::getType -- タイプを得る
//
//	NOTES
//
//	ARGUMENTS
//	const PhysicalFile::DirectArea& cArea_
//		物理ファイルのエリア
//
//	RETURN
//	Bitmap::CompressedBitmapIterator::Type::Value
//		タイプ
//
//	EXCEPTIONS
//
CompressedBitmapIterator::Type::Value
CompressedBitmapIterator::getType(const PhysicalFile::DirectArea& cArea_)
{
	const ModUInt32* p
		= syd_reinterpret_cast<const ModUInt32*>(cArea_.operator const void*());
	return static_cast<Type::Value>(*p);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::split
//		-- 分割する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::CompressedBitmapIterator*
//		分割されたイテレータ
//
//	EXCEPTIONS
//
CompressedBitmapIterator*
CompressedBitmapIterator::split()
{
	_SYDNEY_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::getTransaction
//		-- トランザクションを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Trans::Transaction&
//		トランザクション
//
//	EXCEPTIONS
//
const Trans::Transaction&
CompressedBitmapIterator::getTransaction() const
{
	return m_cFile.getTransaction();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::getFixMode -- FixModeを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Buffer::Page::FixMode::Value
//		FixMode
//
//	EXCEPTIONS
//
Buffer::Page::FixMode::Value
CompressedBitmapIterator::getFixMode() const
{
	return m_cFile.getFixMode();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::getMaxStorableAreaSize
//	   	-- 最大エリアサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	PhysicalFile::AreaSize
//		最大エリアサイズ
//
//	EXCEPTIONS
//
PhysicalFile::AreaSize
CompressedBitmapIterator::getMaxStorableAreaSize() const
{
#ifdef DEBUG
	if (_cDebugPageSize.get() != 0)
		return static_cast<PhysicalFile::AreaSize>(_cDebugPageSize.get());
#endif
	return m_cFile.getMaxStorableAreaSize();
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::getFirstRowID
//		-- 先頭のROWIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		先頭のROWID
//
//	EXCEPTIONS
//
ModUInt32
CompressedBitmapIterator::getFirstRowID()
{
	_SYDNEY_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	Bitmap::CompressedBitmapIterator::getLastRowID
//		-- 最後のROWIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32
//		最後のROWID
//
//	EXCEPTIONS
//
ModUInt32
CompressedBitmapIterator::getLastRowID()
{
	_SYDNEY_THROW0(Exception::BadArgument);
}

//
//	Copyright (c) 2007, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
