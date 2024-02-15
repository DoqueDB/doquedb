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
######################################################################################################
# Net::TRMeister -- Network protocol for Perl DBI/DBD
# member:
# methods:
######################################################################################################
#!/user/bin/perl.exe -w
package Net::TRMeister;

use strict;
use Carp;
use Encode qw/encode decode from_to/;

our $VERSION = '16.5';

#print all stacks information when die
$SIG{__DIE__} = sub{confess "@_";};
#the hash holds error message map
require 'Net/ErrorDef.pl';

#how can I get right encoding from locale?
my $encoding = "utf8"; # temporaly
my $language = "English"; # temporaly
my $system_encoding = "UTF-16BE";

my $ErrorDef = $Net::TRMeister::ErrorDef{$language};
if (!defined $ErrorDef) {
    $ErrorDef = $Net::TRMeister::ErrorDef{"English"};
}

# ModLanguage
my @lang_code = (
"",
"aa",		# Afar
"ab",		# Abkhazian
"af",		# Afrikaans
"am",		# Amharic
"ar",		# Arabic
"as",		# Assamese
"ay",		# Aymara
"az",		# Azerbaijani

"ba",		# Bashkir
"be",		# Byelorussian
"bg",		# Bulgarian
"bh",		# Bihari
"bi",		# Bislama
"bn",		# Bengali; Bangla
"bo",		# Tibetan
"br",		# Breton

"ca",		# Catalan
"co",		# Corsican
"cs",		# Czech
"cy",		# Welsh

"da",		# Danish
"de",		# German
"dz",		# Bhutani

"el",		# Greek
"en",		# English
"eo",		# Esperanto
"es",		# Spanish
"et",		# Estonian
"eu",		# Basque

"fa",		# Persian
"fi",		# Finnish
"fj",		# Fiji
"fo",		# Faroese
"fr",		# French
"fy",		# Frisian

"ga",		# Irish
"gd",		# Scots Gaelic
"gl",		# Galician
"gn",		# Guarani
"gu",		# Gujarati

"ha",		# Hausa
"he",		# Hebrew (formerly iw)
"hi",		# Hindi
"hr",		# Croatian
"hu",		# Hungarian
"hy",		# Armenian

"ia",		# Interlingua
"id",		# Indonesian (formerly in)
"ie",		# Interlingue
"ik",		# Inupiak
"is",		# Icelandic
"it",		# Italian
"iu",		# Inuktitut

"ja",		# Japanese
"jw",		# Javanese

"ka",		# Georgian
"kk",		# Kazakh
"kl",		# Greenlandic
"km",		# Cambodian
"kn",		# Kannada
"ko",		# Korean
"ks",		# Kashmiri
"ku",		# Kurdish
"ky",		# Kirghiz

"la",		# Latin
"ln",		# Lingala
"lo",		# Laothian
"lt",		# Lithuanian
"lv",		# Latvian, Lettish

"mg",		# Malagasy
"mi",		# Maori
"mk",		# Macedonian
"ml",		# Malayalam
"mn",		# Mongolian
"mo",		# Moldavian
"mr",		# Marathi
"ms",		# Malay
"mt",		# Maltese
"my",		# Burmese

"na",		# Nauru
"ne",		# Nepali
"nl",		# Dutch
"no",		# Norwegian

"oc",		# Occitan
"om",		# (Afan) Oromo
"or",		# Oriya

"pa",		# Punjabi
"pl",		# Polish
"ps",		# Pashto, Pushto
"pt",		# Portuguese

"qu",		# Quechua

"rm",		# Rhaeto-Romance
"rn",		# Kirundi
"ro",		# Romanian
"ru",		# Russian
"rw",		# Kinyarwanda

"sa",		# Sanskrit
"sd",		# Sindhi
"sg",		# Sangho
"sh",		# Serbo-Croatian
"si",		# Sinhalese
"sk",		# Slovak
"sl",		# Slovenian
"sm",		# Samoan
"sn",		# Shona
"so",		# Somali
"sq",		# Albanian
"sr",		# Serbian
"ss",		# Siswati
"st",		# Sesotho
"su",		# Sundanese
"sv",		# Swedish
"sw",		# Swahili

"ta",		# Tamil
"te",		# Telugu
"tg",		# Tajik
"th",		# Thai
"ti",		# Tigrinya
"tk",		# Turkmen
"tl",		# Tagalog
"tn",		# Setswana
"to",		# Tonga
"tr",		# Turkish
"ts",		# Tsonga
"tt",		# Tatar
"tw",		# Twi

"ug",		# Uighur
"uk",		# Ukrainian
"ur",		# Urdu
"uz",		# Uzbek

"vi",		# Vietnamese
"vo",		# Volapuk

"wo",		# Wolof

"xh",		# Xhosa

"yi",		# Yiddish (formerly ji)
"yo",		# Yoruba

"za",		# Zhuang
"zh",		# Chinese
"zu"		# Zulu
);

# ModCountry
my @country_code = (
"",
"af",	#   1. AFGHANISTAN
"al",	#   2. ALBANIA
"dz",	#   3. ALGERIA
"as",	#   4. AMERICAN SAMOA
"ad",	#   5. ANDORRA
"ao",	#   6. ANGOLA
"ai",	#   7. ANGUILLA
"aq",	#   8. ANTARCTICA
"ag",	#   9. ANTIGUA AND BARBUDA
"ar",	#  10. ARGENTINA
"am",	#  11. ARMENIA
"aw",	#  12. ARUBA
"au",	#  13. AUSTRALIA
"at",	#  14. AUSTRIA
"az",	#  15. AZERBAIJAN
"bs",	#  16. BAHAMAS
"bh",	#  17. BAHRAIN
"bd",	#  18. BANGLADESH
"bb",	#  19. BARBADOS
"by",	#  20. BELARUS
"be",	#  21. BELGIUM
"bz",	#  22. BELIZE
"bj",	#  23. BENIN
"bm",	#  24. BERMUDA
"bt",	#  25. BHUTAN
"bo",	#  26. BOLIVIA
"ba",	#  27. BOSNIA AND HERZEGOWINA
"bw",	#  28. BOTSWANA
"bv",	#  29. BOUVET ISLAND
"br",	#  30. BRAZIL
"io",	#  31. BRITISH INDIAN OCEAN TERRITORY
"bn",	#  32. BRUNEI DARUSSALAM
"bg",	#  33. BULGARIA
"bf",	#  34. BURKINA FASO
"bi",	#  35. BURUNDI
"kh",	#  36. CAMBODIA
"cm",	#  37. CAMEROON
"ca",	#  38. CANADA
"cv",	#  39. CAPE VERDE
"ky",	#  40. CAYMAN ISLANDS
"cf",	#  41. CENTRAL AFRICAN REPUBLIC
"td",	#  42. CHAD
"cl",	#  43. CHILE
"cn",	#  44. CHINA
"cx",	#  45. CHRISTMAS ISLAND
"cc",	#  46. COCOS (KEELING) ISLANDS
"co",	#  47. COLOMBIA
"km",	#  48. COMOROS
"cd",	#  49. CONGO, Democratic Republic of (was Zaire)
"cg",	#  50. CONGO, People's Republic of
"ck",	#  51. COOK ISLANDS
"cr",	#  52. COSTA RICA
"ci",	#  53. COTE D'IVOIRE
"hr",	#  54. CROATIA (local name: Hrvatska)
"cu",	#  55. CUBA
"cy",	#  56. CYPRUS
"cz",	#  57. CZECH REPUBLIC
"dk",	#  58. DENMARK
"dj",	#  59. DJIBOUTI
"dm",	#  60. DOMINICA
"do",	#  61. DOMINICAN REPUBLIC
"tl",	#  62. EAST TIMOR
"ec",	#  63. ECUADOR
"eg",	#  64. EGYPT
"sv",	#  65. EL SALVADOR
"gq",	#  66. EQUATORIAL GUINEA
"er",	#  67. ERITREA
"ee",	#  68. ESTONIA
"et",	#  69. ETHIOPIA
"fk",	#  70. FALKLAND ISLANDS (MALVINAS)
"fo",	#  71. FAROE ISLANDS
"fj",	#  72. FIJI
"fi",	#  73. FINLAND
"fr",	#  74. FRANCE
"fx",	#  75. FRANCE, METROPOLITAN
"gf",	#  76. FRENCH GUIANA
"pf",	#  77. FRENCH POLYNESIA
"tf",	#  78. FRENCH SOUTHERN TERRITORIES
"ga",	#  79. GABON
"gm",	#  80. GAMBIA
"ge",	#  81. GEORGIA
"de",	#  82. GERMANY
"gh",	#  83. GHANA
"gi",	#  84. GIBRALTAR
"gr",	#  85. GREECE
"gl",	#  86. GREENLAND
"gd",	#  87. GRENADA
"gp",	#  88. GUADELOUPE
"gu",	#  89. GUAM
"gt",	#  90. GUATEMALA
"gn",	#  91. GUINEA
"gw",	#  92. GUINEA-BISSAU
"gy",	#  93. GUYANA
"ht",	#  94. HAITI
"hm",	#  95. HEARD AND MC DONALD ISLANDS
"hn",	#  96. HONDURAS
"hk",	#  97. HONG KONG
"hu",	#  98. HUNGARY
"is",	#  99. ICELAND
"in",	# 100. INDIA
"id",	# 101. INDONESIA
"ir",	# 102. IRAN (ISLAMIC REPUBLIC OF)
"iq",	# 103. IRAQ
"ie",	# 104. IRELAND
"il",	# 105. ISRAEL
"it",	# 106. ITALY
"jm",	# 107. JAMAICA
"jp",	# 108. JAPAN
"jo",	# 109. JORDAN
"kz",	# 110. KAZAKHSTAN
"ke",	# 111. KENYA
"ki",	# 112. KIRIBATI
"kp",	# 113. KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF
"kr",	# 114. KOREA, REPUBLIC OF
"kw",	# 115. KUWAIT
"kg",	# 116. KYRGYZSTAN
"la",	# 117. LAO PEOPLE'S DEMOCRATIC REPUBLIC
"lv",	# 118. LATVIA
"lb",	# 119. LEBANON
"ls",	# 120. LESOTHO
"lr",	# 121. LIBERIA
"ly",	# 122. LIBYAN ARAB JAMAHIRIYA
"li",	# 123. LIECHTENSTEIN
"lt",	# 124. LITHUANIA
"lu",	# 125. LUXEMBOURG
"mo",	# 126. MACAU
"mk",	# 127. MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF
"mg",	# 128. MADAGASCAR
"mw",	# 129. MALAWI
"my",	# 130. MALAYSIA
"mv",	# 131. MALDIVES
"ml",	# 132. MALI
"mt",	# 133. MALTA
"mh",	# 134. MARSHALL ISLANDS
"mq",	# 135. MARTINIQUE
"mr",	# 136. MAURITANIA
"mu",	# 137. MAURITIUS
"yt",	# 138. MAYOTTE
"mx",	# 139. MEXICO
"fm",	# 140. MICRONESIA, FEDERATED STATES OF
"md",	# 141. MOLDOVA, REPUBLIC OF
"mc",	# 142. MONACO
"mn",	# 143. MONGOLIA
"ms",	# 144. MONTSERRAT
"ma",	# 145. MOROCCO
"mz",	# 146. MOZAMBIQUE
"mm",	# 147. MYANMAR
"na",	# 148. NAMIBIA
"nr",	# 149. NAURU
"np",	# 150. NEPAL
"nl",	# 151. NETHERLANDS
"an",	# 152. NETHERLANDS ANTILLES
"nc",	# 153. NEW CALEDONIA
"nz",	# 154. NEW ZEALAND
"ni",	# 155. NICARAGUA
"ne",	# 156. NIGER
"ng",	# 157. NIGERIA
"nu",	# 158. NIUE
"nf",	# 159. NORFOLK ISLAND
"mp",	# 160. NORTHERN MARIANA ISLANDS
"no",	# 161. NORWAY
"om",	# 162. OMAN
"pk",	# 163. PAKISTAN
"pw",	# 164. PALAU
"ps",	# 165. PALESTINIAN TERRITORY, Occupied
"pa",	# 166. PANAMA
"pg",	# 167. PAPUA NEW GUINEA
"py",	# 168. PARAGUAY
"pe",	# 169. PERU
"ph",	# 170. PHILIPPINES
"pn",	# 171. PITCAIRN
"pl",	# 172. POLAND
"pt",	# 173. PORTUGAL
"pr",	# 174. PUERTO RICO
"qa",	# 175. QATAR
"re",	# 176. REUNION
"ro",	# 177. ROMANIA
"ru",	# 178. RUSSIAN FEDERATION
"rw",	# 179. RWANDA
"kn",	# 180. SAINT KITTS AND NEVIS
"lc",	# 181. SAINT LUCIA
"vc",	# 182. SAINT VINCENT AND THE GRENADINES
"ws",	# 183. SAMOA
"sm",	# 184. SAN MARINO
"st",	# 185. SAO TOME AND PRINCIPE
"sa",	# 186. SAUDI ARABIA
"sn",	# 187. SENEGAL
"sc",	# 188. SEYCHELLES
"sl",	# 189. SIERRA LEONE
"sg",	# 190. SINGAPORE
"sk",	# 191. SLOVAKIA (Slovak Republic)
"si",	# 192. SLOVENIA
"sb",	# 193. SOLOMON ISLANDS
"so",	# 194. SOMALIA
"za",	# 195. SOUTH AFRICA
"gs",	# 196. SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS
"es",	# 197. SPAIN
"lk",	# 198. SRI LANKA
"sh",	# 199. ST. HELENA
"pm",	# 200. ST. PIERRE AND MIQUELON
"sd",	# 201. SUDAN
"sr",	# 202. SURINAME
"sj",	# 203. SVALBARD AND JAN MAYEN ISLANDS
"sz",	# 204. SWAZILAND
"se",	# 205. SWEDEN
"ch",	# 206. SWITZERLAND
"sy",	# 207. SYRIAN ARAB REPUBLIC
"tw",	# 208. TAIWAN
"tj",	# 209. TAJIKISTAN
"tz",	# 210. TANZANIA, UNITED REPUBLIC OF
"th",	# 211. THAILAND
"tg",	# 212. TOGO
"tk",	# 213. TOKELAU
"to",	# 214. TONGA
"tt",	# 215. TRINIDAD AND TOBAGO
"tn",	# 216. TUNISIA
"tr",	# 217. TURKEY
"tm",	# 218. TURKMENISTAN
"tc",	# 219. TURKS AND CAICOS ISLANDS
"tv",	# 220. TUVALU
"ug",	# 221. UGANDA
"ua",	# 222. UKRAINE
"ae",	# 223. UNITED ARAB EMIRATES
"gb",	# 224. UNITED KINGDOM
"us",	# 225. UNITED STATES
"um",	# 226. UNITED STATES MINOR OUTLYING ISLANDS
"uy",	# 227. URUGUAY
"uz",	# 228. UZBEKISTAN
"vu",	# 229. VANUATU
"va",	# 230. VATICAN CITY STATE (HOLY SEE)
"ve",	# 231. VENEZUELA
"vn",	# 232. VIET NAM
"vg",	# 233. VIRGIN ISLANDS (BRITISH)
"vi",	# 234. VIRGIN ISLANDS (U.S.)
"wf",	# 235. WALLIS AND FUTUNA ISLANDS
"eh",	# 236. WESTERN SAHARA
"ye",	# 237. YEMEN
"yu",	# 238. YUGOSLAVIA
"zm",	# 239. ZAMBIA
"zw"	# 240. ZIMBABWE
);

