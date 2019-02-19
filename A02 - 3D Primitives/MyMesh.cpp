#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	vector3 v3Next = vector3(0, a_fRadius, 0);
	for (int i = 0; i < a_nSubdivisions + 1; i++)
	{
		double rad = (2 * PI / a_nSubdivisions) * i;
		vector3 v3Prev = vector3(sin(rad)*a_fRadius, cos(rad)*a_fRadius, 0);
		AddTri(vector3(0, 0, 0), v3Prev, v3Next);
		AddTri(v3Next, v3Prev, vector3(0, 0, a_fHeight));
		v3Next = v3Prev;
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	vector3 v3Next = vector3(0, a_fRadius, 0);
	for (int i = 0; i < a_nSubdivisions + 1; i++)
	{
		double rad = (2 * PI / a_nSubdivisions) * i;
		vector3 v3Prev = vector3(sin(rad)*a_fRadius, cos(rad)*a_fRadius, 0);
		AddQuad(v3Next, v3Prev, vector3(v3Next.x, v3Next.y, v3Next.z + a_fHeight), vector3(v3Prev.x, v3Prev.y, v3Next.z + a_fHeight)); // faces of cylinder
		AddTri(vector3(0, 0, 0), v3Prev, v3Next); // top of cylinder
		AddTri(vector3(0, 0, a_fHeight),vector3(v3Next.x, v3Next.y, a_fHeight), vector3(v3Prev.x, v3Prev.y, a_fHeight)); // bottom of cylinder
		v3Next = v3Prev;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	vector3 v3InnerNext = vector3(0, a_fInnerRadius, 0);
	vector3 v3OuterNext = vector3(0, a_fOuterRadius, 0);
	for (int i = 0; i < a_nSubdivisions + 1; i++)
	{
		double rad = (2 * PI / a_nSubdivisions) * i;
		vector3 v3InnerPrev = vector3(sin(rad)*a_fInnerRadius, cos(rad)*a_fInnerRadius, 0);
		vector3 v3OuterPrev = vector3(sin(rad)*a_fOuterRadius, cos(rad)*a_fOuterRadius, 0);

		// inner face
		AddQuad(v3InnerNext, 
				v3InnerPrev, 
				vector3(v3InnerNext.x, v3InnerNext.y, v3InnerNext.z + a_fHeight), 
				vector3(v3InnerPrev.x, v3InnerPrev.y, v3InnerPrev.z + a_fHeight)
		);

		// outer face
		AddQuad(v3OuterPrev, 
				v3OuterNext, 
				vector3(v3OuterPrev.x, v3OuterPrev.y, v3OuterPrev.z + a_fHeight), 
				vector3(v3OuterNext.x, v3OuterNext.y, v3OuterNext.z + a_fHeight)
		);

		// top face
		AddQuad(v3InnerPrev, v3InnerNext, v3OuterPrev, v3OuterNext);

		// bottom face
		AddQuad(vector3(v3InnerNext.x, v3InnerNext.y, v3InnerNext.z + a_fHeight),
				vector3(v3InnerPrev.x, v3InnerPrev.y, v3InnerPrev.z + a_fHeight),
				vector3(v3OuterNext.x, v3OuterNext.y, v3OuterNext.z + a_fHeight),
				vector3(v3OuterPrev.x, v3OuterPrev.y, v3OuterPrev.z + a_fHeight)
		);

		v3InnerNext = v3InnerPrev;
		v3OuterNext = v3OuterPrev;
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fOuterRadius * 2.0f, a_v3Color);
	// D - C
	// | \ |
	// A - B
	/** CREDIT TO WOLFRAM MATHWORLD FOR THE FORMULA FOR TORUS */
	float tRad = a_fOuterRadius / 2 + a_fInnerRadius;		// radius from the center of the hole to the center of the torus
	float cRad = (a_fOuterRadius - a_fInnerRadius) / 2;		// radius of the tube

	vector3 circVec1 = vector3(0, 0, tRad);
	vector3 circVec2 = vector3(0, 0, a_fInnerRadius);
	for (int i = 0; i < a_nSubdivisionsA + 1; i++)
	{
		double radA = (2 * PI / a_nSubdivisionsA) * i;
		vector3 outVec1 = vector3(sin(radA)*tRad, 0, cos(radA)*tRad);
		vector3 outVec2 = vector3(sin(radA)*a_fInnerRadius, 0, cos(radA)*a_fInnerRadius);

		for (int j = 0; j < a_nSubdivisionsB + 1; j++)
		{
			double radB = (2 * PI / a_nSubdivisionsB) * j;
			double radB2 = (2 * PI / a_nSubdivisionsB) * (j + 1);
			vector3 inVec1 = vector3(sin(radB)*cRad, cos(radB)*cRad, 0);
			vector3 inVec2 = vector3(sin(radB2)*cRad, cos(radB2)*cRad, 0);

			// Compute each vertice that will make up one quad of the torus
			vector3 vertexA = inVec1 + circVec1;
			vector3 vertexB = inVec2 + circVec1;
			vector3 vertexC = inVec1 + outVec1;
			vector3 vertexD = inVec2 + outVec1;

			AddQuad(vertexA, vertexB, vertexC, vertexD); // add aforementioned quad
		}
		circVec1 = outVec1; // store the previous outermost unit circle vector as the current unit circle vector
		circVec2 = outVec2;
	}

	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	//GenerateCube(a_fRadius * 2.0f, a_v3Color);
	// Sphere made up of vector points in longitude(rows) and latitude(columns)
	vector3 circVec = vector3(0, a_fRadius, 0);
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		// angles between vertex & starting point/s
		double r1 = (2 * PI / a_nSubdivisions) * i;
		double r2 = (2 * PI / a_nSubdivisions) * (i + 1);

		vector3 nextVectorLat1 = vector3(sin(r1)*a_fRadius, cos(r1)*a_fRadius, 0);
		vector3 nextVectorLat2 = vector3(sin(r2)*a_fRadius, cos(r2)*a_fRadius, 0);

		vector3 circVecLon1 = nextVectorLat1;
		vector3 circVecLon2 = nextVectorLat2;

		for (int j = 0; j < a_nSubdivisions + 1; j++)
		{
			double r3 = (2 * PI / a_nSubdivisions) * j;
			vector3 nextVectorLon1 = vector3(sin(r3)*nextVectorLat1.x, nextVectorLat1.y, cos(r3)*nextVectorLat1.x); // inputting .x for z gets me closer to a sphere?
			vector3 nextVectorLon2 = vector3(sin(r3)*nextVectorLat2.x, nextVectorLat2.y, cos(r3)*nextVectorLat2.x);
			
			AddQuad(circVecLon1, nextVectorLon1, circVecLon2, nextVectorLon2);

			circVecLon1 = nextVectorLon1;
			circVecLon2 = nextVectorLon2;
		}
		circVec = nextVectorLat1;
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}