// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
//
// Unicode 文字情報ファイルのクラス
// 
// Copyright (c) 2023 Ricoh Company, Ltd.
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


#ifndef __MyTOOL_DataRow__
#define __MyTOOL_DataRow__

//
// データ行の基底クラス
//
class UnicodeDataRow
{
public:
	//
	// コンストラクタ、デストラクタ
	//

	// line			--- 1行分の文字列
	// fieldNum		--- line から読みとるフィールドの数
	UnicodeDataRow(const char*	line,
				   const int	fieldNum);
	virtual ~UnicodeDataRow();

	//
	// アクセッサ
	//

	// 任意のフィールドの文字列を取得
	// (最初のフィールドは 0 番とする)
	// (フィールドが空の場合はヌルポインタが返る)
	const char*	getField(const int fieldID) const
	{ return d_fieldArray[fieldID]; };	// 範囲検査してないので注意して使うこと

	// 保持しているフィールドの数を取得
	int			getFieldNum() const
	{ return d_fieldNum; };

private:

	// 保持しているフィールドの数
	const int	d_fieldNum;

	// フィールドの値(文字列)へのポインタを保持する領域
	char**		d_fieldArray;
};

#endif