#####################################################
# Net::TRMeister::new -- constructor
# args:
#	hostname		hostname for server,"localhost" as default
#	port		port number,54321 as default
#	database		database name, "DefaultDB" as default
#	dumpLargeObject		1:dump the binary data    0: just print "size = **"
#	largeObjectBufferSize		if dump the binary data,how much memory is allocated for object
#	truncOverBufferSize		if the size of binary data exceeds the largeObjectBufferSize,should we trunc it (1)or not(0)
#	debug		whether output debug info (1) or not(0)
#	userEncode		user's encoding
# return:
#	class instance,return undef if error
sub new
{
    my $class = shift;
    my %args  = @_;

    my $self = bless
    {
        hostname => $args{hostname}	||"localhost",
        port     => $args{port}	||54321,
        database => $args{database} ||"DefaultDB",
        protocolVersion => $args{protocolVersion},
        user => $args{user},
        password => $args{password},
        timeout  => $args{timeout}  || 60,
        'ServerConnection' => undef,
        'WorkerConnection' => undef,
        dumpLargeObject => $args{dumpLargeObject}||0,
        largeObjectBufferSize => $args{largeObjectBufferSize}||80,
        truncOverBufferSize => $args{truncOverBufferSize}||1,
        debug => $args{debug} ,
        errnum => undef,
        errstr => undef,
        userEncode => $args{userEncode}||'ascii',
    },$class;

    if($self->{debug})
    {
        print "\n[INFO BEGIN]Net::TRMeister::new\n";
        print "[INFO]The args are:\n";
        my $i = 0;
        foreach my $key (keys %args)
        {
            print "[INFO] $i th: key: ".$key." --> value: ".$args{$key}."\n";
            $i++;
        }
        print "[INFO END]Net::TRMeister::new\n";
    }
    #initialize the environment for query
    $self->_initialize;
    return $self;
}
#####################################################
# Net::TRMeister::_initialize -- init the environment
# args:
#	none
# return:
#	1 success
#	undef fail
sub _initialize
{
    my $self = shift;
    #load error message from ErrorDefinition.xml,currently we use static hash map %ErrorDef
    return undef if !defined $self->_open_port_for_server;
    return undef if !defined $self->_get_masterid_slaveid("ServerSocket");
    $self->_begin_connection;
    return undef if !defined $self->_create_session;
    1;
}
#####################################################
# Net::TRMeister::_open_port_for_server -- establish socket with server
# args:
#	none
# return:
#	1 success
#	undef fail
sub _open_port_for_server
{
    my $self = shift;
    my $conn = Net::TRMeister::ServerConnection->new(
        hostname => $self->{hostname},
        port => $self->{port},
        database => $self->{database},
        masterid => $self->{protocolVersion},
        timeout => $self->{timeout},
        debug => $self->{debug},
        father => $self,
        userEncode => $self->{userEncode},);
    $self->{ServerSocket} = $conn;
    $conn->connect;
}
#####################################################
# Net::TRMeister::_open_port_for_worker -- establis socket with worker
# args:
#	slaveid		the unique id which server assigned to this client
# return:
#	1 success
#	undef fail
sub _open_port_for_worker
{
    my $self = shift;
    my $slaveid = shift;
    my $conn = $self->{WorkerSocket};
    $conn->close if defined $conn;
    $conn = Net::TRMeister::WorkerConnection->new(
        hostname => $self->{hostname},
        port => $self->{port},
        database => $self->{database},
        masterid => $self->{protocolVersion},
        user => $self->{user},
        password => $self->{password},
        timeout => $self->{timeout},
        debug => $self->{debug},
        father => $self,
        userEncode =>$self->{userEncode},);
    $self->{WorkerSocket} = $conn;
    $conn->set_slaveid($slaveid);
    $conn->connect;
}
#####################################################
# Net::TRMeister::_get_masterid_slaveid -- get masterid and slaveid by different scoket
# args:
#	socketname		whether use serverconnection or workerconnection
# return:
#	1 success
sub _get_masterid_slaveid
{
    my $self = shift;
    my $socketname = shift;
    my $conn  = $self->{$socketname};
    $conn->get_masterid_slaveid($socketname);
}
#####################################################
# Net::TRMeister::_begin_connection -- begin a connection with server
# args:
#	none
# return:
#	1 success
#	undef fail
sub _begin_connection
{
    my $self = shift;
    my $conn = $self->{ServerSocket};
    return undef if !defined $conn->begin_connection;
    return 1;
}
#####################################################
# Net::TRMeister::_create_session -- create a session for worker
# args:
#	none
# return:
#	1 success
#	undef fail
sub _create_session
{
    my $self = shift;

    if(!defined($self->_begin_worker))
    {
	$self->_close_server_connection;
	$self->_close_worker_connection;
	return undef if !defined $self->_initialize;
    }
    $self->_begin_session;	# return 1 or undef
}
#####################################################
# Net::TRMeister::_begin_worker --
# args:
#	none
# return:
#	1 success
#	undef fail
sub _begin_worker
{
    my $self = shift;
    my $serverconn = $self->{ServerSocket};
    my $workerconn = $self->{WorkerSocket};

    my $slaveid = $workerconn ? $workerconn->{slaveid} :
    Net::TRMeister::Const->SlaveIDAny;
    my($slaveid1,$workerid) = $serverconn->begin_worker($slaveid);
    return undef if !defined($slaveid1);
    if($self->{debug})
    {
        print "\n[INFO EBGIN]Net::TRMeister::_begin_worker\n";
        print "[INFO]slaveid for worker is $slaveid1\n";
        print "[INFO]workerid for worker is $workerid\n";
        print "[INFO END]Net::TRMeister::_begin_worker\n";
    }

    if($slaveid == Net::TRMeister::Const->SlaveIDAny)
    {
        return undef if !defined $self->_open_port_for_worker($slaveid1);
        $self->_get_masterid_slaveid("WorkerSocket");
    }
    $workerconn = $self->{WorkerSocket};
    $workerconn->set_workerid($workerid);
    $workerconn->{socket}->{crypt} = $serverconn->{socket}->{crypt};

    1;
}
#####################################################
# Net::TRMeister::_cancel_worker --
# args:
#	none
# return:
#	1 success
#	undef fail
sub _cancel_worker
{
    my $self = shift;
    my $serverconn = $self->{ServerSocket};
    my $workerconn = $self->{WorkerSocket};

    my $workerid = $workerconn->{workerid};
    return undef if !defined $serverconn->cancel_worker($workerid);
    if($self->{debug})
    {
	print "\n[INFO BEGIN]Net::TRMeister::_cancel_worker\n";
	print "[INFO]workerid for worker is $workerid\n";
	print "[INFO END]Net::TRMeister::_cancel_worker\n";
    }
    1;
}
#####################################################
# Net::TRMeister::cancel_prev_access_result -- evacuate result tuples of last query,emtpy the cache
# args:
#	none
# return:
#	1 success
#	undef fail
sub cancel_prev_access_result
{
    my $self = shift;
    #still have to read the data in network cache pool
    my $workerconn = $self->{WorkerSocket};
    return 1 if !defined($workerconn);
    my $resultset = $workerconn->{resultset};
    return 1 if !defined($resultset);
    #if the result's fetching is not  finished
    if($resultset->is_end_of_result ==0)
    {
	#(1)cancel worker first
	return undef if !defined $self->_cancel_worker;

	if($self->{debug})
	{
	    print "\n[INFO BEGIN]Net::TRMeister::cancel_access_result\n";
	    print "[INFO]The following data are all get from socket cache pool\n";
	    print "[INFO END]Net::TRMeister::cancel_access_result\n";
	}
	#(2) read out all the tuples but do not output
	while(defined($resultset->each))
	{
	    ;
	}
	#(3)set flag for result set
	$resultset->set_end_of_result;
    }
    else
    {
	if($self->{debug})
	{
	    print "\n[INFO BEGIN]Net::TRMeister::cancel_access_result\n";
	    print "[INFO]All the results are accessed ,it does not need to cancel the worker \n";
	    print "\n[INFO END]Net::TRMeister::cancel_access_result\n";
	}
    }
    1;
}
#####################################################
# Net::TRMeister::_begin_session --
# args:
#	none
# return:
#	1 success
#	undef fail
sub _begin_session
{
    my $self = shift;
    my $workerconn = $self->{WorkerSocket};
    $workerconn->begin_session;	  #return 1 or undef
}
# Net::TRMeister::prepare_statement --
# args:
#	statement		the sql statement tobe prepared
# return:
#	1 success
#	undef fail
sub prepare_statement
{
    my $self = shift;
    my $statement = shift;

    $self->_begin_worker;

    my $workerconn = $self->{WorkerSocket};
    $workerconn->prepare_statement($statement);
}
# Net::TRMeister::execute_preparestatement --
# args:
#	prepareid		the id for the sql statement cached in server
#	parameterarray		the parameter for preparestatement
# return:
#	1 success
#	undef fail
sub execute_preparestatement
{
    my $self = shift;
    my $prepareid = shift;
    my $parameterarray = shift;

    return undef if !defined $self->_begin_worker;

    my $workerconn = $self->{WorkerSocket};
    $workerconn->execute_preparestatement($prepareid,$parameterarray);

}
# Net::TRMeister::_clean_prepareid -- clear the preparestatement cached in server side
# args:
#	prepareid		the id for the sql statement cached in server
# return:
#	1 success
#	undef fail
sub _clean_prepareid
{
    my $self = shift;
    my $prepareid = shift;

    my $workerconn = $self->{WorkerSocket};
    $workerconn->clean_prepareid($prepareid); #return 1 or undef
}
# Net::TRMeister::clean_prepareid_for_preparestatement -- clear the preparestatement cached in server side
# args:
#	prepareid		the id for the sql statement cached in server
# return:
#	1 success
#	undef fail
sub clean_prepareid_for_preparestatement
{
    my $self = shift;
    my $prepareid = shift;

    my $workerconn = $self->{WorkerSocket};
    return 1 if !defined($workerconn);

    return undef if !defined $self->_begin_worker;
    return undef if !defined $self->_clean_prepareid($prepareid);

    if($self->{debug})
    {
	print "\n[INFO BEGIN]Net::TRMeister::reset_prepareid_for_preparestatement\n";
	print "[INFO]The prepareid = $prepareid 's query plan is ereas in server side\n";
	print "[INFO END]Net::TRMeister::reset_prepareid_for_preparestatement\n";
    }
    1;
}
# Net::TRMeister::execute_nonpreparestatement --
# args:
#	statement		the sql statement
#	parameter		null
# return:
#	1 success
#	undef fail
sub execute_nonpreparestatement
{
    my $self =shift;
    my $statement = shift;
    my $parameter = shift;  #depending

    return undef if !defined $self->_begin_worker;

    my $workerconn = $self->{WorkerSocket};
    $workerconn->execute_nonpreparestatement($statement); #return 1
}
# Net::TRMeister::create_resultset -- create resultset instance and check first object
# args:
#	none
# return:
#	resultset the instance for result
sub create_resultset
{
    my $self = shift;
    my $workerconn = $self->{WorkerSocket};

    my $resultset = Net::TRMeister::ResultSet->new(
	'socket'=>$workerconn->{socket},
	debug=>$workerconn->{debug},
	dumpLargeObject => $self->{dumpLargeObject},
	largeObjectBufferSize => $self->{largeObjectBufferSize},
	truncOverBufferSize => $self->{truncOverBufferSize},
	userEncode => $self->{userEncode},
	father => $self,
	);

    $workerconn->{resultset} = $resultset;
    $resultset->check_first_object;

    $resultset;
}
# Net::TRMeister::_close_session --
# args:
#	none
# return:
#	1 success
#	undef fail
sub _close_session
{
    my $self = shift;
    my $workerconn = $self->{WorkerSocket};
    return 1 if !defined($workerconn);
    $self->_begin_worker;
    $workerconn->close_session; # return 1 or undef
}
# Net::TRMeister::_close_server_connection -- close server socket
# args:
#	none
# return:
#	1 success
sub _close_server_connection
{
    my $self = shift;

    my $serverconn = $self->{ServerSocket};
    return 1 if !defined($serverconn);
    $serverconn->close_server_connection; # return 1
    $self->{ServerSocket} = undef;
    1;
}
# Net::TRMeister::_close_worker_connection -- close worker socket
# args:
#	none
# return:
#	1 success
sub _close_worker_connection
{
    my $self = shift;

    my $workerconn = $self->{WorkerSocket};
    return 1 if !defined ( $workerconn);
    $workerconn->close_worker_connection;
    $self->{WorkerSocket} = undef;
    1;
}
# Net::TRMeister::close --
# args:
#	none
# return:
#	1 success
sub close
{
    my $self = shift;
    #(1)close session for worker
    return undef if !defined $self->_close_session;
    #(2)close server socket
    $self->_close_server_connection;
    #(2)close worker socket
    $self->_close_worker_connection;

    if($self->{debug})
    {
	print "\n[INFO BEGIN]Net::TRMeister::close\n";
	print "[INFO]Close succcessfully\n";
	print "[INFO END]Net::TRMeister::close\n"
    }
    1;
}

