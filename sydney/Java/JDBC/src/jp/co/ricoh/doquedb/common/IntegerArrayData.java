// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// IntegerArrayData.java -- int配列型をあらわすクラス
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
 * int配列型をあらわすクラス
 *
 */
public class IntegerArrayData extends ArrayData
	implements Serializable
{
	/**
	 * データ型をあらわすクラスを新たに作成する
	 */
	public IntegerArrayData()
	{
		super(DataType.INTEGER);
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param size_			配列要素数の初期容量
	 */
	public IntegerArrayData(int size_)
	{
		super(DataType.INTEGER, size_);
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param value_		配列データ
	 */
	public IntegerArrayData(java.util.Vector value_)
	{
		super(DataType.INTEGER);
		_array = (java.util.Vector)value_.clone();
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param value_		配列データ
	 */
	public IntegerArrayData(IntegerArrayData value_)
	{
		super(DataType.INTEGER);
		_array = (java.util.Vector)value_._array.clone();
	}

	/**
	 * 配列の末尾に要素を追加する
	 *
	 * @param element_	挿入される要素
	 */
	public void addElement(int element_)
	{
		add(new Integer(element_));
	}

	/**
	 * 配列内の指定された位置にある要素を返す
	 *
	 * @param index_	配列内の位置
	 * @return	指定された位置にある要素
	 * @throws	java.lang.ArrayIndexOutOfBoundsException
	 *			範囲外のインデックスが指定された
	 */
	public int getElement(int index_)
		throws ArrayIndexOutOfBoundsException
	{
		return ((Integer)get(index_)).intValue();
	}

	/**
	 * 配列内の指定された位置にある要素を、指定の要素で置き換える
	 *
	 * @param index_	配列内の位置
	 * @param element_	格納する要素
	 * @return	指定された位置に以前あった要素
	 */
	public int setElement(int index_, int element_)
	{
		return ((Integer)set(index_, new Integer(element_))).intValue();
	}

	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream input_)
		throws java.io.IOException
	{
		clear();	// 配列を空にする
		int size = input_.readInt();
		for (int i = 0; i < size; ++i)
		{
			addElement(input_.readInt());
		}
	}

	/**
	 * ストリームに書き出す
	 *
	 * @param output_	出力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @see	Serializable#writeObject(OutputStream) writeObject
	 */
	public void writeObject(OutputStream output_)
		throws java.io.IOException
	{
		int size = getCount();
		output_.writeInt(size);
		for (int i = 0; i < size; ++i)
		{
			output_.writeInt(getElement(i));
		}
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 * @see Serializable#getClassID() getClassID
	 */
	public int getClassID()
	{
		return ClassID.INTEGER_ARRAY_DATA;
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new IntegerArrayData(this);
	}
}

//
// Copyright (c) 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
