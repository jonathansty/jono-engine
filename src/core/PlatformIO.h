#pragma once
#include <memory>
namespace IO {

enum class Mode {
	Invalid,
	Read,
	Write,
};

struct IPlatformFile {

	virtual bool is_binary() const = 0;

	virtual Mode get_mode() const = 0;

	virtual uint32_t write(void* src, uint32_t size) = 0;

	virtual uint32_t read(void* dst, uint32_t size) = 0;

};

struct IPlatformIO {
	virtual ~IPlatformIO() = default;

	virtual bool create_directory(const char* path) = 0;

	virtual bool exists(const char* path) = 0;

	virtual void mount(const char* path) = 0;

	virtual std::shared_ptr<IPlatformFile> open(const char* path, Mode mode = Mode::Invalid, bool binary = false) = 0;

};

using IPlatformFileRef = std::shared_ptr<IPlatformFile>;

std::shared_ptr<IPlatformIO> create();

} // namespace IO