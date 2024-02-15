// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Bitset.cpp -- Bitset の定義ファイル
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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
#include "LibUna/Bitset.h"

_UNA_USING

//
// FUNCTION
//	Bitset::Bitset -- コンストラクタ
//
// NOTES
//	Bitsetクラスのコンストラクタ。引数にBitsetサイズを指定してインスタンスを初期化する。
//	引数を省略した場合は、32bitをデフォルトサイズとして初期化する。
//	各bitの初期値はModFalse。
//
// ARGUMENTS
//	unsigned size_
//		このインスタンスで管理するビット数。
//	ModBoolean = ModFalse
//		初期化ビット
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Bitset::Bitset(unsigned size_, ModBoolean bit_) : _bitsize(size_){

	// 必要なユニットサイズを求める。
	int unitsize = getUnitSize(size_);

	// データ領域を確保する。
	_bitdata = new Unit[unitsize];

	// データ領域を bit_ で初期化する。
	memset(_bitdata, bit_, sizeof(Unit) * unitsize);
}

//
// FUNCTION
//	Bitset::Bitset -- コンストラクタ
//
// NOTES
//	コピーコンストラクタ
//
// ARGUMENTS
//	const Bitset& src_
//		元になるBitsetオブジェクト
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Bitset::Bitset(const Bitset& src_) : _bitsize(src_._bitsize){

 	// 必要なユニットサイズを求める。
	Index unitsize = src_.getUnitSize();

	// データ領域を確保する。
	_bitdata = new Unit[unitsize];

	// データ領域を引数のBitsetインスタンスの値で初期化する。
	for(Index idx=0;idx<unitsize;idx++){
		_bitdata[idx] = src_._bitdata[idx];
	}
}

//
// FUNCTION
//	Bitset::~Bitset -- デストラクタ
//
// NOTES
//	2進値データ領域を開放する。
//
// ARGUMENTS
//
// RETURN
//	なし
//
// EXCEPTIONS
//
Bitset::~Bitset(){
	delete [] _bitdata;
}

//
// FUNCTION
//	Bitset::set -- 位置pos_のビット値をbit_で指定した値に設定する。
//
// NOTES
//	位置pos_のビット値をbit_で指定した値に設定する。
//	bit_を省略した場合は、ModTrueに設定する。
//
// ARGUMENTS
//	Pos pos_
//		ビット位置
//	ModBoolean bit_
//		設定するModBoolean値
//
// RETURN
//	Bitset&
//		*this
//
// EXCEPTIONS
//	OutOfRange
//		pos_< 0 又は pos >= _bitsize の場合に例外を送出する。
//
Bitset&
Bitset::set(Pos pos_, ModBoolean bit_){

	// pos_が有効な範囲の値かをチェックする。
	if(pos_ < 0 || pos_ >= _bitsize)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);

	// インデックスとマスクを取得する。
	Index index;
	Unit mask;
	getIndex(pos_, index, mask);

	// 値を設定する。
	if ( bit_ == ModTrue) {
		_bitdata[index] |= mask;
	} else {
		_bitdata[index] &= ~mask;
	}

	return (*this);
}

//
// FUNCTION
//	Bitset::test -- 位置pos_のビット値を返す。
//
// NOTES
//
// ARGUMENTS
//	Pos pos_
//		値を知りたいbit位置
//
// RETURN
//	ModBoolean
//		指定bit位置の値
//
// EXCEPTIONS
//	OutOfRange
//		pos_< 0 又は pos >= _bitsize の場合に例外を送出する。
//
ModBoolean
Bitset::test(Pos pos_) const{

	// pos_が有効な範囲の値かをチェックする。
	if(pos_ < 0 || pos_ >= _bitsize)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);

	// インデックスとマスクを取得する。
	Index index;
	Unit mask;
	getIndex(pos_, index, mask);

	// 値を取得する。
	return ((_bitdata[index] & mask) == mask ? ModTrue : ModFalse);
}

