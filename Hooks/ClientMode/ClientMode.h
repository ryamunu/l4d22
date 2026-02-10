#pragma once

#include "../../SDK/SDK.h"
inline Vector viewangle;
namespace Hooks {
namespace ClientMode {
inline Hook::CTable Table;

struct CViewSetup {
  int x, x_old;
  int y, y_old;
  int width, width_old;
  int height, height_old;
  bool m_bOrtho;
  float m_OrthoLeft;
  float m_OrthoTop;
  float m_OrthoRight;
  float m_OrthoBottom;
  bool m_bCustomViewMatrix;
  matrix3x4_t m_matCustomViewMatrix;
  char pad_0x68[0x48];
  float fov;
  float fovViewmodel;
  Vector origin;
  Vector angles;
  float zNear;
  float zFar;
  float zNearViewmodel;
  float zFarViewmodel;
  float m_flAspectRatio;
  float m_flNearBlurDepth;
  float m_flNearFocusDepth;
  float m_flFarFocusDepth;
  float m_flFarBlurDepth;
  float m_flNearBlurRadius;
  float m_flFarBlurRadius;
  int m_nDoFQuality;
  int m_nMotionBlurMode;
  float m_flShutterTime;
  Vector m_shutterCloseDelay;
  Vector m_shutterOpenDelay;
  Vector m_shutterClosePortion;
  Vector m_shutterOpenPortion;
  float m_flAperture;
  float m_flFocalLength;
};

namespace OverrideView {
using FN = void(__fastcall *)(void *, void *, CViewSetup *);
constexpr uint32_t Index = 19u;

void __fastcall Detour(void *ecx, void *edx, CViewSetup *View);
} // namespace OverrideView

namespace ShouldDrawLocalPlayer {
using FN = bool(__fastcall *)(void *, void *, C_BasePlayer *);
constexpr uint32_t Index = 13u; // Corrected from 14 to 13 to match arg count

bool __fastcall Detour(void *ecx, void *edx, C_BasePlayer *pPlayer);
} // namespace ShouldDrawLocalPlayer

namespace ShouldDrawFog {
using FN = bool(__fastcall *)(void *, void *);
constexpr uint32_t Index = 18u;

bool __fastcall Detour(void *ecx, void *edx);
} // namespace ShouldDrawFog

namespace CreateMove {
using FN = bool(__fastcall *)(void *, void *, float, CUserCmd *);
constexpr uint32_t Index = 27u;

bool __fastcall Detour(void *ecx, void *edx, float input_sample_frametime,
                       CUserCmd *cmd);
} // namespace CreateMove

namespace DoPostScreenSpaceEffects {
using FN = void(__fastcall *)(void *, void *, const void *);
constexpr uint32_t Index = 37u;

void __fastcall Detour(void *ecx, void *edx, const void *pSetup);
} // namespace DoPostScreenSpaceEffects

namespace GetViewModelFOV {
using FN = float(__fastcall *)(void *, void *);
constexpr uint32_t Index = 40u;

float __fastcall Detour(void *ecx, void *edx);
} // namespace GetViewModelFOV

void Init();
} // namespace ClientMode
} // namespace Hooks