######################################################################################################
# Net::TRMeister::WorkerConnection -- Connection for worker
# member:
# methods:
######################################################################################################
package Net::TRMeister::WorkerConnection;

use IO::Socket;
use strict;
use Carp;
use Encode qw/encode decode from_to/;
#####################################################
# Net::TRMeister::WorkerConnection::new -- constructor
# args:
#	hostname		hostname for server,"localhost" as default
#	port		port number,54321 as default
#	database		database name, "DefaultDB" as default
#	timeout
#	masterid		the protocol version
#	debug		whether output debug info (1) or not(0)
#	userEncode		user's encoding
# return:
#	class instance
sub new
{
    my $class = shift;
    my %args  = @_;

    my $self = bless
    {
        hostname => $args{hostname}	|| "localhost",
        port     => $args{port}	||54321,
        database => $args{database} || "DefaultDB",
        timeout  => $args{timeout}  || 60,
        user => $args{user},
        password => $args{password},
        'socket' => undef,
        masterid => $args{masterid} ||(Net::TRMeister::Const->CurrentVersion |Net::TRMeister::Const->PasswordMask),
        slaveid  => undef,
        workerid => undef,
        sessionid => undef,
        resultset => undef,#holding the result set
        debug => $args{debug},
        father => $args{father},
        userEncode => $args{userEncode},
    },$class;

    if($self->{debug})
    {
        print "\n[INFO BEGIN]Net::TRMeister::WorkerConnection::new\n";
        print "[INFO] A new workconnection is created \n";
        print "[INFO END]Net::TRMeister::WorkerConnection::new\n";
    }
    return $self;
}
#####################################################
# Net::TRMeister::WorkerConnection::connect -- a socket for WorkerConnection is established
# args:
#	none
# return:
#	$trm socket instance or die
sub connect
{
    my $self = shift;
    my $sydney;
    my $trm =Net::TRMeister::Socket->new
    (
        hostname =>  $self->{hostname} ||"localhost",
        port     => $self->{port} ||54321,
        timeout  =>  $self->{timeout} || 60,
        'socket' => undef,
        debug => $self->{debug},
    )or die "\n[ERROR]:Couldn't connect with server\n";

    $self->{socket} = $trm;
    if($self->{debug})
    {
        print "\n[INFO BEGIN]Net::TRMeister::WorkerConnection::connect\n";
        print "The socket for WorkerConnection is created\n";
        print "[INFO END]Net::TRMeister::WorkerConnection::connect\n";
    }
    $trm;
}
#####################################################
# Net::TRMeister::WorkerConnection::get_masterid_slaveid --
# args:
#	socketname		use which socket to read masterid and slaveid
# return:
#	1 success
sub get_masterid_slaveid
{
    my $self = shift;
    my $socketname = shift;

#    print "BEGIN Net::TRMeister::WorkerConnection::get_masterid_slaveid\n";

    my $socket = $self->{socket};
    if($socketname eq "ServerSocket")
    {
        $self->{slaveid} = Net::TRMeister::Const->SlaveIDAny;
    }

    my $message0 = pack('N',$self->{masterid});
    my $message1 = pack('N',$self->{slaveid});
    $socket->send($message0, 0);
    $socket->send($message1, 0);
    $socket->flush;
    my $result0;
    my $result1;
    $socket->recv($result0,4,0);
    $socket->recv($result1,4,0);
    $result0 = unpack('N',$result0);
    $result1 = unpack('N',$result1);

    $self->{masterid} = $result0;
    $self->{slaveid} = $result1;
    if($self->{debug})
    {
        print "\n[INFO BEGIN]Net::TRMeister::WorkerConnection::get_masterid_slaveid\n";
        print "[INFO]MasterId = ".$self->{masterid}." SlaveID = ".$self->{slaveid}."\n";
        print "[INFO END]Net::TRMeister::WorkerConnection::get_masterid_slaveid\n";
    }
    1;
}

