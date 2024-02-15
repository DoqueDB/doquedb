// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataFile.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "KdTree";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "KdTree/DataFile.h"

_SYDNEY_USING
_SYDNEY_KDTREE_USING

namespace
{
}

//
//	FUNCTION public
//	KdTree::DataFile::DataFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	KdTree::FileID& cFileID_
//		ファイルID
//	const Os::Path& cPath_
//		パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DataFile::DataFile(FileID& cFileID_, const Os::Path& cPath_)
	: MultiFile(cFileID_, cPath_)
{
}

//
//	FUNCTION public
//	KdTree::DataFile::~DataFile -- デストラクタ
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
DataFile::~DataFile()
{
}

//
//	FUNCTION public
//	KdTree::DataFile::move -- ファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Os::Path& cNewPath_
//		移動先のパス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DataFile::move(const Trans::Transaction& cTransaction_,
			   const Os::Path& cNewPath_)
{
	if (Os::Path::compare(getPath(), cNewPath_)
		== Os::Path::CompareResult::Unrelated)
	{

		// 実体である OS ファイルが存在するか調べる

		const bool accessible = isAccessible();

		// 古いパスを保存する
		Os::Path cOldPath = getPath();

		int step = 0;
		try
		{
			MultiFile::move(cTransaction_, cNewPath_);
			step++;

			if (accessible)
				// 古いディレクトリを削除する
				rmdir(cOldPath);
			
			step++;
		}
		catch (...)
		{
			switch (step)
			{
			case 1:
				{
					MultiFile::move(cTransaction_, cOldPath);
				}
			case 0:
				if (accessible)

					// ディレクトリを破棄する
					//
					//【注意】  ディレクトリは
					//		  実体である物理ファイルの移動時に
					//		  必要に応じて生成されるが、
					//		  エラー時には削除されないので、
					//		  この関数で削除する必要がある

					rmdir(cNewPath_);
			}
			_SYDNEY_RETHROW;
		}
	}
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
