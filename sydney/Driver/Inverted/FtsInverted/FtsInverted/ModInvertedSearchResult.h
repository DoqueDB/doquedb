// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedSearchResult.h -- ランキング検索結果インタフェイス
// 
// Copyright (c) 1997, 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __ModInvertedSearchResult_H__
#define __ModInvertedSearchResult_H__

#include "ModTypes.h"
#include "ModInvertedTypes.h"
#include "ModPair.h"
#include "ModCharString.h"

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "Inverted/Module.h"
#include "Inverted/SortParameter.h"
#include "Inverted/FileIDNumber.h"
#include "Inverted/FieldType.h"
#endif

class ModInvertedBooleanResult;
class ModInvertedSearchResultScore;
class ModInvertedRankingScoreCombiner;
class ModInvertedRankingScoreNegator;
class ModInvertedSearchResultTF;
class ModInvertedSearchResultScoreAndTF;
class _ModInvertedSearchResultTF;
class _ModInvertedSearchResultScoreAndTF;

//
// CLASS
// ModInvertedSearchResult -- 検索結果にアクセスするための基底クラス
//
// interface class
//
class ModInvertedSearchResult
{
public:
	typedef ModInvertedRankingScoreCombiner	ScoreCombiner;
	typedef ModInvertedRankingScoreNegator	ScoreNegator;
	typedef ModPair<ModInvertedTermNo, ModInvertedTermFrequency> TFListElement;
	typedef ModInvertedVector<TFListElement> TFList;

	ModUInt32 resultType;
	///////////////////////////////////////////////////////
	ModInvertedSearchResult(){resultType = 0;}
	virtual ~ModInvertedSearchResult(){}
	
	// -
	virtual ModInvertedSearchResult *create() const {return 0;};
	// internal create
	virtual ModInvertedSearchResult *_create()const { return create();}
	
	// 実行する
	SYD_INVERTED_FUNCTION
	static ModInvertedSearchResult *factory(const ModUInt32 resultType_);
	
	//--- 積集合 ---
	virtual void setIntersection(const ModInvertedSearchResult* y,
								       ModInvertedSearchResult* z);
	virtual void setIntersection(const ModInvertedSearchResult* y,
									   ModInvertedSearchResult* z,
									   ModInvertedTermNo,
								 ModBoolean bFirst_);
	virtual void setIntersection(const ModInvertedSearchResult* y,
									   ModInvertedSearchResult* z,
									   ScoreCombiner& combiner);

	ModInvertedSearchResult operator&(const ModInvertedSearchResult& y)
	{
		ModInvertedSearchResult z;
		setIntersection(&y, &z);
		return z;
	}
	ModInvertedSearchResult& operator&=(const ModInvertedSearchResult& y)
	{
		ModInvertedSearchResult z;
		setIntersection(&y, &z);
		*this = z;
		return *this;
	}
	
	//--- 和集合 ---
	virtual void setUnion(const ModInvertedSearchResult* y,
						  ModInvertedSearchResult* z);
	virtual void setUnion(const ModInvertedSearchResult* y,
						  ModInvertedSearchResult* z,
						  ModInvertedTermNo);
	virtual void setUnion(const ModInvertedSearchResult*,
						  ModInvertedSearchResult*,
						  ScoreCombiner&);
	virtual void setUnion(const ModInvertedSearchResult *y);
	virtual void setUnion(const ModInvertedSearchResult* y,
						  ModInvertedTermNo);

	//--- 差集合 ---
	virtual void setDifference(const ModInvertedSearchResult* y,
							   ModInvertedSearchResult* z);
	virtual void setDifference(const ModInvertedSearchResult* y,
							   ModInvertedSearchResult* z,
							   ModInvertedTermNo);
	virtual void setDifference(const ModInvertedSearchResult* y);

    //--- マージ ---
	virtual void merge(const ModInvertedSearchResult* y,
					   ModInvertedSearchResult* z);
	virtual void merge(const ModInvertedSearchResult* y,
					   ModInvertedSearchResult* z,
					   ModInvertedTermNo);

	// copy
	virtual void copy(ModInvertedSearchResult* x)
	{
	}

	virtual void copy(ModInvertedSearchResult* x,
					  int from,
					  int to)
	{
	}

	// accessor
	virtual ModInvertedDocumentID getDocID(ModSize i)const{return 0;}
	virtual void setDocID(ModSize i, ModInvertedDocumentID){}

