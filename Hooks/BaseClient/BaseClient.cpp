#include "BaseClient.h"
#include "../../SDK/EntityCache/entitycache.h"
#include "../TerrorPlayer/TerrorPlayer.h"
using namespace Hooks;

void __fastcall BaseClient::LevelInitPreEntity::Detour(void *ecx, void *edx,
                                                       char const *pMapName) {
  Table.Original<FN>(Index)(ecx, edx, pMapName);
}

void __fastcall BaseClient::LevelInitPostEntity::Detour(void *ecx, void *edx) {
  Table.Original<FN>(Index)(ecx, edx);
}

void __fastcall BaseClient::LevelShutdown::Detour(void *ecx, void *edx) {
  Table.Original<FN>(Index)(ecx, edx);
  gEntityCache.Clear();
  f::hitscan.Reset();
}
#include "../../Features/Hitscan/Hitscan.h"
#include "../../Features/Vars.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
#include "../../SDK/L4D2/Interfaces/IInput.h"
#include "../ClientMode/ClientMode.h"

void __fastcall BaseClient::FrameStageNotify::Detour(
    void *ecx, void *edx, ClientFrameStage_t curStage) {
  switch (curStage) {
  case ClientFrameStage_t::FRAME_RENDER_START: {
    if (!I::EngineClient->IsInGame())
      break;

    // Toggle logic
    static bool bPrevState = false;
    bool isKeyDown =
        GetAsyncKeyState(Vars::Misc::ThirdPersonKey.m_Var) & 0x8000;

    if (isKeyDown && !bPrevState) {
      Vars::Misc::ThirdPerson = !Vars::Misc::ThirdPerson;
      // Removed Native Call (Caused Crash in Versus)
    }
    bPrevState = isKeyDown;

    // SHOTGUN LOGIC (Restored for Body Visibility)
    // We rely on Panels.cpp to force HUD visibility (m_iHideHUD = 0)
    if (I::Input) {
      IClientEntity *pLocalEntity = I::ClientEntityList->GetClientEntity(
          I::EngineClient->GetLocalPlayer());

      bool alive = false;
      if (pLocalEntity) {
        C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();
        if (pLocal && pLocal->IsAlive()) {
          alive = true;
        }
      }

      bool active = (alive && Vars::Misc::ThirdPerson);

      I::Input->m_fCameraInThirdPerson() = active;
      I::Input->m_fCameraInThirdPerson_Old() = active;
      I::Input->m_fCameraInThirdPerson_Alt() = active;
    }

    break;
  }
  default:
    break;
  }

  Table.Original<FN>(Index)(ecx, edx, curStage);

  switch (curStage) {
  case ClientFrameStage_t::FRAME_NET_UPDATE_START: {
    gEntityCache.Clear();
    break;
  }
  case ClientFrameStage_t::FRAME_NET_UPDATE_END: {
    if (!I::EngineClient || !I::EngineClient->IsInGame())
      break;
    IClientEntity *pLocalEntity =
        I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
    if (!pLocalEntity)
      break;
    C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();
    if (!pLocal || !pLocal->m_iTeamNum())
      break;
    gEntityCache.Fill();
    break;
  }
  }
}

void BaseClient::Init() {
  XASSERT(Table.Init(I::BaseClient) == false);
  XASSERT(Table.Hook(&LevelInitPreEntity::Detour, LevelInitPreEntity::Index) ==
          false);
  XASSERT(Table.Hook(&LevelInitPostEntity::Detour,
                     LevelInitPostEntity::Index) == false);
  XASSERT(Table.Hook(&LevelShutdown::Detour, LevelShutdown::Index) == false);
  XASSERT(Table.Hook(&FrameStageNotify::Detour, FrameStageNotify::Index) ==
          false);
}