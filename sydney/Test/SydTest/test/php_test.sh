#!/bin/sh
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# 環境セット
installpath=/proj/sydney/work/`whoami`/opt/RICOHtrm
databasepath=/proj/sydney/work/`whoami`/opt/RICOHtrm/db
LD_LIBRARY_PATH="/usr/local/apache2/modules:/proj/sydney/work/`whoami`/opt/RICOHtrm/lib:$LD_LIBRARY_PATH"
PATH="/usr/local/php5/bin:$PATH"
TEST_PHP_EXECUTABLE="/usr/local/php5/bin/php"
ModParameterPath="/proj/sydney/work/`whoami`/opt/RICOHtrm/etc/mod.conf"
SYDPARAM="/proj/sydney/work/`whoami`/opt/RICOHtrm/etc/default.conf"
SYDPARAD="/proj/sydney/work/`whoami`/opt/RICOHtrm/etc/default.conf.bak"
SYDSYSPARAM="/proj/sydney/work/`whoami`/opt/RICOHtrm/etc/system.conf"

mv $SYDPARAM $SYDPARAD
cp -f $SYDPARAD $SYDPARAM
export LD_LIBRARY_PATH PATH TEST_PHP_EXECUTABLE ModParameterPath SYDPARAM SYDSYSPARAM

PortNumber=`cat $SYDPARAM | grep 'PortNumber' | awk '{print $2}'`
echo Server_PasswordFilePath    \"Sydney\" >> $SYDPARAM

echo "extension_dir=/usr/local/apache2/modules" > /usr/local/php5/php.ini
echo "extension=`whoami`_libphp_pdo_trmeister.so" >> /usr/local/php5/php.ini
echo "trmeister.host=localhost" >> /usr/local/php5/php.ini
echo "trmeister.port=$PortNumber" >> /usr/local/php5/php.ini
chown `whoami`:sydney /usr/local/php5/php.ini
chmod ug+w /usr/local/php5/php.ini  

cp -f ../../../PHP/PDO/c.O34-openssl/libphp_pdo_trmeister.so /usr/local/apache2/modules/`whoami`_libphp_pdo_trmeister.so
chown `whoami`:sydney /usr/local/apache2/modules/`whoami`_libphp_pdo_trmeister.so
chmod ug+w /usr/local/apache2/modules/`whoami`_libphp_pdo_trmeister.so

# main
/proj/sydney/work/`whoami`/opt/RICOHtrm/bin/doquedb start

pushd ../../../ModuleTest/PHP/PDO/UnitTest
sh unittest.bat
cd ../PDO_TEST
php -d open_basedir= -d safe_mode=0 -d output_buffering=0 run-tests.php trmeister_tests
popd

/proj/sydney/work/`whoami`/opt/RICOHtrm/bin/trmeister stop

rm -f /usr/local/apache2/modules/`whoami`_libphp_pdo_trmeister.so
rm -f /usr/local/php5/php.ini
rm -f $SYDPARAM
mv $SYDPARAD $SYDPARAM
rm -rf $databasepath/data
rm -rf $databasepath/system
mkdir -p -m 0755 $databasepath/data
mkdir -p -m 0755 $databasepath/system
cp -r $installpath/save/. $databasepath

exit
#
# Copyright (c) 2001, 2023, 2024 Ricoh Company, Ltd.
# All rights reserved.
#
