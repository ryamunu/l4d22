#include "AutoShove.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../Vars.h"

bool CanShoveWitch(int rage) {
  if (rage == 1)
    return true;
  else
    return false;
  return false;
}
bool CAutoShove::CanShove(C_TerrorPlayer *pLocal, C_Infected *pInfected) {
  if (!pLocal || !pInfected)
    return false;

  if (pInfected->IsDormant())
    return false;
  if (!pInfected->ValidEntity(pInfected->m_nSequence(),
                              pInfected->m_usSolidFlags()))
    return false;
  if (!pLocal->GetActiveWeapon())
    return false;
  if (!pLocal->CanBeShoved() || !pLocal->IsReadyToShove())
    return false;
  if (pInfected->GetClientClass() &&
      pInfected->GetClientClass()->m_ClassID == Witch &&
      Vars::Ignore::IgnoreWitchUntilStartled &&
      !CanShoveWitch(pInfected->As<C_Witch *>()->m_rage()))
    return false;

  // --- Prediction Logic for Zonemod ---
  float distance = 0.0f;

#ifndef FLOW_OUTGOING
#define FLOW_OUTGOING 0
#endif

  // Get Latency
  float latency = 0.0f;
  if (I::EngineClient->GetNetChannelInfo())
    latency =
        I::EngineClient->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING);

  // Get Lerp
  float lerp = 0.1f; // default 100ms
  static ConVar *cv_interp = I::Cvars->FindVar("cl_interp");
  static ConVar *cv_interp_ratio = I::Cvars->FindVar("cl_interp_ratio");
  static ConVar *cv_updaterate = I::Cvars->FindVar("cl_updaterate");

  if (cv_interp && cv_interp_ratio && cv_updaterate) {
    if (cv_updaterate->GetFloat() != 0.0f)
      lerp = std::max(cv_interp->GetFloat(),
                      cv_interp_ratio->GetFloat() / cv_updaterate->GetFloat());
  }

  // Calculate Predicted Position
  // We want to shove where they WILL be when our command arrives.
  // Server Time = Client Time + Latency.
  // They are currently at 'Origin' which is 'Lerp' seconds in the past relative
  // to Server Time. Actually, we see them at T-Lerp. We send command, it
  // arrives at T+Latency. Total extrapolation needed: Latency + Lerp.

  float total_pred = latency + lerp;

  // Cap prediction to reasonable limits to avoid shoving ghosts
  // if (total_pred > 0.2f) total_pred = 0.2f;

  Vector vel;
  pInfected->EstimateAbsVelocity(vel);

  Vector predictedPos = pInfected->GetAbsOrigin() + (vel * total_pred);
  distance = (pLocal->GetAbsOrigin() - predictedPos).Lenght();

  // Zonemod Hunter Pounce Speed ~700.
  // 700 * 0.1s = 70 units. Significant difference.

  if (distance > 87.0f)
    return false;

  return true;
}
#include "../../SDK/EntityCache/entitycache.h"
C_BaseEntity *CAutoShove::GetBestTarget(C_TerrorPlayer *pLocal) {
  C_BaseEntity *BestEntity = nullptr;
  float BestDistance = FLT_MAX;

  // Only target COMMONS if AutoShoveCommon is enabled
  if (Vars::Misc::AutoShoveCommon) {
    for (const auto pInfected : gEntityCache.GetGroup(EGroupType::INFECTED)) {
      if (!pInfected)
        continue;
      if (!CanShove(pLocal, pInfected->As<C_Infected *>()))
        continue;

      float dist =
          (pLocal->GetAbsOrigin() - pInfected->GetAbsOrigin()).Lenght();
      if (dist < BestDistance) {
        BestDistance = dist;
        BestEntity = pInfected->As<C_Infected *>();
      }
    }
  }

  // Target SPECIAL INFECTED based on filters
  for (const auto pSpecial :
       gEntityCache.GetGroup(EGroupType::SPECIAL_INFECTED)) {
    if (!pSpecial)
      continue;

    C_TerrorPlayer *pPlayer = pSpecial->As<C_TerrorPlayer *>();
    if (!pPlayer || pPlayer->IsDormant())
      continue;

    // Check class-based filters
    ClientClass *pCC = pPlayer->GetClientClass();
    if (!pCC)
      continue;

    int classId = pCC->m_ClassID;

    // Apply filters - if filter is ON, we SKIP that type
    if (classId == Smoker && !Vars::Misc::AutoShoveSmoker)
      continue;
    if (classId == Boomer && !Vars::Misc::AutoShoveBoomer)
      continue;
    if (classId == Hunter && !Vars::Misc::AutoShoveHunter)
      continue;
    if (classId == Jockey && !Vars::Misc::AutoShoveJockey)
      continue;
    if (classId == Charger && !Vars::Misc::AutoShoveCharger)
      continue;
    if (classId == Spitter && !Vars::Misc::AutoShoveSpitter)
      continue;
    if (classId == Tank && !Vars::Misc::AutoShoveTank)
      continue;

    // Distance check
    float dist = (pLocal->GetAbsOrigin() - pPlayer->GetAbsOrigin()).Lenght();
    if (dist > 87) // Shove range
      continue;

    // Check player is valid
    if (!pPlayer->IsAlive())
      continue;

    if (dist < BestDistance) {
      BestDistance = dist;
      BestEntity = pPlayer;
    }
  }

  return BestEntity;
}

