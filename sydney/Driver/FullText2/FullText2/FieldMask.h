// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FieldMask.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
#ifndef __SYDNEY_FULLTEXT2_FIELDMASK_H
#define __SYDNEY_FULLTEXT2_FIELDMASK_H

#include "FullText2/Module.h"
#include "FullText2/FieldType.h"
#include "FullText2/OpenOption.h"

#include "FileCommon/OpenOption.h"

#include "LogicalFile/TreeNodeInterface.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class FileID;

class FieldMask
{
public:
	//	同時に取得できる固定フィールドは以下のグループに分けられる。
	//	* normal
	//	* word
	//	* length
	//	各グループの詳細はコンストラクタを参照。
	//
	//	補足
	//	* ビットセットで取得できるのは、ROWIDだけを取得するときのみ。
	
	// コンストラクタ(更新用)
	FieldMask(const FileID& cFileID_);
	// コンストラクタ(検索用)
	FieldMask(const FileID& cFileID_, LogicalFile::OpenOption& cOpenOption_);

	// 取得可能かどうかを確認し、可能なら引数を得る
	bool check(const LogicalFile::TreeNodeInterface* pNode_,
			   const ModVector<int>& vecField_,
			   int& iField_,
			   OpenOption::Function::Value& eFunc_,
			   ModUnicodeString& cParam_);

	// 更新するフィールド番号は適切な範囲内か？
	bool checkValueRangeValidity(int n_) const;

	// スコア調整カラムか？
	bool isScoreAdjustField(int n_) const
		{ return (m_score == n_); }

private:
	// 取得グループ
	struct GetType {
		enum Value {
			None = 0,
			
			Normal,		// 通常
			Word,		// ワード
			Length		// 固定値
		};
	};
	
	// 固定フィールドの位置を移動
	void shift();

	// フィールド指定が正しいか確認する
	bool checkFieldExact(const LogicalFile::TreeNodeInterface* pNode_,
						 const ModVector<int>& vecField_);
	bool checkFieldOne(const LogicalFile::TreeNodeInterface* pNode_,
					   const ModVector<int>& vecField_,
					   int& iField_);
	bool checkFieldAny(const LogicalFile::TreeNodeInterface* pNode_,
					   int& iField_);
	// フィールド指定を得る
	bool getField(const LogicalFile::TreeNodeInterface* pNode_,
				  ModVector<int>& vecField);

	// FileID
	const FileID& m_cFileID;
	
	// ROWIDのフィールド番号
	ModInt32 m_rowid;
	// スコア調整カラムのフィールド番号
	ModInt32 m_score;

	// キー数
	int m_iKeyCount;

	// 取得グループ
	GetType::Value m_eGetType;

	// スキャンかどうか
	bool m_bScan;
	// ROWIDのみかどうか
	bool m_bOnlyRowID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_FIELDMASK_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
