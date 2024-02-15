// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SydObjectFactory.java -- JDBCX の SydObjectFactoryクラス
//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.jdbcx;

import java.util.Hashtable;

import javax.naming.Context;
import javax.naming.Name;
import javax.naming.RefAddr;
import javax.naming.Reference;
import javax.naming.spi.ObjectFactory;

/**
 * DataSource のオブジェクファクトリです。
 * 直接使用する事はありません。
 *
 */
class SydObjectFactory implements ObjectFactory
{
	/**
	 * DataSource インスタンスを生成します。
	 * 指定された位置情報または参照情報を使って、オブジェクトを生成します。
	 *
	 * @param	obj_
	 *			オブジェクトの生成に使える位置情報または参照情報を格納する、null の可能性があるオブジェクト
	 * @param	name
	 *			nameCtx に関連するこのオブジェクトの名前、または名前が指定されない場合は null
	 * @param	nameCtx
	 *			name パラメータの指定に関連するコンテキスト、または name がデフォルトの初期コンテキストに関連する場合は null
	 * @param	environment
	 *			null の可能性がある、オブジェクトの生成に使用される環境
	 * @return	データソースオブジェクト
	 */
	public Object getObjectInstance(	Object		obj_,
										Name		name_,
										Context		nameCtx_,
										Hashtable	environment_)throws Exception
	{
//		System.out.print("getObjectInstance()\n");
		Reference rf = (Reference)obj_;
		String className = rf.getClassName();
//		System.out.print("className=" + className + "\n");
		if(className.equals("jp.co.ricoh.doquedb.jdbcx.ConnectionPoolDataSource")){
			// 紛らわしいが？こっちはプールされていないデータソースが返る
			return loadConnectionPool(rf);

		}else if(className.equals("jp.co.ricoh.doquedb.jdbcx.DataSource")){
			// 紛らわしいが？こっちがプールされたデータソースが返る
			return loadPoolingDataSource(rf);

		}else{
			System.out.print("return null\n");
			return null;
		}
	}

	private Object loadPoolingDataSource(Reference rf_)
	{
		// 既に存在すればそれを返す
		String dName = getProperty(rf_, "dataSourceName");
		DataSource ds = DataSource.getDataSource(dName);
		if(ds != null)	{
			return ds;
		}
		// 新規作成
		ds = new DataSource();
		ds.setDataSourceName(dName);
		loadBaseDataSource(ds, rf_);
		String ini = getProperty(rf_, "initialConnections");
		if(ini != null)	{
			ds.setInitialConnections(Integer.parseInt(ini));
		}
		String max = getProperty(rf_, "maxConnections");
		if(max != null)	{
			ds.setMaxConnections(Integer.parseInt(max));
		}
		String timeout = getProperty(rf_, "loginTimeout");
		if(timeout != null)	{
			ds.setLoginTimeout(Integer.parseInt(timeout));
		}
		return ds;
	}

	private Object loadConnectionPool(Reference rf_)
	{
		ConnectionPoolDataSource cpd = new ConnectionPoolDataSource();
		return loadBaseDataSource(cpd, rf_);
	}

	Object loadBaseDataSource(SydDataSource ds_, Reference rf_)
	{
		ds_.setDatabaseName(getProperty(rf_, "databaseName"));
		ds_.setPassword(getProperty(rf_, "password"));
		String port = getProperty(rf_, "portNumber");
		if(port != null){
			ds_.setPortNumber(Integer.parseInt(port));
		}
		ds_.setServerName(getProperty(rf_, "serverName"));
		ds_.setUser(getProperty(rf_, "user"));

		String prepareThreshold = getProperty(rf_, "prepareThreshold");
		if(prepareThreshold != null)
			ds_.setPrepareThreshold(Integer.parseInt(prepareThreshold));

		return ds_;
	}

	String getProperty(Reference rf_, String st_)
	{
		RefAddr rAddr = rf_.get(st_);
		if(rAddr == null){
			return null;
		}
		return (String)rAddr.getContent();
	}

}

//
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
