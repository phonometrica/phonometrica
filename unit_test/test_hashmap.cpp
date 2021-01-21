//
// Created by julien on 21/01/2021.
//

#include <phon/third_party/catch.hpp>
#include <phon/dictionary.hpp>
#include <iostream>

using namespace phonometrica;


TEST_CASE("Test Hashmap", "[Hashmap]")
{
	Dictionary<int> d;
	d.insert({"x", 1});
	d.insert({"y", 2});
	d.insert({"x", 3});
	REQUIRE(d["x"] == 3);
}

