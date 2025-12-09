#include "Backend/RmlUi_File_WiiU.h"
#include "RmlUi/Core/Types.h"
#include <cstdio>
#include <sys/stat.h>
#include <whb/log.h>

size_t VirtualFile::remain() const { return size - pos; }
void const * VirtualFile::current() const { return static_cast<char const *>(data) - pos; }
void VirtualFile::skip(size_t offset) { pos += offset; }

size_t VirtualFile::read(void* buffer, size_t size)
{
    size_t toCopy = std::min(size, remain());
    memcpy(buffer, current(), toCopy);
    skip(toCopy);
    return toCopy;
}

bool VirtualFile::seek(long offset, int origin)
{
    size_t newPos = 0;
    switch(origin)
    {
        case SEEK_SET: newPos = offset; break;
        case SEEK_CUR: newPos = static_cast<long>(pos) + offset; break;
        case SEEK_END: newPos = static_cast<long>(size) + offset; break;
        default: return false;
    }
    if(newPos > size) return false;
    pos = newPos;
    return true;
}

size_t VirtualFile::tell() const
{
    return pos;
}

FileInterface_WiiU::FileInterface_WiiU() {}
FileInterface_WiiU::~FileInterface_WiiU() {}

Rml::FileHandle FileInterface_WiiU::Open(Rml::String const & path)
{
    auto it = virtualFiles.find(path);
    if(it != virtualFiles.end())
    {
        WHBLogPrintf("Opening virtual file: %s", path.c_str());
        return reinterpret_cast<Rml::FileHandle>(&it->second);
    }
    WHBLogPrintf("Opening real file: %s", path.c_str());
    FILE* file = fopen(path.c_str(), "rb");
    if (!file)
    {
        WHBLogPrintf("Failed to open file: %s", path.c_str());
        return 0;
    }
    return reinterpret_cast<Rml::FileHandle>(file);
}

void FileInterface_WiiU::Close(Rml::FileHandle file)
{
    if (!isVirtual(file))
    {
        fclose((FILE*)file);
    }
}

size_t FileInterface_WiiU::Read(void* buffer, size_t size, Rml::FileHandle file)
{
    if (!file) return 0;
    if (isVirtual(file))
    {
        VirtualFile* vFile = reinterpret_cast<VirtualFile*>(file);
        return vFile->read(buffer, size);
    }
    return fread(buffer, 1, size, reinterpret_cast<FILE*>(file));
}

bool FileInterface_WiiU::Seek(Rml::FileHandle file, long offset, int origin)
{
    if(!file) return false;
    if(isVirtual(file)) return reinterpret_cast<VirtualFile*>(file)->seek(offset, origin);
    return fseek((FILE*)file, offset, origin) == 0;
}

size_t FileInterface_WiiU::Tell(Rml::FileHandle file)
{
    if(!file) return 0;
    if(isVirtual(file)) return reinterpret_cast<VirtualFile*>(file)->tell();
    return ftell((FILE*)file);
}

size_t FileInterface_WiiU::Length(Rml::FileHandle file)
{
    if(!file) return 0;
    if(isVirtual(file)) return reinterpret_cast<VirtualFile*>(file)->size;
    FILE* filePtr = reinterpret_cast<FILE*>(file);
    long current = ftell(filePtr);
    fseek(filePtr, 0, SEEK_END);
    long length = ftell(filePtr);
    fseek(filePtr, current, SEEK_SET);
    return length;
}

void FileInterface_WiiU::addVirtual(Rml::String const & path, void const * content, std::size_t size)
{
    virtualFiles[path] = VirtualFile{content, size, 0};
}

void FileInterface_WiiU::addVirtual(Rml::String const & path, uint8_t const * content, std::size_t size)
{
    virtualFiles[path] = VirtualFile{reinterpret_cast<void const *>(content), size, 0};
}

bool FileInterface_WiiU::isVirtual(Rml::FileHandle file)
{
    for (auto& value : virtualFiles)
    {
        if (reinterpret_cast<Rml::FileHandle>(&value.second) == file) return true;
    }
    return false;
}