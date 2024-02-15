// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordProjections.java --
// 
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.doquedb.hibernate.criterion;

import jp.co.ricoh.doquedb.hibernate.usertype.WordType;

import org.hibernate.Criteria;
import org.hibernate.Hibernate;
import org.hibernate.HibernateException;
import org.hibernate.type.Type;
import org.hibernate.criterion.AggregateProjection;
import org.hibernate.criterion.CriteriaQuery;


/**
 * WORDを取得するためのクラス
 *
 */
public class WordProjection extends AggregateProjection
{
	/** コンストラクタ。Projectionsからしか生成できません。*/
	protected WordProjection(String propartyName)
	{
		super("word", propartyName);
	}

	/** 返される型を返します。*/
	public Type[] getTypes(Criteria criteria, CriteriaQuery criteriaQuery)
		throws HibernateException
	{
		return new Type[] {
			criteriaQuery.getFactory().getTypeHelper().custom(jp.co.ricoh.doquedb.hibernate.usertype.WordType.class)
		};
	}
}

//
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