	virtual ModInvertedDocumentScore getScore(ModSize i) const{return 1.0;}
	virtual void setScore(ModSize i, ModInvertedDocumentScore score){}
	virtual void _setTF(ModSize i,ModUInt32 tf){}
	virtual void setTF(ModSize i,TFList tf){}
	virtual ModInvertedTermFrequency _getTF(ModSize i) const{return 0;}
	virtual const TFList* getTF(ModSize i) const{ return 0;}

	// ModInvertedVector originated method
	virtual void pushBack(ModInvertedDocumentID docid,
						  ModInvertedDocumentScore score = 0,
						  ModInvertedTermFrequency tf = 0){}
	virtual void pushBack(ModInvertedDocumentID docid,
						  ModInvertedDocumentScore score,
						  const TFList&){}
	virtual ModSize getSize() const {return 0;};
	virtual void reserve(ModSize n){}
	virtual void erase(ModSize i){ }
	virtual void erase(ModSize i,ModSize j){ }
	virtual void clear(){}
	// sort
	virtual void sort(ModSize begin,ModSize end,_SYDNEY::Inverted::SortParameter::Value id){}
	virtual void sort(_SYDNEY::Inverted::SortParameter::Value id = _SYDNEY::Inverted::SortParameter::RowIdAsc ){}
	virtual void sort(_SYDNEY::Inverted::SortParameter::Value id1,
					  _SYDNEY::Inverted::SortParameter::Value id2)
	{
		// Ignore second sort parameter.
		sort(id1);
	}
	//
	ModUInt32 getType(){return resultType;}
}; // ModInvertedSearchResult

//
// CLASS
// ModInvertedBooleanResult --
//
class ModInvertedBooleanResult:public ModInvertedSearchResult,
							   public ModInvertedVector<ModInvertedDocumentID>
{
public:
	typedef ModInvertedVector<ModInvertedDocumentID>::Iterator Iterator;
	typedef ModInvertedVector<ModInvertedDocumentID> Vector;

	// コンストラクタ
	ModInvertedBooleanResult(const ModSize size = 0,
							 const ModInvertedDocumentID defaultValue
							 = ModInvertedUndefinedDocumentID)
	:ModInvertedVector<ModInvertedDocumentID>(size,defaultValue)
	{
		resultType = 1 << _SYDNEY::Inverted::FieldType::Rowid;
	}

	explicit ModInvertedBooleanResult(const ModInvertedVector<ModInvertedDocumentID>& ids_)
	: ModInvertedVector<ModInvertedDocumentID>(ids_)
	{
		resultType = 1 << _SYDNEY::Inverted::FieldType::Rowid;
	}

	ModInvertedSearchResult *create() const{ return new ModInvertedBooleanResult();}
	
	//
	void setIntersection(const ModInvertedSearchResult* y,
						 ModInvertedSearchResult* z);
	void setUnion(const ModInvertedSearchResult* _y,
				  ModInvertedSearchResult* _z);
	void setDifference(const ModInvertedSearchResult* y,
					   ModInvertedSearchResult* z);
	void merge(const ModInvertedSearchResult* y,
			   ModInvertedSearchResult* z);

	// copy
	void copy(ModInvertedSearchResult *x)
	{
		*(ModInvertedBooleanResult*)this = *(ModInvertedBooleanResult*)x;
	}
	void copy(ModInvertedSearchResult *x_,int from,int to)
	{
		ModInvertedBooleanResult* x = (ModInvertedBooleanResult*)x_;

		ModCopy(x->begin() + from,x->begin() + to  ,this->begin());
	}

	// accessor
	ModInvertedDocumentID getDocID(ModSize i)const{return this->at(i);}
	void setDocID(ModSize i,ModInvertedDocumentID docid){
		this->at(i) = docid;
	}
	// ModInvertedVector originated method
	void pushBack(ModInvertedDocumentID docid,
				  ModInvertedDocumentScore score = 0,
				  ModInvertedTermFrequency tf = 0)
	{ Vector::pushBack(docid);}
	ModSize getSize()const{
		return Vector::getSize();
	}
	void reserve(ModSize n){
		Vector::reserve(n);
	}
	void erase(ModSize i){
		Iterator iter = Vector::begin();
		iter += i;
		Vector::erase(iter);
	}
	void erase(ModSize i,ModSize j){
		if( i < j )
		{
			Iterator first = Vector::begin();
			first += i;
			Iterator last = Vector::begin();
			last += j;
			Vector::erase(first,last);
		}
	}
	Iterator erase(Iterator first,Iterator last){ return Vector::erase(first,last);}
	void clear(){
		Vector::clear();
	}
	// sort
	void sort(ModSize first,ModSize last,_SYDNEY::Inverted::SortParameter::Value id)
	{
		switch(id){
		case _SYDNEY::Inverted::SortParameter::RowIdAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					ModLess<ModInvertedDocumentID>()
					);
			break;
		case _SYDNEY::Inverted::SortParameter::RowIdDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					ModGreater<ModInvertedDocumentID>()
					);
			break;
		}
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id = _SYDNEY::Inverted::SortParameter::RowIdAsc )
	{
		sort(0,Vector::getSize(),id);
	}
}; // ModInvertedBooleanResult

