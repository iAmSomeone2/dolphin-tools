#include <iostream>
#include <boost/format.hpp>

#include "Yaz0/Yaz0.hpp"

using boost::format;

static const fs::path TEST_FILE = fs::path("./test/resources/FlagObj01.arc");

int main(int argc, char* argv[]) {
//    if (argc < 2) {
//        std::cerr << "Usage dolphin-tools [inputFile]" << std::endl;
//        return EXIT_FAILURE;
//    }

//    fs::path yaz0Path = fs::path(argv[1]);

    Yaz0 yaz0 = Yaz0(TEST_FILE);

    int fileSize = yaz0.getInputFileSize();

    std::cout << format("Input file size: 0x%X\n") % fileSize;

    yaz0.decodeAll();

    return EXIT_SUCCESS;
}