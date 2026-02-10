#include "Hitscan.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../Vars.h"

bool TraceToExit(Vector &start, Vector &dir, Vector &End, float step_size,
                 float max_dist) {
  CTraceFilter filter;
  float dist{};
  while (dist <= max_dist) {
    dist += step_size;
    End = start + (dir * dist);

    if (!(I::EngineTrace->GetPointContents_WorldOnly(End) & MASK_SOLID)) {
      return true;
    }
  }

  return false;
}

bool Pos(C_BaseEntity *pSkip, C_BaseEntity *pEntity, Vector from, Vector to) {

  Ray_t ray;
  ray.Init(from, to);

  const uint32_t mask = (MASK_SHOT);

  CTraceFilter filter;
  filter.pSkip = pSkip;

  trace_t trace;
  I::EngineTrace->TraceRay(ray, mask, &filter, &trace);

  return ((trace.m_pEnt && trace.m_pEnt == pEntity));

  // return true;
}

Vector chitscan::GetBestHitbox(C_BaseEntity *pEntity, C_TerrorPlayer *pLocal) {
  Vector vTarget;

  // if (!pEntity->IsZombie())
  //	return Vector(0,0,0);
  C_BaseAnimating *pAnimating = pEntity->As<C_BaseAnimating *>();
  if (!pAnimating)
    return Vector(0, 0, 0);
  int group = HITGROUP_HEAD;
  switch (Vars::Hitscan::iHitbox) {
  case 0:
    group = HITGROUP_HEAD;
    break;
  case 1:
    group = HITGROUP_CHEST;
    break;
  case 2:
    group = HITGROUP_STOMACH;
    break;
  }

  if (!pAnimating->GetHitboxPositionByGroup(group, vTarget))
    return Vector(0, 0, 0);

  if (!Pos(pLocal, pEntity, pLocal->Weapon_ShootPosition(), vTarget))
    return Vector(0, 0, 0);

  if (Vars::Hitscan::RandomizeFactor > 0.0f) {
    float r = Vars::Hitscan::RandomizeFactor;
    vTarget.x += ((float)(rand() % 100) / 100.0f * r * 2.0f) - r;
    vTarget.y += ((float)(rand() % 100) / 100.0f * r * 2.0f) - r;
    vTarget.z += ((float)(rand() % 100) / 100.0f * r * 2.0f) - r;
  }

  return vTarget;
}

Vector chitscan::GetBestAngle(C_BaseEntity *pEntity, C_TerrorPlayer *pLocal) {
  auto Hitbox = GetBestHitbox(pEntity, pLocal);
  if (Hitbox.IsZero())
    return Vector();

  return U::Math.CalcAngle(pLocal->Weapon_ShootPosition(),
                           GetBestHitbox(pEntity, pLocal));
}

inline float CalcFov(const Vector &src, const Vector &dst) {
  Vector v_src = Vector();
  U::Math.AngleVectors(src, &v_src);

  Vector v_dst = Vector();
  U::Math.AngleVectors(dst, &v_dst);

  float result = RAD2DEG(acos(v_dst.Dot(v_src) / v_dst.LenghtSqr()));

  if (!isfinite(result) || isinf(result) || isnan(result))
    result = 0.0f;

  return result;
}

#include "../../SDK/EntityCache/entitycache.h"
#include "../../SDK/GameUtil/GameUtil.h"

