#include "RmlUi_File_WiiU.h"
#include <cstdio>
#include <sys/stat.h>
#include <whb/log.h>

FileInterface_WiiU::FileInterface_WiiU() {}

FileInterface_WiiU::~FileInterface_WiiU() {}

Rml::FileHandle FileInterface_WiiU::Open(const Rml::String& path) {
    WHBLogPrintf("Opening file: %s", path.c_str());
    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp) {
        WHBLogPrintf("Failed to open file: %s", path.c_str());
        return 0;
    }
    return (Rml::FileHandle)fp;
}

void FileInterface_WiiU::Close(Rml::FileHandle file) {
    fclose((FILE*)file);
}

size_t FileInterface_WiiU::Read(void* buffer, size_t size, Rml::FileHandle file) {
    return fread(buffer, 1, size, (FILE*)file);
}

bool FileInterface_WiiU::Seek(Rml::FileHandle file, long offset, int origin) {
    return fseek((FILE*)file, offset, origin) == 0;
}

size_t FileInterface_WiiU::Tell(Rml::FileHandle file) {
    return ftell((FILE*)file);
}

size_t FileInterface_WiiU::Length(Rml::FileHandle file) {
    FILE* fp = (FILE*)file;
    long current = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, current, SEEK_SET);
    return length;
}
