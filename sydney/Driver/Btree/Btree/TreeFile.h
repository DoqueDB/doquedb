// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeFile.h -- ツリーファイルクラスのヘッダーファイル
// 
// Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_TREEFILE_H
#define __SYDNEY_BTREE_TREEFILE_H

#include "Common/Object.h"

#include "PhysicalFile/Types.h"

_SYDNEY_BEGIN

namespace Btree
{

class FileParameter;

//
//	CLASS
//	Btree::TreeFile -- ツリーファイルクラス
//
//	NOTES
//	ツリーファイルクラス。
//
class TreeFile : public Common::Object
{
public:

#ifdef OBSOLETE
	// コンストラクタ
	TreeFile(const FileParameter*	FileParam_);
#endif //OBSOLETE

#ifdef OBSOLETE
	// デストラクタ
	~TreeFile();
#endif //OBSOLETE

	//
	// 静的データメンバ
	//

	// ツリーファイル格納先ディレクトリ名
	static const ModUnicodeString		DirectoryName; // = "Tree"

	// ヘッダページの物理ページ識別子
	static const PhysicalFile::PageID	HeaderPageID; // = 0

private:

}; // end of class Btree::TreeFile

} // end of namespace Btree

_SYDNEY_END

#endif //__SYDNEY_BTREE_TREEFILE_H

//
//	Copyright (c) 2001, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