sub set_slaveid
{
    my $self =shift;
    my $slaveid = shift;
    $self->{slaveid} = $slaveid;
}
sub set_workerid
{
    my $self =shift;
    my $workerid = shift;
    $self->{workerid} = $workerid;
}
#####################################################
# Net::TRMeister::WorkerConnection::begin_session --
# args:
#	none
# return:
#	1 success
#	undef fail
sub begin_session
{
    my $self = shift;
    my $classtype;
    my $request;
    my $socket = $self->{socket};

    my $ret;
    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $ret = $socket->send($classtype,0);

    $request = pack('N',Net::TRMeister::Const->BeginSession2);
    $ret = $socket->send($request,0);

    $classtype=pack('N',Net::TRMeister::Const->StringDataClass);
    my $length = pack ('N',length($self->{database}));
    my $hostname;
    $hostname = encode($system_encoding,$self->{database});
    if($self->{debug})
    {
	print "\n[INFO BEGIN]Net::TRMeister::WorkerConnection::begin_session\n";
	print "the database's name is $self->{database}\n";
	print "the database's user is $self->{user}\n";
	print "the database's password is $self->{password}\n";
	print "\n[INFO END]Net::TRMeister::WorkerConnection::begin_session\n";
    }

    $socket->send($classtype,0);
    $socket->send($length,0);
    $socket->send($hostname,0);

    #user
    $length = pack ('N',length($self->{user}));
    my $user = encode($system_encoding,$self->{user});
    $socket->send($classtype,0);
    $socket->send($length,0);
    $socket->send($user,0);
    #password
    $length = pack ('N',length($self->{password}));
    my $password = encode($system_encoding,$self->{password});
    $socket->send($classtype,0);
    $socket->send($length,0);
    $socket->send($password,0);
    $socket->flush;

    my $session_id;

    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype)==0)
    {
	return undef;
    }
    $socket->recv($session_id,4,0);
    $session_id = unpack('N',$session_id);
    $self->{sessionid} = $session_id;

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) ==0)
    {
	return undef;
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);
    if(Net::TRMeister::Utility->check_status($self,$status)==0)
    {
	return undef;
    }
    if($self->{debug})
    {
	print "\n[INFO BEGIN]Net::TRMeister::WorkerConnection::begin_session\n";
	print "\n[INFO]the session id is $session_id\n";
    }
    1;
}
#####################################################
# Net::TRMeister::WorkerConnection::close_session --
# args:
#	none
# return:
#	1 success
#	undef fail
sub close_session
{
    my $self = shift;
    #return if undef $self->{sessionid} ;
    my $classtype;
    my $request;
    my $socket = $self->{socket};

    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $request = pack('N',Net::TRMeister::Const->EndSession);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$self->{sessionid});
    $socket->send($classtype,0);
    $socket->send($request,0);
    $socket->flush;

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) ==0)
    {
	return undef;
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);
    if(Net::TRMeister::Utility->check_status($self,$status)==0)
    {
	return undef;
    }

    $self->{sessionid} = undef;

    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::close_session***\n";
	print "\n[INFO]the worker session is closed\n";
    }
    1;
}
#####################################################
# Net::TRMeister::WorkerConnection::close_worker_connection --
# args:
#	none
# return:
#	1 success or croak
sub close_worker_connection
{
    my $self = shift;
    $self->{slaveid}  = undef;
    my $socket = $self->{socket};
    $socket->close or croak "\n[FAIL:]connect close failed\n";
    $self->{socket} = undef;
    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::close_worker_connection***\n";
	print "\n[INFO]connect close sucess\n"
    }
    1;
}
#####################################################
# Net::TRMeister::WorkerConnection::execute_nonpreparestatement --
# args:
#	statement: the sql statement to be execute
# return:
#	1 success
sub execute_nonpreparestatement
{
    my $self = shift;
    my $statement = shift;

    my $classtype;
    my $request;
    my $socket = $self->{socket};

    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $request = pack('N',Net::TRMeister::Const->ExecuteStatement);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$self->{sessionid});
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype=pack('N',Net::TRMeister::Const->StringDataClass);

    my $length = from_to($statement,$self->{userEncode},$system_encoding);
    $length = pack('N',$length/2);
    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::execute_nopreparestatement***\n";
	print "[INFO]the execute statement is $statement\n";
    }

    $socket->send($classtype,0);
    $socket->send($length,0);
    $socket->send($statement,0);

    my $elementnum = 0;
    $classtype = pack('N',Net::TRMeister::Const->DataArrayDataClass);
    $elementnum = pack('N',$elementnum);
    $socket->send($classtype,0);
    $socket->send($elementnum,0);
    $socket->flush;

    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::execute_nopreparestatement***\n";
	print "\n[INFO]the SQL statement $statement is successfully sent to Server\n";
    }
    1;
}
#####################################################
# Net::TRMeister::WorkerConnection::prepare_statement --
# args:
#	statement: the sql statement to be prepared
# return:
#	prepareid success
#	undef fail
sub prepare_statement
{
    my $self = shift;
    my $statement = shift;

    my $prepareid = undef;

    my $classtype;
    my $request;
    my $socket = $self->{socket};

    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $request = pack('N',Net::TRMeister::Const->PrepareStatement2);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$self->{sessionid});
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype=pack('N',Net::TRMeister::Const->StringDataClass);
    my $length = from_to($statement,$self->{userEncode},$system_encoding);
    $length = pack('N',$length/2);
    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::prepare_statement***\n";
	print "[INFO]the prepare statement is $statement\n";
    }

    $socket->send($classtype,0);
    $socket->send($length,0);
    $socket->send($statement,0);
    $socket->flush;

    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) ==0)
    {
	return undef;
    }
    $socket->recv($prepareid,4,0);
    $prepareid = unpack('N',$prepareid);

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) ==0)
    {
	return undef;
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);

    if(Net::TRMeister::Utility->check_status($self,$status)==0)
    {
	return undef;
    }

    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::prepare_statement***\n";
	print "\n[INFO]the SQL statement $statement is successfully sent to Server\n";
	print "\n[INFO] and the prepareid is $prepareid\n";
    }

    $prepareid;
}
#####################################################
# Net::TRMeister::WorkerConnection::clean_prepareid --
# args:
#	prepareid: the prepared plan to be clean
# return:
#	1 success
#	undef fail
sub clean_prepareid
{
    my $self = shift;
    my $prepareid = shift;

    my $classtype;
    my $request;
    my $socket = $self->{socket};
    my $sessionid = $self->{sessionid};

    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $request = pack('N',Net::TRMeister::Const->ErasePrepareStatement2);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype=pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$sessionid);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$prepareid);
    $socket->send($classtype,0);
    $socket->send($request,0);
    $socket->flush;

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) ==0)
    {
	return undef;
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);

    if(Net::TRMeister::Utility->check_status($self,$status)==0)
    {
	return undef;
    }
    1;
}

#####################################################
# Net::TRMeister::WorkerConnection::execute_preparestatement --
# args:
#	prepareid: the prepared plan to be executed
#	parameterarray: the parameters for prepare statement
# return:
#	1 success
sub execute_preparestatement
{
    my $self = shift;
    my $prepareid =shift;
    my $parameterarray = shift;

    my $classtype;
    my $request;
    my $socket = $self->{socket};

    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $request = pack('N',Net::TRMeister::Const->ExecutePrepare);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$self->{sessionid});
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $prepareid = pack('N',$prepareid);
    $socket->send($classtype,0);
    $socket->send($prepareid,0);

    $self->_write_dataarraydata($parameterarray);
    $socket->flush;
    1;
}

#
# Net::TRMeister::WorkerConnection::write_arraydata -- write data array data
#
sub _write_dataarraydata
{
    my $self = shift;
    my $parameterarray = shift;
    my $socket = $self->{socket};

    my $elementnum = $#{$parameterarray} + 1;
    my $classtype = pack('N', Net::TRMeister::Const->DataArrayDataClass);
    my $temp0 = pack('N', $elementnum);
    $socket->send($classtype, 0);
    $socket->send($temp0, 0);

    my $length;
    my $element;

    for (my $i = 0; $i < $elementnum; $i++) {
	$element = $parameterarray->[$i];
	if (!defined($element)) {
	    $classtype = pack('N', Net::TRMeister::Const->NullDataClass);
	    $socket->send($classtype);
	} elsif (ref($element) eq "ARRAY") {
	    $self->_write_dataarraydata($element);
	} else {
	    $classtype = pack('N', Net::TRMeister::Const->StringDataClass);
	    $socket->send($classtype, 0);
	    $length = from_to($element, $self->{userEncode}, $system_encoding);
	    $length = pack('N', $length / 2);
	    $socket->send($length, 0);
	    $socket->send($element, 0);
	}
    }
    1;
}

######################################################################################################
# Net::TRMeister::ResultSet-- Processing the result from server
# member:
# methods:
######################################################################################################
package Net::TRMeister::ResultSet;

use IO::Socket;
use strict;
use Carp;
use Encode qw/encode decode from_to/;
use Data::Dumper;

#####################################################
# Net::TRMeister::ResultSet::new -- constructor for Net::TRMeister::ResultSet
# args:
#	socket	socket instacne for result retrieve
#	userEncode		user's encoding
#	dumpLargeObject		1:dump the binary data    0: just print "size = **"
#	largeObjectBufferSize		if dump the binary data,how much memory is allocated for object
#	truncOverBufferSize		if the size of binary data exceeds the largeObjectBufferSize,should we trunc it (1)or not(0)
#	debug		whether output debug info (1) or not(0)
# return:
#	class instance,return undef if error
sub new
{
    my $class = shift;
    my %args  = @_;

    my $self = bless
    {
	'socket' => $args{socket},
	debug => $args{debug} ,
	dumpLargeObject => $args{dumpLargeObject},
	largeObjectBufferSize => $args{largeObjectBufferSize},
	truncOverBufferSize => $args{truncOverBufferSize},
	is_exception => 0,  #whether it is a exceptioin
	exception_number => 0,
	exception_message => 0,
	is_end_of_data => 0,  #whether it is end of data
	is_has_result =>0, #whether it has result
	is_success =>0, #whether the query result is successfully returned
	info => undef,
	column_num =>0,  #number of field
	column_names =>undef, #each field name
	columninfo_array =>undef,
	userEncode => $args{userEncode},
	is_bigint_support => 0,
	is_dataarrydata_flag_readed => 0,
	is_byteorder_lh => 0,
	father => $args{father},

    },$class;

    if($self->{debug})
    {   print "\n****Net::TRMeister::Result****\n";
	print "\n[INFO] a new Result is created \n";
    }
    #whether support int64 in perl
    $self->{is_bigint_support} = Net::TRMeister::Utility->is_bigint_support();
    #byteorder check
    $self->{is_byteorder_lh} = Net::TRMeister::Utility->is_byteorder_lh();
    return $self;
}

