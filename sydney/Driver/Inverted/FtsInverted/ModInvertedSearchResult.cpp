// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedSearchResult.cpp -- ランキング検索結果
// 
// Copyright (c) 1999, 2000, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModInvertedRankingScoreCombiner.h"
#include "ModInvertedSearchResult.h"


//
// FUNCTION public
// ModInvertedSearchResult::factory()
//
// NOTES
// ランキング検索結果オブジェクトを新しく生成する。
//
// ARGUMENTS
//	const ModUInt32 &resultType_
// RETURN
//	ModInvertedSearchResult* 
//
// EXCEPTIONS
// 下位からの例外をそのまま返す
//

ModInvertedSearchResult*
ModInvertedSearchResult::factory(const ModUInt32 resultType_)
{
	if (resultType_ == (1 << _SYDNEY::Inverted::FieldType::Rowid) ||
		resultType_ == 0)
	{
		return new ModInvertedBooleanResult;
	}
	else if (resultType_ == ((1 <<_SYDNEY::Inverted::FieldType::Score) |
							 (1 << _SYDNEY::Inverted::FieldType::Rowid)))
	{
		return new ModInvertedSearchResultScore;
	}
	else if (resultType_ == ((1 << _SYDNEY::Inverted::FieldType::Tf) |
							 (1 << _SYDNEY::Inverted::FieldType::Rowid)))
	{
		return new ModInvertedSearchResultTF;
	}
	else if (resultType_ == ((1 << _SYDNEY::Inverted::FieldType::Tf) |
							 (1 << _SYDNEY::Inverted::FieldType::Score) |
							 (1 << _SYDNEY::Inverted::FieldType::Rowid))) 
	{
		return new ModInvertedSearchResultScoreAndTF;
	}
	//
	// 以下はInternal
	// Internalは、内部処理で一時的に用いる検索結果型を示す。
	// たとえば外部に返す実行結果は以下のような構造を持つ。
	//
	// +-+---Doc1
	// | +-+---Score1
	// |   +---TFList1-+-+--TermNo1
	// |			   | +--TF1_1
	// |			   |
	// |			   +-+--TermNo2
	// |			   | +--TF1_2
	// |			   ...
	// |
	// +-+---Doc2
	// | +-+---Score2
	// |   +---TFList2-+-+--TermNo1
	// |			   | +--TF2_1
	// |			   |
	// |			   +-+--TermNo2
	// |			   | +--TF2_2
	// ...			   ...
	//
	// 一方、内部で処理中の検索結果は、たとえば以下のような構造を持つ。
	//
	// +-+---Doc1
	// | +-+---Score1
	// |   +---TF1_1
	// |
	// +-+---Doc2
	// | +-+---Score2
	// |   +---TF2_1
	// ...
	//
	// Internal指定により各検索語の情報を
	// 配列からスカラで保持するように切り替えている。
	// ちなみにInternalは、TermNoを保持しない。
	// 内部処理中は現在処理中の検索語が明らかなため。
	//
	else if (resultType_ == ((1 << _SYDNEY::Inverted::FieldType::Tf) |
							 (1 << _SYDNEY::Inverted::FieldType::Internal) |
							 (1 << _SYDNEY::Inverted::FieldType::Rowid)))
	{
		return new _ModInvertedSearchResultTF;
	}
	else if (resultType_ == ((1 << _SYDNEY::Inverted::FieldType::Tf) |
							 (1 << _SYDNEY::Inverted::FieldType::Score) |
							 (1 << _SYDNEY::Inverted::FieldType::Internal) |
							 (1 << _SYDNEY::Inverted::FieldType::Rowid)))
	{
		return new _ModInvertedSearchResultScoreAndTF;
	}
	else
	{
		// 例外発生
		ModUnexpectedThrow(ModModuleInvertedFile);
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setIntersection -- 積集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との積集合(z)を生成する
// x と y の両方に含まれるものだけを z に格納する。
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		積集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		積集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setIntersection(const ModInvertedSearchResult* y,
										 ModInvertedSearchResult* z)
{
	// [NOTE] x も y も文書IDの昇順に sort されている。
	// [NOTE] スコアは足し合わせる

	// [YET] termNo付きのsetIntersectionとの使い分けは？
	
	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 積集合の生成
	ModSize i = 0,j = 0;
	while (i != this->getSize() && j != y->getSize()) {
		
		// 文書ID毎に順番に処理
		
		// docid1 と docid2 が等しくない時には、小さい方だけを進める。
		// 等しい時には、その値を z に格納してから両方を進める。
		
		ModInvertedDocumentID docid1 = this->getDocID(i);
		ModInvertedDocumentID docid2 = y->getDocID(j);
		if (docid1 < docid2) {
			++i;
		} else if (docid1 > docid2) {
			++j;
		} else {		// docid == docid2
			z->pushBack(docid1,
						this->getScore(i) + y->getScore(j),
						this->_getTF(i));
			++i;
			++j;
		}
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setIntersection -- 積集合の生成
//
// NOTES
// TF演算用
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		積集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		積集合の格納先
// ModInvertedTermNo termNo
//		検索文に含まれる検索語の通し番号
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setIntersection(const ModInvertedSearchResult *y,
										 ModInvertedSearchResult *z,
										 ModInvertedTermNo termNo,
										 ModBoolean bFirst_)
{
	if(this->getType() & (1 << _SYDNEY::Inverted::FieldType::Tf))
	{
		// TF情報が含まれる場合

		// [NOTE] x(this)とzは非内部型だが、yは内部型

		if (z->getSize()) z->erase(0,z->getSize());

		ModSize i = 0,j = 0;
		while (i != this->getSize() && j != y->getSize()) {
			ModInvertedDocumentID docid1 = this->getDocID(i);
			ModInvertedDocumentID docid2 = y->getDocID(j);
			if (docid1 < docid2) {
				++i;
			} else if (docid1 > docid2) {
				++j;
			} else {		// docid == docid2
				
				TFList tf = *(this->getTF(i));
				tf.pushBack(TFListElement(termNo,y->_getTF(j)));
				z->pushBack(docid1,this->getScore(i) + y->getScore(j),tf);
				++i;
				++j;
			}
		}
		
		if(this->getSize() == 0 && bFirst_ == ModTrue)
		{
			// x が空だった場合
			
			// 一個目の検索条件の時はxが空なので、
			// yのTFをTFListに変換してzに格納する。
			
			while (j < y->getSize()) {
				TFList tf;
				// y->_getTF(j)は、docid2中のtermNoに対応する単語のtfを取得する
				tf.pushBack(TFListElement(termNo,y->_getTF(j)));
				z->pushBack(y->getDocID(j),y->getScore(j),tf);
				++j;
			}
		}
	}else
		// 従来の関数を実行する
		setIntersection(y,z);
}

//
// FUNCTION public
// ModInvertedSearchResult::setIntersection -- 積集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との積集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		積集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		積集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setIntersection(const ModInvertedSearchResult* y,
										 ModInvertedSearchResult* z,
										 ScoreCombiner& combiner)
{
	// [NOTE] スコアはcombiner.combine()を毎回呼ぶ(ので遅いと思われる)
	
	// [YET] TFを含み、かつ、スコア合成器を使いたい場合は？
	
	if (z->getSize()) z->erase(0,z->getSize());

	ModSize i = 0,j = 0;
	while (i != this->getSize() && j != y->getSize()) {
		ModInvertedDocumentID docid1 = this->getDocID(i);
		ModInvertedDocumentID docid2 = y->getDocID(j);
		if (docid1 < docid2) {
			++i;
		} else if (docid2 == docid1) {
			z->pushBack(docid1,combiner.combine(
							this->getScore(i),
							y->getScore(j)
							)
				);
			++i;
			++j;
		} else {
			++j;
		}
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setUnion -- 和集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との和集合(z)を生成する
// 自然文検索の高速版には、以下のsetUnion()を使用する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		和集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		和集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setUnion(const ModInvertedSearchResult* y,
								  ModInvertedSearchResult* z)
{
	// [NOTE] スコアは足し合わせる
	
	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 和集合の生成
	ModSize i = 0,j = 0;
	if(this->getType() & (1 <<_SYDNEY::Inverted::FieldType::Internal)){
		
		// x(this)とyが内部型の場合
		
		while (i != getSize() && j != y->getSize())	{
			// docid1 と docid2 が等しくない時には、
			// 小さい方だけを z に格納し、進める。
			// 等しい時には、その値を z に格納してから両方を進める。
			ModInvertedDocumentID docid1 = getDocID(i);
			ModInvertedDocumentID docid2 = y->getDocID(j);
			if (docid1 < docid2) {
				z->pushBack(docid1,this->getScore(i),this->_getTF(i));
				++i;
			} else if (docid1 > docid2) {
				z->pushBack(docid2,y->getScore(j),y->_getTF(j));
				++j;
			} else {		// docid1 == docid2
				z->pushBack(docid1,this->getScore(i) + y->getScore(j),this->_getTF(i));
				++i;
				++j;
			}
		}

		// x に残りがあれば、それを z に格納する。
		while (i < this->getSize()) {
			z->pushBack(this->getDocID(i),this->getScore(i),this->_getTF(i));
			++i;
		}
		// y に残りがあれば、それを z に格納する。
		while (j < y->getSize()) {
			z->pushBack(y->getDocID(j),y->getScore(j),y->_getTF(j));
			++j;
		}
	}else{

		// x(this)とyが内部型ではない場合
		
		while (i != getSize() && j != y->getSize())	{
			// docid1 と docid2 が等しくない時には、
			// 小さい方だけを z に格納し、進める。
			// 等しい時には、その値を z に格納してから両方を進める。
			ModInvertedDocumentID docid1 = getDocID(i);
			ModInvertedDocumentID docid2 = y->getDocID(j);
			if (docid1 < docid2) {
				z->pushBack(docid1,this->getScore(i),*this->getTF(i));
				++i;
			} else if (docid1 > docid2) {
				z->pushBack(docid2,y->getScore(j),*y->getTF(j));
				++j;
			} else {		// docid1 == docid2
				z->pushBack(docid1,this->getScore(i) + y->getScore(j),*this->getTF(i));
				++i;
				++j;
			}
		}

		// x に残りがあれば、それを z に格納する。
		while (i < this->getSize()) {
			z->pushBack(this->getDocID(i),this->getScore(i),*this->getTF(i));
			++i;
		}
		// y に残りがあれば、それを z に格納する。
		while (j < y->getSize()) {
			z->pushBack(y->getDocID(j),y->getScore(j),*y->getTF(j));
			++j;
		}
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setUnion -- 和集合の生成
//
// NOTES
// TF演算用
// 自身(x)と引数の集合(y)との和集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		和集合を生成するもう一方の検索結果
//		TFを含む場合、内部型。
// ModInvertedSearchResult* z
//		和集合の格納先
//		TFを含む場合、リスト型。
// ModInvertedTermNo termNo
//		検索文に含まれる検索語の通し番号
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setUnion(const ModInvertedSearchResult* y,
								  ModInvertedSearchResult* z,
								  ModInvertedTermNo termNo)
{
	// [NOTE] スコアは足し合わせる
	
	if(this->getType() & (1 <<_SYDNEY::Inverted::FieldType::Tf)){

		// TF情報が含まれている場合

		// thisとzは、１つのdocid毎のTF格納領域は、vector ModPair<ModUInt32,ModUInt32>になるが、
		// yに関しては、１つのdocid毎のTF格納領域は、ModUInt32になる。
		// yのTFは、termNoに該当する検索語のTFが格納されている(はず)。
		// thisとzのvector要素であるModPairのfirstには、単語noが入る

		// 格納先の初期化
		if (z->getSize()) z->erase(0,z->getSize());

		// 和集合の生成
		ModSize i = 0,j = 0;
		while (i != this->getSize() && j != y->getSize()) {
			// *f1 と *f2 が等しくない時には、小さい方だけを z に格納し、進める。
			// 等しい時には、その値を z に格納してから両方を進める。
			ModInvertedDocumentID docid1 = this->getDocID(i);
			ModInvertedDocumentID docid2 = y->getDocID(j);
			if (docid1 < docid2) {
				z->pushBack(docid1,this->getScore(i),*this->getTF(i));
				++i;
			} else if (docid1 > docid2) {
				TFList tf;
				// y->_getTF(j)は、docid2中のtermNoに対応する単語のtfを取得する
				tf.pushBack(TFListElement(termNo,y->_getTF(j)));
				z->pushBack(docid2,y->getScore(j),tf);
				++j;
			} else {		// *f1 == *f2
				TFList tf = *(this->getTF(i));
				// y->_getTF(j)は、docid2中のtermNoに対応する単語のtfを取得する
				// [YET] 同じtermNoを持つ要素が複数存在することになるかも？
				tf.pushBack(TFListElement(termNo,y->_getTF(j)));
				z->pushBack(docid1,this->getScore(i) + y->getScore(j),tf);
				++i;
				++j;
			}
		}
		// x に残りがあれば、それを z に格納する。
		while (i < this->getSize()) {
			z->pushBack(this->getDocID(i),this->getScore(i),*this->getTF(i));
			++i;
		}
		// y に残りがあれば、それを z に格納する。
		while (j < y->getSize()) {
			TFList tf;
				// y->_getTF(j)は、docid2中のtermNoに対応する単語のtfを取得する
			tf.pushBack(TFListElement(termNo,y->_getTF(j)));
			z->pushBack(y->getDocID(j),y->getScore(j),tf);
			++j;
		}
	}else

		// TF情報が含まれていない場合
		
		// 従来の関数を実行する
		setUnion(y,z);
}

//
// FUNCTION public
// ModInvertedSearchResult::setUnion -- 和集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との和集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		和集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		和集合の格納先
// ScoreCombiner& combiner
//		スコア合成器
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setUnion(const ModInvertedSearchResult* y,
								  ModInvertedSearchResult* z,
								  ScoreCombiner& combiner)
{
	// [NOTE] スコアはcombiner.combine()を毎回呼ぶ(ので遅いと思われる)
	// [YET] TFを含み、かつ、スコア合成器を使いたい場合は？
	
	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 和集合の生成
	ModSize i = 0,j = 0;
	while (i != this->getSize() && j != y->getSize()) {
		ModInvertedDocumentID docid1 = this->getDocID(i);
		ModInvertedDocumentID docid2 = y->getDocID(j);

		if (docid2 < docid1) {
			z->pushBack(docid2,combiner.combine(y->getScore(j),0));
			++j;
		} else if (docid2 == docid1) {
			z->pushBack(docid1,combiner.combine(this->getScore(i),
												 y->getScore(j)));
			++i;
			++j;
		} else {
			z->pushBack(docid1,combiner.combine(this->getScore(i),0));
			++i;
		}
	}

	// どちらか残っているものをコピーする
	while (i < this->getSize()) {
		z->pushBack(this->getDocID(i),combiner.combine(this->getScore(i),0));
		++i;
	}
	while (j < y->getSize()) {
		z->pushBack(y->getDocID(j),combiner.combine(y->getScore(j),0));
		++j;
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setUnion -- 和集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との和集合(tmp)を生成し、xをtmpで置き換える。
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		和集合を生成するもう一方の検索結果
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setUnion(const ModInvertedSearchResult* y)
{
	// どちらの結果集合も昇順にソートされていることが前提

	// 和集合の生成
	ModInvertedSearchResult *tmp = this->create();
	tmp->reserve(this->getSize() + y->getSize());
	this->setUnion(y,tmp);
	// 置き換え
	this->copy(tmp);
    delete tmp;
}

//
// FUNCTION public
// ModInvertedSearchResult::setUnion -- 和集合の生成
//
// NOTES
// TF演算用
// 自身(x)と引数の集合(y)との和集合(tmp)を生成し、xをtmpで置き換える。
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		和集合を生成するもう一方の検索結果
// ModInvertedTermNo termNo
//		検索文に含まれる検索語の通し番号
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setUnion(const ModInvertedSearchResult* y,
								  ModInvertedTermNo termNo)
{
	// どちらの結果集合も昇順にソートされていることが前提

	// 和集合の生成
	ModInvertedSearchResult *tmp = this->create();
	tmp->reserve(this->getSize() + y->getSize());
	this->setUnion(y,tmp,termNo);
	//置き換え
	this->copy(tmp);
    delete tmp;
}

//
// FUNCTION public
// ModInvertedSearchResult::setDifference -- 差集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との差集合(z)を生成する
// x だけに含まれるものだけを z に格納する。
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		差集合を生成する差し引く方の検索結果
// ModInvertedSearchResult* z
//		差集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setDifference(const ModInvertedSearchResult* y,
									   ModInvertedSearchResult* z)
{
	// [NOTE] x も y も昇順に sort されている。
	// [NOTE] x がTFを含む場合、x は内部型である。
	
	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 差集合の生成
	ModSize i = 0,j = 0;
	while (i != this->getSize() && j != y->getSize()) {
		ModInvertedDocumentID docid1 = this->getDocID(i);
		ModInvertedDocumentID docid2 = y->getDocID(j);
		if (docid1 < docid2) {
			z->pushBack(docid1,this->getScore(i),this->_getTF(i));
			++i;
		} else if (docid1 > docid2) {
			++j;
		} else {		// docid == docid2
			++i;
			++j;
		}
	}
	// x に残りがあれば、それを z に格納する。
	while (i < this->getSize()) {
		z->pushBack(this->getDocID(i),this->getScore(i),this->_getTF(i));
		++i;
	}
}

//
// FUNCTION public
// ModInvertedSearchResultScore::setDifference --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResultScore::setDifference(const ModInvertedSearchResult* _y,
											ModInvertedSearchResult* _z)
{
	ModInvertedSearchResultScore *y = (ModInvertedSearchResultScore *)_y;
	ModInvertedSearchResultScore *z = (ModInvertedSearchResultScore *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		const ModInvertedDocumentID docid1 = (*first1).first;
		const ModInvertedDocumentID docid2 = (*first2).first;
		if (docid1 < docid2) {
			z->pushBack(first1);
			++first1;
		} else if (docid1 > docid2) {
			++first2;
		} else {		// first == first2
			++first1;
			++first2;
		}
	}
	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(first1);
		++first1;
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setDifference -- 差集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との差集合(tmp)を生成し、自身と置き換える
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		差集合を生成する差し引く方の検索結果
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setDifference(const ModInvertedSearchResult* y)
{
	// どちらの結果集合も昇順にソートされていることが前提

	// 差集合の生成
	ModInvertedSearchResult *tmp = this->create();
	tmp->reserve(this->getSize() + y->getSize());
	this->setDifference(y,tmp);
	// 置き換え
	this->copy(tmp);
	delete tmp;
}

//
// FUNCTION public
// ModInvertedSearchResult::merge -- マージの実行
//
// NOTES
// 自身(x)に引数の集合(y)をマージした集合(z)を生成する
// 和集合と異なり、yにしか存在しない要素は、zには含まれない。
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		マージ集合を生成する追加する方の検索結果
// ModInvertedSearchResult* z
//		マージ集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::merge(const ModInvertedSearchResult* y,
							   ModInvertedSearchResult* z)
{
	// [NOTE] x も y も文書IDの昇順に sort されている。
	// [NOTE] スコアは足し合わせる
	
	// [YET] termNo付きのmergeとの使い分けは？
	
	// 格納先を初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// マージ
	ModSize i = 0,j = 0;
	while (i != this->getSize() && j != y->getSize()) {
		
		// 文書ID毎に順番に処理
		
		ModInvertedDocumentID docid1 = this->getDocID(i);
		ModInvertedDocumentID docid2 = y->getDocID(j);
		if (docid1 < docid2) {
			z->pushBack(docid1,this->getScore(i),this->_getTF(i));
			++i;
		} else if (docid1 > docid2) {
			++j;
		} else {		// docid == docid2
			z->pushBack(docid1,this->getScore(i) + y->getScore(j),this->_getTF(i));
			++i;
			++j;
		}
	}
	// x に残りがあれば、それを z に格納する。
	while (i < this->getSize()) {
		z->pushBack(this->getDocID(i),this->getScore(i),this->_getTF(i));
		++i;
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::merge -- マージの実行
//
// NOTES
// 自身(x)に引数の集合(y)をマージした集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		マージ集合を生成する追加する方の検索結果
// ModInvertedSearchResult* z
//		マージ集合の格納先
// ModInvertedTermNo termNo
//		検索文に含まれる検索語の通し番号
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::merge(const ModInvertedSearchResult *y,
							   ModInvertedSearchResult *z,
							   ModInvertedTermNo termNo)
{
	if(this->getType() & (1 << _SYDNEY::Inverted::FieldType::Tf))
	{
		// [NOTE] x(this)とzは非内部型だが、yは内部型

		if (z->getSize()) z->erase(0,z->getSize());

		ModSize i = 0,j = 0;
		while (i != this->getSize() && j != y->getSize()) {
			ModInvertedDocumentID docid1 = this->getDocID(i);
			ModInvertedDocumentID docid2 = y->getDocID(j);
			if (docid1 < docid2) {
				z->pushBack(docid1,this->getScore(i),*this->getTF(i));
				++i;
			} else if (docid1 > docid2) {
				++j;
			} else {		// docid == docid2
				TFList tf = *(this->getTF(i));
				// [YET] 同じtermNoを持つ要素が複数存在することになるかも？
				tf.pushBack(TFListElement(termNo,y->_getTF(j)));
				z->pushBack(docid1,this->getScore(i) + y->getScore(j),tf);
				++i;
				++j;
			}
		}
		
		if(this->getSize() == 0)
		{
			// x が空の場合
			
			// 特別なケース。TFをTFListに変換する
			// [YET] マージではなくなってしまう。どういうときこうなるのか？
			
			while (j < y->getSize()) {
				TFList tf;
				// y->_getTF(j)は、docid2中のtermNoに対応する単語のtfを取得する
				tf.pushBack(TFListElement(termNo,y->_getTF(j)));
				z->pushBack(y->getDocID(j),y->getScore(j),tf);
				++j;
			}
		}
		else
		{	
			// x が空ではない場合
			
			// this に残りがあれば、それを z に格納する。
			while (i < this->getSize()) {
				z->pushBack(this->getDocID(i),this->getScore(i),*this->getTF(i));
				++i;
			}
		}

	}else
		// 従来の関数を実行する
		merge(y,z);
}

//
// FUNCTION public
// ModInvertedBooleanResult::setIntersection -- 積集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との積集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		積集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		積集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedBooleanResult::setIntersection(const ModInvertedSearchResult* _y,
										  ModInvertedSearchResult* _z)
{
	ModInvertedBooleanResult *y = (ModInvertedBooleanResult *)_y;
	ModInvertedBooleanResult *z = (ModInvertedBooleanResult *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			++first1;
		} else if (*first2 == *first1) {
			z->pushBack(*first1);
			++first1;
			++first2;
		} else {
			++first2;
		}
	}
}

//
// FUNCTION public
// ModInvertedBooleanResult::setUnion -- 和集合の生成
//
// NOTES
// BooleanResult専用の集合演算メンバー関数（高速化）
// 自身(x)と引数の集合(y)との和集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* _y
//		和集合を生成するもう一方の検索結果
// ModInvertedSearchResult* _z
//		和集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedBooleanResult::setUnion(const ModInvertedSearchResult* _y,
								   ModInvertedSearchResult* _z)
{
	ModInvertedBooleanResult *y = (ModInvertedBooleanResult *)_y;
	ModInvertedBooleanResult *z = (ModInvertedBooleanResult *)_z;

	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 和集合の生成
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
   	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			z->pushBack(*first1);
			++first1;
		} else if (*first1 > *first2) {
			z->pushBack(*first2);
			++first2;
		} else {
      		z->pushBack(*first1);
			++first1;
			++first2;
		}
	}

	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(*first1);
		++first1;
	}
	// y に残りがあれば、それを z に格納する。
	while (first2 != last2) {
		z->pushBack(*first2);
		++first2;
	}
}

//
// FUNCTION public
// ModInvertedBooleanResult::setDifference --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedBooleanResult::setDifference(const ModInvertedSearchResult* _y,
										ModInvertedSearchResult* _z)
{
	ModInvertedBooleanResult *y = (ModInvertedBooleanResult *)_y;
	ModInvertedBooleanResult *z = (ModInvertedBooleanResult *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			z->pushBack(*first1);
			++first1;
		} else if (*first1 > *first2) {
			++first2;
		} else {		// first == first2
			++first1;
			++first2;
		}
	}
	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(*first1);
		++first1;
	}
}

