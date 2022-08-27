#pragma once

#include "Core.h"

namespace IO
{

// File mode to indicate if a file can be read/written to.
enum class Mode
{
	Read,
	Write,
};

enum class SeekMode
{
	FromBeginning,
	FromCurrent,
	FromEnd,
};

// IFile
//
// Interface to define operations that can be executed
// on a file. Each platform it's IO implementation will return a pointer to a derived IFile object.
struct IFile
{
	virtual ~IFile() {}

	virtual bool is_binary() const = 0;

	virtual Mode get_mode() const = 0;

	virtual u32 write(void* src, u32 size) = 0;

	virtual u32 read(void* dst, u32 size) = 0;

	virtual void seek(s64 offset, SeekMode mode) = 0;

	virtual u64 tell() const = 0;
};
using IFileRef = std::shared_ptr<IFile>;

// IPlatformIO
//
// Interface defining the available operations to interact with the file system.
struct IPlatformIO
{
	virtual ~IPlatformIO() = default;

	virtual string resolve_path(string const& path) = 0;

	virtual bool create_directory(const char* path) = 0;

	virtual bool exists(const char* path) = 0;

	virtual void mount(const char* path) = 0;

	virtual IFileRef open(const char* path, Mode mode, bool binary = false) = 0;

	virtual void close(IFileRef const& file) = 0;
};
using IPlatformIORef = std::shared_ptr<IPlatformIO>;

IPlatformIORef create();

void set(IPlatformIORef io);

IPlatformIORef const& get();

} // namespace IO