#ifndef RMLUI_BACKENDS_FILE_WIIU_H
#define RMLUI_BACKENDS_FILE_WIIU_H

#include "RmlUi/Config/Config.h"
#include "RmlUi/Core/Types.h"
#include <RmlUi/Core/FileInterface.h>
#include <unordered_map>
#include <utility>

struct VirtualFile
{
    void const * data;
    size_t size;
    size_t pos;

    size_t remain() const;
    void const * current() const;
    void skip(size_t offset);
    size_t read(void* buffer, size_t size);
    bool seek(long offset, int origin);
    size_t tell() const;
};

class FileInterface_WiiU : public Rml::FileInterface {
public:
    FileInterface_WiiU();
    virtual ~FileInterface_WiiU();

    Rml::FileHandle Open(Rml::String const & path) override;
    void Close(Rml::FileHandle file) override;
    size_t Read(void * buffer, size_t size, Rml::FileHandle file) override;
    bool Seek(Rml::FileHandle file, long offset, int origin) override;
    size_t Tell(Rml::FileHandle file) override;
    size_t Length(Rml::FileHandle file) override;

    void addVirtual(Rml::String const & path, void const * content, std::size_t size);
    void addVirtual(Rml::String const & path, uint8_t const * content, std::size_t size);

private:
    std::unordered_map<Rml::String, VirtualFile> virtualFiles; // <path, content>
    bool isVirtual(Rml::FileHandle file);
};

#endif