#####################################################
# Net::TRMeister::ResultSet::_read_decimaldata -- read decimal data
# args:
#	none
# return:
#	decimal string
sub _read_decimaldata
{
    my $self = shift;
    my $socket = $self->{socket};

    my ($presion,$scale,$integer_part,$fraction_part,$flag,$unit_num,$digit_unit);
    my $datavalue="";

    $socket->recv($presion,4,0);
    $presion = unpack('N',$presion);

    $socket->recv($scale,4,0);
    $scale = unpack('N',$scale);

    $socket->recv($integer_part,4,0);
    $integer_part = unpack('N',$integer_part);

    $socket->recv($fraction_part,4,0);
    $fraction_part = unpack('N',$fraction_part);

    $socket->recv($flag,1,0);
    $flag = unpack('c',$flag);

    $socket->recv($unit_num,4,0);
    $unit_num = unpack('N',$unit_num);

    my $i=0;
    my $dot_position = int(($integer_part+Net::TRMeister::Const->DigitNumPerUnit-1)/Net::TRMeister::Const->DigitNumPerUnit);
    for ($i=0;$i<$unit_num;$i++)
    {
	$socket->recv($digit_unit,4,0);
	$digit_unit = unpack('N',$digit_unit);
        $digit_unit = sprintf("%09d", $digit_unit);

	if($i == $dot_position)
	{
	    $datavalue = $datavalue.".";
	}
	$datavalue = $datavalue. $digit_unit;
    }
    my $dot;

    $datavalue =~ s/^0*//g;
    $dot = index $datavalue,".";
    if($dot == -1)
    {
	if(length($datavalue) != 0)
	{
	    return $flag ==1 ?"-".$datavalue:$datavalue;
	}
	else
	{
	    return 0;
	}
    }
    elsif($dot ==0)
    {
	$datavalue = "0".$datavalue;
	$dot = 1;
    }
    else #($dot>0)
    {
	;
    }
    $datavalue = substr $datavalue,0,$dot+$fraction_part+1;

    if($flag == 1)
    {
	return "-".$datavalue;
    }
    else
    {
	return $datavalue;
    }
}
#####################################################
# Net::TRMeister::ResultSet::_read_dataarraydata -- read data array data
# args:
#	none
# return:
#	dataarraydata string
sub _read_dataarraydata
{
    my $self =shift;
    my @tuple;
    my $socket = $self->{socket};

    my $elementnum;

    $socket->recv($elementnum,4,0);
    $elementnum = unpack('N',$elementnum);
    for (my $i = 0; $i < $elementnum; $i++)
    {
	my $dataclass;

	$socket->recv($dataclass,4,0);
	$dataclass = unpack('N',$dataclass);

	if ($dataclass == Net::TRMeister::Const->IntegerDataClass)
	{
	    push @tuple,$self->_read_intergerdata;
	}
	elsif ($dataclass == Net::TRMeister::Const->UnsignedIntegerDataClass )
	{
	    push @tuple,$self->_read_unsignedintergerdata;
	}
	elsif ($dataclass == Net::TRMeister::Const->Integer64DataClass)
	{
	    push @tuple,$self->_readinteger64data;
	}
	elsif ($dataclass == Net::TRMeister::Const->UnsignedInteger64DataClass)
	{
	    $self->{is_exception} = 1;
	    $self->{excetption_message} = "\n[Not Support]UnsignedInteger64 is not support\n";
	    die $self->{excetption_message} ;
	}
	elsif ($dataclass == Net::TRMeister::Const->FloatDataClass)
	{
	    $self->{is_exception} = 1;
	    $self->{excetption_message} = "\n[Not Support]FloatData is not support\n";
	    die $self->{excetption_message} ;
	}
	elsif ($dataclass == Net::TRMeister::Const->DoubleDataClass)
	{
	    push @tuple,$self->_read_doubledata;
	}
	elsif ($dataclass == Net::TRMeister::Const->DecimalDataClass)
	{
	    push @tuple,$self->_read_decimaldata;
	}
	elsif ($dataclass == Net::TRMeister::Const->DateDataClass)
	{
	    $self->{is_exception} = 1;
	    $self->{excetption_message} = "\n[Not Support]DateData is not support\n";
	    die $self->{excetption_message};

	}
	elsif ($dataclass == Net::TRMeister::Const->DateTimeDataClass)
	{
	    push @tuple,$self->_read_datetimedata;
	}
	elsif ($dataclass == Net::TRMeister::Const->NullDataClass)
	{
	    #push @tuple, "(null)";
	    push @tuple, undef;
	}
	elsif ($dataclass == Net::TRMeister::Const->StringDataClass)
	{
	    push @tuple, $self->_read_stringdata;
	}
	elsif ($dataclass == Net::TRMeister::Const->BinaryDataClass)
	{
	    push @tuple, $self->_read_binarydata;
	}
	elsif (($dataclass == Net::TRMeister::Const->IntegerArrayDataClass) or($dataclass == Net::TRMeister::Const->UnsignedIntegerArrayDataClass) or ($dataclass == Net::TRMeister::Const->StringArrayDataClass))
	{
	    $self->{is_exception} = 1;
	    $self->{excetption_message} = "\n[Not Support]IntergerArray,UnsignedIntegerArray are not support\n";
	    die $self->{excetption_message};
	}
	elsif ($dataclass == Net::TRMeister::Const->DataArrayDataClass)
	{
	    push @tuple, $self->_read_dataarraydata;
	}
	elsif ($dataclass == Net::TRMeister::Const->LanguageDataClass)
	{
	    push @tuple, $self->_read_languagedata;
	}
	elsif ($dataclass == Net::TRMeister::Const->WordDataClass)
	{
	    push @tuple, $self->_read_worddata;
	}
	else
	{
	    $self->{is_exception} = 1;
	    $self->{excetption_message} = "\n[Not Support]DataClass ".$dataclass." not supported by TRMeister DBMS\n";
	    die $self->{excetption_message} ;
	}
    }
    \@tuple;
}
#####################################################
# Net::TRMeister::ResultSet::_read_intergerdata -- read integer data
# args:
#	none
# return:
#	integer data string
sub _read_intergerdata
{
    my $self = shift;
    my $socket = $self->{socket};
    my $datavalue;
    my @array;

    $socket->recv($datavalue,4,0) ;
    @array = unpack('C4',$datavalue);
    if ($self->{is_byteorder_lh} == 1)
    {
	@array = reverse @array;
    }
    $datavalue = pack('C4',@array);
    $datavalue = unpack('i',$datavalue);
    $datavalue;
}
#####################################################
# Net::TRMeister::ResultSet::_read_binarydata -- read binary data
# args:
#	none
# return:
#	(size = **) or binary data
sub _read_binarydata
{
    my $self = shift;
    my $socket = $self->{socket};
    my $datavalue;
    my $datalength;

    $socket->recv($datalength,4,0);
    $datalength = unpack('N',$datalength);

    #dumpLargeObject =>1:dump the binary data 0: just print "size = **"
    #largeObjectBufferSize => if dump the binary data,how many memory is allocated for object
    #truncOverBufferSize => if the size of binary data exceeds the largeObjectBufferSize,should we trunc it (1)or not(0)
    if($self->{dumpLargeObject} == 1)
    {
	if($datalength>=$self->{largeObjectBufferSize})
	{
	    if($self->{truncOverBufferSize} == 1)
	    {
		my $temp;
		$socket->recv($datavalue,$self->{largeObjectBufferSize},0);
		$socket->recv($temp,($datalength-$self->{largeObjectBufferSize}),0);
		return $datavalue;
	    }
	    else
	    {
		die "\n[ERROR]BinaryData is too large,truncate is set to false \n";
	    }
	}
	else
	{
	    $socket->recv($datavalue,$datalength,0);
	    return $datavalue;
	}
    }
    else
    {
	my $temp;
	$socket->recv($temp,$datalength,0);
	$datavalue = "size=$datalength";
	return $datavalue;
    }
}
#####################################################
# Net::TRMeister::ResultSet::_read_datetimedata -- read datetime data
# args:
#	none
# return:
#	datatime string
sub _read_datetimedata
{
    my $self = shift;
    my $datavalue="";
    my $socket = $self->{socket};

    my ($year,$month,$date,$hour,$minute,$second,$millisecond,$precision);

    $socket->recv($year,4,0);
    $year = unpack('N',$year);
    $socket->recv($month,4,0);
    $month = unpack('N',$month);
    $socket->recv($date,4,0);
    $date = unpack('N',$date);
    $socket->recv($hour,4,0);
    $hour = unpack('N',$hour);
    $socket->recv($minute,4,0);
    $minute = unpack('N',$minute);
    $socket->recv($second,4,0);
    $second = unpack('N',$second);
    $socket->recv($millisecond,4,0);
    $millisecond = unpack('N',$millisecond);
    $socket->recv($precision,4,0);
    $precision = unpack('N',$precision);
    #{1980-02-12 00:00:00.000}  $preceision is used to determine the leading zero for millisecond
    $datavalue = sprintf("%04d",$year)."-".sprintf("%02d",$month)."-".sprintf("%02d",$date)." ".sprintf("%02d",$hour).":".sprintf("%02d",$minute).":".sprintf("%02d",$second).".".sprintf("%0".$precision."d",$millisecond);
}
#####################################################
# Net::TRMeister::ResultSet::_readinteger64data -- read interger64 data
# args:
#	none
# return:
#	interger64 string
sub _readinteger64data
{
    my $self = shift;
    my $datavalue;
    my $socket = $self->{socket};

    $socket->recv($datavalue,8,0);
    my @array = unpack('C8',$datavalue);
    if ($self->{is_byteorder_lh} == 1)
    {
	@array = reverse(@array);
    }

    if($self->{is_bigint_support} == 1)
    {
	my $str = pack('C8',@array);
	$datavalue = unpack('q',$str);
    }
    else
    {
	$datavalue = "integer64 is not supported in current Perl";
	warn "\ninteger64 is not supported in current Perl\n";
    }
    $datavalue;
}
#####################################################
# Net::TRMeister::ResultSet::_read_doubledata -- read double data
# args:
#	none
# return:
#	doubledata string
sub _read_doubledata
{
    my $self = shift;
    my $datavalue;
    my $socket = $self->{socket};

    $socket->recv($datavalue,8,0);

    my @array = unpack('C8',$datavalue);
    if ($self->{is_byteorder_lh} == 1)
    {
	@array = reverse(@array);
    }
    my $str = pack('C8',@array);
    $datavalue = unpack('d',$str);

    $datavalue;
}
#####################################################
# Net::TRMeister::ResultSet::_read_unsignedintergerdata -- read unsigned integer data
# args:
#	none
# return:
#	unsigned integer data string
sub _read_unsignedintergerdata
{
    my $self = shift;
    my $datavalue;
    my $socket = $self->{socket};

    $socket->recv($datavalue,4,0);
    $datavalue = unpack('N',$datavalue);
}
#####################################################
# Net::TRMeister::ResultSet::_read_stringdata -- read string data
# args:
#	none
# return:
#	string data
sub _read_stringdata
{
    my $self = shift;
    my $socket = $self->{socket};
    my $datavalue;
    my $datalength;

    $socket->recv($datalength,4,0);
    $datalength = unpack('N',$datalength);
    $socket->recv($datavalue,2*$datalength,0);
    if($datalength == 0)
    {
	$datavalue ='';
    }
    else
    {
	from_to($datavalue,$system_encoding,$self->{userEncode});
    }
    $datavalue;
}
#####################################################
# Net::TRMeister::ResultSet::_read_languagedata -- read language data
# args:
#	none
# return:
#	language string
sub _read_languagedata
{
    my $self = shift;
    my $socket = $self->{socket};
    my $elementnum;
    my $datavalue = '';

    $socket->recv($elementnum,4,0);
    $elementnum = unpack('N',$elementnum);

    for (my $i = 0; $i < $elementnum; $i++)
    {
	my $langCode;
	my $countryCode;

	$socket->recv($langCode,2,0);
	$langCode = unpack('n',$langCode);

	$socket->recv($countryCode,2,0);
	$countryCode = unpack('n',$countryCode);

	if ($i != 0)
	{
	    $datavalue = $datavalue . '+';
	}
	$datavalue = $datavalue . $lang_code[$langCode];
	if ($countryCode != 0)
	{
	    $datavalue = $datavalue . '-' . $country_code[$countryCode];
	}
    }

    $datavalue;
}
#####################################################
# Net::TRMeister::ResultSet::_read_worddata -- read word data
# args:
#	none
# return:
#	hash
sub _read_worddata
{
    my $self = shift;
    my $socket = $self->{socket};
    my @category_str = (
	"",
	"Essential",
	"Important",
	"Helpful",
	"EssentialRelated",
	"ImportantRelated",
	"HelpfulRelated",
	"Prohibitive",
	"ProhibitiveRelated"
	);
    my $term;
    my $language;
    my $category;
    my $scale;
    my $df;
    my $result = {};

    $term = $self->_read_stringdata;
    $language = $self->_read_languagedata;

    $socket->recv($category,4,0);
    $category = unpack('N',$category);

    $scale = $self->_read_doubledata;

    $socket->recv($df,4,0);
    $df = unpack('N',$df);

    $result->{term} = $term;
    $result->{language} = $language;
    $result->{category} = $category_str[$category];
    $result->{scale} = $scale;
    $result->{df} = $df;

    $result;
}

