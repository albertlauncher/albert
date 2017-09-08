// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <array>
#include <map>
#include <vector>
#include <string>
#include "configwidget.h"
#include "enginesmodel.h"
#include "extension.h"
#include "core/query.h"
#include "util/standardaction.h"
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
using std::shared_ptr;
using std::vector;
using namespace Core;


namespace {

// http://data.iana.org/TLD/tlds-alpha-by-domain.txt
// Version 2017032500, Last Updated Sat Mar 25 07:07:02 2017 UTC
std::array<std::string, 1530> validTlds = { {"AAA", "AARP", "ABARTH", "ABB",
"ABBOTT", "ABBVIE", "ABC", "ABLE", "ABOGADO", "ABUDHABI", "AC", "ACADEMY",
"ACCENTURE", "ACCOUNTANT", "ACCOUNTANTS", "ACO", "ACTIVE", "ACTOR", "AD",
"ADAC", "ADS", "ADULT", "AE", "AEG", "AERO", "AETNA", "AF", "AFAMILYCOMPANY",
"AFL", "AFRICA", "AG", "AGAKHAN", "AGENCY", "AI", "AIG", "AIGO", "AIRBUS",
"AIRFORCE", "AIRTEL", "AKDN", "AL", "ALFAROMEO", "ALIBABA", "ALIPAY",
"ALLFINANZ", "ALLSTATE", "ALLY", "ALSACE", "ALSTOM", "AM", "AMERICANEXPRESS",
"AMERICANFAMILY", "AMEX", "AMFAM", "AMICA", "AMSTERDAM", "ANALYTICS",
"ANDROID", "ANQUAN", "ANZ", "AO", "AOL", "APARTMENTS", "APP", "APPLE", "AQ",
"AQUARELLE", "AR", "ARAMCO", "ARCHI", "ARMY", "ARPA", "ART", "ARTE", "AS",
"ASDA", "ASIA", "ASSOCIATES", "AT", "ATHLETA", "ATTORNEY", "AU", "AUCTION",
"AUDI", "AUDIBLE", "AUDIO", "AUSPOST", "AUTHOR", "AUTO", "AUTOS", "AVIANCA",
"AW", "AWS", "AX", "AXA", "AZ", "AZURE", "BA", "BABY", "BAIDU", "BANAMEX",
"BANANAREPUBLIC", "BAND", "BANK", "BAR", "BARCELONA", "BARCLAYCARD",
"BARCLAYS", "BAREFOOT", "BARGAINS", "BASEBALL", "BASKETBALL", "BAUHAUS",
"BAYERN", "BB", "BBC", "BBT", "BBVA", "BCG", "BCN", "BD", "BE", "BEATS",
"BEAUTY", "BEER", "BENTLEY", "BERLIN", "BEST", "BESTBUY", "BET", "BF", "BG",
"BH", "BHARTI", "BI", "BIBLE", "BID", "BIKE", "BING", "BINGO", "BIO", "BIZ",
"BJ", "BLACK", "BLACKFRIDAY", "BLANCO", "BLOCKBUSTER", "BLOG", "BLOOMBERG",
"BLUE", "BM", "BMS", "BMW", "BN", "BNL", "BNPPARIBAS", "BO", "BOATS",
"BOEHRINGER", "BOFA", "BOM", "BOND", "BOO", "BOOK", "BOOKING", "BOOTS",
"BOSCH", "BOSTIK", "BOSTON", "BOT", "BOUTIQUE", "BOX", "BR", "BRADESCO",
"BRIDGESTONE", "BROADWAY", "BROKER", "BROTHER", "BRUSSELS", "BS", "BT",
"BUDAPEST", "BUGATTI", "BUILD", "BUILDERS", "BUSINESS", "BUY", "BUZZ", "BV",
"BW", "BY", "BZ", "BZH", "CA", "CAB", "CAFE", "CAL", "CALL", "CALVINKLEIN",
"CAM", "CAMERA", "CAMP", "CANCERRESEARCH", "CANON", "CAPETOWN", "CAPITAL",
"CAPITALONE", "CAR", "CARAVAN", "CARDS", "CARE", "CAREER", "CAREERS", "CARS",
"CARTIER", "CASA", "CASE", "CASEIH", "CASH", "CASINO", "CAT", "CATERING",
"CATHOLIC", "CBA", "CBN", "CBRE", "CBS", "CC", "CD", "CEB", "CENTER", "CEO",
"CERN", "CF", "CFA", "CFD", "CG", "CH", "CHANEL", "CHANNEL", "CHASE", "CHAT",
"CHEAP", "CHINTAI", "CHLOE", "CHRISTMAS", "CHROME", "CHRYSLER", "CHURCH", "CI",
"CIPRIANI", "CIRCLE", "CISCO", "CITADEL", "CITI", "CITIC", "CITY", "CITYEATS",
"CK", "CL", "CLAIMS", "CLEANING", "CLICK", "CLINIC", "CLINIQUE", "CLOTHING",
"CLOUD", "CLUB", "CLUBMED", "CM", "CN", "CO", "COACH", "CODES", "COFFEE",
"COLLEGE", "COLOGNE", "COM", "COMCAST", "COMMBANK", "COMMUNITY", "COMPANY",
"COMPARE", "COMPUTER", "COMSEC", "CONDOS", "CONSTRUCTION", "CONSULTING",
"CONTACT", "CONTRACTORS", "COOKING", "COOKINGCHANNEL", "COOL", "COOP",
"CORSICA", "COUNTRY", "COUPON", "COUPONS", "COURSES", "CR", "CREDIT",
"CREDITCARD", "CREDITUNION", "CRICKET", "CROWN", "CRS", "CRUISE", "CRUISES",
"CSC", "CU", "CUISINELLA", "CV", "CW", "CX", "CY", "CYMRU", "CYOU", "CZ",
"DABUR", "DAD", "DANCE", "DATA", "DATE", "DATING", "DATSUN", "DAY", "DCLK",
"DDS", "DE", "DEAL", "DEALER", "DEALS", "DEGREE", "DELIVERY", "DELL",
"DELOITTE", "DELTA", "DEMOCRAT", "DENTAL", "DENTIST", "DESI", "DESIGN", "DEV",
"DHL", "DIAMONDS", "DIET", "DIGITAL", "DIRECT", "DIRECTORY", "DISCOUNT",
"DISCOVER", "DISH", "DIY", "DJ", "DK", "DM", "DNP", "DO", "DOCS", "DOCTOR",
"DODGE", "DOG", "DOHA", "DOMAINS", "DOT", "DOWNLOAD", "DRIVE", "DTV", "DUBAI",
"DUCK", "DUNLOP", "DUNS", "DUPONT", "DURBAN", "DVAG", "DVR", "DZ", "EARTH",
"EAT", "EC", "ECO", "EDEKA", "EDU", "EDUCATION", "EE", "EG", "EMAIL", "EMERCK",
"ENERGY", "ENGINEER", "ENGINEERING", "ENTERPRISES", "EPOST", "EPSON",
"EQUIPMENT", "ER", "ERICSSON", "ERNI", "ES", "ESQ", "ESTATE", "ESURANCE", "ET",
"EU", "EUROVISION", "EUS", "EVENTS", "EVERBANK", "EXCHANGE", "EXPERT",
"EXPOSED", "EXPRESS", "EXTRASPACE", "FAGE", "FAIL", "FAIRWINDS", "FAITH",
"FAMILY", "FAN", "FANS", "FARM", "FARMERS", "FASHION", "FAST", "FEDEX",
"FEEDBACK", "FERRARI", "FERRERO", "FI", "FIAT", "FIDELITY", "FIDO", "FILM",
"FINAL", "FINANCE", "FINANCIAL", "FIRE", "FIRESTONE", "FIRMDALE", "FISH",
"FISHING", "FIT", "FITNESS", "FJ", "FK", "FLICKR", "FLIGHTS", "FLIR",
"FLORIST", "FLOWERS", "FLY", "FM", "FO", "FOO", "FOOD", "FOODNETWORK",
"FOOTBALL", "FORD", "FOREX", "FORSALE", "FORUM", "FOUNDATION", "FOX", "FR",
"FREE", "FRESENIUS", "FRL", "FROGANS", "FRONTDOOR", "FRONTIER", "FTR",
"FUJITSU", "FUJIXEROX", "FUN", "FUND", "FURNITURE", "FUTBOL", "FYI", "GA",
"GAL", "GALLERY", "GALLO", "GALLUP", "GAME", "GAMES", "GAP", "GARDEN", "GB",
"GBIZ", "GD", "GDN", "GE", "GEA", "GENT", "GENTING", "GEORGE", "GF", "GG",
"GGEE", "GH", "GI", "GIFT", "GIFTS", "GIVES", "GIVING", "GL", "GLADE", "GLASS",
"GLE", "GLOBAL", "GLOBO", "GM", "GMAIL", "GMBH", "GMO", "GMX", "GN", "GODADDY",
"GOLD", "GOLDPOINT", "GOLF", "GOO", "GOODHANDS", "GOODYEAR", "GOOG", "GOOGLE",
"GOP", "GOT", "GOV", "GP", "GQ", "GR", "GRAINGER", "GRAPHICS", "GRATIS",
"GREEN", "GRIPE", "GROUP", "GS", "GT", "GU", "GUARDIAN", "GUCCI", "GUGE",
"GUIDE", "GUITARS", "GURU", "GW", "GY", "HAIR", "HAMBURG", "HANGOUT", "HAUS",
"HBO", "HDFC", "HDFCBANK", "HEALTH", "HEALTHCARE", "HELP", "HELSINKI", "HERE",
"HERMES", "HGTV", "HIPHOP", "HISAMITSU", "HITACHI", "HIV", "HK", "HKT", "HM",
"HN", "HOCKEY", "HOLDINGS", "HOLIDAY", "HOMEDEPOT", "HOMEGOODS", "HOMES",
"HOMESENSE", "HONDA", "HONEYWELL", "HORSE", "HOSPITAL", "HOST", "HOSTING",
"HOT", "HOTELES", "HOTMAIL", "HOUSE", "HOW", "HR", "HSBC", "HT", "HTC", "HU",
"HUGHES", "HYATT", "HYUNDAI", "IBM", "ICBC", "ICE", "ICU", "ID", "IE", "IEEE",
"IFM", "IKANO", "IL", "IM", "IMAMAT", "IMDB", "IMMO", "IMMOBILIEN", "IN",
"INDUSTRIES", "INFINITI", "INFO", "ING", "INK", "INSTITUTE", "INSURANCE",
"INSURE", "INT", "INTEL", "INTERNATIONAL", "INTUIT", "INVESTMENTS", "IO",
"IPIRANGA", "IQ", "IR", "IRISH", "IS", "ISELECT", "ISMAILI", "IST", "ISTANBUL",
"IT", "ITAU", "ITV", "IVECO", "IWC", "JAGUAR", "JAVA", "JCB", "JCP", "JE",
"JEEP", "JETZT", "JEWELRY", "JIO", "JLC", "JLL", "JM", "JMP", "JNJ", "JO",
"JOBS", "JOBURG", "JOT", "JOY", "JP", "JPMORGAN", "JPRS", "JUEGOS", "JUNIPER",
"KAUFEN", "KDDI", "KE", "KERRYHOTELS", "KERRYLOGISTICS", "KERRYPROPERTIES",
"KFH", "KG", "KH", "KI", "KIA", "KIM", "KINDER", "KINDLE", "KITCHEN", "KIWI",
"KM", "KN", "KOELN", "KOMATSU", "KOSHER", "KP", "KPMG", "KPN", "KR", "KRD",
"KRED", "KUOKGROUP", "KW", "KY", "KYOTO", "KZ", "LA", "LACAIXA", "LADBROKES",
"LAMBORGHINI", "LAMER", "LANCASTER", "LANCIA", "LANCOME", "LAND", "LANDROVER",
"LANXESS", "LASALLE", "LAT", "LATINO", "LATROBE", "LAW", "LAWYER", "LB", "LC",
"LDS", "LEASE", "LECLERC", "LEFRAK", "LEGAL", "LEGO", "LEXUS", "LGBT", "LI",
"LIAISON", "LIDL", "LIFE", "LIFEINSURANCE", "LIFESTYLE", "LIGHTING", "LIKE",
"LILLY", "LIMITED", "LIMO", "LINCOLN", "LINDE", "LINK", "LIPSY", "LIVE",
"LIVING", "LIXIL", "LK", "LOAN", "LOANS", "LOCKER", "LOCUS", "LOFT", "LOL",
"LONDON", "LOTTE", "LOTTO", "LOVE", "LPL", "LPLFINANCIAL", "LR", "LS", "LT",
"LTD", "LTDA", "LU", "LUNDBECK", "LUPIN", "LUXE", "LUXURY", "LV", "LY", "MA",
"MACYS", "MADRID", "MAIF", "MAISON", "MAKEUP", "MAN", "MANAGEMENT", "MANGO",
"MARKET", "MARKETING", "MARKETS", "MARRIOTT", "MARSHALLS", "MASERATI",
"MATTEL", "MBA", "MC", "MCD", "MCDONALDS", "MCKINSEY", "MD", "ME", "MED",
"MEDIA", "MEET", "MELBOURNE", "MEME", "MEMORIAL", "MEN", "MENU", "MEO",
"METLIFE", "MG", "MH", "MIAMI", "MICROSOFT", "MIL", "MINI", "MINT", "MIT",
"MITSUBISHI", "MK", "ML", "MLB", "MLS", "MM", "MMA", "MN", "MO", "MOBI",
"MOBILE", "MOBILY", "MODA", "MOE", "MOI", "MOM", "MONASH", "MONEY", "MONSTER",
"MONTBLANC", "MOPAR", "MORMON", "MORTGAGE", "MOSCOW", "MOTO", "MOTORCYCLES",
"MOV", "MOVIE", "MOVISTAR", "MP", "MQ", "MR", "MS", "MSD", "MT", "MTN", "MTPC",
"MTR", "MU", "MUSEUM", "MUTUAL", "MV", "MW", "MX", "MY", "MZ", "NA", "NAB",
"NADEX", "NAGOYA", "NAME", "NATIONWIDE", "NATURA", "NAVY", "NBA", "NC", "NE",
"NEC", "NET", "NETBANK", "NETFLIX", "NETWORK", "NEUSTAR", "NEW", "NEWHOLLAND",
"NEWS", "NEXT", "NEXTDIRECT", "NEXUS", "NF", "NFL", "NG", "NGO", "NHK", "NI",
"NICO", "NIKE", "NIKON", "NINJA", "NISSAN", "NISSAY", "NL", "NO", "NOKIA",
"NORTHWESTERNMUTUAL", "NORTON", "NOW", "NOWRUZ", "NOWTV", "NP", "NR", "NRA",
"NRW", "NTT", "NU", "NYC", "NZ", "OBI", "OBSERVER", "OFF", "OFFICE", "OKINAWA",
"OLAYAN", "OLAYANGROUP", "OLDNAVY", "OLLO", "OM", "OMEGA", "ONE", "ONG", "ONL",
"ONLINE", "ONYOURSIDE", "OOO", "OPEN", "ORACLE", "ORANGE", "ORG", "ORGANIC",
"ORIENTEXPRESS", "ORIGINS", "OSAKA", "OTSUKA", "OTT", "OVH", "PA", "PAGE",
"PAMPEREDCHEF", "PANASONIC", "PANERAI", "PARIS", "PARS", "PARTNERS", "PARTS",
"PARTY", "PASSAGENS", "PAY", "PCCW", "PE", "PET", "PF", "PFIZER", "PG", "PH",
"PHARMACY", "PHILIPS", "PHONE", "PHOTO", "PHOTOGRAPHY", "PHOTOS", "PHYSIO",
"PIAGET", "PICS", "PICTET", "PICTURES", "PID", "PIN", "PING", "PINK",
"PIONEER", "PIZZA", "PK", "PL", "PLACE", "PLAY", "PLAYSTATION", "PLUMBING",
"PLUS", "PM", "PN", "PNC", "POHL", "POKER", "POLITIE", "PORN", "POST", "PR",
"PRAMERICA", "PRAXI", "PRESS", "PRIME", "PRO", "PROD", "PRODUCTIONS", "PROF",
"PROGRESSIVE", "PROMO", "PROPERTIES", "PROPERTY", "PROTECTION", "PRU",
"PRUDENTIAL", "PS", "PT", "PUB", "PW", "PWC", "PY", "QA", "QPON", "QUEBEC",
"QUEST", "QVC", "RACING", "RADIO", "RAID", "RE", "READ", "REALESTATE",
"REALTOR", "REALTY", "RECIPES", "RED", "REDSTONE", "REDUMBRELLA", "REHAB",
"REISE", "REISEN", "REIT", "RELIANCE", "REN", "RENT", "RENTALS", "REPAIR",
"REPORT", "REPUBLICAN", "REST", "RESTAURANT", "REVIEW", "REVIEWS", "REXROTH",
"RICH", "RICHARDLI", "RICOH", "RIGHTATHOME", "RIL", "RIO", "RIP", "RMIT", "RO",
"ROCHER", "ROCKS", "RODEO", "ROGERS", "ROOM", "RS", "RSVP", "RU", "RUHR",
"RUN", "RW", "RWE", "RYUKYU", "SA", "SAARLAND", "SAFE", "SAFETY", "SAKURA",
"SALE", "SALON", "SAMSCLUB", "SAMSUNG", "SANDVIK", "SANDVIKCOROMANT", "SANOFI",
"SAP", "SAPO", "SARL", "SAS", "SAVE", "SAXO", "SB", "SBI", "SBS", "SC", "SCA",
"SCB", "SCHAEFFLER", "SCHMIDT", "SCHOLARSHIPS", "SCHOOL", "SCHULE", "SCHWARZ",
"SCIENCE", "SCJOHNSON", "SCOR", "SCOT", "SD", "SE", "SEAT", "SECURE",
"SECURITY", "SEEK", "SELECT", "SENER", "SERVICES", "SES", "SEVEN", "SEW",
"SEX", "SEXY", "SFR", "SG", "SH", "SHANGRILA", "SHARP", "SHAW", "SHELL",
"SHIA", "SHIKSHA", "SHOES", "SHOP", "SHOPPING", "SHOUJI", "SHOW", "SHOWTIME",
"SHRIRAM", "SI", "SILK", "SINA", "SINGLES", "SITE", "SJ", "SK", "SKI", "SKIN",
"SKY", "SKYPE", "SL", "SLING", "SM", "SMART", "SMILE", "SN", "SNCF", "SO",
"SOCCER", "SOCIAL", "SOFTBANK", "SOFTWARE", "SOHU", "SOLAR", "SOLUTIONS",
"SONG", "SONY", "SOY", "SPACE", "SPIEGEL", "SPOT", "SPREADBETTING", "SR",
"SRL", "SRT", "ST", "STADA", "STAPLES", "STAR", "STARHUB", "STATEBANK",
"STATEFARM", "STATOIL", "STC", "STCGROUP", "STOCKHOLM", "STORAGE", "STORE",
"STREAM", "STUDIO", "STUDY", "STYLE", "SU", "SUCKS", "SUPPLIES", "SUPPLY",
"SUPPORT", "SURF", "SURGERY", "SUZUKI", "SV", "SWATCH", "SWIFTCOVER", "SWISS",
"SX", "SY", "SYDNEY", "SYMANTEC", "SYSTEMS", "SZ", "TAB", "TAIPEI", "TALK",
"TAOBAO", "TARGET", "TATAMOTORS", "TATAR", "TATTOO", "TAX", "TAXI", "TC",
"TCI", "TD", "TDK", "TEAM", "TECH", "TECHNOLOGY", "TEL", "TELECITY",
"TELEFONICA", "TEMASEK", "TENNIS", "TEVA", "TF", "TG", "TH", "THD", "THEATER",
"THEATRE", "TIAA", "TICKETS", "TIENDA", "TIFFANY", "TIPS", "TIRES", "TIROL",
"TJ", "TJMAXX", "TJX", "TK", "TKMAXX", "TL", "TM", "TMALL", "TN", "TO",
"TODAY", "TOKYO", "TOOLS", "TOP", "TORAY", "TOSHIBA", "TOTAL", "TOURS", "TOWN",
"TOYOTA", "TOYS", "TR", "TRADE", "TRADING", "TRAINING", "TRAVEL",
"TRAVELCHANNEL", "TRAVELERS", "TRAVELERSINSURANCE", "TRUST", "TRV", "TT",
"TUBE", "TUI", "TUNES", "TUSHU", "TV", "TVS", "TW", "TZ", "UA", "UBANK", "UBS",
"UCONNECT", "UG", "UK", "UNICOM", "UNIVERSITY", "UNO", "UOL", "UPS", "US",
"UY", "UZ", "VA", "VACATIONS", "VANA", "VANGUARD", "VC", "VE", "VEGAS",
"VENTURES", "VERISIGN", "VERSICHERUNG", "VET", "VG", "VI", "VIAJES", "VIDEO",
"VIG", "VIKING", "VILLAS", "VIN", "VIP", "VIRGIN", "VISA", "VISION", "VISTA",
"VISTAPRINT", "VIVA", "VIVO", "VLAANDEREN", "VN", "VODKA", "VOLKSWAGEN",
"VOLVO", "VOTE", "VOTING", "VOTO", "VOYAGE", "VU", "VUELOS", "WALES",
"WALMART", "WALTER", "WANG", "WANGGOU", "WARMAN", "WATCH", "WATCHES",
"WEATHER", "WEATHERCHANNEL", "WEBCAM", "WEBER", "WEBSITE", "WED", "WEDDING",
"WEIBO", "WEIR", "WF", "WHOSWHO", "WIEN", "WIKI", "WILLIAMHILL", "WIN",
"WINDOWS", "WINE", "WINNERS", "WME", "WOLTERSKLUWER", "WOODSIDE", "WORK",
"WORKS", "WORLD", "WOW", "WS", "WTC", "WTF", "XBOX", "XEROX", "XFINITY",
"XIHUAN", "XIN", "XN--11B4C3D", "XN--1CK2E1B", "XN--1QQW23A", "XN--30RR7Y",
"XN--3BST00M", "XN--3DS443G", "XN--3E0B707E", "XN--3OQ18VL8PN36A",
"XN--3PXU8K", "XN--42C2D9A", "XN--45BRJ9C", "XN--45Q11C", "XN--4GBRIM",
"XN--54B7FTA0CC", "XN--55QW42G", "XN--55QX5D", "XN--5SU34J936BGSG",
"XN--5TZM5G", "XN--6FRZ82G", "XN--6QQ986B3XL", "XN--80ADXHKS", "XN--80AO21A",
"XN--80AQECDR1A", "XN--80ASEHDB", "XN--80ASWG", "XN--8Y0A063A", "XN--90A3AC",
"XN--90AE", "XN--90AIS", "XN--9DBQ2A", "XN--9ET52U", "XN--9KRT00A",
"XN--B4W605FERD", "XN--BCK1B9A5DRE4C", "XN--C1AVG", "XN--C2BR7G",
"XN--CCK2B3B", "XN--CG4BKI", "XN--CLCHC0EA0B2G2A9GCD", "XN--CZR694B",
"XN--CZRS0T", "XN--CZRU2D", "XN--D1ACJ3B", "XN--D1ALF", "XN--E1A4C",
"XN--ECKVDTC9D", "XN--EFVY88H", "XN--ESTV75G", "XN--FCT429K", "XN--FHBEI",
"XN--FIQ228C5HS", "XN--FIQ64B", "XN--FIQS8S", "XN--FIQZ9S", "XN--FJQ720A",
"XN--FLW351E", "XN--FPCRJ9C3D", "XN--FZC2C9E2C", "XN--FZYS8D69UVGM",
"XN--G2XX48C", "XN--GCKR3F0F", "XN--GECRJ9C", "XN--GK3AT1E", "XN--H2BRJ9C",
"XN--HXT814E", "XN--I1B6B1A6A2E", "XN--IMR513N", "XN--IO0A7I", "XN--J1AEF",
"XN--J1AMH", "XN--J6W193G", "XN--JLQ61U9W7B", "XN--JVR189M", "XN--KCRX77D1X4A",
"XN--KPRW13D", "XN--KPRY57D", "XN--KPU716F", "XN--KPUT3I", "XN--L1ACC",
"XN--LGBBAT1AD8J", "XN--MGB9AWBF", "XN--MGBA3A3EJT", "XN--MGBA3A4F16A",
"XN--MGBA7C0BBN0A", "XN--MGBAAM7A8H", "XN--MGBAB2BD", "XN--MGBAI9AZGQP6J",
"XN--MGBAYH7GPA", "XN--MGBB9FBPOB", "XN--MGBBH1A71E", "XN--MGBC0A9AZCG",
"XN--MGBCA7DZDO", "XN--MGBERP4A5D4AR", "XN--MGBI4ECEXP", "XN--MGBPL2FH",
"XN--MGBT3DHD", "XN--MGBTX2B", "XN--MGBX4CD0AB", "XN--MIX891F", "XN--MK1BU44C",
"XN--MXTQ1M", "XN--NGBC5AZD", "XN--NGBE9E0A", "XN--NODE", "XN--NQV7F",
"XN--NQV7FS00EMA", "XN--NYQY26A", "XN--O3CW4H", "XN--OGBPF8FL", "XN--P1ACF",
"XN--P1AI", "XN--PBT977C", "XN--PGBS0DH", "XN--PSSY2U", "XN--Q9JYB4C",
"XN--QCKA1PMC", "XN--QXAM", "XN--RHQV96G", "XN--ROVU88B", "XN--S9BRJ9C",
"XN--SES554G", "XN--T60B56A", "XN--TCKWE", "XN--TIQ49XQYJ", "XN--UNUP4Y",
"XN--VERMGENSBERATER-CTB", "XN--VERMGENSBERATUNG-PWB", "XN--VHQUV",
"XN--VUQ861B", "XN--W4R85EL8FHU5DNRA", "XN--W4RS40L", "XN--WGBH1C",
"XN--WGBL6A", "XN--XHQ521B", "XN--XKC2AL3HYE2A", "XN--XKC2DL3A5EE0H",
"XN--Y9A3AQ", "XN--YFRO4I67O", "XN--YGBI2AMMX", "XN--ZFR164B", "XPERIA", "XXX",
"XYZ", "YACHTS", "YAHOO", "YAMAXUN", "YANDEX", "YE", "YODOBASHI", "YOGA",
"YOKOHAMA", "YOU", "YOUTUBE", "YT", "YUN", "ZA", "ZAPPOS", "ZARA", "ZERO",
"ZIP", "ZIPPO", "ZM", "ZONE", "ZUERICH", "ZW"}
};

std::vector<Websearch::SearchEngine> defaultSearchEngines = {
    {"Google",        "gg ",  ":google",    "https://www.google.com/search?q=%s"},
    {"Youtube",       "yt ",  ":youtube",   "https://www.youtube.com/results?search_query=%s"},
    {"Amazon",        "ama ", ":amazon",    "http://www.amazon.com/s/?field-keywords=%s"},
    {"Ebay",          "eb ",  ":ebay",      "http://www.ebay.com/sch/i.html?_nkw=%s"},
    {"GitHub",        "gh ",  ":github",    "https://github.com/search?utf8=✓&q=%s"},
    {"Wikipedia",     "wiki ",":wikipedia", "https://en.wikipedia.org/w/index.php?search=%s"},
    {"Wolfram Alpha", "=",    ":wolfram",   "https://www.wolframalpha.com/input/?i=%s"},
    {"DuckDuckGo",    "dd",   ":duckduckgo","https://duckduckgo.com/?q=%s"},
};

shared_ptr<Core::Item> buildWebsearchItem(const Websearch::SearchEngine &se, const QString &searchterm) {

    QString urlString = QString(se.url).replace("%s", QUrl::toPercentEncoding(searchterm));
    QUrl url = QUrl(urlString);
    QString desc = QString("Start %1 search in your browser").arg(se.name);

    std::shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
    action->setText(desc);
    action->setAction([=](){ QDesktopServices::openUrl(url); });

    std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>(se.name);
    item->setText(se.name);
    item->setSubtext(desc);
    item->setIconPath(se.iconPath);
    item->setCompletionString(QString("%1%2").arg(se.trigger, searchterm));

    item->setActions({action});

    return item;
}

static constexpr const char * ENGINES_FILE_NAME = "engines.json";

}



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
class Websearch::Private
{
public:
    QPointer<ConfigWidget> widget;
    std::vector<SearchEngine> searchEngines;
};



