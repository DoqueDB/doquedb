// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalBitmapFile.cpp --
// 
// Copyright (c) 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "Bitmap/NormalBitmapFile.h"
#include "Bitmap/Condition.h"
#include "Bitmap/NormalBitmapIterator.h"
#include "Bitmap/BitmapPage.h"

#include "Common/DataArrayData.h"
#include "Common/NullData.h"
#include "Common/UnsignedIntegerData.h"

#include "Os/AutoCriticalSection.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::NormalBitmapFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Bitmap::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
NormalBitmapFile::NormalBitmapFile(const FileID& cFileID_)
	: BitmapFile(cFileID_)
{}

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::~NormalBitmapFile -- デストラクタ
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
NormalBitmapFile::~NormalBitmapFile()
{
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::copy -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::BitmapFile*
//		コピーしたファイル
//
//	EXCEPTIONS
//
BitmapFile*
NormalBitmapFile::copy()
{
	return new NormalBitmapFile(m_cFileID);
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::verify -- 整合性検査
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
void
NormalBitmapFile::verify()
{
	Common::UnsignedIntegerData cValue;
	
	// まずB木の整合性検査
	BtreeFile::verify();
	
	// nullのビットマップの整合性検査
	if (getHeaderPage().getNullID(cValue) == true)
	{
		NormalBitmapIterator ite(*this);
		ite.setDirPage(cValue.getValue());
		ite.verify();
	}
	if (getHeaderPage().getAllNullID(cValue) == true)
	{
		NormalBitmapIterator ite(*this);
		ite.setDirPage(cValue.getValue());
		ite.verify();
	}
	
	// 次にその他のビットマップの整合性検査
	
	Condition cond(m_cFileID);
	search(&cond);	// B木を全数検索
	
	while (get(cValue))
	{
		NormalBitmapIterator ite(*this);
		ite.setDirPage(cValue.getValue());
		ite.verify();
	}
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::getIterator -- ビットマップイテレータを得る
//
//	NOTES
//	BtreeFile::search実行後に実行する必要がある
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::BitmapIterator*
//		ビットマップイテレータ。条件にマッチする間はgetできる。
//		すべてのイテレータを返したら、0を返す
//
//	EXCEPTIONS
//
BitmapIterator*
NormalBitmapFile::getIterator()
{
	// B木の検索結果を得る
	Common::UnsignedIntegerData cValue;
	if (get(cValue) == false)
		return 0;

	NormalBitmapIterator* i = new NormalBitmapIterator(*this);
	i->setDirPage(cValue.getValue());

	return i;
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::getIterator -- ビットマップイテレータを得る
//
//	NOTES
//	BtreeFile::search実行後に実行する必要がある
//
//	ARGUMENTS
//	Common::Data& cKey_
//		キー値 
//
//	RETURN
//	Bitmap::BitmapIterator*
//		ビットマップイテレータ。条件にマッチする間はgetできる。
//		すべてのイテレータを返したら、0を返す
//
//	EXCEPTIONS
//
BitmapIterator*
NormalBitmapFile::getIterator(Common::Data& cKey_)
{
	// B木の検索結果を得る
	Common::UnsignedIntegerData cValue;
	if (get(cKey_, cValue) == false)
		return 0;

	NormalBitmapIterator* i = new NormalBitmapIterator(*this);
	i->setDirPage(cValue.getValue());

	return i;
}

//
//	FUNCTION public
//	Bitmap::NormalBitmapFile::getIteratorForGroupBy
//		-- ビットマップイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::BitmapIterator*
//		ビットマップイテレータ
//
//	EXCEPTIONS
//
BitmapIterator*
NormalBitmapFile::getIteratorForGroupBy(const Common::Data& cValue_)
{
	const Common::UnsignedIntegerData& v
		= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&,
							   cValue_);
	
	NormalBitmapIterator* i = new NormalBitmapIterator(*this);
	i->setDirPage(v.getValue());

	return i;
}

//
//	FUNCTION protected
//	Bitmap::NormalBitmapFile::on -- ビットをONする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		ビットをONするキー値
//	ModUInt32 uiRowID_
//		ビットをONするROWID
//	bool isNull_
//		配列かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapFile::on(const Common::Data& cKey_,
					 ModUInt32 uiRowID_,
					 bool isNull_)
{
	// B木からDIRページを得る
	Common::UnsignedIntegerData cValue;
	PhysicalFile::PageID id = PhysicalFile::ConstValue::UndefinedPageID;
	if (get(cKey_, isNull_, cValue) == true)
		id = cValue.getValue();

	// ビットマップイテレータを確保する
	NormalBitmapIterator iterator(*this);
	iterator.setDirPage(id);

	// ビットをON
	iterator.on(uiRowID_);
			
	if (id == PhysicalFile::ConstValue::UndefinedPageID)
	{
		// B木に存在していなかったので、登録する
		cValue.setValue(iterator.getDirPageID());
		BtreeFile::insert(cKey_, cValue, isNull_);
	}
}

//
//	FUNCTION protected
//	Bitmap::NormalBitmapFile::off -- ビットをOFFする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::Data& cKey_
//		ビットをOFFするキー値
//	ModUInt32 uiRowID_
//		ビットをOFFするROWID
//	bool isNull_
//		配列かつデータがnullの場合はtrue、それ以外の場合はfalse
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
NormalBitmapFile::off(const Common::Data& cKey_,
					  ModUInt32 uiRowID_,
					  bool isNull_)
{
	// B木からDIRページを得る
	Common::UnsignedIntegerData cValue;
	if (get(cKey_, isNull_, cValue) == false)
	{
		// ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	// ビットマップイテレータを確保する
	NormalBitmapIterator iterator(*this);
	iterator.setDirPage(cValue.getValue());

	// ビットをOFF
	iterator.off(uiRowID_);
}

//
//	Copyright (c) 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
