// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayData.java -- 配列データ型をあらわすクラス共通の基底クラス
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
 * 配列データ型をあらわすクラス共通の基底クラス
 *
 */
public abstract class ArrayData extends Data
{
	/** 配列要素のデータ型 */
	private int _elementType;
	/** 配列データ */
	protected java.util.Vector _array;

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param elementType_	{@link DataType データタイプ}
	 */
	public ArrayData(int elementType_)
	{
		super(DataType.ARRAY);
		_elementType = elementType_;
		_array = new java.util.Vector();
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param elementType_	{@link DataType データタイプ}
	 * @param size_			配列要素数の初期容量
	 */
	public ArrayData(int elementType_, int size_)
	{
		super(DataType.ARRAY);
		_elementType = elementType_;
		_array = new java.util.Vector(size_, 1);
		_array.setSize(size_);
	}

	/**
	 * 配列要素のデータタイプを得る
	 *
	 * @return	配列要素の{@link DataType データタイプ}
	 */
	public int getElementType()
	{
		return _elementType;
	}

	/**
	 * 配列要素数を得る
	 *
	 * @return	配列要素数
	 */
	public int getCount()
	{
		return _array.size();
	}

	/**
	 * 配列の末尾に要素を追加する
	 *
	 * @param element_	挿入される要素
	 */
	protected void add(Object element_)
	{
		_array.add(element_);
	}

	/**
	 * 配列内の指定された位置にある要素を返す
	 *
	 * @param index_	配列内の位置
	 * @return	指定された位置にある要素
	 * @throws	java.lang.ArrayIndexOutOfBoundsException
	 *			範囲外のインデックスが指定された
	 */
	protected Object get(int index_)
		throws ArrayIndexOutOfBoundsException
	{
		return _array.get(index_);
	}

	/**
	 * 配列内の指定された位置にある要素を、指定の要素で置き換える
	 *
	 * @param index_	配列内の位置
	 * @param element_	格納する要素
	 * @return	指定された位置に以前あった要素
	 */
	protected Object set(int index_, Object element_)
	{
		if (index_ >= _array.size())
			_array.setSize(index_ + 1);
		return _array.set(index_, element_);
	}

	/**
	 * すべての要素を削除する
	 */
	public void clear()
	{
		_array.clear();
	}

	/**
	 * サイズを設定する
	 */
	public void setSize(int newSize)
	{
		_array.setSize(newSize);
	}

	/**
	 * データが等しいか比較する
	 *
	 * @return	等しい場合はtrue、それ以外の場合はfalse
	 */
	public boolean equals(Object other_)
	{
		boolean result = false;
		if ((other_ instanceof ArrayData) == true)
		{
			ArrayData o = (ArrayData)other_;
			result = _array.equals(o._array);
		}
		return result;
	}

	/**
	 * 文字列に変換する
	 *
	 * @return	文字列
	 */
	public String toString()
	{
		StringBuilder p = new StringBuilder();
		p.append("{");
		for (int i = 0; i < _array.size(); ++i)
		{
			if (i != 0) p.append(",");
			p.append(_array.get(i).toString());
		}
		p.append("}");

		return p.toString();
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
