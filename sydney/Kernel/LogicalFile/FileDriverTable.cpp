// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileDriverTable.cpp -- ドライバIDとドライバ名とライブラリ名を管理する
// 
// Copyright (c) 1999, 2001, 2003, 2005, 2006, 2007, 2008, 2011, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "LogicalFile/FileDriverTable.h"

#include "Common/SystemParameter.h"
#include "Exception/EntryNotFound.h"
#include "Os/AutoCriticalSection.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_LOGICALFILE_USING

namespace
{

//
//	CLASS local
//	_$$::_Parameter -- パラメータを得る
//
class _Parameter
{
public:
	// コンストラクタ
	_Parameter(const char* pParameterName_, const ModCharString& cDefaultValue_)
		: m_bRead(false)
	{
		m_cstrParameterName = pParameterName_;
		m_cDefaultValue = cDefaultValue_;
	}

	// 値を得る
	const ModCharString& get()
	{
		if (m_bRead == false)
		{
			// パラメータを取得する
			ModUnicodeString cValue;
			if (Common::SystemParameter::getValue(m_cstrParameterName, cValue)
				== false)
			{
				// 設定されていない
				m_cValue = m_cDefaultValue;
			}
			else
			{
				// 設定されている
				m_cValue = cValue.getString();
			}
			m_bRead = true;
		}
		return m_cValue;
	}

private:
	// パラメータ
	ModCharString m_cValue;
	// デフォルト
	ModCharString m_cDefaultValue;

	// システムパラメータを参照したか
	bool m_bRead;

	// パラメータ名
	ModCharString m_cstrParameterName;
};

//
//	STRUCT
//	_$$::_Item -- 1つの要素をあらわす構造体
//
//	NOTES
//	ドライバIDとドライバ名をライブラリ名との1つの要素をあらわす構造体
//
struct _Item
{
	//ドライバ名
	const char* m_pszDriverName;
	//ドライバID
	int m_iDriverID;
	//ライブラリ名
	_Parameter* m_pName;
};

//
//	VARIABLE local
//
_Parameter cSyDrvRcd("LogicalFile_RecordFileDriver",	"SyDrvRcd");
_Parameter cSyDrvBtr("LogicalFile_BtreeFileDriver",		"SyDrvBtr2");
_Parameter cSyDrvInv("LogicalFile_InvertedFileDriver",	"SyDrvInv");
_Parameter cSyDrvFts("LogicalFile_FullTextFileDriver",	"SyDrvFts2");
_Parameter cSyDrvVct("LogicalFile_VectorFileDriver",	"SyDrvVct2");
_Parameter cSyDrvLob("LogicalFile_LobFileDriver",		"SyDrvLob");
_Parameter cSyDrvBmp("LogicalFile_BitmapFileDriver",	"SyDrvBmp");
_Parameter cSyDrvAry("LogicalFile_ArrayFileDriver",		"SyDrvAry");
_Parameter cSyDrvKtr("LogicalFile_KdTreeFileDriver",	"SyDrvKtr");

//
//	VARIABLE local
//	DriverTable -- ドライバIDとドライバ名とライブラリ名のテーブル
//
//	NOTES
//	ドライバIDとドライバ名とライブラリ名のテーブル
//
_Item DriverTable[] = {
	//レコードファイル
	{FileDriverName::Record,
	 FileDriverID::Record,
	 &cSyDrvRcd},
	
	//B木ファイル
	{FileDriverName::Btree,
	 FileDriverID::Btree,
	 &cSyDrvBtr},
	
	//転置ファイル
	{FileDriverName::Inverted,
	 FileDriverID::Inverted,
	 &cSyDrvInv},
	
	//全文ファイル
	{FileDriverName::FullText,
	 FileDriverID::FullText,
	 &cSyDrvFts},

	// ベクタファイル
	{FileDriverName::Vector,
	 FileDriverID::Vector,
	 &cSyDrvVct},

	// ロブファイル
	{FileDriverName::Lob,
	 FileDriverID::Lob,
	 &cSyDrvLob},
	
	//bitmapファイル
	{FileDriverName::Bitmap,
	 FileDriverID::Bitmap,
	 &cSyDrvBmp},

	//配列ファイル
	{FileDriverName::Array,
	 FileDriverID::Array,
	 &cSyDrvAry},

	//KD木ファイル
	{FileDriverName::KdTree,
	 FileDriverID::KdTree,
	 &cSyDrvKtr},

	{0,0,0}
};

}