//
// CLASS
// ModInvertedSearchResultScore --
//
class ModInvertedSearchResultScore : public ModInvertedSearchResult,
									 public ModInvertedVector<ModPair<ModInvertedDocumentID,ModInvertedDocumentScore> >
{
	typedef ModPair<ModInvertedDocumentID,ModInvertedDocumentScore> Element;

	class _FirstLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.first < y.first) ? ModTrue : ModFalse; }
	};
	class _FirstGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.first > y.first) ? ModTrue : ModFalse; }
	};
	class _SecondLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.second < y.second) ? ModTrue : ModFalse; }
	};
	class _SecondGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.second > y.second) ? ModTrue : ModFalse; }
	};
public:
	typedef ModInvertedVector<Element>::Iterator Iterator;
	typedef ModInvertedVector<Element> Vector;
	
	// コンストラクタ
	ModInvertedSearchResultScore(const ModSize=0,
								 const Element& defaultValue =
								 Element(ModInvertedUndefinedDocumentID,0.0));
	explicit ModInvertedSearchResultScore (const ModInvertedVector<Element>&);
	
	ModInvertedSearchResult *create() const
		{ return new ModInvertedSearchResultScore();}
	
	// 積集合
	void setIntersection(const ModInvertedSearchResult* y,
						 ModInvertedSearchResult* z);
	void setIntersection(const ModInvertedSearchResult* y,
						 ModInvertedSearchResult* z,
						 ScoreCombiner& combiner);
	// 和集合
	void setUnion(const ModInvertedSearchResult* _y,
				  ModInvertedSearchResult* _z);
	void setUnion(const ModInvertedSearchResult* _y,
				  ModInvertedSearchResult* _z,
				  ScoreCombiner& combiner);
	// 差集合
	void setDifference(const ModInvertedSearchResult* y,
					   ModInvertedSearchResult* z);
    // マージ
	void merge(const ModInvertedSearchResult* y,
			   ModInvertedSearchResult* z);

	// copy
	void copy(ModInvertedSearchResult *x)
	{
		*(ModInvertedSearchResultScore*)this = *(ModInvertedSearchResultScore*)x;
	}
	void copy(ModInvertedSearchResult *x_,int from,int to)
	{
		ModInvertedSearchResultScore* x = (ModInvertedSearchResultScore*)x_;
		ModCopy(x->begin() + from,x->begin() + to  ,this->begin());
	}

	// ModInvertedVector originated method
	void pushBack(ModInvertedDocumentID docid,
				  ModInvertedDocumentScore score = 0.0,
				  ModInvertedTermFrequency tf = 0)
	{
		Vector::pushBack(Element(docid,score));
	}
	void pushBack(const Iterator& iter)
	{
		Vector::pushBack(*iter);
	}
	void pushBack(const Element& data)
	{
		Vector::pushBack(data);
	}
	ModSize getSize()const{
		return Vector::getSize();
	}
	void reserve(ModSize n){
		Vector::reserve(n);
	}
	void erase(ModSize i){
		Iterator iter = Vector::begin();
		iter += i;
		Vector::erase(iter);
	}
	void erase(ModSize i,ModSize j){
		if( i < j )
		{
			Iterator first = Vector::begin();
			first += i;
			Iterator last = Vector::begin();
			last += j;
			Vector::erase(first,last);
		}
	}
	void clear(){
		Vector::clear();
	}
	// accessor
	// get
	ModInvertedDocumentID getDocID(ModSize i)const{
		return this->at(i).first;
	}
	void	setDocID(ModSize i,ModInvertedDocumentID docid){
		(this->at(i)).first = docid;
	}

	ModInvertedDocumentScore getScore(ModSize i) const {
		return this->at(i).second;
	}
	void	setScore(ModSize i,ModInvertedDocumentScore score){
		(this->at(i)).second = score;
	}
	// sort
	void sort(ModSize first,ModSize last,
			  _SYDNEY::Inverted::SortParameter::Value id)
	{
		switch(id){
		case _SYDNEY::Inverted::SortParameter::RowIdAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstLess()
					);
		break;
		case _SYDNEY::Inverted::SortParameter::RowIdDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstGreater()
					);
		break;
		case _SYDNEY::Inverted::SortParameter::ScoreAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_SecondLess()
					);
		break;
		case _SYDNEY::Inverted::SortParameter::ScoreDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_SecondGreater()
					);
		break;
		}
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value	id = _SYDNEY::Inverted::SortParameter::RowIdAsc )
	{
		sort(0,Vector::getSize(),id);
	}
}; // ModInvertedSearchResultScore