//
//	FUNCTION
//		Bitset::test -- 範囲指定のビットチェック
//
//	NOTES
//		開始位置から終了位置まで check_ が続いているか [min_, max_) の範囲で確認する
//
//	ARGUMENTS
//		Pos min_
//			ビットチェック開始位置
//		Pos max_
//			ビットチェック終了位置
//		ModBoolean check_ = ModTrue
//			チェックする連続した値
//
// RETURN
//		ModBoolean
//			check_ が続いている場合、ModTrue
//					 続いていない場合、ModFalse
//
// EXCEPTIONS
//		OutOfRange
//			pos_< 0 又は pos >= _bitsize の場合に例外を送出する。
//
#ifdef OBSOLETE
ModBoolean
Bitset::test(Pos min_, Pos max_, ModBoolean check_) const
{
	// pos_が有効な範囲の値かをチェックする。
	if(min_ < 0 || min_ >= _bitsize || max_ < 0 || max_ >= _bitsize || min_ > max_)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);

ConstIterator it  = begin() + min_;
	ConstIterator fin = begin() + max_;
	for ( ; it != fin; ++it ) {
		if ( *it != check_ )
			return ModFalse;
	}
	return ModTrue;
}

//
//	FUNCTION
//		Bitset::testCount -- 範囲指定のビットカウンタ
//
//	NOTES
//		開始位置から終了位置まで check_ 状態の数を数える
//
//	ARGUMENTS
//		Pos min_
//			ビットチェック開始位置
//		Pos max_
//			ビットチェック終了位置
//		ModBoolean check_ = ModTrue
//			チェックする値
//
// RETURN
//		unsigned int
//			check_ 状態のフラグ数
//
// EXCEPTIONS
//		OutOfRange
//			pos_< 0 又は pos >= _bitsize の場合に例外を送出する。
//
unsigned int
Bitset::testCount(Pos min_, Pos max_, ModBoolean check_) const
{
	// pos_が有効な範囲の値かをチェックする。
	if(min_ < 0 || min_ >= _bitsize || max_ < 0 || max_ > _bitsize || min_ > max_)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);

	unsigned int ret = 0;
	ConstIterator it  = begin() + min_;
	ConstIterator fin = begin() + max_;
	for ( ; it != fin; ++it )
		if ( *it == check_ )
			ret++;
	return ret;
}
#endif

//
// FUNCTION
//	Bitset::operator[] -- インデックスを指定するためのオペレータ。(非const版)
//
// NOTES
//
// ARGUMENTS
//	Pos pos_
//		値を知りたいbit位置
//
// RETURN
//	Bitset::reference
//		指定bitへの参照を保持するreferenceクラスのインスタンス。
//
// EXCEPTIONS
//	OutOfRange
//		pos_< 0 又は pos >= _bitsize の場合に例外を送出する。
//
Bitset::reference
Bitset::operator[](Pos pos_){

	// pos_が有効な範囲の値かをチェックする。
	if(pos_ < 0 || pos_ >= _bitsize)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);

	return reference(*this, pos_);
}

//
// FUNCTION
//	Bitset::operator[] -- インデックスを指定するためのオペレータ。(const版)
//
// NOTES
//
// ARGUMENTS
//	Pos pos_
//		値を知りたいbit位置
//
// RETURN
//	ModBoolean
//		指定bitの値。
//
// EXCEPTIONS
//	OutOfRange
//		pos_< 0 又は pos >= _bitsize の場合に例外を送出する。
//
ModBoolean
Bitset::operator[](Pos pos_) const{

	// pos_が有効な範囲の値かをチェックする。
	if(pos_ < 0 || pos_ >= _bitsize)
		ModThrow(ModModuleStandard,
				 ModCommonErrorBadArgument,
				 ModErrorLevelError);

	return test(pos_);
}

//
// FUNCTION
//	Bitset::count -- ModTrueとなっているビット数取得
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	unsigned
//		ModTrueとなっているビット数
//
// EXCEPTIONS
//
unsigned
Bitset::count(void) const{

 	// ModTrue となっているビット数
	unsigned bitCount = 0;

	// ユニットインデックスの最大値を求める。
	Index maxIndex = getUnitSize(_bitsize);

	for(Index idx=0;idx<maxIndex;idx++){
		Unit mask = 1;
		for(int i=0;i<sizeof(Unit) * ByteWidth;i++){
			bitCount += ((_bitdata[idx] & mask) / mask);
			mask <<= 1;
		}
	}

	return bitCount;
}

