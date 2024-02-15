// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSetMetaData.java -- 結果集合のメタデータ
//
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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
 * 結果集合のメタデータ
 *
 */
public final class ResultSetMetaData extends ArrayData
	implements Serializable
{
	/**
	 * データ型をあらわすクラスを新たに作成する
	 */
	public ResultSetMetaData()
	{
		super(DataType.COLUMN_META_DATA);
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param size_			配列要素数の初期容量
	 */
	public ResultSetMetaData(int size_)
	{
		super(DataType.COLUMN_META_DATA, size_);
	}

	/**
	 * データ型をあらわすクラスを新たに作成する
	 *
	 * @param value_		配列データ
	 */
	public ResultSetMetaData(ResultSetMetaData value_)
	{
		super(DataType.COLUMN_META_DATA);
		_array = (java.util.Vector)value_._array.clone();
	}

	/**
	 * 配列の末尾に要素を追加する。
	 * 要素のインスタンスはコピーされない。
	 *
	 * @param element_	挿入される要素
	 */
	public void addElement(ColumnMetaData element_)
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
	public ColumnMetaData getElement(int index_)
		throws ArrayIndexOutOfBoundsException
	{
		return (ColumnMetaData)get(index_);
	}

	/**
	 * 配列内の指定された位置にある要素を、指定の要素で置き換える
	 * 要素のインスタンスはコピーされない。
	 *
	 * @param index_	配列内の位置
	 * @param element_	格納する要素
	 * @return	指定された位置に以前あった要素
	 */
	public ColumnMetaData setElement(int index_, ColumnMetaData element_)
	{
		return (ColumnMetaData)set(index_, element_);
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
			ColumnMetaData meta = new ColumnMetaData();
			meta.readObject(input_);
			setElement(i, meta);
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
			getElement(i).writeObject(output_);
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
		return ClassID.RESULTSET_META_DATA;
	}

	/**
	 * オブジェクトのコピーを作成して返す
	 *
	 * @return	コピーされたオブジェクト
	 */
	public Object clone()
	{
		return new ResultSetMetaData(this);
	}

	/**
	 * メタデータから適切なデータ型が格納されたDataArrayDataを得る
	 */
	public DataArrayData createTupleData()
	{
		int size = getCount();
		DataArrayData tuple = new DataArrayData(size);
		for (int i = 0; i < size; ++i)
		{
			tuple.add(getElement(i).getDataInstance());
		}
		return tuple;
	}

}

//
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
