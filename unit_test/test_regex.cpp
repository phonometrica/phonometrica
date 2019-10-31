#include <phon/regex.hpp>
#include <phon/third_party/catch.hpp>

using namespace phonometrica;

TEST_CASE("Test regex", "[regex]")
{
	String pattern("^(..)llo");
	String good("hello world");
	String bad("no match here");

	Regex re(pattern);

	REQUIRE(re.match(good));
	REQUIRE(re.count() == 1);

	String result(re.capture(1));

	REQUIRE(result == "he");
	REQUIRE(not re.match(bad));
}

TEST_CASE("Replace regex", "[regex]")
{
	Regex pattern("(..)llo");
	String s("He said hello to me");

	s.replace(pattern, "bye");
	REQUIRE(s == "He said bye to me");
}

TEST_CASE("Replace capture", "[regex]")
{
	Regex pattern("(..)l(l)o");
	String s("He said hello to me");
	String s2 = s;

	s.replace(pattern, "[%2] bye [%1]");
	REQUIRE(s == "He said [l] bye [he] to me");

	s2.replace(pattern, "<i>%%</i>");
	REQUIRE(s2 == "He said <i>hello</i> to me");
}
