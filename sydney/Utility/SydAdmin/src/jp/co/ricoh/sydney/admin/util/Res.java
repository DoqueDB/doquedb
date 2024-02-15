// -*-Mode: Java; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Res.java -- Res って Resource の最初の 3 文字か…たぶん…。
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

package jp.co.ricoh.sydney.admin.util;

import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

import jp.co.ricoh.sydney.admin.util.log.Log;

public class Res {
    private static final String BUNDLE_NAME = "messages";
    private static final String RESOURCE_COLUMN = "%";

    private static Locale locale = null;
    private static ResourceBundle RESOURCE_BUNDLE = null;

    private Res() {
    }

    public static void locale(String path) {
        locale = new Locale(Locale.getDefault().getLanguage(),Locale.getDefault().getCountry());
        RESOURCE_BUNDLE = ResourceBundle.getBundle(path+"."+BUNDLE_NAME,locale);
    }

    public static void locale(String path,String lang,String country) {
        locale = new Locale(lang,country);
        RESOURCE_BUNDLE = ResourceBundle.getBundle(path+"."+BUNDLE_NAME,locale);
    }

	public static String getVersion()
	{
		String	versionString = "0.0.0";
		try {
			versionString = ResourceBundle.getBundle("jp.co.ricoh.sydney.admin.util.resource.version").getString("ADMIN_TOOL_VERSION");
		} catch (MissingResourceException	mre_) {
			mre_.printStackTrace();
		}
		return versionString;
	}
    
    public static String get(String key) {
        try {
            return RESOURCE_BUNDLE.getString(key);
        } catch (MissingResourceException e) {
            //System.out.println("MissingResourceException="+key);
            return '!' + key + '!';
        }
    }
    
    /**
     * リソースファイルから文字列情報を取得する。
     * 「%順番」により置換機能を装備する。順番は1から9までサポート
     * 必ず1からはじめること。
     * （設定例）
     * KEY_NAME=%1を%2に置換した。
     * Res.get("KEY_NAME",new String[]{"key","KEY"});
     * 結果は「keyをKEYに置換した。」となる。
     * @param key リソースキー名
     * @param value 置換後の値。（配列をサポートする。）
     * @return リソース文字列
     */
    public static String get(String key,String[] value) {
        String strTarget = '!' + key + '!';
        
        try {
            // リソースデータの取得
            strTarget = RESOURCE_BUNDLE.getString(key);

            // %のカウント数
            int i = 0;
			int max = 0;
			for(;;){
			    i = strTarget.indexOf(RESOURCE_COLUMN,i);
			    if( i < 0 ){
			        break;
			    }
			    i++;
			    max++;
			}

            // 置換処理
			// 10個以上超えたらnullを返す。
			if( max < 10 ){
				for(i=0;i<max;i++){
				    String repkey = "%" + String.valueOf(i+1);
				    String repVal = value[i].replaceAll("\\\\", "/");
					repVal = repVal.replaceAll("\\$", "\\\\\\$");
				    strTarget = strTarget.replaceAll(repkey, repVal);
				}
			}
            
        } catch (MissingResourceException e) {
            Log.exception("MissingResourceException="+key);
        } catch (Exception e) {
            Log.exception("Exception="+e.toString());
        }

        return strTarget;

    }

}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
