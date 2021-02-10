#pragma 

#include <clang-c/Index.h>

class  Cursor final{
public:
	Cursor(CXCursor c) 
		: _cursor(c) {

	}

	~Cursor() {}

	operator CXCursor() const { return _cursor; }

	void visit_children(CXCursorVisitor fn, CXClientData data);

	CXCursorKind get_kind() const {
		return clang_getCursorKind(_cursor);
	}

	std::string get_spelling() const {
		auto cursorSpelling = clang_getCursorSpelling(_cursor);
		auto value = parser::to_string(cursorSpelling);
		clang_disposeString(cursorSpelling);
		return value;
	}

	std::string get_kind_spelling() const {
		auto spelling = clang_getCursorKindSpelling(clang_getCursorKind(_cursor));
		auto value = parser::to_string(spelling);
		clang_disposeString(spelling);
		return value;
	}


	private:
		CXCursor _cursor;
};

void Cursor::visit_children(CXCursorVisitor fn, CXClientData user_data) {

	std::vector<CXCursor> children = std::vector<CXCursor>{};
	struct ClientData {
		CXCursorVisitor fn;
		std::vector<CXCursor> children;
		CXClientData user_data;
	};
	ClientData data{};
	data.fn = fn;
	data.user_data = user_data;

	clang_visitChildren(
			_cursor, [](CXCursor c, CXCursor parent, CXClientData client_data) {
				ClientData* data = (ClientData*)client_data;
				return data->fn(c, parent, data->user_data);
			},
			&data);
}
