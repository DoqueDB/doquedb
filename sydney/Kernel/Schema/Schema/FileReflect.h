// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileReflect.h -- 再構成で論理ログの内容を反映するためのクラス定義、関数宣言
// 
// Copyright (c) 2000, 2005, 2006, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FILEREFLECT_H
#define	__SYDNEY_SCHEMA_FILEREFLECT_H

#include "Schema/Module.h"
#include "Schema/AccessFile.h"

#include "Common/DataArrayData.h"
#include "Common/Object.h"
#include "Common/UnsignedIntegerData.h"

#include "Opt/LogData.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN

namespace Trans {
	class Transaction;
	namespace Log {
		class Data;
	}
}

_SYDNEY_SCHEMA_BEGIN
class File;

//	CLASS
//	Schema::FileReflect -- 再構成に論理ログの内容を反映するための情報を表すクラス
//
//	NOTES

class FileReflect : public Common::Object
{
public:
	FileReflect(Trans::Transaction& cTrans_, const File& cFile_);
	virtual ~FileReflect();
	
	bool					isUndoNeeded(const Trans::Log::Data& cLogData_);
	bool					isRedoNeeded(const Trans::Log::Data& cLogData_);

	void					undo();
	void					redo();

protected:
private:
	bool					analyzeLogData(const Trans::Log::Data& cLogData_);
	void					reset();

	const Common::UnsignedIntegerData*
							getTupleID() const;

	//--------------------------------------
	// ファイルの読み書きのためのクラス
	//--------------------------------------
	ModAutoPointer<AccessFile>	m_pAccess;

	//--------------------------------------
	// 反映中にログデータひとつごとに変化する情報
	//--------------------------------------
	Opt::LogData::Type::Value	m_eLogType;
	Common::DataArrayData		m_cPreKey;
	Common::DataArrayData		m_cPostKey;
	Common::UnsignedIntegerData	m_cTupleID;

	//--------------------------------------
	// 内部処理用の変数
	//--------------------------------------
	Trans::Transaction&			m_cTrans;
	const File&					m_cFile;
};

//	FUNCTION private
//	Schema::File::getTupleID -- タプルIDのデータを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		タプルIDのデータ
//
//	EXCEPTIONS
//		なし

inline
const Common::UnsignedIntegerData*
FileReflect::
getTupleID() const
{
	return &m_cTupleID;
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_FILEREFLECT_H

//
// Copyright (c) 2000, 2005, 2006, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
