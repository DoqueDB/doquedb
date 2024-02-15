// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Data.java -- データ型をあらわすクラス共通の基底クラス
//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.common;

/**
 * データ型をあらわすクラス共通の基底クラス
 *
 */
public abstract class Data
{
	/** {@link DataType データタイプ} */
	private int _type;

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param type_	{@link DataType データタイプ}
	 */
	public Data(int type_)
	{
		_type = type_;
	}

	/**
	 * {@link DataType データタイプ}を得る
	 *
	 * @return	{@link DataType データタイプ}
	 */
	public int getType()
	{
		return _type;
	}

	/**
	 * 配列要素の{@link DataType データタイプ}を得る
	 *
	 * @return	{@link DataType データタイプ}
	 *			つねにDataType.Undefinedを返す。配列データ型で上書きする。
	 */
	public int getElementType()
	{
		return DataType.UNDEFINED;
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
