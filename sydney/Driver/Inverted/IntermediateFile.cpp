// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntermediateFile.cpp -- 転置ファイルのラッパークラス
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/IntermediateFile.h"

#include "FileCommon/FileOption.h"
#include "FileCommon/IDNumber.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::IntermediateFile::IntermediateFile -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	LogicalFile::FileID& cFileID_
//		ファイルID
//	const Os::Path& cParent_
//		親ディレクトリ名
//	const ModUnicodeString& cDirectory_
//		サブディレクトリ名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
IntermediateFile::IntermediateFile(LogicalFile::FileID* pFileID_,
								   const Os::Path& cParent_,
								   const ModUnicodeString& cDirectory_)
	: m_pFileID(pFileID_), m_cDirectory(cDirectory_), InvertedFile()
{
	setNewPath(cParent_);
	setFileID(*m_pFileID);
}

//
//	FUNCTION public
//	Inverted::IntermediateFile::~IntermediateFile -- デストラクタ
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
IntermediateFile::~IntermediateFile()
{
}

//
//	FUNCTION public
//	Inverted::IntermediateFile::getNewPath -- 新しいパス名を作成する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cParent_
//		新しい親パス
//
//	RETURN
//	Os::Path
//		新しいパス名
//
//	EXCEPTIONS
//
Os::Path
IntermediateFile::getNewPath(const Os::Path& cParent_)
{
	Os::Path cNewPath = cParent_;
	cNewPath.addPart(m_cDirectory);
	return cNewPath;
}

//
//	FUNCTION public
//	Inverted::IntermediateFile::setNewPath -- 新しいパス名をFileIDに設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cParent_
//		新しい親パス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IntermediateFile::setNewPath(const Os::Path& cParent_)
{
	setPath(getNewPath(cParent_));
}

//
//	FUNCTION public
//	Inverted::IntermediateFile::getLockName -- ロック名を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Lock::FileName
//		ロック名
//
//	EXCEPTIONS
//
Lock::FileName
IntermediateFile::getLockName()
{
	return FileCommon::IDNumber(*m_pFileID).getLockName();
}

//
//	FUNCTION private
//	Inverted::IntermediateFile::setPath -- パス名を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Os::Path& cPath_
//		パス名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
IntermediateFile::setPath(const Os::Path& cPath_)
{
	m_pFileID->setString(
		_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Area::Key),
		cPath_);
	m_cPath = cPath_;
}

//
//	Copyright (c) 2003, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
