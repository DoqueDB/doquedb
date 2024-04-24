# Copyright (c) 2024 Ricoh Company, Ltd.
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
constants.py -- IDやタイプ値など定数定義のモジュール

:dict: `class_ids`
:dict: `datatypes`
:dict: `sqltypes`
:class: `Language`
:class: `Country`
"""
# TODO: Language, Countryの実装

from enum import IntEnum, unique


@unique
class ClassID(IntEnum):
    """DoqueDBのCommonのクラスID."""
    # DoqueDBのCommon::ClassIDと同じ数字
    NONE = 0

    STATUS = 1
    INTEGER_DATA = 2
    UNSIGNED_INTEGER_DATA = 3
    INTEGER64_DATA = 4
    UNSIGNED_INTEGER64_DATA = 5
    FLOAT_DATA = 6
    DOUBLE_DATA = 7
    DECIMAL_DATA = 8
    STRING_DATA = 9
    DATE_DATA = 10
    DATE_TIME_DATA = 11
    INTEGER_ARRAY_DATA = 12
    UNSIGNED_INTEGER_ARRAY_DATA = 13
    STRING_ARRAY_DATA = 14
    DATA_ARRAY_DATA = 15
    BINARY_DATA = 16
    NULL_DATA = 17
    EXCEPTION_DATA = 18
    # PARAMETER  = 19
    # BITSET  = 20
    COMPRESSED_STRING_DATA = 21
    COMPRESSED_BINARY_DATA = 22
    # OBJECTID_DATA  = 23
    REQUEST = 24
    LANGUAGE_DATA = 25
    # SQL_DATA  = 26
    COLUMN_META_DATA = 27
    RESULTSET_META_DATA = 28
    WORD_DATA = 29
    ERROR_LEVEL = 30


@unique
class DataType(IntEnum):
    """データ型のタイプ."""
    # DoqueDBのCommon::DataTypeと同じ数字である
    # Javaにはunsignedが存在しないので、その部分はコメントアウトしてある

    DATA = 1000
    INTEGER = 1001
    # UNSIGNED_INTEGER = 1002
    INTEGER64 = 1003
    # UNSIGNED_INTEGER64 = 1004
    STRING = 1005
    FLOAT = 1006
    DOUBLE = 1007
    DECIMAL = 1008
    DATE = 1009
    DATE_TIME = 1010
    BINARY = 1011
    BITSET = 1012
    OBJECTID = 1013
    LANGUAGE = 1014
    COLUMN_META_DATA = 1015
    WORD = 1016

    ARRAY = 2000

    NULL = 3000

    UNDEFINED = 9999


@unique
class SQLType(IntEnum):
    """SQLデータ型のタイプ."""
    # 不明値
    UNKNOWN = 0

    # 文字列型
    CHARACTER = 1
    CHARACTER_VARYING = 2
    NATIONAL_CHARACTER = 3
    NATIONAL_CHARACTER_VARYING = 4

    # バイナリ型
    BINARY = 5
    BINARY_VARYING = 6

    # ラージオブジェクト型
    CHARACTER_LARGE_OBJECT = 7
    NATIONAL_CHARACTER_LARGE_OBJECT = 8
    BINARY_LARGE_OBJECT = 9

    # 整数型
    NUMERIC = 10
    SMALL_INT = 11
    INTEGER = 12
    BIG_INT = 13

    # 小数点型
    DECIMAL = 14
    FLOAT = 15
    REAL = 16
    DOUBLE_PRECISION = 17

    # ブール型
    BOOLEAN = 18

    # 日時型
    DATE = 19
    TIME = 20
    TIMESTAMP = 21

    # 言語型(DoqueDB独自)
    LANGUAGE = 22
    # ワード(DoqueDB独自)
    WORD = 23


class Language:
    """言語種別関連
    """
    # 未知の言語種別コード (=0)
    UNDEFINED = 0

    # Afar を示す言語種別コード (=1)
    AA = 1
    # Abkhazian を示す言語種別コード (=2)
    AB = 2
    # Afrikaans を示す言語種別コード (=3)
    AF = 3
    # Amharic を示す言語種別コード (=4)
    AM = 4
    # Arabic を示す言語種別コード (=5)
    AR = 5
    # Assamese を示す言語種別コード (=6)
    AS = 6
    # Aymara を示す言語種別コード (=7)
    AY = 7
    # Azerbaijani を示す言語種別コード (=8)
    AZ = 8
    # Bashkir を示す言語種別コード (=9)
    BA = 9
    # Byelorussian を示す言語種別コード (=10)
    BE = 10
    # Bulgarian を示す言語種別コード (=11)
    BG = 11
    # Bihari を示す言語種別コード (=12)
    BH = 12
    # Bislama を示す言語種別コード (=13)
    BI = 13
    # Bengali Bangla を示す言語種別コード (=14)
    BN = 14
    # Tibetan を示す言語種別コード (=15)
    BO = 15
    # Breton を示す言語種別コード (=16)
    BR = 16
    # Catalan を示す言語種別コード (=17)
    CA = 17
    # Corsican を示す言語種別コード (=18)
    CO = 18
    # Czech を示す言語種別コード (=19)
    CS = 19
    # Welsh を示す言語種別コード (=20)
    CY = 20
    # Danish を示す言語種別コード (=21)
    DA = 21
    # German を示す言語種別コード (=22)
    DE = 22
    # Bhutani を示す言語種別コード (=23)
    DZ = 23
    # Greek を示す言語種別コード (=24)
    EL = 24
    # English を示す言語種別コード (=25)
    EN = 25
    # Esperanto を示す言語種別コード (=26)
    EO = 26
    # Spanish を示す言語種別コード (=27)
    ES = 27
    # Estonian を示す言語種別コード (=28)
    ET = 28
    # Basque を示す言語種別コード (=29)
    EU = 29
    # Persian を示す言語種別コード (=30)
    FA = 30
    # Finnish を示す言語種別コード (=31)
    FI = 31
    # Fiji を示す言語種別コード (=32)
    FJ = 32
    # Faroese を示す言語種別コード (=33)
    FO = 33
    # French を示す言語種別コード (=34)
    FR = 34
    # Frisian を示す言語種別コード (=35)
    FY = 35
    # Irish を示す言語種別コード (=36)
    GA = 36
    # Scots Gaelic を示す言語種別コード (=37)
    GD = 37
    # Galician を示す言語種別コード (=38)
    GL = 38
    # Guarani を示す言語種別コード (=39)
    GN = 39
    # Gujarati を示す言語種別コード (=40)
    GU = 40
    # Hausa を示す言語種別コード (=41)
    HA = 41
    # Hebrew (formerly iw) を示す言語種別コード (=42)
    HE = 42
    # Hindi を示す言語種別コード (=43)
    HI = 43
    # Croatian を示す言語種別コード (=44)
    HR = 44
    # Hungarian を示す言語種別コード (=45)
    HU = 45
    # Armenian を示す言語種別コード (=46)
    HY = 46
    # Interlingua を示す言語種別コード (=47)
    IA = 47
    # Indonesian (formerly in) を示す言語種別コード (=48)
    ID = 48
    # Interlingue を示す言語種別コード (=49)
    IE = 49
    # Inupiak を示す言語種別コード (=50)
    IK = 50
    # Icelandic を示す言語種別コード (=51)
    IS = 51
    # Italian を示す言語種別コード (=52)
    IT = 52
    # Inuktitut を示す言語種別コード (=53)
    IU = 53
    # Japanese を示す言語種別コード (=54)
    JA = 54
    # Javanese を示す言語種別コード (=55)
    JW = 55
    # Georgian を示す言語種別コード (=56)
    KA = 56
    # Kazakh を示す言語種別コード (=57)
    KK = 57
    # Greenlandic を示す言語種別コード (=58)
    KL = 58
    # Cambodian を示す言語種別コード (=59)
    KM = 59
    # Kannada を示す言語種別コード (=60)
    KN = 60
    # Korean を示す言語種別コード (=61)
    KO = 61
    # Kashmiri を示す言語種別コード (=62)
    KS = 62
    # Kurdish を示す言語種別コード (=63)
    KU = 63
    # Kirghiz を示す言語種別コード (=64)
    KY = 64
    # Latin を示す言語種別コード (=65)
    LA = 65
    # Lingala を示す言語種別コード (=66)
    LN = 66
    # Laothian を示す言語種別コード (=67)
    LO = 67
    # Lithuanian を示す言語種別コード (=68)
    LT = 68
    # Latvian, Lettish を示す言語種別コード (=69)
    LV = 69
    # Malagasy を示す言語種別コード (=70)
    MG = 70
    # Maori を示す言語種別コード (=71)
    MI = 71
    # Macedonian を示す言語種別コード (=72)
    MK = 72
    # Malayalam を示す言語種別コード (=73)
    ML = 73
    # Mongolian を示す言語種別コード (=74)
    MN = 74
    # Moldavian を示す言語種別コード (=75)
    MO = 75
    # Marathi を示す言語種別コード (=76)
    MR = 76
    # Malay を示す言語種別コード (=77)
    MS = 77
    # Maltese を示す言語種別コード (=78)
    MT = 78
    # Burmese を示す言語種別コード (=79)
    MY = 79
    # Nauru を示す言語種別コード (=80)
    NA = 80
    # Nepali を示す言語種別コード (=81)
    NE = 81
    # Dutch を示す言語種別コード (=82)
    NL = 82
    # Norwegian を示す言語種別コード (=83)
    NO = 83
    # Occitan を示す言語種別コード (=84)
    OC = 84
    # (Afan) Oromo を示す言語種別コード (=85)
    OM = 85
    # Oriya を示す言語種別コード (=86)
    OR = 86
    # Punjabi を示す言語種別コード (=87)
    PA = 87
    # Polish を示す言語種別コード (=88)
    PL = 88
    # Pashto, Pushto を示す言語種別コード (=89)
    PS = 89
    # Portuguese を示す言語種別コード (=90)
    PT = 90
    # Quechua を示す言語種別コード (=91)
    QU = 91
    # Rhaeto-Romance を示す言語種別コード (=92)
    RM = 92
    # Kirundi を示す言語種別コード (=93)
    RN = 93
    # Romanian を示す言語種別コード (=94)
    RO = 94
    # Russian を示す言語種別コード (=95)
    RU = 95
    # Kinyarwanda を示す言語種別コード (=96)
    RW = 96
    # Sanskrit を示す言語種別コード (=97)
    SA = 97
    # Sindhi を示す言語種別コード (=98)
    SD = 98
    # Sangho を示す言語種別コード (=99)
    SG = 99
    # Serbo-Croatian を示す言語種別コード (=100)
    SH = 100
    # Sinhalese を示す言語種別コード (=101)
    SI = 101
    # Slovak を示す言語種別コード (=102)
    SK = 102
    # Slovenian を示す言語種別コード (=103)
    SL = 103
    # Samoan を示す言語種別コード (=104)
    SM = 104
    # Shona を示す言語種別コード (=105)
    SN = 105
    # Somali を示す言語種別コード (=106)
    SO = 106
    # Albanian を示す言語種別コード (=107)
    SQ = 107
    # Serbian を示す言語種別コード (=108)
    SR = 108
    # Siswati を示す言語種別コード (=109)
    SS = 109
    # Sesotho を示す言語種別コード (=110)
    ST = 110
    # Sundanese を示す言語種別コード (=111)
    SU = 111
    # Swedish を示す言語種別コード (=112)
    SV = 112
    # Swahili を示す言語種別コード (=113)
    SW = 113
    # Tamil を示す言語種別コード (=114)
    TA = 114
    # Telugu を示す言語種別コード (=115)
    TE = 115
    # Tajik を示す言語種別コード (=116)
    TG = 116
    # Thai を示す言語種別コード (=117)
    TH = 117
    # Tigrinya を示す言語種別コード (=118)
    TI = 118
    # Turkmen を示す言語種別コード (=119)
    TK = 119
    # Tagalog を示す言語種別コード (=120)
    TL = 120
    # Setswana を示す言語種別コード (=121)
    TN = 121
    # Tonga を示す言語種別コード (=122)
    TO = 122
    # Turkish を示す言語種別コード (=123)
    TR = 123
    # Tsonga を示す言語種別コード (=124)
    TS = 124
    # Tatar を示す言語種別コード (=125)
    TT = 125
    # Twi を示す言語種別コード (=126)
    TW = 126
    # Uighur を示す言語種別コード (=127)
    UG = 127
    # Ukrainian を示す言語種別コード (=128)
    UK = 128
    # Urdu を示す言語種別コード (=129)
    UR = 129
    # Uzbek を示す言語種別コード (=130)
    UZ = 130
    # Vietnamese を示す言語種別コード (=131)
    VI = 131
    # Volapuk を示す言語種別コード (=132)
    VO = 132
    # Wolof を示す言語種別コード (=133)
    WO = 133
    # Xhosa を示す言語種別コード (=134)
    XH = 134
    # Yiddish (formerly ji) を示す言語種別コード (=135)
    YI = 135
    # Yoruba を示す言語種別コード (=136)
    YO = 136
    # Zhuang を示す言語種別コード (=137)
    ZA = 137
    # Chinese を示す言語種別コード (=138)
    ZH = 138
    # Zulu を示す言語種別コード (=139)
    ZU = 139

    # 言語種別コードの範囲外開始を示す値 (=140)
    LAST = 140

    code_symbol = (
        "undefined",  # undefined
        "aa",        # 1.Afar
        "ab",        # 2.Abkhazian
        "af",        # 3.Afrikaans
        "am",        # 4.Amharic
        "ar",        # 5.Arabic
        "as",        # 6.Assamese
        "ay",        # 7.Aymara
        "az",        # 8.Azerbaijani

        "ba",        # 9.Bashkir
        "be",        # 10.Byelorussian
        "bg",        # 11.Bulgarian
        "bh",        # 12.Bihari
        "bi",        # 13.Bislama
        "bn",        # 14.Bengali; Bangla
        "bo",        # 15.Tibetan
        "br",        # 16.Breton

        "ca",        # 17.Catalan
        "co",        # 18.Corsican
        "cs",        # 19.Czech
        "cy",        # 20.Welsh

        "da",        # 21.Danish
        "de",        # 22.German
        "dz",        # 23.Bhutani

        "el",        # 24.Greek
        "en",        # 25.English
        "eo",        # 26.Esperanto
        "es",        # 27.Spanish
        "et",        # 28.Estonian
        "eu",        # 29.Basque

        "fa",        # 30.Persian
        "fi",        # 31.Finnish
        "fj",        # 32.Fiji
        "fo",        # 33.Faroese
        "fr",        # 34.French
        "fy",        # 35.Frisian

        "ga",        # 36.Irish
        "gd",        # 37.Scots Gaelic
        "gl",        # 38.Galician
        "gn",        # 39.Guarani
        "gu",        # 40.Gujarati

        "ha",        # 41.Hausa
        "he",        # 42.Hebrew (formerly iw)
        "hi",        # 43.Hindi
        "hr",        # 44.Croatian
        "hu",        # 45.Hungarian
        "hy",        # 46.Armenian

        "ia",        # 47.Interlingua
        "id",        # 48.Indonesian (formerly in)
        "ie",        # 49.Interlingue
        "ik",        # 50.Inupiak
        "is",        # 51.Icelandic
        "it",        # 52.Italian
        "iu",        # 53.Inuktitut

        "ja",        # 54.Japanese
        "jw",        # 55.Javanese

        "ka",        # 56.Georgian
        "kk",        # 57.Kazakh
        "kl",        # 58.Greenlandic
        "km",        # 59.Cambodian
        "kn",        # 60.Kannada
        "ko",        # 61.Korean
        "ks",        # 62.Kashmiri
        "ku",        # 63.Kurdish
        "ky",        # 64.Kirghiz

        "la",        # 65.Latin
        "ln",        # 66.Lingala
        "lo",        # 67.Laothian
        "lt",        # 68.Lithuanian
        "lv",        # 69.Latvian, Lettish

        "mg",        # 70.Malagasy
        "mi",        # 71.Maori
        "mk",        # 72.Macedonian
        "ml",        # 73.Malayalam
        "mn",        # 74.Mongolian
        "mo",        # 75.Moldavian
        "mr",        # 76.Marathi
        "ms",        # 77.Malay
        "mt",        # 78.Maltese
        "my",        # 79.Burmese

        "na",        # 80.Nauru
        "ne",        # 81.Nepali
        "nl",        # 82.Dutch
        "no",        # 83.Norwegian

        "oc",        # 84.Occitan
        "om",        # 85.(Afan) Oromo
        "or",        # 86.Oriya

        "pa",        # 87.Punjabi
        "pl",        # 88.Polish
        "ps",        # 89.Pashto, Pushto
        "pt",        # 90.Portuguese

        "qu",        # 91.Quechua

        "rm",        # 92.Rhaeto-Romance
        "rn",        # 93.Kirundi
        "ro",        # 94.Romanian
        "ru",        # 95.Russian
        "rw",        # 96.Kinyarwanda

        "sa",        # 97.Sanskrit
        "sd",        # 98.Sindhi
        "sg",        # 99.Sangho
        "sh",        # 100.Serbo-Croatian
        "si",        # 101.Sinhalese
        "sk",        # 102.Slovak
        "sl",        # 103.Slovenian
        "sm",        # 104.Samoan
        "sn",        # 105.Shona
        "so",        # 106.Somali
        "sq",        # 107.Albanian
        "sr",        # 108.Serbian
        "ss",        # 109.Siswati
        "st",        # 110.Sesotho
        "su",        # 111.Sundanese
        "sv",        # 112.Swedish
        "sw",        # 113.Swahili

        "ta",        # 114.Tamil
        "te",        # 115.Telugu
        "tg",        # 116.Tajik
        "th",        # 117.Thai
        "ti",        # 118.Tigrinya
        "tk",        # 119.Turkmen
        "tl",        # 120.Tagalog
        "tn",        # 121.Setswana
        "to",        # 122.Tonga
        "tr",        # 123.Turkish
        "ts",        # 124.Tsonga
        "tt",        # 125.Tatar
        "tw",        # 126.Twi

        "ug",        # 127.Uighur
        "uk",        # 128.Ukrainian
        "ur",        # 129.Urdu
        "uz",        # 130.Uzbek

        "vi",        # 131.Vietnamese
        "vo",        # 132.Volapuk

        "wo",        # 133.Wolof

        "xh",        # 134.Xhosa

        "yi",        # 135.Yiddish (formerly ji)
        "yo",        # 136.Yoruba

        "za",        # 137.Zhuang
        "zh",        # 138.Chinese
        "zu"        # 139.Zulu
    )

    @staticmethod
    def to_symbol(code_: int) -> int:
        """言語種別コードから対応するシンボルを返す

        Args:
            code_ (int): 言語種別コード

        Returns:
            int: 言語種別コードを示す文字列（シンボル）
        """
        if not Language.is_valid(code_):
            code_ = Language.UNDEFINED
        return Language.code_symbol[code_]

    @staticmethod
    def is_valid(code_: int) -> bool:
        """言語種別コードが範囲内であるかどうかを確認する.

        Args:
            code_ (int): 言語種別コード

        Returns:
            bool: 言語種別コードが範囲内の場合True. 範囲外の場合False
        """
        if code_ <= Language.UNDEFINED or code_ >= Language.LAST:
            return False
        return True

    @staticmethod
    def to_code(symbol: str) -> int:
        """シンボルから対応する言語種別コードを返す.

        Args:
            symbol (str): 言語種別コードを示す文字列（シンボル）

        Returns:
            int: 言語種別コード
        """
        if len(symbol) != 2:
            return Language.UNDEFINED

        code = Language.UNDEFINED
        try:
            code = Language.code_symbol.index(symbol)
        except ValueError:
            # 対応するindexがない場合はUNDEFINEDのまま
            pass

        return code


class Country:
    """国・地域関連. ModCountry::iso3166 の各国・地域コードと同値のフィールドをもつ.
    """
    # 未知の国・地域コード (=0)
    UNDEFINED = 0

    # AFGHANISTAN を示す国・地域コード (=1)
    AF = 1
    # ALBANIA を示す国・地域コード (=2)
    AL = 2
    # ALGERIA を示す国・地域コード (=3)
    DZ = 3
    # AMERICAN SAMOA を示す国・地域コード (=4)
    AS = 4
    # ANDORRA を示す国・地域コード (=5)
    AD = 5
    # ANGOLA を示す国・地域コード (=6)
    AO = 6
    # ANGUILLA を示す国・地域コード (=7)
    AI = 7
    # ANTARCTICA を示す国・地域コード (=8)
    AQ = 8
    # ANTIGUA AND BARBUDA を示す国・地域コード (=9)
    AG = 9
    # ARGENTINA を示す国・地域コード (=10)
    AR = 10
    # ARMENIA を示す国・地域コード (=11)
    AM = 11
    # ARUBA を示す国・地域コード (=12)
    AW = 12
    # AUSTRALIA を示す国・地域コード (=13)
    AU = 13
    # AUSTRIA を示す国・地域コード (=14)
    AT = 14
    # AZERBAIJAN を示す国・地域コード (=15)
    AZ = 15
    # BAHAMAS を示す国・地域コード (=16)
    BS = 16
    # BAHRAIN を示す国・地域コード (=17)
    BH = 17
    # BANGLADESH を示す国・地域コード (=18)
    BD = 18
    # BARBADOS を示す国・地域コード (=19)
    BB = 19
    # BELARUS を示す国・地域コード (=20)
    BY = 20
    # BELGIUM を示す国・地域コード (=21)
    BE = 21
    # BELIZE を示す国・地域コード (=22)
    BZ = 22
    # BENIN を示す国・地域コード (=23)
    BJ = 23
    # BERMUDA を示す国・地域コード (=24)
    BM = 24
    # BHUTAN を示す国・地域コード (=25)
    BT = 25
    # BOLIVIA を示す国・地域コード (=26)
    BO = 26
    # BOSNIA AND HERZEGOWINA を示す国・地域コード (=27)
    BA = 27
    # BOTSWANA を示す国・地域コード (=28)
    BW = 28
    # BOUVET ISLAND を示す国・地域コード (=29)
    BV = 29
    # BRAZIL を示す国・地域コード (=30)
    BR = 30
    # BRITISH INDIAN OCEAN TERRITORY を示す国・地域コード (=31)
    IO = 31
    # BRUNEI DARUSSALAM を示す国・地域コード (=32)
    BN = 32
    # BULGARIA を示す国・地域コード (=33)
    BG = 33
    # BURKINA FASO を示す国・地域コード (=34)
    BF = 34
    # BURUNDI を示す国・地域コード (=35)
    BI = 35
    # CAMBODIA を示す国・地域コード (=36)
    KH = 36
    # CAMEROON を示す国・地域コード (=37)
    CM = 37
    # CANADA を示す国・地域コード (=38)
    CA = 38
    # CAPE VERDE を示す国・地域コード (=39)
    CV = 39
    # CAYMAN ISLANDS を示す国・地域コード (=40)
    KY = 40
    # CENTRAL AFRICAN REPUBLIC を示す国・地域コード (=41)
    CF = 41
    # CHAD を示す国・地域コード (=42)
    TD = 42
    # CHILE を示す国・地域コード (=43)
    CL = 43
    # CHINA を示す国・地域コード (=44)
    CN = 44
    # CHRISTMAS ISLAND を示す国・地域コード (=45)
    CX = 45
    # COCOS (KEELING) ISLANDS を示す国・地域コード (=46)
    CC = 46
    # COLOMBIA を示す国・地域コード (=47)
    CO = 47
    # COMOROS を示す国・地域コード (=48)
    KM = 48
    # CONGO, Democratic Republic of (was Zaire) を示す国・地域コード (=49)
    CD = 49
    # CONGO, People's Republic of を示す国・地域コード (=50)
    CG = 50
    # COOK ISLANDS を示す国・地域コード (=51)
    CK = 51
    # COSTA RICA を示す国・地域コード (=52)
    CR = 52
    # COTE D'IVOIRE を示す国・地域コード (=53)
    CI = 53
    # CROATIA (local name: Hrvatska) を示す国・地域コード (=54)
    HR = 54
    # CUBA を示す国・地域コード (=55)
    CU = 55
    # CYPRUS を示す国・地域コード (=56)
    CY = 56
    # CZECH REPUBLIC を示す国・地域コード (=57)
    CZ = 57
    # DENMARK を示す国・地域コード (=58)
    DK = 58
    # DJIBOUTI を示す国・地域コード (=59)
    DJ = 59
    # DOMINICA を示す国・地域コード (=60)
    DM = 60
    # DOMINICAN REPUBLIC を示す国・地域コード (=61)
    DO = 61
    # EAST TIMOR を示す国・地域コード (=62)
    TL = 62
    # ECUADOR を示す国・地域コード (=63)
    EC = 63
    # EGYPT を示す国・地域コード (=64)
    EG = 64
    # EL SALVADOR を示す国・地域コード (=65)
    SV = 65
    # EQUATORIAL GUINEA を示す国・地域コード (=66)
    GQ = 66
    # ERITREA を示す国・地域コード (=67)
    ER = 67
    # ESTONIA を示す国・地域コード (=68)
    EE = 68
    # ETHIOPIA を示す国・地域コード (=69)
    ET = 69
    # FALKLAND ISLANDS (MALVINAS) を示す国・地域コード (=70)
    FK = 70
    # FAROE ISLANDS を示す国・地域コード (=71)
    FO = 71
    # FIJI を示す国・地域コード (=72)
    FJ = 72
    # FINLAND を示す国・地域コード (=73)
    FI = 73
    # FRANCE を示す国・地域コード (=74)
    FR = 74
    # FRANCE, METROPOLITAN を示す国・地域コード (=75)
    FX = 75
    # FRENCH GUIANA を示す国・地域コード (=76)
    GF = 76
    # FRENCH POLYNESIA を示す国・地域コード (=77)
    PF = 77
    # FRENCH SOUTHERN TERRITORIES を示す国・地域コード (=78)
    TF = 78
    # GABON を示す国・地域コード (=79)
    GA = 79
    # GAMBIA を示す国・地域コード (=80)
    GM = 80
    # GEORGIA を示す国・地域コード (=81)
    GE = 81
    # GERMANY を示す国・地域コード (=82)
    DE = 82
    # GHANA を示す国・地域コード (=83)
    GH = 83
    # GIBRALTAR を示す国・地域コード (=84)
    GI = 84
    # GREECE を示す国・地域コード (=85)
    GR = 85
    # GREENLAND を示す国・地域コード (=86)
    GL = 86
    # GRENADA を示す国・地域コード (=87)
    GD = 87
    # GUADELOUPE を示す国・地域コード (=88)
    GP = 88
    # GUAM を示す国・地域コード (=89)
    GU = 89
    # GUATEMALA を示す国・地域コード (=90)
    GT = 90
    # GUINEA を示す国・地域コード (=91)
    GN = 91
    # GUINEA-BISSAU を示す国・地域コード (=92)
    GW = 92
    # GUYANA を示す国・地域コード (=93)
    GY = 93
    # HAITI を示す国・地域コード (=94)
    HT = 94
    # HEARD AND MC DONALD ISLANDS を示す国・地域コード (=95)
    HM = 95
    # HONDURAS を示す国・地域コード (=96)
    HN = 96
    # HONG KONG を示す国・地域コード (=97)
    HK = 97
    # HUNGARY を示す国・地域コード (=98)
    HU = 98
    # ICELAND を示す国・地域コード (=99)
    IS = 99
    # INDIA を示す国・地域コード (=100)
    IN = 100
    # INDONESIA を示す国・地域コード (=101)
    ID = 101
    # IRAN (ISLAMIC REPUBLIC OF) を示す国・地域コード (=102)
    IR = 102
    # IRAQ を示す国・地域コード (=103)
    IQ = 103
    # IRELAND を示す国・地域コード (=104)
    IE = 104
    # ISRAEL を示す国・地域コード (=105)
    IL = 105
    # ITALY を示す国・地域コード (=106)
    IT = 106
    # JAMAICA を示す国・地域コード (=107)
    JM = 107
    # JAPAN を示す国・地域コード (=108)
    JP = 108
    # JORDAN を示す国・地域コード (=109)
    JO = 109
    # KAZAKHSTAN を示す国・地域コード (=110)
    KZ = 110
    # KENYA を示す国・地域コード (=111)
    KE = 111
    # KIRIBATI を示す国・地域コード (=112)
    KI = 112
    # KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF を示す国・地域コード (=113)
    KP = 113
    # KOREA, REPUBLIC OF を示す国・地域コード (=114)
    KR = 114
    # KUWAIT を示す国・地域コード (=115)
    KW = 115
    # KYRGYZSTAN を示す国・地域コード (=116)
    KG = 116
    # LAO PEOPLE'S DEMOCRATIC REPUBLIC を示す国・地域コード (=117)
    LA = 117
    # LATVIA を示す国・地域コード (=118)
    LV = 118
    # LEBANON を示す国・地域コード (=119)
    LB = 119
    # LESOTHO を示す国・地域コード (=120)
    LS = 120
    # LIBERIA を示す国・地域コード (=121)
    LR = 121
    # LIBYAN ARAB JAMAHIRIYA を示す国・地域コード (=122)
    LY = 122
    # LIECHTENSTEIN を示す国・地域コード (=123)
    LI = 123
    # LITHUANIA を示す国・地域コード (=124)
    LT = 124
    # LUXEMBOURG を示す国・地域コード (=125)
    LU = 125
    # MACAU を示す国・地域コード (=126)
    MO = 126
    # MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF を示す国・地域コード (=127)
    MK = 127
    # MADAGASCAR を示す国・地域コード (=128)
    MG = 128
    # MALAWI を示す国・地域コード (=129)
    MW = 129
    # MALAYSIA を示す国・地域コード (=130)
    MY = 130
    # MALDIVES を示す国・地域コード (=131)
    MV = 131
    # MALI を示す国・地域コード (=132)
    ML = 132
    # MALTA を示す国・地域コード (=133)
    MT = 133
    # MARSHALL ISLANDS を示す国・地域コード (=134)
    MH = 134
    # MARTINIQUE を示す国・地域コード (=135)
    MQ = 135
    # MAURITANIA を示す国・地域コード (=136)
    MR = 136
    # MAURITIUS を示す国・地域コード (=137)
    MU = 137
    # MAYOTTE を示す国・地域コード (=138)
    YT = 138
    # MEXICO を示す国・地域コード (=139)
    MX = 139
    # MICRONESIA, FEDERATED STATES OF を示す国・地域コード (=140)
    FM = 140
    # MOLDOVA, REPUBLIC OF を示す国・地域コード (=141)
    MD = 141
    # MONACO を示す国・地域コード (=142)
    MC = 142
    # MONGOLIA を示す国・地域コード (=143)
    MN = 143
    # MONTSERRAT を示す国・地域コード (=144)
    MS = 144
    # MOROCCO を示す国・地域コード (=145)
    MA = 145
    # MOZAMBIQUE を示す国・地域コード (=146)
    MZ = 146
    # MYANMAR を示す国・地域コード (=147)
    MM = 147
    # NAMIBIA を示す国・地域コード (=148)
    NA = 148
    # NAURU を示す国・地域コード (=149)
    NR = 149
    # NEPAL を示す国・地域コード (=150)
    NP = 150
    # NETHERLANDS を示す国・地域コード (=151)
    NL = 151
    # NETHERLANDS ANTILLES を示す国・地域コード (=152)
    AN = 152
    # NEW CALEDONIA を示す国・地域コード (=153)
    NC = 153
    # NEW ZEALAND を示す国・地域コード (=154)
    NZ = 154
    # NICARAGUA を示す国・地域コード (=155)
    NI = 155
    # NIGER を示す国・地域コード (=156)
    NE = 156
    # NIGERIA を示す国・地域コード (=157)
    NG = 157
    # NIUE を示す国・地域コード (=158)
    NU = 158
    # norfolk ISLAND を示す国・地域コード (=159)
    NF = 159
    # NORTHERN MARIANA ISLANDS を示す国・地域コード (=160)
    MP = 160
    # NORWAY を示す国・地域コード (=161)
    NO = 161
    # OMAN を示す国・地域コード (=162)
    OM = 162
    # PAKISTAN を示す国・地域コード (=163)
    PK = 163
    # PALAU を示す国・地域コード (=164)
    PW = 164
    # PALESTINIAN TERRITORY, Occupied を示す国・地域コード (=165)
    PS = 165
    # PANAMA を示す国・地域コード (=166)
    PA = 166
    # PAPUA NEW GUINEA を示す国・地域コード (=167)
    PG = 167
    # PARAGUAY を示す国・地域コード (=168)
    PY = 168
    # PERU を示す国・地域コード (=169)
    PE = 169
    # PHILIPPINES を示す国・地域コード (=170)
    PH = 170
    # PITCAIRN を示す国・地域コード (=171)
    PN = 171
    # POLAND を示す国・地域コード (=172)
    PL = 172
    # PORTUGAL を示す国・地域コード (=173)
    PT = 173
    # PUERTO RICO を示す国・地域コード (=174)
    PR = 174
    # QATAR を示す国・地域コード (=175)
    QA = 175
    # REUNION を示す国・地域コード (=176)
    RE = 176
    # ROMANIA を示す国・地域コード (=177)
    RO = 177
    # RUSSIAN FEDERATION を示す国・地域コード (=178)
    RU = 178
    # RWANDA を示す国・地域コード (=179)
    RW = 179
    # SAINT KITTS AND NEVIS を示す国・地域コード (=180)
    KN = 180
    # SAINT LUCIA を示す国・地域コード (=181)
    LC = 181
    # SAINT VINCENT AND THE GRENADINES を示す国・地域コード (=182)
    VC = 182
    # SAMOA を示す国・地域コード (=183)
    WS = 183
    # SAN MARINO を示す国・地域コード (=184)
    SM = 184
    # SAO TOME AND PRINCIPE を示す国・地域コード (=185)
    ST = 185
    # SAUDI ARABIA を示す国・地域コード (=186)
    SA = 186
    # SENEGAL を示す国・地域コード (=187)
    SN = 187
    # SEYCHELLES を示す国・地域コード (=188)
    SC = 188
    # SIERRA LEONE を示す国・地域コード (=189)
    SL = 189
    # SINGAPORE を示す国・地域コード (=190)
    SG = 190
    # SLOVAKIA (Slovak Republic) を示す国・地域コード (=191)
    SK = 191
    # SLOVENIA を示す国・地域コード (=192)
    SI = 192
    # SOLOMON ISLANDS を示す国・地域コード (=193)
    SB = 193
    # SOMALIA を示す国・地域コード (=194)
    SO = 194
    # SOUTH AFRICA を示す国・地域コード (=195)
    ZA = 195
    # SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS を示す国・地域コード (=196)
    GS = 196
    # SPAIN を示す国・地域コード (=197)
    ES = 197
    # SRI LANKA を示す国・地域コード (=198)
    LK = 198
    # ST. HELENA を示す国・地域コード (=199)
    SH = 199
    # ST. PIERRE AND MIQUELON を示す国・地域コード (=200)
    PM = 200
    # SUDAN を示す国・地域コード (=201)
    SD = 201
    # SURINAME を示す国・地域コード (=202)
    SR = 202
    # SVALBARD AND JAN MAYEN ISLANDS を示す国・地域コード (=203)
    SJ = 203
    # SWAZILAND を示す国・地域コード (=204)
    SZ = 204
    # SWEDEN を示す国・地域コード (=205)
    SE = 205
    # SWITZERLAND を示す国・地域コード (=206)
    CH = 206
    # SYRIAN ARAB REPUBLIC を示す国・地域コード (=207)
    SY = 207
    # TAIWAN を示す国・地域コード (=208)
    TW = 208
    # TAJIKISTAN を示す国・地域コード (=209)
    TJ = 209
    # TANZANIA, UNITED REPUBLIC OF を示す国・地域コード (=210)
    TZ = 210
    # THAILAND を示す国・地域コード (=211)
    TH = 211
    # TOGO を示す国・地域コード (=212)
    TG = 212
    # TOKELAU を示す国・地域コード (=213)
    TK = 213
    # TONGA を示す国・地域コード (=214)
    TO = 214
    # TRINIDAD AND TOBAGO を示す国・地域コード (=215)
    TT = 215
    # TUNISIA を示す国・地域コード (=216)
    TN = 216
    # TURKEY を示す国・地域コード (=217)
    TR = 217
    # TURKMENISTAN を示す国・地域コード (=218)
    TM = 218
    # TURKS AND CAICOS ISLANDS を示す国・地域コード (=219)
    TC = 219
    # TUVALU を示す国・地域コード (=220)
    TV = 220
    # UGANDA を示す国・地域コード (=221)
    UG = 221
    # UKRAINE を示す国・地域コード (=222)
    UA = 222
    # UNITED ARAB EMIRATES を示す国・地域コード (=223)
    AE = 223
    # UNITED KINGDOM を示す国・地域コード (=224)
    GB = 224
    # UNITED STATES を示す国・地域コード (=225)
    US = 225
    # UNITED STATES MINOR OUTLYING ISLANDS を示す国・地域コード (=226)
    UM = 226
    # URUGUAY を示す国・地域コード (=227)
    UY = 227
    # UZBEKISTAN を示す国・地域コード (=228)
    UZ = 228
    # VANUATU を示す国・地域コード (=229)
    VU = 229
    # VATICAN CITY STATE (HOLY SEE) を示す国・地域コード (=230)
    VA = 230
    # VENEZUELA を示す国・地域コード (=231)
    VE = 231
    # VIET NAM を示す国・地域コード (=232)
    VN = 232
    # VIRGIN ISLANDS (BRITISH) を示す国・地域コード (=233)
    VG = 233
    # VIRGIN ISLANDS (U.S.) を示す国・地域コード (=234)
    VI = 234
    # WALLIS AND FUTUNA ISLANDS を示す国・地域コード (=235)
    WF = 235
    # WESTERN SAHARA を示す国・地域コード (=236)
    EH = 236
    # YEMEN を示す国・地域コード (=237)
    YE = 237
    # YUGOSLAVIA を示す国・地域コード (=238)
    YU = 238
    # ZAMBIA を示す国・地域コード (=239)
    ZM = 239
    # ZIMBABWE を示す国・地域コード (=240)
    ZW = 240

    # 国・地域コードの範囲外開始を示す値 (=241)
    LAST = 241

    # 各国・地域コードに対応する文字列（シンボル）が格納されている配列。
    code_symbol = (
        "undefined",
        "af",  # 1. AFGHANISTAN
        "al",  # 2. ALBANIA
        "dz",  # 3. ALGERIA
        "as",  # 4. AMERICAN SAMOA
        "ad",  # 5. ANDORRA
        "ao",  # 6. ANGOLA
        "ai",  # 7. ANGUILLA
        "aq",  # 8. ANTARCTICA
        "ag",  # 9. ANTIGUA AND BARBUDA
        "ar",  # 10. ARGENTINA
        "am",  # 11. ARMENIA
        "aw",  # 12. ARUBA
        "au",  # 13. AUSTRALIA
        "at",  # 14. AUSTRIA
        "az",  # 15. AZERBAIJAN
        "bs",  # 16. BAHAMAS
        "bh",  # 17. BAHRAIN
        "bd",  # 18. BANGLADESH
        "bb",  # 19. BARBADOS
        "by",  # 20. BELARUS
        "be",  # 21. BELGIUM
        "bz",  # 22. BELIZE
        "bj",  # 23. BENIN
        "bm",  # 24. BERMUDA
        "bt",  # 25. BHUTAN
        "bo",  # 26. BOLIVIA
        "ba",  # 27. BOSNIA AND HERZEGOWINA
        "bw",  # 28. BOTSWANA
        "bv",  # 29. BOUVET ISLAND
        "br",  # 30. BRAZIL
        "io",  # 31. BRITISH INDIAN OCEAN TERRITORY
        "bn",  # 32. BRUNEI DARUSSALAM
        "bg",  # 33. BULGARIA
        "bf",  # 34. BURKINA FASO
        "bi",  # 35. BURUNDI
        "kh",  # 36. CAMBODIA
        "cm",  # 37. CAMEROON
        "ca",  # 38. CANADA
        "cv",  # 39. CAPE VERDE
        "ky",  # 40. CAYMAN ISLANDS
        "cf",  # 41. CENTRAL AFRICAN REPUBLIC
        "td",  # 42. CHAD
        "cl",  # 43. CHILE
        "cn",  # 44. CHINA
        "cx",  # 45. CHRISTMAS ISLAND
        "cc",  # 46. COCOS (KEELING) ISLANDS
        "co",  # 47. COLOMBIA
        "km",  # 48. COMOROS
        "cd",  # 49. CONGO, Democratic Republic of (was Zaire)
        "cg",  # 50. CONGO, People's Republic of
        "ck",  # 51. COOK ISLANDS
        "cr",  # 52. COSTA RICA
        "ci",  # 53. COTE D'IVOIRE
        "hr",  # 54. CROATIA (local name: Hrvatska)
        "cu",  # 55. CUBA
        "cy",  # 56. CYPRUS
        "cz",  # 57. CZECH REPUBLIC
        "dk",  # 58. DENMARK
        "dj",  # 59. DJIBOUTI
        "dm",  # 60. DOMINICA
        "do",  # 61. DOMINICAN REPUBLIC
        "tl",  # 62. EAST TIMOR
        "ec",  # 63. ECUADOR
        "eg",  # 64. EGYPT
        "sv",  # 65. EL SALVADOR
        "gq",  # 66. EQUATORIAL GUINEA
        "er",  # 67. ERITREA
        "ee",  # 68. ESTONIA
        "et",  # 69. ETHIOPIA
        "fk",  # 70. FALKLAND ISLANDS (MALVINAS)
        "fo",  # 71. FAROE ISLANDS
        "fj",  # 72. FIJI
        "fi",  # 73. FINLAND
        "fr",  # 74. FRANCE
        "fx",  # 75. FRANCE, METROPOLITAN
        "gf",  # 76. FRENCH GUIANA
        "pf",  # 77. FRENCH POLYNESIA
        "tf",  # 78. FRENCH SOUTHERN TERRITORIES
        "ga",  # 79. GABON
        "gm",  # 80. GAMBIA
        "ge",  # 81. GEORGIA
        "de",  # 82. GERMANY
        "gh",  # 83. GHANA
        "gi",  # 84. GIBRALTAR
        "gr",  # 85. GREECE
        "gl",  # 86. GREENLAND
        "gd",  # 87. GRENADA
        "gp",  # 88. GUADELOUPE
        "gu",  # 89. GUAM
        "gt",  # 90. GUATEMALA
        "gn",  # 91. GUINEA
        "gw",  # 92. GUINEA-BISSAU
        "gy",  # 93. GUYANA
        "ht",  # 94. HAITI
        "hm",  # 95. HEARD AND MC DONALD ISLANDS
        "hn",  # 96. HONDURAS
        "hk",  # 97. HONG KONG
        "hu",  # 98. HUNGARY
        "is",  # 99. ICELAND
        "in",  # 100. INDIA
        "id",  # 101. INDONESIA
        "ir",  # 102. IRAN (ISLAMIC REPUBLIC OF)
        "iq",  # 103. IRAQ
        "ie",  # 104. IRELAND
        "il",  # 105. ISRAEL
        "it",  # 106. ITALY
        "jm",  # 107. JAMAICA
        "jp",  # 108. JAPAN
        "jo",  # 109. JORDAN
        "kz",  # 110. KAZAKHSTAN
        "ke",  # 111. KENYA
        "ki",  # 112. KIRIBATI
        "kp",  # 113. KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF
        "kr",  # 114. KOREA, REPUBLIC OF
        "kw",  # 115. KUWAIT
        "kg",  # 116. KYRGYZSTAN
        "la",  # 117. LAO PEOPLE'S DEMOCRATIC REPUBLIC
        "lv",  # 118. LATVIA
        "lb",  # 119. LEBANON
        "ls",  # 120. LESOTHO
        "lr",  # 121. LIBERIA
        "ly",  # 122. LIBYAN ARAB JAMAHIRIYA
        "li",  # 123. LIECHTENSTEIN
        "lt",  # 124. LITHUANIA
        "lu",  # 125. LUXEMBOURG
        "mo",  # 126. MACAU
        "mk",  # 127. MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
        "mg",  # 128. MADAGASCAR
        "mw",  # 129. MALAWI
        "my",  # 130. MALAYSIA
        "mv",  # 131. MALDIVES
        "ml",  # 132. MALI
        "mt",  # 133. MALTA
        "mh",  # 134. MARSHALL ISLANDS
        "mq",  # 135. MARTINIQUE
        "mr",  # 136. MAURITANIA
        "mu",  # 137. MAURITIUS
        "yt",  # 138. MAYOTTE
        "mx",  # 139. MEXICO
        "fm",  # 140. MICRONESIA, FEDERATED STATES OF
        "md",  # 141. MOLDOVA, REPUBLIC OF
        "mc",  # 142. MONACO
        "mn",  # 143. MONGOLIA
        "ms",  # 144. MONTSERRAT
        "ma",  # 145. MOROCCO
        "mz",  # 146. MOZAMBIQUE
        "mm",  # 147. MYANMAR
        "na",  # 148. NAMIBIA
        "nr",  # 149. NAURU
        "np",  # 150. NEPAL
        "nl",  # 151. NETHERLANDS
        "an",  # 152. NETHERLANDS ANTILLES
        "nc",  # 153. NEW CALEDONIA
        "nz",  # 154. NEW ZEALAND
        "ni",  # 155. NICARAGUA
        "ne",  # 156. NIGER
        "ng",  # 157. NIGERIA
        "nu",  # 158. NIUE
        "nf",  # 159. norfolk ISLAND
        "mp",  # 160. NORTHERN MARIANA ISLANDS
        "no",  # 161. NORWAY
        "om",  # 162. OMAN
        "pk",  # 163. PAKISTAN
        "pw",  # 164. PALAU
        "ps",  # 165. PALESTINIAN TERRITORY, Occupied
        "pa",  # 166. PANAMA
        "pg",  # 167. PAPUA NEW GUINEA
        "py",  # 168. PARAGUAY
        "pe",  # 169. PERU
        "ph",  # 170. PHILIPPINES
        "pn",  # 171. PITCAIRN
        "pl",  # 172. POLAND
        "pt",  # 173. PORTUGAL
        "pr",  # 174. PUERTO RICO
        "qa",  # 175. QATAR
        "re",  # 176. REUNION
        "ro",  # 177. ROMANIA
        "ru",  # 178. RUSSIAN FEDERATION
        "rw",  # 179. RWANDA
        "kn",  # 180. SAINT KITTS AND NEVIS
        "lc",  # 181. SAINT LUCIA
        "vc",  # 182. SAINT VINCENT AND THE GRENADINES
        "ws",  # 183. SAMOA
        "sm",  # 184. SAN MARINO
        "st",  # 185. SAO TOME AND PRINCIPE
        "sa",  # 186. SAUDI ARABIA
        "sn",  # 187. SENEGAL
        "sc",  # 188. SEYCHELLES
        "sl",  # 189. SIERRA LEONE
        "sg",  # 190. SINGAPORE
        "sk",  # 191. SLOVAKIA (Slovak Republic)
        "si",  # 192. SLOVENIA
        "sb",  # 193. SOLOMON ISLANDS
        "so",  # 194. SOMALIA
        "za",  # 195. SOUTH AFRICA
        "gs",  # 196. SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS
        "es",  # 197. SPAIN
        "lk",  # 198. SRI LANKA
        "sh",  # 199. ST. HELENA
        "pm",  # 200. ST. PIERRE AND MIQUELON
        "sd",  # 201. SUDAN
        "sr",  # 202. SURINAME
        "sj",  # 203. SVALBARD AND JAN MAYEN ISLANDS
        "sz",  # 204. SWAZILAND
        "se",  # 205. SWEDEN
        "ch",  # 206. SWITZERLAND
        "sy",  # 207. SYRIAN ARAB REPUBLIC
        "tw",  # 208. TAIWAN
        "tj",  # 209. TAJIKISTAN
        "tz",  # 210. TANZANIA, UNITED REPUBLIC OF
        "th",  # 211. THAILAND
        "tg",  # 212. TOGO
        "tk",  # 213. TOKELAU
        "to",  # 214. TONGA
        "tt",  # 215. TRINIDAD AND TOBAGO
        "tn",  # 216. TUNISIA
        "tr",  # 217. TURKEY
        "tm",  # 218. TURKMENISTAN
        "tc",  # 219. TURKS AND CAICOS ISLANDS
        "tv",  # 220. TUVALU
        "ug",  # 221. UGANDA
        "ua",  # 222. UKRAINE
        "ae",  # 223. UNITED ARAB EMIRATES
        "gb",  # 224. UNITED KINGDOM
        "us",  # 225. UNITED STATES
        "um",  # 226. UNITED STATES MINOR OUTLYING ISLANDS
        "uy",  # 227. URUGUAY
        "uz",  # 228. UZBEKISTAN
        "vu",  # 229. VANUATU
        "va",  # 230. VATICAN CITY STATE (HOLY SEE)
        "ve",  # 231. VENEZUELA
        "vn",  # 232. VIET NAM
        "vg",  # 233. VIRGIN ISLANDS (BRITISH)
        "vi",  # 234. VIRGIN ISLANDS (U.S.)
        "wf",  # 235. WALLIS AND FUTUNA ISLANDS
        "eh",  # 236. WESTERN SAHARA
        "ye",  # 237. YEMEN
        "yu",  # 238. YUGOSLAVIA
        "zm",  # 239. ZAMBIA
        "zw"   # 240. ZIMBABWE
    )

    @staticmethod
    def to_symbol(code_: int) -> int:
        """国・地域コードから対応するシンボルを返す

        Args:
            code_ (int): 国・地域コード

        Returns:
            int: 国・地域コードを示す文字列（シンボル）
        """
        if not Country.is_valid(code_):
            code_ = Country.UNDEFINED
        return Country.code_symbol[code_]

    @staticmethod
    def is_valid(code_: int) -> bool:
        """国・地域コードが範囲内であるかどうかを確認する.

        Args:
            code_ (int): 国・地域コード

        Returns:
            bool: 国・地域コードが範囲内の場合True. 範囲外の場合False
        """
        if code_ <= Country.UNDEFINED or code_ >= Country.LAST:
            return False
        return True

    @staticmethod
    def to_code(symbol: str) -> int:
        """シンボルから対応する国・地域コードを返す.

        Args:
            symbol (str): 国・地域コードを示す文字列（シンボル）

        Returns:
            int: 国・地域コード
        """
        if len(symbol) != 2:
            return Country.UNDEFINED

        code = Country.UNDEFINED
        try:
            code = Country.code_symbol.index(symbol)
        except ValueError:
            # 対応するindexがない場合はUNDEFINEDのまま
            pass

        return code
