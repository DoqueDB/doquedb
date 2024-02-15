//
// unamorph.cpp -
//      形態素解析メインモジュール
//		形態素解析処理のメインのモジュール。
// 
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
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

//--------------------------------------------------------------------------
// 必要なヘッダの読み込み
//--------------------------------------------------------------------------
#include <string.h>				/* memset */
#include <limits.h>				/* ULONG_MAX */
#include <assert.h>				/* デバッグ用 */
#include <stdio.h>				/* デバッグ用 */
#include "UnaBase/unamorph.h"	/* 形態素モジュール自身 */
#include "UnaBase/unamdunk.h"	/* UNA_UNK_HYOKI_LIMIT */

//--------------------------------------------------------------------------
// モジュールとエラー管理
//--------------------------------------------------------------------------
#define MODULE_NAME "UNAMORPH"	/* モジュール名 */

/* モジュール内のメッセージ */
#define ERR_VERSION_CON	"Unsupported Connect Table Version"
#define ERR_BRNCH_SIZE	"Lattice buffer overflow"
#define ERR_PATH		"Morph buffer overflow"

//--------------------------------------------------------------------------
// モジュール内部で使う定義、グローバル変数
//--------------------------------------------------------------------------
#define CANT_CON_COST 255			/* 接続不可を表わす接続コスト値
									   MAX_CON_COSTにマッピング */
#define MAX_CON_COST  65535			/* 接続コストの最大値 */
#define	UNAHIN_GROUP_MASK	0xf000	/* UNA品詞大分類(上位8ビット)取得用 */

int debugFlag = 0;	/* 0以外:デバッグ情報出力, 2:anadbg用出力生成 */

/* モジュール内部で使用する関数のプロトタイプ宣言 */
static int LatInit(unaLatticeT *lat);
static int LatClear(unaLatticeT *lat,int hin,unaHinT unaHin,int inTxtLen);
static int AddPath(unaMorphT *wbuf,int wbufLen,unaLatticeT *lat);
static int DebLatSet(unaMorphHandleT *mh,int oldBroth,int st,int ln, int hin,int cost);
static int PrintLat(unaLatticeT *lat);
static ucharT *UnaToUTF8(unaCharT *uStr, uintT uStrLen);