//
// CLASS
// _ModInvertedSearchResultTF --
//
// NOTE
// this class is internal use only
//
class _ModInvertedSearchResultTF : public ModInvertedSearchResult,
								   public ModInvertedVector<ModPair<ModInvertedDocumentID, ModInvertedTermFrequency> >
{
	typedef ModPair<ModInvertedDocumentID, ModInvertedTermFrequency> Element;

	class _FirstLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
			{ return (x.first < y.first) ? ModTrue : ModFalse; }
	};
	class _FirstGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
			{ return (x.first > y.first) ? ModTrue : ModFalse; }
	};
public:
	typedef ModInvertedVector<Element>::Iterator Iterator;
	typedef ModInvertedVector<Element> Vector;
	
	_ModInvertedSearchResultTF(
		const ModSize size = 0,
		const Element& defaultValue =
		Element(ModInvertedUndefinedDocumentID,0));

	ModInvertedSearchResult *create() const{ return new _ModInvertedSearchResultTF();}

	// copy
	void copy(ModInvertedSearchResult *x)
	{
		*(_ModInvertedSearchResultTF*)this = *(_ModInvertedSearchResultTF*)x;
	}
	void copy(ModInvertedSearchResult *x_,int from,int to)
	{
		_ModInvertedSearchResultTF* x = (_ModInvertedSearchResultTF*)x_;
		ModCopy(x->begin() + from,x->begin() + to  ,this->begin());
	}

	// ModInvertedVector originated method
	void pushBack(ModInvertedDocumentID docid,
				  ModInvertedDocumentScore score = 0.0,
				  ModInvertedTermFrequency tf = 0)
	{
		Vector::pushBack(Element(docid,tf));
	}
	ModSize getSize()const{
		return Vector::getSize();
	}
	void reserve(ModSize n){
		Vector::reserve(n);
	}
	void erase(ModSize i){
		Iterator iter = Vector::begin();
		iter += i;
		Vector::erase(iter);
	}
	void erase(ModSize i,ModSize j){
		if( i < j )
		{
			Iterator first = Vector::begin();
			first += i;
			Iterator last = Vector::begin();
			last += j;
			Vector::erase(first,last);
		}
	}
	void clear(){
		Vector::clear();
	}

	// accessor
	// get
	ModInvertedDocumentID getDocID(ModSize i)const{
		return this->at(i).first;
	}
	void	setDocID(ModSize i,ModInvertedDocumentID docid){
		(this->at(i)).first = docid;
	}

	ModInvertedTermFrequency _getTF(ModSize i) const {
		return this->at(i).second;
	}
	void _setTF(ModSize i,ModUInt32 tf) {
		(this->at(i)).second = tf;
	}
	void setTF(ModSize i,TFList tf){
	}
	// sort
	void sort(ModSize first,ModSize last,_SYDNEY::Inverted::SortParameter::Value id)
	{
	switch(id){
	case _SYDNEY::Inverted::SortParameter::RowIdAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstLess()
				);
			break;
	case _SYDNEY::Inverted::SortParameter::RowIdDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstGreater()
				);
			break;
		}
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id = _SYDNEY::Inverted::SortParameter::RowIdAsc )
	{
		 sort(0,Vector::getSize(),id);
	}
}; // _ModInvertedSearchResultTF

