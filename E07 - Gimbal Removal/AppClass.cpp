#include "AppClass.h"
void Application::InitVariables(void)
{
	//init the mesh
	m_pMesh = new MyMesh();
	m_pMesh->GenerateCube(1.0f, C_WHITE);
	//m_pMesh->GenerateCone(2.0f, 5.0f, 3, C_WHITE);
}
void Application::Update(void)
{
	//Update the system so it knows how much time has passed since the last call
	m_pSystem->Update();

	//Is the arcball active?
	ArcBall();

	//Is the first person camera active?
	CameraRotation();
}
void Application::Display(void)
{
	// Clear the screen
	ClearScreen();

	matrix4 m4Projection = m_pCameraMngr->GetProjectionMatrix();
	//matrix4 m4Projection = IDENTITY_M4;	// places us inside the object -> using homogenous coords

	// Switched to member -- float fFOV = 45.0f; // adjusting this value changes how much of our object we are seeing
	//float fAspect = 1000.0f / 720.0f; // ratio between length x width of the screen

	float fAspect = m_pSystem->GetWindowWidth() / m_pSystem->GetWindowHeight(); // ratio between length x width of the screen

	float fNear = 0.0f;
	float fFar = 1000.0f;

	//matrix4 m4Projection = glm::perspective(m_fFov, fAspect, fNear, fFar);
	// promoted to member -- vector3 v3Eye = vector3(0.0f, 0.0f, 5.0f); // position
	vector3 v3Center = m_v3Eye + vector3(0.0f, 0.0f, -1.0f); // what am I looking at? (Target) --> NEEDS TO BE IN FRONT OF FACE (v3Eye plus an offset)
	vector3 v3Upward = vector3(0.0f, 1.0f, 0.0f); // what up means
	matrix4 m4View = glm::lookAt(m_v3Eye, v3Center, v3Upward);
	//m4Projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, fNear, fFar);

	/*** TWO METHODS TO SOLVING GIMBAL LOCK VIA QUATERNIONS 
		 #1 -- PERFORM QUATERNION MULTIPLICATION ON A POINT
		 #2 -- CONVERT QUATERNION TO MATRIX THEN TRANSFORMING THE POINT
			   USING AFOREMENTIONED MATRIX
	***/

	static vector3 v3Orientation = vector3(0, 0, 0); // An orientation is a STATE
											  // A rotation are the steps taken to GET TO an orientation

	// create a quat
	glm::quat qNewQuat;

	// quat needs to be fed rotation angles, so:

	//matrix4 m4OrientationX = glm::rotate(IDENTITY_M4, glm::radians(v3Orientation.x), vector3(1.0f, 0.0f, 0.0f));
	//matrix4 m4OrientationY = glm::rotate(m_m4Model, glm::radians(v3Orientation.y), vector3(0.0f, 1.0f, 0.0f));
	//matrix4 m4OrientationZ = glm::rotate(IDENTITY_M4, glm::radians(v3Orientation.z), vector3(0.0f, 0.0f, 1.0f));

	//matrix4 m4Orientation = m4OrientationX * m4OrientationY * m4OrientationZ;

	// https://glm.g-truc.net/0.9.0/api/a00184.html --> glm::radians --> build a quaternion from an angle and an axis
	/*quaternion qOrientationX = glm::angleAxis(glm::radians(m_v3Rotation.x), AXIS_X);
	quaternion qOrientationY = glm::angleAxis(glm::radians(m_v3Rotation.y), AXIS_Y);
	quaternion qOrientationZ = glm::angleAxis(glm::radians(m_v3Rotation.z), AXIS_Z);*/

	// THIS IS DOING EULER -- DELETE / COMMENT THIS OUT
	m_m4Model = glm::rotate(IDENTITY_M4, glm::radians(m_v3Rotation.x), vector3(1.0f, 0.0f, 0.0f));
	m_m4Model = glm::rotate(m_m4Model, glm::radians(m_v3Rotation.y), vector3(0.0f, 1.0f, 0.0f));
	m_m4Model = glm::rotate(m_m4Model, glm::radians(m_v3Rotation.z), vector3(0.0f, 0.0f, 1.0f));
	m_pMesh->Render(m4Projection, m4View, ToMatrix4(m_m4Model));

	// THIS IS USING QUATERNIONS -- HOWEVER 1.0f CAUSES A CONSTANT
	//m_qOrientation = m_qOrientation * glm::angleAxis(glm::radians(1.0f), vector3(1.0f));
	//m_qOrientation = m_qOrientation * v3Orientation;
	m_pMesh->Render(m4Projection, m4View, ToMatrix4(m_qOrientation));
	
	// draw a skybox
	m_pMeshMngr->AddSkyboxToRenderList();
	
	//render list call
	m_uRenderCallCount = m_pMeshMngr->Render();

	//clear the render list
	m_pMeshMngr->ClearRenderList();
	
	//draw gui
	DrawGUI();
	
	//end the current frame (internally swaps the front and back buffers)
	m_pWindow->display();
}
void Application::Release(void)
{
	SafeDelete(m_pMesh);

	//release GUI
	ShutdownGUI();
}