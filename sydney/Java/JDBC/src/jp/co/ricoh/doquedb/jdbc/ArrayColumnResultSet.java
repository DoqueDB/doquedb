// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayColumnResultSet.java -- 配列型の列値のための結果セットのクラス
//
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbc;

import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * 配列型の列値のための結果セットのクラス。
 *
 */
class ArrayColumnResultSet extends DefaultResultSet
{
	/**
	 * 配列
	 *
	 * @see jp.co.ricoh.doquedb.jdbc.Array
	 */
	private jp.co.ricoh.doquedb.common.DataArrayData _array;

	/**
	 * 配列型の列値の開始要素数(0ベース)
	 */
	private int _index;

	/**
	 * 配列型の列値の要素数。
	 */
	private int	_numberOfElements;

	/**
	 * データカラムのメタデータ
	 */
	private jp.co.ricoh.doquedb.common.ColumnMetaData _columnMetaData;

	/**
	 * メタデータ
	 */
	private ResultSetMetaData _metaData;

	/**
	 * 新しく配列型の列値のための結果セットオブジェクトを作成します。
	 */
	ArrayColumnResultSet(
		jp.co.ricoh.doquedb.common.DataArrayData array_,
		int index_, int count_,
		jp.co.ricoh.doquedb.common.ColumnMetaData metaData_)
	{
		super();

		this._array = array_;
		this._index = index_;
		this._numberOfElements = count_;
		this._columnMetaData = metaData_;
	}

	/**
	 * カーソルを現在の要素から 1 要素先に移動します。
	 * <P>
	 * 各要素に関する行には次の列があります。
	 *	<OL>
	 *	<LI><B>ELEM_IDX</B> int =>
	 *		要素インデックス ( 1 から始まる)
	 *	<LI><B>ELEM_VAL</B>	Object =>
	 *		要素値
	 *	</OL>
	 *
	 * @return	新しい現在の要素が有効な場合は <code>true</code> 、
	 *			それ以上要素がない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean next() throws java.sql.SQLException
	{
		super.closeInputStream();

		if (super._rowNumber == -1) return false;

		if (super._rowNumber >= this._numberOfElements) {

			super._rowNumber = -1;
			super._row = null;
			return false;
		}

		if (super._row == null)
		{
			super._row = new jp.co.ricoh.doquedb.common.DataArrayData(2);
			super._row.setElement(
				0, new jp.co.ricoh.doquedb.common.IntegerData());
		}
		((jp.co.ricoh.doquedb.common.IntegerData)super._row.getElement(0))
			.setValue(super._rowNumber + _index + 1);
		super._row.setElement(
			1, this._array.getElement(super._rowNumber + _index));
		super._rowNumber++;

		return true;
	}

	/**
	 * 自動的にクローズされるとき、これを待つのではなく、
	 * ただちにこの <code>ArrayColumnResultSet</code> オブジェクトの
	 * データベースと JDBC リソースを解放します。
	 *
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public void close() throws java.sql.SQLException
	{
		super.close();
	}

	/**
	 * この <code>ResultSet</code> オブジェクトの列の数、型、およびプロパティを
	 * 取得します。
	 *
	 * @return	この <code>ResultSet</code> オブジェクトの列の記述。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public java.sql.ResultSetMetaData getMetaData()
		throws java.sql.SQLException
	{
		if (_columnMetaData == null)
			throw new NotSupported();

		if (_metaData == null)
		{
			// ないので作成する
			jp.co.ricoh.doquedb.common.ResultSetMetaData meta
				= new jp.co.ricoh.doquedb.common.ResultSetMetaData();
			jp.co.ricoh.doquedb.common.ColumnMetaData columnMeta
				= new jp.co.ricoh.doquedb.common.ColumnMetaData();
			columnMeta.setType(jp.co.ricoh.doquedb.common.SQLTypes.INTEGER);
			columnMeta.setTypeName("int");
			columnMeta.setDisplaySize(10);
			columnMeta.setPrecision(10);
			columnMeta.setNotSearchable(true);
			columnMeta.setReadOnly(true);
			columnMeta.setNotNullable(true);
			meta.addElement(columnMeta);
			columnMeta
				= new jp.co.ricoh.doquedb.common.ColumnMetaData(_columnMetaData);
			columnMeta.setCardinality(0);
			meta.addElement(columnMeta);
			_metaData = new ResultSetMetaData(meta);
		}

		return this._metaData;
	}

	/**
	 * カーソルがこの <code>ArrayColumnResultSet</code> オブジェクトの
	 * 最終行にあるかどうかを取得します。
	 *
	 * @return	カーソルが最終行にある場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isLast() throws java.sql.SQLException
	{
		return (super._rowNumber == this._numberOfElements);
	}

	/**
	 * このオブジェクトへの参照がないと、ガベージコレクションによって
	 * 判断されたときに、ガベージコレクタによって呼び出されます。
	 *
	 * @throws	java.lang.Throwable
	 *			このメソッドで生じた例外。
	 */
	protected void finalize() throws java.lang.Throwable
	{
		this.close();
	}
}

//
// Copyright (c) 2003, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