//
// FUNCTION public
// ModInvertedBooleanResult::merge --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedBooleanResult::merge(const ModInvertedSearchResult* _y,
								ModInvertedSearchResult* _z)
{
	ModInvertedBooleanResult *y = (ModInvertedBooleanResult *)_y;
	ModInvertedBooleanResult *z = (ModInvertedBooleanResult *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2) {
			z->pushBack(*first1);
			++first1;
		} else if (*first1 > *first2) {
			++first2;
		} else {		// first == first2
			z->pushBack(*first1);
			++first1;
			++first2;
		}
	}
	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(*first1);
		++first1;
	}
}

//
// FUNCTION public
// ModInvertedSearchResultScore::ModInvertedSearchResultScore
//	-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModInvertedSearchResultScore::ModInvertedSearchResultScore(
	const ModSize size,
	const Element& defaultValue)
	:ModInvertedVector<Element>(size,defaultValue)
{
	resultType = (1 << _SYDNEY::Inverted::FieldType::Rowid) |
				(1 << _SYDNEY::Inverted::FieldType::Score);
}

//
// FUNCTION public
// ModInvertedSearchResultScore::ModInvertedSearchResultScore
//	-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModInvertedSearchResultScore::ModInvertedSearchResultScore(
	const ModInvertedVector<Element>& ids_)
	: ModInvertedVector<Element>(ids_)
{
	resultType = (1 << _SYDNEY::Inverted::FieldType::Rowid) |
				(1 << _SYDNEY::Inverted::FieldType::Score);	
}

