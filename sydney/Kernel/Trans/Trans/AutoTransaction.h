// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoTransaction.h -- オートトランザクション記述子関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_TRANS_AUTOTRANSACTION_H
#define	__SYDNEY_TRANS_AUTOTRANSACTION_H

#include "Trans/Module.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_TRANS_BEGIN

//	CLASS
//	Trans::AutoTransaction -- オートトランザクション記述子を表すクラス
//
//	NOTES

class AutoTransaction
	: public	ModAutoPointer<Transaction>
{
public:
	// コンストラクター
	AutoTransaction(Transaction* trans);
	// デストラクター
	~AutoTransaction();

	// トランザクション記述子を破棄する
	virtual void			free();
};

//	FUNCTION public
//	Trans::AutoTransaction::AutoTransaction --
//		オートトランザクション記述子を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction*	trans
//			オートトランザクション記述子が保持する
//			トランザクション記述子を格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
AutoTransaction::AutoTransaction(Transaction* trans)
	: ModAutoPointer<Transaction>(trans)
{}

//	FUNCTION public
//	Trans::AutoTransaction::~AutoTransaction --
//		オートトランザクション記述子を表すクラスのデストラクター
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
//		なし

inline
AutoTransaction::~AutoTransaction()
{
	free();
}

//	FUNCTION public
//	Trans::AutoTransaction::free -- 保持するトランザクション記述子を破棄する
//
//	NOTES
//		オートトランザクション記述子の破棄時などに呼び出される
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
void
AutoTransaction::free()
{
	if (isOwner())
		if (Transaction* trans = release()) {
			if (trans->getStatus() == Transaction::Status::InProgress)

				// 実行中のトランザクションはロールバックする

				trans->rollback();

			Transaction::detach(trans);
		}
}

_SYDNEY_TRANS_END
_SYDNEY_END

#endif	// __SYDNEY_TRANS_AUTOTRANSACTION_H

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