//
// CLASS
// ModInvertedSearchResultTF --
//
class ModInvertedSearchResultTF : public ModInvertedSearchResult,
								  public ModInvertedVector<ModPair<ModInvertedDocumentID, ModInvertedSearchResult::TFList> >
{
	typedef ModPair<ModInvertedDocumentID, TFList> Element;

	class _FirstLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.first < y.first) ? ModTrue : ModFalse; }
	};
	class _FirstGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.first > y.first) ? ModTrue : ModFalse; }
	};
 public:
	typedef ModInvertedVector<Element>::Iterator Iterator;
	typedef ModInvertedVector<Element> Vector;
	
	ModInvertedSearchResultTF(
		const ModSize=0,
		const Element& defaultValue =
		Element(ModInvertedUndefinedDocumentID, TFList(0, TFListElement(0,0))));

	ModInvertedSearchResult *create() const{
		return new ModInvertedSearchResultTF();
	}
	ModInvertedSearchResult *_create()const{
		return new _ModInvertedSearchResultTF();
	}

	// copy
	void copy(ModInvertedSearchResult *x)
	{
		if(x->getSize()>0 && x->getTF(0))
			*(ModInvertedSearchResultTF*)this = *(ModInvertedSearchResultTF*)x;
	}
	void copy(ModInvertedSearchResult *x_,
			  int from,
			  int to)
	{
		if(x_->getSize()>0 && x_->getTF(0))
		{
			ModInvertedSearchResultTF* x = (ModInvertedSearchResultTF*)x_;
			ModCopy(x->begin() + from,x->begin() + to  ,this->begin());
		}
	}

	// ModInvertedVector originated method
	void pushBack(ModInvertedDocumentID docid,
				  ModInvertedDocumentScore score,
				  const TFList& tf)
	{
		Vector::pushBack(Element(docid,tf));
	}
	ModSize getSize()const{
		return Vector::getSize();
	}
	void reserve(ModSize n){
		Vector::reserve(n);
	}
	void erase(ModSize i){
		Iterator iter = Vector::begin();
		iter += i;
		Vector::erase(iter);
	}
	void erase(ModSize i,ModSize j){
		if( i < j )
		{
			Iterator first = Vector::begin();
			first += i;
			Iterator last = Vector::begin();
			last += j;
			Vector::erase(first,last);
		}
	}

	void clear(){
		Vector::clear();
	}
	// accessor
	// get
	ModInvertedDocumentID getDocID(ModSize i)const{
		return this->at(i).first;
	}
	void	setDocID(ModSize i,ModInvertedDocumentID docid){
		(this->at(i)).first = docid;
	}

	const TFList* getTF(ModSize i) const {
		return &this->at(i).second;
	}
	void setTF(ModSize i,TFList tf){
		(this->at(i)).second = tf;
	}
	// sort
	void sort(ModSize first,ModSize last,_SYDNEY::Inverted::SortParameter::Value id)
	{
		switch(id){
		case _SYDNEY::Inverted::SortParameter::RowIdAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstLess()
			);
		break;
		case _SYDNEY::Inverted::SortParameter::RowIdDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstGreater()
			 );
		break;
		}
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id = _SYDNEY::Inverted::SortParameter::RowIdAsc)
	{
		 sort(0,Vector::getSize(),id);
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id1,
			  _SYDNEY::Inverted::SortParameter::Value id2)
	{
		 sort(0, getSize(), id1);
		 if (id2 == _SYDNEY::Inverted::SortParameter::TermNoAsc)
		 {
			 for (Iterator i = begin(); i != end(); ++i)
			 {
				 TFList& vec = (*i).second;
				 ModSort(vec.begin(), vec.end());
			 }
		 }
	}
}; // ModInvertedSearchResultTF

//
// CLASS
// _ModInvertedSearchResultScoreAndTF --
//
// NOTE
// this class is internal use only
//
class _ModInvertedSearchResultScoreAndTF : public ModInvertedSearchResult,
										   public ModInvertedVector<ModPair<ModInvertedDocumentID,
										    ModPair<ModInvertedDocumentScore, ModInvertedTermFrequency> > >
{
	typedef ModPair<ModInvertedDocumentScore, ModInvertedTermFrequency> ScorePair;
	typedef ModPair<ModInvertedDocumentID, ScorePair> Element;

	class _FirstLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.first < y.first) ? ModTrue : ModFalse; }
	};
	class _FirstGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.first > y.first) ? ModTrue : ModFalse; }
	};
	class _SecondLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.second.first < y.second.first) ? ModTrue : ModFalse; }
	};
	class _SecondGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.second.first > y.second.first) ? ModTrue : ModFalse; }
	};
