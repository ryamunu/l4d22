#pragma once
#include "../../SDK/SDK.h"

class chitscan {
private:
  Vector GetBestHitbox(C_BaseEntity *pEntity, C_TerrorPlayer *pLocal);
  Vector GetBestAngle(C_BaseEntity *pEntity, C_TerrorPlayer *pLocal);
  C_BaseEntity *GetBestTarget(C_TerrorPlayer *pLocal);

  // State Tracking
  C_BaseEntity *m_lastTarget = nullptr;
  float m_lastSwitchTime = 0.0f;

public:
  void run(C_TerrorPlayer *pLocal, CUserCmd *pCmd);
  void Reset();
};
namespace f {
inline chitscan hitscan;
}