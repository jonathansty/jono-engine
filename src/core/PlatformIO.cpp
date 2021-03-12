#include "PlatformIO.h"

#include <filesystem>
#include <cassert>

namespace IO {

class PlatformFile final : public IPlatformFile {
public:
	PlatformFile(const char* path, FILE* s, Mode m, bool binary) 
		: _path(path)
		, _stream(s)
		, _mode(m)
		, _binary(binary)
	{
	}

	virtual ~PlatformFile() {
		fclose(_stream);
	}

	virtual bool is_binary() const { return _binary; }

	virtual Mode get_mode() const { return _mode; }

	virtual uint32_t write(void* src, uint32_t size) {
		return fwrite(src, size, 1, _stream);
	}

	virtual uint32_t read(void* dst, uint32_t size) {
		return fread(dst, size, 1, _stream);
	}

	std::string _path;
	Mode _mode;
	FILE* _stream;
	bool _binary;
};

class PlatformIO final : public IPlatformIO {
public:

	virtual bool create_directory(const char* path) override {
		std::error_code ec;
		return std::filesystem::create_directory(path, ec);
	}

	virtual bool exists(const char* path) override {
		return std::filesystem::exists(path);
	}

	virtual void mount(const char* path) override {
		_root = path;
	}

	virtual std::shared_ptr<IPlatformFile> open(const char* path, Mode mode, bool binary) override {

		// resolve path
		char tmp[512];
		sprintf(tmp, "%s/%s", _root.c_str(), path);

		if(exists(tmp) || mode == Mode::Write) {
			std::string t = "";
			switch (mode) {
				case Mode::Read:
					t = "r";
					break;
				case Mode::Write:
					t = "w";
					break;
				default:
					break;
			}

			if (binary) {
				t += "b";
			}
			FILE* s;
			fopen_s(&s, tmp, t.c_str());
			return std::make_shared<PlatformFile>(tmp, s, mode, binary);
		}

		return nullptr;
	}

	private:
		std::string _root;
};

std::shared_ptr<IPlatformIO> create() {
#if defined(WIN64)
	return std::make_shared<PlatformIO>();
#else 
	throw std::exception("Platform not supported!");
	return nullptr;
#endif
}

}