bool PosTwo(C_BaseEntity *pSkip, C_BaseEntity *pEntity, Vector from,
            Vector to) {
  Ray_t ray;
  ray.Init(from, to);

  const uint32_t mask = (MASK_SHOT);

  CTraceFilter filter;
  filter.pSkip = pSkip;

  trace_t trace;
  I::EngineTrace->TraceRay(ray, mask, &filter, &trace);

  return ((trace.m_pEnt && trace.m_pEnt == pEntity));
}

Vector GetBestHitbox(C_BaseEntity *pEntity, C_TerrorPlayer *pLocal) {
  Vector vTarget;
  C_BaseAnimating *pAnimating = pEntity->As<C_BaseAnimating *>();
  if (!pAnimating)
    return Vector();
  if (!pAnimating->GetHitboxPositionByGroup(HITGROUP_HEAD, vTarget))
    return Vector();
  if (!PosTwo(pLocal, pEntity, pLocal->EyePosition(), vTarget))
    return Vector(0, 0, 0);

  return vTarget;
}

Vector GetBestAngle(C_BaseEntity *pEntity, C_TerrorPlayer *pLocal) {
  auto Hitbox = GetBestHitbox(pEntity, pLocal);
  if (Hitbox.IsZero())
    return Vector();

  return U::Math.CalcAngle(pLocal->EyePosition(),
                           GetBestHitbox(pEntity, pLocal));
}

inline void FixMovementAgain(CUserCmd *pCmd, Vector m_vOldAngles,
                             float m_fOldForward, float m_fOldSidemove) {
  float deltaView = pCmd->viewangles.y - m_vOldAngles.y;
  float f1;
  float f2;

  if (m_vOldAngles.y < 0.f)
    f1 = 360.0f + m_vOldAngles.y;
  else
    f1 = m_vOldAngles.y;

  if (pCmd->viewangles.y < 0.0f)
    f2 = 360.0f + pCmd->viewangles.y;
  else
    f2 = pCmd->viewangles.y;

  if (f2 < f1)
    deltaView = abs(f2 - f1);
  else
    deltaView = 360.0f - abs(f1 - f2);
  deltaView = 360.0f - deltaView;

  pCmd->forwardmove = cos(DEG2RAD(deltaView)) * m_fOldForward +
                      cos(DEG2RAD(deltaView + 90.f)) * m_fOldSidemove;
  pCmd->sidemove = sin(DEG2RAD(deltaView)) * m_fOldForward +
                   sin(DEG2RAD(deltaView + 90.f)) * m_fOldSidemove;
}

void CAutoShove::run(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  if (!Vars::Misc::AutoShove)
    return;
  if (!pLocal || !pCmd)
    return;

  auto pEntity = GetBestTarget(pLocal);
  if (!pEntity || pEntity->IsDormant() || !pEntity->IsAlive())
    return;
  if (pLocal->m_iTeamNum() != TEAM_SURVIVOR)
    return; // if we're infected, dont fucking do hitscan aimbot..
  if (pLocal->m_isGhost())
    return;
  if (!pLocal->GetActiveWeapon() || !pLocal->IsAlive())
    return;
  Vector bestHitbox = GetBestHitbox(pEntity, pLocal);
  if (bestHitbox.IsZero())
    return;

  if (!PosTwo(pLocal, pEntity, pLocal->EyePosition(), bestHitbox))
    return; // Visibility check

  Vector m_vOldViewAngle = pCmd->viewangles;
  float m_fOldSideMove = pCmd->sidemove;
  float m_fOldForwardMove = pCmd->forwardmove;

  Vector Angle = U::Math.CalcAngle(pLocal->EyePosition(), bestHitbox);

  if (Angle.IsZero())
    return;

  pCmd->viewangles = Angle;
  FixMovementAgain(pCmd, m_vOldViewAngle, m_fOldForwardMove, m_fOldSideMove);
  pCmd->buttons |= IN_ATTACK2;
}
