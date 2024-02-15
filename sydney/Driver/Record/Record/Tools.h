// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Tools.h -- どのクラスにも属さない関数のヘッダーファイル
// 
// Copyright (c) 2000, 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_TOOLS_H
#define __SYDNEY_RECORD_TOOLS_H

#include "Record/Module.h"

#include "Common/Common.h"
#include "Common/DataArrayData.h"
#include "FileCommon/AutoObject.h"
#include "FileCommon/DataManager.h"
#include "PhysicalFile/Types.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace PhysicalFile
{
class File;
class Page;
}

_SYDNEY_RECORD_BEGIN

class FreeAreaManager;
class MetaData;
class PhysicalPosition;
class TargetFields;
class VariableDataObjectInformation;

//
//	CLASS
//	Record::Tools -- ユーティリティ関数群
//
//	NOTES
//
//
class Tools
{
public:
	//
	// フィールドにアクセスする場合によく使われる関数
	//

	//
	// object type
	//

	typedef unsigned char			ObjectType;

	static const Os::Memory::Size	m_ObjectTypeArchiveSize;

	static const ObjectType			m_NormalObjectType;

	static const ObjectType			m_VariableObjectType;

	static const ObjectType			m_IndexObjectType;

	static const ObjectType			m_DivideObjectType;

	static const ObjectType			m_LinkedObjectType;

	static const ObjectType			m_ArrayObjectType;

	static const ObjectType			m_DivideArrayObjectType;

	static const ObjectType			m_DirectObjectType;

	static const ObjectType			m_UndefinedObjectType;

	//
	// 
	//

	typedef int						FieldNum;

	static const Os::Memory::Size	m_FieldNumArchiveSize;

	typedef	int						ElementNum;

	static const Os::Memory::Size	m_ElementNumArchiveSize;

	typedef	ModSize					ObjectIDNum;

	static const Os::Memory::Size	m_ObjectIDNumArchiveSize;

	typedef ModUInt64				ObjectID;

	static const Os::Memory::Size	m_ObjectIDArchiveSize;
	static const ObjectID			m_UndefinedObjectID;

	typedef ModUInt64				ObjectNum;

	static const Os::Memory::Size	m_ObjectNumArchiveSize;

	typedef Os::Memory::Size		FieldLength;

	static const Os::Memory::Size	m_FieldLengthArchiveSize;

	typedef unsigned char			NullMark;

	static const Os::Memory::Size	m_NullMarkArchiveSize;

	static const NullMark			m_ElementIsNull;

	static const NullMark			m_ElementIsNotNull;

	static const Os::Memory::Size	m_DateDataArchiveSize;

	static const Os::Memory::Size	m_TimeDataArchiveSize;

	// レコードファイル内で使用するデータ型
	typedef	FileCommon::DataManager::DataType	DataType;

	// 固定長フィールド値を読み込む
	static const char* readFixedField(const char* pAreaPointer_,
									  const DataType& cDataType_,
									  Common::Data& cCommonData_);

	// フィールド値を書き込む(データは固定長でなければいけない)
	static char* writeFixedField(char* pAreaPointer_,
								 const DataType& cDataType_,
								 const Common::Data& cCommonData_);

	// 可変長フィールドの圧縮前後のサイズを得る
	static void getVariableSize(const Common::Data& cData_,
								const DataType& cDataType_,
								FieldLength& iUncompressedSize_,
								FieldLength& iFieldSize_);

	//
	// 物理ファイルに(ページ、エリアを)アタッチする場合によく使う関数
	//
	static char* getAreaTop(PhysicalFile::Page*			Page_,
							const PhysicalFile::AreaID	AreaID_);

	static const char*
		getConstAreaTop(const PhysicalFile::Page*	Page_,
						const PhysicalFile::AreaID	AreaID_);

	//
	// bitmapに使う関数
	//

	class BitMap
	{
	public:
		BitMap();
		~BitMap();
		ModSize read(const char* pPointer_, ModSize iBase_, ModSize iEnd_);
		ModSize create(ModSize iBase_, ModSize iEnd_);
		ModSize write(char* pPointer_, ModSize iBase_, ModSize iEnd_) const;
		bool test(ModSize iIndex_) const;
		void set(ModSize iIndex_);
		void reset(ModSize iIndex_);
		void initialize(ModSize iBase_, ModSize iEnd_);

