#include "stdafx.h"
#include "EngineFiles/rtti/rtti.h"

struct Foo
{
	REFLECT(Foo);

	Foo() : a(0), b(0), c(0), my_collection() {
		printf("%s\n", __FUNCTION__);
	};
	virtual ~Foo() 
	{  
		printf("%s\n", __FUNCTION__);
		my_collection.clear();
	}
	int a;
	int b;
	int c;

	void add(int x) {
		a += x;
		b += x;
		c += x;
	}

	std::vector<int> my_collection;

};

IMPL_REFLECT(Foo)
{
	type.register_property("a", &Foo::a);
	type.register_property("b", &Foo::b);
	type.register_property("c", &Foo::c);

	type.register_property("my_collection", &Foo::my_collection);
	//type.register_function("add", &Foo::add);
}

struct Bar : public Foo
{
	REFLECT(Bar);

	Bar() : Foo(), x(0)
	{
		printf("%s\n", __FUNCTION__);
	}
	virtual ~Bar()
	{
		printf("%s\n", __FUNCTION__);
	};

	int x;
};

IMPL_REFLECT(Bar)
{
	type._parent = Foo::get_static_type();
	type.register_property("x", &Bar::x);

}

struct CustomTypeNonReflected
{
	int a;
};

struct MyOtherCrazyData : public Bar
{
	REFLECT(MyOtherCrazyData)

		MyOtherCrazyData()
	{
	}

	MyOtherCrazyData(std::string d, double t)
		: x0(t)
	{
	}

	virtual ~MyOtherCrazyData()
	{
	}

	std::string data;
	CustomTypeNonReflected unreflected;
	double x0;
};

IMPL_REFLECT(MyOtherCrazyData)
{
	type._parent = Bar::get_static_type();

	type.register_property("x0", &MyOtherCrazyData::x0);
}


struct TestClass
{
	REFLECT(TestClass);

	TestClass()
	{
	}

	virtual ~TestClass()
	{

	}

	int x;
};
IMPL_REFLECT(TestClass)
{

}




struct Adder
{
	REFLECT(Adder);

	int a = 0;

	int add(int x)
	{
		a += x;
		return a;
	}

	void add_to_a(int b)
	{
		a += b;
	}
};
IMPL_REFLECT(Adder)
{

}


void ExecuteRttiTest_BasicTypes()
{
	using namespace rtti;
	{
		rtti::Object obj = rtti::Object::create<Adder>();
		std::unique_ptr<rtti::MethodBase> method = std::make_unique<rtti::Method<decltype(&Adder::add)>>(&Adder::add);
		method->invoke(obj);
		rtti::Object v = rtti::Object::create_with_copy(15);
		method->invoke(obj, v);

		std::unique_ptr<rtti::MethodBase> add_to_a_method = std::make_unique<rtti::Method<decltype(&Adder::add_to_a)>>(&Adder::add_to_a);
		auto i = rtti::Object::create<int>(15);
		add_to_a_method->invoke(obj, i);
	}



	// For now register our types manually
	if (!Registry::is_init())
	{
		Registry::init();
	}
	Registry::register_type<Foo>();
	Registry::register_type<Bar>();
	Registry::register_type<MyOtherCrazyData>();

	printf("Does Bar inherit from Foo? ... %s!\n", Bar::get_static_type()->inherits(Foo::get_static_type()) ? "true" : "false");
	printf("Does Foo inherit from Bar? ... %s!\n", Foo::get_static_type()->inherits(Bar::get_static_type()) ? "true" : "false");

	// Dumps out all registered types to the console
	Registry::dump_types();

	auto obj = rtti::Object::create<MyOtherCrazyData>("TEST", 2.0);

	TypeInfo* fooType = rtti::Registry::get<Bar>();
	{
		rtti::Object obj = fooType->create_object();
	}
	TypeInfo* barType = rtti::Registry::get<Bar>();
	rtti::Object barObject = barType->create_object();
	//rtti::Object b = rtti::Object::create<Bar>();
	Bar* testB = barObject.get<Bar>();

	//Test out creating rtti objects
	{
		rtti::Object dynamicInt = rtti::Object::create_with_copy<int>(5);
		assert(dynamicInt.get_type()->is_primitive());

		rtti::Object dynamicFloat = rtti::Object::create_with_copy(5.0f);
		assert(dynamicFloat.get_type()->is_primitive());
		printf("DynamicInt: %d\n", *dynamicInt.get<int>());
		printf("DynamicFloat: %.2f\n", *dynamicFloat.get<float>());
	}

	rtti::Object podType = rtti::Object::create_with_copy(Foo{});
	//{
	//	rtti::Method v = podType.find_function("add");
	//	v.invoke(15);
	//}

	rtti::Object barType2 = rtti::Object::create_with_copy(Bar{});
	barType2.set_property("x", -123);
	barType2.set_property("a", 250);
	Bar* bbar = barType2.get<Bar>();
	assert(bbar->x == -123 && bbar->a == 250);


	int xValue = barType2.get_property<int>("x");
	int aValue = barType2.get_property<int>("a");
	int bValue = barType2.get_property<int>("b");
	assert(xValue == -123 && aValue == 250 && bValue == 0);

	// Example of setting properties by name
	Foo* foo = podType.get<Foo>();
	podType.set_property("a", 10);
	assert(foo->a == 10);
	podType.set_property("b", 250);
	assert(foo->b == 250);
	podType.set_property("c", 18);
	assert(foo->c == 18);
}