//
// FUNCTION
//	Bitset::size -- インスタンスの管理するbitサイズを返す。
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	unsigned
//		インスタンスの管理するbitサイズ
//
// EXCEPTIONS
//
unsigned
Bitset::size(void) const{
	return _bitsize;
}

//
// FUNCTION
//	Bitset::operator &= -- 右辺値のBitsetインスタンスと &= 演算を行う。
//
// NOTES
//	右辺値のBitsetインスタンスと &= 演算を行う。
//	Bitsetサイズ(ユニットサイズ)が異なる場合は、重なる部分では &= 演算を行い、
//	残りのbitは長いほうの値を採用する。
//	右辺値のbitsetインスタンスの方がサイズが大きかった場合は、
//	左辺値のbit情報を保持した右辺値の長さのインスタンスを作成し、
//	それと右辺値とで &= 演算を行う。
//	元の左辺値インスタンスは破棄する。
//
// ARGUMENTS
//	const Bitset& rhs_
//		右辺値のBitset
//
// RETURN
//	Bitset&
//		演算結果のBitsetへの参照
//
// EXCEPTIONS
//
Bitset&
Bitset::operator &=(const Bitset& rhs_){
	Index index;
	Unit mask = 0;

	// インデックスとマスクを取得する。
	// ここで取得するマスクは，getIndex関数の第一引数がユニット中のどこにあるかを示すマス
	// ク(位置マスク)であり、ユニット中の一箇所だけビットが立つものである。ここで欲しいの
	// は、立っているビットより下位のビットが全て1となっているマスクである。そのため、
	// getIndexでマスク取得後、マスクを書き換える。
	
	// getIndexの第一引数についての補足
	// rhs_._bitsize = n のとき、rhs_._bitdataは、0  (n-1) までの値をとる。
	// よって、第一引数には、rhs_._bitdataのとりうる最大値(rhs_._bitsize - 1) を渡す。
	getIndex((_bitsize < rhs_._bitsize ? _bitsize : rhs_._bitsize) - 1, index, mask);

	// 取得したマスクを、立っているビットより下位のビットが全て1となるように書き換える。
	// ただし、マスクが 0 の場合は 0 のままにして書き換えない。
	if(mask != 0)
		while((mask&1)== 0)	mask |= (mask >> 1);

	if(_bitsize < rhs_._bitsize){
		// 右辺値のサイズが大きい場合。
		// データ領域を拡張する。
		expand(rhs_._bitsize - _bitsize);

		// &= 演算を行う
		for(Index idx=0;idx<index;idx++){
			_bitdata[idx] &= rhs_._bitdata[idx];
		}
		_bitdata[index] = (~_bitdata[index] ^ mask) & rhs_._bitdata[index];

	}else{
		// 右辺値のサイズが小さいか、左辺値のサイズと等しい場合
		// &= 演算を行う
		for(Index idx=0;idx<index;idx++){
			_bitdata[idx] &= rhs_._bitdata[idx];
		}
		_bitdata[index] &= (~rhs_._bitdata[index] ^ mask);
	}

	return (*this);
}

