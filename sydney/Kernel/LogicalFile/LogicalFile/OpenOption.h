// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h -- オープン時に渡すパラメータークラス
// 
// Copyright (c) 1999, 2000, 2002, 2005, 2007, 2008, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALFILE_OPENOPTION_H
#define __SYDNEY_LOGICALFILE_OPENOPTION_H

#include "LogicalFile/Module.h"
#include "LogicalFile/Parameter.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALFILE_BEGIN

//	CLASS
//	LogicalFile::OpenOption -- オープンパラメーター
//
//	NOTES
//	ファイルをオープンするときに渡すパラメーター
//
//	※このファイルでの export はクラス(OpenOption)に対して行うこと。
//	  メソッド単位の export だと assert が発生する。

class SYD_LOGICALFILE_FUNCTION OpenOption
	: public Parameter::Base
{
public:
	struct KeyNumber {
		enum Value {
			OpenMode,
			ReadSubMode,
			Estimate,
			FieldSelect,
			TargetFieldNumber,
			TargetFieldIndex,			// array
			GetByBitSet,
			CacheAllObject,
			SearchByBitSet,
			GroupBy,
			RankByBitSet,
			GetForConstraintLock,
			ValueNum
		};
	};
	struct OpenMode {
		enum Value {
			Unknown,
			Read,
			Search,
			Update,
			Initialize,
			Batch,
			ValueNum
		};
	};
	struct ReadSubMode {
		enum Value {
			Unknown,
			Scan,
			Fetch,
			ValueNum
		};
	};
	struct DriverNumber {
		enum Value {
			Record		= 0x1000,
			Btree		= 0x2000,
			Inverted	= 0x3000,
			FullText	= 0x4000,
			Vector		= 0x5000,
			Lob			= 0x6000,
			Bitmap		= 0x7000,
			KdTree		= 0x8000,
			Array		= 0xA000
		};
	};

	OpenOption();
	OpenOption(const OpenOption& cOther_);
	~OpenOption();

	// 代入オペレーター
	OpenOption& operator =(const OpenOption& cOther_) {return static_cast<OpenOption&>(Base::operator =(cOther_));}

	//クラスIDを得る
	virtual int getClassID() const;

	static void initialize();

protected:
	// バージョン番号に対応したレイアウトマップを取得する
	virtual const Parameter::LayoutMap* getLayoutMap(int iVersion_) const;

private:
	// バージョン番号に対応したレイアウトマップを取得する
	static const Parameter::LayoutMap* getOldLayoutMap(int iVersion_);
	// バージョン番号に対応したレイアウトマップを生成する
	static void createLayoutMap(int iVersion_, Parameter::LayoutMap& cMap_);

	int* m_piOpenMode;
	int* m_piReadSubMode;
	bool* m_pbEstimate;
	bool* m_pbFieldSelect;
	int* m_piTargetFieldNumber;
	Parameter::Element<int>::Array* m_pvecTargetFieldIndex;
	bool* m_pbGetByBitSet;
	bool* m_pbCacheAllObject;
	Common::Object* m_ppSearchByBitSet;
	bool* m_pbGroupBy;
	Common::Object* m_ppRankByBitSet;
	bool* m_pbGetForConstraintLock;
};

_SYDNEY_LOGICALFILE_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALFILE_OPENOPTION_H

//
//	Copyright (c) 1999, 2000, 2002, 2005, 2007, 2008, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