/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
/** ***************************************************************************/
Websearch::Extension::Extension()
    : Core::Extension("org.albert.extension.websearch"),
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    std::sort(validTlds.begin(), validTlds.end());

    // Move config file from old location to new. (data -> config) TODO: REMOVE in 0.14
    QString oldpath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation))
            .filePath(QString("%1.json").arg(Core::Plugin::id()));
    QString enginesJson = configLocation().filePath(ENGINES_FILE_NAME);
    if ( QFile::exists(oldpath) ) {
        if ( QFile::exists(enginesJson) )
            QFile::remove(oldpath);
        else
            QFile::rename(oldpath, enginesJson);
    }

    // Deserialize engines
    QFile file(enginesJson);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonArray array = QJsonDocument::fromJson(file.readAll()).array();
        SearchEngine searchEngine;
        for ( const QJsonValue& value : array) {
            QJsonObject object = value.toObject();
            searchEngine.name     = object["name"].toString();
            searchEngine.trigger  = object["trigger"].toString();
            searchEngine.iconPath = object["iconPath"].toString();
            searchEngine.url      = object["url"].toString();
            d->searchEngines.push_back(searchEngine);
        }
    } else {
        qWarning() << qPrintable(QString("Could not load from file: '%1'.").arg(enginesJson));
        setEngines(defaultSearchEngines);
    }
}