//
// FUNCTION public
// ModInvertedSearchResultScore::setIntersection -- 積集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との積集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		積集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		積集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResultScore::setIntersection(const ModInvertedSearchResult* _y,
											  ModInvertedSearchResult* _z)
{
	ModInvertedSearchResultScore *y = (ModInvertedSearchResultScore *)_y;
	ModInvertedSearchResultScore *z = (ModInvertedSearchResultScore *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		const ModInvertedDocumentID docid1 = (*first1).first;
		const ModInvertedDocumentID docid2 = (*first2).first;
		if (docid1 < docid2) {
			++first1;
		} else if (docid2 == docid1) {
      		z->pushBack(Element(docid1,(*first1).second + (*first2).second));
			++first1;
			++first2;
		} else {
			++first2;
		}
	}
}

//
// FUNCTION public
// ModInvertedSearchResultScore::setIntersection -- 積集合の生成
//
// NOTES
// 自身(x)と引数の集合(y)との積集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		積集合を生成するもう一方の検索結果
// ModInvertedSearchResult* z
//		積集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResultScore::setIntersection(
			const ModInvertedSearchResult* _y,
			ModInvertedSearchResult* _z,
			ScoreCombiner& combiner)
{
	ModInvertedSearchResultScore *y = (ModInvertedSearchResultScore *)_y;
	ModInvertedSearchResultScore *z = (ModInvertedSearchResultScore *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		const ModInvertedDocumentID docid1 = (*first1).first;
		const ModInvertedDocumentID docid2 = (*first2).first;
		if (docid1 < docid2) {
			++first1;
		} else if (docid2 == docid1) {
      		z->pushBack(Element(docid1,combiner.combine((*first1).second ,(*first2).second)));
			++first1;
			++first2;
		} else {
			++first2;
		}
	}
}

