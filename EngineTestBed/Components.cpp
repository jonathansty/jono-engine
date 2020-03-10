#include "stdafx.h"
#include "Components.h"

#include "Core/TextureResource.h"
#include "Core/ModelResource.h"
#include "Core/MaterialResource.h"

IMPL_REFLECT(SimpleMovement)
{
	type.bind_parent<Component>();
	type.register_property("speed", &SimpleMovement::_speed);
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



SimpleMovement::SimpleMovement(XMFLOAT2 pos, float speed) : Component()
, _speed(speed)
, _elapsed(0.0)
, _offset({ pos.x, pos.y, 0.0f })

{

}

SimpleMovement::SimpleMovement() : Component()
{

}

SimpleMovement::~SimpleMovement()
{

}

void SimpleMovement::on_attach(framework::Entity* ent)
{
	__super::on_attach(ent);

	_offset = ent->get_local_position();
}

void SimpleMovement::update(float dt)
{
	_elapsed += dt * _speed;
	framework::Entity* ent = get_entity();
	XMFLOAT3 up { 0.0f,1.0f,0.0f };
	XMVECTOR rot = XMQuaternionRotationAxis(XMLoadFloat3(&up), XMConvertToRadians(_elapsed));
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

void SimpleMeshComponent::update(float dt)
{

}


void SimpleMeshComponent::render()
{
	auto ctx = GameEngine::Instance()->GetD3DDeviceContext();

	// Draw mesh

	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->IASetIndexBuffer(_resource->_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	UINT strides = { sizeof(ModelResource::VertexType) };
	UINT offsets = { 0 };
	ctx->IASetVertexBuffers(0, 1, _resource->_vert_buffer.GetAddressOf(), &strides, &offsets);

	for (Meshlet const& m : _resource->_meshes)
	{
		{
			_resource->_materials[m.materialID]->apply(ctx);
		}
		ctx->DrawIndexed(m.indexCount, m.firstIndex, m.firstVertex);
	}

}

bool SimpleMeshComponent::is_loaded() const
{
	return _resource->is_loaded();
}

void SimpleMeshComponent::load(std::string const& mesh)
{
	_resource = ResourceLoader::Instance()->load<ModelResource>({ mesh }, true);
}


const float CameraComponent::DEFAULT_FOV = 45.0f;

CameraComponent::CameraComponent()
	: framework::Component()
	, _fov(DEFAULT_FOV)
	, _near_plane(0.5f)
	, _far_plane(1200.0f)
	, _prev_position({ 0.0,0.0 })
	, _x_angle(0.0f)
	, _y_angle(0.0f)
	, _fly_speed(100.0f)
{

}

CameraComponent::~CameraComponent()
{

}

void CameraComponent::look_at(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up /*= { 0.0,0.0f,1.0f }*/)
{
	// forward 
	XMVECTOR t_forward = XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&eye));
	XMVECTOR t_right = XMVector3Cross(t_forward, XMLoadFloat3(&up));
	XMVECTOR t_up = XMVector3Cross(t_right, t_forward);

	XMFLOAT4X4 matrix{};
	XMStoreFloat4((XMFLOAT4*)&matrix._11, t_right);
	XMStoreFloat4((XMFLOAT4*)&matrix._21, t_up);
	XMStoreFloat4((XMFLOAT4*)&matrix._31, t_forward);
	matrix._44 = 1.0f;

	this->get_entity()->set_local_position(eye);

	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(0, 0, 0);
	this->get_entity()->set_rotation(quat);
	//XMVECTOR quat = XMQuaternionRotationMatrix(XMLoadFloat4x4(&matrix));
	//this->get_entity()->set_rotation(quat);
}

void CameraComponent::update(float dt)
{
	// Ignore any input if ImGui is focused
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	{
		_prev_position = GameEngine::Instance()->GetMousePositionDOUBLE2();
		return;
	}

	XMFLOAT4 fwd = { 0.0f,0.0f, 1.0f, 0.0f };
	XMFLOAT4 right = { 1.0f,0.0f, 0.0f,0.0f };
	XMFLOAT4 v_up{ 0.0f,1.0f,0.0, 0.0f };

	XMVECTOR forward = XMLoadFloat4(&fwd);
	XMVECTOR rght = XMLoadFloat4(&right);
	XMVECTOR up = XMLoadFloat4(&v_up);

	XMMATRIX world = get_entity()->get_local_transform();

	forward = XMVector4Transform(forward, world);
	rght = XMVector4Transform(rght, world);



	Entity* ent = get_entity();
	XMVECTOR movement = XMVectorZero();
	float fly_speed = _fly_speed;

	if (GameEngine::Instance()->IsKeyboardKeyDown(VK_LSHIFT))
	{
		fly_speed *= 1.5f;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('W'))
	{
		movement += movement + forward * fly_speed * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('S'))
	{
		movement += movement + forward * -fly_speed * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('A'))
	{
		movement += rght * -fly_speed * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('D'))
	{
		movement += rght * fly_speed * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown(VK_SPACE))
	{
		movement += up * (fly_speed * 0.5f) * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown(VK_LCONTROL))
	{
		movement -= up * (fly_speed * 0.5f) * dt;
	}


	XMVECTOR pos = XMLoadFloat3(&ent->get_local_position());
	pos += movement;
	XMFLOAT3 result{};
	XMStoreFloat3(&result, pos);
	ent->set_local_position(result);

	// Handle rotation
	{
		DOUBLE2 current = GameEngine::Instance()->GetMousePositionDOUBLE2();
		double x = current.x - _prev_position.x;
		double y = current.y - _prev_position.y;
		_prev_position = current;

		_x_angle += x;
		_y_angle += y;
		// Do stuff

		float x_angle = XMConvertToRadians(25.0f) * dt * _x_angle;
		float y_angle = XMConvertToRadians(25.0f) * dt * _y_angle;

		XMVECTOR x_quat = XMQuaternionRotationAxis(up, x_angle);

		XMVECTOR rght0 = XMLoadFloat4(&right);
		rght0 = XMVector4Transform(rght0, XMMatrixRotationQuaternion(x_quat));

		XMVECTOR y_quat = XMQuaternionRotationAxis(rght0, y_angle);
		XMVECTOR result = XMQuaternionMultiply(x_quat, y_quat);
		ent->set_rotation(result);
	}


}

