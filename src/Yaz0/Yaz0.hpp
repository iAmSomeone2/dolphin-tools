#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <filesystem>

namespace fs = std::filesystem;

using std::vector, std::queue;

typedef vector<uint8_t> data_block;

/**
 * Struct representation of a Yaz0 header.
 */
struct Yaz0Header {
    char        tag[5];
    uint32_t    uncompressedSize;
    uint32_t    reserved[2];
};

/**
 * Struct representing the header at the beginning of a compressed group.
 */
struct GroupHeader {
        uint32_t dataStart;
        uint32_t runLength;
};

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
    explicit Yaz0(const fs::path& yaz0FilePath);

    /**
     * Takes a big-endian int and converts it to little-endian
     * 
     * @param num big-endian number
     * 
     * @return little-endian version of num
     */
    static uint32_t toLittleEndian(const uint32_t& num);

    /**
     * Generates an output path for the decoded RARC file based on the name of the original
     * file and the read position in the data.
     * 
     * @param srcPath path of file being decoded.
     * @param bytePosition read position in the decoded data.
     * 
     * @return output path for decoded RARC file
     */
    fs::path generateRarcPath(fs::path srcPath, int bytePosition); 

    /**
     * Reads a 4-byte value from the source data at 'index'.
     * 
     * @note This DOES NOT modify the srcPos instance variable.
     * 
     * @param index point at where to read the value from.
     * 
     * @returns 32-bit value read at that position.
     */
    uint32_t readDoubleWordAt(const int& index);

    /**
     * Reads a 4-byte value from the source data at the current source
     * position and increments srcPos accordingly.
     * 
     * @note This DOES modify the srcPos instance variable.
     * 
     * @returns 32-bit value read at that position.
     */
    uint32_t readDoubleWord();

    /**
     * Decodes a block of Yaz0 data.
     * 
     * @param expectedSize expected size of output block.
     * 
     * @return data_block containing the decoded data.
     */
    data_block decodeBlock();

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

private:
    Yaz0Header currentHeader;
    fs::path sourcePath;
    fs::path outPath;
    vector<uint8_t> sourceData;
    uint64_t inputFileSize = 0;
    uint64_t srcPos = 0;
    uint32_t blockNum = 0;

    /**
     * Reads four bytes as a string starting at 'index' and checks if the
     * characters match 'Yaz0'.
     * 
     * @param index index to attempt to read string from.
     * 
     * @return true if the string, Yaz0, is found at that location.
     */
    bool isHeaderTag(const uint64_t& index);

    /**
     * Reads a 'code' byte and converts it to boolean queue.
     * 
     * @param codeByte byte containing copy instructions
     * 
     * @returns a queue containing the boolean values read from the 'code' byte.
     */
    static queue<bool> generateCopyInsQueue(uint8_t codeByte);

    GroupHeader generateGroupHeader(uint64_t destPos);
};