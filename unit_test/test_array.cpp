#include <phon/third_party/catch.hpp>
#include <phon/string.hpp>
#include <phon/array.hpp>
#include <iostream>

using namespace phonometrica;

#if 0
template<typename T>
static void print(const Array<T> &mat)
{
	std::cout << "----------------------------------------\n";
	for (int i = 1; i <= mat.nrow(); i++)
	{
		for (int j = 1; j <= mat.ncol(); j++)
		{
			std::cout << mat(i,j);
			if (j < mat.ncol()) std::cout << ", ";
		}
		std::cout << std::endl;
	}
}
#endif

TEST_CASE("Test array", "[Array]")
{
	Array<double> mat(3, 5, 0);
	REQUIRE(mat.ndim() == 2);
}

