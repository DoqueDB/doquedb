// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableField.h -- 可変長用フィールド反復子クラスのヘッダーファイル
// 
// Copyright (c) 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_VARIABLEFIELD_H
#define __SYDNEY_RECORD_VARIABLEFIELD_H

#include "Common/Common.h"
#include "Common/Object.h"

#include "Record/Module.h"
#include "Record/Tools.h"
#include "Record/VariableIterator.h"

_SYDNEY_BEGIN

namespace Common
{
class Data;
}

_SYDNEY_RECORD_BEGIN

class LinkedObject;
class MetaData;

//
//	CLASS
//	Record::VariableField -- 可変長用フィールド反復子のクラス
//
//	NOTES
//	オブジェクトのフィールドをフィールド番号の順に昇順で走査するためのクラス。
//
//	反復子がコンストラクトされてからデストラクトされるまでの間にフィールド
//	値が更新されたり、オブジェクトそのものが削除されることはないと仮定する。
//
//	呼出側で、削除や更新が起きないことを保証しなければいけない。
//
class VariableField : public Common::Object
{
public:
	// コンストラクタ / デストラクタ
	VariableField(const MetaData& cMetaData_,
				  const VariableIterator::ObjectHeader& cHeader_,
				  LinkedObject& cLinkedObject_);
	~VariableField();

	//
	// アクセッサ
	//
	typedef Tools::FieldNum	FieldID;

	// フィールドIDを取得
	FieldID getFieldID() const;

	// フィールド値を読み込む
	void readField(Common::Data& cData_) const;
	// フィールド値を読み込む
	Common::DataArrayData::Pointer readField() const;

	// フィールド値を更新する
	void updateField(const Common::DataArrayData::Pointer& pData_);

	// 任意の位置(フィールドID)に移動
	bool seek(FieldID	iFieldID_);

	// Nullでないフィールドの何番目かを得る
#ifdef OBSOLETE
	ModSize getPosition() const;
	ModSize getPosition(FieldID	iFieldID_) const;
#endif //OBSOLETE

	// Common::Dataをバッファーにダンプする
	static void dumpData(const Tools::DataType& cType_,
						 const Tools::DataType& cElementType_,
						 const Common::DataArrayData::Pointer& pData_,
						 char* pBuffer_);

private:
	// 読み込んだデータをCommon::Dataにする
	void createData(const VariableIterator::ObjectSize& cSize_,
					const Tools::DataType& cType_,
					const Tools::DataType& cElementType_,
					Common::Data& cData_,
					const char* pBuffer_) const;

	// メタデータには各フィールドのデータ型など、様々な情報が詰まっている
	const MetaData&					m_cMetaData;
	
	// オブジェクトのサイズなどの情報が入ったオブジェクト
	const VariableIterator::ObjectHeader&	m_cHeader;
	
	// フィールドデータを読み込むリンクオブジェクト
	LinkedObject&					m_cLinkedObject;

	// NULLでない可変長フィールドの何番目を見ているかを示す
	mutable ModSize					m_iPosition;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_VARIABLEFIELD_H

//
//	Copyright (c) 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
