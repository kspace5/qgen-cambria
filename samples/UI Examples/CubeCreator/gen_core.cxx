#include "gen_core.h"

namespace qg {
	// declare global
	FbxManager*   gSdkManager = NULL;
	FbxScene*        gScene = NULL;
	FbxFileTexture*  gTexture = NULL;
	FbxSurfacePhong* gMaterial = NULL;

	int    gCubeNumber = 1;     // Cube Number
	int    gCubeRotationAxis = 1;     // Cube Rotation Axis 0==X, 1==Y, 2==Z
	double gCubeXPos = 0.0;   // initial CubXPos
	double gCubeYPos = 20.0;  // initial CubeYPos
	double gCubeZPos = 0.0;   // initial CubeZPos

	FbxAnimLayer* gAnimLayer = NULL;  // holder of animation curves
	FbxString* gAppPath = NULL;     // path where the application started


#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif

	bool InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
	{
		//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
		pManager = FbxManager::Create();
		if (!pManager)
		{
			FBXSDK_printf("Error: Unable to create FBX Manager!\n");
			exit(1);
		}
		else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

		//Create an IOSettings object. This object holds all import/export settings.
		FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
		pManager->SetIOSettings(ios);

		//Create an FBX scene. This object holds most objects imported/exported from/to files.
		pScene = FbxScene::Create(pManager, "My Scene");
		if (!pScene)
		{
			FBXSDK_printf("Error: Unable to create FBX scene!\n");
			exit(1);
		}
		return true;
	}

	void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
	{
		//Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
		if (pManager) pManager->Destroy();
		if (pExitStatus) FBXSDK_printf("Program Success!\n");
	}

	// to create a basic scene
	bool CreateScene()
	{
		// Initialize the FbxManager and the FbxScene
		if (InitializeSdkObjects(gSdkManager, gScene) == false)
		{
			return false;
		}

		// create a marker
		FbxNode* lMarker = CreateMarker(gScene, "Marker");

		// create a camera
		FbxNode* lCamera = CreateCamera(gScene, "Camera");

		// set the camera point of interest on the marker
		SetCameraPointOfInterest(lCamera, lMarker);

		// set the marker position
		SetMarkerDefaultPosition(lMarker);

		// set the camera position
		SetCameraDefaultPosition(lCamera);

		// build a minimum scene graph
		FbxNode* lRootNode = gScene->GetRootNode();
		lRootNode->AddChild(lMarker);
		lRootNode->AddChild(lCamera);

		// set camera switcher as the default camera
		gScene->GetGlobalSettings().SetDefaultCamera((char *)lCamera->GetName());

		return true;
	}


	// Create a marker to use a point of interest for the camera. 
	FbxNode* CreateMarker(FbxScene* pScene, char* pName)
	{
		FbxMarker* lMarker = FbxMarker::Create(pScene, pName);

		FbxNode* lNode = FbxNode::Create(pScene, pName);

		lNode->SetNodeAttribute(lMarker);

		return lNode;
	}

	// Create a camera.
	FbxNode* CreateCamera(FbxScene* pScene, char* pName)
	{
		FbxCamera* lCamera = FbxCamera::Create(pScene, pName);

		// Set camera property for a classic TV projection with aspect ratio 4:3
		lCamera->SetFormat(FbxCamera::eNTSC);

		FbxNode* lNode = FbxNode::Create(pScene, pName);

		lNode->SetNodeAttribute(lCamera);

		return lNode;
	}

	// Set target of the camera.
	void SetCameraPointOfInterest(FbxNode* pCamera, FbxNode* pPointOfInterest)
	{
		// Set the camera to always point at this node.
		pCamera->SetTarget(pPointOfInterest);
	}

