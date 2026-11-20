#ifdef _WIN32
	#define ROD_TEST_EXPORT __declspec(dllexport)
#else
	#define ROD_TEST_EXPORT
#endif

extern "C" ROD_TEST_EXPORT auto RodTestValue() -> int
{
	constexpr int testValue = 42;
	return testValue;
}
