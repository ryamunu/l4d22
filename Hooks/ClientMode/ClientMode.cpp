#include "ClientMode.h"

#include "../../Features/AutoShove/AutoShove.h"
#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Hitscan/Hitscan.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Sequence Freezing/SequenceFreezing.h"
#include "../../Features/Vars.h"
#include <algorithm>

using namespace Hooks;

bool __fastcall ClientMode::ShouldDrawFog::Detour(void *ecx, void *edx) {
  return Table.Original<FN>(Index)(ecx, edx);
}

bool __fastcall ClientMode::CreateMove::Detour(void *ecx, void *edx,
                                               float input_sample_frametime,
                                               CUserCmd *cmd) {
  const bool bResult =
      Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd);

  if (!cmd || !cmd->command_number)
    return bResult;

  IClientEntity *pLocalEntity =
      I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
  if (!pLocalEntity)
    return bResult;

  C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();
  if (!pLocal || pLocal->deadflag())
    return bResult;

  C_BaseCombatWeapon *pBaseWeapon = pLocal->GetActiveWeapon();
  if (!pBaseWeapon)
    return bResult;

  C_TerrorWeapon *pWeapon = pBaseWeapon->As<C_TerrorWeapon *>();
  f::misc->run(pLocal, cmd);

  F::EnginePrediction.Start(pLocal, cmd);
  {
    f::SequenceFreezing.Run(cmd, pLocal);
    f::hitscan.run(pLocal, cmd);
    f::autoshove.run(pLocal, cmd);
    F::NoSpread.Run(pLocal, pWeapon, cmd);
  }
  F::EnginePrediction.Finish(pLocal, cmd);

  U::Math.ClampAngles(cmd->viewangles);
  viewangle = cmd->viewangles;
  I::Prediction->SetLocalViewAngles(cmd->viewangles);

  return false;
}

void __fastcall ClientMode::DoPostScreenSpaceEffects::Detour(
    void *ecx, void *edx, const void *pSetup) {
  Table.Original<FN>(Index)(ecx, edx, pSetup);
}

float __fastcall ClientMode::GetViewModelFOV::Detour(void *ecx, void *edx) {
  return Table.Original<FN>(Index)(ecx, edx);
}
#include "../../SDK/L4D2/Interfaces/EngineTrace.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../../Util/Math/Math.h"

void __fastcall ClientMode::OverrideView::Detour(void *ecx, void *edx,
                                                 CViewSetup *View) {
  Table.Original<FN>(Index)(ecx, edx, View);

  if (!View || !Vars::Misc::ThirdPerson || !I::EngineClient->IsInGame())
    return;

  IClientEntity *pLocalEntity =
      I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
  if (!pLocalEntity)
    return;

  C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();
  if (!pLocal || !pLocal->IsAlive())
    return;

  Vector forward, right, up;
  U::Math.angleVectors(View->angles, &forward, &right, &up);

  Vector eyePosition = pLocal->EyePosition();
  float cameraDistance = Vars::Misc::ThirdPersonDistance;
  if (cameraDistance < 0.0f)
    cameraDistance = 0.0f;

  Vector desiredOrigin = eyePosition;
  desiredOrigin += up * Vars::Misc::CameraVertical;
  desiredOrigin += right * Vars::Misc::CameraHorizontal;
  desiredOrigin -= forward * cameraDistance;

  Ray_t ray;
  ray.Init(eyePosition, desiredOrigin);

  CTraceFilter filter;
  filter.pSkip = pLocalEntity;

  trace_t trace;
  I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

  if (trace.fraction < 1.0f) {
    constexpr float kCameraPadding = 8.0f;
    const float distanceSafe = (cameraDistance > 1.0f) ? cameraDistance : 1.0f;
    float cameraFraction = trace.fraction - (kCameraPadding / distanceSafe);
    if (cameraFraction < 0.0f)
      cameraFraction = 0.0f;

    View->origin = eyePosition + (desiredOrigin - eyePosition) * cameraFraction;
  } else {
    View->origin = desiredOrigin;
  }

  if (I::Input) {
    I::Input->m_fCameraInThirdPerson() = true;
    I::Input->m_fCameraInThirdPerson_Old() = true;
    I::Input->m_fCameraInThirdPerson_Alt() = true;
  }
}

bool __fastcall ClientMode::ShouldDrawLocalPlayer::Detour(
    void *ecx, void *edx, C_BasePlayer *pPlayer) {
  if (Vars::Misc::ThirdPerson && I::EngineClient->IsInGame()) {
    return true;
  }
  return Table.Original<FN>(Index)(ecx, edx, pPlayer);
}

void ClientMode::Init() {
  XASSERT(Table.Init(I::ClientMode) == false);
  XASSERT(Table.Hook(&ShouldDrawLocalPlayer::Detour,
                     ShouldDrawLocalPlayer::Index) == false);
  XASSERT(Table.Hook(&ShouldDrawFog::Detour, ShouldDrawFog::Index) == false);
  XASSERT(Table.Hook(&CreateMove::Detour, CreateMove::Index) == false);
  XASSERT(Table.Hook(&DoPostScreenSpaceEffects::Detour,
                     DoPostScreenSpaceEffects::Index) == false);
  XASSERT(Table.Hook(&GetViewModelFOV::Detour, GetViewModelFOV::Index) ==
          false);
  XASSERT(Table.Hook(&OverrideView::Detour, OverrideView::Index) == false);
}
