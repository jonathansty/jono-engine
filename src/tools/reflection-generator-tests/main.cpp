#include <fmt/core.h>

#include "fundamental_type.h"
#include "simple.h"
#include "reflection.gen.h"
#include <assert.h>



void access_reflection();

int main() {
	reflection::init_database();

	access_reflection();

}
void dump_type(reflection::type_info const* info) {
	fmt::print("Type: {}\n", info->name);
	fmt::print("Size: {}\n", info->size);
	for (auto const& m : info->fields) {
		fmt::print("\t \"{}\" : {} -> \t{{ offset= {} }}\n", m.name, m.type->name, m.offset);
	}
}

void access_reflection() {
	using namespace reflection;

	reflection::get_database().for_each_type([](reflection::type_info_handle info) {
		dump_type(info);
	});

	reflection::type_info_handle info = reflection::get_database().get<book_information>();

	object obj = object::create<book_information>();

	book_information* d = (book_information*)obj.data;
	d->number_of_pages = 543;
	fmt::print("value: {}\n", d->number_of_pages);


	uint32_t data = 123;

	// Set the data programmatically
	uint8_t* byte_ptr = (uint8_t*)obj.data;
	auto it = std::find_if(info->fields.begin(), info->fields.end(), [](field_info const& field) {
		return strcmp(field.name, "number_of_pages") == 0;
	});

	int i;
	fmt::print("String size: {}\n", sizeof(std::string));
	if(it != info->fields.end()) {
		field_info const* f = &*it;
		assert(sizeof(data) == f->type->size);

		uint8_t* dst = byte_ptr + f->offset;
		memcpy(dst, &data, sizeof(data));


		book_information* d = (book_information*)obj.data;
		fmt::print("value: {}\n", d->number_of_pages);
	}


}
