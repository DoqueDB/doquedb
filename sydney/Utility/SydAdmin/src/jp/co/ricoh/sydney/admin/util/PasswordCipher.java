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

import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

public class PasswordCipher {
    /**
     * 
     */
    public PasswordCipher() {

    }

    public byte[] getKey() {
//        public String getKey() {
        byte[] raw = null;
        try {
            KeyGenerator kgen = KeyGenerator.getInstance("DES");
            SecretKey skey = kgen.generateKey();
            raw = skey.getEncoded();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        //return this.keyName;
        return raw;
    }
    
//    public byte[] encrypt(String key, String text) {
  public byte[] encrypt(byte[] key,String text) {
        byte[] encrypted = null;
        try {
            DESKeySpec ks = new DESKeySpec(key);
//            DESKeySpec ks = new DESKeySpec(key.getBytes());
            SecretKeyFactory kfact = SecretKeyFactory.getInstance("DES");
            SecretKey sk = kfact.generateSecret(ks);

            Cipher cipher = Cipher.getInstance("DES/ECB/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE,sk);
            encrypted = cipher.doFinal(text.getBytes());
        } catch (Exception e) {
            // TODO 自動生成された catch ブロック
            e.printStackTrace();
        }
        return encrypted;
    }


    public String decrypt(byte[] key,byte[] encrypted) {
//        public String decrypt(String key,byte[] encrypted) {
        String decrypt = null;
        try {
            //DESKeySpec ks = new DESKeySpec(key.getBytes());
            DESKeySpec ks = new DESKeySpec(key);
            SecretKeyFactory kfact = SecretKeyFactory.getInstance("DES");
            SecretKey sk = kfact.generateSecret(ks);
            
            Cipher cipher = Cipher.getInstance("DES/ECB/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE,sk);
	        decrypt = new String(cipher.doFinal(encrypted));
	    } catch (Exception e) {
            e.printStackTrace();
	    }
        return decrypt;
    }

}

//
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