//
// FUNCTION
//	Bitset::operator |= -- 右辺値のBitsetインスタンスと |= 演算を行う。
//
// NOTES
//	右辺値のBitsetインスタンスと |= 演算を行う。
//	Bitsetサイズ(ユニットサイズ)が異なる場合は、重なる部分では |= 演算を行い、
//	残りのbitは長いほうの値を採用する。
//	右辺値のbitsetインスタンスの方がサイズが大きかった場合は、
//	左辺値のbit情報を保持した右辺値の長さのインスタンスを作成し、
//	それと右辺値とで |= 演算を行う。
//	元の左辺値インスタンスは破棄する。
//
// ARGUMENTS
//	const Bitset& rhs_
//		右辺値のBitset
//
// RETURN
//	Bitset&
//		演算結果のBitsetへの参照
//
// EXCEPTIONS
//
Bitset&
Bitset::operator |=(const Bitset& rhs_)
{
	// 短いほうのサイズを取得する
	Index shortIndex;
	Unit mask = 0;
	getIndex((_bitsize < rhs_._bitsize ? _bitsize : rhs_._bitsize) - 1, shortIndex, mask);

	// 長いほうのサイズを取得する
	Index longIndex;
	getIndex((_bitsize < rhs_._bitsize ? rhs_._bitsize : _bitsize) - 1, longIndex, mask);

	if(_bitsize < rhs_._bitsize){
		// 右辺値のサイズが大きい場合。
		// データ領域を拡張する。
		expand(rhs_._bitsize - _bitsize);

		// |= 演算を行う
		Index idx;
		for ( idx = 0; idx < shortIndex; ++idx )
			_bitdata[idx] |= rhs_._bitdata[idx];

		// 残りを加える
		for ( ; idx < longIndex; ++idx )
			_bitdata[idx] = rhs_._bitdata[idx];

	}else{
		// 右辺値のサイズが小さいか、左辺値のサイズと等しい場合
		// |= 演算を行う
		for ( Index idx = 0; idx <= shortIndex; ++idx )
			_bitdata[idx] |= rhs_._bitdata[idx];
	}

	return (*this);
}

//
// FUNCTION
//	Bitset::operator ^= -- 右辺値のBitsetインスタンスと ^= 演算を行う。
//
// NOTES
//	右辺値のBitsetインスタンスと ^= 演算を行う。
//	Bitsetサイズ(ユニットサイズ)が異なる場合は、重なる部分では ^= 演算を行い、
//	残りのbitは長いほうの値を採用する。
//	右辺値のbitsetインスタンスの方がサイズが大きかった場合は、
//	左辺値のbit情報を保持した右辺値の長さのインスタンスを作成し、
//	それと右辺値とで |= 演算を行う。
//	元の左辺値インスタンスは破棄する。
//
// ARGUMENTS
//	const Bitset& rhs_
//		右辺値のBitset
//
// RETURN
//	Bitset&
//		演算結果のBitsetへの参照
//
// EXCEPTIONS
//
Bitset&
Bitset::operator ^=(const Bitset& rhs_)
{
	// 短いほうのサイズを取得する
	Index shortIndex;
	Unit mask = 0;
	getIndex((_bitsize < rhs_._bitsize ? _bitsize : rhs_._bitsize) - 1, shortIndex, mask);

	// 長いほうのサイズを取得する
	Index longIndex;
	getIndex((_bitsize < rhs_._bitsize ? rhs_._bitsize : _bitsize) - 1, longIndex, mask);

	if(_bitsize < rhs_._bitsize){
		// 右辺値のサイズが大きい場合。
		// データ領域を拡張する。
		expand(rhs_._bitsize - _bitsize);

		// ^= 演算を行う
		Index idx;
		for ( idx = 0; idx < shortIndex; ++idx )
			_bitdata[idx] ^= rhs_._bitdata[idx];

		// 残りを加える
		for ( ; idx < longIndex; ++idx )
			_bitdata[idx] = rhs_._bitdata[idx];

	}else{
		// 右辺値のサイズが小さいか、左辺値のサイズと等しい場合
		// ^= 演算を行う
		for ( Index idx = 0; idx <= shortIndex; ++idx)
			_bitdata[idx] ^= rhs_._bitdata[idx];

	}

	return (*this);
}

//
// FUNCTION
//	Bitset::operator = -- = 演算子
//
// NOTES
//
// ARGUMENTS
//	const Bitset& rhs_
//		右辺値のBitset
//
// RETURN
//	Bitset&
//		演算結果のBitsetへの参照
//
// EXCEPTIONS
//
Bitset&
Bitset::operator = (const Bitset& rhs_)
{
	if ( this != &rhs_ ) {

		// ビットサイズの更新
		_bitsize = rhs_._bitsize;

		// 必要なユニットサイズを求める。
		int unitsize = rhs_.getUnitSize();

		// データ領域を確保する。
		delete _bitdata;
		_bitdata = new Unit[unitsize];

		// データ領域をコピーする
		ModOsDriver::Memory::copy(_bitdata, rhs_._bitdata, sizeof(Unit) * unitsize);
	}
	return *this;
}

