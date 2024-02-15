// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapFile.cpp --
// 
// Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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
#include "Bitmap/BitmapFile.h"
#include "Bitmap/Condition.h"

#include "Common/DataArrayData.h"
#include "Common/NullData.h"

#include "Exception/BadArgument.h"
#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_BITMAP_USING

//
//	FUNCTION public
//	Bitmap::BitmapFile::BitmapFile -- コンストラクタ
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
BitmapFile::BitmapFile(const FileID& cFileID_)
	: BtreeFile(cFileID_), m_cFileID(cFileID_), m_pHeaderPage(0)
{}

//
//	FUNCTION public
//	Bitmap::BitmapFile::~BitmapFile -- デストラクタ
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
BitmapFile::~BitmapFile()
{
	delete m_pHeaderPage;
}

//
//	FUNCTION public
//	Bitmap::BitmapFile::create -- ファイルを作成する
//
//	NOTES
//	オープン後に呼び出す必要がある
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
BitmapFile::create()
{
	// まず基底から
	File::create();
	
	try
	{
		// ヘッダーを初期化する
		initializeHeaderPage();

		// 変更を確定する
		flushAllPages();
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		recoverAllPages();
		destroy();
		rmdir();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	BitmapFile::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		キーとバリューの組
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitmapFile::insert(const Common::DataArrayData& cTuple_)
{
	// 引数チェック
	ModUInt32 uiRowID;
	if (checkArgument(cTuple_, uiRowID) == false)
	{
		// ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (m_cFileID.isArray())
	{
		// キーを得る
		const Common::Data& cKey = *(cTuple_.getElement(0));
		if (cKey.isNull())
		{
			// ビットをONする
			on(cKey, uiRowID, true);
		}
		else
		{
			// 配列
			const Common::DataArrayData& cKeyArray =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cKey);
			int n = cKeyArray.getCount();
			for (int i = 0; i < n; ++i)
			{
				// キーの配列要素を得る
				const Common::Data& cElement = *(cKeyArray.getElement(i));
				// ビットをONする
				on(cElement, uiRowID);
			}
		}
	}
	else
	{
		// 通常

		// キーを得る
		const Common::Data& cKey = *(cTuple_.getElement(0));
		// ビットをONする
		on(cKey, uiRowID);
	}

	// エントリ数を更新する
	getHeaderPage().incrementCount();
	// 最大ROWIDを更新する
	getHeaderPage().setMaxRowID(uiRowID);
}

//
//	FUNCTION public
//	BitmapFile::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		キーとバリューの組
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BitmapFile::expunge(const Common::DataArrayData& cTuple_)
{
	// 引数チェック
	ModUInt32 uiRowID;
	if (checkArgument(cTuple_, uiRowID) == false)
	{
		// ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	if (m_cFileID.isArray())
	{
		// キーを得る
		const Common::Data& cKey = *(cTuple_.getElement(0));
		if (cKey.isNull())
		{
			// ビットをOFFする
			off(cKey, uiRowID, true);
		}
		else
		{
			// 配列
			const Common::DataArrayData& cKeyArray =
				_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&, cKey);
			int n = cKeyArray.getCount();
			for (int i = 0; i < n; ++i)
			{
				// キーの配列要素を得る
				const Common::Data& cElement = *(cKeyArray.getElement(i));
				// ビットをOFFする
				off(cElement, uiRowID);
			}
		}
	}
	else
	{
		// 通常

		// キーを得る
		const Common::Data& cKey = *(cTuple_.getElement(0));
		// ビットをOFFする
		off(cKey, uiRowID);
	}

	// エントリ数を更新する
	getHeaderPage().decrementCount();
}

//
//	FUNCTION public
//	Bitmap::BitmapFile::flushAllPages -- 変更を確定する
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
BitmapFile::flushAllPages()
{
	if (m_pHeaderPage) m_pHeaderPage->detach();
	BtreeFile::flushAllPages();
}

//
//	FUNCTION public
//	Bitmap::BitmapFile::recoverAllPages -- 変更を破棄する
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
BitmapFile::recoverAllPages()
{
	if (m_pHeaderPage) m_pHeaderPage->detach();
	BtreeFile::recoverAllPages();
}

//
//	FUNCTION public
//	Bitmap::BitmapFile::getHeaderPage -- ヘッダーページを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Bitmap::HeaderPage&
//		ヘッダーページへの参照
//
//	EXCEPTIONS
//
Bitmap::HeaderPage&
BitmapFile::getHeaderPage()
{
	if (m_pHeaderPage == 0)
	{
		m_pHeaderPage = new HeaderPage(*this);
	}
	if (m_pHeaderPage->getPhysicalPage() == 0)
	{
		m_pHeaderPage->setPhysicalPage(attachPhysicalPage(0));
	}
	return *m_pHeaderPage;
}

//
//	FUNCTION protected
//	Bitmap::BitmapFile::initializeHeaderPage -- ヘッダーページを初期化する
//
//	NOTES
//	create時にしか呼ばれない
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
BitmapFile::initializeHeaderPage()
{
	// ページID=0のページを得る
	PhysicalFile::Page* p = allocatePhysicalPage();
	if (p->getID() != 0)
		_SYDNEY_THROW0(Exception::Unexpected);

	m_pHeaderPage = new HeaderPage(*this);
	m_pHeaderPage->setPhysicalPage(p);
	m_pHeaderPage->initialize();
}

//
//	FUNCTION protected
//	Bitmap::BitmapFile::checkArgument -- 引数をチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const Common::DataArrayData& cTuple_
//		引数
//	ModUInt32& uiRowID_
//		ROWID
//
//	RETURN
//	bool
//		引数が正しい場合は ture、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
BitmapFile::checkArgument(const Common::DataArrayData& cTuple_,
						  ModUInt32& uiRowID_)
{
	bool result = false;
	if (cTuple_.getCount() >= 2)
	{
		// とりあえず、要素は2以上ある

		const Common::Data& cKey = *(cTuple_.getElement(0));
		
		if ((m_cFileID.isArray() &&
			 (cKey.getType() == Common::DataType::Array || cKey.isNull())) ||
			(!m_cFileID.isArray() && cKey.getType() != Common::DataType::Array))
		{
			// 第2要素をチェックする
			
			const Common::Data& cValue = *(cTuple_.getElement(1));
			if (cValue.getType() == Common::DataType::UnsignedInteger)
			{
				// ok!
				uiRowID_
					= _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData&,
										   cValue).getValue();
				result = true;
			}
		}
	}
	return result;
}

//
//	Copyright (c) 2005, 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
