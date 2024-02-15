//-*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntermediateFileID.h -- 論理ファイルIDの基底クラス
// 
// Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_INTERMEDIATEFILEID_H
#define __SYDNEY_INVERTED_INTERMEDIATEFILEID_H

#include "Inverted/Module.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/OpenOption.h"
#include "Lock/Name.h"
#include "FileCommon/HintArray.h"
#include "Os/Path.h"
#include "Common/IntegerArrayData.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

typedef LogicalFile::FileID LogicalFileID;

#if 0
//	FileID格納順(互換性のためこの順番)
//
enum FileIDNumber {_Exp0 = 0,_Ins0 ,_Exp1,_Ins1};
#endif
class IntermediateFileID : public LogicalFileID
{
public:
	
	// コンストラクタ
	SYD_INVERTED_FUNCTION	
	IntermediateFileID(const LogicalFile::FileID& cFileID_);
	// デストラクタ
	SYD_INVERTED_FUNCTION
	virtual ~IntermediateFileID();

	// マウントされているか
	SYD_INVERTED_FUNCTION	
	bool isMounted() const;
	// マウントされているかどうかを設定する
	SYD_INVERTED_FUNCTION	
	void setMounted(bool flag_);

	// 一時データベースかどうか
	SYD_INVERTED_FUNCTION	
	bool isTemporary() const;
	// ReadOnlyかどうか
	SYD_INVERTED_FUNCTION	
	bool isReadOnly() const;

	// ロック名を得る
	SYD_INVERTED_FUNCTION	
	const Lock::FileName& getLockName() const;
	// パス名を得る
	SYD_INVERTED_FUNCTION	
	const Os::Path& getPath() const;
	// パス名を設定する
	SYD_INVERTED_FUNCTION	
	void setPath(const Os::Path& cPath_);

	// 大転置のファイルIDを得る
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getInverted() const;
	// 挿入用転置のファイルIDを得る
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getInsert0();
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getInsert1();
	// 削除用転置のファイルIDを得る
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getExpunge0();
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getExpunge1();
	// セクション情報ファイルのファイルIDを得る
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getSection();
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getDelayProcFileID(int iIndex_);
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID& getInvertedFileID() const;

	// 子FileIDに親の現在値を設定する
	SYD_INVERTED_FUNCTION
	LogicalFile::FileID &setCurrentValue(LogicalFile::FileID& cFileID_) const;
	// 大転置のファイルIDを設定する
	SYD_INVERTED_FUNCTION
	void setInverted(const LogicalFile::FileID& cFileID_);
	// 大転置以外のファイルIDを設定する
	SYD_INVERTED_FUNCTION
	void setInverted(const LogicalFile::FileID& cFileID_,int id_);
private:
	// ヒントを解釈し、格納されている文字列を得る
	bool readHint(ModUnicodeString& cstrHint_,
				  FileCommon::HintArray& cHintArray_,
				  const ModUnicodeString& cstrKey_,
				  ModUnicodeString& cstrValue_);

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
};
_SYDNEY_INVERTED_END
_SYDNEY_END


#endif //__SYDNEY_INVERTED_INTERMEDIATEFILEID_H

//
//	Copyright (c) 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
