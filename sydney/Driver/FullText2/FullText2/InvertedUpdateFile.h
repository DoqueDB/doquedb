// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedUpdateFile.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_INVERTEDUPDATEFILE_H
#define __SYDNEY_FULLTEXT2_INVERTEDUPDATEFILE_H

#include "FullText2/Module.h"
#include "FullText2/InvertedFile.h"
#include "FullText2/UpdateListManager.h"

#include "Os/Path.h"

class ModUnicodeString;
class ModInvertedCoder;

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedSection;

//
//	CLASS
//	FullText2::InvertedUpdaetFile
//		-- 更新操作をサポートする転置ファイルの
//		   基本的なインターフェースを規定するためのクラス
//
//	NOTES
//	このクラスは InvertedSection が保持する内部的なクラスである
//
class InvertedUpdateFile : public InvertedFile
{
public:
	// コンストラクタ
	InvertedUpdateFile(InvertedSection& cSection_,
					   const Os::Path& cPath_);
	// デストラクタ
	virtual ~InvertedUpdateFile();

	// ファイル内容をクリアする
	virtual void clear() = 0;

	// ページをsaveする
	virtual bool saveAllPages() = 0;

	// 大転置の文書IDに変換する
	virtual DocumentID convertToBigDocumentID(DocumentID uiDocumentID_,
											  int& iUnitNumber_)
		{
			iUnitNumber_ = -1;
			return uiDocumentID_;
		}
	
	// 小転置の文書IDを得る
	//
	//	削除用小転置にデータを入力するときには、
	//	呼び出し側で、本メソッドを実行し、小転置の文書IDを取得する必要がある
	//	ListManager::insert 内で内部的に変換できない理由は以下の通り
	//
	//	エラー処理で expunge を実行する必要があるが、
	//	大転置の文書IDから小転置の文書IDに変換するための
	//	ベクターは保持していないので、エラー処理が行えなくなる
	//	そのため呼び出し側で小転置の文書IDを保持する必要がある
	//
	virtual DocumentID assignDocumentID(DocumentID uiBigDocumentID_,
										int iUnitNumber_)
		{ return uiBigDocumentID_; }
	
	// ユニット番号を得る
	virtual int getUnitNumber() const { return -1; }

	// 大転置ID <-> 小転置IDのベクターから削除する
	virtual void expungeIDVector(DocumentID uiDocumentID_) {}
	
	// 圧縮器を得る
	ModInvertedCoder* getIdCoder(const ModUnicodeString& cstrKey_);
	ModInvertedCoder* getFrequencyCoder(const ModUnicodeString& cstrKey_);
	ModInvertedCoder* getLengthCoder(const ModUnicodeString& cstrKey_);
	ModInvertedCoder* getLocationCoder(const ModUnicodeString& cstrKey_);

	// 検索用リストマネージャーを得る
	ListManager* getListManager() { return getUpdateListManager(); }
	// 更新用リストマネージャーを得る
	virtual UpdateListManager* getUpdateListManager() = 0;

	// 削除対象のIDブロックを削除する
	virtual void expungeIdBlock() = 0;

	// 削除IDブロックのUndoログをクリアする
	virtual void clearDeleteIdBlockUndoLog() = 0;
	// 削除対象のIDブロック情報をクリアする
	virtual void clearDeleteIdBlockLog() = 0;

protected:
	// 転置ファイルセクション
	InvertedSection& m_cSection;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_INVERTEDUPDATEFILE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
