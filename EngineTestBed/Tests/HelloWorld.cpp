#include "stdafx.h"
#include "HelloWorld.h"
#include "core/identifier.h"

struct Foo {
	REFLECT(Foo);

	int a;
	int b;
	int c;
};

IMPL_REFLECT(Foo)
{
	get_type()->_members["a"] = { rtti::Registry::get<int>(), offsetof(Foo, a) };
	get_type()->_members["b"] = { rtti::Registry::get<int>(), offsetof(Foo, b) };
	get_type()->_members["c"] = { rtti::Registry::get<int>(), offsetof(Foo, c) };
}

struct Bar : public Foo
{
	REFLECT(Bar);

	int x;
};

IMPL_REFLECT(Bar)
{
	get_type()->_parent = Foo::get_type();
	get_type()->_members["x"] = { rtti::Registry::get<int>(), offsetof(Bar, x) };

}


void HelloWorldGame::GameStart()
{
	_test = std::make_shared<Bitmap>(String("Resources/Pickups/coinBronze.png"));

	using namespace rtti;

	// For now register our types manually
	Registry::register_type("Foo", Foo::get_type());
	Registry::register_type("Bar", Bar::get_type());

	printf("Does Bar inherit from Foo? ... %s!\n",Bar::get_type()->inherits(Foo::get_type())? "true": "false");
	printf("Does Foo inherit from Bar? ... %s!\n",Foo::get_type()->inherits(Bar::get_type())? "true": "false");

	// Dumps out all registered types to the console
	Registry::dump_types();

	// Test out creating rtti objects
	rtti::Object dynamicInt = rtti::Object::create_with_copy<int>(5);
	assert(dynamicInt.get_type()->is_primitive());

	rtti::Object dynamicFloat = rtti::Object::create_with_copy(5.0f);
	assert(dynamicFloat.get_type()->is_primitive());

	rtti::Object podType = rtti::Object::create_with_copy(Foo{});

	printf("DynamicInt: %d\n", *dynamicInt.get<int>());
	printf("DynamicFloat: %.2f\n", *dynamicFloat.get<float>());

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
	Registry::for_each_type([](std::pair<std::string, TypeInfo*> info) {
		ImGui::Text("%s", info.first.c_str());
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
