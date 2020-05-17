#ifndef OBJ_H
#define OBJ_H

//-------------------------------------------------------------
//                       INCLUDES                             -
//-------------------------------------------------------------
#include <vector>
#include "model.h"

//STL vectors
using std::vector;

//No use including all the math classes here, so we will just define 2 and 3 element vectors here
struct Vector3
{
	float m_fVec[3];

	Vector3(float * fVec)
	{
		memcpy(m_fVec, fVec, sizeof(float[3]));
	}
};
struct Vector2
{
	float m_fVec[2];

	Vector2(float * fVec)
	{
		memcpy(m_fVec, fVec, sizeof(float[2]));
	}
};


//-------------------------------------------------------------
//- SObjFace
//- Indices for each face of the OBJ file
struct SObjFace
{
	unsigned int m_uiVertIdx[3];
	unsigned int m_uiTexCoordIdx[3];
	unsigned int m_uiNormalIdx[3];

	SObjFace(int * ipData)
	{
		//copy the data;
		m_uiVertIdx[0] = ipData[0]-1;
		m_uiVertIdx[1] = ipData[1]-1;
		m_uiVertIdx[2] = ipData[2]-1;

		m_uiTexCoordIdx[0] = ipData[3]-1;
		m_uiTexCoordIdx[1] = ipData[4]-1;
		m_uiTexCoordIdx[2] = ipData[5]-1;

		m_uiNormalIdx[0] = ipData[6]-1;
		m_uiNormalIdx[1] = ipData[7]-1;
		m_uiNormalIdx[2] = ipData[8]-1;
	}
};

class CObj : public CModel
{
public:

	//Load the model
	bool Load(const char * szFilename);
	//Render the model
	void Render();

	//default constructor
	CObj();
	//Load a file 
	CObj(const char * szFilename);

public:

	//The OBJ format does not tell us how many of each thing thing we have
	//so we will use STL vectors in order to have resizable arrays to accomadate
	//the exact number everything
	//an STL vector to hold vertices
	vector<Vector3> m_vVertices;
	//an STL vector to hold vertex normals
	vector<Vector3> m_vNormals;
	//an STL vector to hold the texture coordinates
	vector<Vector2> m_vTexCoords;
	//an STL vector for the faces
	vector<SObjFace> m_vFaces;

	//Pointer to objects to cut down on operator[] use with std::vector
	Vector3 * m_pVerts;
	Vector3 * m_pNormals;
	Vector2 * m_pTexCoords;
	SObjFace * m_pFaces;

	bool m_bHasTexCoords;
	bool m_bHasNormals;
};


#endif //OBJ_H