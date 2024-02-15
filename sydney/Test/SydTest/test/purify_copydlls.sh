#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
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
ver=v14.0

rm -f "$PROGRAMFILES/Rational/Purify/cache/*"
cp d:/proj/Sydney/$ver/Kernel/*/Purify/*.pdb d:/proj/Sydney/$ver/tools/setup/sydney
cp d:/proj/Sydney/$ver/Driver/*/Purify/*.pdb d:/proj/Sydney/$ver/tools/setup/sydney

pushd c:/proj/Sydney/$ver/Tools/setup
for dir1 in single multi recovery # split
do
    for dir2 in normal except
    do
	for ext in dll pdb exe
	do
		echo $dir1/$dir2
		rm -f d:/proj/Sydney/$ver/test/SydTest/Test/$dir1/$dir2/*.$ext
		cp sydney/*.$ext d:/proj/Sydney/$ver/test/SydTest/Test/$dir1/$dir2
	done
    done
done
popd

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
