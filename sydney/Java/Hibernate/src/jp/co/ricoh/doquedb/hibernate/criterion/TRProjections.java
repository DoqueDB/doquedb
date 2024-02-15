// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TRProjections.java -- 
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

/**
 * Doquedb専用のプロジェクション
 */
public class TRProjections
{
	/**
	 * スコアを取得する
	 */
	public static ScoreProjection score(String propertyName)
	{
		return new ScoreProjection(propertyName);
	}

	/**
	 * ワードを取得する
	 */
	public static WordProjection word(String propertyName)
	{
		return new WordProjection(propertyName);
	}
	
}

//
// Copyright (c) 2007, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
