#include "BasePlayer.h"

#include "../../Features/Vars.h"

using namespace Hooks;

#include "../../SDK/L4D2/Interfaces/EngineTrace.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../../Util/Math/Math.h"

void __fastcall BasePlayer::CalcPlayerView::Detour(C_BasePlayer *pThis,
                                                   void *edx, Vector &eyeOrigin,
                                                   Vector &eyeAngles,
                                                   float &fov) {
  if (Vars::Removals::NoVisualRecoil && pThis &&
      !pThis->deadflag()) // Thanks Spook for telling me to do it here.
  {
    const Vector vOldPunch = pThis->GetPunchAngle();

    pThis->m_vecPunchAngle().Init();
    Func.Original<FN>()(pThis, edx, eyeOrigin, eyeAngles, fov);
    pThis->m_vecPunchAngle() = vOldPunch;
  } else {
    Func.Original<FN>()(pThis, edx, eyeOrigin, eyeAngles, fov);
  }

  // Third Person Logic (Multiplayer Fix)
  if (Vars::Misc::ThirdPerson && pThis && !pThis->deadflag()) {
    IClientEntity *pLocalEntity =
        I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
    if (pLocalEntity &&
        (void *)pLocalEntity == (void *)pThis) { // Check if local player
      // Calculate Camera Position
      Vector forward, right, up;
      U::Math.angleVectors(eyeAngles, &forward, &right, &up);

      Vector camOffset = eyeOrigin;
      camOffset += up * Vars::Misc::CameraVertical;
      camOffset += right * Vars::Misc::CameraHorizontal;
      camOffset -= forward * Vars::Misc::ThirdPersonDistance;

      // Trace
      Ray_t ray;
      ray.Init(eyeOrigin, camOffset);
      CTraceFilter filter;
      filter.pSkip = pLocalEntity;
      trace_t trace;
      I::EngineTrace->TraceRay(ray, MASK_SOLID, &filter, &trace);

      if (trace.fraction < 1.0f) {
        eyeOrigin = trace.endpos;
      } else {
        eyeOrigin = camOffset;
      }

      // Manual write removed
    }
  }
}

void BasePlayer::Init() {
  // CalcPlayerView
  {
    using namespace CalcPlayerView;

    const FN pfCalcPlayerView =
        reinterpret_cast<FN>(U::Offsets.m_dwCalcPlayerView);
    XASSERT(pfCalcPlayerView == nullptr);

    if (pfCalcPlayerView)
      XASSERT(Func.Init(pfCalcPlayerView, &Detour) == false);
  }
}