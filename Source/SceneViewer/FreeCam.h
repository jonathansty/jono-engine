#pragma once

class FreeCam
{
public:
	FreeCam(shared_ptr<RenderWorld> const& world);

	~FreeCam();

	void tick(double deltaTime);

private:
	shared_ptr<RenderWorld> _world;

	float3 _position;

	f32 _x_rotation;
	f32 _y_rotation;
	quaternion _rotation;
};