//
// FUNCTION public
// ModInvertedSearchResultScore::setUnion -- 和集合の生成
//
// NOTES
// SearchResultScore専用の集合演算メンバー関数（高速化）
// 自身(x)と引数の集合(y)との和集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* _y
//		和集合を生成するもう一方の検索結果
// ModInvertedSearchResult* _z
//		和集合の格納先
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResultScore::setUnion(const ModInvertedSearchResult* _y,
									   ModInvertedSearchResult* _z)
{
	// [NOTE] スコアは足し合わせる
	
	ModInvertedSearchResultScore *y = (ModInvertedSearchResultScore *)_y;
	ModInvertedSearchResultScore *z = (ModInvertedSearchResultScore *)_z;

	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 和集合の生成
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
   	while (first1 != last1 && first2 != last2) {
		// docid1 と docid2 が等しくない時には、
		// 小さい方だけを z に格納し、進める。
		// 等しい時には、その値を z に格納してから両方を進める。
		const ModInvertedDocumentID docid1 = (*first1).first;
		const ModInvertedDocumentID docid2 = (*first2).first;
         
		if (docid1 < docid2) {
			z->pushBack(first1);
			++first1;
		} else if (docid1 > docid2) {
			z->pushBack(first2);
			++first2;
		} else {		// docid1 == docid2
      		z->pushBack(Element(docid1,(*first1).second + (*first2).second));
			++first1;
			++first2;
		}
	}

	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(first1);
		++first1;
	}
	// y に残りがあれば、それを z に格納する。
	while (first2 != last2) {
		z->pushBack(first2);
		++first2;
	}
}

