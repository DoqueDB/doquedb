// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OtherInformationFile2.cpp -- 全文ファイルのその他情報を格納するファイル
// 
// Copyright (c) 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "FullText/OtherInformationFile2.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/DataArrayData.h"
#include "Common/DoubleData.h"
#include "Common/ObjectIDData.h"
#include "Common/UnsignedIntegerData.h"

#include "PhysicalFile/DirectArea.h"

#include "Exception/BadArgument.h"

#include "Schema/File.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT_USING

namespace
{
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::OtherInformationFile2 -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText::FileID& cFileID_
//		ファイルID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OtherInformationFile2::OtherInformationFile2(FullText::FileID& cFileID_)
	: m_cVectorFile(cFileID_), m_pVariableFile(0)
{
	attach();
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::~OtherInformationFile2 -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OtherInformationFile2::~OtherInformationFile2()
{
	detach();
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::mount -- マウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::mount(const Trans::Transaction& cTransaction_)
{
	m_cVectorFile.mount(cTransaction_);
	if (m_pVariableFile)
	{
		try
		{
			m_pVariableFile->mount(cTransaction_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			m_cVectorFile.unmount(cTransaction_);
			_SYDNEY_RETHROW;
		}
	} 
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::unmount -- アンマウントする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::unmount(const Trans::Transaction& cTransaction_)
{
	m_cVectorFile.unmount(cTransaction_);
	if (m_pVariableFile)
	{
		try
		{
			m_pVariableFile->unmount(cTransaction_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			m_cVectorFile.mount(cTransaction_);
			_SYDNEY_RETHROW;
		}
	} 
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const bool bRestorable_
//		リストア可能かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::startBackup(const Trans::Transaction& cTransaction_,
								   const bool bRestorable_)
{
	m_cVectorFile.startBackup(cTransaction_, bRestorable_);
	if (m_pVariableFile)
	{
		try
		{
			m_pVariableFile->startBackup(cTransaction_, bRestorable_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			m_cVectorFile.endBackup(cTransaction_);
			_SYDNEY_RETHROW;
		}
	} 
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::verify -- 整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	Admin::Verification::Treatment::Value uiTreatment_
//		動作
//	Admin::Verification::Progress& cProgress_
//		経過
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::verify(
	const Trans::Transaction& cTransaction_,
	Admin::Verification::Treatment::Value uiTreatment_,
	Admin::Verification::Progress& cProgress_)
{
	if (isMounted(cTransaction_))
	{
		// ベクターファイルのverify
		m_cVectorFile.verify(cTransaction_, uiTreatment_, cProgress_);
	}
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::move -- ファイルを移動する
//
//	NOTES
//	移動元と移動先のパスが異なっていることが前提。
//
//	ARGUMENTS
//	const Trans::Transaction& cTransaction_
//		トランザクション
//	const Common::StringArrayData& cArea_
//		移動先のエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::move(const Trans::Transaction& cTransaction_,
							const Common::StringArrayData& cArea_)
{
	// 古いパスを取っておく
	Os::Path cOrgPath = m_cVectorFile.getFileID().getPath();
	
	m_cVectorFile.move(cTransaction_, cArea_);
	if (m_pVariableFile)
	{
		try
		{
			m_pVariableFile->move(cTransaction_, cArea_);
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			try
			{
				Common::StringArrayData cOrgArea;
				cOrgArea.setElement(0, cOrgPath);
				m_cVectorFile.move(cTransaction_, cOrgArea);
			}
			catch (...)
			{
				SydErrorMessage << "Recovery failed." << ModEndl;
				Schema::File::setAvailability(
					m_cVectorFile.getFileID().getLockName(),
					false);
			}
			_SYDNEY_RETHROW;
		}
	} 
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::insert -- 挿入する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const Common::DataArrayData& cValue_
//		挿入データ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::insert(ModUInt32 uiRowID_,
							  const Common::DataArrayData& cValue_)
{
	//
	//	【注意】
	//	今のところ、このファイルにはスコア調整フィールドと
	//	セクション情報と特徴語データしか挿入されない
	//

	FileID& cFileID = m_cVectorFile.getFileID();

	; _SYDNEY_ASSERT(cValue_.getCount() ==
					 (int)cFileID.getVectorElementFieldCount());

	if (isMounted(m_cVectorFile.getTransaction()) == false)
	{
		// まだ作成されていないので作成する
		substantiate();
	}

	try
	{
		if (cFileID.isVariableFile() == false)
		{
			// 可変長にデータを挿入しないので、
			// 与えられたデータをそのままベクターに挿入する
		
			m_cVectorFile.insert(uiRowID_, cValue_);
		}
		else
		{
			// 可変長データを挿入する場合
			// ベクターファイルに、可変長データを指すObjectIDを挿入
			// 可変長ファイルに、可変長データを挿入
			// (その可変長データにはObjectIDでアクセス可能)
			
			Common::DataArrayData cVectorValue;
			cVectorValue.reserve(cFileID.getVectorElementFieldCount());

			for (int n = 0; n != cValue_.getCount(); ++n)
			{
				Common::Data::Pointer p = cValue_.getElement(n);
			
				switch (cFileID.getOtherFileElementType(n))
				{
				case FileID::DataType::Double:
				case FileID::DataType::UnsignedInteger:
					// ベクターだけ
					cVectorValue.pushBack(p);
					break;
				case FileID::DataType::Binary:
				case FileID::DataType::UnsignedIntegerArray:
					{
						Common::ObjectIDData* pOID = new Common::ObjectIDData;
						pOID->setNull();
						if (!p->isNull())
						{
							// 可変長に登録する
							PhysicalFile::DirectArea::ID oid
								= m_pVariableFile->insert(*(p.get()));
							pOID->setValue(oid.m_uiPageID, oid.m_uiAreaID);
						}
						cVectorValue.pushBack(pOID);
					}
					break;
				default:
					;
				}
			}

			// ベクターに挿入する
			m_cVectorFile.insert(uiRowID_, cVectorValue);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		recoverAllPages();
		_SYDNEY_RETHROW;
	}
	flushAllPages(); 
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::update -- 更新する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	const Common::DataArrayData& cValue_
//		変更する値
//	const ModVector<int>& vecUpdateField_
//		変更する位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::update(ModUInt32 uiRowID_,
							  const Common::DataArrayData& cValue_,
							  const ModVector<int>& vecUpdateField_)
{
	try
	{
		// 更新対象に可変長フィールドがあるかどうかチェックする

		bool bVariable = false;

		FileID& cFileID = m_cVectorFile.getFileID();
		ModVector<int>::ConstIterator i = vecUpdateField_.begin();
		for (; i != vecUpdateField_.end(); ++i)
		{
			switch (cFileID.getOtherFileElementType(*i))
			{
			case FileID::DataType::Binary:
			case FileID::DataType::UnsignedIntegerArray:
				bVariable = true;
				break;
			}
		}

		if (bVariable)
		{
			Common::DataArrayData cPrev;
		
			i = vecUpdateField_.begin();
			for (; i != vecUpdateField_.end(); ++i)
			{
				switch (cFileID.getOtherFileElementType(*i))
				{
				case FileID::DataType::Double:
					cPrev.pushBack(new Common::DoubleData);
					break;
				case FileID::DataType::UnsignedInteger:
					cPrev.pushBack(new Common::UnsignedIntegerData);
					break;
				case FileID::DataType::Binary:
				case FileID::DataType::UnsignedIntegerArray:
					cPrev.pushBack(new Common::ObjectIDData);
					break;
				}
			}

			// 可変長フィールドを更新するので、ベクターの内容を読み込む
			m_cVectorFile.get(uiRowID_, cPrev, vecUpdateField_);

			int n = 0;
			i = vecUpdateField_.begin();
			for (; i != vecUpdateField_.end(); ++i, ++n)
			{
				Common::Data::Pointer p = cPrev.getElement(n);
			
				switch (cFileID.getOtherFileElementType(*i))
				{
				case FileID::DataType::Double:
				case FileID::DataType::UnsignedInteger:
					{
						*p = *cValue_.getElement(n);
					}
					break;
				case FileID::DataType::Binary:
				case FileID::DataType::UnsignedIntegerArray:
					{
					// OIDを得る
					Common::ObjectIDData& c =
						_SYDNEY_DYNAMIC_CAST(Common::ObjectIDData&,
											 *(p.get()));

					PhysicalFile::DirectArea::ID oid;

					if (!c.isNull())
					{
						oid.m_uiPageID = c.getFormerValue();
						oid.m_uiAreaID = c.getLatterValue();
						// 可変長から削除する
						m_pVariableFile->expunge(oid);
						c.setNull();
					}
					if (!cValue_.getElement(n)->isNull())
					{
						// 可変長に挿入する
						oid = m_pVariableFile->insert(
							*(cValue_.getElement(n).get()));
						c.setValue(oid.m_uiPageID, oid.m_uiAreaID);
					}
					}
					break;
				}
			}

			// ベクターを更新する
			m_cVectorFile.update(uiRowID_, cPrev, vecUpdateField_);
		}
		else
		{
			// 固定長だけ

			// ベクターを更新する
			m_cVectorFile.update(uiRowID_, cValue_, vecUpdateField_);
		}
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		recoverAllPages();
		_SYDNEY_RETHROW;
	}
	flushAllPages();
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::expunge -- 削除する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::expunge(ModUInt32 uiRowID_)
{
	try
	{
		FileID& cFileID = m_cVectorFile.getFileID();

		if (cFileID.isVariableFile())
		{
			// 可変長ファイルがあるので、ベクターを読み出し削除する
			Common::DataArrayData cPrev;
			ModVector<int> vecGetFields;

			ModSize n = cFileID.getVectorElementFieldCount();
			for (ModSize i = 0; i != n; ++i)
			{
				switch (cFileID.getOtherFileElementType(i))
				{
				case FileID::DataType::Binary:
				case FileID::DataType::UnsignedIntegerArray:
					cPrev.pushBack(new Common::ObjectIDData);
					vecGetFields.pushBack(static_cast<int>(i));
					break;
				}
			}

			// ベクターから可変長のOIDを得る
			m_cVectorFile.get(uiRowID_, cPrev, vecGetFields);

			for (int j = 0; j != cPrev.getCount(); ++j)
			{
				const Common::ObjectIDData& c =
					_SYDNEY_DYNAMIC_CAST(const Common::ObjectIDData&,
										 *(cPrev.getElement(j)));

				if (!c.isNull())
				{
					PhysicalFile::DirectArea::ID oid;
					oid.m_uiPageID = c.getFormerValue();
					oid.m_uiAreaID = c.getLatterValue();
				
					// 可変長から削除する
					m_pVariableFile->expunge(oid);
				}
			}
		}
		// ベクターを削除する
		m_cVectorFile.expunge(uiRowID_);
	}
#ifdef NO_CATCH_ALL
	catch (Exception::Object&)
#else
	catch (...)
#endif
	{
		recoverAllPages();
		_SYDNEY_RETHROW;
	}
	flushAllPages();
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::get -- 取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	int iField_
//		取得するフィールド番号
//	Common::Data& cValue_
//		取得結果を格納するデータ型
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
FullText::OtherInformationFile2::get(ModUInt32 uiRowID_,
									 int iField_,
									 Common::Data& cValue_)
{
	FileID& cFileID = m_cVectorFile.getFileID();

	switch (cFileID.getOtherFileElementType(iField_))
	{
	case FileID::DataType::Binary:
	case FileID::DataType::UnsignedIntegerArray:
		{
			// 可変長フィールドはまずObjectIDを取得する
			Common::ObjectIDData cObjectID;
			m_cVectorFile.get(uiRowID_, iField_, cObjectID);
			if (!cObjectID.isNull())
			{
				// 可変長ファイルから読み出す
				PhysicalFile::DirectArea::ID oid;
				oid.m_uiPageID = cObjectID.getFormerValue();
				oid.m_uiAreaID = cObjectID.getLatterValue();
				m_pVariableFile->get(oid, cValue_);
			}
			else
			{
				cValue_.setNull();
			}
		}
		break;
	default:
		// 固定長はベクターから読み出す
		m_cVectorFile.get(uiRowID_, iField_, cValue_);
		break;
	}
}

//
//	FUNCTION public
//	FullText::OtherInformationFile2::getArea -- 可変長ファイルのエリアを取得する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiRowID_
//		ROWID
//	int iField_
//		取得するフィールド番号
//
//	RETURN
//	PhysicalFile::DirectArea
//		取得したエリア
//
//	EXCEPTIONS
//
PhysicalFile::DirectArea
FullText::OtherInformationFile2::getArea(ModUInt32 uiRowID_,
										 int iField_)
{
	PhysicalFile::DirectArea cArea;
	
	FileID& cFileID = m_cVectorFile.getFileID();

	switch (cFileID.getOtherFileElementType(iField_))
	{
	case FileID::DataType::Binary:
	case FileID::DataType::UnsignedIntegerArray:
		{
			// ObjectIDを取得する
			Common::ObjectIDData cObjectID;
			m_cVectorFile.get(uiRowID_, iField_, cObjectID);
			if (!cObjectID.isNull())
			{
				// 可変長ファイルからエリアを取得する
				PhysicalFile::DirectArea::ID oid;
				oid.m_uiPageID = cObjectID.getFormerValue();
				oid.m_uiAreaID = cObjectID.getLatterValue();
				cArea = m_pVariableFile->attachArea(oid);
			}
		}
		break;
	default:
		_TRMEISTER_THROW0(Exception::BadArgument);
		break;
	}

	return cArea;
}

//
//	FUNCTION private
//	FullText::OtherInformationFile2::substantiate -- ファイルを作成する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::substantiate()
{
	// ベクターファイルを作成する
	m_cVectorFile.create(m_cVectorFile.getTransaction());

	if (m_pVariableFile)
	{
		try
		{
			// 可変長ファイルを作成する
			m_pVariableFile->create(m_cVectorFile.getTransaction());
		}
#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			// ベクターファイルを破棄する
			m_cVectorFile.destroy(m_cVectorFile.getTransaction());
			_SYDNEY_RETHROW;
		}
	}
}

//
//	FUNCTION private
//	FullText::OtherInformationFile2::attach -- 物理ファイルをアタッチする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::attach()
{
	if (m_cVectorFile.getFileID().isVariableFile())
	{
		m_pVariableFile = new VariableFile(m_cVectorFile.getFileID());
	}
}

//
//	FUNCTION private
//	FullText::OtherInformationFile2::detach -- 物理ファイルをデタッチする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OtherInformationFile2::detach()
{
	if (m_pVariableFile)
	{
		delete m_pVariableFile, m_pVariableFile = 0;
	}
}

//
//	Copyright (c) 2005, 2006, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