public:
	typedef ModInvertedVector<Element>::Iterator Iterator;
	typedef ModInvertedVector<Element> Vector;
	
	_ModInvertedSearchResultScoreAndTF(
		const ModSize size = 0,
		const Element& defaultValue=
		Element(ModInvertedUndefinedDocumentID, ScorePair(0.0,0)));
	
	ModInvertedSearchResult *create() const{ return new _ModInvertedSearchResultScoreAndTF();}
	
	// copy
	void copy(ModInvertedSearchResult *x)
	{
		*(_ModInvertedSearchResultScoreAndTF*)this = *(_ModInvertedSearchResultScoreAndTF*)x;
	}
	void copy(ModInvertedSearchResult *x_,
			  int from,
			  int to)
	{
		_ModInvertedSearchResultScoreAndTF* x = (_ModInvertedSearchResultScoreAndTF*)x_;
		ModCopy(x->begin() + from,x->begin() + to  ,this->begin());
	}

	// ModInvertedVector originated method
	void pushBack(ModInvertedDocumentID docid,
				  ModInvertedDocumentScore score = 0.0,
				  ModInvertedTermFrequency tf = 0)
	{
		Vector::pushBack(Element(docid,ScorePair(score,tf)));
	}
	ModSize getSize()const{
		return Vector::getSize();
	}
	void reserve(ModSize n){
		Vector::reserve(n);
	}
	void erase(ModSize i){
		Iterator iter = Vector::begin();
		iter += i;
		Vector::erase(iter);
	}
	void erase(ModSize i,ModSize j){
		if( i < j )
		{
			Iterator first = Vector::begin();
			first += i;
			Iterator last = Vector::begin();
			last += j;
			Vector::erase(first,last);
		}
	}
	void clear(){
		Vector::clear();
	}
	// accessor
	ModInvertedDocumentID getDocID(ModSize i)const{
		return this->at(i).first;
	}
	void	setDocID(ModSize i,ModInvertedDocumentID docid){
		(this->at(i)).first = docid;
	}

	ModInvertedDocumentScore getScore(ModSize i)const{
		return this->at(i).second.first;
	}
	void	setScore(ModSize i,ModInvertedDocumentScore score){
		(this->at(i)).second.first = score;
	}

	ModInvertedTermFrequency _getTF(ModSize i)const{
		return this->at(i).second.second;
	}
	void _setTF(ModSize i,ModUInt32 tf) {
		(this->at(i)).second.second = tf;
	}
	void setTF(ModSize i,TFList tf){
	}

	// sort
	void sort(ModSize first,
			  ModSize last,
			  _SYDNEY::Inverted::SortParameter::Value id)
	{
		switch(id){
		case _SYDNEY::Inverted::SortParameter::RowIdAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstLess()
				);
			break;
		case _SYDNEY::Inverted::SortParameter::RowIdDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstGreater()
				);
			break;
		case _SYDNEY::Inverted::SortParameter::ScoreAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_SecondLess()
				);

			break;
		case _SYDNEY::Inverted::SortParameter::ScoreDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_SecondGreater()
				);
			break;
		}
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id = _SYDNEY::Inverted::SortParameter::RowIdAsc )
	{
		sort(0,Vector::getSize(),id);
	}
}; // _ModInvertedSearchResultScoreAndTF

//
// CLASS
// ModInvertedSearchResultScoreAndTF --
//
class ModInvertedSearchResultScoreAndTF : public ModInvertedSearchResult,
										  public ModInvertedVector<ModPair<ModInvertedDocumentID,
																   ModPair<ModInvertedDocumentScore,
																   ModInvertedSearchResult::TFList > > >
{
	typedef ModPair<ModInvertedDocumentScore, ModInvertedSearchResult::TFList> ScorePair;
	typedef ModPair<ModInvertedDocumentID, ScorePair> Element;

	class _FirstLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
			{ return (x.first < y.first) ? ModTrue : ModFalse; }
	};
	class _FirstGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
			{ return (x.first > y.first) ? ModTrue : ModFalse; }
	};
	class _SecondLess
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
			{ return (x.second.first < y.second.first) ? ModTrue : ModFalse; }
	};
	class _SecondGreater
	{
	public:
		ModBoolean operator() (const Element& x, const Element& y)
		{ return (x.second.first > y.second.first) ? ModTrue : ModFalse; }
	};
