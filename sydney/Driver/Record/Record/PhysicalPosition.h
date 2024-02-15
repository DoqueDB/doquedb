// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// PhysicalPosition.h -- 位置クラスのヘッダーファイル
// 
// Copyright (c) 2000, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD_PHYSICALPOSITION_H
#define __SYDNEY_RECORD_PHYSICALPOSITION_H

#include "Common/Common.h"
#include "Common/Object.h"
#include "PhysicalFile/Types.h"

#include "Record/Tools.h"

_SYDNEY_BEGIN

namespace Record
{

//
//	CLASS
//	Record::PhysicalPosition -- 位置クラス
//
//	NOTES
//	物理ファイル上の任意の位置を表すクラス。
//	データメンバに物理ページIDとエリアIDを持っている。
//	オブジェクトIDを本クラスに代入することが可能(コンストラクタに渡すこと
//	も可能)で、本クラスからオブジェクトIDを作成することも可能。
//	
class PhysicalPosition
{
public:

	// コンストラクタ
#ifndef SYD_COVERAGE
	PhysicalPosition();
#endif //SYD_COVERAGE

	PhysicalPosition(const Tools::ObjectID	ObjectID_);

	PhysicalPosition(const PhysicalFile::PageID	PageID_,
					 const PhysicalFile::AreaID	AreaID_);

	// デストラクタ
	~PhysicalPosition();

	//
	// アクセッサ
	//

	// 物理ページIDを返す
	PhysicalFile::PageID getPageID() const;
#ifndef SYD_COVERAGE
	// エリアIDを返す
	PhysicalFile::AreaID getAreaID() const;
#endif //SYD_COVERAGE
	
	// 位置情報をオブジェクトIDに変換して返す
	Tools::ObjectID getObjectID() const;
	static Tools::ObjectID getObjectID(PhysicalFile::PageID PageID_,
									   PhysicalFile::AreaID AreaID_);

	// メモリーからオブジェクトIDに変換して返す
	static Tools::ObjectID readObjectID(const char* p);
	// オブジェクトIDをメモリーに書き込む
	static void writeObjectID(char* p, Tools::ObjectID id);

	//
	// マニピュレータ
	//

#ifndef SYD_COVERAGE
	// 物理ページIDを変更
	void setPageID(const PhysicalFile::PageID	PageID_);

	// エリアIDを変更
	void setAreaID(const PhysicalFile::AreaID	AreaID_);

	//
	// 比較演算子
	//

	bool operator==(const PhysicalPosition&	PhysicalPosition_) const;
	bool operator!=(const PhysicalPosition&	PhysicalPosition_) const;
	bool operator<(const PhysicalPosition&	PhysicalPosition_) const;
	bool operator<=(const PhysicalPosition&	PhysicalPosition_) const;
	bool operator>(const PhysicalPosition&	PhysicalPosition_) const;
	bool operator>=(const PhysicalPosition&	PhysicalPosition_) const;
#endif //SYD_COVERAGE

//private:

	PhysicalFile::PageID	m_PageID;		// 物理ページID
	PhysicalFile::AreaID	m_AreaID;		// エリアID

	// 未初期化状態では物理ページIDを表すデータメンバが Undefined に
	// なっていることを保証する。
	// このとき、エリアIDの値は不定(どんな値になるのか保証しない)
};

} // end of namespace Record

_SYDNEY_END

#endif // __SYDNEY_RECORD_PHYSICALPOSITION_H

//
//	Copyright (c) 2000, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
