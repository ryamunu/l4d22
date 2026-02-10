#include "ESP.h"
#include "../../SDK/DrawManager/DrawManager.h"
#include "../../SDK/L4D2/Entities/C_BaseAnimating.h"
#include "../../SDK/L4D2/Entities/C_BaseEntity.h"
#include "../../SDK/L4D2/Entities/C_Infected.h"
#include "../../SDK/L4D2/Entities/C_TerrorPlayer.h"
#include "../../SDK/L4D2/Includes/const.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"
#include "../../SDK/L4D2/Interfaces/ICollideable.h"
#include "../../SDK/SDK.h"
#include "../../Util/Math/Math.h"
#include "../Vars.h"
#include <algorithm>

// Helper: Get bone position from bone matrix
inline Vector GetBonePosition(const matrix3x4_t &boneMatrix) {
  return Vector(boneMatrix[0][3], boneMatrix[1][3], boneMatrix[2][3]);
}

// Helper: Draw skeleton using bone parent hierarchy
// DISABLED - mstudiobone_t structure offsets are incorrect for L4D2
// TODO: Research correct L4D2 bone structure and fix skeleton ESP
void DrawSkeleton(C_BaseEntity *pEntity, Color color) {
  // Skeleton ESP temporarily disabled - structure offsets are wrong
  return;

  // Original code preserved below for future reference
  /*
  if (!pEntity || !I::ModelInfo || !I::GlobalVars)
    return;

  C_BaseAnimating *pAnim = pEntity->As<C_BaseAnimating *>();
  if (!pAnim)
    return;

  const model_t *pModel = pEntity->GetModel();
  if (!pModel)
    return;

  studiohdr_t *pStudioHdr = I::ModelInfo->GetStudiomodel(pModel);
  if (!pStudioHdr || pStudioHdr->numbones <= 0)
    return;

  // Get bone matrix
  matrix3x4_t boneMatrix[128];
  if (!pAnim->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX,
                         I::GlobalVars->curtime))
    return;

  // Draw bones using parent hierarchy
  for (int i = 0; i < pStudioHdr->numbones && i < 128; i++) {
    mstudiobone_t *pBone = pStudioHdr->pBone(i);
    if (!pBone)
      continue;

    int parent = pBone->parent;
    // Skip bones without parents (root bones)
    if (parent < 0 || parent >= pStudioHdr->numbones)
      continue;

    // Get bone positions
    Vector pos1 = GetBonePosition(boneMatrix[i]);
    Vector pos2 = GetBonePosition(boneMatrix[parent]);

    // Skip invalid bones (at origin or too far apart)
    Vector origin(0, 0, 0);
    if (pos1.DistTo(origin) < 1.0f || pos2.DistTo(origin) < 1.0f)
      continue;

    float dist = pos1.DistTo(pos2);
    // Skip very small bones (fingers ~3-5 units)
    // Show spine (6-10), arms/legs (10-30 units)
    if (dist < 6.0f || dist > 35.0f)
      continue;

    Vector screen1, screen2;
    if (G::Util.W2S(pos1, screen1) && G::Util.W2S(pos2, screen2)) {
      G::Draw.Line((int)screen1.x, (int)screen1.y, (int)screen2.x,
                   (int)screen2.y, color);
    }
  }
  */ // End of disabled skeleton code
}

// Helper: Get entity box with adjustable height
bool GetEntityBoxWithHeight(C_BaseEntity *pEntity, float entityHeight, int &x,
                            int &y, int &w, int &h) {
  if (!pEntity)
    return false;

  Vector origin = pEntity->GetAbsOrigin();

  Vector screenBottom, screenTop;
  if (!G::Util.W2S(origin, screenBottom))
    return false;

  Vector topPos = origin;
  topPos.z += entityHeight;
  if (!G::Util.W2S(topPos, screenTop))
    return false;

  float height = screenBottom.y - screenTop.y;
  if (height <= 0)
    return false;

  float width = height / 2.0f;

  x = (int)(screenTop.x - width / 2);
  y = (int)screenTop.y;
  w = (int)width;
  h = (int)height;

  // Sanity check - reject unreasonably sized boxes
  if (w <= 0 || h <= 0 || w > 400 || h > 400)
    return false;

  return true;
}

