#include <fstream>
#include <iostream>
#include <cstring>
#include <string>

#include <boost/format.hpp>

#include "Yaz0.hpp"

using boost::format, std::string;

/**
 * Contructs a new Yaz0 instance.
 * 
 * @param yaz0FilePath filesystem path used to located input file.
 */
Yaz0::Yaz0(const fs::path& yaz0FilePath)
{
    this->sourcePath = yaz0FilePath;
    std::ifstream yaz0Stream = std::ifstream(yaz0FilePath, std::ios::ate | std::ios::binary);

    if (!yaz0Stream.is_open())
    {
        throw std::runtime_error("Failed to open Yaz0 file.");
    }

    this->inputFileSize = yaz0Stream.tellg();
    this->sourceData.resize(this->inputFileSize);
    yaz0Stream.seekg(std::ifstream::beg);

    yaz0Stream.read(reinterpret_cast<char *>(this->sourceData.data()), this->inputFileSize);

    yaz0Stream.close();
}

/**
 * Reads a 4-byte value from the source data at 'index'.
 * 
 * @note This does not modify the srcPos instance variable.
 * 
 * @param index point at where to read the value from.
 * 
 * @returns 32-bit value read at that position.
 */
uint32_t Yaz0::readDoubleWordAt(const int& index) {
    if (this->sourceData.size() - index < 0) {
        throw std::runtime_error("Index too near end of data.");
    }

    uint32_t byte1 = static_cast<uint32_t>(this->sourceData[index]);
    uint32_t byte2 = static_cast<uint32_t>(this->sourceData[index+1]);
    uint32_t byte3 = static_cast<uint32_t>(this->sourceData[index+2]);
    uint32_t byte4 = static_cast<uint32_t>(this->sourceData[index+3]);

    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

/**
 * Reads a 4-byte value from the source data at the current source
 * position and increments srcPos accordingly.
 * 
 * @note This DOES modify the srcPos instance variable.
 * 
 * @returns 32-bit value read at that position.
 */
uint32_t Yaz0::readDoubleWord() {
    this->srcPos += 4;
    return this->readDoubleWordAt(this->srcPos - 4);
}

GroupHeader Yaz0::generateGroupHeader(uint64_t destPos) {
    GroupHeader header;
    uint8_t byte1 = this->sourceData[this->srcPos];
    uint8_t byte2 = this->sourceData[this->srcPos+1];
    this->srcPos += 2;

    /*
        Somewhere in this method is an error that's causing the dataStart
        address to be completely wrong.
    */

    uint32_t rev = (((byte1 & 0xF) << 8) | byte2) + 1;
    header.dataStart = destPos - rev;

    if ((byte1 & 0xF0) != 0) {
        header.runLength = (byte1 >> 4) + 2;
    } else {
        header.runLength = this->sourceData[this->srcPos] + 0x12;
        this->srcPos++;
    }

    return header;
}

/**
 * Decodes a block of Yaz0 data.
 * 
 * @param expectedSize expected size of output block.
 * 
 * @return data_block containing the decoded data.
 */
data_block Yaz0::decodeBlock()
{
    std::ofstream rarcStream = std::ofstream(this->outPath, std::ios::binary);
    if (!rarcStream.is_open()) {
        std::stringstream errorMsg;
        errorMsg << format("Failed to open %s for writing.") % this->outPath.string();
        throw std::runtime_error(errorMsg.str());
    }

    data_block decodedBlock;
    decodedBlock.resize(this->currentHeader.uncompressedSize);
    uint32_t decodeIdx = 0;
    queue<bool> insQueue = Yaz0::generateCopyInsQueue(this->sourceData[this->srcPos]);
    this->srcPos++;
    
    uint64_t filePos = 0;
    while (decodeIdx < this->currentHeader.uncompressedSize)
    {
        uint32_t writtenByteCount = 0;

        if (insQueue.empty()) {
            insQueue = Yaz0::generateCopyInsQueue(this->sourceData[this->srcPos]);
            this->srcPos++;
        }

        bool insVal = insQueue.front();
        insQueue.pop();

        if(insVal) {
            // Directly copy contents
            decodedBlock[decodeIdx] = this->sourceData[this->srcPos];
            decodeIdx++;
            this->srcPos++;
            writtenByteCount = 1;
        } else {
            // Run-length encoded data
            GroupHeader groupHeader = this->generateGroupHeader(decodeIdx);

            uint64_t copyLoc = groupHeader.dataStart;
            // Copy the data run
            for (uint32_t i = 0; i < groupHeader.runLength; i++) {
                decodedBlock[decodeIdx] = decodedBlock[copyLoc];
                copyLoc++;
                decodeIdx++;
                writtenByteCount++;
            }
        }

        // Write data out to file
        rarcStream.write(reinterpret_cast<char*>(decodedBlock.data() + filePos), writtenByteCount);
        rarcStream.flush();
        filePos += writtenByteCount;
    }

    rarcStream.close();
    decodedBlock.shrink_to_fit();

    this->blockNum++;

    return decodedBlock;
}

/**
 * Decodes the entire contents of the Yaz0 data.
 */
void Yaz0::decodeAll() {
    this->srcPos = 0;

    while (this->srcPos < this->inputFileSize)
    {
        // Search for a Yaz0 block
        bool blockFound = false;
        while (this->srcPos + 3 < this->inputFileSize && !blockFound) {
            if (!this->isHeaderTag(this->srcPos)) {
                this->srcPos++;
            } else {
                this->currentHeader.tag[4] = '\0';
                strncpy(this->currentHeader.tag, reinterpret_cast<char*>(this->sourceData.data() + this->srcPos), 4);
                this->srcPos += 4;
                this->currentHeader.uncompressedSize = this->readDoubleWord();
                this->currentHeader.reserved[0] = this->readDoubleWord();
                this->currentHeader.reserved[1] = this->readDoubleWord();
                blockFound = true;
            }
        }

        if (this->srcPos + 3 >= this->inputFileSize) {
            // The end of the file has been reached.
            return;
        }

        // uint32_t size = this->sourceData.size() * 2;
        // try
        // {
        //     size = this->readDoubleWordAt(this->srcPos);
        // }
        // catch (const std::runtime_error& err)
        // {
        //     std::cerr << err.what() << std::endl;
        //     exit(-1);
        // }

        this->outPath = Yaz0::generateRarcPath(this->sourcePath, this->srcPos);

        // std::ofstream rarcStream = std::ofstream(this->outPath, std::ios::binary);

        // if (!rarcStream.is_open()) {
        //     std::stringstream errorMsg;
        //     errorMsg << format("Failed to open %s for writing.") % this->outPath.string();
        //     throw std::runtime_error(errorMsg.str());
        // }

        std::cout << format("Writing %s\n") % this->outPath.string();

        std::cout << format("Writing 0x%X bytes\n") % this->currentHeader.uncompressedSize;

        // this->srcPos += 2;

        vector<uint8_t> decodedBlock = this->decodeBlock();

        // rarcStream.write(reinterpret_cast<char*>(decodedBlock.data()), decodedBlock.size());
        // rarcStream.close();

        std::cout << format("Read 0x%X bytes from input data\n") % this->srcPos;
    }
}

/**
 * Reads four bytes as a string starting at 'index' and checks if the
 * characters match 'Yaz0'.
 * 
 * @param index index to attempt to read string from.
 * 
 * @return true if the string, Yaz0, is found at that location.
 */
bool Yaz0::isHeaderTag(const uint64_t& index) {
    char *tagTxt = new char[5];
    tagTxt[0] = this->sourceData[index];
    tagTxt[1] = this->sourceData[index+1];
    tagTxt[2] = this->sourceData[index+2];
    tagTxt[3] = this->sourceData[index+3];
    tagTxt[4] = '\0';

    int result = strncmp("Yaz0", tagTxt, 4);
    delete[] tagTxt;

    return result == 0;
}

/**
 * Reads a 'code' byte and converts it to boolean queue.
 * 
 * @param codeByte byte containing copy instructions
 * 
 * @returns a queue containing the boolean values read from the 'code' byte.
 */
queue<bool> Yaz0::generateCopyInsQueue(uint8_t codeByte) {
    queue<bool> insQueue;

    for (int i = 7; i >= 0; i--) {
        bool bit = static_cast<bool>((codeByte & 0b10000000) >> i);
        insQueue.push(bit);
        codeByte = codeByte << 1;
    }

    return insQueue;
}

/**
 * Takes a big-endian int and converts it to little-endian
 * 
 * @param num big-endian number
 * 
 * @return little-endian version of num
 */
uint32_t Yaz0::toLittleEndian(const uint32_t &num)
{
    uint32_t byte1 = static_cast<uint32_t>(num & 0xFF);
    uint32_t byte2 = static_cast<uint32_t>((num >> 8) & 0xFF);
    uint32_t byte3 = static_cast<uint32_t>((num >> 16) & 0xFF);
    uint32_t byte4 = static_cast<uint32_t>(num >> 24);

    return (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
}

/**
 * Generates an output path for the decoded RARC file based on the name of the original
 * file and the read position in the data.
 * 
 * @param srcPath path of file being decoded.
 * @param bytePosition read position in the decoded data.
 * 
 * @return output path for decoded RARC file
 */
fs::path Yaz0::generateRarcPath(fs::path srcPath, int bytePosition) {
    std::stringstream pathString;
    string fileName = srcPath.replace_extension("").filename().string();
    pathString << format("%s_%d.rarc") % fileName % this->blockNum;
    return fs::path(pathString.str());
}

int Yaz0::getInputFileSize() {
    return this->inputFileSize;
}