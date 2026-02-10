#include "ClientMode.h"

#include "../../Features/AutoShove/AutoShove.h"
#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Hitscan/Hitscan.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Sequence Freezing/SequenceFreezing.h"
#include "../../Features/Vars.h"

using namespace Hooks;

bool __fastcall ClientMode::ShouldDrawFog::Detour(void *ecx, void *edx) {
  return Table.Original<FN>(Index)(ecx, edx);
}

bool __fastcall ClientMode::CreateMove::Detour(void *ecx, void *edx,
                                               float input_sample_frametime,
                                               CUserCmd *cmd) {
  if (!cmd || !cmd->command_number)
    return Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd);

  if (Table.Original<FN>(Index)(ecx, edx, input_sample_frametime, cmd))
    I::Prediction->SetLocalViewAngles(cmd->viewangles);
  DWORD pep;
  __asm mov pep, ebp;
  bool *BSendPacket =
      reinterpret_cast<bool *>(*reinterpret_cast<char **>(pep) - 0x1D);

  IClientEntity *pLocalEntity =
      I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
  if (!pLocalEntity)
    return false;
  C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();

  if (pLocal && !pLocal->deadflag()) {
    C_BaseCombatWeapon *pBaseWeapon = pLocal->GetActiveWeapon();
    if (pBaseWeapon) {
      C_TerrorWeapon *pWeapon = pBaseWeapon->As<C_TerrorWeapon *>();
      {
        // Removed Dangerous Teleport/Crash Code
        // This was likely causing "Untrusted" bans due to Infinite angles.
        f::misc->run(pLocal, cmd); // run bhop before prediction for obv reasons
        F::EnginePrediction.Start(pLocal, cmd);
        {
          f::SequenceFreezing.Run(cmd, pLocal);
          f::hitscan.run(pLocal, cmd);
          f::autoshove.run(pLocal, cmd);
          F::NoSpread.Run(pLocal, pWeapon, cmd);
        }
        F::EnginePrediction.Finish(pLocal, cmd);

        // CRITICAL: Clamp Angles to prevent Untrusted Bans
        U::Math.ClampAngles(cmd->viewangles);

        viewangle = cmd->viewangles;

        static int choked = 0;
        // Disabled FakeLag temporarily due to crash (0x1D offset issue)
        /*
        if (Vars::Misc::FakeLag) {
          if (choked < Vars::Misc::FakeLagFactor) {
            *BSendPacket = false;
            choked++;
          } else {
            *BSendPacket = true;
            choked = 0;
          }
        }
        */
      }
    }
  }
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

  if (Vars::Misc::ThirdPerson && I::EngineClient->IsInGame()) {
    IClientEntity *pLocalEntity =
        I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
    if (!pLocalEntity)
      return;

    C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();
    if (!pLocal || !pLocal->IsAlive())
      return; // Don't override if dead (spectator)

    // Get appropriate angles safely
    Vector viewAngles = View->angles; // Use current view angles override

    // Calculate backward vector
    // Calculate backward vector
    Vector forward, right, up;
    U::Math.angleVectors(viewAngles, &forward, &right, &up);

    // Desired camera position
    // Offset: Start from eyes
    Vector vecEyePos = pLocal->EyePosition();

    // Apply offsets from menu
    Vector camOffset = vecEyePos;
    camOffset += up * Vars::Misc::CameraVertical;
    camOffset += right * Vars::Misc::CameraHorizontal;
    camOffset -= forward * Vars::Misc::ThirdPersonDistance; // Move back

    // Trace ray to prevent wall clipping
    Ray_t ray;
    ray.Init(vecEyePos, camOffset); // Legacy Init takes only start/end

    CTraceFilter filter;
    filter.pSkip = pLocalEntity;

    trace_t trace;
    I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

    if (trace.fraction < 1.0f) {
      // Collision detected, move camera to hit position
      View->origin = trace.endpos;
    } else {
      View->origin = camOffset;
    }

    // FORCE 0xAD (Critical for Versus Body Rendering)
    // Ensures state is correct right before rendering
    if (I::Input) {
      I::Input->m_fCameraInThirdPerson() = true;
      I::Input->m_fCameraInThirdPerson_Old() = true;
      I::Input->m_fCameraInThirdPerson_Alt() = true;
    }
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