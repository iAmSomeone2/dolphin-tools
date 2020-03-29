//
// Created by bdavidson on 3/26/20.
//

#pragma once

#include <cstdint>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

using std::vector;

typedef vector<uint8_t> data_block;

/**
 * Class for handling Yaz0 files.
 *
 * @since 0.1
 *
 * @author Brenden Davidson
 */
class Yaz0 {
public:
    /**
     * Contructs a new Yaz0 instance.
     *
     * @param yaz0FilePath filesystem path used to located input file.
     */
    explicit Yaz0(const fs::path &yaz0FilePath);

    /**
     * Takes a big-endian int and converts it to little-endian
     *
     * @param num big-endian number
     *
     * @return little-endian version of num
     */
    static uint32_t toLittleEndian(const uint32_t &num);

    /**
     * Generates an output path for the decoded RARC file based on the name of the original
     * file and the read position in the data.
     *
     * @param srcPath path of file being decoded.
     * @param bytePosition read position in the decoded data.
     *
     * @return output path for decoded RARC file
     */
    static fs::path generateRarcPath(fs::path srcPath, int bytePosition);

    uint32_t readDoubleWordAt(int index);

    /**
     * Decodes a block of Yaz0 data.
     *
     * @param expectedSize expected size of output block.
     *
     * @return data_block containing the decoded data.
     */
    data_block decodeBlock(int expectedSize);

    /**
     * Decodes the entire contents of the Yaz0 data.
     */
    void decodeAll();

    /**
     * Retrieves size of input file.
     *
     * @return size of input file in bytes
     */
    int getInputFileSize();

    /**
     * Retrieves expected size of destination file.
     *
     * @return expected size of destination file in bytes
     */
    int getDestFileSize();
};
