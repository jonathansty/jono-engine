#pragma once
#include <functional>

#define REFLECT(classname) \
	static rtti::TypeInfo_Register<classname> _sType; \
	static rtti::TypeInfo* get_type() { return &_sType._info;} \
	static void reflect();


#define IMPL_REFLECT(classname) \
	rtti::TypeInfo_Register<classname> classname##::_sType = rtti::TypeInfo_Register<classname>(#classname);  \
	void classname##::reflect()

namespace rtti
{
	class TypeInfo;

	struct TypeFlags
	{
		enum Flags : uint32_t
		{
			None = 0,
			PrimitiveBit = 0x1,
		};
	};

	// Type info
	class TypeInfo final
	{
	public:

		template<typename T>
		static TypeInfo create(const char* name)
		{
			return TypeInfo(name, sizeof(T));
		}

		TypeInfo(const char* name, size_t size)
			: _name(name)
			, _size(size)
			, _members()
			, _parent(nullptr)
			, _children()
			, _flags()
		{

		}

		~TypeInfo()
		{

		}

		const char* get_name() const { return _name.c_str(); }
		std::size_t get_size() const { return _size; }

		bool is_primitive()
		{
			return (_flags & TypeFlags::PrimitiveBit) == TypeFlags::PrimitiveBit;

		}

		bool inherits(TypeInfo* type) 
		{
			TypeInfo* parent = _parent;
			while (parent != nullptr)
			{
				if (parent == type)
				{
					return true;
				}
				parent = parent->_parent;
			}
			return false;

		}
	
		struct PropertyInfo
		{
			TypeInfo* type;
			size_t offset;
		};

		TypeInfo* _parent;
		std::vector<TypeInfo*> _children;
		std::string _name;
		std::size_t _size;
		TypeFlags::Flags _flags;

		std::unordered_map<std::string, PropertyInfo> _members;

	};


	struct PrimitiveTypeResolver
	{
		template<typename T>
		static TypeInfo* get_primitive_type();

#define DECLARE_PRIMITIVE_TYPE(type)  \
		template<> \
		static TypeInfo* get_primitive_type<type>() \
		{ \
			static TypeInfo _staticInt = TypeInfo(#type, sizeof(type)); \
			_staticInt._flags = TypeFlags::Flags(int(TypeFlags::PrimitiveBit) | int(_staticInt._flags)); \
			return &_staticInt; \
		}

		DECLARE_PRIMITIVE_TYPE(int);
		DECLARE_PRIMITIVE_TYPE(unsigned int);
		DECLARE_PRIMITIVE_TYPE(float);
		DECLARE_PRIMITIVE_TYPE(double);
		DECLARE_PRIMITIVE_TYPE(short);
		DECLARE_PRIMITIVE_TYPE(std::string);

	};

	struct TypeResolver
	{
		template<typename T>
		static TypeInfo* get()
		{
			return T::get_type();
		}

		template<>
		static TypeInfo* get<int>() { return PrimitiveTypeResolver::get_primitive_type<int>(); }

		template<>
		static TypeInfo* get<unsigned int>() { return PrimitiveTypeResolver::get_primitive_type<unsigned int>(); }

		template<>
		static TypeInfo* get<short>() { return PrimitiveTypeResolver::get_primitive_type<short>(); }

		template<>
		static TypeInfo* get<float>() { return PrimitiveTypeResolver::get_primitive_type<float>(); }

		template<>
		static TypeInfo* get<double>() { return PrimitiveTypeResolver::get_primitive_type<double>(); }

		template<>
		static TypeInfo* get<std::string>() { return PrimitiveTypeResolver::get_primitive_type<std::string>(); }

	};

	// Type database
	class Registry final
	{
	public:

		static void init()
		{
			assert(!_init);
			_init = true;

			register_type("int", TypeResolver::get<int>());
			register_type("unsigned int", TypeResolver::get<unsigned int>());
			register_type("float", TypeResolver::get<float>());
			register_type("double", TypeResolver::get<double>());
			register_type("short", TypeResolver::get<short>());
			register_type("std::string", TypeResolver::get<std::string>());

		}

		template<typename T>
		static TypeInfo* get()
		{
			if (!_init)
			{
				init();
			}
			return TypeResolver::get<T>();
		}

		static void register_type(const char* name, TypeInfo* info)
		{
			_types[name] = info;
		}

		static void for_each_type(std::function<void(std::pair<std::string, TypeInfo*> const& pair)> func)
		{
			for (auto it = _types.begin(); it != _types.end(); ++it)
			{
				func(*it);
			}
		}

#ifdef _DEBUG
		static void dump_types()
		{
			for_each_type([](auto it)
				{
					printf("------------------\n");
					printf("Key: %s\n", it.first.c_str());
					printf("  Type Name: %s\n", it.second->get_name());
					printf("  Size: %zd\n", it.second->get_size());
					printf("------------------\n");
			});
		}
#endif
	private:
		static bool _init;
		static std::map<std::string, TypeInfo*> _types;

	};

	__declspec(selectany) bool Registry::_init = false;

	__declspec(selectany) std::map<std::string, TypeInfo*> Registry::_types;

	template<typename T>
	struct TypeInfo_Register
	{
		TypeInfo_Register(const char* name)
			: _info(rtti::TypeInfo::create<T>(name))
		{
			T::reflect();
		}

		TypeInfo _info;
	};


	// RTTI Object containing the actual object data and type info related to the object
	class Object
	{
		public:

			template<typename T>
			static Object create_with_copy(T obj)
			{
				T* d = new T();
				memcpy(d, (const void*)(&obj), sizeof(T));
				return Object(d, Registry::get<T>());
			}

			template<typename T>
			T* get() {
				if (Registry::get<T>() == _type)
				{
					return reinterpret_cast<T*>(_data);
				}
				return nullptr;
			}

			template<typename T>
			bool set_property(std::string const& field, T const& value)
			{
				assert(_type->_members.find(field) != _type->_members.end());

				TypeInfo::PropertyInfo property_info = _type->_members[field];
				TypeInfo* property_type = property_info.type;

				if (property_type != Registry::get<T>())
					return false;

				size_t offset = property_info.offset;
				size_t size = property_type->get_size();

				uint8_t* dst = (uint8_t*)_data;
				memcpy(dst + offset, &value, size);

				return true;
			}

			~Object()
			{
				delete _data;
			}

			TypeInfo* get_type() const 
			{
				return _type;
			}

		private:
			Object(void* data, TypeInfo* type)
				: _data(data)
				, _type(type)
			{

			}

			TypeInfo* _type;
			void* _data;
	};


}