public:
	typedef ModInvertedVector<Element>::Iterator Iterator;
	typedef ModInvertedVector<Element> Vector;
	
	ModInvertedSearchResultScoreAndTF(
		const ModSize size = 0,
		const Element& defaultValue =
		Element(ModInvertedUndefinedDocumentID,
				ScorePair(0.0, TFList(0,TFListElement(0,0))) ) );
	
	ModInvertedSearchResult* create() const{
		return new ModInvertedSearchResultScoreAndTF();
	}
	ModInvertedSearchResult* _create() const{
		return new _ModInvertedSearchResultScoreAndTF();
	}

	// copy
	void copy(ModInvertedSearchResult* x)
	{
		if(x->getSize()>0 && x->getTF(0))
			*(ModInvertedSearchResultScoreAndTF*)this = *(ModInvertedSearchResultScoreAndTF*)x;
	}
	void copy(ModInvertedSearchResult *x_,int from,int to)
	{
		if(x_->getSize()>0 && x_->getTF(0))
		{
			ModInvertedSearchResultScoreAndTF* x = (ModInvertedSearchResultScoreAndTF*)x_;
			ModCopy(x->begin() + from,x->begin() + to  ,this->begin());
		}
	}

	// ModInvertedVector originated method
	void pushBack(ModInvertedDocumentID docid,
				  ModInvertedDocumentScore score,
				  const TFList& tf)
	{
		 Vector::pushBack(Element(docid, ScorePair(score,tf)));
	}
	ModSize getSize()const{
		return	Vector::getSize();
	}
	void reserve(ModSize n){
		Vector::reserve(n);
	}
	void erase(ModSize i){
		Iterator iter = Vector::begin();
		iter += i;
		Vector::erase(iter);
	}
	void erase(ModSize i,ModSize j){
		if( i < j )
		{
			Iterator first = Vector::begin();
			first += i;
			Iterator last = Vector::begin();
			last += j;
			Vector::erase(first,last);
		}
	}
	void clear(){
		Vector::clear();
	}
	// accessor
	ModInvertedDocumentID getDocID(ModSize i)const{
		return this->at(i).first;
	}
	void setDocID(ModSize i,ModInvertedDocumentID docid){
		(this->at(i)).first = docid;
	}

	ModInvertedDocumentScore getScore(ModSize i)const{
		return this->at(i).second.first;
	}
	void setScore(ModSize i,ModInvertedDocumentScore score){
		(this->at(i)).second.first = score;
	}

	const TFList* getTF(ModSize i)const{
		return &this->at(i).second.second;
	}
	void setTF(ModSize i,TFList tf){
		(this->at(i)).second.second = tf;
	}

	// sort
	void sort(ModSize first,
			  ModSize last,
			  _SYDNEY::Inverted::SortParameter::Value id)
	{
		switch(id){
		case _SYDNEY::Inverted::SortParameter::RowIdAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstLess()
				);
		break;
		case _SYDNEY::Inverted::SortParameter::RowIdDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_FirstGreater()
				);
		break;
		case _SYDNEY::Inverted::SortParameter::ScoreAsc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_SecondLess()
				);

		break;
		case _SYDNEY::Inverted::SortParameter::ScoreDesc:
			ModSort(Vector::begin() + first,
					Vector::begin() + last,
					_SecondGreater()
				);
		break;
		}
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id = _SYDNEY::Inverted::SortParameter::RowIdAsc )
	{
		sort(0,Vector::getSize(),id);
	}
	void sort(_SYDNEY::Inverted::SortParameter::Value id1,
			  _SYDNEY::Inverted::SortParameter::Value id2)
	{
		 sort(0, getSize(), id1);
		 if (id2 == _SYDNEY::Inverted::SortParameter::TermNoAsc)
		 {
			 for (Iterator i = begin(); i != end(); ++i)
			 {
				 TFList& vec = (*i).second.second;
				 ModSort(vec.begin(), vec.end());
			 }
		 }
	}
}; // ModInvertedSearchResultScoreAndTF

typedef ModInvertedSearchResult ModInvertedRankingResult;

#endif //__ModInvertedSearchResult_H__

//
// Copyright (c) 1997, 1999, 2000, 2002, 2003, 2004, 2005, 2006, 2009, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
