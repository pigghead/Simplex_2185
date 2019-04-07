#include "MyOctant.h"
using namespace Simplex;

// declare statics
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

void MyOctant::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_uID = m_uOctantCount;  // initial octant ID
	m_uOctantCount++;
	m_uLevel = 0;  // initial level of the octant
	m_uChildren = 0;  // number of children the current octant has

	m_fSize = 0.0f;

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
	// Initialize variables
	Init();
	m_uMaxLevel = a_nMaxlevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uOctantCount = 0;
	m_uID = m_uOctantCount;
	m_pRoot = this;  // this is the root node
	m_lChild.clear();
	std::vector<vector3> ObjList;

	uint ObjCount = m_pEntityMngr->GetEntityCount();  // get the number of entities in the world and add their min/max to the list
	for (size_t i = 0; i < ObjCount; i++)
	{
		MyEntity* TempEnt = m_pEntityMngr->GetEntity(i);
		MyRigidBody* TempRig = TempEnt->GetRigidBody();  // store this particular entity's rigidbody to add its minimums and maximums
		ObjList.push_back(TempRig->GetMinGlobal());
		ObjList.push_back(TempRig->GetMaxGlobal());
	}
	MyRigidBody* RigidBodyList = new MyRigidBody(ObjList);  // create a pointer to the list of the mins and maxes

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

	m_fSize = max * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(max));
	m_v3Max = m_v3Center + (vector3(max));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)  // constructor (child octant)
{
	Init();
	//a_v3Center = m_v3Center;
	//a_fSize = m_fSize;
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;
	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);
	m_uOctantCount++;
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
	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;

	for (size_t i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}


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

MyOctant::~MyOctant(void) { Release(); }

void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	m_pMeshMngr->GetInstance();
	m_pEntityMngr->GetInstance();

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	std::swap(m_pParent, other.m_pParent);
	for (size_t i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}

}

float MyOctant::GetSize(void) { return m_fSize; }
vector3 MyOctant::GetCenterGlobal(void) { return m_v3Center; }
vector3 MyOctant::GetMinGlobal(void) { return m_v3Min; }
vector3 MyOctant::GetMaxGlobal(void) { return m_v3Max; }

bool MyOctant::IsColliding(uint a_uRBIndex)
{
	uint ObjCount = m_pEntityMngr->GetEntityCount();  // amount of entities in existence

	if (a_uRBIndex >= ObjCount)  // if index exceeds object count, no collision
	{
		return false;
	}

	MyEntity* temp_Entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* temp_Rigidbody = temp_Entity->GetRigidBody();
	vector3 temp_v3Min = temp_Rigidbody->GetMinGlobal();
	vector3 temp_v3Max = temp_Rigidbody->GetMaxGlobal();

	// test the axes
	if (m_v3Max.x < temp_v3Min.x) return false;
	if (m_v3Min.x > temp_v3Max.x) return false;

	if (m_v3Max.y < temp_v3Min.y) return false;
	if (m_v3Min.y > temp_v3Max.y) return false;

	if (m_v3Max.z < temp_v3Min.z) return false;
	if (m_v3Min.z > temp_v3Max.z) return false;

	// if all else fails, then we are colliding
	return true;
}

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
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
	
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint LeafCount = m_lChild.size();
	for (size_t i = 0; i < LeafCount; i++)
	{
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::ClearEntityList(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
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

	// MyOctant[6] : top back left
	temp_v3Center.x += temp_fSizeD;  // move to the right
	m_pChild[6] = new MyOctant(temp_v3Center, temp_fSizeD);

	// MyOctant[7] : top front left
	temp_v3Center.z += temp_fSizeD;
	m_pChild[7] = new MyOctant(temp_v3Center, temp_fSizeD);

	for (size_t i = 0; i < 0; i++)
	{
		// assign each child its root, parent, and level
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		m_pChild[i]->m_pRoot = m_pRoot;

		// Recursively subdivide if there are more than 5 entities
		// in any given octant
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

MyOctant* MyOctant::GetChild(uint a_nChild) 
{ 
	if (a_nChild > 7)  // if we're at the end (a leaf) there are no more children
		return nullptr;
	return m_pChild[a_nChild];
}
MyOctant* MyOctant::GetParent(uint a_nParent) { return m_pParent; }

bool MyOctant::IsLeaf(void)
{
	if (m_uChildren == 0) return true;

	return false;
}

bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	uint temp_Count = 0;
	uint ObjCount = m_pEntityMngr->GetEntityCount();
	for (size_t i = 0; i < ObjCount; i++)
	{
		if (IsColliding(i) == true)
		{
			temp_Count++;
		}
		if (temp_Count > a_nEntities)
		{
			return true;
		}
	}
	return false;
}

void MyOctant::KillBranches(void)
{
	if (IsLeaf())
		return;

	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
 	}
	m_uChildren = 0;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	// only make a tree starting from the root
	if (m_uLevel != 0)
	{
		return;
	}

	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;
	m_EntityList.clear();
	KillBranches();
	m_lChild.clear();

	// If our current Octant has more objects in it than the ideal
	// entity count, subdivide the octant.
	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}
	AssignIDtoEntity();  // give the octant an ID
	ConstructList();
}

void MyOctant::AssignIDtoEntity(void) 
{
	// find node with no children
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}

	if (IsLeaf())
	{
		uint EntitiesCount = m_pEntityMngr->GetEntityCount();
		for (size_t j = 0; j < EntitiesCount; j++)
		{
			if (IsColliding(j))
			{
				m_EntityList.push_back(j);
				m_pEntityMngr->AddDimension(j, m_uID);
			}
		}
	}
}

uint MyOctant::GetOctantCount(void) { return m_uOctantCount; }

void MyOctant::Release(void)
{
	if (this == m_pRoot)
	{
		KillBranches();
	}

	// reset values to default; clear lists
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

void MyOctant::ConstructList(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}