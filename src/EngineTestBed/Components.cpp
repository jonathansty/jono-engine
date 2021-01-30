#include "testbed.stdafx.h"
#include "Components.h"

#include "Core/TextureResource.h"
#include "Core/ModelResource.h"
#include "Core/MaterialResource.h"
#include "Core/Material.h"


IMPL_REFLECT(SimpleMovement2D)
{
	type.bind_parent<Component>();
	type.register_property("speed", &SimpleMovement2D::_speed);
	type.register_function("reset", &SimpleMovement2D::reset);

}

IMPL_REFLECT(SimpleMovement3D) {
	type.bind_parent<Component>();
	type.register_property("speed", &SimpleMovement3D::_speed);
}


IMPL_REFLECT(BitmapComponent)
{
	type.bind_parent<Component>();
}

IMPL_REFLECT(CameraComponent)
{
	type.bind_parent<Component>();

	type.register_property<CameraComponent,float>("FOV", 
		[](CameraComponent* comp, float const* fov) 
		{
			comp->_fov = *fov;
		}, 
		[](CameraComponent* comp, float** out) 
		{
			*out = &comp->_fov;
		}
	);

	type.register_property<CameraComponent, float>("NearPlane",
		[](CameraComponent* comp, float const* v)
		{
			float n = std::max(*v, 0.01f);
			comp->_near_plane = n;
		},
		[](CameraComponent* comp, float** out)
		{
			*out = &comp->_near_plane;
		}
		);

	type.register_property<CameraComponent, float>("FarPlane",
		[](CameraComponent* comp, float const* v)
		{
			float n = std::max(*v, 0.01f);
			comp->_far_plane = n;
		},
		[](CameraComponent* comp, float** out)
		{
			*out = &comp->_far_plane;
		}
		);

	type.register_property("FlySpeed", &CameraComponent::_fly_speed);
}

IMPL_REFLECT(LightComponent)
{
	type.bind_parent<Component>();
}

IMPL_REFLECT(SimpleMeshComponent)
{
	type.bind_parent<Component>();

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

	for (Meshlet const& m : _resource->_meshes)
	{
		std::shared_ptr<MaterialResource> material = _resource->_materials[m.materialID]; 
		(*material)->apply();
		ctx->DrawIndexed((UINT)m.indexCount, (UINT)m.firstIndex, (INT)m.firstVertex);
	}

}

bool SimpleMeshComponent::is_loaded() const
{
	return _resource->is_loaded();
}

void SimpleMeshComponent::set_model(std::string const& mesh)
{
	_resource = ResourceLoader::instance()->load<ModelResource>({ mesh }, false);
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
	, _fly_speed(100.0f)
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
	if (!GameEngine::instance()->is_viewport_focused() || !GameEngine::instance()->is_mouse_button_down(VK_LBUTTON))
	{
		_prev_position = GameEngine::instance()->get_mouse_pos_in_viewport();
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

		_x_angle += (float)x;
		_y_angle += (float)y;
		// Do stuff

		float x_angle = hlslpp::radians(hlslpp::float1(25.0f)) * dt * _x_angle;
		float y_angle = hlslpp::radians(hlslpp::float1(25.0f)) * dt * _y_angle;

		quaternion x_quat = hlslpp::axisangle(up.xyz, x_angle);

		right = hlslpp::mul(right, float4x4(x_quat));

		quaternion y_quat = hlslpp::axisangle(right.xyz, y_angle);
		quaternion transform_result = x_quat* y_quat;
		ent->set_rotation(transform_result);
	}


}

