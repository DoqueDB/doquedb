// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessLobFile.h -- 再構成や整合性検査で論理ファイルを読み書きするためのクラス定義、関数宣言
// 
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_ACCESSLOBFILE_H
#define	__SYDNEY_SCHEMA_ACCESSLOBFILE_H

#include "Schema/AccessFile.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::AccessLobFile -- 論理ファイルを読み書きするための情報を表すクラス
//
//	NOTES

class AccessLobFile : public AccessFile
{
public:
	AccessLobFile(Trans::Transaction& cTrans_, const File& cFile_)
		: AccessFile(cTrans_, cFile_) {}
	~AccessLobFile() {}

protected:
	virtual bool			isGetData(Field* pField_) const;
private:
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_ACCESSLOBFILE_H

//
// Copyright (c) 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
