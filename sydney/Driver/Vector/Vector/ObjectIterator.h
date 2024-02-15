// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectIterator.h -- オブジェクト反復子のヘッダファイル
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_OBJECTITERATOR_H
#define __SYDNEY_VECTOR_OBJECTITERATOR_H

#include "Vector/Module.h"

#include "Buffer/Page.h"
#include "PhysicalFile/Page.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
}

namespace PhysicalFile
{
class File;
}

namespace Vector
{
class FileParameter;
class OpenParameter;
class PageManager;
class FileInformation;
class Object;

//
//	CLASS
//	Vector::ObjectIterator -- オブジェクト反復子。
//
//	NOTES
//
// 反復子の特徴
// - 反復子は常にオブジェクトを指している。
// - ファイルにオブジェクトがひとつも存在しない場合、
// - 反復子が指しているオブジェクトを含め、どんなオブジェクトが
//   削除されても正常に動作する。
//	

class SYD_VECTOR_FUNCTION_TESTEXPORT ObjectIterator : public Common::Object
{
public:
	// コンストラクタはprotected

	// デストラクタ
	virtual ~ObjectIterator();

	// アクセサ

	// 以下の8つの関数は仮想とする(派生クラスで定義する)
	
	// オブジェクトをファイルから取得する
	virtual bool get(Common::DataArrayData* pTuple_) { return false; }
	// 検索するオブジェクトを指定する
    virtual void fetch(ModUInt32 ulVectorKey_) {}
	// オブジェクトをマークする
	virtual void mark() {}
	// オブジェクトの読み込みを巻き戻す
	virtual void rewind() {}
	// 反復子を初期状態に戻す
	virtual void reset() {}
	
	// マニピュレータ

	// オブジェクトの更新を行う
	virtual void update(ModUInt32 ulVectorKey_,
						const Common::DataArrayData& rObject_) {}
	// オブジェクトの挿入を行う
	virtual void insert(ModUInt32 ulVectorKey_,
						const Common::DataArrayData& rObject_) {}
	// オブジェクトの削除を行う
	virtual void expunge(ModUInt32 ulVectorKey_) {}

protected:
	// コンストラクタ
	ObjectIterator(
		FileParameter&		rFileParameter_,
		OpenParameter&		rOpenParameter_,
		PageManager&		rPageManager_);


	// 反復子が使用するファイルパラメータへの参照
	const FileParameter& m_rFileParameter;

	// 反復子が使用するオープンパラメータへの参照
	const OpenParameter& m_rOpenParameter;

	// 反復子が使用するページマネージャへの参照
	PageManager& m_rPageManager;
};

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_OBJECTITERATOR_H

//
//	Copyright (c) 2000, 2001, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