sub get_column_num  #return the column num for the result
{
    my $self = shift;

    return $self->{column_num};
}
#return the column num for the result
sub get_column_names
{
    my $self = shift;

    return $self->{column_names};
}

sub get_exception_number
{
    my $self = shift;

    return $self->{exception_number};
}
sub get_exception_message
{
    my $self = shift;

    return $self->{exception_message};
}

sub is_error
{
    my $self = shift;

    return ($self->{is_exception}==1);
}
#####################################################
# Net::TRMeister::ResultSet::check_first_object -- the first object from socket shows whether there is a error or not
# args:
#	none
# return:
#	1 or die for unexpected
sub check_first_object
{
    my $self = shift;
    my $socket = $self->{socket};

    my $classtype;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);

    if(Net::TRMeister::Utility->check_classtype($self,$classtype)==0)
    {
	$self->{is_exception} = 1;
	$self->{is_success} = 1; #we successfully finish process result
	$self->{is_has_data} = 0;
	$self->{is_end_of_data} = 1;
	return undef;
    }

    if($classtype == Net::TRMeister::Const->StatusClass )
    {
	my $statusvalue;

	$socket->recv($statusvalue,4,0);
	$statusvalue = unpack('N',$statusvalue);

	if($statusvalue == Net::TRMeister::Const->Success)
	{
	    $self->{info} = "\nSuccess\n";
	    $self->{is_success} = 1; #we successfully finish process result
	    $self->{is_has_data} = 0;
	    $self->{is_end_of_data} = 1;
	}
	elsif($statusvalue == Net::TRMeister::Const->Canceled)
	{
	    $self->{info} = "\nThe result is cancelled\n";
	    $self->{is_success} = 1; #we successfully finish process result
	    $self->{is_has_data} = 0;
	    $self->{is_end_of_data} = 1;
	}
	elsif($statusvalue == Net::TRMeister::Const->HasMoreData)
	{
	    $self->{info} = "\nThere are still more result\n";
	    $self->{is_success} = 0; #we successfully finish process result
	    $self->{is_has_data} = 1;
	    $self->{is_end_of_data} = 0;
	}
	else
	{
	    $self->{is_exception} = 1;
	    $self->{exception_message} ="\nUnknown Error.The stauts is $statusvalue\n";
	    $self->{info} = "\nUnknown Error........\n";

	    $self->{is_success} = 1;
	    $self->{is_has_data} = 0;
	    $self->{is_end_of_data} = 1;
	}
    }
    elsif($classtype == Net::TRMeister::Const->ResultSetMetaDataClass)
    {
	my $columnnum;
	my ($type,$count,$length,$string,$integer);
	my @columninfo_array;
	$socket->recv($columnnum,4,0);
	$columnnum = unpack('N',$columnnum);

	for(my $i=0;$i<$columnnum;$i++)
	{
	    $socket->recv($type,4,0);
	    $type = unpack('N',$type);
	    push @{$columninfo_array[$i]},$type;

	    $socket->recv($count,4,0);
	    $count = unpack('N',$count);

	    my $actualcount = Net::TRMeister::Const->StringValueNum-$count;

	    while($count)
	    {
		$socket->recv($length,4,0);
		$length = unpack('N',$length);
		$socket->recv($string,2* $length,0);
		from_to($string,$system_encoding,$self->{userEncode});

		push @{$columninfo_array[$i]},$string;

		$count--;
	    }
	    while($actualcount)
	    {
		push @{$columninfo_array[$i]},undef;
		$actualcount--;
	    }

	    $socket->recv($count,4,0);
	    $count = unpack('N',$count);

	    $actualcount = Net::TRMeister::Const->IntegerValueNum-$count;

	    while($count)
	    {
		$socket->recv($integer,4,0);
		$integer = unpack('N',$integer);

		push @{$columninfo_array[$i]},$integer;

		$count--;
	    }
	    while($actualcount)
	    {
		push @{$columninfo_array[$i]},undef;
		$actualcount--;
	    }

	    $socket->recv($integer,4,0);
	    $integer = unpack('N',$integer);
	    push @{$columninfo_array[$i]},$integer;
	}
	$self->{columninfo_array} = \@columninfo_array;
	if($self->{debug})
	{
	    print "\n[INFO]the ColumnMetadata for columns are\n";
	    print Dumper(@{$self->{columninfo_array}} );
	}
	$self->{column_num} = $columnnum;
	my @namearray;
	for(my $j=0;$j<$columnnum;$j++)
	{
	    if($columninfo_array[$j][2] eq '')
	    {
		push @namearray,$columninfo_array[$j][5];
	    }
	    else
	    {
		push @namearray,$columninfo_array[$j][2];
	    }
	    #push @namearray,(($columninfo_array[$j][2] eq '')? $columninfo_array[$j][2] : $columninfo_array[$j][5]);
	}
	$self->{column_names} = \@namearray;
	if($self->{debug})
	{
	    print "\n[INFO]the Column's names are\n";
	    print Dumper(@{$self->{column_names}} );
	}
	$self->{info} = "the ResultMetadata is successfully read\n";
	$self->{is_success} = 1; #we successfully finish process result

	$self->{is_has_data} = 1;
	$self->{is_end_of_data} = 0;

    }
    elsif($classtype == Net::TRMeister::Const->DataArrayDataClass)
    {
	$self->{is_dataarrydata_flag_readed} = 1;
	$self->{info} = "This operation has no header\n";
	$self->{is_success} = 1; #we successfully finish process result

	$self->{is_has_data} = 1;
	$self->{is_end_of_data} = 0;
	#may be no column name but has result.e.g. back up command,
	#regard its column_num is 1
	$self->{column_num}  = 1;
    }
    else
    {
	$self->{is_exception} = 1;
	$self->{exception_message} = $self->{info} = "Unknown case in check_first_object\n";
	$self->{is_success} = 0; #we failed to finish process result
	$self->{is_has_data} = 0;
	$self->{is_end_of_data} = 1;
    }
    1;
}
#####################################################
# Net::TRMeister::ResultSet::each -- return next tuple from resultset
# args:
#	none
# return:
#	tuple  next tuple
#	undef	end of result
#	die for unexpected error
sub each
{
    my $self = shift;
    my $socket = $self->{socket};

    if( ($self->{is_success}==0) or (($self->{is_success}==1) and ($self->{is_has_data}==0)))
    {
	return undef;
    }
    my $classtype;
    if($self->{is_dataarrydata_flag_readed} == 1)
    {
	$classtype = Net::TRMeister::Const->DataArrayDataClass;
	$self->{is_dataarrydata_flag_readed} = 0;
    }
    else
    {
	$socket->recv($classtype,4,0);
	$classtype = unpack('N',$classtype);

    }

    if(Net::TRMeister::Utility->check_classtype($self,$classtype)==0)
    {
	$self->{is_exception} = 1;
	$self->{is_end_of_data} = 1;
	$self->{is_has_result} = 0;
	$self->{is_has_data} = 0;
	return undef;
    }

    my $status;
    if($classtype == Net::TRMeister::Const->None)
    {
	$socket->recv($classtype,4,0);
	$classtype = unpack('N',$classtype);

	$socket->recv($status,4,0);
	$status = unpack('N',$status);
	if($status == Net::TRMeister::Const->Success or $status == Net::TRMeister::Const->Canceled)
	{
	    $self->{is_end_of_data} = 1;
	    $self->{is_has_result} = 0;
	    $self->{is_has_data} = 0;
	    return undef;
	}
	else
	{
	    die "[Error]Unknow error in each() function\n";
	}
    }
    elsif($classtype == Net::TRMeister::Const->DataArrayDataClass)
    {
	my $tuple = $self->_read_dataarraydata;
	if($self->{debug})
	{
	    print "\n the tuple values are\n";
	    print Dumper(@{$tuple});
	}

	$self->{is_end_of_data} = 0;
	$self->{is_has_result} = 1;

	return $tuple;
    }
    else
    {
	die "\n[ERROR]Unknown error in each function\n";
    }
}
sub is_end_of_result
{
    my $self  = shift;

    $self->{is_end_of_data};
}
sub set_end_of_result
{
    my $self = shift;

    $self->{is_end_of_data} = 1;
}
sub has_result
{
    my $self =shift;

    $self->{is_has_data};
}