//
// FUNCTION
//	Bitset::operator ~ -- ~ 演算子
//
// NOTES
//
// ARGUMENTS
//		なし
//
// RETURN
//	Bitset&
//		演算結果のBitsetへの参照
//
// EXCEPTIONS
//
Bitset&
Bitset::operator ~()
{
	unsigned int max = getUnitSize();

	for ( unsigned int i = 0; i < max; ++i )
		_bitdata[i] = ~_bitdata[i];

	return (*this);
}

//
// FUNCTION
//	Bitset::getIndex -- マスクを取得する。
//
// NOTES
//	pos_で指定したビット位置のユニットインデックスとマスクを、
//	それぞれ参照引数 Index& dst_, Unit& mask_ に書き込む。
//
// ARGUMENTS
//	const Pos pos_
//		ビット位置
//	const Index& idx_
//		ユニットインデックスの参照
//	const Unit& mask_
//		マスクの参照
//
// RETURN
//	なし
//
// EXCEPTIONS
//
void
Bitset::getIndex(const Pos pos_, Index& idx_, Unit& mask_) const{
	// ユニットインデックスの取得
	idx_ = pos_ / (sizeof(Unit) * ByteWidth);

	// マスク取得
	mask_ = 1 << (pos_ % (sizeof(Unit) * ByteWidth));
}

//
// FUNCTION
//	Bitset::getUnitSize -- ユニットインデックスを計算する。
//
// NOTES
//	ビットサイズから1ユニットのビット数を勘案してユニットのサイズを求めるヘルパーメソッド
//	1ユニットのビット数は、sizeof(Unit) * ByteWidth で定義される。
//
// ARGUMENTS
//	const unsigned& bsize_
//		ビットサイズ
//
// RETURN
//	Bitset::Index
//		ビットサイズのビット列を格納するためのユニットサイズ
//
// EXCEPTIONS
//
Bitset::Index
Bitset::getUnitSize(const unsigned& bsize_) const{
	return (bsize_ / (sizeof(Unit) * ByteWidth) + (bsize_ % (sizeof(Unit) * ByteWidth) == 0 ? 0 : 1));
}

//
// FUNCTION
//	Bitset::getUnitSize -- メソッドを起動したBitsetインスタンスのユニットサイズを求める
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	Bitset::Index
//		メソッドを起動したBitsetインスタンスのユニットサイズ
//
// EXCEPTIONS
//
Bitset::Index
Bitset::getUnitSize() const{
	return getUnitSize(_bitsize);
}

///
// FUNCTION
//	Bitset::getBitData -- ビットデータを取得する。
//
// NOTES
//
// ARGUMENTS
//
// RETURN
//	Bitset::Unit*
//		ビットデータ
//
// EXCEPTIONS
//
#ifdef OBSOLETE
Bitset::Unit*
Bitset::getBitData(void) const{
	return _bitdata;
}
#endif

//
// FUNCTION
//	Bitset::expand -- データ領域を拡張する。引数で指定したビット数分の領域を拡張する。
//
// NOTES
//
// ARGUMENTS
//	unsigned size_
//		拡張するビット数
//
// RETURN
//	なし
//
// EXCEPTIONS
//
void
Bitset::expand(unsigned size_){
	// 拡張後のユニットサイズを求める。
	int unitsize = getUnitSize(_bitsize + size_);

	// 新しいデータ領域を確保する。
	Unit* temp = _bitdata;
	_bitdata = new Unit[unitsize];

	// データ領域を0で初期化する。
	memset(_bitdata, 0, sizeof(Unit) * unitsize);

	// 元の値をコピーする。
	for(Index idx=0;idx<getUnitSize();idx++){
		_bitdata[idx] = temp[idx];
	}

	// 元の領域をdeleteする。
	delete[] temp;

	// サイズを設定する。
	_bitsize += size_;
}

