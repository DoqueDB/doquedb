// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::TreeWalker -- ツリーをたどる
// 
// Copyright (c) 1999, 2000, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef SYD_COVERAGE

#ifndef __SYDNEY_STATEMENT_TREEWALKER_H
#define __SYDNEY_STATEMENT_TREEWALKER_H

#include "Common/Object.h"
#include "Statement/DLL.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class Object;

//
//	CLASS
//		TreeWalker -- ツリーをたどる
//
//	NOTES
//		ツリーをdepth-first・行きがけにたどる
//		メソッドの再帰呼び出しではなく、メンバとしてスタックを持つ
//		これにより、任意の位置で中断・再開し、またはリセットすることができる。
//
//		TreeWalker自体は抽象クラスである。
//		これを継承してサブクラスを作り、
//		findメソッドを実装することで利用できる。
//
class SYD_STATEMENT_FUNCTION TreeWalker : public Common::Object
{
public:
	// 次のアクション
	enum NextAction {
		Continue = 0,			// 次へ進む
		Prune,					// この部分木の残りをスキップ
		Stop					// 全体の処理を終了
	};
	// コンストラクタ
	TreeWalker(Statement::Object*& pRoot_);
	// デストラクタ
	virtual ~TreeWalker();

	// ツリーをたどりながらfindを呼ぶ
	void traverse();
	// 再開位置をリセットする
	void reset();

protected:
	// 目的とする部分木に対して仕事をする
	virtual NextAction find(Statement::Object*& pNode_) = 0;
	// トップからここまでに途中通ってきたノード
	ModVector<Statement::Object*> m_vecpStack;
	// 各ノードの何番目を処理中か
	ModVector<ModSize> m_vecuiIndex;

private:
	// コピーコンストラクタは使わない
	TreeWalker(const TreeWalker& cOther_);
	// 代入オペレーターは使わない
	TreeWalker& operator=(const TreeWalker& cOther_);

	// トップへのポインタ
	Statement::Object*& m_pRoot;
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_TREEWALKER_H

#endif

//
// Copyright (c) 1999, 2000, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
