// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModTree.cpp -- ModTree のメンバ定義
// 
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
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


#include "ModCommon.h"
#include "ModTree.h"

// --------------------
// ModFreeFunction の実装
// --------------------
//
// FUNCTION
// ModTreeFunctions::increment -- ノードを指すポインタを一つ進める
//
// NOTES
// この関数は ModTree 上でノードを一つ進めるために用いる。
//
// ARGUMENTS
// ModTreeNode*& node
//		進める対象となるノードを指すポインタ
// ModTreeNode* nilNode
//		木の terminater の役割を果たすノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		erase されたノードを引数にした

void
ModTreeFunctions::increment(
	ModTreeNode*& node, const ModTreeNode* nilNode)
{
	if (node->getRight() == node || node->getLeft() == node) {
		ModErrorMessage << "Illegal node was passed to increment." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
	if (node->getRight() != nilNode) {
		// 右部分木があるならその最左端が次のノード
		for (node = node->getRight();
			 node->getLeft() != nilNode; node = node->getLeft()) ;
	} else {
		// 右部分木がないなら自分が右枝にいる限り遡る
		ModTreeNode* parent = node->getParent();
		while (node == parent->getRight()) {
			node = parent;
			parent = parent->getParent();
		}
		if (node->getRight() != parent) {
			node = parent;
		}
	}
}

void
ModTreeFunctions::increment(
	const ModTreeNode*& node, const ModTreeNode* nilNode)
{
	if (node->getRight() == node || node->getLeft() == node) {
		ModErrorMessage << "Illegal node was passed to increment." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
	if (node->getRight() != nilNode) {
		// 右部分木があるならその最左端が次のノード
		for (node = node->getRight();
			 node->getLeft() != nilNode; node = node->getLeft()) ;
	} else {
		// 右部分木がないなら自分が右枝にいる限り遡る
		const ModTreeNode* parent = node->getParent();
		while (node == parent->getRight()) {
			node = parent;
			parent = parent->getParent();
		}
		if (node->getRight() != parent) {
			node = parent;
		}
	}
}

//
// FUNCTION
// ModTreeFunctions::decrement -- ノードを指すポインタを一つ戻す
//
// NOTES
// この関数は ModTree 上でノードを一つ戻すのに用いる。
//
// ARGUMENTS
// ModTreeNode*& node
//		戻す対象となるノードを指すポインタ
// ModTreeNode* nilNode
//		木の terminater の役割を果たすノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		erase されたノードを引数にした

void
ModTreeFunctions::decrement(ModTreeNode*& node, const ModTreeNode* nilNode)
{
	if (node->getRight() == node || node->getLeft() == node) {
		ModErrorMessage << "Illegal node was passed to decrement." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
	if (node->getColor() == ModTreeNode::red &&
		node->getParent()->getParent() == node) {
		node = node->getRight();
	} else if (node->getLeft() != nilNode) {
		for (node = node->getLeft();
			 node->getRight() != nilNode; node = node->getRight()) ;
	} else {
		ModTreeNode *parent = node->getParent();
		while (node == parent->getLeft()) {
			node = parent;
			parent = parent->getParent();
		}
		node = parent;
	}
}

void
ModTreeFunctions::decrement(
	const ModTreeNode*& node, const ModTreeNode* nilNode)
{
	if (node->getRight() == node || node->getLeft() == node) {
		ModErrorMessage << "Illegal node was passed to decrement." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}
	if (node->getColor() == ModTreeNode::red &&
		node->getParent()->getParent() == node) {
		node = node->getRight();
	} else if (node->getLeft() != nilNode) {
		for (node = node->getLeft();
			 node->getRight() != nilNode; node = node->getRight()) ;
	} else {
		const ModTreeNode *parent = node->getParent();
		while (node == parent->getLeft()) {
			node = parent;
			parent = parent->getParent();
		}
		node = parent;
	}
}

//
// FUNCTION
// ModTreeFunctions::rotateRight -- ノードの右回転
//
// NOTES
// この関数は insert の下請けとして木構造の一部を変えるのに用いる。
//
// ARGUMENTS
// ModTreeNode* node
//		構造を変えたい部分木のルートに位置するノードへのポインタ
// ModTreeNode*& root
//		root に位置するノードへのポインタ
// ModTreeNode* nilNode
//		木構造の terminater の役割を果たすノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModTreeFunctions::rotateRight(ModTreeNode* node, ModTreeNode*& root,
							  const ModTreeNode* nilNode)
{
	//
	//       PP                 PP
	//      /                  /
	//    node      ==>      node    tmp
	//    /  \               /  \    /
	//  tmp  ZZ             YY  ZZ  XX
	//  / \
	// XX  YY
	//
	ModTreeNode* temporaryNode = node->getLeft();
	node->setLeft(temporaryNode->getRight());

	if (temporaryNode->getRight() != nilNode) {
		temporaryNode->getRight()->setParent(node);
	}

	//
	//      PP                   PP
	//     /                    /
	//   node    tmp   ==>    tmp
	//   /  \    /            / \
    //  YY  ZZ  XX           XX node
	//                          /  \ 
	//                         YY  ZZ
	//
	temporaryNode->setParent(node->getParent());
	if (node == root) {
		root = temporaryNode;
	} else if (node == node->getParent()->getRight()) {
		node->getParent()->setRight(temporaryNode);
	} else {
		node->getParent()->setLeft(temporaryNode);
	}
	temporaryNode->setRight(node);
	node->setParent(temporaryNode);
}

//
// FUNCTION
// ModTreeFunctions::rotateLeft -- ノードの左回転
//
// NOTES
// この関数は insert の下請けとして木構造の一部を変えるのに用いる。
//
// ARGUMENTS
// ModTreeNode* node
//		構造を変えたい部分木のルートに位置するノードへのポインタ
// ModTreeNode*& root
//		root に位置するノードへのポインタ
// ModTreeNode* nilNode
//		木構造の terminater の役割を果たすノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
void
ModTreeFunctions::rotateLeft(ModTreeNode* node, ModTreeNode*& root,
							 const ModTreeNode* nilNode)
{
	//
	//       PP                 PP
	//      /                  /
	//    node      ==>      node    tmp
	//    /  \               /  \      \ 
	//   XX  tmp            XX  YY     ZZ
	//       / \
	//      YY  ZZ
	//
	ModTreeNode* temporaryNode = node->getRight();
	node->setRight(temporaryNode->getLeft());

	if (temporaryNode->getLeft() != nilNode) {
		temporaryNode->getLeft()->setParent(node);
	}

	//
	//      PP                   PP
	//     /                    /
	//   node    tmp   ==>    tmp
	//   /  \      \          / \
    //  XX  YY     ZZ      node  ZZ
	//                      /  \ 
	//                     XX  YY
	//
	temporaryNode->setParent(node->getParent());
	if (node == root) {
		root = temporaryNode;
	} else if (node == node->getParent()->getLeft()) {
		node->getParent()->setLeft(temporaryNode);
	} else {
		node->getParent()->setRight(temporaryNode);
	}
	temporaryNode->setLeft(node);
	node->setParent(temporaryNode);
}

//
// FUNCTION
// ModTreeFunction::insert -- ノードを挿入する
//
// NOTES
// この関数は ModTreeNode をノードとした木構造に新たにノードを挿入するために
// 用いる。
//
// ARGUMENTS
// ModBoolean leftFlag
//		左に伸びるように挿入するか否かを指定するフラグ。
// ModTreeNode* candidNode
//		挿入する場所の候補となるノードを指すポインタ
// ModTreeNode* parentNode
//		挿入する場所の親に位置するノードを指すポインタ
// ModTreeNode* insertNode
//		挿入するノードを指すポインタ
// ModTreeNode*& rightMost
//		最右端に位置するノードを指すポインタ
// ModTreeNode*& leftMost
//		最左端に位置するノードを指すポインタ
// ModTreeNode*& root
//		root に位置するノードを指すポインタ
// ModTreeNode* header
//		木構造の先頭を指すポインタ
// ModTreeNode* nilNode
//		木構造の terminater を表すノードへのポインタ
//
// RETURN
// なし
//
// EXCEPTIONS
// なし

void
ModTreeFunctions::insert(ModBoolean leftFlag, ModTreeNode* candidNode,
						 ModTreeNode* parentNode, ModTreeNode* insertNode,
						 ModTreeNode*& rightMost, ModTreeNode*& leftMost,
						 ModTreeNode*& root, ModTreeNode* header,
						 ModTreeNode* nilNode)
{
	if (leftFlag == ModTrue) {
		//
		// parent の左にぶら下げる
		//
		parentNode->setLeft(insertNode);
		if (parentNode == header) {
			// header に付いたなら root、rightMost のノードになる
			root = insertNode;
			rightMost = insertNode;
		} else if (parentNode == leftMost) {
			// leftMost の left に付いたなら leftMost のノードになる
			leftMost = insertNode;
		}
	} else {
		//
		// parent の右にぶら下げる
		//
		parentNode->setRight(insertNode);
		if (parentNode == rightMost) {
			// rightMost の right に付いたなら rightMost のノードになる
			rightMost = insertNode;
		}
	}

	//
	// insertNode に関連するリンクをつける
	//
	insertNode->setParent(parentNode);
	insertNode->setLeft(nilNode);
	insertNode->setRight(nilNode);

	//
	// 木の構造を２分木になるように調整する
	//
	candidNode = insertNode;
	candidNode->setColor(ModTreeNode::red);
	while (candidNode != root
		   && candidNode->getParent()->getColor() == ModTreeNode::red) {
		if (candidNode->getParent()
			== candidNode->getParent()->getParent()->getLeft()) {
			parentNode = candidNode->getParent()->getParent()->getRight();
			if (parentNode->getColor() == ModTreeNode::red) {
				candidNode->getParent()->setColor(ModTreeNode::black);
				parentNode->setColor(ModTreeNode::black);
				candidNode->getParent()->getParent()->setColor(ModTreeNode::red);
				candidNode = candidNode->getParent()->getParent();
			} else {
				if (candidNode == candidNode->getParent()->getRight()) {
					candidNode = candidNode->getParent();
					ModTreeFunctions::rotateLeft(candidNode, root, nilNode);
				}
				candidNode->getParent()->setColor(ModTreeNode::black);
				candidNode->getParent()->getParent()->setColor(ModTreeNode::red);
				ModTreeFunctions::rotateRight(candidNode->getParent()->getParent(),
											  root, nilNode);
			}
		} else {
			parentNode = candidNode->getParent()->getParent()->getLeft();
			if (parentNode->getColor() == ModTreeNode::red) {
				candidNode->getParent()->setColor(ModTreeNode::black);
				parentNode->setColor(ModTreeNode::black);
				candidNode->getParent()->getParent()->setColor(ModTreeNode::red);
				candidNode = candidNode->getParent()->getParent();
			} else {
				if (candidNode == candidNode->getParent()->getLeft()) {
					candidNode = candidNode->getParent();
					ModTreeFunctions::rotateRight(candidNode, root, nilNode);
				}
				candidNode->getParent()->setColor(ModTreeNode::black);
				candidNode->getParent()->getParent()->setColor(ModTreeNode::red);
				ModTreeFunctions::rotateLeft(candidNode->getParent()->getParent(),
											 root, nilNode);
			}
		}
	}
	root->setColor(ModTreeNode::black);
}

//
// FUNCTION
// ModTreeFunctions::erase -- ノードを消去する
//
// NOTES
// この関数は指定したノードを消去して木構造を作り直すために用いる。
//
// ARGUMENTS
// ModTreeNode* node
//		消去するノードを指すポインタ
// ModTreeNode*& rightMost
//		最右端に位置するノードを指すポインタ
// ModTreeNode*& leftMost
//		最左端に位置するノードを指すポインタ
// ModTreeNode*& root
//		root に位置するノードを指すポインタ
// ModTreeNode* header
//		木構造の先頭を指すポインタ
// ModTreeNode* nilNode
//		木構造の terminater を表すノードへのポインタ
//
// RETURN
// 消去したノードを指すポインタを返す。
//
// EXCEPTIONS
// ModCommonErrorBadArgument
//		erase されたノードを引数にした

ModTreeNode*
ModTreeFunctions::erase(ModTreeNode* node, ModTreeNode*& rightMost,
						ModTreeNode*& leftMost, ModTreeNode*& root,
						ModTreeNode* header, const ModTreeNode* nilNode)
{
	if (node->getRight() == node || node->getLeft() == node) {
		ModErrorMessage << "Illegal node was passed to erase." << ModEndl;
		ModThrow(ModModuleStandard, ModCommonErrorBadArgument,
				 ModErrorLevelError);
	}

	ModTreeNode* node2 = node;
	ModTreeNode* node3;

	if (node2->getLeft() == nilNode) {
		node3 = node2->getRight();
	} else if (node2->getRight() == nilNode) {
		node3 = node2->getLeft();
	} else {
		node2 = node2->getRight();
		while (node2->getLeft() != nilNode) {
			node2 = node2->getLeft();
		}
		node3 = node2->getRight();
	}
	if (node2 != node) {
		node->getLeft()->setParent(node2);
		node2->setLeft(node->getLeft());
		if (node2 != node->getRight()) {
			node3->setParent(node2->getParent());
			node2->getParent()->setLeft(node3);
			node2->setRight(node->getRight());
			node->getRight()->setParent(node2);
		} else {
			node3->setParent(node2);
		}

		if (root == node) {
			root = node2;
		} else if (node->getParent()->getLeft() == node) {
			node->getParent()->setLeft(node2);
		} else {
			node->getParent()->setRight(node2);
		}
		node2->setParent(node->getParent());
		ModTreeNode::ColorType temporaryColor = node2->getColor();
		node2->setColor(node->getColor());
		node->setColor(temporaryColor);
		node2 = node;
    } else {
		node3->setParent(node2->getParent());
		if (root == node) {
			root = node3;
		} else if (node->getParent()->getLeft() == node) {
			node->getParent()->setLeft(node3);
		} else {
			node->getParent()->setRight(node3);
		}

		if (leftMost == node)
			leftMost = (node->getRight() == nilNode) ?
				node->getParent() :
				ModTreeFunctions::minimum(node3, header, nilNode);

		if (rightMost == node)
			rightMost = (node->getLeft() == nilNode) ?
				node->getParent() :
				ModTreeFunctions::maximum(node3, header, nilNode);
    }

	if (node2->getColor() != ModTreeNode::red) {
		ModTreeNode* temporaryNode;
		while (node3 != root && node3->getColor() == ModTreeNode::black) {
			if (node3 == node3->getParent()->getLeft()) {
				temporaryNode = node3->getParent()->getRight();

				if (temporaryNode->getColor() == ModTreeNode::red) {
					temporaryNode->setColor(ModTreeNode::black);
					node3->getParent()->setColor(ModTreeNode::red);
					ModTreeFunctions::rotateLeft(node3->getParent(), root,
												 nilNode);

					temporaryNode = node3->getParent()->getRight();
				}

				if (temporaryNode->getLeft()->getColor() == ModTreeNode::black
					&& temporaryNode->getRight()->getColor() == ModTreeNode::black) {
					temporaryNode->setColor(ModTreeNode::red);
					node3 = node3->getParent();
				} else {
					if (temporaryNode->getRight()->getColor() ==
						ModTreeNode::black) {
						temporaryNode->getLeft()->setColor(ModTreeNode::black);
						temporaryNode->setColor(ModTreeNode::red);
						ModTreeFunctions::rotateRight(temporaryNode, root,
													  nilNode);
						temporaryNode = node3->getParent()->getRight();
					}

					temporaryNode->setColor(node3->getParent()->getColor());
					node3->getParent()->setColor(ModTreeNode::black);
					temporaryNode->getRight()->setColor(ModTreeNode::black);
					ModTreeFunctions::rotateLeft(node3->getParent(), root,
												 nilNode);
					break;
				}
			} else {
				temporaryNode = node3->getParent()->getLeft();

				if (temporaryNode->getColor() == ModTreeNode::red) {
					temporaryNode->setColor(ModTreeNode::black);
					node3->getParent()->setColor(ModTreeNode::red);
					ModTreeFunctions::rotateRight(node3->getParent(), root,
												  nilNode);
					temporaryNode = node3->getParent()->getLeft();
				}

				if (temporaryNode->getRight()->getColor() == ModTreeNode::black
					&& temporaryNode->getLeft()->getColor() == ModTreeNode::black) {
					temporaryNode->setColor(ModTreeNode::red);
					node3 = node3->getParent();
				} else {
					if (temporaryNode->getLeft()->getColor() ==
						ModTreeNode::black) {
						temporaryNode->getRight()->setColor(ModTreeNode::black);
						temporaryNode->setColor(ModTreeNode::red);
						ModTreeFunctions::rotateLeft(temporaryNode, root,
													 nilNode);
						temporaryNode = node3->getParent()->getLeft();
					}

					temporaryNode->setColor(node3->getParent()->getColor());
					node3->getParent()->setColor(ModTreeNode::black);
					temporaryNode->getLeft()->setColor(ModTreeNode::black);
					ModTreeFunctions::rotateRight(node3->getParent(), root,
												  nilNode);
					break;
				}
			}
		}
		node3->setColor(ModTreeNode::black);
	}
	// 消去されるノードはループを形成することで不正であることを表現
	node2->setParent(node2);
	node2->setRight(node2);
	node2->setLeft(node2);

	return node2;
}

//	FUNCTION public
//	ModTreeFunctions::minimum --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

ModTreeNode*
ModTreeFunctions::minimum(ModTreeNode* node, ModTreeNode* header,
						  const ModTreeNode* nilNode)
{
	if (node == nilNode)
		return header;

	for (; node->getLeft() != nilNode; node = node->getLeft()) ;
	return node;
}

//	FUNCTION public
//	ModTreeFunctions::maximum --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

ModTreeNode*
ModTreeFunctions::maximum(ModTreeNode* node, ModTreeNode* header,
						  const ModTreeNode* nilNode)
{
	if (node == nilNode)
		return header;

	for (; node->getRight() != nilNode; node = node->getRight()) ;
	return node;
}

//
// Copyright (c) 1997, 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
