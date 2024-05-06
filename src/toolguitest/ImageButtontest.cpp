#include "gtest/gtest.h"
#include "toolgui/ImageButton.h"

TEST(testImageButton, testBind)
{
    int test_value = 1;
    ImageButton button("new_file.png");
    button.Bind([&](){test_value = 2;});
    EXPECT_EQ(test_value, 1);	
    auto evt1 = button.LMBDown(SDL_Point{0,0});
    EXPECT_EQ(test_value, 1);	
    auto evt2 = button.LMBUp(SDL_Point{0,0});
    EXPECT_EQ(test_value, 2);
    EXPECT_EQ(evt1, MI::ClickEvent::NONE);
    EXPECT_EQ(evt2, MI::ClickEvent::CLICKED);
}

TEST(testImageButton, testLoad)
{
    ImageButton button1("assets/blabla.png");
    ImageButton button2("assets/new_file.png");
    EXPECT_THROW(button1.loadimage(), std::runtime_error);
    EXPECT_NO_THROW(button2.loadimage());
}

TEST(testImageButton, testRect)
{
    ImageButton button1("assets/blabla.png");
    auto res = button1.GetRect();
    EXPECT_EQ(res.x, 0);
    EXPECT_EQ(res.y, 0);
    EXPECT_EQ(res.w, 0);
    EXPECT_EQ(res.h, 0);

    button1.SetRect({1,1,1,1});
    auto res2 = button1.GetRect();
    EXPECT_EQ(res2.x, 1);
    EXPECT_EQ(res2.y, 1);
    EXPECT_EQ(res2.w, 1);
    EXPECT_EQ(res2.h, 1);

}
