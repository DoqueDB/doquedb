// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataArrayData.java -- Data配列型をあらわすクラス
//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
 * Data配列型をあらわすクラス
 *
 */
public class DataArrayData extends ArrayData
	implements Serializable
{
	/**
	 * データ型をあらわすクラスを新たに作成する
	 */
	public DataArrayData()
	{
		super(DataType.DATA);
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param size_			配列要素数の初期容量
	 */
	public DataArrayData(int size_)
	{
		super(DataType.DATA, size_);
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param value_		配列データ
	 */
	public DataArrayData(DataArrayData value_)
	{
		super(DataType.DATA);
		_array = (java.util.Vector)value_._array.clone();
	}

	/**
	 * 配列の末尾に要素を追加する。
	 * 要素のインスタンスはコピーされない。
	 *
	 * @param element_	挿入される要素
	 */
	public void addElement(Data element_)
	{
		add(element_);
	}

	/**
	 * 配列内の指定された位置にある要素を返す
	 *
	 * @param index_	配列内の位置
	 * @return	指定された位置にある要素
	 * @throws	java.lang.ArrayIndexOutOfBoundsException
	 *			範囲外のインデックスが指定された
	 */
	public Data getElement(int index_)
		throws ArrayIndexOutOfBoundsException
	{
		return (Data)get(index_);
	}

	/**
	 * 配列内の指定された位置にある要素を、指定の要素で置き換える
	 * 要素のインスタンスはコピーされない。
	 *
	 * @param index_	配列内の位置
	 * @param element_	格納する要素
	 * @return	指定された位置に以前あった要素
	 */
	public Data setElement(int index_, Data element_)
	{
		return (Data)set(index_, element_);
	}

	/**
	 * ストリームから読み込む
	 *
	 * @param input_	入力用のストリーム
	 * @throws	java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws	java.lang.ClassNotFoundException
	 *			読み込んだ{@link ClassID クラスID}のクラスが見つからない
	 * @see Serializable#readObject(InputStream) readObject
	 */
	public void readObject(InputStream input_)
		throws java.io.IOException, ClassNotFoundException
	{
		int size = input_.readInt();
		setSize(size);
		for (int i = 0; i < size; ++i)
		{
			setElement(i, (Data)input_.readObject((Serializable)getElement(i)));
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
			output_.writeObject((Serializable)getElement(i));
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
		return ClassID.DATA_ARRAY_DATA;
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new DataArrayData(this);
	}

	/**
	 * 中身をアサインする
	 */
	public void assign(DataArrayData data)
	{
		int size = data.getCount();
		setSize(size);
		for (int i = 0; i < size; ++i)
		{
			set(i, data.get(i));
		}
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
