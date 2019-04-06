#include "MyOctant.h"
using namespace Simplex;

void MyOctant::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_uMaxLevel = 3;
	m_uIdealEntityCount = 5;

	m_uID = m_uOctantCount;  // initial octant ID
	m_uLevel = 0;  // initial level of the octant
	m_uChildren = 0;  // number of children the current octant has

	m_fSize = 5.0f;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pParent = nullptr;
	for (size_t i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
 	}

	//m_EntityList = nullptr;

	m_pRoot = nullptr;
}

// Big 3 (ctor, dtor, copy assignment operator)
MyOctant::MyOctant(uint a_nMaxlevel, uint a_nIdealEntityCount)  // constructor (root octant)
{
	Init();
	m_uMaxLevel = a_nMaxlevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;
	m_pRoot = this;
	std::vector<vector3> ObjList;

	uint ObjCount = m_pEntityMngr->GetEntityCount();  // get the number of entities in the world and add them to the list
	for (size_t i = 0; i < ObjCount; i++)
	{
		MyEntity* TempEnt = m_pEntityMngr->GetEntity(i);
		MyRigidBody* TempRig = TempEnt->GetRigidBody();
		ObjList.push_back(TempRig->GetMinGlobal);
		ObjList.push_back(TempRig->GetMaxGlobal);
	}
	MyRigidBody* RigidBodyList = new MyRigidBody(ObjList);

	vector3 v3HalfWidth = RigidBodyList->GetHalfWidth();
	float max = v3HalfWidth.x;
	for (size_t i = 0; i < 3; i++)
	{
		if (max < v3HalfWidth[i])
		{
			max = v3HalfWidth[i];
		}
	}

	vector3 v3Center = RigidBodyList->GetCenterLocal();
	// delete contents of list + rigidbody
	ObjList.clear();
	SafeDelete(RigidBodyList);

	m_fSize = max;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(max));
	m_v3Max = m_v3Center + (vector3(max));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)  // constructor (child octant)
{
	Init();
	a_v3Center = m_v3Center;
	a_fSize = m_fSize;
}

MyOctant::MyOctant(MyOctant const& other)  // copy constructor
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_uChildren = other.m_uChildren;

	m_fSize = other.m_fSize;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pParent = other.m_pParent;

	for (size_t i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}

	m_pRoot = other.m_pRoot;

	m_lChild = other.m_lChild;
}

MyOctant& MyOctant::operator=(MyOctant const& other)  // overloaded assignment operator
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

void MyOctant::Swap(MyOctant& other)
{
	
}

void MyOctant::Release(void)
{
	// TODO
}

void MyOctant::ConstructList(void)
{
	// TODO
}

float MyOctant::GetSize(void) { return m_fSize; }
vector3 MyOctant::GetCenterGlobal(void) { return m_v3Center; }
vector3 MyOctant::GetMinGlobal(void) { return m_v3Min; }
vector3 MyOctant::GetMaxGlobal(void) { return m_v3Max; }

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}

	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex);
	}
}

void MyOctant::Display(vector3 a_v3Color)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
	}
	
}

void MyOctant::Subdivide(void)
{
	if (m_uLevel >= m_uMaxLevel)  // Once we have reached as far as we can go, stop subdividing
		return;

	if (m_uChildren != 0)  // if there are children, we already subdivided
		return;

	m_uChildren = 8;  // this is the number of children yielded when subdiving a cube

	float temp_fSize = m_fSize / 4.0f;
	float temp_fSizeD = temp_fSize * 2.0f;
	vector3 temp_v3Center;

	// I drew a cube then subdivided it into 8 sections to visualize how to
	// store each section
	// MyOctant[0] : bottom front left
	temp_v3Center = m_v3Center;  // start in the center
	temp_v3Center.x -= temp_fSizeD;  // move to the left (-x)
	temp_v3Center.y -= temp_fSizeD;  // move down (-y)
	temp_v3Center.z += temp_fSizeD;  // move forward (+z)
	m_pChild[0] = new MyOctant(temp_v3Center, temp_fSizeD);

	// MyOctant[1] : bottom back left
	temp_v3Center.z -= temp_fSizeD;  // simply move backwards
	m_pChild[1] = new MyOctant(temp_v3Center, temp_fSizeD);

	// MyOctant[2] : bottom back right
	temp_v3Center.x += temp_fSizeD;  // move to the right
	m_pChild[2] = new MyOctant(temp_v3Center, temp_fSizeD);

	// MyOctant[3] : bottom front right
	temp_v3Center.z += temp_fSizeD;  // move forward
	m_pChild[3] = new MyOctant(temp_v3Center, temp_fSizeD);
	// bottom half now constructed

	// MyOctant[4] : top front right
	temp_v3Center.y += temp_fSizeD;  // move up; constructing top half now
	m_pChild[4] = new MyOctant(temp_v3Center, temp_fSizeD);

	// MyOctant[5] : top back right
	temp_v3Center.z -= temp_fSizeD;  // move backwards
	m_pChild[5] = new MyOctant(temp_v3Center, temp_fSizeD);
}

MyOctant* MyOctant::GetChild(uint a_nChild) { return *m_pChild; }
MyOctant* MyOctant::GetParent(uint a_nParent) { return m_pParent; }