//
// FUNCTION public
// ModInvertedSearchResultScore::setUnion -- 和集合の生成
//
// NOTES
// SearchResultScore専用の集合演算メンバー関数（高速化）
// 自身(x)と引数の集合(y)との和集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* _y
//		和集合を生成するもう一方の検索結果
// ModInvertedSearchResult* _z
//		和集合の格納先
// ScoreCombiner& combiner
//		スコア合成器
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResultScore::setUnion(const ModInvertedSearchResult* _y,
									   ModInvertedSearchResult* _z,
									   ScoreCombiner& combiner)
{
	// [NOTE] スコアはcombiner.combine()を毎回呼ぶ(ので遅いと思われる)
	
	ModInvertedSearchResultScore *y = (ModInvertedSearchResultScore *)_y;
	ModInvertedSearchResultScore *z = (ModInvertedSearchResultScore *)_z;

	// 格納先の初期化
	if (z->getSize()) z->erase(0,z->getSize());

	// 和集合の生成
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
   	while (first1 != last1 && first2 != last2) {
		// docid1 と docid2 が等しくない時には、小さい方だけを z に格納し、進める。
		// 等しい時には、その値を z に格納してから両方を進める。
		const ModInvertedDocumentID docid1 = (*first1).first;
		const ModInvertedDocumentID docid2 = (*first2).first;
         
		if (docid1 < docid2) {
			z->pushBack(Element(docid1,combiner.combine((*first1).second,0)));
			++first1;
		} else if (docid1 > docid2) {
			z->pushBack(Element(docid2,combiner.combine((*first2).second,0)));
			++first2;
		} else {		// docid1 == docid2
      		z->pushBack(Element(docid1,combiner.combine((*first1).second , (*first2).second)));
			++first1;
			++first2;
		}
	}

	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(Element((*first1).first,combiner.combine((*first1).second,0)));
		++first1;
	}
	// y に残りがあれば、それを z に格納する。
	while (first2 != last2) {
		z->pushBack(Element((*first2).first,combiner.combine((*first2).second,0)));
		++first2;
	}
}

