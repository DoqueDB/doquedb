// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VariableIterator.h -- 可変長フィールドを格納するファイルにアクセスするためのイテレーター
// 
// Copyright (c) 2001, 2004, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_VARIABLEITERATOR_H
#define __SYDNEY_RECORD_VARIABLEITERATOR_H

#include "Common/Common.h"
#include "Common/DataArrayData.h"	// Common::DataArrayData::Pointer のため
#include "Common/Object.h"
#include "Record/Module.h"
#include "Record/VariableFile.h"

_SYDNEY_BEGIN

namespace Trans
{
class Transaction;
}

namespace Admin
{
	namespace Verification
	{
		class Progress;
	}
}

_SYDNEY_RECORD_BEGIN

class TargetFields;
class LinkedObject;
class VariableField;

//	CLASS
//	Record::VariableIterator --
//		VariableFile内を走査するイテレーターを表すクラス
//
//	NOTES

class VariableIterator : public Common::Object
{
	struct ObjectHeader;
	friend struct ObjectHeader;
public:
	// VariableFieldはVariableIteratorの中を自由に操作できる必要がある
	friend class VariableField;

	struct SearchPage
	{
		enum Value
		{
			Read = 0,				// 探すだけ
			Replace,				// 見つけたページをアタッチした状態にする
			Verify,					// 整合性検査用
			ValueNum
		};
	};

	struct Operation
	{
		enum Value
		{
			Read = 0,				// 読み込み
			Write,					// 書き込み
			Batch,					// バッチモードの書き込み
			ValueNum
		};
	};

	// コンストラクター
	VariableIterator(VariableFile& cFile_,
					 Operation::Value eOperation_);
	// デストラクター
	~VariableIterator();

#ifdef OBSOLETE
	// イテレーターが有効かを得る
	bool isValid() const;
#endif //OBSOLETE

	// イテレーターが指しているオブジェクトのオブジェクトIDを得る
	Tools::ObjectID getObjectID() const;

	// イテレーターを移動する
	bool seek(Tools::ObjectID iObjectID_);

	// オブジェクトを読み込む
	void read(DirectFile::DataPackage& cData_,
			  const TargetFields* pTarget_);

#ifdef RECORD_CHECK_NULL
	// オブジェクトがすべてNullであることを確認する
	void assureNull(DirectFile::DataPackage& cData_,
					const TargetFields* pTarget_);
#endif

	// オブジェクトを挿入する
	void insert(const DirectFile::DataPackage& cData_,
				Tools::ObjectID& iFreeID_,
				const TargetFields* pTarget_ = 0);

	// オブジェクトを更新する
	void update(const DirectFile::DataPackage& cOldObjectHeader_,
				const DirectFile::DataPackage& cNewData_,
				const TargetFields* pTarget_,
				Tools::ObjectID& iFreeID_);

	// オブジェクトを削除する
	void expunge(Tools::ObjectID iObjectID_,
				 Tools::ObjectID& iFreeID_);

	// オブジェクトを指す変数をクリアする
	void detachObject();

	//////////////////////////
	// 整合性検査用のメソッド
	//////////////////////////
	// イテレーターが指しているオブジェクトの整合性検査を行う
	void verifyData(DirectFile::DataPackage& cData_,
					Admin::Verification::Treatment::Value iTreatment_,
					Admin::Verification::Progress& cProgress_);

private:
	// オブジェクトヘッダー内で各フィールドに対応するサイズを表す型
	struct ObjectSize
	{
		// 第一要素 -- 付加情報
		// 配列: 要素数
		// 通常: 圧縮前のサイズ
		union {
			Tools::ElementNum m_iElementNumber;		// 配列の要素数
			Tools::FieldLength m_iUncompressedSize;	// 圧縮前のサイズ
		};
		// 第二要素 -- ファイル上で各フィールドが占めるサイズ
		// 配列(要素固定長): NULL bitmap + nullでない要素の値
		// 配列(要素可変長): NULL bitmap + nullでない要素の圧縮前/後のサイズ + 値
		// 通常: 値そのもの
		Tools::FieldLength m_iFieldSize;
	};

	// オブジェクトヘッダー
	struct ObjectHeader
	{
		typedef ModPair<Tools::FieldNum, ObjectSize> Element;
		
		// nullでないフィールドのファイル上の位置とフィールドのサイズ
		ModVector<Element>	m_vecData;

		void clear()
		{
			m_vecData.erase(m_vecData.begin(), m_vecData.end());
		}
	};

	//////////////
	// 内部関数 //
	//////////////
	// オブジェクトにアクセスするための変数を作る
	void initializeLinkedObject();

	// イテレーターを無効化する
	void invalidate();

	// オブジェクトのヘッダを読み書きする
	void readHeader(const DirectFile::DataPackage& cData_);
	void writeHeader(const DirectFile::DataPackage& cData_,
					 const TargetFields* pTarget_);
	// オブジェクトのヘッダを作る
	Os::Memory::Size makeHeader(const DirectFile::DataPackage& cData_,
								const TargetFields* pTarget_);

	// 可変長のサイズを読み書きする
	ObjectSize readObjectSize(Tools::FieldNum iFieldID_);
	void writeObjectSize(const ObjectSize& cSize_,
						 Tools::FieldNum iFieldID_);

	// 可変長のサイズを作る
	ObjectSize makeObjectSize(const Common::Data& cData_,
							  Tools::FieldNum iFieldID_);

	// 挿入が必要かを得る
	bool isNecessaryToInsert(const DirectFile::DataPackage& cData_);

	// 更新前のデータを必要なら読み込む
	void readOldData(const DirectFile::DataPackage& cOldObjectHeader_,
					 const DirectFile::DataPackage& cNewData_,
					 const TargetFields* pTarget_,
					 DirectFile::DataPackage& cData_,
					 TargetFields& cTarget_);

	////////////////////
	// データメンバー //
	////////////////////

	// イテレーターが走査するファイル
	VariableFile& m_cFile;

	// 操作の種類
	Operation::Value m_eOperation;

	// イテレーターが指しているオブジェクトID(リンクオブジェクトの先頭)
	Tools::ObjectID m_iObjectID;

	// リンクオブジェクトを操作するためのクラス
	LinkedObject* m_pLinkedObject;

	// 現在指しているオブジェクトのヘッダ情報
	ObjectHeader m_cObjectHeader;
};

_SYDNEY_RECORD_END
_SYDNEY_END

#endif // __SYDNEY_RECORD_VARIABLEITERATOR_H

//
//	Copyright (c) 2001, 2004, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
