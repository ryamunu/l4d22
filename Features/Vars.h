#pragma once
#include "../SDK/L4D2/Includes/color.h"
#include "../SDK/SDK.h"

template <class T> class CVar {
public:
  T m_Var;
  const wchar_t *m_szDisplayName;
};

// Generic ESP Settings Struct for easy expansion
struct C_ESP_Settings {
  bool Enabled = false;
  bool Box = false;
  bool Name = false;
  bool Healthbar = false;
  bool NumericHP = false;
  bool Distance = false;
  bool Skeleton = false;
  bool VisCheck = false;
  Color DetailColor;

  // Color for the main entity (Box/Skeleton)
  Color MainColor = {255, 255, 255, 255};
};

namespace Vars {
namespace Menu {
inline bool Opened = false;
inline int TitleBarH = 16;
inline int ShadowSize = 128;
inline int SpacingX = 8;
inline int SpacingY = 4;
inline int SpacingText = 4;
inline int CheckBoxW = 10;
inline int CheckBoxH = 10;
inline int CheckBoxFillSize = 4;
inline int ButtonTabW = 198;
inline int ButtonTabH = 25;
inline int ButtonW = 90;
inline int ButtonH = 20;
inline int ButtonWSmall = 70;
inline int ButtonHSmall = 16;
inline int ComboBoxW = 90;
inline int ComboBoxH = 14;
inline int InputBoxW = 90;
inline int InputBoxH = 14;
inline int InputColorBoxW = 30;

namespace Colors {
inline Color Text = {255, 255, 255, 190};
inline Color Widget = {255, 255, 255, 190};
inline Color WidgetActive = {255, 255, 255, 255};
inline Color ShadowColor = {20, 20, 20, 255};
inline Color OutlineMenu = {50, 50, 50, 255};
} // namespace Colors
} // namespace Menu

namespace Hitscan {
inline bool bEnable = true;
inline bool bAutoShoot = true;
inline bool bAutoPistol = false;
inline bool bKey = true;
inline bool bWaitForHitbox = true;
inline bool bSilentAim = false;
inline int iFov = 15;
inline int MaxPlayers = 2;
inline int iHitbox = 1; // 0=Head, 1=Chest, 2=Stomach
inline int iRcsFov = 15;
inline CVar<int> AimKey{VK_LBUTTON, L"Aim Key"};
inline bool MultiPointIfNoHitboxesAreVisible = false;
inline bool AimFovCircle = true;
inline Color AimFov = {255, 255, 255, 255};
inline bool bSmoothing = true;
inline float fSmoothFactor = 5.0f;
inline int SmoothMethod = 0; // 0 = Static, 1 = Dynamic
inline float RandomizeFactor = 0.0f;
inline float CurveFactor = 0.5f;
inline float TargetSwitchDelay = 0.0f;

inline bool IgnoreWalls = false;
inline bool IgnoreSmoker = false;
inline bool IgnoreBoomer = false;
inline bool IgnoreHunter = false;
inline bool IgnoreSpitter = false;
inline bool IgnoreJockey = false;
inline bool IgnoreCharger = false;
inline bool IgnoreTank = false;
inline bool IgnoreWitch = false;
} // namespace Hitscan

namespace Backtrack {
inline bool Enabled = false;
inline int BacktrackMS = 200;
} // namespace Backtrack

namespace Exploits {
inline bool Speedhack = false;
inline CVar<int> Key{VK_CONTROL, L"SpeedHack"};
inline int SpeedHackValue = 5;
} // namespace Exploits

namespace Misc {
inline bool Namestealer = false;
inline bool Bunnyhop = true;
inline int BhopChance = 100;
inline bool HumanRandomizer = false;
inline bool AutoStrafe = true;
inline float AutoStrafeSmooth = 1.0f;
inline bool EdgeJump = false;
inline bool MiniJump = false;
inline bool JumpBug = false;
inline bool EdgeBug = false;
inline bool SequenceFreezing = true;
inline bool DisableInterp = true;
inline int SequenceFreezingValue = 0;
inline CVar<int> Key{VK_MENU, L"Sequence Freezing"};

inline bool FakeLag = false;
inline int FakeLagFactor = 0; // Choke amount

inline bool AutoShove = true;
inline int AutoShoveDelay = 500;
inline bool AutoShoveCommon = true;
inline bool AutoShoveTank = false;
inline bool AutoShoveHunter = true;
inline bool AutoShoveSmoker = true;
inline bool AutoShoveJockey = true;
inline bool AutoShoveCharger = true;
inline bool AutoShoveBoomer = true;
inline bool AutoShoveSpitter = true;
inline bool AutoShoveWitch = false;

inline bool ThirdPerson = false;
inline int ThirdPersonDistance = 120;
inline int CameraHeight = 0;
inline int CameraHorizontal = 0;
inline int CameraVertical = 0;
inline CVar<int> ThirdPersonKey{0x0, L"ThirdPerson Key"};

inline bool Teleport = false;
inline CVar<int> TeleportKey{VK_P, L"Teleport Key"};

inline CVar<int> EdgeJumpKey{0, L"Edge Jump Key"};
} // namespace Misc

namespace Removals {
inline bool NoSpread = false;
inline bool NoRecoil = false;
inline bool NoVisualRecoil = false;
inline bool BoomerVisual = false;
inline bool VisualSpread = false;
inline bool DisablePostProcessing = false;
} // namespace Removals

namespace ESP {
// New Organized ESP settings
// Survivors uses the struct too but with defaults:
inline C_ESP_Settings Survivor = {
    true,  true,  true,  true,         false,
    false, false, false, {0, 0, 0, 0}, {100, 200, 255, 255}};
inline C_ESP_Settings Infected = {
    true,  true,  true,  true,         false,
    false, false, false, {0, 0, 0, 0}, {217, 25, 50, 255}}; // SI
inline C_ESP_Settings Common = {
    false, false, false, false,        false,
    false, false, false, {0, 0, 0, 0}, {200, 200, 200, 255}};
inline C_ESP_Settings Tank = {
    true,  true,  false, true,         false,
    false, false, false, {0, 0, 0, 0}, {255, 0, 0, 255}};
// Witch uses struct but might not need all logic
inline C_ESP_Settings Witch = {
    true, true,  true,  true,         false,
    true, false, false, {0, 0, 0, 0}, {255, 0, 150, 255}};
// Ghost mode infected (when spawning as infected)
inline C_ESP_Settings Ghost = {
    true,         true,
    false,        false,
    false,        false,
    false,        false,
    {0, 0, 0, 0}, {150, 150, 255, 100}}; // Light blue/transparent

inline bool TankFrustration = true;

// Legacy variables needed for Config loader compatibility (or updated config
// loader) To avoid compilation errors in Config.cpp (which I can update), I
// will define them as references or just clean up Config.cpp Actually, since I
// rewrote Config.h to access these directly, I need to update Config.h as well.
// For now, I'll keep the new structure clean.

// Items ESP
inline bool ItemsEnabled = true;
inline float ItemsMaxDistance = 1500.0f;
inline bool ItemsWeapons = true;
inline bool ItemsMedkits = true;
inline bool ItemsPills = true;
inline bool ItemsAdrenaline = true;
inline bool ItemsDefib = true;
inline bool ItemsProps = true;
} // namespace ESP

namespace Chams {
// inline bool Players = true; // DEPRECATED
inline bool Survivors = true;
inline bool InfectedPlayers = true;
inline bool Infected = true; // Common infected
inline bool Tank = true;
inline bool Witch = true;
inline bool Boomer = true;
inline bool Hunter = true;
inline bool Smoker = true;
inline bool Jockey = true;
inline bool Charger = true;
inline bool Spitter = true;

inline Color PlayerColor = {240, 137, 229, 255};
inline Color PlayerInfectedColor = {150, 15, 15, 255};
inline Color InfectedColor = {150, 15, 15, 255}; // Common infected
inline Color TankColor = {255, 0, 0, 255};
inline Color WitchColor = {255, 0, 150, 255};
inline Color BoomerColor = {0, 255, 0, 255};
inline Color HunterColor = {255, 150, 0, 255};
inline Color SmokerColor = {0, 150, 255, 255};
inline Color JockeyColor = {255, 255, 0, 255};
inline Color ChargerColor = {150, 100, 0, 255};
inline Color SpitterColor = {0, 255, 100, 255};
} // namespace Chams

namespace Ignore {
inline bool IgnoreWitchUntilStartled = true;
inline bool IgnoreFriends = true;
inline bool IgnoreTeammates = true;
inline bool IgnoreCommon = true;
} // namespace Ignore

namespace DT {
inline bool Enable = true;
inline bool Recharging;
inline bool shifting = false;
inline int ticks = 0;
inline int Shifted = 0;
} // namespace DT

namespace Visuals {
inline bool SpectatorList = false;
inline int SpectatorListX = 20;
inline int SpectatorListY = 200;
} // namespace Visuals

} // namespace Vars