#include "sceneviewer.pch.h"
#include "Components.h"

#include "Core/TextureResource.h"
#include "Core/ModelResource.h"
#include "Core/MaterialResource.h"
#include "Core/Material.h"


RTTR_REGISTRATION{
	using namespace rttr;

	registration::class_<SimpleMovement2D>("SimpleMovement2D")(Versioning::version(2))
		.constructor()(rttr::policy::ctor::as_raw_ptr);

	registration::class_<SimpleMovement3D>("SimpleMovement3D")(Versioning::version(2))
		.constructor()(rttr::policy::ctor::as_raw_ptr)
		.property("Speed", &SimpleMovement3D::_speed)
		.property("Elapsed", &SimpleMovement3D::_elapsed)
		;

	registration::class_<BitmapComponent>("BitmapComponent")(Versioning::version(2))
		.constructor()(rttr::policy::ctor::as_raw_ptr);

	registration::class_<CameraComponent>("CameraComponent")(Versioning::version(2))
		.constructor()(rttr::policy::ctor::as_raw_ptr)
		.property("FoV", &CameraComponent::_fov);

	registration::class_<LightComponent>("LightComponent")(Versioning::version(2))
		.constructor()(rttr::policy::ctor::as_raw_ptr)
		.property("Intensity", &LightComponent::_intensity)
		.property("Color", &LightComponent::_color);

	registration::class_<SimpleMeshComponent>("SimpleMeshComponent")(Versioning::version(2))
		.constructor()(rttr::policy::ctor::as_raw_ptr)
		.property("ModelPath", &SimpleMeshComponent::get_model_path, &SimpleMeshComponent::set_model_path);
}

SimpleMovement2D::SimpleMovement2D(XMFLOAT2 pos, float speed) : Component()
, _speed(speed)
, _elapsed(0.0)
, _offset(float4{ pos.x, pos.y, 0.0f, 1.0f })

{

}

SimpleMovement2D::SimpleMovement2D()
		: Component()
		, _speed(0.0)
		, _elapsed(0.0)
		, _offset(float4{0.0f,0.0f,0.0f,1.0f}) {

}

SimpleMovement2D::~SimpleMovement2D()
{

}

void SimpleMovement2D::on_attach(framework::Entity* ent)
{
	__super::on_attach(ent);

	_offset = ent->get_local_position();
}

void SimpleMovement2D::update(float dt)
{
	_elapsed += dt * _speed;
	framework::Entity* ent = get_entity();
	float3 up { 0.0f,0.0f,1.0f };

	quaternion rot = ent->get_rotation();
	quaternion added = hlslpp::axisangle(up, hlslpp::radians(hlslpp::float1(dt * _speed)));
	rot *= added;
	ent->set_rotation(rot);
	//ent->set_local_position(_offset.x + cos(_elapsed) * 100.0, _offset.y + sin(_elapsed) * 100.0);
}




SimpleMeshComponent::SimpleMeshComponent()
{

}

SimpleMeshComponent::~SimpleMeshComponent()
{

}

void SimpleMeshComponent::on_attach(Entity* ent)
{
	__super::on_attach(ent);
}

void SimpleMeshComponent::on_detach(Entity* ent)
{
	__super::on_detach(ent);
}

