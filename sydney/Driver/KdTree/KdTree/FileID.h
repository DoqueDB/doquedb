// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileID.h -- LogicalFile::FileID のラッパークラス
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

#ifndef __SYDNEY_KDTREE_FILEID_H
#define __SYDNEY_KDTREE_FILEID_H

#include "KdTree/Module.h"
#include "KdTree/Node.h"
#include "Common/IntegerArrayData.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "LogicalFile/OpenOption.h"
#include "Lock/Name.h"
#include "Os/Path.h"

#include "FileCommon/HintArray.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	TYPEDEF
//	Inverted::LogicalFileID --
//
//	NOTES
//	Inverted::FileIDが直接LogicalFile::FileIDを継承できないので、
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
			// 次元数
			Dimension = LogicalFile::FileID::DriverNumber::KdTree,
			// 最大計算回数
			MaxCalculateCount,
			// 探索タイプ
			TraceType
		};
	};
	
	// バージョン
	enum
	{
		Version1 = 0,

		// バージョン数
		ValueNum,
		// 現在のバージョン
		CurrentVersion = ValueNum - 1
	};

	// コンストラクタ
	FileID(const LogicalFile::FileID& cLogicalFileID_);
	// デストラクタ
	virtual ~FileID();

	// ファイルIDの内容を作成する
	void create();

	// ページサイズ
	int getPageSize() const;
	// 次元数を得る
	int getDimension() const;
	// 最大計算回数を得る
	int getMaxCalculateCount() const;
	// 探索タイプを得る
	Node::TraceType::Value getTraceType() const;

	// LockNameを得る
	const Lock::FileName& getLockName() const;

	// 読み取り専用か
	bool isReadOnly() const;
	// 一時か
	bool isTemporary() const;
	// マウントされているか
	bool isMounted() const;
	// マウントフラグを設定する
	void setMounted(bool bFlag_);

	// パス名を得る
	const Os::Path& getPath() const;
	void setPath(const Os::Path& cPath_);

	// 差分ファイルのサブパスを得る
	static const Os::Path& getSmallPath1();
	static const Os::Path& getSmallPath2();

	// バージョンをチェックする
	bool checkVersion(int iVersion_) const;
	// バージョンをチェックする
	static bool checkVersion(const LogicalFile::FileID& cLogicalFileID_);

	// 更新パラメータを設定する
	bool getUpdateParameter(const Common::IntegerArrayData& cUpdateField_,
							LogicalFile::OpenOption& cOpenOption_) const;

	// ヒントをパースする
	static void parseHint(const ModUnicodeString& cstrHint_,
						  Node::TraceType::Value& eTraceType_,
						  int& iMaxCount_);
	// 探索タイプの文字列から列挙型に変更する
	static Node::TraceType::Value castTraceType(const ModUnicodeString& type);
	// ヒントを読み出す
	static bool readHint(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_,
						 const ModUnicodeString& cstrKey_,
						 ModUnicodeString& cstrValue_);

private:
	// 次元数を設定する
	void setDimension(ModSize uiPageSize_);
	
	// 最大計算回数を得る
	static int
	getMaxCalculateCount(const ModUnicodeString& cstrHint_,
						 const FileCommon::HintArray& cHintArray_);
	// 探索タイプを得る
	static Node::TraceType::Value
	getTraceType(const ModUnicodeString& cstrHint_,
				 const FileCommon::HintArray& cHintArray_);
	
	// 以下はFileIDの中にあるが、スピードを考え同じ値をメンバーとして持つもの

	// Fileのロック名
	mutable Lock::FileName m_cLockName;
	// パス名
	mutable Os::Path m_cPath;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_FILEID_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
