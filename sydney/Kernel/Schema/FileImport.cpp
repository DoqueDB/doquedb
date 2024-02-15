// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileImport.cpp -- File::importの定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/FileReflect.h"

#include "Exception/NotSupported.h"

#include "Trans/LogData.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::File::undoTuple --
//		import中にロールバックされたタプルの更新をUNDOする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::Log::Data& cLogData_
//			UNDOする内容を表すログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
undoTuple(Trans::Transaction& cTrans_, const Trans::Log::Data& cLogData_)
{
	if (getIndexID() == ID::Invalid) {
		// 索引のファイル以外については今のところ
		// 再構成後の論理ログの反映を行うことはできない

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// 現在の値を調べてUNDOが必要かを調べる
	if (!m_pFileReflect->isUndoNeeded(cLogData_)) {
		// すでにUNDO後の状態になっているので不要
		return;
	}

	// 実際にUNDOする
	m_pFileReflect->undo();
}

//	FUNCTION public
//	Schema::File::redoTuple --
//		新規に作成したファイルに既存のデータを反映する
//
//	NOTES
//		ファイルの各フィールドから反映するデータの元になるフィールドに向かって
//		sourceの関係が作られている必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::Log::Data& cLogData_
//			REDOする内容を表すログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
redoTuple(Trans::Transaction& cTrans_, const Trans::Log::Data& cLogData_)
{
	if (getIndexID() == ID::Invalid) {
		// 索引のファイル以外については今のところ
		// 再構成後の論理ログの反映を行うことはできない

		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// 現在の値を調べてREDOが必要かを調べる
	if (!m_pFileReflect->isRedoNeeded(cLogData_)) {
		// すでにREDO後の状態になっているので不要
		return;
	}

	// 実際にREDOする
	m_pFileReflect->redo();
}

//	FUNCTION public
//	Schema::File::startImport --
//		importの開始時の処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
startImport(Trans::Transaction& cTrans_)
{
	if (!m_pFileReflect) {
		// 各論理ファイルに応じたFileReflectクラスのインスタンスを得る
		m_pFileReflect = new FileReflect(cTrans_, *this);
	}
}

//	FUNCTION public
//	Schema::File::endImport --
//		importの終了時の処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
File::
endImport(Trans::Transaction& cTrans_)
{
	delete m_pFileReflect, m_pFileReflect = 0;
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
