#include <avakar/wtf8.h>
#include <catch2/catch.hpp>
using namespace avakar::wtf8;

#if WCHAR_MAX == 0xffff
#define TEST_WCHAR 1
#endif

TEST_CASE("empty string is valid WTF-8/UTF-8/UTF-16")
{
	REQUIRE(is_wtf8(""));
	REQUIRE(is_wtf8(u8""));

	REQUIRE(is_utf8(""));
	REQUIRE(is_utf8(u8""));

#if TEST_WCHAR
	REQUIRE(is_utf16(L""));
#endif
	REQUIRE(is_utf16(u""));
}

TEST_CASE("empty strings convert to empty strings")
{
#if TEST_WCHAR
	REQUIRE(to_wstring("") == L"");
	REQUIRE(to_wstring(u8"") == L"");
	REQUIRE(to_string(L"") == "");
	REQUIRE(to_u8string(L"") == u8"");
#endif

	REQUIRE(to_u16string("") == u"");
	REQUIRE(to_u16string(u8"") == u"");
	REQUIRE(to_string(u"") == "");
	REQUIRE(to_u8string(u"") == u8"");
}

TEST_CASE("UTF-8 sequences are valid WTF-8")
{
	auto validate = [](auto const * s) {
		REQUIRE(is_wtf8(s));
		REQUIRE(is_utf8(s));
	};

	validate("a");
	validate("ab");
	validate("\xc3\xa1");
	validate("\xc3\xa1");
	validate("\xe2\xba\x81");
	validate("\xf0\x9f\x98\x80");

	validate("the e0 prefix can sometimes be overlong, but not this one: \xe0\xa0\x81");
}

TEST_CASE("invalid WTF-8 sequences are invalid UTF-8")
{
	auto validate = [](auto const * s) {
		REQUIRE_FALSE(is_wtf8(s));
		REQUIRE_FALSE(is_utf8(s));
	};

	validate("unexpected continuation: \x80");
	validate("overlong 2byte sequence: \xc0\x81");
	validate("too much continuation: \xc2\x81\x81");
	validate("not enough continuation: \xc2");
	validate("not enough continuation: \xc2 ");
	validate("not enough continuation: \xc2\xc2\x81");

	validate("not enough continuation: \xe1");
	validate("not enough continuation: \xe1 ");
	validate("not enough continuation: \xe1\xe1");
	validate("not enough continuation: \xe1\xe1\x81");

	validate("not enough continuation: \xe1\x81");
	validate("not enough continuation: \xe1\x81 ");
	validate("not enough continuation: \xe1\x81\xe1");
	validate("not enough continuation: \xe1\x81\xe1\x81\x82");

	validate("overlong 3byte sequence: \xe0\x90\x81");

	validate("not enough continuation: \xf0");
	validate("not enough continuation: \xf0 ");
	validate("not enough continuation: \xf0\xf0");
	validate("not enough continuation: \xf0\xf0\x9f\x98\x80");

	validate("not enough continuation: \xf0\x9f");
	validate("not enough continuation: \xf0\x9f ");
	validate("not enough continuation: \xf0\x9f\xf0");
	validate("not enough continuation: \xf0\x9f\xf0\x9f\x98\x80");

	validate("not enough continuation: \xf0\x9f\x98");
	validate("not enough continuation: \xf0\x9f\x98 ");
	validate("not enough continuation: \xf0\x9f\x98\xf0");
	validate("not enough continuation: \xf0\x9f\x98\xf0\x9f\x98\x80");

	validate("overlong 4byte sequence: \xf0\x8f\x98\x80");

	validate("code point beyond U+10FFFF: \xf4\xbf\x98\x80");

	validate("invalid initial char: \xf5\xbf\x98\x80\x80");
}

TEST_CASE("surrogates are allowed in WTF-8")
{
	REQUIRE(is_wtf8("\xed\xa0\x80"));
	REQUIRE(!is_utf8("\xed\xa0\x80"));

	REQUIRE(is_wtf8("\xed\xb0\x80"));
	REQUIRE(!is_utf8("\xed\xb0\x80"));

	REQUIRE(is_wtf8("\xed\xa0\x80\xed\xb0\x80"));
	REQUIRE(!is_utf8("\xed\xa0\x80\xed\xb0\x80"));

	REQUIRE(is_wtf8("\xed\xa1\x80\xed\xb0\x80"));
	REQUIRE(!is_utf8("\xed\xa1\x80\xed\xb0\x80"));
}