// Helper: Check if entity is near ANY player (equipped weapons are near their
// owners)
bool IsNearAnyPlayer(C_BaseEntity *pEntity) {
  if (!pEntity || !I::ClientEntityList)
    return false;

  Vector itemPos = pEntity->GetAbsOrigin();

  // Iterate through all potential players
  for (int i = 1; i <= 32; i++) {
    IClientEntity *pClientEntity = I::ClientEntityList->GetClientEntity(i);
    if (!pClientEntity)
      continue;

    C_BaseEntity *pPlayer = pClientEntity->As<C_BaseEntity *>();
    if (!pPlayer || pPlayer->IsDormant())
      continue;

    ClientClass *pClass = pPlayer->GetClientClass();
    if (!pClass)
      continue;

    // Only check players (survivors and special infected)
    if (pClass->m_ClassID != CTerrorPlayer && pClass->m_ClassID != SurvivorBot)
      continue;

    // Check if item is very close to this player
    float dist = itemPos.DistTo(pPlayer->GetAbsOrigin());
    if (dist < 100.0f) // Within 100 units = probably equipped
      return true;
  }
  return false;
}

void CFeatures_ESP::Render() {
  // Safety: Check all required interfaces
  if (!I::EngineClient || !I::ClientEntityList || !I::EngineClient->IsInGame())
    return;

  int localPlayerIndex = I::EngineClient->GetLocalPlayer();
  if (localPlayerIndex <= 0)
    return;

  IClientEntity *pLocalEntity =
      I::ClientEntityList->GetClientEntity(localPlayerIndex);
  if (!pLocalEntity)
    return;
  C_BaseEntity *pLocal = pLocalEntity->As<C_BaseEntity *>();
  if (!pLocal)
    return;

  int maxEntities = I::ClientEntityList->GetMaxEntities();
  if (maxEntities <= 0 || maxEntities > 4096)
    return;

  for (int i = 1; i <= maxEntities; i++) {
    if (i == localPlayerIndex) // Skip local player
      continue;

    IClientEntity *pClientEntity = I::ClientEntityList->GetClientEntity(i);
    if (!pClientEntity)
      continue;

    C_BaseEntity *pEntity = pClientEntity->As<C_BaseEntity *>();
    if (!pEntity || pEntity->IsDormant())
      continue;

    ClientClass *pClientClass = pEntity->GetClientClass();
    if (!pClientClass)
      continue;
    const int classId = pClientClass->m_ClassID;

    // Check IgnoreTeammates
    if (Vars::Ignore::IgnoreTeammates) {
      int localTeam = pLocal->GetTeamNumber();
      int entityTeam = pEntity->GetTeamNumber();

      // 1. Ignore Teammates (Survivors vs Survivors, Infected vs Infected)
      if (localTeam == entityTeam &&
          (classId == CTerrorPlayer || classId == SurvivorBot))
        continue;

      // 2. Ignore Common Infected if I am Infected (User request)
      if (localTeam == TEAM_INFECTED && classId == Infected)
        continue;
    }
    if (classId == CTerrorPlayer || classId == SurvivorBot) {
      int teamNum = pEntity->GetTeamNumber();

      // SURVIVOR
      if (teamNum == TEAM_SURVIVOR) {
        if (!Vars::ESP::Survivor.Enabled)
          continue;

        C_TerrorPlayer *pPlayer = pEntity->As<C_TerrorPlayer *>();
        if (!pPlayer || !pPlayer->IsAlive())
          continue;

        int x, y, w, h;
        if (!GetEntityBoxWithHeight(pEntity, 72.0f, x, y, w, h))
          continue;

        // Box
        if (Vars::ESP::Survivor.Box) {
          G::Draw.OutlinedRect(x, y, w, h, Vars::ESP::Survivor.MainColor);
          G::Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(0, 0, 0, 255));
        }

        // Health Bar
        if (Vars::ESP::Survivor.Healthbar) {
          int health = pPlayer->GetHealth();
          int maxHealth = pPlayer->GetMaxHealth();
          if (maxHealth <= 0)
            maxHealth = 100;
          if (health > maxHealth)
            health = maxHealth;

          int barH = (int)((float)h * ((float)health / (float)maxHealth));
          int barY = y + (h - barH);

          G::Draw.Rect(x - 6, y - 1, 4, h + 2, Color(0, 0, 0, 255));
          Color hpColor = Color(0, 255, 0, 255);
          if (health < maxHealth * 0.5)
            hpColor = Color(255, 255, 0, 255);
          if (health < maxHealth * 0.2)
            hpColor = Color(255, 0, 0, 255);
          G::Draw.Rect(x - 5, barY, 2, barH, hpColor);
        }

        // Numeric HP
        if (Vars::ESP::Survivor.NumericHP) {
          char hpText[16];
          sprintf_s(hpText, "%d HP", pEntity->GetHealth());
          G::Draw.String(EFonts::ESP, x + w + 4, y, Color(255, 255, 255, 255),
                         TXT_DEFAULT, hpText);
        }

        // Name
        if (Vars::ESP::Survivor.Name) {
          player_info_t info;
          if (I::EngineClient->GetPlayerInfo(i, &info)) {
            G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                           Color(255, 255, 255, 255), TXT_CENTERX, "%s",
                           info.name);
          }
        }

        // Skeleton
        if (Vars::ESP::Survivor.Skeleton) {
          DrawSkeleton(pEntity, Vars::ESP::Survivor.MainColor);
        }
        continue;
      }

      // SPECIAL INFECTED (player-controlled)
      else if (teamNum == TEAM_INFECTED) {
        if (!Vars::ESP::Infected.Enabled)
          continue;

        C_TerrorPlayer *pPlayer = pEntity->As<C_TerrorPlayer *>();
        if (!pPlayer || !pPlayer->IsAlive())
          continue;

        // Check if ghost mode (spawning as infected)
        bool isGhost = pPlayer->m_isGhost();

        // Check if Tank
        bool isTank = false;
        if (!isGhost && I::ModelInfo) {
          const model_t *model = pEntity->GetModel();
          if (model) {
            const char *mName = I::ModelInfo->GetModelName(model);
            if (mName && strstr(mName, "hulk")) {
              isTank = true;
            }
          }
        }

        // Select appropriate settings
        C_ESP_Settings *settings;
        if (isGhost) {
          settings = &Vars::ESP::Ghost;
        } else if (isTank) {
          settings = &Vars::ESP::Tank;
        } else {
          settings = &Vars::ESP::Infected;
        }
        if (!settings->Enabled)
          continue;

        int x, y, w, h;
        float height = isGhost ? 72.0f : (isTank ? 100.0f : 72.0f);
        if (!GetEntityBoxWithHeight(pEntity, height, x, y, w, h))
          continue;

        if (settings->Box) {
          G::Draw.OutlinedRect(x, y, w, h, settings->MainColor);
          G::Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(0, 0, 0, 255));
        }

        if (settings->Healthbar && !isGhost) {
          int health = pPlayer->GetHealth();
          int maxHealth = pPlayer->GetMaxHealth();
          if (maxHealth <= 0)
            maxHealth = isTank ? 6000 : 100;
          if (health > maxHealth)
            health = maxHealth;

          // Clamp height to reasonable values
          int clampedH = h;
          if (clampedH > 200)
            clampedH = 200;
          if (clampedH < 10)
            clampedH = 10;

          int barH =
              (int)((float)clampedH * ((float)health / (float)maxHealth));
          if (barH < 0)
            barH = 0;
          if (barH > clampedH)
            barH = clampedH;
          int barY = y + (clampedH - barH);

          G::Draw.Rect(x - 5, y, 3, clampedH, Color(0, 0, 0, 200));
          Color hpColor = Color(0, 255, 0, 255);
          if (health < maxHealth * 0.5)
            hpColor = Color(255, 255, 0, 255);
          if (health < maxHealth * 0.2)
            hpColor = Color(255, 0, 0, 255);
          G::Draw.Rect(x - 4, barY, 2, barH, hpColor);
        }

        if (settings->NumericHP && !isGhost) {
          char hpText[16];
          sprintf_s(hpText, "%d HP", pEntity->GetHealth());
          G::Draw.String(EFonts::ESP, x + w + 4, y, Color(255, 255, 255, 255),
                         TXT_DEFAULT, hpText);
        }

        // Tank Frustration Bar (right side, orange) - only for player tanks
        if (isTank && Vars::ESP::TankFrustration && !isGhost) {
          int frustration = pPlayer->m_frustration();
          if (frustration > 100)
            frustration = 100;
          if (frustration < 0)
            frustration = 0;

          // Draw bar on right side (after health text position)
          int barX = x + w + 8;
          int barH = (int)((float)h * ((float)frustration / 100.0f));
          int barY = y + (h - barH);

          Color frustColor = Color(255, 165, 0, 255); // Orange
          G::Draw.Rect(barX, y - 1, 5, h + 2,
                       Color(0, 0, 0, 255));                 // Wider background
          G::Draw.Rect(barX + 1, barY, 3, barH, frustColor); // Wider bar

          // Safe String: Frustration
          char frustText[16];
          sprintf_s(frustText, "%d%%", frustration);
          G::Draw.String(EFonts::ESP, barX + 8, y + h / 2 - 6, frustColor,
                         TXT_DEFAULT, "%s", frustText);
        }

        // Name - use Steam name for players with ghost indicator
        if (settings->Name) {
          player_info_t info;
          if (I::EngineClient->GetPlayerInfo(i, &info)) {
            if (isGhost) {
              char ghostName[128];
              sprintf_s(ghostName, "[GHOST] %s", info.name);
              G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                             settings->MainColor, TXT_CENTERX, "%s", ghostName);
            } else {
              G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                             Color(255, 255, 255, 255), TXT_CENTERX, "%s",
                             info.name);
            }
          } else {
            G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                           Color(255, 255, 255, 255), TXT_CENTERX,
                           isTank ? "Tank"
                                  : (isGhost ? "Ghost" : "S.Infected"));
          }
        }

        // Skeleton
        if (settings->Skeleton) {
          DrawSkeleton(pEntity, settings->MainColor);
        }
        continue;
      }
    }

    // ========== COMMON INFECTED ==========
    else if (classId == Infected) {
      if (!Vars::ESP::Common.Enabled)
        continue;

      C_Infected *pInfected = pEntity->As<C_Infected *>();
      if (!pInfected)
        continue;

      // Check if valid (not dead)
      if (!pInfected->ValidEntity(pInfected->m_nSequence(),
                                  pInfected->m_usSolidFlags()))
        continue;

      int x, y, w, h;
      if (!GetEntityBoxWithHeight(pEntity, 60.0f, x, y, w, h))
        continue;

      if (Vars::ESP::Common.Box) {
        G::Draw.OutlinedRect(x, y, w, h, Vars::ESP::Common.MainColor);
      }

      if (Vars::ESP::Common.Name) {
        G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                       Color(255, 255, 255, 255), TXT_CENTERX, "Common");
      }

      // Distance
      if (Vars::ESP::Common.Distance) {
        float dist = pLocal->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin());
        char distText[32];
        sprintf_s(distText, "%.0fm", dist / 52.49f); // Units to meters
        G::Draw.String(EFonts::ESP, x + w / 2, y + h + 2,
                       Color(255, 255, 255, 255), TXT_CENTERX, distText);
      }

      // Skeleton
      if (Vars::ESP::Common.Skeleton) {
        DrawSkeleton(pEntity, Vars::ESP::Common.MainColor);
      }
      continue;
    }

    // ========== AI SPECIAL INFECTED (Campaign Bots) ==========
    else if (classId == Hunter || classId == Smoker || classId == Boomer ||
             classId == Jockey || classId == Charger || classId == Spitter) {
      if (!Vars::ESP::Infected.Enabled)
        continue;

      // Skip dead infected
      if (pEntity->GetHealth() <= 0)
        continue;

      int x, y, w, h;
      if (!GetEntityBoxWithHeight(pEntity, 72.0f, x, y, w, h))
        continue;

      if (Vars::ESP::Infected.Box) {
        G::Draw.OutlinedRect(x, y, w, h, Vars::ESP::Infected.MainColor);
        G::Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(0, 0, 0, 255));
      }

      // Health Bar
      if (Vars::ESP::Infected.Healthbar) {
        int health = pEntity->GetHealth();
        int maxHealth = pEntity->GetMaxHealth();
        if (maxHealth <= 0)
          maxHealth = 100;
        if (health > maxHealth)
          health = maxHealth;

        int barH = (int)((float)h * ((float)health / (float)maxHealth));
        int barY = y + (h - barH);

        G::Draw.Rect(x - 6, y - 1, 4, h + 2, Color(0, 0, 0, 255));
        Color hpColor = Color(255, 50, 50, 255);
        G::Draw.Rect(x - 5, barY, 2, barH, hpColor);
      }

      // Name based on class ID
      if (Vars::ESP::Infected.Name) {
        const char *siName = "S.Infected";
        if (classId == Hunter)
          siName = "Hunter";
        else if (classId == Smoker)
          siName = "Smoker";
        else if (classId == Boomer)
          siName = "Boomer";
        else if (classId == Jockey)
          siName = "Jockey";
        else if (classId == Charger)
          siName = "Charger";
        else if (classId == Spitter)
          siName = "Spitter";

        G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                       Color(255, 100, 100, 255), TXT_CENTERX, siName);
      }

      // Numeric HP
      if (Vars::ESP::Infected.NumericHP) {
        char hpText[16];
        sprintf_s(hpText, "%d HP", pEntity->GetHealth());
        G::Draw.String(EFonts::ESP, x + w + 4, y, Color(255, 255, 255, 255),
                       TXT_DEFAULT, hpText);
      }

      // Distance
      if (Vars::ESP::Infected.Distance) {
        float dist = pLocal->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin());
        char distText[32];
        sprintf_s(distText, "%.0fm", dist / 52.49f);
        G::Draw.String(EFonts::ESP, x + w / 2, y + h + 2,
                       Color(255, 255, 255, 255), TXT_CENTERX, distText);
      }

      // Skeleton
      if (Vars::ESP::Infected.Skeleton) {
        DrawSkeleton(pEntity, Vars::ESP::Infected.MainColor);
      }
      continue;
    }

    // ========== WITCH ==========
    else if (classId == Witch) {
      if (!Vars::ESP::Witch.Enabled)
        continue;

      // Use proper witch validity check (same as entitycache)
      C_Witch *pWitch = pEntity->As<C_Witch *>();
      if (!pWitch)
        continue;
      if (pWitch->m_usSolidFlags() & SolidFlags_t::FSOLID_NOT_SOLID)
        continue; // Dead witch
      if (pWitch->m_nSequence() > 70)
        continue; // Invalid sequence

      int x, y, w, h;
      if (!GetEntityBoxWithHeight(pEntity, 56.0f, x, y, w, h))
        continue;

      if (Vars::ESP::Witch.Box) {
        G::Draw.OutlinedRect(x, y, w, h, Vars::ESP::Witch.MainColor);
      }

      if (Vars::ESP::Witch.Name) {
        G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                       Color(255, 255, 255, 255), TXT_CENTERX, "Witch");
      }

      // Skeleton
      if (Vars::ESP::Witch.Skeleton) {
        DrawSkeleton(pEntity, Vars::ESP::Witch.MainColor);
      }
      continue;
    }

    // ========== AI TANK ==========
    else if (classId == Tank) {
      if (!Vars::ESP::Tank.Enabled)
        continue;

      // Skip dead tanks
      if (pEntity->GetHealth() <= 0)
        continue;

      int x, y, w, h;
      // Tank is hunched - use smaller height estimate (72 units)
      if (!GetEntityBoxWithHeight(pEntity, 72.0f, x, y, w, h))
        continue;

      if (Vars::ESP::Tank.Box) {
        G::Draw.OutlinedRect(x, y, w, h, Vars::ESP::Tank.MainColor);
        G::Draw.OutlinedRect(x - 1, y - 1, w + 2, h + 2, Color(0, 0, 0, 255));
      }

      // TEMPORARILY DISABLED - Tank health bar causing issues
      // TODO: Fix health bar size issue and re-enable
      /*
      if (Vars::ESP::Tank.Healthbar) {
        int health = pEntity->GetHealth();
        int maxHealth = 6000;
        if (health > maxHealth)
          health = maxHealth;
        if (health < 0)
          health = 0;

        // Clamp h to reasonable values
        int clampedH = h;
        if (clampedH > 300)
          clampedH = 300;
        if (clampedH < 10)
          clampedH = 10;

        int barH = (int)((float)clampedH * ((float)health / (float)maxHealth));
        if (barH < 0)
          barH = 0;
        if (barH > clampedH)
          barH = clampedH;
        int barY = y + (clampedH - barH);

        // Draw smaller, proportional bar
        G::Draw.Rect(x - 5, y, 3, clampedH, Color(0, 0, 0, 200)); // Background
        Color hpColor = Color(150, 0, 255, 255);
        G::Draw.Rect(x - 4, barY, 2, barH, hpColor);
      }
      */

      if (Vars::ESP::Tank.NumericHP) {
        char hpText[16];
        sprintf_s(hpText, "%d HP", pEntity->GetHealth());
        G::Draw.String(EFonts::ESP, x + w + 4, y, Color(255, 255, 255, 255),
                       TXT_DEFAULT, hpText);
      }

      // Note: Tank Frustration only works for player-controlled tanks (handled
      // in CTerrorPlayer section) AI tanks don't have m_frustration property

      if (Vars::ESP::Tank.Name) {
        G::Draw.String(EFonts::ESP, x + w / 2, y - 12,
                       Color(255, 255, 255, 255), TXT_CENTERX, "Tank");
      }

      // Skeleton - TEMPORARILY DISABLED for Tank to prevent crashes
      // TODO: Fix skeleton struct offsets and re-enable
      /*
      if (Vars::ESP::Tank.Skeleton) {
        DrawSkeleton(pEntity, Vars::ESP::Tank.MainColor);
      }
      */
      continue;
    }

    // ========== ITEMS ESP ==========
    if (!Vars::ESP::ItemsEnabled)
      continue;

    Vector itemPos = pEntity->GetAbsOrigin();
    Vector localPos = pLocal->GetAbsOrigin();
    float distToLocal = itemPos.DistTo(localPos);
    if (distToLocal > Vars::ESP::ItemsMaxDistance)
      continue;

    // Skip items that are near any player (equipped weapons)
    if (IsNearAnyPlayer(pEntity))
      continue;

    std::string itemName = "";
    switch (classId) {
    // Medical items
    case CFirstAidKit:
      if (Vars::ESP::ItemsMedkits)
        itemName = "Medkit";
      break;
    case CPainPills:
      if (Vars::ESP::ItemsPills)
        itemName = "Pills";
      break;
    case CItem_Adrenaline:
      if (Vars::ESP::ItemsAdrenaline)
        itemName = "Adrenaline";
      break;
    case CItemDefibrillator:
      if (Vars::ESP::ItemsDefib)
        itemName = "Defib";
      break;

    // Props
    case CGasCan:
      if (Vars::ESP::ItemsProps)
        itemName = "Gas Can";
      break;
    case CPropaneTank:
      if (Vars::ESP::ItemsProps)
        itemName = "Propane";
      break;
    case COxygenTank:
      if (Vars::ESP::ItemsProps)
        itemName = "Oxygen Tank";
      break;
    case CFireworkCrate:
      if (Vars::ESP::ItemsProps)
        itemName = "Fireworks";
      break;

    // SMGs
    case CSubMachinegun:
      if (Vars::ESP::ItemsWeapons)
        itemName = "SMG";
      break;
    case CSMG_Silenced:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Silenced SMG";
      break;
    case CSMG_MP5:
      if (Vars::ESP::ItemsWeapons)
        itemName = "MP5";
      break;

    // Shotguns
    case CPumpShotgun:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Pump Shotgun";
      break;
    case CShotgun_Chrome:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Chrome Shotgun";
      break;
    case CAutoShotgun:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Auto Shotgun";
      break;
    case CShotgun_SPAS:
      if (Vars::ESP::ItemsWeapons)
        itemName = "SPAS-12";
      break;

    // Rifles
    case CRifle_AK47:
      if (Vars::ESP::ItemsWeapons)
        itemName = "AK-47";
      break;
    case CRifle_Desert:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Desert Rifle";
      break;
    case CRifle_SG552:
      if (Vars::ESP::ItemsWeapons)
        itemName = "SG552";
      break;
    case CRifle_M60:
      if (Vars::ESP::ItemsWeapons)
        itemName = "M60";
      break;

    // Snipers
    case CSniperRifle:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Sniper";
      break;
    case CSniper_Military:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Military Sniper";
      break;
    case CSniper_Scout:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Scout";
      break;
    case CSniper_AWP:
      if (Vars::ESP::ItemsWeapons)
        itemName = "AWP";
      break;

    // Pistols
    case CPistol:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Pistol";
      break;
    case CMagnumPistol:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Magnum";
      break;

    // Special
    case CChainsaw:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Chainsaw";
      break;
    case CGrenadeLauncher:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Grenade Launcher";
      break;
    case CWeaponSpawn:
      if (Vars::ESP::ItemsWeapons)
        itemName = "Weapon";
      break;

    default:
      continue;
    }

    if (itemName.empty())
      continue;

    // Just draw item name at the item's screen position (no box)
    Vector screenPos;
    if (!G::Util.W2S(pEntity->GetAbsOrigin(), screenPos))
      continue;

    G::Draw.String(EFonts::ESP, (int)screenPos.x, (int)screenPos.y - 12,
                   Color(255, 255, 255, 255), TXT_CENTERX, "%s",
                   itemName.c_str());
  }
}