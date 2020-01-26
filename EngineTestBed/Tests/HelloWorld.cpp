#include "stdafx.h"
#include "HelloWorld.h"
#include "core/identifier.h"
#include "rtti/rtti.h"

struct Foo 
{
	REFLECT(Foo);

	Foo() : a(0), b(0), c(0) {};
	virtual ~Foo() {  }
	int a;
	int b;
	int c;

	void add(int x) {
		a += x;
		b += x;
		c += x;
	}

};

IMPL_REFLECT(Foo)
{
	type.register_property("a", &Foo::a);
	type.register_property("b", &Foo::b);
	type.register_property("c", &Foo::c);

	//type.register_function("add", &Foo::add);
}

struct Bar : public Foo
{
	REFLECT(Bar);

	Bar() : x(0) 
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
	type._parent = Foo::get_type();
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
	type._parent = Bar::get_type();

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

template<typename ReturnType, typename ...Args>
struct FunctionInvoke
{
	ReturnType operator()(ReturnType(*func)(Args...), Args... args)
	{
		return func(args...);
	}

};

int TestFunc(int a )
{
	return a + 15;
}

void HelloWorldGame::GameStart()
{
	_test = std::make_shared<Bitmap>(String("Resources/Pickups/coinBronze.png"));

	FunctionInvoke<int, int> invoker{};
	int result = invoker(TestFunc, 15);


	using namespace rtti;

	// For now register our types manually
	Registry::init();
	Registry::register_type<Foo>();
	Registry::register_type<Bar>();
	Registry::register_type<MyOtherCrazyData>();

	printf("Does Bar inherit from Foo? ... %s!\n", Bar::get_type()->inherits(Foo::get_type()) ? "true" : "false");
	printf("Does Foo inherit from Bar? ... %s!\n", Foo::get_type()->inherits(Bar::get_type()) ? "true" : "false");

	// Dumps out all registered types to the console
	Registry::dump_types();

	auto obj = rtti::Object::create<MyOtherCrazyData>("TEST", 2.0);

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


	int* xValue = barType2.get_property<int>("x");
	int* aValue = barType2.get_property<int>("a");
	int* bValue = barType2.get_property<int>("b");
	assert(*xValue == -123 && *aValue == 250 && *bValue == 0);

	// Example of setting properties by name
	Foo* foo = podType.get<Foo>();
	podType.set_property("a", 10);
	assert(foo->a == 10);
	podType.set_property("b", 250);
	assert(foo->b == 250);
	podType.set_property("c", 18);
	assert(foo->c == 18);

}

void HelloWorldGame::GameEnd()
{

}

void HelloWorldGame::GamePaint(RECT rect)
{
	auto engine = GameEngine::Instance();
	engine->DrawSolidBackground(COLOR(0, 0, 0));
	engine->SetColor(COLOR(255, 0, 0));
	engine->DrawRect(0, 0, 100, 100);

	engine->DrawBitmap(_test.get());
}

void HelloWorldGame::DebugUI()
{
	static bool s_open = true;
	ImGui::Begin("Types", &s_open);

	using namespace rtti;
	ImGui::Columns(4);
	ImGui::Text("Key");
	ImGui::NextColumn();

	ImGui::Text("Name");
	ImGui::NextColumn();

	ImGui::Text("Size");
	ImGui::NextColumn();

	ImGui::Text("Primitive");
	ImGui::NextColumn();
	ImGui::Separator();

	// Loop over each type
	Registry::for_each_type([](std::pair<std::type_index, TypeInfo*> info) {
		ImGui::Text("%u", info.first.hash_code());
		ImGui::NextColumn();

		ImGui::Text("%s", info.second->get_name());
		ImGui::NextColumn();

		ImGui::Text("%d", info.second->get_size());
		ImGui::NextColumn();

		bool is_primive = info.second->is_primitive();
		ImGui::Checkbox("", &is_primive);
		ImGui::NextColumn();

	});

	ImGui::End();
}
