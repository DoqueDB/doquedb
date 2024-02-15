// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnMetaData.java -- カラムのメタデータ
//
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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
 * カラムのメタデータ
 *
 */
public final class ColumnMetaData
	implements Serializable
{
	/** SQLデータ型 */
	private int type;

	/** データ型名 */
	private String typeName;
	/** カラム名 */
	private String columnName;
	/** テーブル名 */
	private String tableName;
	/** データベース名 */
	private String databaseName;
	/** カラム別名 */
	private String columnAliasName;
	/** テーブル別名 */
	private String tableAliasName;

	/** 最大表示サイズ */
	private int displaySize;
	/** 10進桁数 */
	private int precision;
	/** 小数点以下の桁数 */
	private int scale;
	/** 配列要素数 */
	private int cardinality;

	/** 属性 */
	private int flag;

	/** 自動採番 */
	private final static int AUTO_INCREMENT = (1 << 0);
	/** 大文字小文字が区別されない */
	private final static int CASE_INSENSITIVE = (1 << 1);
	/** 符号なし */
	private final static int UNSIGNED = (1 << 2);
	/** 検索不可 */
	private final static int NOT_SEARCHABLE = (1 << 3);
	/** 読み出し専用 */
	private final static int READ_ONLY = (1 << 4);
	/** NULL不許可 */
	private final static int NOT_NULL = (1 << 5);
	/** ユニーク */
	private final static int UNIQUE = (1 << 6);

	/** コンストラクタ */
	public ColumnMetaData()
	{
		this.type = SQLTypes.UNKNOWN;
		this.typeName = "";
		this.columnName = "";
		this.tableName = "";
		this.databaseName = "";
		this.columnAliasName = "";
		this.tableAliasName = "";
		this.displaySize = 0;
		this.precision = 0;
		this.scale = 0;
		this.cardinality = 0;
		this.flag = 0;
	}

	/** コンストラクタ */
	public ColumnMetaData(ColumnMetaData metaData)
	{
		this.type = metaData.type;
		this.typeName = metaData.typeName;
		this.columnName = metaData.columnName;
		this.tableName = metaData.tableName;
		this.databaseName = metaData.databaseName;
		this.columnAliasName = metaData.columnAliasName;
		this.tableAliasName = metaData.tableAliasName;
		this.displaySize = metaData.displaySize;
		this.precision = metaData.precision;
		this.scale = metaData.scale;
		this.cardinality = metaData.cardinality;
		this.flag = metaData.flag;
	}

	/** データ型を得る */
	public int getType()
	{
		return type;
	}
	/** データ型を設定する */
	public void setType(int type)
	{
		this.type = type;
	}

	/** データ型名を得る */
	public String getTypeName()
	{
		return typeName;
	}
	/** データ型名を設定する */
	public void setTypeName(String typeName)
	{
		this.typeName = typeName;
	}

	/** カラム名を得る */
	public String getColumnName()
	{
		return columnName;
	}
	/** カラム名を設定する */
	public void setColumnName(String columnName)
	{
		this.columnName = columnName;
	}

	/** テーブル名を得る */
	public String getTableName()
	{
		return tableName;
	}
	/** テーブル名を設定する */
	public void setTableName(String tableName)
	{
		this.tableName = tableName;
	}

	/** データベース名を得る */
	public String getDatabaseName()
	{
		return databaseName;
	}
	/** データベース名を設定する */
	public void setDatabaseName(String databaseName)
	{
		this.databaseName = databaseName;
	}

	/** カラム別名を得る */
	public String getColumnAliasName()
	{
		return columnAliasName;
	}
	/** カラム別名を設定する */
	public void setColumnAliasName(String columnAliasName)
	{
		this.columnAliasName = columnAliasName;
	}

	/** テーブル別名を得る */
	public String getTableAliasName()
	{
		return tableAliasName;
	}
	/** テーブル別名を設定する */
	public void setTableAliasName(String tableAliasName)
	{
		this.tableAliasName = tableAliasName;
	}

	/** 最大表示サイズを得る */
	public int getDisplaySize()
	{
		return displaySize;
	}
	/** 最大表示サイズを設定する */
	public void setDisplaySize(int displaySize)
	{
		this.displaySize = displaySize;
	}

	/** 10進桁数を得る */
	public int getPrecision()
	{
		return precision;
	}
	/** 10進桁数を設定する */
	public void setPrecision(int precision)
	{
		this.precision = precision;
	}

	/** 小数点以下の桁数を得る */
	public int getScale()
	{
		return scale;
	}
	/** 小数点以下の桁数を設定する */
	public void setScale(int scale)
	{
		this.scale = scale;
	}

	/** 配列要素数を得る */
	public int getCardinality()
	{
		return cardinality;
	}
	/** 配列要素数を設定する */
	public void setCardinality(int cardinality)
	{
		this.cardinality = cardinality;
	}

	/** 自動採番かどうか */
	public boolean isAutoIncrement()
	{
		return ((flag & AUTO_INCREMENT) != 0) ? true : false;
	}
	/** 自動採番かどうかを設定する */
	public void setAutoIncrement(boolean v)
	{
		if (v == true)
			flag &= AUTO_INCREMENT;
		else
			flag &= ~AUTO_INCREMENT;
	}

	/** 大文字小文字が区別されないかどうか */
	public boolean isCaseInsensitive()
	{
		return ((flag & CASE_INSENSITIVE) != 0) ? true : false;
	}
	/** 大文字小文字が区別されないかどうかを設定する */
	public void setCaseInsensitive(boolean v)
	{
		if (v == true)
			flag &= CASE_INSENSITIVE;
		else
			flag &= ~CASE_INSENSITIVE;
	}

	/** 符号なしかどうか */
	public boolean isUnsigned()
	{
		return ((flag & UNSIGNED) != 0) ? true : false;
	}
	/** 符号なしかどうかを設定する */
	public void setUnsigned(boolean v)
	{
		if (v == true)
			flag &= UNSIGNED;
		else
			flag &= ~UNSIGNED;
	}

	/** 検索不可かどうか */
	public boolean isNotSearchable()
	{
		return ((flag & NOT_SEARCHABLE) != 0) ? true : false;
	}
	/** 検索不可かどうかを設定する */
	public void setNotSearchable(boolean v)
	{
		if (v == true)
			flag &= NOT_SEARCHABLE;
		else
			flag &= ~NOT_SEARCHABLE;
	}

	/** 読み出し専用かどうか */
	public boolean isReadOnly()
	{
		return ((flag & READ_ONLY) != 0) ? true : false;
	}
	/** 読み出し専用かどうかを設定する */
	public void setReadOnly(boolean v)
	{
		if (v == true)
			flag &= READ_ONLY;
		else
			flag &= ~READ_ONLY;
	}

	/** NULLをセットできないかどうか */
	public boolean isNotNullable()
	{
		return ((flag & NOT_NULL) != 0) ? true : false;
	}
	/** NULLをセットできないかどうかを設定する */
	public void setNotNullable(boolean v)
	{
		if (v == true)
			flag &= NOT_NULL;
		else
			flag &= ~NOT_NULL;
	}

	/** ユニークかどうか */
	public boolean isUnique()
	{
		return ((flag & UNIQUE) != 0) ? true : false;
	}
	/** ユニークかどうかを設定する */
	public void setUnique(boolean v)
	{
		if (v == true)
			flag &= UNIQUE;
		else
			flag &= ~UNIQUE;
	}

	/** 配列かどうか */
	public boolean isArray()
	{
		return (cardinality != 0) ? true : false;
	}

	/**
	 * ストリームから読み込む
	 *
	 * @param input		入力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 * @throws java.lang.ClassNotFoundException
	 *			{@link ClassID クラスID}のクラスが見つからない
	 */
	public void readObject(InputStream input)
		throws java.io.IOException,
			   ClassNotFoundException
	{
		// タイプ
		this.type = input.readInt();

		// ModUnicodeString
		int c = input.readInt();
		if (c != 0) { typeName = UnicodeString.readObject(input); c--; }
		if (c != 0) { columnName = UnicodeString.readObject(input); c--; }
		if (c != 0) { tableName = UnicodeString.readObject(input); c--; }
		if (c != 0) { databaseName = UnicodeString.readObject(input); c--; }
		if (c != 0) { columnAliasName = UnicodeString.readObject(input); c--; }
		if (c != 0) { tableAliasName = UnicodeString.readObject(input); c--; }

		// int
		c = input.readInt();
		if (c != 0) { displaySize = input.readInt(); c--; }
		if (c != 0) { precision = input.readInt(); c--; }
		if (c != 0) { scale = input.readInt(); c--; }
		if (c != 0) { cardinality = input.readInt(); c--; }

		// flag
		flag = input.readInt();
	}

	/**
	 * ストリームに書き出す
	 *
	 * @param output	出力用のストリーム
	 * @throws java.io.IOException
	 *			入出力関係の例外が発生した
	 */
	public void writeObject(OutputStream output)
		throws java.io.IOException
	{
		// タイプ
		output.writeInt(type);

		// ModUnicodeString
		output.writeInt(6);
		UnicodeString.writeObject(output, typeName);
		UnicodeString.writeObject(output, columnName);
		UnicodeString.writeObject(output, tableName);
		UnicodeString.writeObject(output, databaseName);
		UnicodeString.writeObject(output, columnAliasName);
		UnicodeString.writeObject(output, tableAliasName);

		// int
		output.writeInt(4);
		output.writeInt(displaySize);
		output.writeInt(precision);
		output.writeInt(scale);
		output.writeInt(cardinality);

		// flag
		output.writeInt(flag);
	}

	/**
	 * {@link ClassID クラスID}を得る
	 *
	 * @return {@link ClassID クラスID}
	 */
	public int getClassID() { return ClassID.COLUMN_META_DATA; }

	/** 文字列表現を得る */
	public String toString() { return getColumnAliasName(); }

	/**
	 * 適切なDataクラスのインスタンスを得る
	 */
	public Data getDataInstance()
	{
		Data data = null;
		if (isArray() == true)
		{
			data = new DataArrayData();
		}
		else
		{
			switch (getDataType(getType()))
			{
			case DataType.STRING:
				data = new StringData();
				break;
			case DataType.BINARY:
				data = new BinaryData();
				break;
			case DataType.INTEGER:
				data = new IntegerData();
				break;
			case DataType.INTEGER64:
				data = new Integer64Data();
				break;
			case DataType.DECIMAL:
				data = new DecimalData();
				break;
			case DataType.DOUBLE:
				data = new DoubleData();
				break;
			case DataType.DATE:
				data = new DateData();
				break;
			case DataType.DATE_TIME:
				data = new DateTimeData();
				break;
			case DataType.LANGUAGE:
				data = new LanguageData();
				break;
			case DataType.WORD:
				data = new WordData();
				break;
			}
		}
		return data;
	}

	/** DataTypeを得る */
	public static int getDataType(int type)
	{
		int dataType = DataType.UNDEFINED;

		switch (type)
		{
		case SQLTypes.CHARACTER:
		case SQLTypes.CHARACTER_VARYING:
		case SQLTypes.NATIONAL_CHARACTER:
		case SQLTypes.NATIONAL_CHARACTER_VARYING:
			dataType = DataType.STRING;
			break;
		case SQLTypes.BINARY:
		case SQLTypes.BINARY_VARYING:
			dataType = DataType.BINARY;
			break;
		case SQLTypes.INTEGER:
			dataType = DataType.INTEGER;
			break;
		case SQLTypes.BIG_INT:
			dataType = DataType.INTEGER64;
			break;
		case SQLTypes.DECIMAL:
		case SQLTypes.NUMERIC:
			dataType = DataType.DECIMAL;
			break;
		case SQLTypes.DOUBLE_PRECISION:
			dataType = DataType.DOUBLE;
			break;
		case SQLTypes.DATE:
			dataType = DataType.DATE;
			break;
		case SQLTypes.TIMESTAMP:
			dataType = DataType.DATE_TIME;
			break;
		case SQLTypes.LANGUAGE:
			dataType = DataType.LANGUAGE;
			break;
		case SQLTypes.WORD:
			dataType = DataType.WORD;
			break;
		}

		return dataType;
	}
}

//
// Copyright (c) 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