######################################################################################################
# Net::TRMeister::ServerConnection-- Connection to server
# member:
# methods:
######################################################################################################
package Net::TRMeister::ServerConnection;

use IO::Socket;
use strict;
use Carp;
use Encode qw/encode decode from_to/;

#####################################################
# Net::TRMeister::ServerConnection::new -- constructor
# args:
#	hostname		hostname for server,"localhost" as default
#	port		port number,54321 as default
#	database		database name, "DefaultDB" as default
#	timeout
#	masterid		the protocol version
#	debug		whether output debug info (1) or not(0)
#	userEncode		user's encoding
# return:
#	class instance
sub new
{
    my $class = shift;
    my %args  = @_;

    my $self = bless
    {
        hostname => $args{hostname}	||"localhost",
        port     => $args{port}	||54321,
        database => $args{database},
        timeout  => $args{timeout}  || 60,
        'socket' => undef,
        masterid => $args{masterid} ||(Net::TRMeister::Const->CurrentVersion | Net::TRMeister::Const->PasswordMask),
        slaveid  => undef,
        debug => $args{debug},
        father => $args{father},
        userEncode => $args{userEncode},
    },$class;

    if($self->{debug})
    {   print "\n****Net::TRMeister::Connection****\n";
        print "\n[INFO] a new connection is created \n";
    }
    return $self;
}
#####################################################
# Net::TRMeister::ServerConnection::connect -- connect to server
# args:
#	none
# return:
#	Net::TRMeister::Socket instance
sub connect
{
    my $self = shift;
    my $sydney;

    my $trm =Net::TRMeister::Socket->new
	(
		 hostname =>  $self->{hostname} ||"localhost",
		 port     => $self->{port} ||54321,
		 timeout  =>  $self->{timeout} || 60,
		 'socket' => undef,
		 debug => $self->{debug},
	)or die "\n [FAIL]:Couldn't connect \n";

    $self->{socket} = $trm;
    if($self->{debug})
    {
	print "\n[INFO]success connected\n";
    }
    $trm;
}
#####################################################
# Net::TRMeister::ServerConnection::get_masterid_slaveid --
# args:
#	none
# return:
#	1
sub get_masterid_slaveid
{
    my $self = shift;
    my $socketname = shift;

    my $socket = $self->{socket};
    if($socketname eq "ServerSocket")
    {
	$self->{slaveid} = Net::TRMeister::Const->SlaveIDAny;
    }
    my $required_masterid = $self->{masterid};  # for debug
    my $message0 = pack('N',$self->{masterid});
    my $message1 = pack('N',$self->{slaveid});
    $socket->send($message0, 0);
    $socket->send($message1, 0);
    $socket->flush;
    my $result0;
    my $result1;
    $socket->recv($result0,4,0);
    $socket->recv($result1,4,0);
    $result0 = unpack('N',$result0);
    $result1 = unpack('N',$result1);

    $self->{masterid} = $result0;
    $self->{slaveid} = $result1;
    if($self->{debug})
    {
        print "\n****Net::TRMeister::ServerConnection::get_masterid_slaveid****\n";
        print "[INFO]MasterId = ".$self->{masterid}." SlaveID = ".$self->{slaveid}."\n";
        print "[INFO] required MasterId : " . sprintf( "0x%x", $required_masterid) . "\n";
        print "[INFO] response MasterId : " . sprintf( "0x%x", $self->{masterid} ) . "\n";
    }

    1;
}
#####################################################
# Net::TRMeister::ServerConnection::begin_connection -- connect to server
# args:
#	none
# return:
#	1 success
#	undef fail
sub begin_connection
{
    my $self = shift;
    my $socket = $self->{socket};

    my $classtype;
    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    my $request;
    $request = pack('N',Net::TRMeister::Const->BeginConnection);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype=pack('N',Net::TRMeister::Const->StringDataClass);
    my $length = pack ('N',length($self->{hostname}));
    my $hostname;
    $hostname = encode($system_encoding,$self->{hostname});
    if($self->{debug})
    {
        print "\n***Net::TRMeister::ServerConnection::begin_connection***\n";
        print "[INFO]the host's name is $hostname and its length is ".length($hostname)."\n";
    }

    $socket->send($classtype,0);
    $socket->send($length,0);
    $socket->send($hostname,0);
    $socket->flush;

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) == 0)
    {
        if( $self->{debug} ){
            print "error ClassType : $classtype ", $self->{father}->{errstr}, "\n";
            die;
        }
        return undef;
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);
    if(Net::TRMeister::Utility->check_status($self,$status) == 0)
    {
        if($self->{debug})
        {
            if($status == Net::TRMeister::Const->Success && $classtype == Net::TRMeister::Const->StatusClass )
            {
                print "\n[INFO]the status for begin connection is success\n";
            }
            else
            {
                print "\n[INFO]the status for begin connection is $status\n";
            }
        }
        return undef;
    }
    1;
}
#####################################################
# Net::TRMeister::ServerConnection::begin_worker -- begin a wroker
# args:
#	none
# return:
#	workerid and slaveid  ,failed both return undef
sub begin_worker
{
    my $self = shift;
    my $slaveid = shift;

    my $socket = $self->{socket};

    my $classtype;
    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    my $request;
    $request = pack('N',Net::TRMeister::Const->BeginWorker);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$slaveid);
    $socket->send($classtype,0);
    $socket->send($request,0);
    $socket->flush;

    my($slaveid1,$workerid);

    #read SlaveID
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) == 0)
    {
        return (undef,undef);
    }

    $socket->recv($slaveid1,4,0);
    $slaveid1 = unpack('N',$slaveid1);
    #read WorkerID
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) == 0)
    {
        return (undef,undef);
    }

    $socket->recv($workerid,4,0);
    $workerid = unpack('N',$workerid);

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) == 0)
    {
        return (undef,undef);
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);
    if(Net::TRMeister::Utility->check_status($self,$status)==0)
    {
        return (undef,undef);
    }

    if($self->{debug})
    {
        if($status == Net::TRMeister::Const->Success && $classtype == Net::TRMeister::Const->StatusClass )
        {
            print "\n[INFO]Beginworker success\n";
        }
        else
        {
            print "\n[INFO]Beginworker fails and its status is $status\n";
        }
    }

    ($slaveid1,$workerid);
}
#####################################################
# Net::TRMeister::ServerConnection::cancel_worker --
# args:
#	none
# return:
#	1 success
#	undef fail
sub cancel_worker
{
    my $self = shift;
    my $workerid = shift;

    my $socket = $self->{socket};

    my $classtype;
    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    my $request;
    $request = pack('N',Net::TRMeister::Const->CancelWorker);
    $socket->send($classtype,0);
    $socket->send($request,0);

    $classtype= pack('N',Net::TRMeister::Const->IntegerDataClass);
    $request = pack('N',$workerid);
    $socket->send($classtype,0);
    $socket->send($request,0);
    $socket->flush;

    my $status;
    $socket->recv($classtype,4,0);
    $classtype = unpack('N',$classtype);
    if(Net::TRMeister::Utility->check_classtype($self,$classtype) ==0)
    {
	return undef;
    }
    $socket->recv($status,4,0);
    $status = unpack('N',$status);

    if($self->{debug})
    {
	print "\nThe worker is canclled and the status is $status\n";
    }
    1;
}
#####################################################
# Net::TRMeister::ServerConnection::close_server_connection --
# args:
#	none
# return:
#	1 success
#	undef fail
sub close_server_connection
{
    my $self = shift;

    my $classtype;
    my $request;
    my $socket = $self->{socket};

    $classtype= pack('N',Net::TRMeister::Const->RequestClass);
    $request = pack('N',Net::TRMeister::Const->EndConnection);
    $socket->send($classtype,0);
    $socket->send($request,0);
    $socket->flush;

    my $status;
    $socket->recv($classtype,4,0);
    $socket->recv($status,4,0);
    $classtype = unpack('N',$classtype);
    $status = unpack('N',$status);

    $self->{slaveid} = undef;
    $socket->close or croak "\n[FAIL:]connect close failed\n";
    $self->{socket} = undef;

    if($self->{debug})
    {
	print "\n***Net::TRMeister::Connection::close_server_connection***\n";
	print "\n[INFO]the server connection is closed\n";
    }
    1;
}

#wappered IO::Socket::INET
package Net::TRMeister::Socket;

use IO::Socket;
use strict;
use Carp;

use constant FlushSize => 65536;

sub new
{
    my $class = shift;
    my %args  = @_;

    my $self = bless
    {
        hostname => $args{hostname}	||"localhost",
        port     => $args{port}	||54321,
        database => $args{database},
        timeout  => $args{timeout}  || 60,
        'socket' => undef,
        debug => $args{debug},
        buf => '',
        buflen => 0,
    },$class;
    $self->connect;
    if($self->{debug})
    {   print "\n****Net::TRMeister::Connection****\n";
        print "\n[INFO] a new connection is created \n";
    }
    return $self;
}

sub connect
{
    my $self = shift;
    my $trm = IO::Socket::INET->new(
        PeerAddr => $self->{hostname} ||"localhost",
        PeerPort => $self->{port} ||54321,
        Proto    => 'tcp',
        Timeout  => $self->{timeout} || 60,
    ) or die "\n [FAIL]:Couldn't connect \n";

    $trm->autoflush(0);
    $self->{socket} = $trm;
    if($self->{debug})
    {
        print "\n[INFO]success connected\n";
    }
    $trm;
}