//
// FUNCTION public
// ModInvertedSearchResult::setDifference -- 差集合の生成
//
// NOTES
// TF演算用
// 自身(x)と引数の集合(y)との差集合(z)を生成する
//
// ARGUMENTS
// const ModInvertedSearchResult* y
//		差集合を生成する差し引く方の検索結果
// ModInvertedSearchResult* z
//		差集合の格納先
// ModInvertedTermNo termNo
//		検索文に含まれる検索語の通し番号
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResult::setDifference(const ModInvertedSearchResult *y,
									   ModInvertedSearchResult *z,
									   ModInvertedTermNo termNo)
{
	// [YET] 使われていない。
	//  termNo無し版は、SearchCapsule::getDocumentFrequency()から呼ばれている。
	
	if(this->getType() & (1 <<_SYDNEY::Inverted::FieldType::Tf)){

		// TF情報が含まれる場合

		// 格納先の初期化
		if (z->getSize()) z->erase(0,z->getSize());
		
		ModSize i = 0,j = 0;
		while (i != this->getSize() && j != y->getSize()) {
			// docid1 と docid2 が等しくない時には、小さい方だけを進める。
			// 等しい時には、その値を z に格納してから両方を進める。
			ModInvertedDocumentID docid1 = this->getDocID(i);
			ModInvertedDocumentID docid2 = y->getDocID(j);
			if (docid1 < docid2) {
				z->pushBack(docid1,this->getScore(i),*this->getTF(i));
				++i;
			} else if (docid1 > docid2) {
				++j;
			} else {		// docid == docid2
				++i;
				++j;
			}
		}
		// x に残りがあれば、それを z に格納する。
		while (i < this->getSize()) {
			z->pushBack(this->getDocID(i),this->getScore(i),*this->getTF(i));
			++i;
		}
	}else
		
		// TF情報が含まれていない場合

		// 従来の関数を実行する
		setDifference(y,z);
}

