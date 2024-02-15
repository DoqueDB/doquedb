// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- 論理ファイルIDの基底クラス
// 
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_FILEID_H
#define __SYDNEY_LOGICALFILE_FILEID_H

#include "LogicalFile/Module.h"
#include "LogicalFile/Parameter.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

//	CLASS
//	LogicalFile::FileID -- 論理ファイルID
//
//	NOTES
//	ファイルを識別するための論理ファイルID
//
//	※このファイルでの export はクラス(FileID)に対して行うこと。
//	  メソッド単位の export だと assert が発生する。

class SYD_LOGICALFILE_FUNCTION FileID
	: public Parameter::Base
{
public:
	struct KeyNumber {
		enum Value {
			Mounted,
			Area,						// array
			Temporary,
			PageSize,
			FieldNumber,
			FieldType,					// array
			FieldLength,				// array
			FieldEncodingForm,			// array; version2
			FieldFraction,				// array
			FieldFixed,					// array; version3
			FieldCollation,				// array; version5
			ElementType,				// array
			ElementLength,				// array
			ElementEncodingForm,		// array; version2
			ElementFixed,				// array; version3
			ReadOnly,
			AreaHint,
			FileHint,
			FieldHint,					// array
			SchemaDatabaseID,
			SchemaTableID,
			SchemaFileObjectID,
			KeyFieldNumber,
			VirtualFieldNumber,			// version4
			Unique,
			Version,
			ValueNum
		};
	};
	struct DriverNumber {
		enum Value {
			Record		= 0x1000,
			Btree		= 0x2000,
			Inverted	= 0x3000,
			FullText	= 0x4000,
			Vector		= 0x5000,
			Lob			= 0x6000,
			Bitmap		= 0x7000,
			Array		= 0x8000,
			FullText2	= 0x9000,
			KdTree		= 0xa000
		};
	};
	struct Unique {
		enum Value {
			Object,
			KeyField,
			ValueNum
		};
	};

	FileID();
	FileID(const FileID& cOther_);
	~FileID();

	// 代入オペレーター
	FileID& operator =(const FileID& cOther_) {return static_cast<FileID&>(Base::operator =(cOther_));}

	//クラスIDを得る
	virtual int getClassID() const;

	static void initialize();

protected:
	// バージョン番号に対応したレイアウトマップを取得する
	virtual const Parameter::LayoutMap* getLayoutMap(int iVersion_) const;

private:
	// バージョン番号に対応したレイアウトマップを取得する
	static const Parameter::LayoutMap* getOldLayoutMap(int iVersion_);
	// バージョン番号に対応したレイアウトマップを生成する
	static void createLayoutMap(int iVersion_, Parameter::LayoutMap& cMap_);

	bool* m_pbMounted;
	Parameter::Element<ModUnicodeString>::Array* m_pvecArea;
	bool* m_pbTemporary;
	int* m_piPageSize;
	int* m_piFieldNumber;
	Parameter::Element<int>::Array* m_pvecFieldType;
	Parameter::Element<int>::Array* m_pvecFieldLength;
	Parameter::Element<int>::Array* m_pvecFieldFraction;
	Parameter::Element<int>::Array* m_pvecFieldEncodingForm;
	Parameter::Element<bool>::Array* m_pvecFieldFixed;
	Parameter::Element<int>::Array* m_pvecFieldCollation;
	Parameter::Element<int>::Array* m_pvecElementType;
	Parameter::Element<int>::Array* m_pvecElementLength;
	Parameter::Element<int>::Array* m_pvecElementEncodingForm;
	Parameter::Element<bool>::Array* m_pvecElementFixed;
	bool* m_pbReadOnly;
	ModUnicodeString* m_pcstrAreaHint;
	ModUnicodeString* m_pcstrFileHint;
	Parameter::Element<ModUnicodeString>::Array* m_pvecFieldHint;
	int* m_piDatabaseID;
	int* m_piTableID;
	int* m_piFileID;
	int* m_piKeyFieldNumber;
	int* m_piVirtualFieldNumber;
	int* m_piUnique;
	int* m_piVersion;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_FILEID_H

//
//	Copyright (c) 1999, 2000, 2002, 2004, 2005, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