TEST_CASE("unparied surrogates are not valid UTF-16")
{
	static constexpr char16_t long_high_surrogate1[] = { 0xd810, 0 };
	static constexpr char16_t long_high_surrogate2[] = { 0xd810, ' ', 0 };
	static constexpr char16_t long_high_surrogate3[] = { 0xd810, 0xd810, 0 };
	static constexpr char16_t long_high_surrogate4[] = { 0xd810, 0xd810, 0xdc00, 0 };

	static constexpr char16_t long_low_surrogate1[] = { 0xdc00, 0 };
	static constexpr char16_t overlong_surrogate_pair1[] = { 0xd800, 0xdc00, 0 };

	static constexpr char16_t surrogate_pair1[] = { 0xd840, 0xdc00, 0 };

	REQUIRE(is_utf16(u"\u1000"));
	REQUIRE(!is_utf16(long_high_surrogate1));
	REQUIRE(!is_utf16(long_high_surrogate2));
	REQUIRE(!is_utf16(long_high_surrogate3));
	REQUIRE(!is_utf16(long_high_surrogate4));
	REQUIRE(!is_utf16(long_low_surrogate1));
	REQUIRE(!is_utf16(overlong_surrogate_pair1));
	REQUIRE(is_utf16(surrogate_pair1));
}

TEST_CASE("simple conversions")
{
	REQUIRE(to_u16string("") == u"");
	REQUIRE(to_u16string("a") == u"a");
	REQUIRE(to_u16string("\xc3\xa1") == u"\u00e1");

	REQUIRE_THROWS(to_u16string("\xc3"));

#if TEST_WCHAR
	REQUIRE_THROWS(to_wstring("\xc3"));
#endif
}

constexpr char8_t const lipsum[] = u8R"(
Lorem ípsűm dolor sít amet, commodo áppetéré vis űt, mei facér dicám fuissét
an. Ea cúm pöstea invenire réferrentur. Ut scrípta vőlúptűa his. Ei aperiam
molestíáe vis, ut populö interesset íus. Clíta cétéros eös et, íd sensibus
alíqüando sit.

Facéte senténtiae ea mei. Nostrüm suscipít dispűtátíöni ín sed, ea his őratío
feugait árgumentum. Ex vis justó indoctúm, vivendö füísset accommódare éx
eüm. Illüm deléctús víx ne. Púrto únum invidunt vis et, id sit qúas lörem
tatiőn.

In vim labore dispútándó cőncludaturque, vís an commune placerat. Te pro
tatíön scribentur, át qui erős malorum, his eu primís diceret legendós. Prí
ín dolorem öpórteat quaestiö, cú has elit páúló déleniti. Vix quem qűaestio
at, nó per viris cöngüe pártíendó. Vidisse blandit hendrerit nö sed. Errör
pőpulo laborámus et sit, stet ignötá per ex.

Eüm et vítae delenit, no sőleát dícunt érroribús sít. Gloriatur conceptam
conclúdatúrqúe düo ét. Postulant maluísset definítionés séd ad, deléctűs
vivendum nec ut, régióne áltérüm tacímates no mel. Duo qúod scriptőrem át,
póssit principes cönsúlatu sit in.

Et nöstrő corpora pri, cú est magná mnesarchum sadipscing. Legere blándit
volútpat ne eős, eripúit dőlörem dűő ei, mea possim réförmidáns cömprehensam
et. Has ád eius dölores petentiúm. Ex őption dissentiás sea, laboraműs
reprimiqúe eos eu. Rébum mazim cüm ne, dicam aliquip dölorem ea mea, éüm ei
sale facilisi.

Debitís vűlputaté vim ad. In his erant éloqúentiam theóphrastűs. Víris decőre
sit at. Id qűót elít chóro nam, at saperet antíopám nec. Vix homero láóreet
epicürei ei, vix te verear cömmodő déníque. Sea qűót dispútando dissentiűnt
cu.

Est né nemöré dólores invenire, ex est aperiám vivendö, át gráecö delicata
voluptatibús vís. Delicata indöctúm séd éu. Düo ei chóro mandamus adipiscing,
nec id töta sölűtá delicátissimi, chorö tamquám qualísque eüm éi. Fierent
éleifend cönceptam ne vim.

Méi id dicta áltérá, qüis légimús blandit vel ut. Ex sint siműl vitae vím. Ea
illűm féugait qui. Nő eös düis nemoré partiendó, est ea utróqűé propriaé
malüissét. Cúm an meís quandö molestíe, animal defíniébas intéresset ut sed,
detractö placerat an vél. Consul rationibus vőlűptatibús meí ea, nó pro
utroque éleifend appellantur, et eránt regione mel. Te veritus mediocrem mea.

Melius sadipscing has et, mei ludus moderatius appellantur né. Pertínax
sententiaé ad pro, méi mundi impedit scriptorém íd. Solet nostér graecís sit
eú, usű talé ubique íntérprétaris no, prö qűod libris vivendüm té. Cu pro
tatión sanctus tibíqué, ex paulö dicám sensérit mea. Nam ea fugít cónseqúát
ómittantűr, vím populo éirmod percipitűr éi.

