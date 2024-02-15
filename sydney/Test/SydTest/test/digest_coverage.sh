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
# mount check
( df --block-size=1 h:|grep 11423232 >/dev/null) || (echo "H: incorrect." ; exit)

if [ ! -e x:/ ]; then
  echo "X: drive not mounted."
  exit
fi

if [ ! -e $WINDIR/system32/taskkill.exe \
     -a ! -e "$PROGRAMFILES/Support Tools/kill" ]; then
  echo "Support tools not installed."
  exit
fi

if [ -e d:/dm_bak ]; then
  mv d:/dm_bak d:/dm_bak_bak
  echo "Escaped d:/dm_bak to d:/dm_bak_bak ."
fi

sumi () {
bash SydTest_sh.txt -f 0000 -t 0220 -lcNn single
bash SydTest_sh.txt -f 0294 -t 0320 -lcNn single
bash SydTest_sh.txt -f 0394 -t 0420 -lcNn single
bash SydTest_sh.txt -f 0494 -t 0520 -lcNn single
bash SydTest_sh.txt -f 0594 -t 0620 -lcNn single
bash SydTest_sh.txt -f 0694 -t 0720 -lcNn single
bash SydTest_sh.txt -f 0794 -t 0820 -lcNn single
bash SydTest_sh.txt -f 0894 -t 0920 -lcNn single
bash SydTest_sh.txt -f 0994 -t 1650 -lcNn single
bash SydTest_sh.txt -f 1680 -t 2016 -lcNn single
bash SydTest_sh.txt -s 2600 -lcn single
bash SydTest_sh.txt -s 2630 -lcn single
bash SydTest_sh.txt -f 2920 -t 2952 -lcNn single
bash SydTest_sh.txt -s 3074 -lcn single
bash SydTest_sh.txt -f 3504 -t 4226 -lcNn single
bash SydTest_sh.txt -f 4210 -t 4226 -lcNn single
bash SydTest_sh.txt -f 4300 -t 4922 -lcNn single

bash SydTest_sh.txt -f 0001 -t 2913 -lce single
bash SydTest_sh.txt -f 3501 -t 3727 -lce single
bash SydTest_sh.txt -f 3891 -t 4201 -lce single
bash SydTest_sh.txt -f 4219 -t 4923 -lce single

bash SydTest_sh.txt -f 5000 -t 5060 -lcNn multi
bash SydTest_sh.txt -f 5700 -t 5768 -lcNn multi
bash SydTest_sh.txt -f 5730 -t 5768 -lcNn multi
bash SydTest_sh.txt -f 6000 -t 6078 -lcNn multi
bash SydTest_sh.txt -f 7010 -t 7478 -lcNn multi
}
bash SydTest_sh.txt -f 7274 -t 7478 -lcNn multi
bash SydTest_sh.txt -f 7600 -t 7632 -lcNn multi
bash SydTest_sh.txt -f 8500 -t 9300 -lcNn multi

bash SydTest_sh.txt -s 5031 -lce multi
bash SydTest_sh.txt -f 8121 -t 9301 -lce multi

# hijouni jikanga kakarunode atomawashi
bash SydTest_sh.txt -s 3154 -lcn single
bash SydTest_sh.txt -s 3234 -lcn single
bash SydTest_sh.txt -s 3314 -lcn single

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
