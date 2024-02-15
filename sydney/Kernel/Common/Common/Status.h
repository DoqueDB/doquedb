// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Status.h -- 
// 
// Copyright (c) 1999, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_STATUS_H
#define __TRMEISTER_COMMON_STATUS_H

#include "Common/ExecutableObject.h"
#include "Common/Externalizable.h"
#include "ModUnicodeString.h"

_TRMEISTER_BEGIN

namespace Common {

//
//	CLASS
//	Status -- ステータスをあらわすクラス
//
//	NOTES
//	ポート等でやり取りするステータスをあらわすクラス
//
class SYD_COMMON_FUNCTION Status : public ExecutableObject,
							public Externalizable
{
public:
	//
	//	ENUM
	//	Type -- ステータスの種別をあらわす
	//
	//	NOTES
	//	ステータスの種別をあらわす
	//		Success		正常終了
	//		Error		エラー終了
	//		Canceled	中断終了
	//		HasMoreData	(複文で)続きのデータがある
	//		Undefined	不明
	//
	enum Type
	{
		Success = 0,		//正常
		Error = 1,			//エラー
		Canceled = 2,		//キャンセル
		HasMoreData = 3,	//続きのデータがある
		Undefined = -1		//不明
	};

	//コンストラクタ(1)
	Status();
	//コンストラクタ(2)
	Status(Type eStatus_);
	//デストラクタ
	~Status();

	//ステータスを得る
	Type getStatus() const;

	//エラーかどうか
	bool isError() const;

	//シリアル化
	void serialize(ModArchive& cArchiver_);

	//クラスIDを得る
	int getClassID() const;

	//文字列で得る
	ModUnicodeString toString() const;
	
private:
	//ステータス
	Type m_eStatus;
};

}

_TRMEISTER_END

#endif // __TRMEISTER_COMMON_STATUS_H

//
//	Copyright (c) 1999, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

