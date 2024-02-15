// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntermediateFile.h --
// 
// Copyright (c) 2003, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INTERMEDIATEFILE_H
#define __SYDNEY_INVERTED_INTERMEDIATEFILE_H

#include "Inverted/Module.h"
#include "Inverted/InvertedFile.h"
#include "LogicalFile/FileID.h"
#include "Os/Path.h"
#include "Lock/Name.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//	CLASS
//	Inverted::IntermediateFile --
//
//	NOTES
//
//
class IntermediateFile : public InvertedFile
{
public:
	// コンストラクタ
	SYD_INVERTED_FUNCTION	
	IntermediateFile(){}
	SYD_INVERTED_FUNCTION
	IntermediateFile(LogicalFile::FileID * pFileID_,
					 const Os::Path& cParent_,
					 const ModUnicodeString& cDirectory_);
	// デストラクタ
	SYD_INVERTED_FUNCTION
	virtual ~IntermediateFile();

	SYD_INVERTED_FUNCTION	
	InvertedFile* getInvertedFile(){ return this;}
	// ファイルIDを得る
	SYD_INVERTED_FUNCTION	
	const LogicalFile::FileID* getFileID() const { return m_pFileID; }

	// パスを得る
	SYD_INVERTED_FUNCTION	
	Os::Path getPath() const { return m_cPath; }

	SYD_INVERTED_FUNCTION	
	ModUnicodeString & getDirectory() { return m_cDirectory; }
	
	// 新しいパスを得る
	SYD_INVERTED_FUNCTION	
	Os::Path getNewPath(const Os::Path& cParent_);
	// パスを設定する
	SYD_INVERTED_FUNCTION	
	void setNewPath(const Os::Path& cParent_);

	// ロック名を得る
	SYD_INVERTED_FUNCTION	
	Lock::FileName getLockName();

private:
	// パスを設定する
	void setPath(const Os::Path& cPath_);
	
	// ファイルID
	LogicalFile::FileID *m_pFileID;
	// ディレクトリ
	Os::Path m_cPath;
	// サブディレクトリのみ
	ModUnicodeString m_cDirectory;
protected:
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_INTERMEDIATEFILE_H

//
//	Copyright (c) 2003, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