/** ***************************************************************************/
Websearch::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *Websearch::Extension::widget(QWidget *parent) {
    if (d->widget.isNull())
        d->widget = new ConfigWidget(this, parent);
    return d->widget;
}



/** ***************************************************************************/
QStringList Websearch::Extension::triggers() const {
      QStringList triggers;
      for ( const SearchEngine& se : d->searchEngines )
          triggers.push_back(se.trigger);
      return triggers;
}



/** ***************************************************************************/
void Websearch::Extension::handleQuery(Core::Query * query) const {

    if ( query->searchTerm().isEmpty() )
        return;

    if ( !query->trigger().isNull() ) {
        for (const SearchEngine &se : d->searchEngines)
            if (query->searchTerm().startsWith(se.trigger))
                // Implicit move
                query->addMatch(buildWebsearchItem(se, query->searchTerm().mid(se.trigger.size())));
    }
    else
    {
        QUrl url = QUrl::fromUserInput(query->searchTerm());

        // Check syntax and TLD validity
        if ( url.isValid() && ( // Check syntax
             query->searchTerm().trimmed().startsWith("http://") ||
             query->searchTerm().trimmed().startsWith("https://") ||
             (QRegularExpression(R"R(\S+\.\S+$)R").match(url.host()).hasMatch() &&  // Check if not an emty tld
              std::binary_search(validTlds.begin(), validTlds.end(), // Check tld validiy
                                 url.topLevelDomain().mid(1).toUpper().toLocal8Bit().constData()) ))) {

            shared_ptr<StandardAction> action = std::make_shared<StandardAction>();
            action->setText("Open URL");
            action->setAction([url](){
                QDesktopServices::openUrl(url);
            });

            std::shared_ptr<StandardItem> item = std::make_shared<StandardItem>("valid_url");
            item->setText(QString("Open url in browser"));
            item->setSubtext(QString("Visit %1").arg(url.authority()));
            item->setCompletionString(query->searchTerm());
            QString icon = XDG::IconLookup::iconPath({"www", "web-browser", "emblem-web"});
            item->setIconPath(icon.isEmpty() ? ":favicon" : icon);
            item->setActions({action});

            query->addMatch(std::move(item), UINT_MAX);
        }
    }
}



