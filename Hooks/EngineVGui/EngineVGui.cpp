#include "EngineVGui.h"

#include "../../Features/ESP/ESP.h"
#include "../../Features/Vars.h"
#include "../../SDK/L4D2/Interfaces/IConVar.h"
using namespace Hooks;

unsigned int __fastcall EngineVGui::GetPanel::Detour(void *ecx, void *edx,
                                                     VGuiPanel_t type) {
  return Table.Original<FN>(Index)(ecx, edx, type);
}

bool __fastcall EngineVGui::IsGameUIVisible::Detour(void *ecx, void *edx) {
  return Table.Original<FN>(Index)(ecx, edx);
}

void __fastcall EngineVGui::ActivateGameUI::Detour(void *ecx, void *edx) {
  Table.Original<FN>(Index)(ecx, edx);
}

void __fastcall EngineVGui::Paint::Detour(void *ecx, void *edx, int mode) {
  Table.Original<FN>(Index)(ecx, edx, mode);

  if (!(mode & PAINT_UIPANELS))
    return;

  // Safety checks for interfaces
  if (!I::BaseClient || !I::MatSystemSurface || !I::EngineClient)
    return;

  if (!G::Draw.m_nScreenW)
    G::Draw.m_nScreenW = I::BaseClient->GetScreenWidth();

  if (!G::Draw.m_nScreenH)
    G::Draw.m_nScreenH = I::BaseClient->GetScreenHeight();

  // Don't render if screen size is invalid
  if (G::Draw.m_nScreenW <= 0 || G::Draw.m_nScreenH <= 0)
    return;

  I::MatSystemSurface->StartDrawing();
  {
    if (I::EngineClient->IsInGame()) {
      // FOV Circle (only if enabled)
      if (Vars::Hitscan::AimFovCircle) {
        static ConVar *fov =
            I::Cvars ? I::Cvars->FindVar("fov_desired") : nullptr;
        if (fov) {
          float FovDesired = fov->GetFloat();
          if (FovDesired > 0.0f) {
            float flR = tanf(DEG2RAD(Vars::Hitscan::iFov) / 2.0f) /
                        tanf(DEG2RAD(FovDesired) / 2.0f) * G::Draw.m_nScreenW;
            Color clr = Vars::Hitscan::AimFov;
            G::Draw.OutlinedCircle(G::Draw.m_nScreenW / 2,
                                   G::Draw.m_nScreenH / 2, flR, 68, clr);
          }
        }
      }
      // ESP (with safety checks in Render function)
      F::ESP.Render();
    }
  }
  I::MatSystemSurface->FinishDrawing();
}

void EngineVGui::Init() {
  XASSERT(Table.Init(I::EngineVGui) == false);
  XASSERT(Table.Hook(&GetPanel::Detour, GetPanel::Index) == false);
  XASSERT(Table.Hook(&IsGameUIVisible::Detour, IsGameUIVisible::Index) ==
          false);
  XASSERT(Table.Hook(&ActivateGameUI::Detour, ActivateGameUI::Index) == false);
  XASSERT(Table.Hook(&Paint::Detour, Paint::Index) == false);
}