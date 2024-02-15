// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessFullText.h -- 再構成や整合性検査で論理ファイルを読み書きするためのクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_ACCESSFULLTEXT_H
#define	__SYDNEY_SCHEMA_ACCESSFULLTEXT_H

#include "Schema/AccessFile.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::AccessFullText -- 論理ファイルを読み書きするための情報を表すクラス
//
//	NOTES

class AccessFullText : public AccessFile
{
public:
	AccessFullText(Trans::Transaction& cTrans_, const File& cFile_)
		: AccessFile(cTrans_, cFile_) {}
	~AccessFullText() {}

protected:
	// 定数に対応するTreeNodeInterfaceを作る
	// 全文ファイルはNullDataの扱いが異なるのでオーバーライドする
	virtual TreeNode::Base*	createVariable(const Common::Data::Pointer& pVariable_);

private:
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_ACCESSFULLTEXT_H

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
