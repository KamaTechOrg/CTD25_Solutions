#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/Img.h"
#include "../src/Board.h"
#include "../src/Command.h"

TEST_CASE("Img loads and resizes") {
    Img img;
    CHECK_THROWS(img.read("no_such_file.png"));

    // use a small dummy image generated on the fly
    cv::Mat dummy(10,20,CV_8UC3, cv::Scalar(10,20,30));
    cv::imwrite("tmp.png", dummy);

    img.read("tmp.png");
    CHECK(img.loaded());
    CHECK(img.mat().rows == 10);
    CHECK(img.mat().cols == 20);

    img.read("tmp.png", {5,5}, false);
    CHECK(img.mat().rows == 5);
    CHECK(img.mat().cols == 5);
}

TEST_CASE("Command stores parameters") {
    Command cmd{123, "PW_(6,2)", "Move", {Cell{6,2}, Cell{4,2}}};
    CHECK_EQ(cmd.type, "Move");
    CHECK(cmd.params.size()==2);
}

TEST_CASE("Board clones underlying pixels") {
    Img a; a.read("tmp.png");
    Board b{70,70,8,8,a};
    auto copy = b.clone();
    CHECK(copy.img.mat().data != b.img.mat().data);     // deep copy
}
