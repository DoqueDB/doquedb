// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FileVerify.h -- 整合性検査のためのクラス定義、関数宣言
// 
// Copyright (c) 2000, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FILEVERIFY_H
#define	__SYDNEY_SCHEMA_FILEVERIFY_H

#include "Schema/Module.h"
#include "Schema/AccessFile.h"
#include "Schema/TupleID.h"

#include "Admin/Verification.h"

#include "Common/Object.h"
#include "Common/DataArrayData.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common {
	class DataArrayData;
	class IntegerArrayData;
	class Parameter;
}
namespace Trans {
	class Transaction;
}
_SYDNEY_SCHEMA_BEGIN

class Column;
class File;
class Field;
namespace TreeNode {
	class Base;
	class Field;
	class Variable;
}

//	CLASS
//	Schema::FileVerify -- 整合性検査のための情報を表すクラス
//
//	NOTES

class FileVerify : public Common::Object
{
public:
	FileVerify(Table& cTable_, const ModVector<File*>& vecFile_);
	virtual ~FileVerify();

	void					verify(Admin::Verification::Progress& cProgress_,
								   Trans::Transaction& cTrans_,
								   Admin::Verification::Treatment::Value eTreatment_,
								   TupleID::Value* pMaxRowID_ = 0,
								   int* pMaxIdentity_ = 0,
								   int* pMinIdentity_ = 0);

protected:
private:
	void					verifyLogicalFile(Admin::Verification::Progress& cProgress_,
											  Trans::Transaction& cTrans_,
											  Admin::Verification::Treatment::Value eTreatment_,
											  const File* pStartFile_,
											  ModSize& iStartFilePosition_);
	void					verifyTuple(Admin::Verification::Progress& cProgress_,
										Trans::Transaction& cTrans_,
										Admin::Verification::Treatment::Value eTreatment_,
										const Common::DataArrayData* pTuple_,
										ModSize iStartFilePosition_);
	void					verifyCount(Admin::Verification::Progress& cProgress_,
										Admin::Verification::Treatment::Value eTreatment_,
										ModInt64 iExpectedCount_);
	void					verifyData(Admin::Verification::Progress& cProgress_,
									   Trans::Transaction& cTrans_,
									   Admin::Verification::Treatment::Value eTreatment_,
									   AccessFile& cAccess_,
									   File& cFile_,
									   const Common::DataArrayData& cData_,
									   bool bNoCheck_ = false);
	void					verifyIntegrity(Admin::Verification::Progress& cProgress_,
											Trans::Transaction& cTrans_,
											Admin::Verification::Treatment::Value eTreatment_,
											const File& cFile_,
											Field& cField_,
											const Column& cColumn_,
											const Common::Data& cData_);

	bool					isKeyAllFound(Trans::Transaction& cTrans_,
										  AccessFile& cAccess_,
										  File& cFile_,
										  Common::DataArrayData& cKey_,
										  const Common::Data::Pointer*& pValue_,
										  bool& bMayNotExist_, bool& bAllNull_);
	void					registerData(Admin::Verification::Progress& cProgress_,
										 Trans::Transaction& cTrans_,
										 Admin::Verification::Treatment::Value eTreatment_,
										 AccessFile& cAccess_,
									     File& cFile_,
									     const Common::DataArrayData& cData_);
	void					fillNullData(Trans::Transaction& cTrans_,
										 AccessFile& cAccess_,
										 File& cFile_);

	void					clearMap();
	const Common::Data::Pointer*
							getFieldData(Field* pField_) const;
	void					setFieldData(Trans::Transaction& cTrans_,
										 Field* pField_,
										 const Common::Data::Pointer& pData_,
										 bool bCheckSource_ = true);

	//--------------------------------------
	// ファイルの読み書きのためのクラス
	//--------------------------------------
	ModVector<AccessFile*>	m_vecAccess;

	//--------------------------------------
	// フィールドとデータの対応を記録するための型、変数
	//--------------------------------------
	typedef ModHashMap<Schema::Object::ID::Value,
					   Common::Data::Pointer,
					   ModHasher<Schema::Object::ID::Value> >
							FieldDataMap;

	FieldDataMap			m_mapFieldData;

	//--------------------------------------
	// ファイルのタプル数を記録する変数
	//--------------------------------------
	ModVector<ModInt64>		m_vecTupleCount;

	//--------------------------------------
	// コンストラクターで与えられる変数
	//--------------------------------------
	Table&					m_cTable;
	const ModVector<File*>&	m_vecFile;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_FILEVERIFY_H

//
// Copyright (c) 2000, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
