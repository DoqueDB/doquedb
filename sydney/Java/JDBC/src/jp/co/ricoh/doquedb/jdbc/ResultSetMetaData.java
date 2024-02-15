// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResultSetMetaData.java -- JDBC 結果セットメタデータクラス
//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2015, 2023 Ricoh Company, Ltd.
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

import java.sql.SQLException;

import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * JDBC 結果セットメタデータクラス。
 * <P>
 * <code>ResultSet</code> オブジェクトの列の型とプロパティに関する情報を
 * 取得するのに使用できるオブジェクトです。
 *
 */
public class ResultSetMetaData implements java.sql.ResultSetMetaData
{
	/** 結果セットの各列のメタデータ。 */
	private jp.co.ricoh.doquedb.common.ResultSetMetaData metaData;

	/**
	 * 新しく <code>ResultSetMetaData</code> オブジェクトを作成します。
	 *
	 * @param	metaData
	 *			DoqueDB自の結果集合メタデータ
	 */
	ResultSetMetaData(jp.co.ricoh.doquedb.common.ResultSetMetaData metaData)
	{
		this.metaData = metaData;
	}

	/**
	 * 結果セットオブジェクトの列数を返します。
	 *
	 * @return	結果セットオブジェクトの列数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 * @see		jp.co.ricoh.doquedb.jdbc.ResultSet
	 */
	public int getColumnCount() throws java.sql.SQLException
	{
		return metaData.getCount();
	}

	/**
	 * 指定された列が自動的に番号付けされて
	 * 読み込み専用として扱われるかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isAutoIncrement(int	columnIndex_)
		throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).isAutoIncrement();
	}

	/**
	 * 列の大文字小文字が区別されるかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isCaseSensitive(int	columnIndex_)
		throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).isCaseInsensitive()
			? false : true;
	}

	/**
	 * 指定された列を where 節で使用できるかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isSearchable(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).isNotSearchable()
			? false : true;
	}

	/**
	 * 指定された列がキャッシュの値かどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isCurrency(int	columnIndex_) throws java.sql.SQLException
	{
		return false;
	}

	/**
	 * 指定された列に <code>null</code> 値が許可されるかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列に <code>null</code> 値が許可されるかどうか。
	 *			<code>java.sql.ResultSetMetaData.columnNoNulls</code> 、
	 *			<code>java.sql.ResultSetMetaData.columnNullable</code> 、
	 *			<code>java.sql.ResultSetMetaData.columnNullableUnknown</code>
	 *			のうちの 1 つ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int isNullable(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).isNotNullable()
			? columnNoNulls : columnNullable;
	}

	/**
	 * 指定された列の値が符号付き数値かどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isSigned(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).isUnsigned()
			? false : true;
	}

	/**
	 * 指定された列の通常の最大幅を文字数で示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列の幅として許可される通常の最大文字数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getColumnDisplaySize(int	columnIndex_)
		throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).getDisplaySize();
	}

	/**
	 * 印刷や表示に使用する、指定された列の推奨タイトルを取得します。
	 * DoqueDBではgetColumnNameと同じ文字列を返します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列の推奨タイトル。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getColumnLabel(int	columnIndex_)
		throws java.sql.SQLException
	{
		return getColumnName(columnIndex_);
	}

	/**
	 * 指定された列の名前を取得します。
	 * DoqueDBではSQL文にエイリアスが指定されていればエイリアスを、
	 * 指定されていなければ列名を返します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	列名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getColumnName(int	columnIndex_)
		throws java.sql.SQLException
	{
		String name
			= metaData.getElement(columnIndex_ - 1).getColumnAliasName();
		if (name == null || name.length() == 0)
			name = metaData.getElement(columnIndex_ - 1).getColumnName();
		return name;
	}

	/**
	 * 指定された列のテーブルのスキーマを取得します。
	 * DoqueDBではデータベース名を返します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	スキーマ名。適用不可の場合は、 "" 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getSchemaName(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).getDatabaseName();
	}

	/**
	 * 指定された列の 10 進桁数を取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	精度。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getPrecision(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).getPrecision();
	}

	/**
	 * 指定された列の小数点以下の桁数を取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	スケール (桁数) 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getScale(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).getScale();
	}

	/**
	 * 指定された列のテーブル名を取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	テーブル名。適用不可の場合は、 "" 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getTableName(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).getTableName();
	}

	/**
	 * 指定された列のテーブルのカタログ名を取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列が現れるテーブルのカタログ名。
	 *			適用不可の場合は、 "" 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getCatalogName(int	columnIndex_)
		throws java.sql.SQLException
	{
		return "";
	}

	/**
	 * 指定された列の SQL 型を取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	<code>java.sql.Types</code> からの SQL 型。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getColumnType(int	columnIndex_) throws java.sql.SQLException
	{
		int type = 0;

		if (metaData.getElement(columnIndex_ - 1).isArray() == true)
		{
			type = java.sql.Types.ARRAY;
		}
		else
		{
			type = getJDBCType(metaData.getElement(columnIndex_ - 1).getType());
		}

		return type;
	}

	/**
	 * 指定された列のデータベース固有の型名を取得します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	データベースが使用する型名。
	 *			列の型がユーザ定義型の場合は、完全指定された型名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getColumnTypeName(int	columnIndex_)
		throws java.sql.SQLException
	{
		return (metaData.getElement(columnIndex_ - 1).isArray() == false)
			? metaData.getElement(columnIndex_ - 1).getTypeName()
			: metaData.getElement(columnIndex_ - 1).getTypeName() + " array";
	}

	/**
	 * 指定された列が絶対的に書き込み可能でないかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isReadOnly(int	columnIndex_) throws java.sql.SQLException
	{
		return metaData.getElement(columnIndex_ - 1).isReadOnly();
	}

	/**
	 * 指定された列への書き込みを成功させることができるかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isWritable(int	columnIndex_) throws java.sql.SQLException
	{
		return isReadOnly(columnIndex_) ? false : true;
	}

	/**
	 * 指定された列の書き込みが必ず成功するかどうかを示します。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isDefinitelyWritable(int	columnIndex_)
		throws java.sql.SQLException
	{
		return (isReadOnly(columnIndex_)
				|| metaData.getElement(columnIndex_ - 1).isNotNullable()
				|| metaData.getElement(columnIndex_ - 1).isUnique())
			? false : true;
	}

	/**
	 * Java クラスの完全指定された名前を返します。
	 * 列から値を検索するために <code>ResultSet.getObject</code> メソッドが
	 * 呼び出されると、この Java クラスのインスタンスが生成されます。
	 * <code>ResultSet.getObject</code> メソッドは、
	 * このメソッドで返されたクラスのサブクラスを返す場合もあります。
	 *
	 * @param	columnIndex_
	 *			最初の列は 1 、 2 番目の列は 2 、などとする。
	 * @return	指定された列の値を取り出すために
	 *			<code>ResultSet.getObject</code> メソッドによって使用される
	 *			Java プログラミング言語のクラスの完全指定された名前。
	 *			カスタムマッピングに使用されるクラス名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getColumnClassName(int	columnIndex_)
		throws java.sql.SQLException
	{
		String buf = null;
		if (metaData.getElement(columnIndex_ - 1).isArray() == true)
		{
			buf = "java.sql.Array";
		}
		else
		{
			switch (metaData.getElement(columnIndex_ - 1).getType())
			{
			case jp.co.ricoh.doquedb.common.SQLTypes.CHARACTER:
			case jp.co.ricoh.doquedb.common.SQLTypes.CHARACTER_VARYING:
			case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER:
			case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER_VARYING:
			case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER_LARGE_OBJECT:
				buf = "java.lang.String";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.BINARY:
			case jp.co.ricoh.doquedb.common.SQLTypes.BINARY_VARYING:
			case jp.co.ricoh.doquedb.common.SQLTypes.BINARY_LARGE_OBJECT:
				buf = new byte[1].getClass().getName();
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.INTEGER:
				buf = "java.lang.Integer";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.BIG_INT:
				buf = "java.lang.Long";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.DOUBLE_PRECISION:
				buf = "java.lang.Double";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.DATE:
				buf = "java.sql.Date";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.TIMESTAMP:
				buf = "java.sql.Timestamp";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.LANGUAGE:
				buf = "jp.co.ricoh.doquedb.common.LanguageData";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.WORD:
				buf = "jp.co.ricoh.doquedb.common.WordData";
				break;
			case jp.co.ricoh.doquedb.common.SQLTypes.DECIMAL:
				buf = "java.math.BigDecimal";
			}
		}
		return buf;
	}

	/**
	 * 文字列表現を得る
	 */
	public String toString()
	{
		return metaData.toString();
	}

