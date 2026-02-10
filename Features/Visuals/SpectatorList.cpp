#include "SpectatorList.h"
#include "../../SDK/L4D2/Entities/C_BasePlayer.h"
#include "../../SDK/L4D2/Entities/C_TerrorPlayer.h"
#include "../NewMenu/ImGui/imgui.h"
#include "../Vars.h"


void Features::Visuals::SpectatorList::Render() {
  if (!Vars::Visuals::SpectatorList)
    return;

  if (!I::EngineClient || !I::EngineClient->IsInGame())
    return;

  int localIndex = I::EngineClient->GetLocalPlayer();
  if (localIndex == -1)
    return;

  IClientEntity *pLocalEntity =
      I::ClientEntityList->GetClientEntity(localIndex);
  if (!pLocalEntity)
    return;

  // C_TerrorPlayer* pLocal = pLocalEntity->As<C_TerrorPlayer*>(); --
  // CBasePlayer is enough for checks

  // Set window flags
  ImGuiWindowFlags window_flags = 0;
  if (Vars::Menu::Opened)
    window_flags = 0;
  else
    window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoBackground;

  // Use Vars for position? or let ImGui handle it?
  // If menu is closed, we want it fixed or user-positioned?
  // Aimware usually allows moving it in menu.

  if (!Vars::Menu::Opened) {
    ImGui::SetNextWindowPos(ImVec2((float)Vars::Visuals::SpectatorListX,
                                   (float)Vars::Visuals::SpectatorListY),
                            ImGuiCond_Always);
    // Transparent background when menu closed
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.4f));
  } else {
    // Opaque and movable when menu open
    ImGui::SetNextWindowPos(ImVec2((float)Vars::Visuals::SpectatorListX,
                                   (float)Vars::Visuals::SpectatorListY),
                            ImGuiCond_Once);
  }

  if (ImGui::Begin("Spectators", &Vars::Visuals::SpectatorList, window_flags)) {
    // Store position if moved
    if (Vars::Menu::Opened) {
      ImVec2 pos = ImGui::GetWindowPos();
      Vars::Visuals::SpectatorListX = (int)pos.x;
      Vars::Visuals::SpectatorListY = (int)pos.y;
    }

    if (!Vars::Menu::Opened) {
      ImGui::TextColored(ImVec4(1, 1, 1, 1), "Spectators");
      ImGui::Separator();
    }

    bool shownAny = false;
    for (int i = 1; i <= I::EngineClient->GetMaxClients(); i++) {
      if (i == localIndex)
        continue;

      IClientEntity *pEntity = I::ClientEntityList->GetClientEntity(i);
      if (!pEntity)
        continue;

      C_BasePlayer *pPlayer = pEntity->As<C_BasePlayer *>();
      if (!pPlayer || pPlayer->IsDormant())
        continue;

      // Check if spectator
      // Team 1 is spectator
      if (pPlayer->GetTeamNumber() != 1)
        continue;

      // Check observer target
      // Handle m_hObserverTarget
      C_BaseEntity *target = pPlayer->GetObserverTarget();
      if (!target)
        continue;

      // Check if target is ME
      IClientEntity *pTargetClient = target->GetIClientEntity();
      if (pTargetClient && pTargetClient->entindex() == localIndex) {
        player_info_t info;
        if (I::EngineClient->GetPlayerInfo(i, &info)) {
          const char *mode = "Spectating";
          int obsMode = pPlayer->m_iObserverMode();
          switch (obsMode) {
          case 4:
            mode = "First Person";
            break;
          case 5:
            mode = "Third Person";
            break;
          case 6:
            mode = "Free Cam";
            break; // Usually doesn't target entity?
          }

          ImGui::Text("%s - %s", info.name, mode);
          shownAny = true;
        }
      }
    }

    if (!shownAny && Vars::Menu::Opened) {
      ImGui::TextDisabled("No spectators.");
    }
  }
  ImGui::End();

  if (!Vars::Menu::Opened) {
    ImGui::PopStyleColor();
  }
}
