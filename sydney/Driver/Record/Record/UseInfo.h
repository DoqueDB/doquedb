// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UseInfo.h --
//		レコードファイル内で使用している物理ページおよび物理エリアの
//		各識別子を登録するための情報クラスのヘッダファイル
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_USEINFO_H
#define __SYDNEY_RECORD_USEINFO_H

#include "Common/Object.h"

#include "PhysicalFile/Types.h"

#include "ModMap.h"

_SYDNEY_BEGIN

namespace Record
{

class File;

//
//	CLASS
//	Record::UseInfo --
//		レコードファイル内で使用している物理ページおよび物理エリアの
//		各識別子を登録するための情報クラス
//
//	NOTES
//	レコードファイル内で使用している物理ページおよび物理エリアの
//	各識別子を登録するための情報クラス。
//	整合性検査の際に物理ファイルマネージャに通知するために使用する。
//
class UseInfo
{
	friend class DirectFile;
	friend class VariableFile;

public:

	// 物理エリア識別子ベクター
	// （各物理ページごとに1つのベクター）
	typedef ModVector<PhysicalFile::AreaID>	AreaIDs;

	// 登録情報
	typedef
		ModMap<PhysicalFile::PageID, AreaIDs, ModLess<PhysicalFile::PageID> >
		Table;

	// コンストラクタ
	UseInfo();

	// デストラクタ
	~UseInfo();

	// レコードファイルで使用している物理ページと物理エリアの
	// 識別子を追加登録する
	void append(const PhysicalFile::PageID	PageID_,
				const PhysicalFile::AreaID	AreaID_
				= PhysicalFile::ConstValue::UndefinedAreaID);

	// 物理エリア識別子ベクターへの参照を返す
	const AreaIDs& getAreaIDs(const PhysicalFile::PageID	PageID_) const;

#ifdef OBSOLETE
	// 物理ページ内での最終物理エリアの識別子を返す
	PhysicalFile::AreaID
		getLastAreaID(const PhysicalFile::PageID	PageID) const;
#endif //OBSOLETE

private:

	// 登録情報
	Table					m_Table;

	// レコードファイルで使用中の最終物理ページの識別子
	PhysicalFile::PageID	m_LastPageID;

	// 物理エリア識別子ベクターの最大要素数
	PhysicalFile::AreaNum	m_AreaIDsMaxCount;

}; // end of class UseInfo

} // end of namespace Record

_SYDNEY_END

#endif //__SYDNEY_RECORD_USEINFO_H

//
//	Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