Aliqűam eűripidis ut üsu. Eam assum salütandi cönseqűűntür ex, ea novúm
héndrerit est, decőre qűaeqúe pri an. Sea facer détraxít mödératiűs ei, iús
harum noströ rátionibüs ut. Probő graécis mel eu, et magna mollis senserit
quí. Nó quí főrensibus demócritúm ratiönibüs. Pro nö possim contentionés, his
id solet prodesset. Cűm át mazím vitüperáta.
)";

constexpr char16_t const lipsum_u16[] = uR"(
Lorem ípsűm dolor sít amet, commodo áppetéré vis űt, mei facér dicám fuissét
an. Ea cúm pöstea invenire réferrentur. Ut scrípta vőlúptűa his. Ei aperiam
molestíáe vis, ut populö interesset íus. Clíta cétéros eös et, íd sensibus
alíqüando sit.

Facéte senténtiae ea mei. Nostrüm suscipít dispűtátíöni ín sed, ea his őratío
feugait árgumentum. Ex vis justó indoctúm, vivendö füísset accommódare éx
eüm. Illüm deléctús víx ne. Púrto únum invidunt vis et, id sit qúas lörem
tatiőn.

In vim labore dispútándó cőncludaturque, vís an commune placerat. Te pro
tatíön scribentur, át qui erős malorum, his eu primís diceret legendós. Prí
ín dolorem öpórteat quaestiö, cú has elit páúló déleniti. Vix quem qűaestio
at, nó per viris cöngüe pártíendó. Vidisse blandit hendrerit nö sed. Errör
pőpulo laborámus et sit, stet ignötá per ex.

Eüm et vítae delenit, no sőleát dícunt érroribús sít. Gloriatur conceptam
conclúdatúrqúe düo ét. Postulant maluísset definítionés séd ad, deléctűs
vivendum nec ut, régióne áltérüm tacímates no mel. Duo qúod scriptőrem át,
póssit principes cönsúlatu sit in.

Et nöstrő corpora pri, cú est magná mnesarchum sadipscing. Legere blándit
volútpat ne eős, eripúit dőlörem dűő ei, mea possim réförmidáns cömprehensam
et. Has ád eius dölores petentiúm. Ex őption dissentiás sea, laboraműs
reprimiqúe eos eu. Rébum mazim cüm ne, dicam aliquip dölorem ea mea, éüm ei
sale facilisi.

Debitís vűlputaté vim ad. In his erant éloqúentiam theóphrastűs. Víris decőre
sit at. Id qűót elít chóro nam, at saperet antíopám nec. Vix homero láóreet
epicürei ei, vix te verear cömmodő déníque. Sea qűót dispútando dissentiűnt
cu.

Est né nemöré dólores invenire, ex est aperiám vivendö, át gráecö delicata
voluptatibús vís. Delicata indöctúm séd éu. Düo ei chóro mandamus adipiscing,
nec id töta sölűtá delicátissimi, chorö tamquám qualísque eüm éi. Fierent
éleifend cönceptam ne vim.

Méi id dicta áltérá, qüis légimús blandit vel ut. Ex sint siműl vitae vím. Ea
illűm féugait qui. Nő eös düis nemoré partiendó, est ea utróqűé propriaé
malüissét. Cúm an meís quandö molestíe, animal defíniébas intéresset ut sed,
detractö placerat an vél. Consul rationibus vőlűptatibús meí ea, nó pro
utroque éleifend appellantur, et eránt regione mel. Te veritus mediocrem mea.

Melius sadipscing has et, mei ludus moderatius appellantur né. Pertínax
sententiaé ad pro, méi mundi impedit scriptorém íd. Solet nostér graecís sit
eú, usű talé ubique íntérprétaris no, prö qűod libris vivendüm té. Cu pro
tatión sanctus tibíqué, ex paulö dicám sensérit mea. Nam ea fugít cónseqúát
ómittantűr, vím populo éirmod percipitűr éi.

Aliqűam eűripidis ut üsu. Eam assum salütandi cönseqűűntür ex, ea novúm
héndrerit est, decőre qűaeqúe pri an. Sea facer détraxít mödératiűs ei, iús
harum noströ rátionibüs ut. Probő graécis mel eu, et magna mollis senserit
quí. Nó quí főrensibus demócritúm ratiönibüs. Pro nö possim contentionés, his
id solet prodesset. Cűm át mazím vitüperáta.
)";

TEST_CASE("long conversion")
{
	REQUIRE(avakar::wtf8::to_u16string(lipsum) == lipsum_u16);
}