sub send
{
    my $self = shift;
    my $data =shift;
    my $option = shift;
    my $length = length($data);
    $self->{buf} = $self->{buf} . $data;
    $self->{buflen} = $self->{buflen} + $length;
#    if ($self->{buflen} > 65536)
    if ($self->{buflen} > FlushSize)    {
        # > 64KB
        $self->flush;
    }
    $length;
}

sub recv
{
    my $self = $_[0];
    # $_[1] : data reference
    my $length =$_[2];
    my $option = $_[3];
    my $socket = $self->{'socket'};

    if( defined $self->{crypt} ){
        while( 1 ){
#            print "rbuf size : ", length( $self->{rbuf} ), "\n";
            last if( $length <= length( $self->{rbuf} ) );

            my $raw = undef;
            while( 1 ){
#                print "recv!\n";
                my $received = undef;
                $socket->recv( $received, FlushSize, $option );
                my $recieved_size = length $received;
                $raw .= $received;
#                print "recieved size : ", $recieved_size, "\n";

                last if( $recieved_size < FlushSize );
            }
            die "incomplete block : " . length( $raw ) . " " . ( length( $raw ) % $self->{crypt}->blocksize ) . "\n" if( length( $raw ) % $self->{crypt}->blocksize );

            if( 0 < length( $raw ) ){
                my $start = 0;
                while( $start < length( $raw ) ){
                    my $block = substr( $raw, $start, $self->{crypt}->blocksize );
                    $self->{rbuf} .= $self->{crypt}->decrypt( $block );
                    $start += length $block;
                }
            }
        }

        $_[1] = substr( $self->{rbuf}, 0, $length, undef );
    }else{
        my $count = 0 ;
        my $sum = $length;
        my $received = 0;
        my $substring;
        $_[1] = '';
        while($count!=$length)
        {
#            print "recv : no encrypt\n";
            $socket->recv($substring,$sum,$option);
            if (length($substring) == 0)
            {
                die "\n[Error] socket recv error\n";
            }
            $_[1] = $_[1] . $substring;
            $received = length($substring);
            $count = $count + $received;
            $sum = $sum - $received;
        }
    }
    1;
}

sub flush
{
    my $self = shift;
    my $socket = $self->{'socket'};

    my $data = $self->{buf};
    my $length = $self->{buflen};
    if( defined $self->{crypt} ){
#        print "flush : crypt\n";
#        print "plaintext size : $length\n";

#print "Common Key : ", $self->{crypt}->{key}, "\n";
#print "plaintext :\n", unpack( 'H*', $data ), "\n";
        my $encrypted;
        my $start = 0;
        while( $start < length( $data ) ){
            my $block = substr( $data, $start, $self->{crypt}->blocksize - 1 );
            $encrypted .= $self->{crypt}->encrypt( $block );
            $start += length $block;
        }
        $data = $encrypted;
        $length = length $data;
#        print "encrypted size : $length\n";
    }
#    print "flush : $length\n";

    my $count = $length;
    my $sum = 0;
    my $sent = 0;
    while ($count != 0)
    {
        $sent = $socket->send(substr($data, $sum, $count), 0)
            or die "\n[Error] socket send error\n";
        $count = $count - $sent;
        $sum = $sum + $sent;
    }
    $socket->flush() or die "\n[Error] socket flush error\n";
    $self->{buf} = undef;
    $self->{buflen} = 0;
    1;
}

sub close
{
    my $self = shift;
    my $socket = $self->{socket};

    $socket->close() or die "\n[Error] socket close error\n";
    1;
}

sub generate_common_key
{
    my $str = String::Random->new->randregex('.{16}');
#    my $str = '1234567890abcdef';
#    print "Common Key : $str\n";

    return $str;
}

######################################################################################################
# Net::TRMeister::Utility-- Some utility tools
# member:
# methods:
######################################################################################################
package Net::TRMeister::Utility;
use Encode qw/encode decode from_to/;
use Config;
#####################################################
# Net::TRMeister::Utility::check_classtype -- check the received class type
# args:
#	$self  the connection
#	$classtype the classtype to be checked
# return:
#	1 valid classtype
#	0 invalid classtype
sub check_classtype
{
    my $utility = shift;
    my $self = shift;
    my $classtype = shift;
    if($classtype == Net::TRMeister::Const->ExceptionClass)
    {
        $self->{father}->{errnum} = 1;
        $self->{father}->{errstr} = "Exception";
        return 0;
    }
    elsif($classtype == Net::TRMeister::Const->ErrorLevelClass)
    {
        my $socket = $self->{socket};
        my ($errorlevel,$errornumber,$argnum,$argstring,$linenumber);
        my @xmlstring;
        $argstring = "Exception:: ( ";

        my ($classtype,$length,$string);
        $socket->recv($errorlevel,4,0);
        $errorlevel = unpack('N',$errorlevel);
        $argstring = $argstring." ErrorLevel is $errorlevel ";

        $socket->recv($classtype,4,0);
        $classtype = unpack('N',$classtype);
        #check here for exception calss [need]
        $socket->recv($errornumber,4,0);
        $errornumber = unpack('N',$errornumber);
        $self->{exception_number} = $errornumber;

        $socket->recv($argnum,4,0);
        $argnum = unpack('N',$argnum);
        for(my $i=0;$i<$argnum;$i++)
        {
            $socket->recv($length,4,0);
            $length = unpack('N',$length);
            $socket->recv($string,2* $length,0);
            from_to($string,$system_encoding,$self->{userEncode});
            push @xmlstring,$string;
            $argstring = $argstring.$string."  ";
        }
        $argstring = $argstring." ) in Module: ";

        $socket->recv($length,4,0);
        $length = unpack('N',$length);
        $socket->recv($string,2* $length,0);

        from_to($string,$system_encoding,$self->{userEncode});
        $argstring = $argstring." $string File: ";

        $socket->recv($length,4,0);
        $length = unpack('N',$length);
        $socket->recv($string,2* $length,0);
        from_to($string,$system_encoding,$self->{userEncode});
        $argstring = $argstring." $string Line: ";

        $socket->recv($linenumber,4,0);
        $linenumber = unpack('N',$linenumber);
        $argstring = $argstring." $linenumber \n ";
        my $e = $ErrorDef->{$errornumber};
        Encode::from_to($e, "utf8", $encoding);

        for(my $j=0;$j<$argnum;$j++)
        {
            $e=~ s/\%[\d]+/$xmlstring[$j]/;
        }
        $self->{father}->{errnum} = $errornumber;
        $self->{father}->{errstr} = $e;
        $self->{info} = $argstring;
        if($self->{debug})
        {
            print "\n [Details] $argstring \n";
        }
        return 0;
    }
    else
    {
        return 1;
    }
}
#####################################################
# Net::TRMeister::Utility::check_status -- check the received status
# args:
#	$self  the connection
#	$classtype the status to be checked
# return:
#	1 valid status
#	0 invalid status
sub check_status
{
    my $utility = shift;
    my $self = shift;
    my $status =shift;

    if($status == Net::TRMeister::Const->Success)
    {
        return 1;
    }
    else
    {
        $self->{father}->{errnum} = 1;
        $self->{father}->{errstr} = "Status Error\n";
        return 0;
    }
}
#####################################################
# Net::TRMeister::Utility::is_bigint_support -- if current perl support bigint,64bit int
# args:
#	none
# return:
#	1 support
#	0 not support
sub is_bigint_support
{
    my $utility = shift;
    if($Config{use64bitint} eq 'define' || $Config{longsize} >= 8)
    {
	return 1;
    }
    else
    {
	return 0;
    }
}
#####################################################
# Net::TRMeister::Utility::is_byteorder_lh -- if current perl byteoder is LH
# args:
#	none
# return:
#	1 LH
#	0 HL
sub is_byteorder_lh
{
    my $value = pack('C4', 0, 0, 0, 1);
    $value = unpack('i', $value);
    if ($value == 1) {
	return 0;
    } else {
	return 1;
    }
}
######################################################################################################
# Net::TRMeister::Const- Some constant values
# member:
# methods:
######################################################################################################
package Net::TRMeister::Const;

#for net protocol version
use constant CurrentVersion =>3;
use constant PasswordMask => 0x01000000;

#const for ConnectionSlaveID
use constant SlaveIDMinimun=>0;
use constant SlaveIDMaximum=>0x7fffffff;
use constant SlaveIDAny=>0x80000000;
use constant SlaveIDUndefined=>0xffffffff;

#use for datatype process
use constant None  => 0;
use constant StatusClass	=> 1;
use constant IntegerDataClass => 2;
use constant UnsignedIntegerDataClass => 3;
use constant Integer64DataClass => 4 ;
use constant UnsignedInteger64DataClass =>5;
use constant FloatDataClass=>6;
use constant DoubleDataClass=>7;
use constant DecimalDataClass=>8;
use constant StringDataClass=>9;
use constant DateDataClass=>10;
use constant DateTimeDataClass=>11;
use constant IntegerArrayDataClass=>12;
use constant UnsignedIntegerArrayDataClass=>13;
use constant StringArrayDataClass=>14;
use constant DataArrayDataClass=>15;
use constant BinaryDataClass=>16;
use constant NullDataClass=>17;
use constant ExceptionClass=>18;
use constant ParameterClass=>19;
use constant BitSetClass=>20;
use constant CompressedStringDataClass=>21;
use constant CompressedBinaryDataClass=>22;
use constant ObjectIDDataClass=>23;
use constant RequestClass=>24;
use constant LanguageDataClass=>25;
use constant SQLDataClass=>26;
use constant ColumnMetaDataClass=>27;
use constant ResultSetMetaDataClass=>28;
use constant WordDataClass=>29;
use constant ErrorLevelClass=>30;
use constant DefaultDataClass=>31;

#cmd for communication
use constant BeginConnection => 1;
use constant EndConnection => 2;
use constant BeginSession =>3;
use constant BeginSession2 =>17;
use constant EndSession=>4;
use constant BeginWorker=>5;
use constant CancelWorker=>6;
use constant Shutdown=>7;
use constant ExecuteStatement=>8;
use constant PrepareStatement=>9;
use constant ExecutePrepare=>10;
use constant ErasePrepareStatement=>11;
use constant ReuseConnection=>12;
use constant NoReuseConnection=>13;
use constant CheckAvailability=>14;
use constant PrepareStatement2=>15;
use constant ErasePrepareStatement2=>16;
use constant Sync => 101;

#for common::status
use constant Success =>0;
use constant Error=>1;
use constant Canceled =>2;
use constant HasMoreData =>3;

#for decimal
use constant DigitNumPerUnit =>9;

#for ResultSetMetaData's ColumnMetaData
use constant StringValueNum =>6;
use constant IntegerValueNum => 4;
1;

#
# Copyright (c) 2001, 2023 Ricoh Company, Ltd.
# All rights reserved.
#
