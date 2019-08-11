
// use the fbxsdk.h
#include "BaseWrapper.h"

namespace qg {
	class QuadFace {
		int indices[4] = { -1,-1,-1,-1 };
		float uvs[2 * 4]; // four pairs
	};
	class MeshStructure {
		vector<glm::vec3> vertPositions;
		vector<int> quadFaces;

		// Key mappping structure used for queries
		map<int, QuadFace*[]> indexFaceMap;
	};

	// to create an instance of the SDK manager
	bool InitializeSdkObjects(
		FbxManager*& pSdkManager,
		FbxScene*& pScene
	);


	// to destroy an instance of the SDK manager
	void DestroySdkObjects(
		FbxManager* pSdkManager,
		bool pExitStatus
	);

	// to save a scene to a FBX file
	bool Export(
		const char* pFilename,
		int pFileFormat
	);

	// to save a scene to a FBX file
	bool SaveScene(
		FbxManager* pSdkManager,
		FbxDocument* pScene,
		const char* pFilename,
		int pFileFormat,
		bool pEmbedMedia
	);

	// to create a basic scene
	bool CreateScene();

	// Create a marker to use a point of interest for the camera. 
	FbxNode* CreateMarker(
		FbxScene* pScene,
		char* pName
	);

	// Create a camera
	FbxNode* CreateCamera(
		FbxScene* pScene,
		char* pName
	);

	void SetCameraPointOfInterest(
		FbxNode* pCamera,
		FbxNode* pPointOfInterest
	);

	void SetMarkerDefaultPosition(
		FbxNode* pMarker
	);

	void SetCameraDefaultPosition(
		FbxNode* pCamera
	);

	// to get the root node
	const FbxNode* GetRootNode();

	// to get the root node name
	const char * GetRootNodeName();

	// create a new cube under the root node
	void CreateCube(bool pWithTexture, bool pAnimate);

	void CreateCubeDetailed(
		char* pCubeName,
		double pX,
		double pY,
		double pZ,
		int pRotateAxe,
		bool pWithTexture,
		bool pAnim
	);

	// Create a cube mesh
	FbxNode* CreateCubeMesh(FbxScene* pScene, char* pName);

	FbxNode* CreateGenMesh(FbxScene* pScene, char* pName);

	//------------------TEMP TESTS---------------------//
	//-------------------------------  MAIN  --------------------------------------//
	void testPositionRadialSpreader();

	void invokeTests();
	
}