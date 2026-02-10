#include "ModelRender.h"
#include "../../Features/Vars.h"
using namespace Hooks;

void __fastcall ModelRender::ForcedMaterialOverride::Detour(
    void *ecx, void *edx, IMaterial *newMaterial,
    OverrideType_t nOverrideType) {
  Table.Original<FN>(Index)(ecx, edx, newMaterial, nOverrideType);
}

void OverridematerialXQZ(IMaterial *mat, float r, float g, float b) {
  mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
  mat->ColorModulate(r / 255, g / 255, b / 255);
  I::ModelRender->ForcedMaterialOverride(mat);
}

void Overridematerial(IMaterial *mat, float r, float g, float b) {
  mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);
  mat->SetMaterialVarFlag(MATERIAL_VAR_ZNEARER, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOCULL, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_NOFOG, true);
  mat->SetMaterialVarFlag(MATERIAL_VAR_HALFLAMBERT, true);
  mat->ColorModulate(r / 255, g / 255, b / 255);
  I::ModelRender->ForcedMaterialOverride(mat);
}

void __fastcall ModelRender::DrawModelExecute::Detour(
    void *ecx, void *edx, const DrawModelState_t &state,
    const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld) {
  if (!I::EngineClient->IsInGame())
    Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
  static IMaterial *material = I::MaterialSystem->FindMaterial(
      "debug/debugambientcube", "Model textures");
  static IMaterial *vomitboomer = I::MaterialSystem->FindMaterial(
      ("particle/screenspaceboomervomit"), "Particle textures");

  if (pInfo.pModel && pInfo.entity_index && material) {
    if (vomitboomer) {
      vomitboomer->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW,
                                      Vars::Removals::BoomerVisual);
    }
    IClientEntity *pClientEntity =
        I::ClientEntityList->GetClientEntity(pInfo.entity_index);
    if (!pClientEntity)
      return Table.Original<FN>(Index)(ecx, edx, state, pInfo,
                                       pCustomBoneToWorld);
    C_BaseEntity *pEntity = pClientEntity->As<C_BaseEntity *>();
    if (pEntity && !pEntity->IsDormant()) {
      if (pEntity->GetClientClass()) {
        IClientEntity *pLocalEntity = I::ClientEntityList->GetClientEntity(
            I::EngineClient->GetLocalPlayer());
        C_TerrorPlayer *pLocal =
            pLocalEntity ? pLocalEntity->As<C_TerrorPlayer *>() : nullptr;

        bool shouldIgnore = false;
        if (pLocal && Vars::Ignore::IgnoreTeammates) {
          int localTeam = pLocal->GetTeamNumber();
          int entityTeam =
              pEntity->GetTeamNumber(); // Works for Survivors/Special Infected

          // 1. Ignore Teammates
          if (pEntity->GetClientClass()->m_ClassID == SurvivorBot ||
              pEntity->GetClientClass()->m_ClassID == CTerrorPlayer) {
            if (localTeam == entityTeam)
              shouldIgnore = true;
          }
          // 2. Ignore Common Infected if I am Infected
          if (localTeam == TEAM_INFECTED &&
              pEntity->GetClientClass()->m_ClassID == Infected) {
            shouldIgnore = true;
          }
        }

        // Check for Survivors and Infected Players
        if (!shouldIgnore &&
            (pEntity->GetClientClass()->m_ClassID == SurvivorBot ||
             pEntity->GetClientClass()->m_ClassID == CTerrorPlayer)) {
          const bool bIsSurvivor =
              (pEntity->As<C_TerrorPlayer *>()->GetTeamNumber() ==
               TEAM_SURVIVOR);

          bool shouldDraw = false;
          if (bIsSurvivor && Vars::Chams::Survivors)
            shouldDraw = true;
          if (!bIsSurvivor && Vars::Chams::InfectedPlayers)
            shouldDraw = true;

          if (shouldDraw) {
            const Color clrTeam = bIsSurvivor
                                      ? Vars::Chams::PlayerColor
                                      : Vars::Chams::PlayerInfectedColor;
            if (pEntity->As<C_TerrorPlayer *>()->IsAlive()) {
              OverridematerialXQZ(material, clrTeam.r(), clrTeam.g(),
                                  clrTeam.b());
              Table.Original<FN>(Index)(ecx, edx, state, pInfo,
                                        pCustomBoneToWorld);
            }
            Overridematerial(material, clrTeam.r(), clrTeam.g(), clrTeam.b());
          }
        }
        if (!shouldIgnore && Vars::Chams::Infected) {
          if (pEntity->IsZombie()) {
            // Handle Common Infected
            if (pEntity->GetClientClass()->m_ClassID == Infected) {
              if (pEntity->ValidEntity(
                      pEntity->As<C_Infected *>()->m_nSequence(),
                      pEntity->As<C_Infected *>()->m_usSolidFlags())) {
                OverridematerialXQZ(material, Vars::Chams::InfectedColor.r(),
                                    Vars::Chams::InfectedColor.g(),
                                    Vars::Chams::InfectedColor.b());
                Table.Original<FN>(Index)(ecx, edx, state, pInfo,
                                          pCustomBoneToWorld);
              }
              Overridematerial(material, Vars::Chams::InfectedColor.r(),
                               Vars::Chams::InfectedColor.g(),
                               Vars::Chams::InfectedColor.b());
            }
            // Handle Special Infected and Bosses
            else {
              Color chamsColor = Vars::Chams::InfectedColor; // Default
              bool shouldDraw = false;

              int classId = pEntity->GetClientClass()->m_ClassID;
              switch (classId) {
              case Tank:
                if (Vars::Chams::Tank) {
                  chamsColor = Vars::Chams::TankColor;
                  shouldDraw = true;
                }
                break;
              case Witch:
                if (Vars::Chams::Witch) {
                  chamsColor = Vars::Chams::WitchColor;
                  shouldDraw = true;
                }
                break;
              case Boomer:
                if (Vars::Chams::Boomer) {
                  chamsColor = Vars::Chams::BoomerColor;
                  shouldDraw = true;
                }
                break;
              case Hunter:
                if (Vars::Chams::Hunter) {
                  chamsColor = Vars::Chams::HunterColor;
                  shouldDraw = true;
                }
                break;
              case Smoker:
                if (Vars::Chams::Smoker) {
                  chamsColor = Vars::Chams::SmokerColor;
                  shouldDraw = true;
                }
                break;
              case Jockey:
                if (Vars::Chams::Jockey) {
                  chamsColor = Vars::Chams::JockeyColor;
                  shouldDraw = true;
                }
                break;
              case Charger:
                if (Vars::Chams::Charger) {
                  chamsColor = Vars::Chams::ChargerColor;
                  shouldDraw = true;
                }
                break;
              case Spitter:
                if (Vars::Chams::Spitter) {
                  chamsColor = Vars::Chams::SpitterColor;
                  shouldDraw = true;
                }
                break;
              }

              if (shouldDraw) {
                OverridematerialXQZ(material, chamsColor.r(), chamsColor.g(),
                                    chamsColor.b());
                Table.Original<FN>(Index)(ecx, edx, state, pInfo,
                                          pCustomBoneToWorld);
                Overridematerial(material, chamsColor.r(), chamsColor.g(),
                                 chamsColor.b());
              }
            }
          }
        }
      }
    }
  }

  Table.Original<FN>(Index)(ecx, edx, state, pInfo, pCustomBoneToWorld);
  I::ModelRender->ForcedMaterialOverride(nullptr);
}

void ModelRender::Init() {
  XASSERT(Table.Init(I::ModelRender) == false);
  XASSERT(Table.Hook(&ForcedMaterialOverride::Detour,
                     ForcedMaterialOverride::Index) == false);
  XASSERT(Table.Hook(&DrawModelExecute::Detour, DrawModelExecute::Index) ==
          false);
}