unsigned short tmpTbl[]={
   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
  90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
 160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
 180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
 210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
 240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
 250, 251, 252, 253, 254, 255, 256, 257, 258, 259,
 260, 261, 262, 263, 264, 265, 266, 267, 268, 269,
 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
 280, 281, 282, 283, 284, 285, 286, 287, 288, 289,
 290, 291, 292, 293, 294, 295, 296, 297, 298, 299,
 300, 301, 302, 303, 304, 305, 306, 307, 308, 309,
 310, 311, 312, 313, 314, 315, 316, 317, 318, 319,
 320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
 330, 331, 332, 333, 334, 335, 336, 337, 338, 339,
 340, 341, 342, 343, 344, 345, 346, 347, 348, 349,
 350, 351, 352, 353, 354, 355, 356, 357, 358, 359,
 360, 361, 362, 363, 364, 365, 366, 367, 368, 369,
 370, 371, 372, 373, 374, 375, 376, 377, 378, 379,
 380, 381, 382, 383, 384, 385, 386, 387, 388, 389,
 390, 391, 392, 393, 394, 395, 396, 397, 398, 399,
 400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
 410, 411, 412, 413, 414, 415, 416, 417, 418, 419,
 420, 421, 422, 423, 424, 425, 426, 427, 428, 429,
 430, 431, 432, 433, 434, 435, 436, 437, 438, 439,
 440, 441, 442, 443, 444, 445, 446, 447, 448, 449,
 450, 451, 452, 453, 454, 455, 456, 457, 458, 459,
 460, 461, 462, 463, 464, 465, 466, 467, 468, 469,
 470, 471, 472, 473, 474, 475, 476, 477, 478, 479,
 480, 481, 482, 483, 484, 485, 486, 487, 488, 489,
 490, 491, 492, 493, 494, 495, 496, 497, 498, 499,
 500, 501, 502, 503, 504, 505, 506, 507, 508, 509,
 510, 511, 512, 513, 514, 515, 516, 517, 518, 519,
 520, 521, 522, 523, 524, 525, 526, 527, 528, 529,
 530, 531, 532, 533, 534, 535, 536, 537, 538, 539,
 540, 541, 542, 543, 544, 545, 546, 547, 548, 549,
 550, 551, 552, 553, 554, 555, 556, 557, 558, 559,
 560, 561, 562, 563, 564, 565, 566, 567, 568, 569,
 570, 571, 572, 573, 574, 575, 576, 577, 578, 579,
 580, 581, 582, 583, 584, 585, 586, 587, 588, 589,
 590, 591, 592, 593, 594, 595, 596, 597, 598, 599,
 600, 601, 602, 603, 604, 605, 606, 607, 608, 609,
 610, 611, 612, 613, 614, 615, 616, 617, 618, 619,
 620, 621, 622, 623, 624, 625, 626, 627, 628, 629,
 630, 631, 632, 633, 634, 635, 636, 637, 638, 639,
 640, 641, 642, 643, 644, 645, 646, 647, 648, 649,
 650, 651, 652, 653, 654, 655, 656, 657, 658, 659,
 660, 661, 662, 663, 664, 665, 666, 667, 668, 669,
 670, 671, 672, 673, 674, 675, 676, 677, 678, 679,
 680, 681, 682, 683, 684, 685, 686, 687, 688, 689,
 690, 691, 692, 693, 694, 695, 696, 697, 698, 699,
 700, 701, 702, 703, 704, 705, 706, 707, 708, 709,
 710, 711, 712, 713, 714, 715, 716, 717, 718, 719,
 720, 721, 722, 723, 724, 725, 726, 727, 728, 729,
 730, 731, 732, 733, 734, 735, 736, 737, 738, 739,
 740, 741, 742, 743, 744, 745, 746, 747, 748, 749,
 750, 751, 752, 753, 754, 755, 756, 757, 758, 759,
 760, 761, 762, 763, 764, 765, 766, 767, 768, 769,
 770, 771, 772, 773, 774, 775, 776, 777, 778, 779,
 780, 781, 782, 783, 784, 785, 786, 787, 788, 789,
 790, 791, 792, 793, 794, 795, 796, 797, 798, 799,
 800, 801, 802, 803, 804, 805, 806, 807, 808, 809,
 810, 811, 812, 813, 814, 815, 816, 817, 818, 819,
 820, 821, 822, 823, 824, 825, 826, 827, 828, 829,
 830, 831, 832, 833, 834, 835, 836, 837, 838, 839,
 840, 841, 842, 843, 844, 845, 846, 847, 848, 849,
 850, 851, 852, 853, 854, 855, 856, 857, 858, 859,
 860, 861, 862, 863, 864, 865, 866, 867, 868, 869,
 870, 871, 872, 873, 874, 875, 876, 877, 878, 879,
 880, 881, 882, 883, 884, 885, 886, 887, 888, 889,
 890, 891, 892, 893, 894, 895, 896, 897, 898, 899,
 900, 901, 902, 903, 904, 905, 906, 907, 908, 909,
 910, 911, 912, 913, 914, 915, 916, 917, 918, 919,
 920, 921, 922, 923, 924, 925, 926, 927, 928, 929,
 930, 931, 932, 933, 934, 935, 936, 937, 938, 939,
 940, 941, 942, 943, 944, 945, 946, 947, 948, 949,
 950, 951, 952, 953, 954, 955, 956, 957, 958, 959,
 960, 961, 962, 963, 964, 965, 966, 967, 968, 969,
 970, 971, 972, 973, 974, 975, 976, 977, 978, 979,
 980, 981, 982, 983, 984, 985, 986, 987, 988, 989,
 990, 991, 992, 993, 994, 995, 996, 997, 998, 999,
 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009,
 1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019,
 1020, 1021, 1022, 1023, 1024, 1025, 1026, 1027, 1028, 1029,
 1030, 1031, 1032, 1033, 1034, 1035, 1036, 1037, 1038, 1039,
 1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049,
 1050, 1051, 1052, 1053, 1054, 1055, 1056, 1057, 1058, 1059,
 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069,
 1070, 1071, 1072, 1073, 1074, 1075, 1076, 1077, 1078, 1079,
 1080, 1081, 1082, 1083, 1084, 1085, 1086, 1087, 1088, 1089,
 1090, 1091, 1092, 1093, 1094, 1095, 1096, 1097, 1098, 1099,
 1100, 1101, 1102, 1103, 1104, 1105, 1106, 1107, 1108, 1109,
 1110, 1111, 1112, 1113, 1114, 1115, 1116, 1117, 1118, 1119,
 1120, 1121, 1122, 1123, 1124, 1125, 1126, 1127, 1128, 1129,
 1130, 1131, 1132, 1133, 1134, 1135, 1136, 1137, 1138, 1139,
 1140, 1141, 1142, 1143, 1144, 1145, 1146, 1147, 1148, 1149,
 1150, 1151, 1152, 1153, 1154, 1155, 1156, 1157, 1158, 1159,
 1160, 1161, 1162, 1163, 1164, 1165, 1166, 1167, 1168, 1169,
 1170, 1171, 1172, 1173, 1174, 1175, 1176, 1177, 1178, 1179,
 1180, 1181, 1182, 1183, 1184, 1185, 1186, 1187, 1188, 1189,
 1190, 1191, 1192, 1193, 1194, 1195, 1196, 1197, 1198, 1199,
 1200, 1201, 1202, 1203, 1204, 1205, 1206, 1207, 1208, 1209,
 1210, 1211, 1212, 1213, 1214, 1215, 1216, 1217, 1218, 1219,
 1220, 1221, 1222, 1223, 1224, 1225, 1226, 1227, 1228, 1229,
 1230, 1231, 1232, 1233, 1234, 1235, 1236, 1237, 1238, 1239,
 1240, 1241, 1242, 1243, 1244, 1245, 1246, 1247, 1248, 1249,
 1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258, 1259,
 1260, 1261, 1262, 1263, 1264, 1265, 1266, 1267, 1268, 1269,
 1270, 1271, 1272, 1273, 1274, 1275, 1276, 1277, 1278, 1279,
 1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289,
 1290, 1291, 1292, 1293, 1294, 1295, 1296, 1297, 1298, 1299,
 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309,
 1310, 1311, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319,
 1320, 1321, 1322, 1323, 1324, 1325, 1326, 1327, 1328, 1329,
 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339,
 1340, 1341, 1342, 1343, 1344, 1345, 1346, 1347, 1348, 1349,
 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359,
 1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369,
 1370, 1371, 1372, 1373, 1374, 1375, 1376, 1377, 1378, 1379,
 1380, 1381, 1382, 1383, 1384, 1385, 1386, 1387, 1388, 1389,
 1390, 1391, 1392, 1393, 1394, 1395, 1396, 1397, 1398, 1399,
 1400, 1401, 1402, 1403, 1404, 1405, 1406, 1407, 1408, 1409,
 1410, 1411, 1412, 1413, 1414, 1415, 1416, 1417, 1418, 1419,
 1420, 1421, 1422, 1423, 1424, 1425, 1426, 1427, 1428, 1429,
 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439,
 1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, 1449,
 1450, 1451, 1452, 1453, 1454, 1455, 1456, 1457, 1458, 1459,
 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, 1469,
 1470, 1471, 1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479,
 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487, 1488, 1489,
 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499,
 1500, 1501, 1502, 1503, 1504, 1505, 1506, 1507, 1508, 1509,
 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519,
 1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529,
 1530, 1531, 1532, 1533, 1534, 1535, 1536, 1537, 1538, 1539,
 1540, 1541, 1542, 1543, 1544, 1545, 1546, 1547, 1548, 1549,
 1550, 1551, 1552, 1553, 1554, 1555, 1556, 1557, 1558, 1559,
 1560, 1561, 1562, 1563, 1564, 1565, 1566, 1567, 1568, 1569,
 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579,
 1580, 1581, 1582, 1583, 1584, 1585, 1586, 1587, 1588, 1589,
 1590, 1591, 1592, 1593, 1594, 1595, 1596, 1597, 1598, 1599,
 1600, 1601, 1602, 1603, 1604, 1605, 1606, 1607, 1608, 1609,
 1610, 1611, 1612, 1613, 1614, 1615, 1616, 1617, 1618, 1619,
 1620, 1621, 1622, 1623, 1624, 1625, 1626, 1627, 1628, 1629,
 1630, 1631, 1632, 1633, 1634, 1635, 1636, 1637, 1638, 1639,
 1640, 1641, 1642, 1643, 1644, 1645, 1646, 1647, 1648, 1649,
 1650, 1651, 1652, 1653, 1654, 1655, 1656, 1657, 1658, 1659,
 1660, 1661, 1662, 1663, 1664, 1665, 1666, 1667, 1668, 1669,
 1670, 1671, 1672, 1673, 1674, 1675, 1676, 1677, 1678, 1679,
 1680, 1681, 1682, 1683, 1684, 1685, 1686, 1687, 1688, 1689,
 1690, 1691, 1692, 1693, 1694, 1695, 1696, 1697, 1698, 1699,
 1700, 1701, 1702, 1703, 1704, 1705, 1706, 1707, 1708, 1709,
 1710, 1711, 1712, 1713, 1714, 1715, 1716, 1717, 1718, 1719,
 1720, 1721, 1722, 1723, 1724, 1725, 1726, 1727, 1728, 1729,
 1730, 1731, 1732, 1733, 1734, 1735, 1736, 1737, 1738, 1739,
 1740, 1741, 1742, 1743, 1744, 1745, 1746, 1747, 1748, 1749,
 1750, 1751, 1752, 1753, 1754, 1755, 1756, 1757, 1758, 1759,
 1760, 1761, 1762, 1763, 1764, 1765, 1766, 1767, 1768, 1769,
 1770, 1771, 1772, 1773, 1774, 1775, 1776, 1777, 1778, 1779,
 1780, 1781, 1782, 1783, 1784, 1785, 1786, 1787, 1788, 1789,
 1790, 1791, 1792, 1793, 1794, 1795, 1796, 1797, 1798, 1799,
 1800, 1801, 1802, 1803, 1804, 1805, 1806, 1807, 1808, 1809,
 1810, 1811, 1812, 1813, 1814, 1815, 1816, 1817, 1818, 1819,
 1820, 1821, 1822, 1823, 1824, 1825, 1826, 1827, 1828, 1829,
 1830, 1831, 1832, 1833, 1834, 1835, 1836, 1837, 1838, 1839,
 1840, 1841, 1842, 1843, 1844, 1845, 1846, 1847, 1848, 1849,
 1850, 1851, 1852, 1853, 1854, 1855, 1856, 1857, 1858, 1859,
 1860, 1861, 1862, 1863, 1864, 1865, 1866, 1867, 1868, 1869,
 1870, 1871, 1872, 1873, 1874, 1875, 1876, 1877, 1878, 1879,
 1880, 1881, 1882, 1883, 1884, 1885, 1886, 1887, 1888, 1889,
 1890, 1891, 1892, 1893, 1894, 1895, 1896, 1897, 1898, 1899,
 1900, 1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908, 1909,
 1910, 1911, 1912, 1913, 1914, 1915, 1916, 1917, 1918, 1919,
 1920, 1921, 1922, 1923, 1924, 1925, 1926, 1927, 1928, 1929,
 1930, 1931, 1932, 1933, 1934, 1935, 1936, 1937, 1938, 1939,
 1940, 1941, 1942, 1943, 1944, 1945, 1946, 1947, 1948, 1949,
 1950, 1951, 1952, 1953, 1954, 1955, 1956, 1957, 1958, 1959,
 1960, 1961, 1962, 1963, 1964, 1965, 1966, 1967, 1968, 1969,
 1970, 1971, 1972, 1973, 1974, 1975, 1976, 1977, 1978, 1979,
 1980, 1981, 1982, 1983, 1984, 1985, 1986, 1987, 1988, 1989,
 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999};

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_init
//
// ABSTRACT:	  形態素関連の初期化
//
// FUNCTION:
//	  形態素関連の初期化を行う
//
// RETURN:
//	  UNA_OK	正常終了
//	  負の値	エラー
//
// NOTE:
//	  なし
//
int unaMorph_init(
		unaMorphHandleT *mh,	/* ハンドラ */
		const char *cnctTblImg,	/* 接続表 */
		unsigned int maxWordLen /* 最大単語長 */
)
{
	const char *imgPtr;	/* 接続表を格納したメモリへのポインタ */
	int  ver; 			/* version番号 */

	/* 接続表の情報を設定する */
	mh->cnctTblImg = cnctTblImg;
	imgPtr = cnctTblImg + UNA_COM_SIZE;
	ver = 116;
	if (strcmp(imgPtr,UNA_CON_VER)!=0){ /* バージョンが違う */
		if (strcmp(imgPtr,UNA_CON_VER_115)==0){ /* 非圧縮バージョン */
			ver = 115;
		}
		else{
			UNA_RETURN(ERR_VERSION_CON,NULL);
		}
	}
	imgPtr += UNA_VER_SIZE;
	mh->morpHinNumMax = *(uintT *)imgPtr;	/* 品詞数 */

	/* バイトオーダーの違うファイルをここではじく */
	if ( mh->morpHinNumMax >= 0x10000){
		UNA_RETURN(ERR_VERSION_CON,NULL);
	}

	imgPtr += sizeof(uintT);

	/* UNA品詞番号への変換テーブル*/
	mh->unaHinNumTable	= (const ushortT *)imgPtr;
	imgPtr += (mh->morpHinNumMax * sizeof(ushortT));

	if ( ver == 116){
		mh->kakariNumMax = *(uintT *)imgPtr;
		imgPtr += sizeof(uintT);
		mh->ukeNumMax = *(uintT *)imgPtr;
		imgPtr += sizeof(uintT);
		mh->kakari = (const ushortT*)imgPtr;
		imgPtr += (mh->morpHinNumMax * sizeof(ushortT));
		mh->uke = (const ushortT*)imgPtr;
		imgPtr += (mh->morpHinNumMax * sizeof(ushortT));
		mh->connectCostTable = (const ucharT*)imgPtr;	/* 接続コストテーブル */
		imgPtr += ((mh->kakariNumMax)*(mh->ukeNumMax)*sizeof(ucharT));
	}
	else{
		mh->kakariNumMax = (mh->morpHinNumMax);
		mh->ukeNumMax = (mh->morpHinNumMax);
		mh->kakari = (const ushortT*)tmpTbl;
		mh->uke = (const ushortT*)tmpTbl;
		mh->connectCostTable = (const ucharT*)imgPtr;	/* 接続コストテーブル */
		imgPtr += ((mh->morpHinNumMax)*(mh->morpHinNumMax)*sizeof(ucharT));
	}
	mh->hinNamePos = (int*)imgPtr;		/* 品詞番号→品詞プール中の位置 */

	imgPtr += (mh->morpHinNumMax * sizeof(int));
	mh->hinNamePool = (unaCharT*)imgPtr;		/* 品詞プール */

	/* 仮想文末品詞を求めておく */
	mh->hinBunEnd = UNA_HIN_KUTEN;

	/* 前接形態素の形態素品詞番号をセットする(一番始めは仮想文末品詞) */
	mh->maeHin = mh->hinBunEnd;

	mh->mwLen = UNA_HYOKI_LEN_MAX;
	if ( maxWordLen >0 && maxWordLen<UNA_HYOKI_LEN_MAX){
		mh->mwLen = maxWordLen;
	}

	/* ラティス領域の初期化 */
	LatInit(&mh->lat);

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_term
//
// ABSTRACT:	  形態素関連の終了処理
//
// FUNCTION:
//	  形態素関連の終了処理を行う
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//
int unaMorph_term(
		unaMorphHandleT *mh		/* ハンドラ */
)
{
	/* 値のリセット */
	mh->cnctTblImg			= (const char *)NULL;
	mh->morpHinNumMax		= 0;
	mh->unaHinNumTable		= (const unaHinT *)NULL;
	mh->connectCostTable	= (const ucharT*)NULL;
	mh->hinBunEnd			= 0;
	mh->maeHin				= 0;
	mh->mwLen = 0;
	LatInit(&mh->lat);

	mh->kakariNumMax = 0;
	mh->ukeNumMax = 0;
	mh->kakari = (const ushortT*)NULL;
	mh->uke = (const ushortT*)NULL;
	
	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_setDebugFlag
//
// ABSTRACT:  デバッグフラグの設定
//
// FUNCTION:
//	  デバッグフラグを設定する
//      0: デバッグ表示なし
//      1: ラティス登録時のコスト情報を表示する
//      2: ラティス情報を表示する (anadbg用)
//      3: ラティス情報と登録時の詳細情報を表示する
//
// RETURN:
//	  UNA_OK	正常終了
//	  UNA_STOP	中断
//
// NOTE:
//
int unaMorph_setDebugFlag(int flag)
{
	if (flag < 0 || flag > 3) {
		return UNA_STOP;
	}
	debugFlag = flag;
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_gen
//
// ABSTRACT:	  形態素解析をする
//
// FUNCTION:
//	  テキスト中の指定した範囲を形態素解析する。
//
// RETURN:
//	  正の値				生成した形態素の数
//	  UNA_STOP				中断
//	  負の値(UNA_STOP以外)	エラー番号
//
// NOTE:
//	  UNAライブラリモジュールの中では、形態素解析を行うメインモジュール
//	  という位置付けにある
//
//	  (注1)
//		収束が確定していても終了位置判定を先に行う。これは、終了位置なら
//		次回の前接品詞を仮想文節頭末とするため。
//	  (注2)
//		まず、仮想文節頭末をテキストの終わりに一時的に付加して形態素ラティス
//		を強引に収束させる。
//		なお、考えられるlatticeEndの最大値は UNA_LOCAL_TEXT_SIZE である。
//		(→ unaMorph_latSet参照)
//	  (注3)
//		txtPos=3 なら、その直前 txtPos=2 で終わった形態素があるかどうか
//		ということをチェックする。brnchIndex は [1] 舛覆里 brnchIndex[2]
//		の内容を判断するのではなく、brnchIndex[3] の内容を判断する。
//	  (注4)
//		この処理は同位置からの同長未登録語を生成しないために行う。
//		[1][UNA_UNK_HYOKI_LIMIT]を使用する
//	  (注5)
//		戻り値はこれと UNA_OK だけ UNA_SYUSOKU が返ってくる時は、
//		オーバーフローまたは優先登録あり
//	  (注6)
//		次の2つをもって収束としている
//		(1)ある位置まで解析した時の形態素の末尾がそろっている
//		(2)その次に引けた形態素がただ1つである
//		(1)は、curLatEnd == txtPos により判断できる。なぜなら、
//		curLatEnd > txtPos なら、まだ解析していない部分文字列があるわけで、
//		そこを解析した結果末尾位置が更新されるということが考えられるから
//		である。具体的には、txtPos==3 迄解析してcurLatEnd==6 とすると、
//		txtPos==4,5の場合を解析してみてcurLatEnd==6 なら初めて(1)の条件が
//		成立するといえる。(解析して、curLatEnd==8などになったらだめ)
//		なお、curLatEndは、1起算のため、curLatEnd==6の位置は txtPosでいうと
//		5である。
//
int unaMorph_gen(
	unaMorphHandleT *mh,	/* ハンドラ */
	unaMorphT *wbuf,		/* 形態素解析結果が格納される配列 */
	int wbufLen,			/* 形態素解析結果が格納される配列の要素数 */
	unaCharT *tbuf,			/* 入力テキストが格納されたバッファ */
	int inTxtLen,			/* 入力テキストの長さ */
	unaFuncClT *dFunc,		/* 辞書検索関数配列(複数の検索関数を設定可) */
	int *processedTxtLen,	/* 形態素解析されたテキストの長さ(文字数) */
	unaStopFuncT stopFunc	/* 中断関数 */
)
{
	unaLatticeT *l;	 	 /* 形態素ラティス */
	int rv;				 /* 関数の返り値 */
	int i;				 /* ループ変数 */
	int txtPos;			 /* テキスト上で解析を行う開始位置(オフセット) */
	int wbufPos;		 /* 形態素解析結果バッファの現在位置 */
	int syusokuFlg;		 /* 収束したらUNA_TRUEそれ以外UNA_FALSE */
	int curLatEnd;		 /* 辞書引きした最先端の位置が入る */
	int cnt;			 /* 形態素候補末尾がある位置でそろった時の次の形態素
						    候補の数(接続コストとのからみで収束判定に必要)*/
	int morCnt;			 /* ラティスに格納された形態素数 */
	int ed;				 /* 形態素の文字列上での末尾位置(1起算) */
	int prIdx;			 /* 優先登録形態素を検出した検定関数の辞書番号 */
	unaHinT unaHin;		 /* UNA品詞 */

	/* 中断要求があれば解析を中止して戻る */
	if (stopFunc()) {
		return UNA_STOP;
	}

	/* 初期設定 */
	l = &mh->lat;		/* ラティス領域のセット */
	l->tbuf = tbuf;		/* テキストへのポインタのセット */
	prIdx = 0;
	wbufPos = 0;
	syusokuFlg = UNA_FALSE;

	/* ラティスをクリア */
	unaMorph_getUnaHin(mh, mh->maeHin, &unaHin);
	rv = LatClear(l,mh->maeHin,unaHin,inTxtLen);
	assert(rv == UNA_OK);

	/* 以下文字列の終わりまで解析を繰り返す */
	for (txtPos = 0;;txtPos++) {	/* forever */
		/* 終了位置にきたら終了する */
		if (txtPos >= l->txtLen || tbuf[txtPos] == 0) {	/* 注1 */
			assert(l->latticeEnd <= UNA_LOCAL_TEXT_SIZE);	/* 注2 */

			(void)unaMorph_latSet(mh,l->latticeEnd,1,mh->hinBunEnd,
									0,0,0,UNAMORPH_DEFAULT_PRIO,UNA_FALSE);
			(void)unaMorph_linkWithParent(mh,txtPos,l->curBrnchPos,
														l->curBrnchPos);
			if (debugFlag == 2 || debugFlag == 3) {
				/* デバッグ情報を出力する */
				fprintf(stdout,"----- Return after AddPath because text end -----\n");
			}
			rv = AddPath(wbuf,wbufLen,l);	/* 最後の分のパスを掃き出す */
			if (rv < 0) {	/* エラー */
				return rv;	/*@ AddPathがエラーを返すテストは下で行った */
			}
			wbufPos += (rv - 1); /* 最後に付加した仮想文節頭末の分は引く */
			*processedTxtLen = l->latticeEnd - 1; /*@ 同上 */
			mh->maeHin = mh->hinBunEnd;	/* 次回の前接形態素の形態素品詞番号*/
			if ( wbufPos>0){
				mh->maeHin = wbuf[wbufPos-1].hinshi;
			}
			break;
		}

		/* ラティスが収束したら終了する */
		if (syusokuFlg == UNA_TRUE) {
			if (debugFlag == 2 || debugFlag == 3) {
				/* デバッグ情報を出力する */
				if ((uintT)(l->latticeEnd)>=l->txtLen || tbuf[l->latticeEnd]==0){
					fprintf(stdout,"----- Return after AddPath because text end -----\n");
				}
				else {
					fprintf(stdout,"----- Return after AddPath because SYUSOKU -----\n");
				}
			}
			rv = AddPath(wbuf,wbufLen,l);	/* 最後の分のパスを掃き出す */
			if (rv < 0) {	/* エラー */
				return rv;
			}
			wbufPos += rv;	/* 形態素列の書き込み位置を更新 */
			*processedTxtLen = l->latticeEnd;	/* 解析済テキスト長をセット */
			assert(l->latticeEnd <= l->txtLen);
			
			/* 次回の前接形態素の形態素品詞番号をセット */
			mh->maeHin = mh->hinBunEnd;	/*デフォルトは仮想文節頭末*/
			if (l->latticeEnd < l->txtLen && tbuf[l->latticeEnd] != 0){
				mh->maeHin =(l->morBrnch)[l->curBrnchPos].hin; /*末尾品詞*/
			}
			break;
		}

		/* 該当位置で終了する形態素がなければ辞書引きしない(注3) */
		if ((l->brnchIndex)[txtPos] == UNAMORPH_LAT_BRNCH_MAX) {
			continue;
		}

		/* ループ前初期設定 */
		memset(&((l->morChk)[1]),0x00,UNA_UNK_HYOKI_LIMIT);/* (注4) */
		curLatEnd = l->latticeEnd;			/* ラティス収束チェック用 */
		cnt = 0;								/* ラティス収束チェック用 */
		l->stBrnchPos = (ucharT)(l->curBrnchPos + 1); /* 優先登録処理用*/
		l->prBrnchPos = 0;							  /* 優先登録処理用*/

		/* 用意した辞書の数だけ辞書引き */
		for (i = 0; dFunc[i].func != 0; ++i) {
			rv=(*(unaDicFuncT)dFunc[i].func)(mh,txtPos,i,&morCnt,dFunc[i].arg);
			if (rv == UNA_SYUSOKU) { /* 注5 */
				prIdx = i;
				syusokuFlg = UNA_TRUE;
				break;
			}
			cnt += morCnt;
		}

		/* 親形態素とリンクを張る */
		if (l->prBrnchPos != 0) {	/* 優先登録語あり */
			(void)(*(unaPrioFuncT)dFunc[prIdx].func2)(mh,txtPos,&ed,
									dFunc[prIdx].arg);
		}
		else {
			(void)unaMorph_linkWithParent(mh,txtPos,l->stBrnchPos,
									l->curBrnchPos);
		}

		/* 収束判定(注6) */
		if (curLatEnd == txtPos && cnt == 1){
			syusokuFlg = UNA_TRUE;
		}
	}
	/* 格納した形態素の数を返す */
	return wbufPos;
}


//--------------------------------------------------------------------------
// MODULE:	  unaMorph_latSet
//
// ABSTRACT:    形態素バッファへの候補セット
//
// FUNCTION:
//	  形態素枝バッファに辞書引き候補をセットする。
//
// RETURN:
//	  UNA_OK		正常終了
//	  上記以外		エラー
//
// NOTE:
//	  - コメント中の兄弟とは末尾位置を等しくする形態素の事であり、図で示すと
//
//		これはテキストの例です。
//                  ↓brnchIndex[7]によりポイントされる(9だとすると)
//				   morBrnch[9]
//                  ↓morBrnch[9].Obrotherによりポイントされる(6だとすると)
//				   morBrnch[6]
//                  ↓morBrnch[6].Obrotherによりポイントされる(4だとすると)
//				   morBrnch[4]
//                    突端のmorBrnch[4].Obrotherは自分自身をポイント(4)
//
//	  - なお優先登録語(連語)も1つの形態素としていったんラティスに登録される。
//		その場合、兄弟枝ともリンクされる。
//
//	(注1)
//		強制収束時には、仮想文節頭末がテキスト末尾に付加された形で
//		ラティスへの登録が行われるが、その時考えられる st の最大値は
//		UNA_LOCAL_TEXT_SIZE である。よってその時の、ed の最大値は
//		UNA_LOCAL_TEXT_SIZE + 1 となる
//
//  (注2)
//		強制収束時には必ず仮想文節頭末が登録されなければならないので
//		登録終了後にオーバーフロー判定を行う。さらにその後、文末発生によって
//		１語分の登録可能性があるのでBRNCH_MAX-1で判定する。
//
//		なお、連語が存在した場合は展開し直す事によりオーバーフローを回避
//		できる可能性があるがそこまでの対応はしていない
//
//  V1.6 複数辞書対応 2017.10.11
//  	複数辞書対応により、優先度の高い辞書の同形語がすでに登録されていたら、
//  	優先度の低い辞書の登録語は登録しないで返るよう処理を変更した。
//		ただし、英語トークン、未登録語、タグから呼び出されるときは、
//		辞書優先度が1であり、必ず登録される。
//
int unaMorph_latSet(
	unaMorphHandleT *mh,	/* ハンドラ */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int ln,					/* 長さ */
	int hin,				/* 品詞番号 */
	int cost,				/* コスト */
	uintT appI,				/* アプリケーションインデックス情報 */
	uintT subI,				/* 下位構造情報 */
	ucharT dicPrio,			/* 辞書優先度 */
	int prioFlg				/* 優先登録フラグ */
)
{
	unaLatticeT *lat;		/* 形態素枝バッファ */
	int broth;				/* 兄弟枝へのポインタ */
	int ed;					/* 形態素の文字列上での末尾位置(オフセット) */
	unaHinT unaHin;			/* UNA品詞番号 */

	/* 無効語のコストは強制的に0とする */
	if (hin == LOCAL_HIN_VOID) {
		cost = 0;
	}

	/* 形態素品詞からUNA品詞を取得する */
	if (hin < mh->morpHinNumMax) {
		unaMorph_getUnaHin(mh, (ushortT)hin, &unaHin);
	} else {
		/* 無効語(LOCAL_HIN_VOID)のとき */
		unaHin = 0;
	}

	/* ラティスの設定 */
	lat = &(mh->lat);
	
	/* 兄弟リンクを張り替えるかもしれないのであらかじめ兄弟枝を調べておく */
	ed = st + ln;	/* ed はテキスト上での形態素末尾位置(オフセット)となる */
	assert(ed >= 0);
	assert(ed <= UNA_LOCAL_TEXT_SIZE + 1);	/* 注1 */

	if ((lat->brnchIndex)[ed] == UNAMORPH_LAT_BRNCH_MAX) {
		/* まだ兄弟枝なし */
		/* curBrnchPosをカウントアップ(オーバーフローチェックは後でやる) */
		(lat->curBrnchPos)++;	/* オーバーフローしないはず */
		assert(lat->curBrnchPos <= UNAMORPH_LAT_BRNCH_MAX);
		broth = lat->curBrnchPos;		/* 兄弟は自分 */
	}
	else {
		/* 既に兄弟枝あり */
		broth = (lat->brnchIndex)[ed];	/* 先頭の兄弟を取得 */

		/* 辞書優先度の高い同形かつ同品詞の語があったら登録せずに戻る */
		int n = broth;
		while (1) {
			unaBrnchT *p = &(lat->morBrnch[n]);
			if (p->st == st && p->dicPrio < dicPrio &&
				(p->unaHin & UNAHIN_GROUP_MASK) == (unaHin & UNAHIN_GROUP_MASK)) {
				/* 開始位置が同じ(同形語)で辞書優先度が高く、*/
				/* かつ品詞大分類が同一の候補があった */
				if (debugFlag == 3) {
					/* デバッグ情報を出力する */
					fprintf(stdout, "latSet:\"%s\" hin=%d unaHin=%04x cost=%d"
							" appI=%X(dicNum=%d) subI=%X prio=%d =>not registered\n",
							UnaToUTF8(&((lat->tbuf)[st]), ln), hin, unaHin, cost,
							appI, (appI>>24), subI, dicPrio);
					fflush(stdout);
				}
				return UNA_OK;
			}
			if (p->Obrother == n) {
				/* 兄弟枝が尽きた */
				break;
			}
			n = p->Obrother;
		}

		/* curBrnchPosをカウントアップ(オーバーフローチェックは後でやる) */
		(lat->curBrnchPos)++;	/* オーバーフローしないはず */
		assert(lat->curBrnchPos <= UNAMORPH_LAT_BRNCH_MAX);
	}
	if (debugFlag == 3) {
		/* デバッグ情報を出力する */
		fprintf(stdout,
			"latSet:\"%s\" hin=%d unaHin=%04x cost=%d appI=%X(dicNum=%d) subI=%X prio=%d\n",
			UnaToUTF8(&((lat->tbuf)[st]), ln), hin, unaHin,
			cost, appI, (appI>>24), subI, dicPrio);
	}

	/* 実際に形態素を格納する(自分が先頭の兄弟になる) */
	(lat->morBrnch)[lat->curBrnchPos].st = (sshortT)st;
	(lat->morBrnch)[lat->curBrnchPos].ln = (ucharT)ln;
	(lat->morBrnch)[lat->curBrnchPos].hin = (ushortT)hin;
	(lat->morBrnch)[lat->curBrnchPos].unaHin = unaHin;
	(lat->morBrnch)[lat->curBrnchPos].cost = (ushortT)cost;
	(lat->morBrnch)[lat->curBrnchPos].acumCost
								= UNA_MAX_ACUM_COST; /* 最大値を入れておく */
	(lat->morBrnch)[lat->curBrnchPos].parent = lat->curBrnchPos;
	(lat->morBrnch)[lat->curBrnchPos].appI = appI;
	(lat->morBrnch)[lat->curBrnchPos].subI = subI;
	(lat->morBrnch)[lat->curBrnchPos].dicPrio = dicPrio;
	(lat->morBrnch)[lat->curBrnchPos].Obrother = (ucharT)broth;

	/* 文字位置からのインデックス情報の変更 */
	(lat->brnchIndex)[ed] = lat->curBrnchPos;	/* 自分が先頭の兄弟になる */
	if (ed > lat->latticeEnd) {
		lat->latticeEnd = ed;					/* 形態素末尾位置の更新 */
	}

	/* この長さの形態素を登録したというフラグを立てる */
	/* (このフラグ配列は入力のある位置からの辞書引き開始時に初期化され、 */
	/*  ある長さの辞書語が見つかったときにフラグが立ち、未登録語処理で   */
	/*  それと同じ長さの未登録語を登録しないようにしている)              */
	/* 無効語は“登録済み”とせず、同形の未登録語が登録されるようにする */
	if (hin != LOCAL_HIN_VOID) {
		(lat->morChk)[ln] = UNA_TRUE;
	}

	/* 優先登録の処理 */
	if (prioFlg == UNA_TRUE) {
		lat->prBrnchPos = lat->curBrnchPos;
	}

	/* 格納オーバーフローのチェック(注2) */
	if (lat->curBrnchPos >= UNAMORPH_LAT_BRNCH_MAX-1) {
		(void)unaMorph_resetLatEnd(lat,ed);
		UNA_RETURN(ERR_BRNCH_SIZE,NULL);	/* この返り値を返すと上位関数
												で収束したものとみなす */
	}

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_linkWithParent
//
// ABSTRACT:    親形態素とリンクする
//
// FUNCTION:
//	  コスト最小法を用いて親形態素へのリンクを張る
//
// RETURN:
//	  UNA_OK		正常終了
//
// NOTE:
//	  - 親を捜す形態素は、lat.morBrnch[stPos][endPos]である
//	  - アルゴリズムはコスト最小法を用いる。
//	  - 前接形態素が4つで親を捜す形態素が3つの場合は、4*3=12回の計算が
//		必要である。
//	  - 親を捜す形態素は、テキスト上の同一位置から始まっていなければならない
//		(下記図参照)
//
//		前接形態素		今回登録した形態素
//			------		-----
//			  ----		-------
//		   -------		--------
//			    --		↑
//						開始位置は等しい
//
int unaMorph_linkWithParent(
	unaMorphHandleT	*mh,	/* 形態素解析メインモジュールハンドル領域 */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int stPos,				/* ループの開始位置(brnchIndexの要素番号) */
	int endPos				/* ループの終了位置(brnchIndexの要素番号) */
)
{
	unaLatticeT *lat;		/* 形態素枝バッファ */
	int i;					/* ループ変数 */
	int oldBroth;			/* 兄弟枝へのポインタ */
	int youngBroth;			/* 兄弟枝へのポインタ */
	ushortT conCost;		/* 接続コスト */
	uintT acumCost;			/* 累積コスト */
	uintT ukeCode;			/* うけコード */
	unaBrnchT *mb;			/* 対象となっている形態素枝 */
	unaBrnchT *iMor;		/* i番目の形態素枝 */
	uintT debugCost[UNAMORPH_LAT_BRNCH_MAX][UNAMORPH_LAT_BRNCH_MAX]; /* デバッグ用 */

	/* 初期設定 */
	lat	= &(mh->lat);		/* ラティスの設定 */
	mb  = lat->morBrnch;

	for (i = stPos;i <= endPos;i++) {
		/* 親枝との接続 */
		assert(st >= 0);
		assert(st <= UNA_LOCAL_TEXT_SIZE);
		iMor = mb+i;
		/* 自分が無効語なら親ノードと接続しない(孤立ノードとなる) */
		if (iMor->hin == LOCAL_HIN_VOID) {
			iMor->acumCost = 0; /* 累積コストは参照されないので0としておく */
			iMor->parent = i; /* 親は自分自身 */
			continue;
		}
		oldBroth = (lat->brnchIndex)[st]; /* 前接形態素(st+1文字目始まりの
					形態素の前接形態素はst文字目終わりだから[st])をセット */
		ukeCode = (mh->uke)[iMor->hin];
		assert(ukeCode !=9999); /* 誤った品詞番号を意味する */
		do {
			/* 前接形態素が無効語なら対象としない */
			if (mb[oldBroth].hin == LOCAL_HIN_VOID) {
				youngBroth = oldBroth;
				oldBroth = mb[oldBroth].Obrother;
				continue;
			}

			/* コストの計算 */
			assert((mh->kakari)[mb[oldBroth].hin] != 9999);
			conCost = mh->connectCostTable[
				(mh->kakari)[ mb[oldBroth].hin]*(mh->ukeNumMax) + ukeCode];
			if (conCost == CANT_CON_COST) {
				conCost = MAX_CON_COST;
						/* CANT_CON_COSTはMAX_CON_COSTにマッピング */
			}
			acumCost = mb[oldBroth].acumCost + conCost;
			if (debugFlag == 3) {
				/* 親までの累積コスト+接続コスト+自分の形態素コスト(デバッグ用) */
				debugCost[oldBroth][i] = mb[oldBroth].acumCost + conCost + iMor->cost;
			}
			/* 通常はUNA_MAX_ACUM_COSTとの比較が必要だがUNA_LOCAL_TEXT_SIZE
			の値、costの最大値より、現状ではUNA_MAX_ACUM_COSTを超えることは
			有得ない */
			if (debugFlag == 1) {
				/* ラティス登録時のコスト情報を出力する */
				DebLatSet((unaMorphHandleT *)mh,oldBroth,st,iMor->ln,
											iMor->hin,iMor->cost);
			}
			if (iMor->acumCost >= acumCost) {
				iMor->parent	= (ucharT)oldBroth;
				iMor->acumCost	= acumCost;
			}
			youngBroth = oldBroth;
			oldBroth = mb[oldBroth].Obrother;
		} while (oldBroth != youngBroth);
		/* パスが決まってから形態素コストを足す */
		iMor->acumCost += iMor->cost;
	}

	if (debugFlag == 3) {
		/* ラティス登録時の詳細情報を出力する */
		fprintf(stdout, "linkWithParent:\n");
		for (i = stPos;i <= endPos;i++) {
			if (i > stPos) {
				fprintf(stdout, "  ---\n");
			}
			oldBroth = (lat->brnchIndex)[st];
			do {
				if (mb[i].hin == LOCAL_HIN_VOID) { /* 自分が無効語のとき */
					/* (any)->[5]あああ(0) =>skip (void current) */
					fprintf(stdout, "  (any)->[%d]%s(%d) =>skip (void current)\n",
						i,
						UnaToUTF8(&((lat->tbuf)[mb[i].st]), mb[i].ln),
						mb[i].hin);
					break;
				} else if (mb[oldBroth].hin == LOCAL_HIN_VOID) { /* 親が無効語のとき */
					/* [1]あああ(0)->[5]いいい(234) =>skip (void parent) */
					fprintf(stdout, "  [%d]%s(%d)->",
						oldBroth,
						UnaToUTF8(&((lat->tbuf)[mb[oldBroth].st]), mb[oldBroth].ln),
						mb[oldBroth].hin);
					fprintf(stdout, "[%d]%s(%d) =>skip (void parent)\n",
						i,
						UnaToUTF8(&((lat->tbuf)[mb[i].st]), mb[i].ln),
						mb[i].hin);
				} else {
					/* [1]あああ(123)->[5]いいい(234) =>parent */
					fprintf(stdout, "  [%d]%s(%d)->",
						oldBroth,
						UnaToUTF8(&((lat->tbuf)[mb[oldBroth].st]), mb[oldBroth].ln),
						mb[oldBroth].hin);
					fprintf(stdout, "[%d]%s(%d) (cost=%d)",
						i,
						UnaToUTF8(&((lat->tbuf)[mb[i].st]), mb[i].ln),
						mb[i].hin,
						debugCost[oldBroth][i]);
					if (mb[i].parent == (ucharT)oldBroth) {
						fprintf(stdout, " =>parent");
					}
					fprintf(stdout, "\n");
				}
				youngBroth = oldBroth;
				oldBroth = mb[oldBroth].Obrother;
			} while (oldBroth != youngBroth);
		}
		fflush(stdout);
	}

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_resetLatEnd
//
// ABSTRACT:    ラティス終了位置調整処理
//
// FUNCTION:
//	  ラティスの終了位置を調整する。
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//	  latticeEnd が、curBrnchPos の形態素の末尾より突き出ていた場合の処理。
//	  例えばオーバーフロー等が発生すると、最後の要素番号(curBrnchPos)の
//	  形態素から手繰って適当につながったパスができるのでlatticeEnd が、
//	  そのcurBrnchPos の形態素の末尾より突き出てしまう事がある。
//	  その時、突き出た部分の文字列は無視されて、次回はlatticeEnd の
//	  次の位置から解析が始まる。結果一部のテキストが無視されたようになる。
//	  従って以下の処理が必要となる。
//	  - 次回の解析の開始位置を正しくする。(latticeEndの書き換え)
//	  但し latticeEnd は brnchIndexのクリアに使用されるので
//	  - brnchIndex のクリア漏れがない様に突き出た部分に対応するbrnchIndex
//		をクリアする
//	  なお、latticeEnd == ed == UNA_LOCAL_TEXT_SIZE + 1 の場合、
//	  memset の第一引数が、配列外を指してしまうので、
//	  バイパスするようにしている
//
int unaMorph_resetLatEnd(
	unaLatticeT *lat,	/* 形態素ラティス */
	int ed				/* 一番最後の形態素の実末尾位置、
						   即ち新たに解析終了位置となる位置 */
)
{
	assert (lat->latticeEnd >= ed);
	assert (ed <= UNA_LOCAL_TEXT_SIZE + 1);	/* +1 は仮想文節頭末の分 */
	if (lat->latticeEnd == ed) {	/* バイパス */
		return UNA_OK;
	}
	
	memset(&lat->brnchIndex[ed + 1],UNAMORPH_LAT_BRNCH_MAX,
											lat->latticeEnd - ed);
	lat->latticeEnd = ed;

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_getHinName
//
// ABSTRACT:	  品詞名の取得
//
// FUNCTION:
//	  形態素品詞番号から形態素品詞名を取得する
//
// RETURN:
//	  NULL文字(0のみのユニコード)	対応する品詞名なし
//	  それ以外			対応する品詞名
//
// NOTE:
//
const unaCharT *unaMorph_getHinName(
	const unaMorphHandleT *mh,	/* ハンドラ */
	ushortT  morHin			/* 形態素品詞番号 */
)
{
	/* 上位関数でチェック済みであるが念のため */
	assert(morHin < mh->morpHinNumMax);

	/* 品詞番号からの変換テーブルより品詞名を得る */
	return ( (mh->hinNamePool) +(mh->hinNamePos)[morHin]);
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_getUnaHin
//
// ABSTRACT:	  UNA品詞番号の取得
//
// FUNCTION:
//	  形態素品詞番号からUNA品詞番号を取得する
//
// RETURN:
//	  UNA_OK			正常終了
//
// NOTE:
//
int unaMorph_getUnaHin(
	const unaMorphHandleT *mh,	/* ハンドラ */
	ushortT  morHin,			/* 形態素品詞番号 */
	unaHinT *unaHin				/* UNA品詞 */
)
{
	/* 上位関数でチェック済みであるが念のため */
	assert(morHin < mh->morpHinNumMax);

	/* UNA品詞番号への変換テーブルよりUNA品詞を得る */
	*unaHin = mh->unaHinNumTable[morHin];

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  LatInit
//
// ABSTRACT:    形態素ラティスバッファの初期化
//
// FUNCTION:
//	  形態素ラティスのバッファの初期化する。unaLatticeT型を作成したとき、
//    LatClear()に先立ってこの関数を呼んでおく必要がある。
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//	  なし。
//
static int LatInit(
	unaLatticeT *lat			/* 形態素ラティス */
	)
{
	/* brnchIndex に「何も無し」の書き込み */
	memset(&lat->brnchIndex,UNAMORPH_LAT_BRNCH_MAX,UNA_LOCAL_TEXT_SIZE + 2);

	/* morChk のクリア(初回のみ全領域クリア) */
	memset(&lat->morChk,0,UNA_HYOKI_LEN_MAX + 1);

	/* 位置を初期化 */
	lat->latticeEnd = 0; /*@ 初回の LatClear() のため必要 */

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:	  LatClear
//
// ABSTRACT:    形態素ラティスバッファのクリア
//
// FUNCTION:
//	  形態素ラティスがクリアされる。
//
// RETURN:
//	  UNA_OK	正常終了
//
// NOTE:
//	  (注1)
//		unaMorph_gen で元々指定されてきた文字列長が UNA_LOCAL_TEXT_SIZE
//		以上か負の場合には UNA_LOCAL_TEXT_SIZE がセットされる
//	  (注2)
//		文字列長がUNA_LOCAL_TEXT_SIZEを超え、かつサロゲートペアがUNA_LOCAL
//		_TEXT_SIZEをまたぐ場合は、UNA_LOCAL_TEXT_SIZE-1がセットされる
//
static int LatClear(
	unaLatticeT *lat,		/* 形態素ラティス */
	int hin,				/* 形態素ラティスの根っこにあたる品詞 */
	unaHinT unaHin,			/* hinに対応するUNA品詞 */
	int inTxtLen			/* 入力テキストの長さ */
)
{
	/* 形態素枝バッファへの文字位置からのインデックスを「何もなし」に */
	assert(lat->latticeEnd >= 0 && lat->latticeEnd <= UNA_LOCAL_TEXT_SIZE+1);
	memset(&lat->brnchIndex,UNAMORPH_LAT_BRNCH_MAX,lat->latticeEnd + 1);

	/* 形態素ラティス用の根っこを登録 */
	(lat->morBrnch)[0].st		= 0;
	(lat->morBrnch)[0].ln		= 0;
	(lat->morBrnch)[0].hin		= (ushortT)hin;
	(lat->morBrnch)[0].unaHin	= unaHin;
	(lat->morBrnch)[0].acumCost	= 0;
	(lat->morBrnch)[0].cost		= 0;
	(lat->morBrnch)[0].parent	= UNAMORPH_LAT_BRNCH_MAX;
	(lat->morBrnch)[0].Obrother	= 0;

	/* テキスト長のセット(注1)(注2) */
	if (inTxtLen < 0) {
		lat->txtLen = UNA_LOCAL_TEXT_SIZE;
	}
	else if (inTxtLen >= UNA_LOCAL_TEXT_SIZE) {
#ifdef PARTIAL_SURROGATE_PAIR_SUPPORT
		if (lat->tbuf[UNA_LOCAL_TEXT_SIZE - 1] >= 0xd800 &&
			lat->tbuf[UNA_LOCAL_TEXT_SIZE - 1] <= 0xdbff) {
			if (inTxtLen >= UNA_LOCAL_TEXT_SIZE + 1 &&
				lat->tbuf[UNA_LOCAL_TEXT_SIZE] >= 0xdc00 &&
				lat->tbuf[UNA_LOCAL_TEXT_SIZE] <= 0xdfff) {
				lat->txtLen = UNA_LOCAL_TEXT_SIZE - 1;
			}
			else {
				lat->txtLen = UNA_LOCAL_TEXT_SIZE;
			}
		}
		else {
			lat->txtLen = UNA_LOCAL_TEXT_SIZE;
		}
#else
		lat->txtLen = UNA_LOCAL_TEXT_SIZE;
#endif
	}
	else{
		lat->txtLen = inTxtLen;
	}
	/* その他周辺データの書き換え */
	lat->latticeEnd = 0;
	lat->curBrnchPos = 0;
	(lat->brnchIndex)[0] = 0;

	/* 正常終了 */
	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:   AddPath
//
// ABSTRACT:	  最適パスの付加
//
// FUNCTION:
//	  形態素ラティス中の最適パスを形態素列の後ろに付加する。
//
// RETURN:
//	  正の値	実際に格納した形態素数
//	  負の値	エラー番号
//
// NOTE:
//	  特になし。
//
static int AddPath(
	unaMorphT *wbuf,	/* 形態素列を格納するバッファ */
	int wbufLen,		/* 形態素列を格納するバッファのサイズ */
	unaLatticeT *lat	/* 形態素ラティス */
)
{
	int i;				/* ラティスの枝の位置 */
	int addMorLen;		/* 形態素パスの長さ */
	int morphPos;		/* 形態素の格納位置 */

	if (debugFlag == 2 || debugFlag == 3) {
		/* ラティス情報を出力する */
		PrintLat(lat);
	}

	/*
	 * 以下は格納の準備
	 */
	/* 形態素パスの長さを数える */
	i = lat->curBrnchPos;		/* 現在格納可能な形態素枝位置 */
	addMorLen = 0;
	do {
		addMorLen++;
		i = (lat->morBrnch)[i].parent;
	} while (i != 0);
	/* 形態素列バッファのオーバーフローチェック */
	if (addMorLen >= wbufLen) {
		UNA_RETURN(ERR_PATH,NULL);
	}
	/*
	 * ラティスの内容を形態素列データに格納する
	 */
	/* 末尾から格納していく */
	morphPos = addMorLen - 1; /* 格納位置の計算 */
	for (i = lat->curBrnchPos; i != 0; i = (lat->morBrnch)[i].parent) {
		/* 格納 */
		wbuf[morphPos].hinshi = (ushortT)(lat->morBrnch)[i].hin;
		wbuf[morphPos].start = lat->tbuf + (lat->morBrnch)[i].st;
		wbuf[morphPos].length = (lat->morBrnch)[i].ln;
		wbuf[morphPos].appI = (lat->morBrnch)[i].appI;
		wbuf[morphPos].subI = (lat->morBrnch)[i].subI;
		wbuf[morphPos].cost = (lat->morBrnch)[i].cost;
		/* 格納位置の更新 */
		morphPos--;
	}

	/* 格納した数を返す */
	return addMorLen;
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_termSentece
//
// ABSTRACT:	文末を指示する
//
// FUNCTION:
//	  形態素解析モジュールが持つ次回解析用の先頭品詞を文末品詞に設定する
//
// RETURN:
//	  なし
//
// NOTE:
//		この関数を実行しない場合、常に形態素解析モジュールは、前回の解析結果に
//		続ける形で解析を実行する。
//
void unaMorph_termSentence(
	unaMorphHandleT *mh	/* ハンドラ */
)
{
	(mh->maeHin) = (mh->hinBunEnd);
}

//--------------------------------------------------------------------------
// MODULE:	  unaMorph_getSenteceTail
//
// ABSTRACT:	現在の文末品詞を取得する
//
// FUNCTION:
//	  形態素解析モジュールが持つ文末品詞(次回解析用の先頭品詞)を取得する
//
// RETURN:
//	  品詞番号
//
// NOTE:
//
int unaMorph_getSentenceTail(
	unaMorphHandleT *mh	/* ハンドラ */
)
{
	return (mh->maeHin);
}

//--------------------------------------------------------------------------
// MODULE:   unaMorph_setSenteceTail
//
// ABSTRACT:	文末品詞を設定する
//
// FUNCTION:
//	  形態素解析モジュールが持つ文末品詞を指定の値に設定する
//
// RETURN:
//	  設定された品詞番号
//
// NOTE:
//
int unaMorph_setSentenceTail(
	unaMorphHandleT *mh,	/* ハンドラ */
	int	hinNum	/* 品詞の指定 */
)
{
	(mh->maeHin) = hinNum;

	return (mh->maeHin);
}

//--------------------------------------------------------------------------
// MODULE:   DebLatSet
//
// ABSTRACT:   unaMorph_latSet デバッグ用関数
//
// FUNCTION:
//	  unaMorph_latSet のデバッグ情報を表示する
//
// RETURN:
//	  UNA_OK
//
// NOTE:
//	  デバッグ用である
//
static int DebLatSet(
	unaMorphHandleT *mh,	/* ハンドラ */
	int oldBroth,			/* 前接形態素へのポインタ(morBrnchの要素番号) */
	int st,					/* 文字列中のはじまり位置(オフセット) */
	int ln,					/* 長さ */
	int hin,				/* 品詞番号 */
	int cost				/* コスト */
	)
{
	unaLatticeT *lat;		/* 形態素ラティス */

	lat = &(mh->lat);

	fprintf(stdout,
		"maeHin=%s(hinNo=%d cost=%d) ",
		UnaToUTF8(
			&((lat->tbuf)[(lat->morBrnch)[oldBroth].st]),
			(lat->morBrnch)[oldBroth].ln
		),
		(lat->morBrnch)[oldBroth].hin,
		(lat->morBrnch)[oldBroth].cost
	);
	fprintf(stdout,
		"atoHin=%s(hinNo%d cost=%d) ",
		UnaToUTF8(
			&((lat->tbuf)[st]),
			ln
		),
		hin,
		cost
	);
	fprintf(stdout,
		"conCost=%d\n",
		mh->connectCostTable[(mh->kakari)[(lat->morBrnch)[oldBroth].hin]
		 * mh->ukeNumMax + (mh->uke)[hin]]
	);

	return UNA_OK;
}

//--------------------------------------------------------------------------
// MODULE:   PrintLat
//
// ABSTRACT:   lattice 表示用関数
//
// FUNCTION:
//	  lattice の中身を表示する
//
// RETURN:
//	  UNA_OK
//
// NOTE:
//	  デバッグ用である
//	  解析ツールanadbgで使用しているため、修正注意!!
//
static int PrintLat(
	unaLatticeT *lat	/* 形態素ラティス */
)
{
	int i;				/* ループ変数 */

	if (debugFlag == 3) {
		fprintf(stdout, "PrintLat:\n");
		for (i = 0; i <= lat->curBrnchPos; i++){
			fprintf(stdout,
				"  [%d]%s:hin=%d unaHin=%04x cost=%d acumCost=%u parent=[%d] Obrother=[%d] "
				"appI=%X subI=%X prio=%d\n",
				i,
				UnaToUTF8(
					&((lat->tbuf)[(lat->morBrnch)[i].st]),
					(lat->morBrnch)[i].ln),
				(lat->morBrnch)[i].hin,(lat->morBrnch)[i].unaHin,(lat->morBrnch)[i].cost,
				(lat->morBrnch)[i].acumCost,(lat->morBrnch)[i].parent,
				(lat->morBrnch)[i].Obrother,(lat->morBrnch)[i].appI,
				(lat->morBrnch)[i].subI,(int)((lat->morBrnch)[i].dicPrio));
		}
	}

	if (debugFlag == 2) {
		for (i = 0; i <= lat->curBrnchPos; i++){
			fprintf(stdout,
				"[%d]%s:hin=%d unaHin=%04x cost=%d acumCost=%u parent=[%d] Obrother=[%d] "
				"appI=%X subI=%X prio=%d\n",
				i,
				UnaToUTF8(
					&((lat->tbuf)[(lat->morBrnch)[i].st]),
					(lat->morBrnch)[i].ln),
				(lat->morBrnch)[i].hin,(lat->morBrnch)[i].unaHin,(lat->morBrnch)[i].cost,
				(lat->morBrnch)[i].acumCost,(lat->morBrnch)[i].parent,
				(lat->morBrnch)[i].Obrother,(lat->morBrnch)[i].appI,
				(lat->morBrnch)[i].subI,(int)((lat->morBrnch)[i].dicPrio));
		}

		/* "%d文字目迄解析済 morBrnch[%d]迄使用済\n" */
		fprintf(stdout,"latticeEnd = %d, curBrnchPos = %d\n",
			lat->latticeEnd,lat->curBrnchPos);

		 /* "その位置を終わりとする形態素へのポインタ\n" */
		fprintf(stdout,"List of brnchIndex[]:\n");
		for (i = 0; i <= lat->curBrnchPos; i++) {
			fprintf(stdout,"%d=[%d] ",i,lat->brnchIndex[i]); /* 繋げて表示 */
		}

		/* "\n未登録語のチェック用配列\n" */
		fprintf(stdout,"\nList of morChk[]\n");
		for (i = 0; i <= UNA_UNK_HYOKI_LIMIT; i++) {
			fprintf(stdout,"%d=[%d] ",i,lat->morChk[i]); /* 繋げて表示 */
		}
		fprintf(stdout,"\n"); /* 最後に改行 */
	}

	/* 正常終了 */
	return UNA_OK;
}

ucharT _MarkUTF8[4] = {
	0x00, 0x00, 0xc0, 0xe0
};

static ucharT *UnaToUTF8(
	unaCharT *uStr,
	uintT uStrLen
	)
{
	static ucharT buf[1024];
	ucharT *p = buf;
	int i = 0;
	int s = 0;
	int low_surrogate = 0;
	unaCharT c, c2;

	while (i < uStrLen)
	{
		c = uStr[i++];
		low_surrogate = 0;
		if (c < 0x80)		 s = 1;
		else if (c < 0x800)  s = 2;
		else if (c < 0xd800
			  || c > 0xdfff) s = 3;
		else if (c < 0xdc00) s = 4;	// 0xd800-0xdbff：サロゲートペア前半
		else {
			// 0xdc00-0xdfff：サロゲートペア後半(出現しないはず)
			low_surrogate = 1;
			s = 3;
		}
		if (low_surrogate) {
			// サロゲートペア後半が単体で出現した
			// REPLACEMENT CHARACTER(U+FFFD, 0xef 0xbf 0xbd)に変換する
			p[0] = 0xef;
			p[1] = 0xbf;
			p[2] = 0xbd;
		} else if (s == 4) {
			// サロゲートペア前半が出現した
			// サロゲートペア前半後半を合わせて4バイトのUTF8表現に変換する
			// UCS2 [11011xxx xxyyyyyy] [110111yy zzzzzzzz]
			//    + [00000000 01000000]
			// 					↓
			//      [11011XXX XXyyyyyy] [110111yy zzzzzzzz]
			// 					↓
			// UTF8 [11110XXX] [10XXyyyy] [10yyyyzz] [10zzzzzz]
			c2 = uStr[i++];
			if (c2 < 0xdc00 || c2 >= 0xe000) {
				// 後続文字がサロゲートペア後半でない
				// サロゲートペア前半を単体でREPLACEMENT CHARACTERに変換する
				// 後続文字は次のループで処理する
				i--;
				p[0] = 0xef;
				p[1] = 0xbf;
				p[2] = 0xbd;
				s = 3;
			} else {
				p[0] = ((c + 0x0040 & 0x0300) >> 8) | 0xf0;
				p[1] = ((c + 0x0040 & 0x00fc) >> 2) | 0x80;
				p[2] = ((c & 0x0003) << 4) | ((c2 & 0x03c0) >> 6) | 0x80;
				p[3] = (c2 & 0x003f) | 0x80;
			}
		} else {
			// サロゲートペア以外のUCS2文字を1～3バイトのUTF8表現に変換する
			p += s;
			switch (s)
			{
			case 3: *--p = (c & 0x3f) | 0x80; c >>= 6;
			case 2: *--p = (c & 0x3f) | 0x80; c >>= 6;
			case 1: *--p = (c | _MarkUTF8[s]);
			}
		}
		p += s;
		if (c == 0) break;
	}
	*p = 0;
	return buf;
}

//--------------------------------------------------------------------------
// Copyright (c) 1998-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//--------------------------------------------------------------------------

