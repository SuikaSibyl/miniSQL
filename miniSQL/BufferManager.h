#ifndef MINISQL_BUFFERMANAGER_H
#define MINISQL_BUFFERMANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <map>
#include <utility>
#include "DataStructure.h"

using namespace std;
using namespace MINISQL_BASE;

struct Block {
    string filename;
    int id; // id in buffer list
    unsigned int blockID; // block offset
    bool dirty; // whether need to write to disk
    bool busy; // whether is free or not
    int LRUCount;
    char content[BlockSize];

    Block() { reset(); }

    void reset() {
        dirty = busy = false;
        memset(content, 0, BlockSize);
    }

    void mark(string filename, unsigned int blockID) {
        this->filename = filename;
        this->blockID = blockID;
    }

    Block &flush() {
        fstream fp;
        fp.open(filename, ios::in | ios::out | ios::ate | ios::binary);
        fp.seekg(blockID * BlockSize, ios::beg);
//        cout << "Flushing: " << content << endl;
        fp.write(content, BlockSize);
        fp.close();
        return *this;
    }
};


class BufferManager {
public:
    BufferManager();

    ~BufferManager() = default;

    int getBlockTail(string filename);

    void setDirty(const string &filename, unsigned int blockID);

    char *getBlock(string filename, unsigned int blockID, bool allocate = false);

    void flushAllBlocks(); // write all content in block to disk

    void createFile(string);

    Block &getLRU();

    void removeFile(string filename);

    void setFree(string filename, unsigned int blockID);

private:
    typedef map<pair<string, unsigned int>, Block &> TypeBlockMap;

    TypeBlockMap blockMap;

    std::vector<Block> blockBuffer;

    void setBusy(int id); // mark as using

    // find block id which is available
    Block &getFreeBlock();

    Block &findBlockPair(string filename, unsigned int blockID) const;

    int maxLRU;
};

#endif //MINISQL_BUFFERMANAGER_H
