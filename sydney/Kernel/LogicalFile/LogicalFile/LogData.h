// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.h --
// 
// Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_LOGDATA_H
#define __SYDNEY_LOGICALFILE_LOGDATA_H

#include "LogicalFile/Module.h"

#include "Trans/LogData.h"

class ModArchive;

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

//
//  CLASS
//  LogicalFile::LogData -- ファイルドライバー 内で扱うログクラス
//
//  NOTES
//	ログは Common::Externalizable を継承しているが、
//	そのインスタンスを取得するには、インスタンス取得関数を
//	Common::Externalizable に登録しておく必要がある。
//	しかし、ファイルドライバのロードは、ログの読み出しよりも後に
//	実行されるので、ファイルドライバ内でインスタンス取得関数を
//	実装することはできない。
//	よって、LogicalFile 内で、すべてのファイルドライバーの論理ログを実装する
//	こととする。
//
class LogData : public Trans::Log::ModificationData
{
public:
	typedef Trans::Log::ModificationData Super;

	//	ENUM
	//	LogicalFile::LogData::Type::Value -- ログの種別をあらわす
	//
	//	NOTES
	//	ログの種別をあらわす
	//	この値が直接ログに書かれるので、値を変更してはならない
	//
	struct Type
	{
		enum Value
		{
			Undefined			= 0,

			FullTextMerge		= 1,	// 全文索引のマージ
			KdTreeMerge			= 2,	// KD-Tree索引のマージ

			ValueNum
		};
	};

	// コンストラクタ(1)
	LogData();
	// コンストラクタ(2)
	LogData(const LogicalFile::LogData::Type::Value eType_);
	// デストラクタ
	virtual ~LogData();

	// このクラスをシリアル化する
	void serialize(ModArchive& archiver_);

	// 種別を得る
	Type::Value getType() const { return m_eType; }
	// 種別を設定する
	void setType(Type::Value eType_) { m_eType = eType_; }

protected:
private:
	// コピーしない
	LogData(const LogData& cOther_);
	LogData& operator=(const LogData& cOther_);

	// 種別
	Type::Value m_eType;
};

//
//	CLASS
//	LogicalFile::FullTextMergeLog -- 全文索引のマージのログ
//
//	NOTES
//
class SYD_LOGICALFILE_FUNCTION FullTextMergeLog : public LogData
{
public:
	// コンストラクタ
	FullTextMergeLog();
	// デストラクタ
	virtual ~FullTextMergeLog();

	// このクラスのクラス ID を得る
	int getClassID() const;

private:
	// 今のところ何のデータもない
};

//
//	CLASS
//	LogicalFile::KdTreeMergeLog -- KD-Tree索引のマージのログ
//
//	NOTES
//
class SYD_LOGICALFILE_FUNCTION KdTreeMergeLog : public LogData
{
public:
	// コンストラクタ
	KdTreeMergeLog();
	// デストラクタ
	virtual ~KdTreeMergeLog();

	// このクラスのクラス ID を得る
	int getClassID() const;

private:
	// 今のところ何のデータもない
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif // __SYDNEY_LOGICALFILE_LOGDATA_H

//
//	Copyright (c) 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
