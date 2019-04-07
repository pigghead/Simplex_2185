/*----------------------------------------------
Programmer: Justin Armstrong (jxa1762@rit.edu)
Date: 2019/04
----------------------------------------------*/

#ifndef __MYOCTANT_H_
#define __MYOCTANT_H_

#include "MyEntityManager.h"


namespace Simplex
{

class MyOctant
{
	static uint m_uOctantCount;  // number of octants in existance
	static uint m_uMaxLevel;  // maximum level an octant can go
	static uint m_uIdealEntityCount;  // how many entities can exist in an octant

	uint m_uID = 0;  // stores ID for current octant
	uint m_uLevel = 0;  // current level of the octant
	uint m_uChildren = 0;  // number of children in octant 0 or 8

	float m_fSize = 0.0f;  // size of the octant

	MeshManager* m_pMeshMngr = nullptr;
	MyEntityManager* m_pEntityMngr = nullptr;

	// Center, min, max of octant
	vector3 m_v3Center = vector3(0.0f);
	vector3 m_v3Min = vector3(0.0f);
	vector3 m_v3Max = vector3(0.0f);

	MyOctant* m_pParent = nullptr;  // storage of parent of current octant
	MyOctant* m_pChild[8];  // storage for the children in current octant

	std::vector<uint> m_EntityList;  // collection of entities in the current octant

	MyOctant* m_pRoot = nullptr;  // root octant
	std::vector<MyOctant*> m_lChild;  // nodes that contain objects

public:
	MyOctant(uint a_nMaxLevel = 2, uint a_nIdealEntityCount = 5);  //d
	MyOctant(vector3 a_v3Center, float a_fSize);  //d
	MyOctant(MyOctant const& other);  //d
	MyOctant& operator= (MyOctant const& other);  //d
	~MyOctant(void);
	void Swap(MyOctant& other);
	float GetSize(void);  //d
	vector3 GetCenterGlobal(void);  //d 
	vector3 GetMinGlobal(void);  //d
	vector3 GetMaxGlobal(void);  //d
	bool IsColliding(uint a_uRBIndex);
	void Display(uint a_nIndex, vector3 a_v3Color = C_YELLOW);
	void Display(vector3 a_v3Color = C_YELLOW);
	void DisplayLeafs(vector3 a_v3Color = C_YELLOW);
	void ClearEntityList(void);
	void Subdivide(void);
	MyOctant* GetChild(uint a_nChild);
	MyOctant* GetParent(uint a_nParent);
	bool IsLeaf(void);
	bool ContainsMoreThan(uint a_nEntities);
	void KillBranches(void);
	void ConstructTree(uint a_nMaxLevel = 3);
	void AssignIDtoEntity(void);
	uint GetOctantCount(void);

private:
	void Release(void);
	void Init(void);
	void ConstructList(void);
};  // class

}  // namespace

#endif  // __MYOCTANT_H_
