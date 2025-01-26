#include <Windows.h>
#include "config.h"
#include <iostream>

bool Config::bMenu = false;
bool Config::bHealth = false;
bool Config::bAmmo = false;
bool Config::bFly = false;
bool Config::bOneHit = false;
bool Config::bRapidFire = false;
bool Config::bRecoil = false;
bool Config::bTp = false;
bool Config::bAimbot = false;
bool Config::bEsp = false;
bool Config::bHealthBar = false;
bool Config::bSnapLines = false;
bool Config::bTriggerbot = false;
bool Config::bBunnyhop = false;
bool Config::bVisCheck = false;
bool Config::bAimFov = false;
float Config::aimbotSmooth = 0.0f;
float Config::fovRadius = 50.0f;

float Config::savedXpos = 0.0f;
float Config::savedYpos = 0.0f;
float Config::savedZpos = 0.0f;