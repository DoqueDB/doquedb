// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.cpp -- 論理ファイルで使用するパラメーターの基底クラス
// 
// Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "LogicalFile";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"
#include "Common/Object.h"
#include "Os/AutoCriticalSection.h"
#include "Exception/BadArgument.h"
#include "LogicalFile/Parameter.h"
#include "LogicalFile/FileID.h" // メンバーとしてFileIDも持てるようにするため

_SYDNEY_USING
using namespace LogicalFile;

namespace {

//	CONST
//	$$$::_iExtraMapSize --
//		レイアウトマップを超えるパラメーター値を格納するのに使うHashMapの初期サイズ
//
//	NOTES
//	Btreeでキーフィールドが2つの場合のOpenOptionにおける最大数を使用している

const ModSize _iExtraValuesSize = 17;

//	CONST
//	$$$::_iOldVersionElementNumberIndex --
//		バージョンが古いためにレイアウトマップにあってもメンバーにないエントリーが
//		配列要素であったときにその要素数を別に記録しておくためのインデックス番号
//
//	NOTES

const Parameter::Key::IndexType _iOldVersionElementNumberIndex = -1;

//	FUNCTION
//	$$$::_getPointer -- オフセットにしたがってポインターを得る
//
//	NOTES

inline void*
_getPointer(void* pBase_, int iOffset_)
{
	return *(void**)((char*)pBase_ + iOffset_);
}
inline const void*
_getPointer(const void* pBase_, int iOffset_)
{
	return *(void**)((const char*)pBase_ + iOffset_);
}

//	TEMPLATE FUNCTION
//	$$$::_setValue -- オフセットにしたがってパラメーター値をセットする
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーター値の型
//
//	NOTES

template <class __T__>
inline __T__*
_setValue(void* pBase_, int iOffset_, __T__* pValue_)
{
	if (iOffset_ >= 0) {
		return *(__T__**)((char*)(pBase_) + iOffset_) = pValue_;
	}
	return 0;
}

}

//	FUNCTION
//	LogicalFile::Parameter::Key::serialize -- パラメーターキーをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		シリアル化の対象となるアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
void
Parameter::Key::
serialize(ModArchive& cArchiver_)
{
	cArchiver_(first);
	cArchiver_(second);
}

//	FUNCTION
//	LogicalFile::Parameter::Key::toString -- パラメーターキーを文字列化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		文字列を格納するストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS
void
Parameter::Key::
toString(ModUnicodeOstrStream& cStream_) const
{
	cStream_ << "<" << first << "," << second << ">";
}

//	FUNCTION
//	LogicalFile::Parameter::Key::hashValue -- パラメーターキーのハッシュ値を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ハッシュ値
//
//	EXCEPTIONS

ModSize
Parameter::Key::
hashValue() const
{
	return getKey() * 203 + getIndex();
}


//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::getValue -- パラメーター値を得る
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const Key& cKey_
//			パラメーター値を指定するキー
//		__T__& cValue_
//			取得したパラメーター値を返す
//
//	RETURN
//	パラメーターが設定されていればtrue、なければfalse
//
//	EXCEPTIONS

