// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenParameter.h --
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_OPENPARAMETER_H
#define __SYDNEY_RECORD_OPENPARAMETER_H

#include "Record/Module.h"

#include "Common/Common.h"
#include "Common/Object.h"

#include "FileCommon/OpenMode.h"


_SYDNEY_BEGIN

namespace Record
{

//
//	CLASS
//	Record::OpenParameter -- オープンオプションを保持するクラス
//
//	NOTES
//	オープンオプションを保持するクラス
//
class OpenParameter : public Common::Object
{
public:
	// コンストラクタ
	OpenParameter(const FileCommon::OpenMode::Mode	iOpenMode_,
				  const bool 						bEstimate_,
				  const bool						bSortOrder_);

	// ファイルをオープンするモード(インサート、削除など)
	FileCommon::OpenMode::Mode	m_iOpenMode;

	// コストを見積もるためにオープンした場合は true。レコードファイルは
	// 見積もりの場合もそうでない場合もオープン時の動作は変化しないので、
	// この値によって動作が変化することはない
	bool						m_bEstimate;

	// SCAN モードで get する場合、false ならば昇順でオブジェクトを
	// 取り出す。
	bool						m_bSortOrder;
};

}

_SYDNEY_END

#endif //__SYDNEY_RECORD_OPENPARAMETER_H

//
//	Copyright (c) 2000, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
