#include "gtest/gtest.h"
#include "toolgui/ToolbarScene.h"
#include "toolgui/SplitterScene.h"


TEST(testToolbarScene, testConstructor)
{
    ToolbarScene object;
    auto ImageButton_vec = object.GetButtons();
    EXPECT_NE(ImageButton_vec.size(), 0);
    for (auto& item:ImageButton_vec)
    {
        EXPECT_NE(item, nullptr);
    }
}

TEST(testToolbarScene, testSetRect)
{
    int max_height = 100;

    ToolbarScene object;
    object.SetRect({0,0,800,200});
    auto ImageButton_vec = object.GetButtons();
    EXPECT_EQ(object.GetRect().h, 200);
    EXPECT_EQ(ImageButton_vec[0]->GetRect().x, 0);
    EXPECT_EQ(ImageButton_vec[0]->GetRect().y, 0);
    EXPECT_GE(ImageButton_vec[1]->GetRect().x, 
              ImageButton_vec[0]->GetRect().x + ImageButton_vec[0]->GetRect().w);
    for (auto& item: ImageButton_vec)
    {
        EXPECT_EQ(item->GetRect().h, max_height);
    }
}

TEST(testToolbarScene, testAddButton)
{
    ToolbarScene object;
    object.SetRect({0,0,800,200});
    std::string_view location = "meh";
    object.InsertButton(ImageButton(location), 0);
    EXPECT_EQ(object.GetButtons()[0]->GetPath(), location);
}
