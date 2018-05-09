#include "AppClass.h"
using namespace Simplex;
void Application::InitVariables(void)
{
	//Set the position and target of the camera
	m_pCameraMngr->SetPositionTargetAndUp(
		vector3(0.0f, 5.0f, 25.0f), //Position
		vector3(0.0f, 0.0f, 0.0f),	//Target
		AXIS_Y);					//Up

	m_pLightMngr->SetPosition(vector3(0.0f, 3.0f, 13.0f), 1); //set the position of first light (0 is reserved for ambient light)

	
	/*for (int i = 0; i < 10; i++)
	{
		m_pEntityMngr->AddEntity("Planets\\01_Mercury.obj", "Mercury_" + std::to_string(i));
		vector3 v3Position = vector3(glm::sphericalRand(12.0f));
		v3Position.y = 2.5f;
		matrix4 m4Position = glm::translate(v3Position);
		m_pEntityMngr->SetModelMatrix(m4Position);
		m_pEntityMngr->UsePhysicsSolver();
		m_pEntityMngr->SetMass(100);
	}*/

	m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Ground");
	m_pEntityMngr->SetModelMatrix(glm::translate(vector3(-50.0f, 0.0f, -50.0f)) * glm::scale(vector3(100.0f, 1.0f, 100.0f)));

	for (int i = 0; i < 60; i++)
	{
		m_pEntityMngr->AddEntity("Minecraft\\Cube.obj", "Cube_" + std::to_string(i));
		vector3 v3Position = vector3(glm::sphericalRand(40.0f));
		v3Position.y = 1.0f;
		matrix4 m4Position = glm::translate(v3Position);
		m_pEntityMngr->SetModelMatrix(m4Position);
		m_pEntityMngr->UsePhysicsSolver();
		m_pEntityMngr->SetMass(2);
	}

#pragma region Octree
	m_uOctantLevels = 1;
	m_pRoot = new MyOctant(m_uOctantLevels, 5);
#pragma endregion

}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();


	static float fTimer = 0;
	static uint uClock = m_pSystem->GenClock();		//generate a new clock for that timer
	fTimer = m_pSystem->GetDeltaTime(uClock);		//get the delta time for that timer
	int time = static_cast<int>(fTimer);

	if ((time % 60) == 0)
	{
		SafeDelete(m_pRoot);
		m_pRoot = new MyOctant(m_uOctantLevels, 5);
	}

	//Is the ArcBall active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();

	//Update Entity Manager
	m_pEntityMngr->Update();

	//Set the model matrix for the main object
	//m_pEntityMngr->SetModelMatrix(m_m4Steve, "Steve");

	//Add objects to render list
	m_pEntityMngr->AddEntityToRenderList(-1, true);
	//m_pEntityMngr->AddEntityToRenderList(-1, true);
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();

	m_pRoot->Display(m_uOctantID);
	// use quat rotation to calculate where to look
	vector3 m_v3NewLookDirection = m_qRotateCamera * -AXIS_Z;

	// use new position plus new look direction to get target
	m_v3NewTargetToLookAt = m_pCameraMngr->GetPosition() + (m_v3NewLookDirection * 6.0f);

	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();

	//draw gui,
	DrawGUI();

	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	//Release MyEntityManager
	MyEntityManager::ReleaseInstance();

	//release GUI
	ShutdownGUI();
}