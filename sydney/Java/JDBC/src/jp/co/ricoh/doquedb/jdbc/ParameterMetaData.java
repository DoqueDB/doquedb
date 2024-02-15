// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ParameterMetaData.java -- JDBC のパラメータメタデータクラス
//
// Copyright (c) 2003, 2006, 2015, 2023 Ricoh Company, Ltd.
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

import jp.co.ricoh.doquedb.common.ColumnMetaData;
import jp.co.ricoh.doquedb.exception.BadArgument;
import jp.co.ricoh.doquedb.exception.NotSupported;

/**
 * JDBC のパラメータメタデータクラス。
 *
 */
public class ParameterMetaData implements java.sql.ParameterMetaData
{
	/** パラメータの数。 */
	private int					_numberOfParameters;

	/** パラメータの各列のメタデータ。 */
	private ColumnMetaData[]	_parameterMetaDatas;

	/** 各パラメータのパラメータモード。 */
	private int[]				_parameterModes;

	/**
	 * 新しくパラメータメタデータオブジェクトを作成します。
	 *
	 * @param	prepareStatement_
	 *			コンパイル結果オブジェクト。
	 */
	ParameterMetaData(
			jp.co.ricoh.doquedb.client.PrepareStatement	prepareStatement_,
			String	sql_/* [YET!] まだ prepareStatement_ から
						          メタデータを得られないので、
						          とりあえず、このオブジェクトで SQL 文を
						          パースして、_numberOfParameters くらいは
						          設定する。 */)
	{
		// [YET!] 各パラメータのメタデータを、prepareStatement_ から得る！
		this._numberOfParameters = dummyGetNumOfParam(sql_);

		this._parameterMetaDatas = null;

		this._parameterModes = null;
	}

	/**
	 * この <code>ParameterMetaData</code> オブジェクトが情報を含む
	 * <code>java.sql.PreparedStatement</code> 内のパラメータの数を取得します。
	 *
	 * @return	パラメータの数。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getParameterCount() throws java.sql.SQLException
	{
		return this._numberOfParameters;
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータで <code>null</code> 値が許可されるかどうかを
	 * 取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	指定されたパラメータの <code>null</code> 値の状態。
	 *			<code>java.sql.ParameterMetaData.parameterNoNulls</code> 、
	 *			<code>java.sql.ParameterMetaData.parameterNullable</code> 、
	 *			または <code>
	 *			java.sql.ParameterMetaData.parameterNullableUnknown</code>
	 *			のうちの 1 つ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int isNullable(int	parameterIndex_) throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].isNullable();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータの値が符号付き数値かどうかを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	上記の場合は <code>true</code> 、
	 *			そうでない場合は <code>false</code> 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public boolean isSigned(int	parameterIndex_) throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].isSigned();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータの 10 進桁数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	精度。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getPrecision(int	parameterIndex_) throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].getPrecision();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータの小数点以下の桁数を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	スケール (桁数) 。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getScale(int	parameterIndex_) throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].getScale();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータの SQL 型を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	<code>java.sql.Types</code> からの SQL 型。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getParameterType(int	parameterIndex_)
		throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].getSqlType();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータのデータベース固有の型名を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	データベースが使用する型名。
	 *			パラメータの型がユーザ定義型の場合は、完全指定された型名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getParameterTypeName(int	parameterIndex_)
		throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].getTypeName();
	}

	/**
	 * <B>[サポート外]</B>
	 * インスタンスが
	 * <code>java.sql.PreparedStatement.setObject</code> メソッドに
	 * 渡される Java クラスの完全指定された名前を取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	指定されたパラメータの値を設定するために
	 *			<code>java.sql.PreparedStatement.setObject</code>
	 *			メソッドによって使用される Java プログラミング言語の
	 *			クラスの完全指定された名前。
	 *			カスタムマッピングに使用されるクラス名。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public String getParameterClassName(int	parameterIndex_)
		throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterMetaDatas[parameterIndex_ - 1].getClassName();
	}

	/**
	 * <B>[サポート外]</B>
	 * 指定されたパラメータのモードを取得します。
	 * <P>
	 * <B>注:</B>
	 * 現在のバージョンでは、サポートしていません。
	 *
	 * @param	parameterIndex_
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @return	指定されたパラメータのモード。
	 *			<code>java.sql.ParameterMetaData.parameterModeIn </code> 、
	 *			<code>java.sql.ParameterMetaData.parameterModeOut </code> 、
	 *			<code>java.sql.ParameterMetaData.parameterModeInOut </code> 、
	 *			または
	 *			<code>java.sql.ParameterMetaData.parameterModeUnknown </code>
	 *			のうちの 1 つ。
	 * @throws	java.sql.SQLException
	 *			データベースアクセスエラーが発生した場合。
	 */
	public int getParameterMode(int	parameterIndex_)
		throws java.sql.SQLException
	{
		throw new NotSupported();

//		checkParameterIndex(parameterIndex_);
//		return this._parameterModes[parameterIndex_ - 1];
	}

	/**
	 * 指定されたパラメータのインデックスが有効かどうかを調べます。
	 *
	 * @param	parameterIndex_
	 *			パラメータのインデックス。
	 *			最初のパラメータは 1 、 2 番目のパラメータは 2 、などとする。
	 * @throws	jp.co.ricoh.doquedb.exception.BadArgument
	 *			指定されたパラメータのインデックスが無効である。
	 */
	void checkParameterIndex(int	parameterIndex_)
		throws BadArgument
	{
		if (parameterIndex_ < 1 ||
			parameterIndex_ > this._numberOfParameters) {
			// [YET!] SQLSTATE は、
			//        data exception - ???
			//        (22???)
			throw new BadArgument();
		}
	}

	/**
	 * 仮実装用メソッド。
	 *
	 * @param	sql_
	 *			データベースに送られる SQL 文。
	 */
	private int dummyGetNumOfParam(String	sql_)
	{
		//
		// jp.co.ricoh.doquedb.client.PrepareStatement から
		// メタデータが得られるようになれば、このメソッドは不要になる。
		// とりあえずパースする。
		//

		int	idx = 0;
		int	numQ = 0;
		while (idx != -1) {
			idx = sql_.indexOf("?", idx);
			if (idx != -1) {
				numQ++;
				idx++;
			}
		}

		return numQ;
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
// Copyright (c) 2003, 2006, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
