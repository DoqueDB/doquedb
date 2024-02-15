// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/CheckedField.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_SCALAR_CHECKEDFIELD_H
#define __SYDNEY_PLAN_SCALAR_CHECKEDFIELD_H

#include "Plan/Scalar/FieldWrapper.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Scalar::CheckedField -- Interface for schema field
//
//	NOTES
class CheckedField
	: public FieldWrapper
{
public:
	typedef FieldWrapper Super;
	typedef CheckedField This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Field* pField_,
						const Utility::FileSet& cFileSet_);
	// destructor
	~CheckedField() {}

	// accessor
	const Utility::FileSet& getFileSet() {return m_cFileSet;}
	void removeFile(Interface::IFile* pFile_) {m_cFileSet.remove(pFile_);}

////////////////////////////////////
// Field::
	virtual Field* checkRetrieve(Opt::Environment& cEnvironment_)
	{return this;}
	virtual Field* checkPut(Opt::Environment& cEnvironment_)
	{return this;}
	virtual bool isChecked() {return true;}
	virtual CheckedField* getChecked() {return this;}

protected:
private:
	// constructor
	CheckedField(Field* pField_,
				 const Utility::FileSet& cFileSet_)
		: Super(pField_, pField_->getDataType()),
		  m_cFileSet(cFileSet_)
	{}

	Utility::FileSet m_cFileSet;
};

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_SCALAR_CHECKEDFIELD_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