	/**
	 * カラムのメタデータを得る
	 */
	jp.co.ricoh.doquedb.common.ColumnMetaData
	getColumnMetaData(int columnIndex_)
	{
		return metaData.getElement(columnIndex_ - 1);
	}

	/**
	 * DoqueDBのSQLデータ型をJDBCのSQLデータ型へマップする
	 */
	static int getJDBCType(int trmeisterType_)
	{
		int type = 0;

		switch (trmeisterType_)
		{
		case jp.co.ricoh.doquedb.common.SQLTypes.CHARACTER:
		case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER:
			type =  java.sql.Types.CHAR;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.CHARACTER_VARYING:
		case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER_VARYING:
			type =  java.sql.Types.VARCHAR;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.BINARY:
			type =  java.sql.Types.BINARY;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.BINARY_VARYING:
			type =  java.sql.Types.VARBINARY;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.INTEGER:
			type =  java.sql.Types.INTEGER;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.BIG_INT:
			type = java.sql.Types.BIGINT;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.DECIMAL:
			type = java.sql.Types.DECIMAL;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.DOUBLE_PRECISION:
			type =  java.sql.Types.DOUBLE;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.DATE:
			type =  java.sql.Types.DATE;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.TIMESTAMP:
			type =  java.sql.Types.TIMESTAMP;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.LANGUAGE:
		case jp.co.ricoh.doquedb.common.SQLTypes.WORD:
			type =  java.sql.Types.OTHER;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.BINARY_LARGE_OBJECT:
			type = java.sql.Types.BLOB;
			break;
		case jp.co.ricoh.doquedb.common.SQLTypes.NATIONAL_CHARACTER_LARGE_OBJECT:
			type = java.sql.Types.CLOB;
			break;
		}
		return type;
	}

	/**
	 * <B>[サポート外]</B>
	 */
	@Override
	public <T> T unwrap(Class<T> iface) throws SQLException {
		// サポート外

		throw new NotSupported();
	}

	/**
	 * <B>[サポート外]</B>
	 * <P>
	 * 常に<code>false</code>を返します
	 */
	@Override
	public boolean isWrapperFor(Class<?> iface) throws SQLException {
		// unwrap を実装している場合は true を返すが、
		// それ以外の場合は false を返す

		return false;
	}

}

//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