//
//	VARIABLE private
//	LogicalFile::FileDriverTable::m_mapLibraryByID
//										-- ドライバIDとライブラリ名のマップ
//
//	NOTES
//	ドライバIDとライブラリ名のマップ。
//
ModMap<int, ModString, ModLess<int> >
FileDriverTable::m_mapLibraryByID;

//
//	VARIABLE private
//	LogicalFile::FileDriverTable::m_mapLibraryByName
//										-- ドライバ名とライブラリ名のマップ
//
//	NOTES
//	ドライバ名とライブラリ名のマップ。
//
ModMap<ModString, ModString, ModLess<ModString> >
FileDriverTable::m_mapLibraryByName;

//
//	VARIABLE private
//	LogicalFile::FileDriverTable::m_cCriticalSection -- 排他制御用
//
//	NOTES
//	排他制御用のクリティカルセクション
//
Os::CriticalSection
FileDriverTable::m_cCriticalSection;

//
//	VARIABLE private
//	LogicalFile::FileDriverTable::m_bInitialized -- 初期化したかどうか
//
//	NOTES
//	初期化したかどうか
//
bool
FileDriverTable::m_bInitialized = false;

//
//	FUNCTION public
//	LogicalFile::FileDriverTable::getLibraryName -- ライブラリ名を得る
//
//	NOTES
//	ファイルドライバIDからライブラリ名を得る。
//
//	ARGUMENTS
//	int iDriverID_
//		ファイルドライバID
//
//	RETURN
//	ModString
//		ライブラリ名
//
//	EXCEPTIONS
//	Common::EntryNotFoundException
//		ファイルドライバが見つからない
//
ModString
FileDriverTable::getLibraryName(int iDriverID_)
{
	// 初期化
	initialize();
	
	//マップを検索する
	ModMap<int, ModString, ModLess<int> >::Iterator i;
	i = m_mapLibraryByID.find(iDriverID_);
	if (i == m_mapLibraryByID.end())
	{
		//見つからない
		ModUnicodeOstrStream oStr;
		oStr << "driver id " << iDriverID_;
		_SYDNEY_THROW1(Exception::EntryNotFound, oStr.getString());
	}
	return (*i).second;
}

//
//	FUNCTION public
//	LogicalFile::FileDriverTable::getLibraryName -- ライブラリ名を得る
//
//	NOTES
//	ファイルドライバ名からライブラリ名を得る。
//
//	ARGUMENTS
//	const ModString& cstrDriverName_
//		ファイルドライバ名
//
//	RETURN
//	ModString
//		ライブラリ名
//
//	EXCEPTIONS
//	Common::EntryNotFoundException
//		ファイルドライバが見つからない
//
#ifndef SYD_COVERAGE

ModString
FileDriverTable::getLibraryName(const ModString& cstrDriverName_)
{
	// 初期化
	initialize();
	
	//マップを検索する
	ModMap<ModString, ModString, ModLess<ModString> >::Iterator i;
	i = m_mapLibraryByName.find(cstrDriverName_);
	if (i == m_mapLibraryByName.end())
	{
		//見つからない
		ModUnicodeOstrStream oStr;
		oStr << "driver " << cstrDriverName_;
		_SYDNEY_THROW1(Exception::EntryNotFound, oStr.getString());
	}
	return (*i).second;
}

#endif // end of #ifndef SYD_COVERAGE

//
//	FUNCTION private
//	LogicalFile::FileDriverTable::initialize -- 初期化する
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
FileDriverTable::initialize()
{
	Os::AutoCriticalSection cAuto(m_cCriticalSection);
	if (m_bInitialized == false)
	{
		//マップに格納する
		int i = 0;
		while (DriverTable[i].m_pName)
		{
			//ドライバID <-> ライブラリ名
			m_mapLibraryByID.insert(DriverTable[i].m_iDriverID,
									DriverTable[i].m_pName->get());
			//ドライバ名 <-> ライブラリ名
			m_mapLibraryByName.insert(DriverTable[i].m_pszDriverName,
									  DriverTable[i].m_pName->get());
			i++;
		}
		m_bInitialized = true;
	}
}

//
//	Copyright (c) 1999, 2001, 2003, 2005, 2006, 2007, 2008, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