/*
todo non-infected zombie classes are fucked?? makes it shoot that entity even
when its dead???? FIXED L0L
*/
bool CanShootWitch(int rage) {
  if (rage == 1)
    return true;
  else
    return false;
  return false;
}
C_BaseEntity *chitscan::GetBestTarget(C_TerrorPlayer *pLocal) {
  C_BaseEntity *BestEntity = nullptr;
  float FOV = FLT_MAX;

  for (const auto pInfected : gEntityCache.GetGroup(EGroupType::INFECTED)) {
    if (!pInfected || pInfected->IsDormant())
      continue;
    if (!pInfected->As<C_Infected *>()->ValidEntity(
            pInfected->As<C_Infected *>()->m_nSequence(),
            pInfected->As<C_Infected *>()->m_usSolidFlags()))
      continue;

    if (Vars::Ignore::IgnoreCommon)
      continue;

    Vector Viewangles;
    I::EngineClient->GetViewAngles(Viewangles);
    Vector vAngleTo =
        U::Math.CalcAngle(pLocal->Weapon_ShootPosition(),
                          GetBestHitbox(pInfected->As<C_Infected *>(), pLocal));
    float GetFOV = CalcFov(Viewangles, vAngleTo);

    if (GetFOV < FOV && GetFOV < Vars::Hitscan::iFov) {

      FOV = GetFOV;
      BestEntity = pInfected->As<C_Infected *>();
    }
  }
  for (const auto Special :
       gEntityCache.GetGroup(EGroupType::SPECIAL_INFECTED)) {
    if (!Special || Special->IsDormant())
      continue;
    C_TerrorPlayer *pPlayer = Special->As<C_TerrorPlayer *>();
    if (!pPlayer)
      continue;

    if (!pPlayer->GetClientClass())
      continue;
    if (!pPlayer->GetClientClass())
      continue;
    int classId = pPlayer->GetClientClass()->m_ClassID;
    if (classId == Boomer && Vars::Hitscan::IgnoreBoomer)
      continue;
    if (classId == Smoker && Vars::Hitscan::IgnoreSmoker)
      continue;
    if (classId == Hunter && Vars::Hitscan::IgnoreHunter)
      continue;
    if (classId == Spitter && Vars::Hitscan::IgnoreSpitter)
      continue;
    if (classId == Jockey && Vars::Hitscan::IgnoreJockey)
      continue;
    if (classId == Charger && Vars::Hitscan::IgnoreCharger)
      continue;
    if (classId == Tank && Vars::Hitscan::IgnoreTank)
      continue;

    if (G::Util.IsInvalid(pLocal, pPlayer))
      continue;
    Vector Viewangles;
    I::EngineClient->GetViewAngles(Viewangles);
    Vector vAngleTo = U::Math.CalcAngle(pLocal->Weapon_ShootPosition(),
                                        GetBestHitbox(pPlayer, pLocal));
    float GetFOV = CalcFov(Viewangles, vAngleTo);

    if (GetFOV < FOV && GetFOV < Vars::Hitscan::iFov) {

      FOV = GetFOV;
      BestEntity = pPlayer;
    }
  }
  for (const auto Witch : gEntityCache.GetGroup(EGroupType::WITCH)) {
    if (!Witch)
      continue;
    C_Witch *pPlayer = Witch->As<C_Witch *>();
    if (!pPlayer)
      continue;
    if (G::Util.IsInvalid(pLocal, pPlayer))
      continue;
    if (Vars::Ignore::IgnoreWitchUntilStartled &&
        !CanShootWitch(pPlayer->m_rage()))
      continue;
    Vector Viewangles;
    I::EngineClient->GetViewAngles(Viewangles);
    Vector vAngleTo = U::Math.CalcAngle(pLocal->Weapon_ShootPosition(),
                                        GetBestHitbox(pPlayer, pLocal));
    float GetFOV = CalcFov(Viewangles, vAngleTo);

    if (GetFOV < FOV && GetFOV < Vars::Hitscan::iFov) {
      FOV = GetFOV;
      BestEntity = pPlayer;
    }
  }
  for (const auto Players : gEntityCache.GetGroup(EGroupType::CTERRORPLAYER)) {
    // yay..
    // we're still using the same retarded IsInfectedAlive function
    // beacuse of versus mode being autistic or something.
    // ..God I fucking hate the l4d2 SDK.
    if (!Players || Players->IsDormant())
      continue;
    if (Players->As<C_TerrorPlayer *>()->m_iTeamNum() == pLocal->m_iTeamNum() &&
        Vars::Ignore::IgnoreTeammates)
      continue;
    // the only difference between this and CBaseEntity's IsValid entity is..
    // well, it doesnt check some stuff or something
    if (!G::Util.IsInfectedAlive(
            Players->As<C_TerrorPlayer *>()->m_usSolidFlags(),
            Players->As<C_TerrorPlayer *>()->m_nSequence(),
            Players->As<C_TerrorPlayer *>()->GetClientClass() &&
                Players->As<C_TerrorPlayer *>()->GetClientClass()->m_ClassID ==
                    CTerrorPlayer)) {
      continue;
    }
    player_info_t pInfo;
    if (!I::EngineClient->GetPlayerInfo(Players->entindex(), &pInfo))
      continue;
    int nIndex = Players->entindex();

    if (gEntityCache.Friends[nIndex] &&
        Vars::Ignore::IgnoreFriends) // if the guy is our friend and if we have
                                     // IgnoreFriends on, return true.
    {
      continue;
    }
    // don't we already check if we're ghost in run?
    // we dont need to check m_iTeamNum for this, I think.

    if (Players->GetClientClass() &&
        Players->GetClientClass()->m_ClassID ==
            CTerrorPlayer) //&& Players->m_iTeamNum() != pLocal->m_iTeamNum())
    {
      if (Players->As<C_TerrorPlayer *>()->m_isGhost())
        continue;
    }
    // retarded fix for that one level
    if (Players->GetClientClass() &&
        Players->GetClientClass()->m_ClassID == SurvivorBot &&
        Players->As<C_TerrorPlayer *>()->m_iTeamNum() == TEAM_INFECTED) {
      player_info_t pInfo;
      if (!I::EngineClient->GetPlayerInfo(Players->entindex(), &pInfo))
        continue;
      if (pInfo.name == "Bill" || pInfo.name == "Francis" ||
          pInfo.name == "Louis" || pInfo.name == "Zoey")
        continue;
    }
    Vector Viewangles;
    I::EngineClient->GetViewAngles(Viewangles);
    Vector vAngleTo = U::Math.CalcAngle(
        pLocal->Weapon_ShootPosition(),
        GetBestHitbox(Players->As<C_TerrorPlayer *>(), pLocal));
    float GetFOV = CalcFov(Viewangles, vAngleTo);

    if (GetFOV < FOV && GetFOV < Vars::Hitscan::iFov) {

      FOV = GetFOV;
      BestEntity = Players->As<C_TerrorPlayer *>(); // woops..
    }
  }
  return BestEntity;
}
inline void FixMovement(CUserCmd *pCmd, Vector m_vOldAngles,
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

// ... (previous code)

void chitscan::Reset() {
  m_lastTarget = nullptr;
  m_lastSwitchTime = 0.0f;
}

void chitscan::run(C_TerrorPlayer *pLocal, CUserCmd *pCmd) {
  /* */
  if (!Vars::Hitscan::bEnable)
    return;
  if (!GetAsyncKeyState(Vars::Hitscan::AimKey.m_Var)) {
    // If we release the key, we should probably reset the target sticking logic
    // too? For now, let's keep it consistent with "switching" logic.
    return;
  }

  auto pEntity = GetBestTarget(pLocal);

  // Use member variables instead of static
  // static C_BaseEntity *lastTarget = nullptr;
  // static float lastSwitchTime = 0.0f;

  // Safety: Reset lastTarget if invalid or no target found
  if (!pEntity) {
    m_lastTarget = nullptr;
  }

  // FIX: Detect if time went backwards (Map Change/Round Restart)
  if (I::EngineClient->GetLastTimeStamp() < m_lastSwitchTime) {
    m_lastSwitchTime = 0.0f;
    m_lastTarget = nullptr;
  }

  if (pEntity != m_lastTarget) {
    if (I::EngineClient->GetLastTimeStamp() - m_lastSwitchTime <
        Vars::Hitscan::TargetSwitchDelay)
      return;
    m_lastSwitchTime = I::EngineClient->GetLastTimeStamp();
    m_lastTarget = pEntity;
  }
  if (!pEntity || !pLocal)
    return;
  if (G::Util.IsInvalid(pLocal, pEntity))
    return;
  if (pLocal->m_iTeamNum() != TEAM_SURVIVOR)
    return; // if we're infected, dont fucking do hitscan aimbot..
  if (pLocal->m_isGhost() || pLocal->IsGhost())
    return;
  Vector hitbox = GetBestHitbox(pEntity, pLocal);
  if (hitbox.IsZero())
    return;
  if (!pLocal->GetActiveWeapon())
    return;
  if (pLocal->GetActiveWeapon()->As<C_TerrorWeapon *>()->GetWeaponID() ==
      WEAPON_MELEE)
    return;
  if (!Pos(pLocal, pEntity, pLocal->Weapon_ShootPosition(),
           GetBestHitbox(pEntity, pLocal)))
    return; // Visibility check

  Vector m_vOldViewAngle = pCmd->viewangles;
  float m_fOldSideMove = pCmd->sidemove;
  float m_fOldForwardMove = pCmd->forwardmove;

  Vector Angle = GetBestAngle(pEntity, pLocal);
  if (Angle.IsZero())
    return;

  if (Vars::Hitscan::bSmoothing) {
    Vector currentAngles;
    I::EngineClient->GetViewAngles(currentAngles);

    Vector delta = Angle - currentAngles;

    // Normalize delta
    if (delta.x > 180.0f)
      delta.x -= 360.0f;
    if (delta.x < -180.0f)
      delta.x += 360.0f;
    if (delta.y > 180.0f)
      delta.y -= 360.0f;
    if (delta.y < -180.0f)
      delta.y += 360.0f;

    float smooth = Vars::Hitscan::fSmoothFactor;
    if (Vars::Hitscan::SmoothMethod == 1) { // Dynamic
      // Randomize smooth slightly to look human
      float random_jitter =
          1.0f + (((rand() % 100) / 100.0f) * 0.4f - 0.2f); // 0.8 to 1.2
      smooth *= random_jitter;
    }

    if (smooth < 1.0f)
      smooth = 1.0f;

    pCmd->viewangles = currentAngles + (delta / smooth);

    // Clamp
    if (pCmd->viewangles.x > 89.0f)
      pCmd->viewangles.x = 89.0f;
    if (pCmd->viewangles.x < -89.0f)
      pCmd->viewangles.x = -89.0f;
    if (pCmd->viewangles.y > 180.0f)
      pCmd->viewangles.y -= 360.0f;
    if (pCmd->viewangles.y < -180.0f)
      pCmd->viewangles.y += 360.0f;
  } else {
    pCmd->viewangles = Angle;
  }

  if (!Vars::Hitscan::bSilentAim)
    I::EngineClient->SetViewAngles(pCmd->viewangles);

  if (Vars::Hitscan::bSilentAim)
    FixMovement(pCmd, m_vOldViewAngle, m_fOldForwardMove, m_fOldSideMove);

  // AutoShoot / Triggerbot Logic
  // User verified: "Make Aimbot Key act as Fire Key" & "Improve Trigger Hitbox"
  // Trigger logic now fires at ANY valid enemy in crosshair, not just the
  // locked target.
  bool bShouldAutoShoot =
      Vars::Hitscan::bAutoShoot || Vars::Hitscan::bWaitForHitbox;

  if (bShouldAutoShoot) {
    Vector forward;
    U::Math.AngleVectors(pCmd->viewangles, &forward);
    Vector start = pLocal->Weapon_ShootPosition();
    Vector end = start + (forward * 8192.f);

    CTraceFilter filter;
    filter.pSkip = pLocal;

    Ray_t ray;
    ray.Init(start, end);

    trace_t tr;
    I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

    if (tr.m_pEnt) {
      // Check if entity is a valid enemy (Infected)
      bool bIsValidEnemy = false;
      if (tr.m_pEnt->IsZombie() && tr.m_pEnt->IsAlive() &&
          tr.m_pEnt->m_iTeamNum() != pLocal->m_iTeamNum()) {
        bIsValidEnemy = true;
      }

      // Also check for Commons if needed (optional, but good for Triggerbot)
      // For now, stick to Special Infected/Players as per Aimbot logic usually
      // But user said "infectado", so assume Specials.

      if (bIsValidEnemy) {
        if (Vars::Hitscan::bWaitForHitbox) {
          int desiredGroup = HITGROUP_HEAD;
          switch (Vars::Hitscan::iHitbox) {
          case 0:
            desiredGroup = HITGROUP_HEAD;
            break;
          case 1:
            desiredGroup = HITGROUP_CHEST;
            break;
          case 2:
            desiredGroup = HITGROUP_STOMACH;
            break;
          }

          // Strict hitbox check
          if (tr.hitgroup == desiredGroup) {
            pCmd->buttons |= IN_ATTACK;
          }
        } else {
          // If not waiting for exact hitbox, just shoot if we hit the enemy
          pCmd->buttons |= IN_ATTACK;
        }
      }
    }
  }
  if (Vars::Misc::DisableInterp && (pCmd->buttons & IN_ATTACK)) {
    static ConVar *cv_interp =
        I::Cvars ? I::Cvars->FindVar("cl_interp") : nullptr;
    static ConVar *cv_interp_ratio =
        I::Cvars ? I::Cvars->FindVar("cl_interp_ratio") : nullptr;
    static ConVar *cv_updaterate =
        I::Cvars ? I::Cvars->FindVar("cl_updaterate") : nullptr;
    if (cv_interp && cv_interp_ratio && cv_updaterate &&
        cv_updaterate->GetFloat() != 0.0f) {
      pCmd->tick_count = TIME_TO_TICKS(
          pEntity->m_flSimulationTime() +
          std::max(cv_interp->GetFloat(),
                   cv_interp_ratio->GetFloat() / cv_updaterate->GetFloat()));
    }
  }
}