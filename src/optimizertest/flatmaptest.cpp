

TEST(testFlatMap, testcopy_to)
{
    opt::FlatMap map(1);
    map.modify(0,1);
    opt::FlatMap map2(1);
    map2.modify(0,2);
    map.copy_to(map2);
    EXPECT_EQ(map.get(0), map2.get(0));
}

TEST(testFlatMap, testcopy)
{
    opt::FlatMap map(1);
    map.modify(0,1);
    opt::FlatMap map2{ map };
    opt::FlatMap map3;
    opt::FlatMap::sync(map, map3);
    EXPECT_EQ(map.get(0), map2.get(0));
    EXPECT_EQ(map.get(0), map3.get(0));
}

TEST(testFlatMap, testAdd)
{
    opt::FlatMap map(4);
    map.modify(0,1);
    map.modify(1,2);
    map.modify(2,3);
    map.modify(3,0.5);

    std::vector<std::pair<int64_t, double>> result_vector{ {0,1},{1,2},{2,3}, {3, 0.5} };
    auto data = map.data();

    for (size_t i = 0; i < data.size(); ++i) {
        const auto val = std::pair<int64_t, double>{ i, data[i] };
        EXPECT_EQ(val, result_vector[i]) << "Vectors x and y differ at index " << i;
    }
}

TEST(testFlatMap, testGet)
{
    opt::FlatMap map(4);
    map.modify(0,1);
    map.modify(1,2);
    map.modify(2,3);
    map.modify(3,0.5);

    auto val1 = map.get(0);
    auto val2 = map.get(1);
    auto val3 = map.get(2);
    auto val4 = map.get(3);

    EXPECT_EQ(val1, 1);
    EXPECT_EQ(val2, 2);
    EXPECT_EQ(val3, 3);
    EXPECT_EQ(val4, 0.5);
}
