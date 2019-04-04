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
		ObjList.push_back(TempRig->GetMinGlobal);
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
	a_v3Center = m_v3Center;
	a_fSize = m_fSize;
}

MyOctant::MyOctant(MyOctant const& other)  // copy constructor
{
	
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
MyOctant* MyOctant::GetChild(uint a_nChild) { return *m_pChild; }
MyOctant* MyOctant::GetParent(uint a_nParent) { return m_pParent; }