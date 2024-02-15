// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Archiver.h -- 
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_ARCHIVER_H
#define __SYDNEY_KDTREE_ARCHIVER_H

#include "KdTree/Module.h"
#include "KdTree/IndexFile.h"

#include "Version/Page.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//	CLASS
//	KdTree::Archiver --
//
//	NOTES
//
class Archiver
{
public:
	// コンストラクタ
	Archiver(IndexFile& cIndexFile_, bool bUpdate_);
	// デストラクタ
	virtual ~Archiver();

	// 書き込む
	void write(int n_);
	void write(const char* p_, int size_);

	// 読み込む
	void read(int& n_);
	void read(char* p_, int size_);

private:
	// 次のページを得る
	void nextPage();
	
	// 索引ファイル
	IndexFile& m_cIndexFile;
	// 更新モードかどうか
	bool m_bUpdate;

	// ページ
	Version::Page::Memory m_cPage;
	// ページID
	Version::Page::ID m_uiPageID;
	
	// ページ内位置
	union {
		const char* m_pConstBuffer;
		char* m_pBuffer;
	};

	// ページの終端
	union {
		const char* m_pConstEnd;
		char* m_pEnd;
	};
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_ARCHIVER_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
