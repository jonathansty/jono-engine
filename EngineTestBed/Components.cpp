#include "stdafx.h"
#include "Components.h"

#include "Core/TextureResource.h"
#include "Core/ModelResource.h"

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
	XMFLOAT3 up { 0.0f,0.0f,1.0f };
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
	ctx->VSSetShader(_resource->_vert_shader.Get(), nullptr, 0);
	ctx->PSSetShader(_resource->_pixel_shader.Get(), nullptr, 0);

	ctx->IASetInputLayout(_resource->_input_layout.Get());
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->IASetIndexBuffer(_resource->_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	UINT strides = { sizeof(DirectX::VertexPositionNormalColorTexture) };
	UINT offsets = { 0 };
	ctx->IASetVertexBuffers(0, 1, _resource->_vert_buffer.GetAddressOf(), &strides, &offsets);
	ctx->DrawIndexed(UINT(_resource->_index_count), 0, 0);
}

bool SimpleMeshComponent::is_loaded() const
{
	return _resource->is_loaded();
}

void SimpleMeshComponent::load(std::string const& mesh)
{
	_resource = ResourceLoader::Instance()->load<ModelResource>({ mesh });
}


const float CameraComponent::DEFAULT_FOV = 45.0f;

CameraComponent::CameraComponent()
	: framework::Component()
	, _fov(DEFAULT_FOV)
	, _near_plane(0.01f)
	, _far_plane(100.0f)
	, _prev_position({ 0.0,0.0 })
	, _x_angle(0.0f)
	, _y_angle(0.0f)
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
	if (GameEngine::Instance()->IsKeyboardKeyDown('W'))
	{
		movement += movement + forward * 10.0f * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('S'))
	{
		movement += movement + forward * -10.0f * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('A'))
	{
		movement += rght * -10.0f * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown('D'))
	{
		movement += rght * 10.0f * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown(VK_SPACE))
	{
		movement += up * 4.0f * dt;
	}

	if (GameEngine::Instance()->IsKeyboardKeyDown(VK_LCONTROL))
	{
		movement -= up * 4.0f * dt;
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
		//ent->set_rotation(XMQuaternionMultiply(quat,y_quat));

	}


}

