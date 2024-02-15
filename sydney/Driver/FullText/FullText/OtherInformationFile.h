// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile.h -- 全文のその他情報を格納するファイル
// 
// Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE_H
#define __SYDNEY_FULLTEXT_OTHERINFORMATIONFILE_H

#include "FullText/Module.h"
#include "FullText/File.h"

#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "PhysicalFile/DirectArea.h"

#include "ModVector.h"
#include "ModTypes.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

//	CLASS
//	FullText::OtherInformationFile
//		-- 全文ファイルの転置以外の情報を管理する
//
//	NOTES
//
class OtherInformationFile : public File
{
public:
	// 挿入する
	virtual void insert(ModUInt32 uiRowID_,
						const Common::DataArrayData& cValue_) = 0;
	// 更新する
	virtual void update(ModUInt32 uiRowID_,
						const Common::DataArrayData& cValue_,
						const ModVector<int>& vecUpdateFields_) = 0;
	// 削除する
	virtual void expunge(ModUInt32 uiRowID_) = 0;

	// 取得する
	virtual void get(ModUInt32 uiRowID_,
					 int iField_,
					 Common::Data& cValue_) = 0;

	// エリアを取得する
	virtual PhysicalFile::DirectArea getArea(ModUInt32 uiRowID_, int iField_)
	{
		return PhysicalFile::DirectArea();
	}

	// エリアの最大サイズを得る
	virtual ModSize getMaxStorableAreaSize() const { return 0; }
};

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_OTHERINFORMATIONFILE_H

//
//	Copyright (c) 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
