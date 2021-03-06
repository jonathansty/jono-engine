

#include "runtime/reflection.h"

/*
 * declarations 
 */
struct character_data;
struct data;
struct inventory_item;
struct enemy_data;
struct book_information;

/*
 * Templates 
 */
template<> struct reflection::helpers::type_name<character_data> { static constexpr char const* name = "character_data"; };
template<> struct reflection::helpers::type_name<data> { static constexpr char const* name = "data"; };
template<> struct reflection::helpers::type_name<float> { static constexpr char const* name = "float"; };
template<> struct reflection::helpers::type_name<bool> { static constexpr char const* name = "bool"; };
template<> struct reflection::helpers::type_name<int> { static constexpr char const* name = "int"; };
template<> struct reflection::helpers::type_name<std::string> { static constexpr char const* name = "std::string"; };
template<> struct reflection::helpers::type_name<inventory_item> { static constexpr char const* name = "inventory_item"; };
template<> struct reflection::helpers::type_name<uint32_t> { static constexpr char const* name = "uint32_t"; };
template<> struct reflection::helpers::type_name<enemy_data> { static constexpr char const* name = "enemy_data"; };
template<> struct reflection::helpers::type_name<book_information> { static constexpr char const* name = "book_information"; };
