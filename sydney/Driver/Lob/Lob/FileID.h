// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileIDのラッパークラス
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

#ifndef __SYDNEY_LOB_FILEID_H
#define __SYDNEY_LOB_FILEID_H

#include "Lob/Module.h"
#include "LogicalFile/FileID.h"
#include "Lock/Name.h"
#include "Buffer/Page.h"
#include "Trans/Transaction.h"
#include "FileCommon/HintArray.h"

#include "ModCharString.h"

_SYDNEY_BEGIN
_SYDNEY_LOB_BEGIN

//
//	TYPEDEF
//	Lob::LogicalFileID --
//
//	NOTES
//	Lob::FileIDが直接LogicalFile::FileIDを継承できないので、
//	このtypedefを間に挟む。VC6のバグ。
//
typedef LogicalFile::FileID LogicalFileID;

//
//	CLASS
//	Inverted::FileID -- 転置ファイルドライバーのFileID
//
//	NOTES
//	
//
class FileID : public LogicalFileID
{
public:
	struct KeyID
	{
		enum Value
		{
			// 圧縮かどうか
			Compressed = LogicalFile::FileID::DriverNumber::Lob
		};
	};

	//
	//	STRUCT
	//	Lob::LogicalInterface::FileType
	//
	struct FileType
	{
		enum Value
		{
			Unknown,
			BLOB,		// binary
			CLOB,		// char (まだサポートしていない)
			NCLOB		// ModUnicodeChar
		};
	};
	
	// コンストラクタ
	FileID(const LogicalFile::FileID& cLogicalFileID_);
	// デストラクタ
	virtual ~FileID();

	// ファイルIDの内容を作成する
	void create();

	// ページサイズ
	int getPageSize() const;

	// LockNameを得る
	const Lock::FileName& getLockName() const;

	// 読み取り専用か
	bool isReadOnly() const;
	// 一時か
	bool isTemporary() const;
	// マウントされているか
	bool isMounted() const;
	void setMounted(bool bFlag_);

	// 圧縮か
	bool isCompressed() const;

	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// バージョンをチェックする
	static bool checkVersion(const LogicalFile::FileID& cLogicalFileID_);

	// ファイルタイプを得る
	FileType::Value getFileType() const;

private:
	// FileIDの内容が正しいかチェックする
	void check();

	// 圧縮かどうかをヒントから設定する
	void setCompressed(ModUnicodeString& cstrHint_,
					   FileCommon::HintArray& cHintArray_);

	// ヒントを解釈し、格納されている文字列を得る
	bool readHint(ModUnicodeString& cstrHint_,
				  FileCommon::HintArray& cHintArray_,
				  const ModUnicodeString& cstrKey_,
				  ModUnicodeString& cstrValue_);

	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
	// FileType
	mutable FileType::Value m_eFileType;
	// 圧縮か
	mutable int m_iCompressed;
};

_SYDNEY_LOB_END
_SYDNEY_END

#endif //__SYDNEY_LOB_FILEID_H

//
//	Copyright (c) 2003, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
