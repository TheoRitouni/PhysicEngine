// CollisionEngine.cpp : définit le point d'entrée pour l'application console.
//

#pragma comment(lib, "legacy_stdio_definitions.lib")
#include "stdafx.h"


#include <iostream>
#include <string>

#include "Application.h"
#include "SceneManager.h"


#include "Scenes/SceneDebugCollisions.h"
#include "Scenes/SceneBouncingPolys.h"
#include "Scenes/SceneSimplePhysic.h"
#include "Scenes/SceneSpheres.h"
#include "Scenes/SceneComplexPhysic.h"
#include "Scenes/SceneSmallPhysic.h"

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
/*
* Entry point
*/
int _tmain(int argc, char** argv)
{
    InitApplication(1260, 768, 50.0f);


    gVars->pSceneManager->AddScene(new CSceneSimplePhysic());
    gVars->pSceneManager->AddScene(new CSceneComplexPhysic(25));
    gVars->pSceneManager->AddScene(new CSceneSmallPhysic());


    RunApplication();
    return 0;
}

