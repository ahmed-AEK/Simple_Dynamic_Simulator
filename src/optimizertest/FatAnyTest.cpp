

TEST(testFatAny, testConstructor)
{
	opt::FatAny obj;
	opt::FatAny obj2{ int{1} };

	EXPECT_EQ(obj.IsEmpty(), true);
	EXPECT_EQ(obj.contains<int>(), false);
	EXPECT_EQ(obj.contains<int*>(), false);
	EXPECT_EQ(obj2.contains<int>(), true);
}

TEST(testFatAny, testget)
{
	opt::FatAny obj1{ int{1} };
	opt::FatAny obj2{ float{1} };

	EXPECT_EQ(obj1.contains<int>(), true);
	EXPECT_EQ(obj1.get<int>(), 1);
	EXPECT_EQ(obj2.contains<float>(), true);
	EXPECT_EQ(obj2.contains<float>(), float{1});
}

TEST(testFatAny, testmove)
{
	opt::FatAny obj1{ int{1} };
	opt::FatAny obj2{ float{1} };
	opt::FatAny obj3{ std::move(obj1) };
	opt::FatAny obj4;
	obj4 = std::move(obj2);

	EXPECT_EQ(obj3.contains<int>(), true);
	EXPECT_EQ(obj3.get<int>(), 1);
	EXPECT_EQ(obj4.contains<float>(), true);
	EXPECT_EQ(obj4.contains<float>(), float{ 1 });
}

TEST(testFatAny, testconst)
{
	const opt::FatAny obj1{ int{1} };

	EXPECT_EQ(obj1.contains<int>(), true);
	EXPECT_EQ(obj1.get<int>(), 1);
}

TEST(testFatAny, teststores_shared_ptr)
{
	opt::FatAny obj1{ std::make_shared<int>(1) };
	

	EXPECT_EQ(obj1.contains<std::shared_ptr<int>>(), true);
	EXPECT_EQ(*obj1.get<std::shared_ptr<int>>(), 1);
}

TEST(testFatAny, teststores_two_double)
{
	struct twoDouble
	{
		double a, b;
		bool operator==(const twoDouble& other) const { return a == other.a && b == other.b; }
	};
	opt::FatAny obj1{ twoDouble{1,2} };
	twoDouble other{ 1,2 };

	EXPECT_EQ(obj1.contains<twoDouble>(), true);
	EXPECT_EQ(obj1.get<twoDouble>(), other);
}

TEST(testFatAny, testdestructor)
{
	int called = 0;
	struct increment_on_destruct
	{
		int* value;
		explicit increment_on_destruct(int* value) : value(value) {}
		increment_on_destruct(increment_on_destruct&& other) noexcept : 
			value(std::exchange(other.value, nullptr)) { }
		~increment_on_destruct() { if (value) *value += 1; }
	};

	{
		opt::FatAny obj1{ increment_on_destruct{&called} };
	}

	EXPECT_EQ(called, 1);
}