//
// Created by bdavidson on 3/26/20.
//

#include <filesystem>
#include <gtest/gtest.h>

#include <Yaz0.hpp>

namespace fs = std::filesystem;

class Yaz0Test : public ::testing::Test {
protected:
    const fs::path TEST_ARC_PATH = fs::path("./test/resources/FlagObj01.arc");
    const int EXPECTED_ARC_SIZE = 18866;
};

TEST_F(Yaz0Test, OpenArcFile) {
    Yaz0 yaz0 = Yaz0(TEST_ARC_PATH);
    int actualArcSize = yaz0.getInputFileSize();

    // ASSERT_EQ(actualArcSize, EXPECTED_ARC_SIZE);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}