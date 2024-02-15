// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLogicalLogFile.h --
//		自動論理ログファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_AUTOLOGICALLOGFILE_H
#define	__SYDNEY_SCHEMA_AUTOLOGICALLOGFILE_H

#include "Schema/Module.h"

#include "Common/Object.h"

#include "LogicalLog/File.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS
//	Schema::AutoLogicalLogFile --
//		オブジェクト生成時に自動的に論理ログファイルをアタッチし、
//		破棄時に自動的にデタッチするクラス
//
//	NOTES

class AutoLogicalLogFile
	: public Common::Object
{
public:
	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::AutoLogicalLogFile -- コンストラクター
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	AutoLogicalLogFile()
		: m_pFile(0)
	{ }

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::AutoLogicalLogFile -- コンストラクター
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		const ModUnicodeString& cPath_
	//			アタッチする論理ログファイルのパス名
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	AutoLogicalLogFile(const ModUnicodeString& cPath_)
		: m_pFile(0)
	{
		m_pFile = LogicalLog::File::attach(cPath_);
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::AutoLogicalLogFile -- コンストラクター
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		const AutoLogicalLogFile& cOther_
	//			コピー元
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	AutoLogicalLogFile(const AutoLogicalLogFile& cOther_)
		: m_pFile(cOther_.m_pFile)
	{
		const_cast<AutoLogicalLogFile&>(cOther_).m_pFile = 0;
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::~AutoLogicalLogFile -- デストラクター
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	~AutoLogicalLogFile()
	{
		detach();
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::attach -- ファイルをアタッチする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		const ModUnicodeString& cPath_
	//			論理ログファイルのパス名
	//
	//	RETURN
	//		アタッチしたファイル
	//
	//	EXCEPTIONS

	LogicalLog::File*
	attach(const ModUnicodeString& cPath_)
	{
		// すでにアタッチされていたら一度デタッチする
		detach();

		return m_pFile = LogicalLog::File::attach(cPath_);
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::detach -- ファイルをデタッチする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	void
	detach()
	{
		if (m_pFile) {
			LogicalLog::File::detach(m_pFile);
			m_pFile = 0;
		}
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::create -- 論理ログファイルを作成する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	void
	create()
	{
		m_pFile->create();
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::destroy -- 論理ログファイルを破棄する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	void
	destroy()
	{
		m_pFile->destroy();
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::mount -- 論理ファイルをマウントする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS
	//
	void
	mount()
	{
		m_pFile->mount();
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::unmount -- 論理ファイルをアンマウントする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		なし
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS
	//
	void
	unmount()
	{
		m_pFile->unmount();
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::flush -- 論理ファイルをフラッシュする
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		const LogicalLog::LSN lsn = NoLSN
	//			記述する LSN
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS
	//
	//
	void
	flush(const LogicalLog::LSN lsn_ = LogicalLog::NoLSN)
	{
		m_pFile->flush(lsn_);
	}

	//	FUNCTION public
	//	Schema::AutoLogicalLogFile::move -- 論理ログファイルを移動する
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		const ModUnicodeString& cPath_
	//			移動先のパス名
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	void
	move(const ModUnicodeString& cPath_)
	{
		m_pFile->rename(cPath_);
	}

private:
	LogicalLog::File*			m_pFile;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_AUTOLOGICALLOGFILE_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