// static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
bool
Parameter::ValueExtractor<__T__, __eType__>::
getValue(const Base& cParameter_, const Key& cKey_, __T__& cValue_)
{
	if ((ModSize)cKey_.getKey() >= cParameter_.m_pMap->getSize()) {
		// キーがレイアウトマップを超えている
		// -> 専用の関数で値を得る
		return getExtraValue(cParameter_, cKey_, cValue_);
	}

	// キーがレイアウトマップ内の値である
	// -> レイアウトマップからキーに対応するエントリーを得る
	const LayoutMapEntry& cEntry = (*cParameter_.m_pMap)[cKey_.getKey()];
	if (cEntry.m_eType == LayoutMapEntry::Type::None) return false;
	if (cEntry.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	if (cEntry.m_iOffset < 0) {
		// バージョンの違いによりレイアウトマップにはあるがメンバーにない
		// -> ExtraValuesを使う
		return getExtraValue(cParameter_, cKey_, cValue_);
	}

	// エントリーにあるオフセットを使ってパラメーター値のポインターを得る
	const void* pPointer = _getPointer(&cParameter_, cEntry.m_iOffset);
	if (!pPointer) return false;

	if (cEntry.m_bArray) {
		// レイアウトマップに配列要素であると指定してある
		const typename Element<__T__>::Array& vecData =
			*static_cast<const typename Element<__T__>::Array*>(pPointer);
		if ((ModSize)cKey_.getIndex() >= vecData.getSize()) return false;
		if (!vecData[cKey_.getIndex()]) return false;
		cValue_ = vecData[cKey_.getIndex()].getValue();
	} else {
		// 単独要素である
		cValue_ = *static_cast<const __T__*>(pPointer);
	}
	return true;
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::setValue -- パラメーター値を設定する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const Key& cKey_
//			パラメーター値を指定するキー
//		const __T__& cValue_
//			設定するパラメーター値
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
setValue(Base& cParameter_, const Key& cKey_, const __T__& cValue_)
{
	if ((ModSize)cKey_.getKey() >= cParameter_.m_pMap->getSize()) {
		// キーがレイアウトマップを超えている
		// -> 専用の関数で値を得る
		setExtraValue(cParameter_, cKey_, cValue_);
		return;
	}

	// キーがレイアウトマップ内の値である
	// -> レイアウトマップからキーに対応するエントリーを得る
	const LayoutMapEntry& cEntry = (*cParameter_.m_pMap)[cKey_.getKey()];
	if (cEntry.m_eType == LayoutMapEntry::Type::None) return;
	if (cEntry.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	if (cEntry.m_iOffset < 0) {
		// バージョンの違いによりレイアウトマップにはあるがメンバーにない
		// -> ExtraValuesを使う
		setExtraValue(cParameter_, cKey_, cValue_);
		return;
	}

	// エントリーにあるオフセットを使って現在のパラメーター値のポインターを得る
	void* pPointer = _getPointer(&cParameter_, cEntry.m_iOffset);

	if (cEntry.m_bArray) {
		// レイアウトマップに配列要素であると指定してある
		if (!pPointer) {
			// 現在値が入っていない場合は新たにnewしておく
			pPointer
				= _setValue(&cParameter_,
							cEntry.m_iOffset,
							new typename Element<__T__>::Array);
		}
		typename Element<__T__>::Array& vecData =
			*static_cast<typename Element<__T__>::Array*>(pPointer);

		const Key::IndexType size = static_cast<Key::IndexType>(vecData.getSize());//unsigned : signed 不一致を訂正
		if (cKey_.getIndex() >= size) {

			// キーの要素番号が現在の要素数以上である

			// ベクタの要素格納用領域を必要であれば拡張する
			//
			//【注意】	拡張の回数を減らすように現在の要素数の倍か
			//			指定された要素番号まで領域を拡張する

			vecData.reserve((size * 2 > cKey_.getIndex()) ?
							size * 2 : cKey_.getIndex());

			if (const ModSize n = cKey_.getIndex() - size)

				// 末尾の要素から指定された要素番号までの間に
				// 初期値の要素を挿入する

				vecData.insert(vecData.end(), n, Element<__T__>());

			// 指定された要素を挿入する

			vecData.pushBack(Element<__T__>(cValue_));

		} else {
			vecData[cKey_.getIndex()] = Element<__T__>(cValue_);
		}
	} else {
		// 単独要素である
		if (!pPointer) {
			// 現在値が入っていない場合は新たにnewしておく
			pPointer
				= _setValue(&cParameter_,
							cEntry.m_iOffset,
							new __T__);
		}
		*static_cast<__T__*>(pPointer) = cValue_;
	}
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::copyValue -- パラメーター値をコピーする
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//		レイアウトマップを超えるパラメーターに関しては本関数は呼ばれない
//
//	ARGUMENTS
//		LogicalFile::Parameter::Base& cDst_
//			コピー先のパラメーター
//		const LogicalFile::Parameter::Base& cSrc_
//			コピー元のパラメーター
//		const LayoutMapEntry& cEntry_
//			コピーするパラメーター値に対応するレイアウトエントリー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
copyValue(Base& cDst_, const Base& cSrc_, const LayoutMapEntry& cEntry_)
{
	if (cEntry_.m_eType == LayoutMapEntry::Type::None) return;
	if (cEntry_.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	if (cEntry_.m_iOffset < 0) {
		// バージョンの違いによりレイアウトマップにはあるがメンバーにない
		// -> ExtraValuesを使うのでここでは何もしない
		return;
	}

	// コピー元とコピー先のポインターを得る
	const void* pSrcPointer = _getPointer(&cSrc_, cEntry_.m_iOffset);
	void* pDstPointer = _getPointer(&cDst_, cEntry_.m_iOffset);
	if (pSrcPointer) {
		// コピー元がNULLでない場合、値を取得してコピー先に設定する
		if (cEntry_.m_bArray) {
			// レイアウトマップに配列要素という指定がある
			const typename Element<__T__>::Array& vecSrc =
				*static_cast<const typename Element<__T__>::Array*>(pSrcPointer);
			int n = vecSrc.getSize();
			if (!pDstPointer) {
				pDstPointer
					= _setValue(&cDst_,
								cEntry_.m_iOffset,
								new typename Element<__T__>::Array);
			}
			typename Element<__T__>::Array& vecDst =
				*static_cast<typename Element<__T__>::Array*>(pDstPointer);
			vecDst = vecSrc;
		} else {
			// 単独要素である
			if (!pDstPointer) {
				pDstPointer
					= _setValue(&cDst_,
								cEntry_.m_iOffset,
								new __T__);
			}
			*static_cast<__T__*>(pDstPointer) =
				*static_cast<const __T__*>(pSrcPointer);
		}
	} else {
		// コピー元がNULLの場合、コピー先の値がすでに入っていたらクリアする
		if (pDstPointer) {
			deleteValue(cDst_, cEntry_);
			_setValue<void>(&cDst_, cEntry_.m_iOffset, 0);
		}
	}
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::deleteValue -- パラメーター値を破棄する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//		レイアウトマップを超えるパラメーターに関しては本関数は呼ばれない
//
//	ARGUMENTS
//		LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const LayoutMapEntry& cEntry_
//			破棄するパラメーター値に対応するレイアウトエントリー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
deleteValue(Base& cParameter_, const LayoutMapEntry& cEntry_)
{
	if (cEntry_.m_eType == LayoutMapEntry::Type::None) return;
	if (cEntry_.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	if (cEntry_.m_iOffset < 0) {
		// バージョンの違いによりレイアウトマップにはあるがメンバーにない
		// -> ExtraValuesを使うのでここでは何もしない
		return;
	}

	void* pPointer = _getPointer(&cParameter_, cEntry_.m_iOffset);
	if (pPointer) {
		if (cEntry_.m_bArray) {
			// Vectorの各要素はElement<__T__>::Arrayのデストラクト時に同時に破棄されることが前提
			delete static_cast<typename Element<__T__>::Array*>(pPointer);
		} else {
			delete static_cast<__T__*>(pPointer);
		}
		_setValue<void>(&cParameter_, cEntry_.m_iOffset, 0);
	}
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::getExtraValue --
//		レイアウトマップを超えるパラメーター値を得る
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const Key& cKey_
//			パラメーター値を指定するキー
//		__T__& cValue_
//			取得したパラメーター値を返す
//
//	RETURN
//	パラメーター値が設定されていればtrue、なければfalse
//
//	EXCEPTIONS

// static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
bool
Parameter::ValueExtractor<__T__, __eType__>::
getExtraValue(const Base& cParameter_, const Key& cKey_, __T__& cValue_)
{
	// バージョンの違いによりレイアウトマップにあっても
	// ExtraValuesに格納されることがありうるので以下の検査は不要
	/*
	if ((ModSize)cKey_.getKey() < cParameter_.m_pMap->getSize()) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	*/

	// HashMapから値を得る
	ExtraValues::ConstIterator i = cParameter_.m_cExtraValues.find(cKey_);

	// HashMapになければパラメーター値はない
	if (i == cParameter_.m_cExtraValues.end()) return false;

	// 値を取得する
	const ExtraElement& element = ExtraValues::getValue(i);
	if (element.m_eType == LayoutMapEntry::Type::None) return false;
	if (element.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	const void* pPointer = element.getPointer();
	if (!pPointer) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	cValue_ = *static_cast<const __T__*>(pPointer);

	return true;
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::setExtraValue --
//		レイアウトマップを超えるパラメーター値を設定する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const Key& cKey_
//			パラメーター値を指定するキー
//		const __T__& cValue_
//			設定するパラメーター値
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
setExtraValue(Base& cParameter_, const Key& cKey_, const __T__& cValue_)
{
	// バージョンの違いによりレイアウトマップにあっても
	// ExtraValuesに格納されることがありうるので以下の検査は不要
	/*
	if ((ModSize)cKey_.getKey() < cParameter_.m_pMap->getSize()) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	*/

	// HashMapから値を得る
	ExtraValues::Iterator i = cParameter_.m_cExtraValues.find(cKey_);

	// HashMapになければ新たに設定する
	if (i == cParameter_.m_cExtraValues.end()) {
		ExtraElement element;
		element.setPointer(&cValue_, __eType__);
		(void)cParameter_.m_cExtraValues.insert(cKey_, element, ModTrue /* NoDuplicationCheck */);
		return;
	}

	// HashMapにあれば値を書き換える
	ExtraElement& element = ExtraValues::getValue(i);
	element.setPointer(&cValue_, __eType__);
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::serialize -- パラメーター値のシリアライズ
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//		レイアウトマップを超えるパラメーターに関しては本関数は呼ばれない
//
//	ARGUMENTS
//		LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const LayoutMapEntry& cEntry_
//			シリアライズするパラメーター値に対応するレイアウトエントリー
//		LogicalFile::Parameter::Key::Type iKey_
//			エントリーに対応するキー
//		ModArchive& cArchiver_
//			対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
serialize(Base& cParameter_, const LayoutMapEntry& cEntry_, Key::Type iKey_, ModArchive& cArchiver_)
{
	if (cEntry_.m_eType == LayoutMapEntry::Type::None) return;
	if (cEntry_.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	if (cEntry_.m_iOffset < 0) {
		// バージョンが古いため対応するメンバーがない
		// -> 専用の関数を使う
		serializeOldVersionEntry(cParameter_, cEntry_, iKey_, cArchiver_);
		return;
	}

	// StoreとLoadで同じコードにする
	// -> existsの設定はLoad時は無駄であるがステータスをチェックするのと
	//    値を代入するのと大差はないと思われる
	void* pPointer = _getPointer(&cParameter_, cEntry_.m_iOffset);
	char exists = (pPointer) ? 1 : 0;
	cArchiver_(exists);
	if (exists) {
		if (cEntry_.m_bArray) {
			int n;
			if (!pPointer) {
				if (!cArchiver_.isLoad()) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				cArchiver_(n);
				pPointer
					= _setValue(&cParameter_,
								cEntry_.m_iOffset,
								new typename Element<__T__>::Array(n));
			} else {
				if (!cArchiver_.isStore()) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				n = static_cast<typename Element<__T__>::Array*>(pPointer)->getSize();
				cArchiver_(n);
			}
			typename Element<__T__>::Array& vecData =
				*static_cast<typename Element<__T__>::Array*>(pPointer);
			for (int i = 0; i < n; ++i) {
				serializeVectorElement(vecData[i], cArchiver_);
			}
		} else {
			if (!pPointer) {
				if (!cArchiver_.isLoad()) {
					_SYDNEY_THROW0(Exception::BadArgument);
				}
				pPointer
					= _setValue(&cParameter_,
								cEntry_.m_iOffset,
								new __T__);
			}
			serializeScalar(*static_cast<__T__*>(pPointer), cArchiver_);
		}
	}
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::serializeScalar --
//		配列でない値をシリアライズする
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		__T__& cValue_
//			対象の値
//		ModArchive& cArchiver_
//			対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
serializeScalar(__T__& cValue_, ModArchive& cArchiver_)
{
	cArchiver_(cValue_);
}

// FileIDはさらにObjectPointerになっているので別に定義する
template <>
void
Parameter::ValueExtractor<Common::ObjectPointer<FileID>, Parameter::LayoutMapEntry::Type::FileID>::
serializeScalar(Common::ObjectPointer<FileID>& cValue_, ModArchive& cArchiver_)
{
	if (cArchiver_.isStore()) {
		int tmp = (cValue_.get() != 0)?1:0;
		cArchiver_ << tmp;
		if (tmp) {
			cArchiver_ << *cValue_;
		}
	} else {
		int tmp;
		Common::ObjectPointer<FileID> pFileID;
		cArchiver_ >> tmp;
		if (tmp) {
			pFileID = new FileID;
			cArchiver_ >> *pFileID;
			cValue_ = pFileID;
		}
	}
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::serializeVectorElement --
//		配列要素をシリアライズする
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		LogicalFile::Parameter::Element<__T__>& cElement_
//			対象の配列要素
//		ModArchive& cArchiver_
//			対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
serializeVectorElement(Element<__T__>& cElement_, ModArchive& cArchiver_)
{
	cArchiver_(cElement_.first);
	if (cElement_.first)
		serializeScalar(cElement_.second, cArchiver_);
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::serializeOldVersionEntry --
//		古いバージョンでメンバーだったパラメーター値をシリアル化する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//		バージョンの違いによりメンバーとしてもはや存在しなくなったエントリーを読み込む
//
//	ARGUMENTS
//		LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const LayoutMapEntry& cEntry_
//			シリアライズするパラメーター値に対応するレイアウトエントリー
//		LogicalFile::Parameter::Key::Type iKey_
//			エントリーに対応するキー
//		ModArchive& cArchiver_
//			対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
serializeOldVersionEntry(Base& cParameter_, const LayoutMapEntry& cEntry_, Key::Type iKey_, ModArchive& cArchiver_)
{	;	}
// 現在、“メンバーとしてもはや存在しなくなったエントリー”は、なし。
#ifdef OBSOLETE
{
	; _SYDNEY_ASSERT(cEntry_.m_iOffset < 0);

	// ★注意★
	// serializeの動作と合わせなければならない

	if (cArchiver_.isLoad()) {
		char exists;
		cArchiver_ >> exists;
		if (exists) {
			if (cEntry_.m_bArray) {
				int n;
				cArchiver_ >> n;
				// 要素数を記録する特別なエントリーを作る
				ValueExtractor<int, LayoutMapEntry::Type::Integer>::setExtraValue(cParameter_,
																				  Key(iKey_, _iOldVersionElementNumberIndex),
																				  n);
				// 要素数分データを読み込み、ExtraValuesに格納する
				__T__ cData;
				for (int i = 0; i < n; ++i) {
					bool found;
					cArchiver_ >> found;
					if (found) {
						serializeScalar(cData, cArchiver_);
						setExtraValue(cParameter_, Key(iKey_, i), cData);
					}
				}
			} else {
				__T__ cData;
				serializeScalar(cData, cArchiver_);
				setExtraValue(cParameter_, Key(iKey_), cData);
			}
		}
	} else {
		// Storeの場合対象となるデータは以前記録されていたものをserializeで読み込んだときしかない
		; _SYDNEY_ASSERT(cArchiver_.isStore());

		char exists;
		if (cEntry_.m_bArray) {
			// 配列要素の場合要素数を記録する特別なエントリーがある
			int n = 0;
			ValueExtractor<int, LayoutMapEntry::Type::Integer>::getExtraValue(cParameter_,
																			  Key(iKey_, _iOldVersionElementNumberIndex),
																			  n);
			exists = (n) ? 1 : 0;
			cArchiver_ << exists;
			if (exists) {
				cArchiver_ << n;
				__T__ cData;
				for (int i = 0; i < n; ++i) {
					bool found = getExtraValue(cParameter_, Key(iKey_, i), cData);
					cArchiver_ << found;
					if (found) {
						serializeScalar(cData, cArchiver_);
					}
				}
			}
		} else {
			__T__ cData;
			bool found = getExtraValue(cParameter_, Key(iKey_), cData);
			exists = found ? 1 : 0;
			cArchiver_ << exists;
			if (exists) {
				serializeScalar(cData, cArchiver_);
			}
		}
	}
}
#endif // OBSOLETE

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::toString -- パラメーター値の文字列化
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//		レイアウトマップを超えるパラメーターに関しては本関数は呼ばれない
//
//	ARGUMENTS
//		LogicalFile::Parameter::Base& cParameter_
//			対象のパラメーター
//		const LayoutMapEntry& cEntry_
//			シリアライズするパラメーター値に対応するレイアウトエントリー
//		ModOstrStream& cStream_
//			対象の文字列ストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
toString(const Base& cParameter_, const LayoutMapEntry& cEntry_, ModUnicodeOstrStream& cStream_)
{
	if (cEntry_.m_eType == LayoutMapEntry::Type::None) return;
	if (cEntry_.m_eType != __eType__) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	if (cEntry_.m_iOffset < 0) {
		// バージョンの違いによりレイアウトマップにはあるがメンバーにない
		// -> ExtraValuesを使うのでここでは何もしない
		return;
	}

	const void* pPointer = _getPointer(&cParameter_, cEntry_.m_iOffset);
	if (!pPointer) {
		cStream_ << "null";
		return;
	}

	if (cEntry_.m_bArray) {
		const typename Element<__T__>::Array& vecData =
			*static_cast<const typename Element<__T__>::Array*>(pPointer);
		int n = vecData.getSize();

		cStream_ << "[";
		for (int i = 0; i < n; ++i) {
			if (i) cStream_ << ",";
			toStringVectorElement(vecData[i], cStream_);
		}
		cStream_ << "]";
	} else {
		toStringScalar(*static_cast<const __T__*>(pPointer), cStream_);
	}
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::toStringScalar --
//		配列でない値を文字列化する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		const __T__& cValue_
//			対象の値
//		ModUnicodeOstrStream& cStream_
//			対象のストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
toStringScalar(const __T__& cValue_, ModUnicodeOstrStream& cStream_)
{
	cStream_ << cValue_;
}

// ObjectPointerとFileIDは文字列化する方法が違うので別に定義する
template <>
void
Parameter::ValueExtractor<const Common::Object*, Parameter::LayoutMapEntry::Type::ObjectPointer>::
toStringScalar(const Common::Object* const& cValue_, ModUnicodeOstrStream& cStream_)
{
	cStream_ << *cValue_;
}
template <>
void
Parameter::ValueExtractor<Common::ObjectPointer<FileID>, Parameter::LayoutMapEntry::Type::FileID>::
toStringScalar(const Common::ObjectPointer<FileID>& cValue_, ModUnicodeOstrStream& cStream_)
{
	cStream_ << *cValue_;
}

//	TEMPLATE FUNCTION
//	LogicalFile::Parameter::ValueExtractor::toStringVectorElement --
//		配列要素を文字列化する
//
//	TEMPLATE ARGUMENTS
//	class __T__
//		パラメーターの型
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value __eType__
//		パラメーターの型を表すenum型の値(debug用)
//
//	NOTES
//
//	ARGUMENTS
//		const LogicalFile::Parameter::Element<__T__>& cElement_
//			対象の配列要素
//		ModUnicodeOstrStream& cStream_
//			対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//static
template <class __T__, Parameter::LayoutMapEntry::Type::Value __eType__>
void
Parameter::ValueExtractor<__T__, __eType__>::
toStringVectorElement(const Element<__T__>& cElement_, ModUnicodeOstrStream& cStream_)
{
	if (cElement_.first) {
		cStream_ << cElement_.second;
	}
}

// ObjectPointerとFileIDは文字列化の方法が違うので別に定義する
template <>
void
Parameter::ValueExtractor<const Common::Object*, Parameter::LayoutMapEntry::Type::ObjectPointer>::
toStringVectorElement(const Element<const Common::Object*>& cElement_, ModUnicodeOstrStream& cStream_)
{
	if (cElement_.first) {
		cStream_ << *cElement_.second;
	}
}
template <>
void
Parameter::ValueExtractor<Common::ObjectPointer<FileID>, Parameter::LayoutMapEntry::Type::FileID>::
toStringVectorElement(const Element<Common::ObjectPointer<FileID> >& cElement_, ModUnicodeOstrStream& cStream_)
{
	if (cElement_.first) {
		cStream_ << *cElement_.second;
	}
}


//////////////////
// ExtraElement //
//////////////////

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::ExtraElement --
//		コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::ExtraElement::ExtraElement& v
//		コピー元のオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Parameter::ExtraElement::
ExtraElement(const ExtraElement& v)
	: m_eType(LayoutMapEntry::Type::None)
{
	*this = v;
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::operator= --
//		代入オペレーター
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::ExtraElement::ExtraElement& v
//		代入元のオブジェクト
//
//	RETURN
//	代入された結果のオブジェクトへの参照
//
//	EXCEPTIONS

Parameter::ExtraElement&
Parameter::ExtraElement::
operator =(const ExtraElement& v)
{
	switch (v.m_eType) {
	case LayoutMapEntry::Type::String:
		setString(*v.un.m_pstrValue);		break;
	case LayoutMapEntry::Type::Integer:
		setInteger(v.un.m_iValue);		break;
	case LayoutMapEntry::Type::LongLong:
		setLongLong(v.un.m_llValue);		break;
	case LayoutMapEntry::Type::Boolean:
		setBoolean(v.un.m_bValue);		break;
	case LayoutMapEntry::Type::Double:
		setDouble(v.un.m_dValue);			break;
	case LayoutMapEntry::Type::ObjectPointer:
		setObjectPointer(v.un.m_pObject);	break;
	case LayoutMapEntry::Type::FileID:
		// ★注意★
		// 代入でもObjectPointerがコピーされるだけでFileIDは同じインスタンスをさす
		setFileID(*v.un.m_pFileID);	break;
	case LayoutMapEntry::Type::None:
		clear();
		m_eType = LayoutMapEntry::Type::None;
	}

	return *this;
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::getPointer --
//		マップに格納されている値をさすポインターを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	マップに格納されている値をさすポインター
//
//	EXCEPTIONS

const void*
Parameter::ExtraElement::
getPointer() const
{
	switch (m_eType) {
	case LayoutMapEntry::Type::String:
		return un.m_pstrValue; // 文字列とFileIDだけはメンバーがポインター
	case LayoutMapEntry::Type::Integer:
		return &un.m_iValue;
	case LayoutMapEntry::Type::LongLong:
		return &un.m_llValue;
	case LayoutMapEntry::Type::Boolean:
		return &un.m_bValue;
	case LayoutMapEntry::Type::Double:
		return &un.m_dValue;
	case LayoutMapEntry::Type::ObjectPointer:
		return &un.m_pObject;
	case LayoutMapEntry::Type::FileID:
		return un.m_pFileID; // 文字列とFileIDだけはメンバーがポインター
	case LayoutMapEntry::Type::None:
		break;
	}
	return 0;
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::getPointer --
//		マップに格納される値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const void* pPointer_
//		設定する値をさすポインター
//	LogicalFile::Parameter::LayoutMapEntry::Type::Value eType_
//		設定する値の型
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Parameter::ExtraElement::
setPointer(const void* pPointer_, LayoutMapEntry::Type::Value eType_)
{
	if (!pPointer_) {
		_SYDNEY_THROW0(Exception::BadArgument);
	}
	switch (eType_) {
	case LayoutMapEntry::Type::String:
		setString(*static_cast<const ModUnicodeString*>(pPointer_)); break;
	case LayoutMapEntry::Type::Integer:
		setInteger(*static_cast<const int*>(pPointer_)); break;
	case LayoutMapEntry::Type::LongLong:
		setLongLong(*static_cast<const ModInt64*>(pPointer_)); break;
	case LayoutMapEntry::Type::Boolean:
		setBoolean(*static_cast<const bool*>(pPointer_)); break;
	case LayoutMapEntry::Type::Double:
		setDouble(*static_cast<const double*>(pPointer_)); break;
	case LayoutMapEntry::Type::ObjectPointer:
		setObjectPointer(*static_cast<const Common::Object*const*>(pPointer_)); break;
	case LayoutMapEntry::Type::FileID:
		setFileID(*static_cast<const Common::ObjectPointer<FileID>*>(pPointer_)); break;
	case LayoutMapEntry::Type::None:
		clear();
		m_eType = LayoutMapEntry::Type::None;
	}
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::setXXX --
//		マップに格納される値を設定する
//
//	NOTES
//		型が分かっている場合に使用する
//
//	ARGUMENTS
//	設定するパラメーター値
//
//	RETURN
//	なし
//
//	EXCEPTIONS

//String
void
Parameter::ExtraElement::
setString(const ModUnicodeString& cstrValue_)
{
	if (m_eType != LayoutMapEntry::Type::String) {
		clear();
		un.m_pstrValue = new ModUnicodeString(cstrValue_);
	} else {
		*un.m_pstrValue = cstrValue_;
	}
	m_eType = LayoutMapEntry::Type::String;
}

//Integer
void
Parameter::ExtraElement::
setInteger(int iValue_)
{
	clear();
	m_eType = LayoutMapEntry::Type::Integer;
	un.m_iValue = iValue_;
}

//LongLong
void
Parameter::ExtraElement::
setLongLong(ModInt64 llValue_)
{
	clear();
	m_eType = LayoutMapEntry::Type::LongLong;
	un.m_llValue = llValue_;
}

//Boolean
void
Parameter::ExtraElement::
setBoolean(bool bValue_)
{
	clear();
	m_eType = LayoutMapEntry::Type::Boolean;
	un.m_bValue = bValue_;
}

//Double
void
Parameter::ExtraElement::
setDouble(double dValue_)
{
	clear();
	m_eType = LayoutMapEntry::Type::Double;
	un.m_dValue = dValue_;
}

//Pointer to Object
void
Parameter::ExtraElement::
setObjectPointer(const Common::Object* pObject_)
{
	clear();
	m_eType = LayoutMapEntry::Type::ObjectPointer;
	un.m_pObject = pObject_;
}

//FileID
void
Parameter::ExtraElement::
setFileID(const Common::ObjectPointer<FileID>& pFileID_)
{
	if (m_eType != LayoutMapEntry::Type::FileID) {
		clear();
		un.m_pFileID = new Common::ObjectPointer<FileID>(pFileID_);
	} else {
		*un.m_pFileID = pFileID_;
	}
	m_eType = LayoutMapEntry::Type::FileID;
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::serialize --
//		マップに格納されている値をシリアル化する
//
//	NOTES
//		ObjectPointerはシリアル化の対象とならない
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Parameter::ExtraElement::
serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore()) {
		//書出し
		//タイプを書く
		int iType = m_eType;
		cArchiver_ << iType;
		//値を書く
		switch (m_eType) {
		case LayoutMapEntry::Type::String:
			cArchiver_ << *un.m_pstrValue;
			break;
		case LayoutMapEntry::Type::Integer:
			cArchiver_ << un.m_iValue;
			break;
		case LayoutMapEntry::Type::LongLong:
			cArchiver_ << un.m_llValue;
			break;
		case LayoutMapEntry::Type::Boolean:
			{
				int v = (un.m_bValue==true)?1:0;
				cArchiver_ << v;
				break;
			}
		case LayoutMapEntry::Type::Double:
			cArchiver_ << un.m_dValue;
			break;
		case LayoutMapEntry::Type::FileID:
			{
				int tmp = ((*un.m_pFileID).get() != 0)?1:0;
				cArchiver_ << tmp;
				if (tmp) {
					cArchiver_ << **un.m_pFileID;
				}
				break;
			}
		case LayoutMapEntry::Type::None:
		case LayoutMapEntry::Type::ObjectPointer:
			break;
		}
	}
	else
	{
		//読込み
		//タイプを読む
		int iType;
		cArchiver_ >> iType;
		//値を読む
		switch (iType)
		{
		case LayoutMapEntry::Type::String:
			{
				ModUnicodeString cstrValue;
				cArchiver_ >> cstrValue;
				setString(cstrValue);
				break;
			}
		case LayoutMapEntry::Type::Integer:
			{
				int iValue;
				cArchiver_ >> iValue;
				setInteger(iValue);
				break;
			}
		case LayoutMapEntry::Type::LongLong:
			{
				ModInt64 llValue;
				cArchiver_ >> llValue;
				setLongLong(llValue);
				break;
			}
		case LayoutMapEntry::Type::Boolean:
			{
				int v;
				cArchiver_ >> v;
				setBoolean((v==1)?true:false);
				break;
			}
		case LayoutMapEntry::Type::Double:
			{
				double dValue;
				cArchiver_ >> dValue;
				setDouble(dValue);
				break;
			}
		case LayoutMapEntry::Type::FileID:
			{
				int tmp;
				Common::ObjectPointer<FileID> pFileID;
				cArchiver_ >> tmp;
				if (tmp) {
					pFileID = new FileID;
					cArchiver_ >> *pFileID;
				}
				setFileID(pFileID);
				break;
			}
		case LayoutMapEntry::Type::None:
			break;
		case LayoutMapEntry::Type::ObjectPointer:
			setObjectPointer(0);
			break;
		}
	}
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::toString --
//		マップに格納されている値を文字列化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUnicodeOstrStream& cStream_
//		対象のストリーム
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Parameter::ExtraElement::
toString(ModUnicodeOstrStream& cStream_) const
{
	cStream_ << "Type:";
	switch (m_eType) {
	case LayoutMapEntry::Type::String:
		cStream_ << "String";
		cStream_ << ",Value:";
		cStream_ << *un.m_pstrValue;
		break;
	case LayoutMapEntry::Type::Integer:
		cStream_ << "Integer";
		cStream_ << ",Value:";
		cStream_ << un.m_iValue;
		break;
	case LayoutMapEntry::Type::LongLong:
		cStream_ << "LongLong";
		cStream_ << ",Value:";
		cStream_ << un.m_llValue;
		break;
	case LayoutMapEntry::Type::Boolean:
		cStream_ << "Boolean";
		cStream_ << ",Value:";
		cStream_ << un.m_bValue;
		break;
	case LayoutMapEntry::Type::Double:
		cStream_ << "Double";
		cStream_ << ",Value:";
		cStream_ << un.m_dValue;
		break;
	case LayoutMapEntry::Type::ObjectPointer:
		cStream_ << "ObjectPointer";
		cStream_ << ",Value:";
		if (un.m_pObject)
			cStream_ << *un.m_pObject;
		else
			cStream_ << "null";
		break;
	case LayoutMapEntry::Type::FileID:
		cStream_ << "FileID";
		cStream_ << ",Value:";
		if ((*un.m_pFileID).get())
			cStream_ << **un.m_pFileID;
		else
			cStream_ << "null";
		break;
	case LayoutMapEntry::Type::None:
		cStream_ << "None";
		break;
	}
}

//	FUNCTION
//	LogicalFile::Parameter::ExtraElement::clear --
//		マップに格納されている値を破棄する
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

void
Parameter::ExtraElement::
clear()
{
	switch (m_eType) {
	case LayoutMapEntry::Type::String:
		delete un.m_pstrValue;
		un.m_pstrValue = 0;
		break;
	case LayoutMapEntry::Type::FileID:
		delete un.m_pFileID;
		un.m_pFileID = 0;
		break;
	}
	m_eType = LayoutMapEntry::Type::None;
}



//	FUNCTION
//	LogicalFile::Parameter::Base::getString --
//		文字列型のパラメーター値を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::Key& cKey_
//		キー
//
//	RETURN
//	パラメーター値
//	設定されていない場合は空の値が返る
//
//	EXCEPTIONS

ModUnicodeString
Parameter::Base::
getString(const Key& cKey_) const
{
	ModUnicodeString cstrResult;
	(void)getString(cKey_, cstrResult);
	return cstrResult;
}

//	FUNCTION
//	LogicalFile::Parameter::Base::getString --
//		文字列型のパラメーター値を得る
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::Key& cKey_
//		キー
//	ModUnicodeString& cstrValue_
//		パラメーター値を設定する変数への参照
//
//	RETURN
//	パラメーター値が設定されている場合はtrue、
//	設定されていない場合はfalse
//
//	EXCEPTIONS

bool
Parameter::Base::
getString(const Key& cKey_,
		  ModUnicodeString& cstrValue_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>::getValue(*this, cKey_, cstrValue_);
}

//	FUNCTION
//	LogicalFile::Parameter::Base::getString --
//		文字列型のパラメーター値を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::Key& cKey_
//		キー
//	const ModUnicodeString& cstrValue_
//		パラメーター値を設定する変数への参照
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Parameter::Base::
setString(const Key& cKey_,
		  const ModUnicodeString& cstrValue_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>::setValue(*this, cKey_, cstrValue_);
}

//////////////////////////////////////////////////////////////////////////////
// 以下はパラメーター値の型が異なるだけで仕様は同じなのでコメントを割愛する //
//////////////////////////////////////////////////////////////////////////////

//Integer
int
Parameter::Base::
getInteger(const Key& cKey_) const
{
	int iResult = 0;
	(void)getInteger(cKey_, iResult);
	return iResult;
}

bool
Parameter::Base::
getInteger(const Key& cKey_, int& iValue_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<int, LayoutMapEntry::Type::Integer>::getValue(*this, cKey_, iValue_);
}

void
Parameter::Base::
setInteger(const Key& cKey_, int iValue_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<int, LayoutMapEntry::Type::Integer>::setValue(*this, cKey_, iValue_);
}

//LongLong
ModInt64
Parameter::Base::
getLongLong(const Key& cKey_) const
{
	ModInt64 llResult = 0;
	(void)getLongLong(cKey_, llResult);
	return llResult;
}

bool
Parameter::Base::
getLongLong(const Key& cKey_, ModInt64& llValue_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>::getValue(*this, cKey_, llValue_);
}
void
Parameter::Base::
setLongLong(const Key& cKey_, ModInt64 llValue_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>::setValue(*this, cKey_, llValue_);
}

//Boolean
bool
Parameter::Base::
getBoolean(const Key& cKey_) const
{
	bool bResult = false;
	(void)getBoolean(cKey_, bResult);
	return bResult;
}

bool
Parameter::Base::
getBoolean(const Key& cKey_, bool& bValue_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<bool, LayoutMapEntry::Type::Boolean>::getValue(*this, cKey_, bValue_);
}

void
Parameter::Base::
setBoolean(const Key& cKey_, bool bValue_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<bool, LayoutMapEntry::Type::Boolean>::setValue(*this, cKey_, bValue_);
}

//Double
double
Parameter::Base::
getDouble(const Key& cKey_) const
{
	double dblResult = 0;
	(void)getDouble(cKey_, dblResult);
	return dblResult;
}

bool
Parameter::Base::
getDouble(const Key& cKey_, double& dValue_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<double, LayoutMapEntry::Type::Double>::getValue(*this, cKey_, dValue_);
}

void
Parameter::Base::
setDouble(const Key& cKey_, double dValue_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<double, LayoutMapEntry::Type::Double>::setValue(*this, cKey_, dValue_);
}

//Pointer to Object
Common::Object*
Parameter::Base::
getObjectPointer(const Key& cKey_) const
{
	const Common::Object* pResult = 0;
	(void)getObjectPointer(cKey_, pResult);
	return const_cast<Common::Object*>(pResult);
}

bool
Parameter::Base::
getObjectPointer(const Key& cKey_,
				 const Common::Object*& pObject_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>::getValue(*this, cKey_, pObject_);
}

void
Parameter::Base::
setObjectPointer(const Key& cKey_,
				 const Common::Object* pObject_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>::setValue(*this, cKey_, pObject_);
}

//FileID
Common::ObjectPointer<FileID>
Parameter::Base::
getFileID(const Key& cKey_) const
{
	Common::ObjectPointer<FileID> pResult;
	(void)ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::getValue(*this, cKey_, pResult);
	return pResult;
}

bool
Parameter::Base::
getFileID(const Key& cKey_, Common::ObjectPointer<FileID>& pFileID_) const
{
	Os::AutoCriticalSection l(m_cLatch);
	return ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::getValue(*this, cKey_, pFileID_);
}

void
Parameter::Base::
setFileID(const Key& cKey_, const Common::ObjectPointer<FileID>& pFileID_)
{
	Os::AutoCriticalSection l(m_cLatch);
	ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::setValue(*this, cKey_, pFileID_);
}

//////////////////////////////////////////////////////////////////////////////
// 以上はパラメーター値の型が異なるだけで仕様は同じなのでコメントを割愛した //
//////////////////////////////////////////////////////////////////////////////

//	FUNCTION
//	LogicalFile::Parameter::Base::clear --
//		すべてのパラメーター値を破棄する
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

void
Parameter::Base::
clear()
{
	Os::AutoCriticalSection l(m_cLatch);
	ModSize n = m_pMap->getSize();
	for (ModSize i = 0; i < n; ++i) {
		const LayoutMapEntry& cEntry = (*m_pMap)[i];
		switch (cEntry.m_eType) {
		case LayoutMapEntry::Type::String:
			ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>::deleteValue(*this, cEntry);
			break;
		case LayoutMapEntry::Type::Integer:
			ValueExtractor<int, LayoutMapEntry::Type::Integer>::deleteValue(*this, cEntry);
			break;
		case LayoutMapEntry::Type::LongLong:
			ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>::deleteValue(*this, cEntry);
			break;
		case LayoutMapEntry::Type::Boolean:
			ValueExtractor<bool, LayoutMapEntry::Type::Boolean>::deleteValue(*this, cEntry);
			break;
		case LayoutMapEntry::Type::Double:
			ValueExtractor<double, LayoutMapEntry::Type::Double>::deleteValue(*this, cEntry);
			break;
		case LayoutMapEntry::Type::ObjectPointer:
			ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>::deleteValue(*this, cEntry);
			break;
		case LayoutMapEntry::Type::FileID:
			ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::deleteValue(*this, cEntry);
			break;
		default:
			break;
		}
		_setValue<void>(this, cEntry.m_iOffset, 0);
	}

	m_cExtraValues.clear();
}

//	FUNCTION
//	LogicalFile::Parameter::Base::serialize --
//		すべてのパラメーター値をシリアル化する
//
//	NOTES
//		ObjectPointer型のパラメーターはシリアル化の対象とならない
//
//	ARGUMENTS
//	ModArchive& cArchiver_
//		対象のアーカイバー
//
//	RETURN
//	なし
//
//	EXCEPTIONS
void
Parameter::Base::
serialize(ModArchive& cArchiver_)
{
	Os::AutoCriticalSection l(m_cLatch);

	// バージョン番号を格納または取得する
	cArchiver_(m_iVersion);
	if (cArchiver_.isLoad()) {
		setLayoutMap(m_iVersion);
		clear();
	}
	ModSize n = m_pMap->getSize();
	for (ModSize i = 0; i < n; ++i) {
		const LayoutMapEntry& cEntry = (*m_pMap)[i];
		switch (cEntry.m_eType) {
		case LayoutMapEntry::Type::String:
			ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>::serialize(*this, cEntry, i, cArchiver_);
			break;
		case LayoutMapEntry::Type::Integer:
			ValueExtractor<int, LayoutMapEntry::Type::Integer>::serialize(*this, cEntry, i, cArchiver_);
			break;
		case LayoutMapEntry::Type::LongLong:
			ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>::serialize(*this, cEntry, i, cArchiver_);
			break;
		case LayoutMapEntry::Type::Boolean:
			ValueExtractor<bool, LayoutMapEntry::Type::Boolean>::serialize(*this, cEntry, i, cArchiver_);
			break;
		case LayoutMapEntry::Type::Double:
			ValueExtractor<double, LayoutMapEntry::Type::Double>::serialize(*this, cEntry, i, cArchiver_);
			break;
		case LayoutMapEntry::Type::FileID:
			ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::serialize(*this, cEntry, i, cArchiver_);
			break;
		case LayoutMapEntry::Type::ObjectPointer:
			// シリアライズできない
			break;
		default:
			break;
		}
	}

	// レイアウトマップを超えるパラメーター値のシリアル化
	if (cArchiver_.isLoad()) {
		// 読み出し
		//総数を得る
		int number;
		cArchiver_ >> number;

		ExtraValues::ValueType	v;
		for (int i = 0 ; i < number; ++i) {
			//キーと値を得てHashMapに入れる
			cArchiver_ >> v.first;
			cArchiver_ >> v.second;
			m_cExtraValues.insert(v, ModTrue /* NoDuplicationCheck */);
		}
	} else {
		// 書き出し
		//総数を書出す
		int number = m_cExtraValues.getSize();
		cArchiver_ << number;
		for (ExtraValues::Iterator i = m_cExtraValues.begin();
			 i != m_cExtraValues.end(); ++i) {
			//キーと値を書く
			cArchiver_ << (*i).first;
			cArchiver_ << (*i).second;
		}
	}
}

//	FUNCTION
//	LogicalFile::Parameter::Base::~Base --
//		デストラクター
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

//virtual
Parameter::Base::
~Base()
{
	clear();
}

//	FUNCTION protected
//	LogicalFile::Parameter::Base::Base --
//		コンストラクター
//
//	NOTES
//		サブクラスからしか呼ばれない
//
//	ARGUMENTS
//	int iVersion_
//		レイアウトのバージョン番号
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Parameter::Base::
Base()
	: m_iVersion(-1), m_pMap(0), m_cExtraValues(_iExtraValuesSize)
{}

//	FUNCTION protected
//	LogicalFile::Parameter::Base::Base --
//		コピーコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::Base& cOther_
//		コピー元のオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Parameter::Base::
Base(const Base& cOther_)
	: m_iVersion(cOther_.m_iVersion), m_pMap(cOther_.m_pMap),
	  m_cExtraValues(_iExtraValuesSize)
{
	//すべてのエントリを初期化する
	ModSize n = m_pMap->getSize();
	for (ModSize i = 0; i < n; ++i) {
		_setValue<void>(this, (*m_pMap)[i].m_iOffset, 0);
	}
	//メンバーをコピーする
	*this = cOther_;
}

void
Parameter::Base::
setLayoutMap(int iVersion_)
{
	m_iVersion = iVersion_;
	m_pMap = getLayoutMap(iVersion_);

	//すべてのエントリを初期化する
	ModSize n = m_pMap->getSize();
	for (ModSize i = 0; i < n; ++i) {
		_setValue<void>(this, (*m_pMap)[i].m_iOffset, 0);
	}
}

//	FUNCTION protected
//	LogicalFile::Parameter::Base::operator= --
//		代入オペレーター
//
//	NOTES
//
//	ARGUMENTS
//	const LogicalFile::Parameter::Base& cOther_
//	   代入元のオブジェクト
//
//	RETURN
//	なし
//
//	EXCEPTIONS

Parameter::Base&
Parameter::Base::
operator =(const Base& cOther_)
{
	if (this != &cOther_) {
		// コピー先とコピー元の両方について排他する必要がある
		// ★注意★
		// コピー先とコピー元を互い違いに同時にコピーするとデッドロックに陥る場合があるが
		// それを同時に行うことは考えられない
		Os::AutoCriticalSection l1(m_cLatch);
		{ // デストラクトの順序を保証するためのスコープ
		Os::AutoCriticalSection l2(cOther_.m_cLatch);

		if (m_iVersion != cOther_.m_iVersion) {
			//バージョンが異なる場合、cOther_のバージョンに合わせる
			clear();
			setLayoutMap(m_iVersion = cOther_.m_iVersion);
		}
		ModSize n = m_pMap->getSize();
		for (ModSize i = 0; i < n; ++i) {
			const LayoutMapEntry& cEntry = (*m_pMap)[i];
			switch (cEntry.m_eType) {
			case LayoutMapEntry::Type::String:
				ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>::copyValue(*this, cOther_, cEntry);
				break;
			case LayoutMapEntry::Type::Integer:
				ValueExtractor<int, LayoutMapEntry::Type::Integer>::copyValue(*this, cOther_, cEntry);
				break;
			case LayoutMapEntry::Type::LongLong:
				ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>::copyValue(*this, cOther_, cEntry);
				break;
			case LayoutMapEntry::Type::Boolean:
				ValueExtractor<bool, LayoutMapEntry::Type::Boolean>::copyValue(*this, cOther_, cEntry);
				break;
			case LayoutMapEntry::Type::Double:
				ValueExtractor<double, LayoutMapEntry::Type::Double>::copyValue(*this, cOther_, cEntry);
				break;
			case LayoutMapEntry::Type::ObjectPointer:
				ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>::copyValue(*this, cOther_, cEntry);
				break;
			case LayoutMapEntry::Type::FileID:
				ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::copyValue(*this, cOther_, cEntry);
				break;
			default:
				break;
			}
		}
		m_cExtraValues.clear();
		m_cExtraValues.insert(cOther_.m_cExtraValues.begin(), cOther_.m_cExtraValues.end(),
							  ModTrue /* NoDuplicationCheck */);
		} // デストラクトの順序を保証するためのスコープ終了
	}
	return *this;
}

//	FUNCTION protected
//	LogicalFile::Parameter::Base::toString --
//		文字列にする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	パラメーター値を並べた文字列
//
//	EXCEPTIONS

ModUnicodeString
Parameter::Base::
toString() const
{
	Os::AutoCriticalSection l(m_cLatch);

	ModUnicodeOstrStream o;

	o << "LogicalFile::Parameter : ";

	ModSize n = m_pMap->getSize();
	for (ModSize i = 0; i < n; ++i) {
		o << "(Key:" << i << ",Type:";
		const LayoutMapEntry& cEntry = (*m_pMap)[i];
		switch (cEntry.m_eType) {
		case LayoutMapEntry::Type::String:
			o << "String";
			o << ",Value:";
			ValueExtractor<ModUnicodeString, LayoutMapEntry::Type::String>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::Integer:
			o << "Integer";
			o << ",Value:";
			ValueExtractor<int, LayoutMapEntry::Type::Integer>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::LongLong:
			o << "LongLong";
			o << ",Value:";
			ValueExtractor<ModInt64, LayoutMapEntry::Type::LongLong>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::Boolean:
			o << "Boolean";
			o << ",Value:";
			ValueExtractor<bool, LayoutMapEntry::Type::Boolean>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::Double:
			o << "Double";
			o << ",Value:";
			ValueExtractor<double, LayoutMapEntry::Type::Double>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::ObjectPointer:
			o << "ObjectPointer";
			o << ",Value:";
			ValueExtractor<const Common::Object*, LayoutMapEntry::Type::ObjectPointer>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::FileID:
			o << "FileID";
			o << ",Value:";
			ValueExtractor<Common::ObjectPointer<FileID>, LayoutMapEntry::Type::FileID>::toString(*this, cEntry, o);
			break;
		case LayoutMapEntry::Type::None:
		default:
			o << "None";
			break;
		}
		o << ")";
	}
	{
		for (ExtraValues::ConstIterator i = m_cExtraValues.begin();
			 i != m_cExtraValues.end(); ++i) {
			//キーと値を書く
			o << "(Key:";
			(*i).first.toString(o);
			o << ",";
			(*i).second.toString(o);
			o << ")";
		}
	}

	return ModUnicodeString(o.getString());
}

// FUNCTION public
//	LogicalFile::Parameter::Base::getVersion -- バージョン番号を得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	int
//
// EXCEPTIONS

int
Parameter::Base::
getVersion() const
{
	return m_iVersion;
}

// 初期化配列からレイアウトマップを作る
//static
void
Parameter::Base::
assignMap(LayoutMap& cMap_, int iMaxKey_,
		  const ModPair<int, LayoutMapEntry>* pSpec_,
		  const ModPair<int, LayoutMapEntry>* pLast_)
{
	// まずはKeyの数だけデフォルト値を埋める
	cMap_.assign(iMaxKey_);
	// レイアウト指定にしたがって上書きする
	while (pSpec_ != pLast_) {
		; _SYDNEY_ASSERT(pSpec_->first >= 0);
		; _SYDNEY_ASSERT(pSpec_->first < iMaxKey_);
		cMap_[pSpec_->first] = pSpec_->second;
		++pSpec_;
	}
}

//
//	Copyright (c) 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