void SimpleMeshComponent::render()
{
	auto ctx = GameEngine::instance()->GetD3DDeviceContext();

	// Draw mesh
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->IASetIndexBuffer(_resource->_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	UINT strides = { sizeof(ModelResource::VertexType) };
	UINT offsets = { 0 };
	ctx->IASetVertexBuffers(0, 1, _resource->_vert_buffer.GetAddressOf(), &strides, &offsets);

	for (Mesh const& m : _resource->_meshes)
	{
		std::shared_ptr<MaterialResource> material = _resource->_materials[m.materialID]; 
		(*material)->apply();
		ctx->DrawIndexed((UINT)m.indexCount, (UINT)m.firstIndex, (INT)m.firstVertex);
	}

}

bool SimpleMeshComponent::is_loaded() const
{
	return _resource && _resource->is_loaded();
}

void SimpleMeshComponent::set_model_path(std::string mesh)
{
	_resource = ResourceLoader::instance()->load<ModelResource>({ mesh }, false);
}

std::string SimpleMeshComponent::get_model_path() {
	if(_resource) {
		return _resource->get_init_parameters().path;
	} else {
		return "";
	}
}

const float CameraComponent::DEFAULT_FOV = 45.0f;

CameraComponent::CameraComponent()
	: framework::Component()
	, _fov(DEFAULT_FOV)
	, _near_plane(0.5f)
	, _far_plane(1200.0f)
	, _prev_position(float2{ 0.0,0.0 })
	, _x_angle(0.0f)
	, _y_angle(0.0f)
	, _fly_speed(16.0f)
{

}

CameraComponent::~CameraComponent()
{

}

void CameraComponent::look_at(float3 eye, float3 target, float3 up /*= { 0.0,0.0f,1.0f }*/)
{
	// forward 
	float3 t_forward = hlslpp::normalize(target - eye);
	float3 t_right = hlslpp::cross(t_forward, up);
	float3 t_up = hlslpp::cross(t_right, t_forward);

	float4x4 matrix = hlslpp::float4x4(float4(t_right, 0.0f), float4(t_up, 0.0f), float4(t_right, 0.0f), float4(0.0f));

	this->get_entity()->set_local_position(eye);

	quaternion quat = hlslpp::euler(float3{ 0.0f, 0.0f, 0.0f});
	this->get_entity()->set_rotation(quat);
	//XMVECTOR quat = XMQuaternionRotationMatrix(XMLoadFloat4x4(&matrix));
	//this->get_entity()->set_rotation(quat);
}

void CameraComponent::update(float dt)
{

	// Ignore any input if ImGui is focused
	bool bSkip = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	if (bSkip || !GameEngine::instance()->is_viewport_focused() || !GameEngine::instance()->is_mouse_button_down(VK_LBUTTON))
	{
		_prev_position = GameEngine::instance()->get_mouse_pos_in_viewport();
		return;
	}

	if (GameEngine::instance()->is_key_pressed('R')) {
		get_entity()->set_local_position(float3{0, 0, 0});
		get_entity()->set_rotation(quaternion::identity());
		return;
	}

	float4 right   = { 1.0f, 0.0f, 0.0f, 0.0f };
	float4 up	   = { 0.0f, 1.0f, 0.0f, 0.0f };
	float4 forward = { 0.0f, 0.0f, 1.0f, 0.0f };

	float4x4 world = get_entity()->get_local_transform();

	// Update forward to be the world forward
	forward = hlslpp::mul(forward, world);
	right   = hlslpp::mul(right, world);

	Entity* ent = get_entity();
	float4 movement = float4(0.0f);
	float fly_speed = _fly_speed;

	if (GameEngine::instance()->is_key_down(VK_LSHIFT)) {
		fly_speed *= 1.5f;
	}


	if (GameEngine::instance()->is_key_down('W')) {
		movement += movement + forward * fly_speed * dt;
	}

	if (GameEngine::instance()->is_key_down('S')) {
		movement += movement + forward * -fly_speed * dt;
	}

	if (GameEngine::instance()->is_key_down('A')) {
		movement += right * -fly_speed * dt;
	}

	if (GameEngine::instance()->is_key_down('D')) {
		movement += right * fly_speed * dt;
	}

	if (GameEngine::instance()->is_key_down(VK_SPACE)) {
		movement += up * (fly_speed * 0.5f) * dt;
	}

	if (GameEngine::instance()->is_key_down(VK_LCONTROL)) {
		movement -= up * (fly_speed * 0.5f) * dt;
	}


	float4 pos = ent->get_local_position();
	pos += movement;
	ent->set_local_position(pos);

	// Handle rotation
	{
		float2 current = GameEngine::instance()->get_mouse_pos_in_viewport();
		double x = current.x - _prev_position.x;
		double y = current.y - _prev_position.y;
		_prev_position = current;

		_x_angle -= (float)x;
		_y_angle -= (float)y;
		// Do stuff

		float x_angle = hlslpp::radians(hlslpp::float1(4.9f)) * dt * _x_angle;
		float y_angle = hlslpp::radians(hlslpp::float1(4.9f)) * dt * _y_angle;

		quaternion x_quat = hlslpp::axisangle(up.xyz, x_angle);

		// recalculate our right vector for the new position
		right = float4(hlslpp::mul(x_quat, right.xyz), 0.0);
		quaternion y_quat = hlslpp::axisangle(right.xyz, y_angle);
		quaternion transform_result = y_quat * x_quat;
		ent->set_rotation(transform_result);
	}


}