//
// FUNCTION public
// ModInvertedSearchResultScore::merge --
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
void
ModInvertedSearchResultScore::merge(const ModInvertedSearchResult* _y,
									ModInvertedSearchResult* _z)
{
	ModInvertedSearchResultScore *y = (ModInvertedSearchResultScore *)_y;
	ModInvertedSearchResultScore *z = (ModInvertedSearchResultScore *)_z;
	if (z->getSize()) z->erase(0,z->getSize());
	Iterator first1 = this->begin();
	Iterator last1 = this->end();
	Iterator first2 = y->begin();
	Iterator last2 = y->end();
  	while (first1 != last1 && first2 != last2) {
		const ModInvertedDocumentID docid1 = (*first1).first;
		const ModInvertedDocumentID docid2 = (*first2).first;
		if (docid1 < docid2) {
			z->pushBack(first1);
			++first1;
		} else if (docid1 > docid2) {
			++first2;
		} else {		// first == first2
      		z->pushBack(Element(docid1,(*first1).second + (*first2).second));
			++first1;
			++first2;
		}
	}
	// x に残りがあれば、それを z に格納する。
	while (first1 != last1) {
		z->pushBack(first1);
		++first1;
	}
}

//
// FUNCTION public
// _ModInvertedSearchResultTF::_ModInvertedSearchResultTF
//	-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
_ModInvertedSearchResultTF::_ModInvertedSearchResultTF(
	const ModSize size,
	const Element& defaultValue)
	:ModInvertedVector<Element>(size,defaultValue)
{
	resultType = (1 <<_SYDNEY::Inverted::FieldType::Internal)|
		(1 << _SYDNEY::Inverted::FieldType::Rowid) |
		(1 << _SYDNEY::Inverted::FieldType::Tf);
}

