#pragma once

class RenderWorld;
class RenderWorldCamera;

class OrbitCamera
{
public:
	OrbitCamera(shared_ptr<RenderWorld> const& world);

	~OrbitCamera();

	void Tick(double deltaTime);

private:
	shared_ptr<RenderWorld> m_World;

	float3 _center;
	float _zoom;
	float _timer;
	float _up_timer;
};