	// Set marker default position.
	void SetMarkerDefaultPosition(FbxNode* pMarker)
	{
		// The marker is positioned above the origin. There is no rotation and no scaling.
		pMarker->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));
		pMarker->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
		pMarker->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
	}

	// Compute the camera position.
	void SetCameraDefaultPosition(FbxNode* pCamera)
	{
		// set the initial camera position
		FbxVector4 lCameraLocation(0.0, 200.0, -100.0);
		pCamera->LclTranslation.Set(lCameraLocation);
	}

	// to get the root node
	const FbxNode* GetRootNode()
	{
		return gScene->GetRootNode();
	}

	// to get the root node name
	const char * GetRootNodeName()
	{
		return GetRootNode()->GetName();
	}

	//-------------------------------EXPORT--------------------------------------//
	//------------------------------------------------------------------------------//

	// to save a scene to a FBX file
	bool Export(
		const char* pFilename,
		int pFileFormat
	)
	{
		return SaveScene(gSdkManager, gScene, pFilename, pFileFormat, true); // true -> embed texture file
	}

	// to save a scene to a FBX file
	bool SaveScene(FbxManager* pSdkManager, FbxDocument* pScene, const char* pFilename, int pFileFormat, bool pEmbedMedia)
	{
		if (pSdkManager == NULL) return false;
		if (pScene == NULL) return false;
		if (pFilename == NULL) return false;

		bool lStatus = true;

		// Create an exporter.
		FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

		if (pFileFormat < 0 || pFileFormat >= pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount())
		{
			// Write in fall back format if pEmbedMedia is true
			pFileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

			if (!pEmbedMedia)
			{
				//Try to export in ASCII if possible
				int lFormatIndex, lFormatCount = pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();

				for (lFormatIndex = 0; lFormatIndex<lFormatCount; lFormatIndex++)
				{
					if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
					{
						FbxString lDesc = pSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
						char *lASCII = "ascii";
						if (lDesc.Find(lASCII) >= 0)
						{
							pFileFormat = lFormatIndex;
							break;
						}
					}
				}
			}
		}

		// Initialize the exporter by providing a filename.
		if (lExporter->Initialize(pFilename, pFileFormat, pSdkManager->GetIOSettings()) == false)
		{
			return false;
		}

		// Set the export states. By default, the export states are always set to 
		// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
		// shows how to change these states.
		IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
		IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

		// Export the scene.
		lStatus = lExporter->Export(pScene);

		// Destroy the exporter.
		lExporter->Destroy();

		return lStatus;
	}

	//-------------------------------GENERATOR--------------------------------------//
	//------------------------------------------------------------------------------//

	// create a new cube
	void CreateCube(bool pWithTexture, bool pAnimate)
	{
		// make a new cube name
		FbxString lCubeName = "Cube number ";
		lCubeName += FbxString(gCubeNumber);

		// create a new cube
		CreateCubeDetailed(lCubeName.Buffer(),
			gCubeXPos,
			gCubeYPos,
			gCubeZPos,
			gCubeRotationAxis,
			pWithTexture,
			pAnimate
		);

		// compute for next cube creation    
		gCubeNumber++; // cube number

					   // set next pos
		if (gCubeXPos >= 0.0)
		{
			gCubeXPos += 50.0;
			gCubeXPos *= -1.0;
			gCubeRotationAxis++; // change rotation axis
		}
		else
		{
			gCubeXPos *= -1.0;
		}

		// go up
		gCubeYPos += 30.0;

		if (gCubeRotationAxis > 2) gCubeRotationAxis = 0; // cube rotation
	}

	// create a new cube
	void CreateCubeDetailed(char* pCubeName,
		double pX,
		double pY,
		double pZ,
		int pRotateAxe,
		bool pWithTexture,
		bool pAnimate
	)
	{
		FbxNode* lCube = CreateCubeMesh(gScene, pCubeName);

		// set the cube position
		lCube->LclTranslation.Set(FbxVector4(pX, pY, pZ));

		if (pAnimate)
		{
			//QG AnimateCube(lCube, gAnimLayer, pRotateAxe);
		}

		if (pWithTexture)
		{
			// if we asked to create the cube with a texture, we need 
			// a material present because the texture connects to the
			// material DiffuseColor property
			//QG AddMaterials(lCube->GetMesh());
		}

		gScene->GetRootNode()->AddChild(lCube);
	}

	// Create a cube mesh. 
	FbxNode* CreateCubeMesh(FbxScene* pScene, char* pName)
	{
		int i, j;
		FbxMesh* lMesh = FbxMesh::Create(pScene, pName);

		FbxVector4 lControlPoint0(-50, 0, 50);
		FbxVector4 lControlPoint1(50, 0, 50);
		FbxVector4 lControlPoint2(50, 100, 50);
		FbxVector4 lControlPoint3(-50, 100, 50);
		FbxVector4 lControlPoint4(-50, 0, -50);
		FbxVector4 lControlPoint5(50, 0, -50);
		FbxVector4 lControlPoint6(50, 100, -50);
		FbxVector4 lControlPoint7(-50, 100, -50);

		FbxVector4 lNormalXPos(1, 0, 0);
		FbxVector4 lNormalXNeg(-1, 0, 0);
		FbxVector4 lNormalYPos(0, 1, 0);
		FbxVector4 lNormalYNeg(0, -1, 0);
		FbxVector4 lNormalZPos(0, 0, 1);
		FbxVector4 lNormalZNeg(0, 0, -1);

		// Create control points.
		lMesh->InitControlPoints(24);
		FbxVector4* lControlPoints = lMesh->GetControlPoints();

		lControlPoints[0] = lControlPoint0;
		lControlPoints[1] = lControlPoint1;
		lControlPoints[2] = lControlPoint2;
		lControlPoints[3] = lControlPoint3;
		lControlPoints[4] = lControlPoint1;
		lControlPoints[5] = lControlPoint5;
		lControlPoints[6] = lControlPoint6;
		lControlPoints[7] = lControlPoint2;
		lControlPoints[8] = lControlPoint5;
		lControlPoints[9] = lControlPoint4;
		lControlPoints[10] = lControlPoint7;
		lControlPoints[11] = lControlPoint6;
		lControlPoints[12] = lControlPoint4;
		lControlPoints[13] = lControlPoint0;
		lControlPoints[14] = lControlPoint3;
		lControlPoints[15] = lControlPoint7;
		lControlPoints[16] = lControlPoint3;
		lControlPoints[17] = lControlPoint2;
		lControlPoints[18] = lControlPoint6;
		lControlPoints[19] = lControlPoint7;
		lControlPoints[20] = lControlPoint1;
		lControlPoints[21] = lControlPoint0;
		lControlPoints[22] = lControlPoint4;
		lControlPoints[23] = lControlPoint5;

		// We want to have one normal for each vertex (or control point),
		// so we set the mapping mode to eByControlPoint.
		FbxGeometryElementNormal* lGeometryElementNormal = lMesh->CreateElementNormal();

		lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);

		// Set the normal values for every control point.
		lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

		lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
		lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);


		// Array of polygon vertices.
		int lPolygonVertices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,12, 13,
			14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };


		// Create UV for Diffuse channel.
		FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV("DiffuseUV");
		FBX_ASSERT(lUVDiffuseElement != NULL);
		lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
		lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

		FbxVector2 lVectors0(0, 0);
		FbxVector2 lVectors1(1, 0);
		FbxVector2 lVectors2(1, 1);
		FbxVector2 lVectors3(0, 1);

		lUVDiffuseElement->GetDirectArray().Add(lVectors0);
		lUVDiffuseElement->GetDirectArray().Add(lVectors1);
		lUVDiffuseElement->GetDirectArray().Add(lVectors2);
		lUVDiffuseElement->GetDirectArray().Add(lVectors3);

		//Now we have set the UVs as eIndexToDirect reference and in eByPolygonVertex  mapping mode
		//we must update the size of the index array.
		lUVDiffuseElement->GetIndexArray().SetCount(24);

		// Create polygons. Assign texture and texture UV indices.
		for (i = 0; i < 6; i++)
		{
			// all faces of the cube have the same texture
			lMesh->BeginPolygon(-1, -1, -1, false);

			for (j = 0; j < 4; j++)
			{
				// Control point index
				lMesh->AddPolygon(lPolygonVertices[i * 4 + j]);

				// update the index array of the UVs that map the texture to the face
				lUVDiffuseElement->GetIndexArray().SetAt(i * 4 + j, j);
			}

			lMesh->EndPolygon();
		}

		// create a FbxNode
		FbxNode* lNode = FbxNode::Create(pScene, pName);

		// set the node attribute
		lNode->SetNodeAttribute(lMesh);

		// set the shading mode to view texture
		lNode->SetShadingMode(FbxNode::eTextureShading);

		// rescale the cube
		lNode->LclScaling.Set(FbxVector4(0.3, 0.3, 0.3));

		// return the FbxNode
		return lNode;
	}
}



//-------------------------------MAIN--------------------------------------//
//------------------------------------------------------------------------------//


#include <iostream>
#include <string>
#include <chrono>
using namespace std::chrono;
using namespace std;

int main(int argc, const char* argv[])
{
	std::string outFileName = "mgen_";
	milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

	if (argc > 1) {
		outFileName += argv[1] + string("_") + to_string(ms.count()) + ".fbx";
	}
	else {
		cout << "No args: Using default file name: ";
		outFileName += to_string(ms.count()) + ".fbx";
	}
	cout << outFileName << endl;

	qg::CreateScene();
	// create a new cube with option selected
	//args bool (lWithTexture, lWithAnimation);
	qg::CreateCube(false, false);

	char gszOutputFile[_MAX_PATH];           // File name to export
	int  gWriteFileFormat = -1;             // Write file format

	qg::Export(outFileName.c_str(), gWriteFileFormat);


	// dont forget to delete the SdkManager 
	// and all objects created by the SDK manager
	qg::DestroySdkObjects(qg::gSdkManager, true);
}