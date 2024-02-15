// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TreeWalker.cpp -- ツリーをたどる
// 
// Copyright (c) 1999, 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
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

namespace {
	const char moduleName[] = "Statement";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Common/Assert.h"
#include "Statement/Module.h"
#include "Statement/Object.h"
#include "Statement/TreeWalker.h"

#include "Exception/BadArgument.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::TreeWalker::TreeWalker -- コンストラクタ
//
//	NOTES
//		コンストラクタ
//
//	ARGUMENTS
//		Statement::Object*& pRoot_
//			探索したい木のルートへのポインタ変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	BadArgumentException
//		pRoot_がNULLポインタだった
//
TreeWalker::TreeWalker(Statement::Object*& pRoot_)
	: m_pRoot(pRoot_),
	  m_vecpStack(),
	  m_vecuiIndex()
{
	if (pRoot_ == 0) {
		throw Exception::BadArgument(moduleName, srcFile, __LINE__);
	}
}

//
//	FUNCTION public
//		Statement::TreeWalker::~TreeWalker -- デストラクタ
//
//	NOTES
//		デストラクタ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外をそのまま再送
//
TreeWalker::~TreeWalker()
{
}

//
//	FUNCTION public
//		Statement::TreeWalker::traverse -- ツリーをたどりながらfindを呼ぶ
//
//	NOTES
//		ツリーをたどりながらfindを呼ぶ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外をそのまま再送
//
void
TreeWalker::traverse()
{
	// 現在処理中の深さ
	ModSize uiDepth = m_vecpStack.getSize();
	NextAction eNext;

	// 最初は自分を渡す
	if (uiDepth == 0) {
		eNext = find(m_pRoot);
		// Pruneが来たら次回も終りになるように
		if (eNext == Prune) {
			// ダミーの親を作り、ラスト位置を記録
			// Index[d] == Stack[d].Sizeならその段は終り
			m_vecpStack.pushBack(m_pRoot);
			m_vecuiIndex.pushBack(m_pRoot->getElementVector()->getSize());
			return;
		}
		// 次の開始位置を設定
		m_vecpStack.pushBack(m_pRoot);
		m_vecuiIndex.pushBack(0);
		++uiDepth;
		// Stopだったら、次は子へ進む状態でリターン
		if (eNext == Stop)
			return;
	}

	while (uiDepth > 0) {
		// uiDepth段目のループ
		// Stack[uiDepth-1]が親で、そのIndex[uiDepth-1]番目の子を処理する
		Statement::Object* pParent = m_vecpStack[uiDepth-1];
		ModSize uiChild = m_vecuiIndex[uiDepth-1];
		// pParent->getElementVector()[uiChild]がfindに渡すべきノード(へのポインタ)
		if (pParent == 0 ||
			uiChild >= pParent->getElementVector()->getSize()) {
			// この段は終り
			--uiDepth;
			m_vecpStack.popBack();
			m_vecuiIndex.popBack();
			// 次の弟へ進む
			if (uiDepth > 0)
				++m_vecuiIndex[uiDepth-1];
			continue;
		}
		// findを呼ぶ
		ModVector<Statement::Object*>& pvecpChildren =
			*pParent->getElementVector();
		if(pvecpChildren[uiChild] == 0) {
			// ノードが入っていなければ、スキップ
			eNext = Prune;
		} else {
			// findはここでpvecpChildren[uiChild]を書き換えてもよい
			eNext = find(pvecpChildren[uiChild]);
		}
		// *pvecpChildren[uiChild]の子を処理する
		if (eNext != Prune) {
			m_vecpStack.pushBack(pvecpChildren[uiChild]);
			m_vecuiIndex.pushBack(0);
			++uiDepth;
			continue;
		}
		// *pvecpChildren[uiChild]には子がいない
		// 弟の処理へ進む
		++m_vecuiIndex[uiDepth-1];
		// Stopだったらここでリターン
		if (eNext == Stop)
			return;
	}
}

//
//	FUNCTION 
//		Statement::TreeWalker::reset -- 再開位置をリセットする
//
//	NOTES
//		再開位置をリセットする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外をそのまま再送
//
void
TreeWalker::reset()
{
	m_vecpStack.clear();
	m_vecuiIndex.clear();
}

#endif

//
// Copyright (c) 1999, 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