		void save();

		BitMap& operator= (const BitMap& cOther_);

	private:
		char* m_pPointer;
		const char* m_pOriginal;
		ModSize m_iBase;
		ModSize m_iEnd;
	};

	// bitmapに必要なバイト数を得る
	static ModSize getBitmapSize(ModSize n_);

	// bitmapを書き込む
	static ModSize writeBitmap(char*	pAreaPointer_,
							   ModSize	iBeginIndex_,
							   ModSize	iEndIndex_,
							   const BitMap&	cBitMap_);
	// null bitmapを読み込む
	static ModSize readBitmap(const char*	pAreaPointer_,
							  ModSize		iBeginIndex_,
							  ModSize		iEndIndex_,
							  BitMap&		cBitMap_);

	// ObjectIDに関する関数
	static ModSize readObjectID(const char* pAreaPointer_,
								Tools::ObjectID& cResult_);
	static ModSize writeObjectID(char* pAreaPointer_,
								 Tools::ObjectID cObjectID_);

	//
	// オブジェクトのオブジェクト種を判定する場合によく使う関数
	//

#ifndef SYD_COVERAGE
	// オブジェクト種に VariableObjectType のビットが立っている場合は true
	static bool isVariableObjectType(const Tools::ObjectType	ObjectType_);
#endif //SYD_COVERAGE

#ifndef SYD_COVERAGE
	// オブジェクト種に DirectObjectType のビットが立っている場合は true
	static bool isDirectObjectType(const Tools::ObjectType	ObjectType_);
#endif //SYD_COVERAGE

#ifndef SYD_COVERAGE
	// オブジェクト種に IndexObjectType のビットが立っている場合は true
	static bool isIndexObjectType(const Tools::ObjectType	ObjectType_);
#endif //SYD_COVERAGE

	// オブジェクト種に LinkedObjectType のビットが立っている場合は true
	static bool isLinkedObjectType(const Tools::ObjectType	ObjectType_);

#ifndef SYD_COVERAGE
	// オブジェクト種に DivideObjectType のビットが立っている場合は true
	static bool isDivideObjectType(const ObjectType	ObjectType_);
#endif //SYD_COVERAGE

	// コンフィギュレーション
	struct Configuration {

		// ページあたりのオブジェクト数の最小値
		static int getMinimumObjectPerPage();

	};

private:
};


//
//	CLASS
//	AutoFile -- 
//
//	NOTES
//	スコープの前後でファイルを attach/detach する。
//
template< class T >
class AutoFile : FileCommon::AutoObject
{
public:
	AutoFile(T& cFile_)
		: m_pFile(&cFile_)
	{
		m_pFile->attachFile();
	}
	AutoFile(T* pFile_)
		: m_pFile(pFile_)
	{
		if (m_pFile) m_pFile->attachFile();
	}
	~AutoFile()
	{
		detach();
	}
	void detach()
	{
		if (m_pFile) m_pFile->detachFile();
		m_pFile = 0;
	}
private:
	T* m_pFile;
};


//
//	CLASS
//	AutoDetachPageAll -- 
//
//	NOTES
//	スコープの最後でページを detachAll する。
//
template< class T >
class AutoDetachPageAll : FileCommon::AutoObject
{
public:
#ifdef OBSOLETE
	AutoDetachPageAll(T& cFile_)
		: m_bSucceeded(false)
		, m_pFile(&cFile_)
	{
	}
#endif //OBSOLETE
	AutoDetachPageAll(T* pFile_)
		: m_bSucceeded(false)
		, m_pFile(pFile_)
	{
	}
	~AutoDetachPageAll()
	{
		detachAll();
	}
	void succeeded() throw()
	{
		m_bSucceeded = true;
	}
	void detachAll()
	{
		// attachしていたページをすべてdetachする
		// ※detachPageAll() は複数回実行可能
		if (m_pFile) m_pFile->detachPageAll(m_bSucceeded);
	}
private:
	bool m_bSucceeded;
	T* const m_pFile;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_TOOLS_H

//
//	Copyright (c) 2000, 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
