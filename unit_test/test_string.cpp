#include <phon/third_party/catch.hpp>
#include <phon/string.hpp>
#include <phon/dictionary.hpp>

TEST_CASE("Test empty string", "[string]")
{
	String s;
    REQUIRE(s.use_count() == 2);
	REQUIRE(s.empty());
	REQUIRE(s.capacity() == meta::pointer_size);
}

TEST_CASE("Test COW", "[string]")
{
	String s1("hello");
	String s2 = s1;

	REQUIRE(s1 == "hello");
	REQUIRE(s2 == s1);
	REQUIRE(s1.shared());

	s2.reserve(32);
	REQUIRE(s1.unique());
	REQUIRE(s2.unique());
	REQUIRE(s1 == "hello");
	REQUIRE(s2 == "hello");
}

TEST_CASE("Test insertion", "[string]")
{
	String s1("hello");
	String s2 = s1;
	String s3("world!");

	s2.append(" world!");
	REQUIRE(s1 == "hello");
	REQUIRE(s2 == "hello world!");
	REQUIRE(s2.size() == 12);

	s3.prepend("hello");
	REQUIRE(s3 == "helloworld!");
	s3.insert(s3.begin() + 5, " ");
	REQUIRE(s3 == "hello world!");

	String s4("abcdeflmn");
	String s5 = s4;
	s4.insert(s4.begin() + 6, "ghijk");
	REQUIRE(s4 == "abcdefghijklmn");
	REQUIRE(s5 == "abcdeflmn");

	String s6("abcdefjklmn");
	String s7 = s6;
	s6.insert(s6.begin() + 6, "ghi");
	REQUIRE(s6 == "abcdefghijklmn");
	REQUIRE(s7 == "abcdefjklmn");

	String s8("ɛ̃tɑ̃pstif");
	s8.insert(5, "ɛ");
	REQUIRE(s8 == "ɛ̃tɑ̃pɛstif");
}

TEST_CASE("Test unicode conversion", "[string]")
{
	String s1("안녕하세요");
	auto s2 = s1.to_utf16();
	REQUIRE(s1 == String::from_utf16(s2));

	auto s3 = s1.to_utf32();
	REQUIRE(s1 == String::from_utf32(s3));

	auto s4 = s1.to_wide();
	REQUIRE(s1 == String::from_wide(s4));
}

TEST_CASE("Test unicode", "[string]")
{
	String hangul("안녕하세요");
	String ipa("ɛ̃troɛ̹"); // tʷɛl
	String hindi("अनुच्छेद");
	String intempestif("ɛ̃tɑ̃pɛstif");
	String noel("noël");
	String devanagari("षि");
	String tamil("நி");
	String e1("é");
	String e2("é");

	REQUIRE(hangul.grapheme_count() == 5);
	REQUIRE(ipa.grapheme_count() == 5);
	REQUIRE(hindi.grapheme_count() == 5);
	REQUIRE(intempestif.grapheme_count() == 9);
	REQUIRE(noel.grapheme_count() == 4);
	REQUIRE(devanagari.grapheme_count() == 1);
	REQUIRE(tamil.grapheme_count() == 1);
	REQUIRE(e1.grapheme_count() == 1);
	REQUIRE(e2.grapheme_count() == 1);
}

TEST_CASE("Test substring", "[string]")
{
	String hangul("안녕하세요");
	REQUIRE(hangul.starts_with("안녕"));
	REQUIRE(hangul.ends_with("세요"));
	REQUIRE(hangul.contains("하세"));
}

TEST_CASE("Test trimming", "[string]")
{
	String s("\t\thello !  \n");
	REQUIRE(s.ltrim() == "hello !  \n");
	s = "\t\thello !  \n";
	REQUIRE(s.rtrim() == "\t\thello !");
	s = "\t\thello !  \n";
	REQUIRE(s.trim() == "hello !");

	String s2("\t\thello !  \n");
	String s3 = s2;
	REQUIRE(s2.trim() == "hello !");
	REQUIRE(s3 == "\t\thello !  \n");
}


TEST_CASE("Test string transformation", "[string]")
{
	String s1("c'était ça");
	String s2("C'ÉTAIT ÇA");
	String s3("aç tiaté'c");
	String s0 = s1;
	String noel("noël");

	REQUIRE(s1.to_upper() == s2);
	REQUIRE(s2.to_lower() == s0);
	REQUIRE(s0.reverse() == s3);
	REQUIRE(noel.reverse() == "lëon");
}


TEST_CASE("Test string replacement", "[string]")
{
	String s1("c'étXXXit çXXX");
	String s2 = s1; // shared string doesn't use the same path for replacement, so we check both.
	String s3("c'étXXXit çXXX");
	REQUIRE(s1.replace_first("XXX", "a") == "c'était çXXX");
	REQUIRE(s3.replace_first("XXX", "a") == "c'était çXXX");

	String s4("c'était ça");
	String s5(s4);
	String s6(s4);

	REQUIRE(s4.remove("a") == "c'étit ç");
	REQUIRE(s5.remove_first("a") == "c'étit ça");
	REQUIRE(s6.remove_last("a") == "c'était ç");

	String s7("c'était ça");
	String s8(s7);
	String s9(s7);

	REQUIRE(s7.replace("a", "XXX") == "c'étXXXit çXXX");
	REQUIRE(s8.replace_first("a", "XXX") == "c'étXXXit ça");
	REQUIRE(s9.replace_last("a", "XXX") == "c'était çXXX");

	REQUIRE(s7.replace_first("XXX", "a") == "c'était çXXX");
	REQUIRE(s7.replace("XXX", "a") == "c'était ça");
	REQUIRE(s7.replace("ç", "Ç") == "c'était Ça");
}

TEST_CASE("Test string left/right", "[string]")
{
	String s1("안녕하세요");
	String s2("an ascii string");
	String noel("noël");

	REQUIRE(s1.left(3) == "안녕하");
	REQUIRE(s1.right(2) == "세요");

	REQUIRE(s2.left(4) == "an a");
	REQUIRE(s2.right(4) == "ring");
	REQUIRE(noel.left(3) == "noë");
	REQUIRE(noel.right(2) == "ël");
}

TEST_CASE("Test join/split string", "[string]")
{
	String s1("alpha;beta;gamma");
	auto strings = s1.split(";");

	REQUIRE(strings.size() == 3);
	REQUIRE(strings[1] == "alpha");
	REQUIRE(strings[2] == "beta");

	REQUIRE(strings.contains("gamma"));

	String s2(String::join(strings, "::"));

	REQUIRE(s2 == "alpha::beta::gamma");
}

TEST_CASE("Test string dictionary", "[string]")
{
	Dictionary<String> map;
	map["john"] = "smith";
	map["hello"] = "world";
	
	REQUIRE(map["hello"] == "world");
	REQUIRE(map["john"] == "smith");
	
	auto values = map.values();
	REQUIRE(values.contains("world"));
}