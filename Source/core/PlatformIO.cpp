#include "core.pch.h"
#include "PlatformIO.h"


namespace IO {

#ifdef WIN64

class Win64File final : public IFile {
public:
	Win64File(const char* path, FILE* s, Mode m, bool binary) 
		: _path(path)
		, _stream(s)
		, _mode(m)
		, _binary(binary)
	{
	}

	virtual ~Win64File() {
		// When a file goes out of scope force a close
		if (_stream) {
			fclose(_stream);
			_stream = nullptr;
		}
	}

	virtual bool is_binary() const { return _binary; }

	virtual Mode get_mode() const { return _mode; }

	virtual uint32_t write(void* src, uint32_t size) {
		return static_cast<uint32_t>(fwrite(src, size, 1, _stream));
	}

	virtual u32 read(void* dst, u32 size) {
		return static_cast<u32>(fread(dst, size, 1, _stream));
	}

	virtual void seek(s64 offset, SeekMode mode) {
		int m = SEEK_CUR;
		if (mode == SeekMode::FromBeginning) {
			m = SEEK_SET;
		}
		if(mode == SeekMode::FromEnd) {
			m = SEEK_END;
		}
		fseek(_stream, long(offset), m);
	}

	virtual u64 tell() const { return ftell(_stream); }

	std::string _path;
	Mode _mode;
	FILE* _stream;
	bool _binary;
};

class Win64IO final : public IPlatformIO {
public:
	Win64IO()
			: _root(".") {
	}

	virtual bool create_directory(const char* path) override {
		std::error_code ec;
		return std::filesystem::create_directory(path, ec);
	}

	virtual bool exists(const char* path) override {
		std::string tmp = resolve_path(path);
		return std::filesystem::exists(tmp);
	}

	virtual void mount(const char* path) override {
		_root = path;
	}

	virtual std::string resolve_path(std::string const& path) override {
		// Check if path is relative
		std::filesystem::path p{ path };

		// Absolute paths just get resolved straight
		if (p.is_absolute()) {
			return path;
		}

		// Relative paths need to be resolved to the root
		if (!path.starts_with("..") && !path.starts_with('.')) {
			char tmp[512];
			sprintf_s(tmp, "%s/%s", _root.c_str(), path.c_str());
			return tmp;
		}
		return path;
	}

	virtual std::shared_ptr<IFile> open(const char* path, Mode mode, bool binary) override {

		if(exists(path) || mode == Mode::Write) {
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

			std::string tmp = resolve_path(path);
			FILE* s;
			auto err = fopen_s(&s, tmp.c_str(), t.c_str());
			if (s == nullptr) {
				char buffer[256];
				strerror_s(buffer, err);
				fmt::print("Failed to open file. Error: {}",buffer);
				return nullptr;
			}
			return std::make_shared<Win64File>(tmp.c_str(), s, mode, binary);
		}

		return nullptr;
	}

	virtual void close(IFileRef const& file) override {

		Win64File* winFile = (Win64File*)file.get();
		if(winFile->_stream) {
			fclose(winFile->_stream);
		}

		// Clear out the stream on the file
		winFile->_stream = nullptr;
	}


	private:
		std::string _root;
};

#endif

IPlatformIORef create() {
#if defined(WIN64)
	return std::make_shared<Win64IO>();
#else 
	throw std::exception("Platform not supported!");
	return nullptr;
#endif
}

static IPlatformIORef s_io;
void set(IPlatformIORef io) 
{
	s_io = io;
}

IPlatformIORef const& get() {
	// Create IO if it doesn't exist yet!
	if (!s_io) {
		s_io = create();
	}
	return s_io;
}


}