/** ***************************************************************************/
vector<shared_ptr<Core::Item>> Websearch::Extension::fallbacks(const QString & searchterm) {
    vector<shared_ptr<Core::Item>> res;
    for (const SearchEngine &se : d->searchEngines)
        res.push_back(buildWebsearchItem(se, searchterm));
    return res;
}



/** ***************************************************************************/
const std::vector<Websearch::SearchEngine> &Websearch::Extension::engines() const {
    return d->searchEngines;
}



/** ***************************************************************************/
void Websearch::Extension::setEngines(const std::vector<Websearch::SearchEngine> &engines) {
    d->searchEngines = engines;
    emit enginesChanged(d->searchEngines);

    // Serialize the engines
    QFile file(configLocation().filePath(ENGINES_FILE_NAME));
    if (file.open(QIODevice::WriteOnly)) {
        QJsonArray array;
        for ( const SearchEngine& searchEngine : d->searchEngines ) {
            QJsonObject object;
            object["name"]     = searchEngine.name;
            object["url"]      = searchEngine.url;
            object["trigger"]  = searchEngine.trigger;
            object["iconPath"] = searchEngine.iconPath;
            array.append(object);
        }
        file.write(QJsonDocument(array).toJson());
    } else
        qCritical() << qPrintable(QString("Could not write to file: '%1'.").arg(file.fileName()));
}



/** ***************************************************************************/
void Websearch::Extension::restoreDefaultEngines() {
    setEngines(defaultSearchEngines);
}