//
// FUNCTION
//	Bitset::getString -- 文字列取得関数
//
// NOTES
//		ビットの On, Off を 0, 1 を用いて表現する
//		ex. 0100111011
//
// ARGUMENTS
//		なし
//
// RETURN
// 		ModUnicodeString
//
// EXCEPTIONS
//
#ifdef OBSOLETE
ModUnicodeString
Bitset::getString() const
{
	ModUnicodeOstrStream str;

	int max = size();
	for ( int i = 0; i < max; ++i )
		str << (test(i) ? '1' : '0');

	return str.getString();
}
#endif

//
// FUNCTION
//	Bitset::begin -- 開始位置イテレータの取得
//
// NOTES
//
// ARGUMENTS
//		なし
//
// RETURN
// 		ConstIterator イテレータオブジェクト
//
// EXCEPTIONS
//
#ifdef OBSOLETE
Bitset::ConstIterator
Bitset::begin() const
{
	return ConstIterator(this, 0);
}

//
// FUNCTION
//	Bitset::begin -- 終了位置イテレータの取得
//
// NOTES
//
// ARGUMENTS
//		なし
//
// RETURN
// 		ConstIterator イテレータオブジェクト
//
// EXCEPTIONS
//
Bitset::ConstIterator
Bitset::end() const
{
	return ConstIterator(this, _bitsize);
}
#endif

Bitset::reference::~reference(){}

Bitset::reference&
Bitset::reference::operator=(ModBoolean bit_){
	_Pbs -> set(_Off, bit_);
	return (*this);
}	

#ifdef OBSOLETE
Bitset::reference&	
Bitset::reference::operator=(const reference& Bs_){
	_Pbs -> set(_Off, ModBoolean(Bs_));
	return (*this);
}

ModBoolean	
Bitset::reference::operator~() const{
	return ((!_Pbs -> test(_Off)) ? ModTrue : ModFalse);
}
#endif

/*	
operator ModBoolean() const{
	return (_Pbs -> test(_Off));
}
*/

Bitset::reference::reference(Bitset& x_, unsigned p_)
	:_Pbs(&x_), _Off(p_){}

Bitset::reference::operator ModBoolean() const{
	return (_Pbs -> test(_Off));
}

#ifdef OBSOLETE
Bitset::ConstIterator::ConstIterator(const Bitset* bitset_ = 0,
		ModSize pos_ = 0)
	: _parent(bitset_), _pos(pos_) {}

// ++operator
Bitset::ConstIterator& 
Bitset::ConstIterator::operator ++() {
	++_pos;
	return *this;
}

Bitset::ConstIterator 
Bitset::ConstIterator::operator ++(int dummy_) {
	return ConstIterator(_parent, _pos + 1);
}

// +operator
Bitset::ConstIterator 
Bitset::ConstIterator::operator + ( int i_ ) {
	return ConstIterator(_parent, _pos + i_);
}

// *operator
ModBoolean 
Bitset::ConstIterator::operator *() const {
	return _parent->test(_pos);
}

// ==operator
bool 
Bitset::ConstIterator::operator ==(const ConstIterator& src_) const {
	return (_pos == src_._pos);
}

// !=Operator
bool 
Bitset::ConstIterator::operator !=(const ConstIterator& src_) const {
	return (_pos != src_._pos);
}
	
// ==operator
ModBoolean
Bitset::operator == (const Bitset& other_)
{
	if ( this != &other_ ) {
		// size is different
		if ( _bitsize != other_._bitsize )
			return ModFalse;

		// compare every unit value
		Index max = getUnitSize();
		for ( Index i = 0; i < max; ++i ) {
			if ( _bitdata[i] != other_._bitdata[i] )
				return ModFalse;
		}
	}

	// If it comes to here, correct answer
	return ModTrue;
}

// != operator
ModBoolean
Bitset::operator != (const Bitset& other_)
{
	return !(*this == other_) ? ModTrue : ModFalse;
}
#endif

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
