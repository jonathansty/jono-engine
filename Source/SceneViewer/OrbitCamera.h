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

	float3 m_Center;
	float m_Zoom;
	float m_Timer;
	float m_UpTimer;
};