//
// FUNCTION public
// ModInvertedSearchResultTF::ModInvertedSearchResultTF
//	-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModInvertedSearchResultTF::ModInvertedSearchResultTF(
	const ModSize size,
	const Element& defaultValue)
	:ModInvertedVector<Element>(size,defaultValue)
{
	resultType = (1 <<_SYDNEY::Inverted::FieldType::Rowid) |
				 ( 1 << _SYDNEY::Inverted::FieldType::Tf);
}

//
// FUNCTION public
// _ModInvertedSearchResultScoreAndTF::_ModInvertedSearchResultScoreAndTF
//	-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
_ModInvertedSearchResultScoreAndTF::_ModInvertedSearchResultScoreAndTF(
	const ModSize size,
	const Element& defaultValue)
	:ModInvertedVector<Element>(size,defaultValue)
{
	resultType = (1 <<_SYDNEY::Inverted::FieldType::Internal)|
		(1 << _SYDNEY::Inverted::FieldType::Rowid) |
		(1 << _SYDNEY::Inverted::FieldType::Score) |
		(1 << _SYDNEY::Inverted::FieldType::Tf);
}

//
// FUNCTION public
// ModInvertedSearchResultScoreAndTF::ModInvertedSearchResultScoreAndTF
//	-- コンストラクタ
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//
// EXCEPTIONS
//
ModInvertedSearchResultScoreAndTF::ModInvertedSearchResultScoreAndTF(
	const ModSize size,
	const Element& defaultValue)
	:ModInvertedVector<Element>(size,defaultValue)
{
	resultType = (1 << _SYDNEY::Inverted::FieldType::Rowid) |
		(1 << _SYDNEY::Inverted::FieldType::Score) |
		(1 << _SYDNEY::Inverted::FieldType::Tf);
}

//
// Copyright (c) 1999, 2000, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
