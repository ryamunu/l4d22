#include "Panels.h"
#include "../../Features/NewMenu/CMenu.h"
#include "../../Features/Vars.h"
#include "../../SDK/L4D2/Entities/C_TerrorPlayer.h"
#include "../../SDK/L4D2/Interfaces/ClientEntityList.h"
#include "../../SDK/L4D2/Interfaces/EngineClient.h"


using namespace Hooks;

void __fastcall Panels::PaintTraverse::Detour(void *ecx, void *edx,
                                              unsigned int vguiPanel,
                                              bool forceRepaint,
                                              bool allowForce) {
  static unsigned int vguiFocusOverlayPanel;
  if (vguiFocusOverlayPanel == NULL) {
    const char *szName = I::VGuiPanel->GetName(vguiPanel);
    if (szName[0] == 'F' && szName[5] == 'O' && szName[12] == 'P') {
      vguiFocusOverlayPanel = vguiPanel;
    }
  }

  if (vguiFocusOverlayPanel == vguiPanel) {
    I::VGuiPanel->SetMouseInputEnabled(vguiPanel, g_Menu.menuOpen);

    // Force HUD Visible in Third Person
    if (Vars::Misc::ThirdPerson && I::EngineClient->IsInGame()) {
      IClientEntity *pLocalEntity = I::ClientEntityList->GetClientEntity(
          I::EngineClient->GetLocalPlayer());
      if (pLocalEntity) {
        C_TerrorPlayer *pLocal = pLocalEntity->As<C_TerrorPlayer *>();
        if (pLocal) {
          pLocal->m_iHideHUD() = 0; // Force flag to 0 (Show All)
        }
      }
    }
  }

  Table.Original<FN>(index)(ecx, edx, vguiPanel, forceRepaint, allowForce);
}

void Panels::Init() {
  XASSERT(Table.Init(I::VGuiPanel) == false);
  XASSERT(Table.Hook(&PaintTraverse::Detour, PaintTraverse::index) == false);
}