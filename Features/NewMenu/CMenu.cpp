#include "CMenu.h"
#include "../../Util/Config.h"
#include "../Vars.h"
#include "../Visuals/SpectatorList.h"
#include "font_awesome_5.h"
#include <algorithm>

static int tab = 0;
auto MenuSize = ImVec2{1000, 550};

#define FixSlider ImGui::PushItemWidth(ImGui::GetCurrentWindow()->Size.x - 16)

ImVec4 to_vec4(float r, float g, float b, float a) {
  return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

ImVec4 mColor(Color color) {
  return to_vec4(color.r(), color.g(), color.b(), color.a());
}

Color vColor(ImVec4 color) {
  return {(byte)(color.x * 256.0f > 255 ? 255 : color.x * 256.0f),
          (byte)(color.y * 256.0f > 255 ? 255 : color.y * 256.0f),
          (byte)(color.z * 256.0f > 255 ? 255 : color.z * 256.0f),
          (byte)(color.w * 256.0f > 255 ? 255 : color.w * 256.0f)};
}

void ColorPicker(const char *label, Color &color, bool alpha = true) {
  ImVec4 col = mColor(color);
  ImGui::PushItemWidth(150);
  if (alpha) {
    if (ImGui::ColorEdit4(label, &col.x,
                          ImGuiColorEditFlags_NoInputs |
                              ImGuiColorEditFlags_NoLabel |
                              ImGuiColorEditFlags_AlphaPreview))
      color = vColor(col);
  } else {
    if (ImGui::ColorEdit3(label, &col.x,
                          ImGuiColorEditFlags_NoInputs |
                              ImGuiColorEditFlags_NoLabel))
      color = vColor(col);
  }
  ImGui::PopItemWidth();
}

bool InputKeybind(const char *label, CVar<int> &output,
                  bool bAllowNone = true) {
  auto VK2STR = [&](const short key) -> const char * {
    switch (key) {
    case VK_LBUTTON:
      return "M1";
    case VK_RBUTTON:
      return "M2";
    case VK_MBUTTON:
      return "M3";
    case VK_XBUTTON1:
      return "M4";
    case VK_XBUTTON2:
      return "M5";
    case VK_SPACE:
      return "SPC";
    case 0x0:
      return "None";
    case VK_SHIFT:
    case VK_LSHIFT:
    case VK_RSHIFT:
      return "SHFT";
    case VK_CONTROL:
      return "CTRL";
    case VK_MENU:
      return "ALT";
    default:
      if (key >= 'A' && key <= 'Z') {
        static char buf[2];
        buf[0] = (char)key;
        buf[1] = 0;
        return buf;
      }
      if (key >= '0' && key <= '9') {
        static char buf[2];
        buf[0] = (char)key;
        buf[1] = 0;
        return buf;
      }
      break;
    }
    return "?";
  };

  const auto id = ImGui::GetID(label);
  ImGui::PushID(label);

  if (ImGui::GetActiveID() == id) {
    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImGui::GetColorU32(ImGuiCol_ButtonActive));
    ImGui::Button("...", ImVec2(45, 17));
    ImGui::PopStyleColor();

    for (short n = 0; n < 256; n++) {
      if ((n > 0x0 && n < 0x7) || (n > 'A' - 1 && n < 'Z' + 1) ||
          (n > '0' - 1 && n < '9' + 1) || n == VK_LSHIFT || n == VK_RSHIFT ||
          n == VK_SHIFT || n == VK_ESCAPE || n == VK_CONTROL || n == VK_MENU) {
        if (!ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[0]) {
          ImGui::ClearActiveID();
          break;
        }
        if (GetAsyncKeyState(n) & 0x8000) {
          if (n == VK_ESCAPE && bAllowNone) {
            output.m_Var = 0x0;
            ImGui::ClearActiveID();
            break;
          }
          if (n != VK_INSERT && n != VK_HOME) {
            output.m_Var = n;
            ImGui::ClearActiveID();
            break;
          }
        }
      }
    }
  } else if (ImGui::Button(VK2STR(output.m_Var), ImVec2(45, 17))) {
    ImGui::SetActiveID(id, ImGui::GetCurrentWindow());
  }

  ImGui::PopID();
  return true;
}

void ZeniiwareStyle() {
  ImGuiStyle &s = ImGui::GetStyle();
  s.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);
  s.Colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
  s.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);
  s.Colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  s.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
  s.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  s.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
  s.Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
  s.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
  s.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
  s.Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
  s.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  s.Colors[ImGuiCol_ButtonActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
  s.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
  s.Colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
  s.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
  s.WindowRounding = 0.0f;
  s.ChildRounding = 2.0f;
  s.FrameRounding = 2.0f;
  s.WindowPadding = ImVec2(0, 0);
  s.FramePadding = ImVec2(6, 3);
  s.ItemSpacing = ImVec2(8, 4);
}

void SectionHeader(const char *label) {
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 1.0f));
  ImGui::Text("%s", label);
  ImGui::PopStyleColor();
  ImGui::Separator();
  ImGui::Spacing();
}

// Helper for rendering ESP Group Settings
// showDistance: false for Survivors and Bosses
void DrawESPGroup(const char *label, C_ESP_Settings &settings, int groupID,
                  bool showDistance = true) {
  ImGui::PushID(groupID);
  ImGui::Checkbox(label, &settings.Enabled);
  ImGui::SameLine();
  ColorPicker("##Color", settings.MainColor);

  if (settings.Enabled) {
    ImGui::Indent(10.0f);
    ImGui::Checkbox("Box", &settings.Box);
    ImGui::SameLine();
    ImGui::Checkbox("Name", &settings.Name);
    ImGui::Checkbox("Health Bar", &settings.Healthbar);
    ImGui::SameLine();
    ImGui::Checkbox("Numeric HP", &settings.NumericHP);
    ImGui::Checkbox("Skeleton", &settings.Skeleton);
    if (showDistance) {
      ImGui::SameLine();
      ImGui::Checkbox("Distance", &settings.Distance);
    }
    ImGui::Unindent(10.0f);
  }
  ImGui::PopID();
  ImGui::Spacing();
}

//=============================================================================
// TAB: AIMBOT
//=============================================================================
void Tab_Aimbot() {
  ImGui::Columns(3, nullptr, false);
  // General
  ImGui::BeginChild("AimGeneral", ImVec2(0, 480), true);
  SectionHeader("General");
  ImGui::Checkbox("Enable Aimbot", &Vars::Hitscan::bEnable);
  ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40);
  InputKeybind("##AimKey", Vars::Hitscan::AimKey);
  ImGui::Checkbox("NoSpread", &Vars::Removals::NoSpread);
  ImGui::Checkbox("Silent Aim", &Vars::Hitscan::bSilentAim);
  ImGui::Checkbox("Wait for Hitbox", &Vars::Hitscan::bWaitForHitbox);
  ImGui::Spacing();
  ImGui::Checkbox("Enable Smoothing", &Vars::Hitscan::bSmoothing);
  ImGui::PushItemWidth(-1);
  ImGui::SliderFloat("Smooth Factor", &Vars::Hitscan::fSmoothFactor, 1.0f,
                     20.0f, "%.1f");
  ImGui::Combo("Smooth Method", &Vars::Hitscan::SmoothMethod,
               "Static\0Dynamic\0");
  ImGui::SliderFloat("Randomize Factor", &Vars::Hitscan::RandomizeFactor, 0.0f,
                     10.0f, "%.1f");
  ImGui::SliderFloat("Curve Factor", &Vars::Hitscan::CurveFactor, 0.1f, 2.0f,
                     "%.1f");
  ImGui::SliderFloat("Switch Delay", &Vars::Hitscan::TargetSwitchDelay, 0.0f,
                     1.0f, "%.2f s");
  ImGui::SliderInt("Aimbot FOV", &Vars::Hitscan::iFov, 0, 180);
  ImGui::Checkbox("Show FOV Circle", &Vars::Hitscan::AimFovCircle);
  if (Vars::Hitscan::AimFovCircle) {
    ImGui::SameLine();
    float fovCol[4] = {
        Vars::Hitscan::AimFov.r() / 255.f, Vars::Hitscan::AimFov.g() / 255.f,
        Vars::Hitscan::AimFov.b() / 255.f, Vars::Hitscan::AimFov.a() / 255.f};
    if (ImGui::ColorEdit4("##FovColor", fovCol,
                          ImGuiColorEditFlags_NoInputs |
                              ImGuiColorEditFlags_AlphaBar)) {
      Vars::Hitscan::AimFov = Color(fovCol[0] * 255, fovCol[1] * 255,
                                    fovCol[2] * 255, fovCol[3] * 255);
    }
  }
  static const char *hitboxes[] = {"Head", "Chest", "Stomach"};
  ImGui::Combo("Hitbox", &Vars::Hitscan::iHitbox, hitboxes,
               IM_ARRAYSIZE(hitboxes));
  ImGui::PopItemWidth();
  ImGui::Spacing();
  ImGui::Checkbox("Multipoint Aim",
                  &Vars::Hitscan::MultiPointIfNoHitboxesAreVisible);
  ImGui::Checkbox("Backtrack", &Vars::Backtrack::Enabled);
  ImGui::PushItemWidth(-1);
  ImGui::SliderInt("Backtrack MS", &Vars::Backtrack::BacktrackMS, 0, 1000,
                   "%d ms");
  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::NextColumn();

  // RCS
  ImGui::BeginChild("AimRCS", ImVec2(0, 480), true);
  SectionHeader("RCS (Recoil Control)");
  ImGui::Checkbox("Enable RCS", &Vars::Removals::NoRecoil);
  static int rcsMode = 0; // Placeholder
  ImGui::PushItemWidth(-1);
  ImGui::Combo("RCS Mode", &rcsMode, "Aiming\0Always\0");
  ImGui::SliderInt("RCS FOV", &Vars::Hitscan::iRcsFov, 0, 180);
  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::NextColumn();

  // Advanced Filters
  ImGui::BeginChild("AimFilters", ImVec2(0, 480), true);
  SectionHeader("Advanced Filters");
  ImGui::Checkbox("Auto Fire", &Vars::Hitscan::bAutoShoot);
  ImGui::Checkbox("Auto Pistol", &Vars::Hitscan::bAutoPistol);
  ImGui::Checkbox("Ignore Behind Walls", &Vars::Hitscan::IgnoreWalls);
  ImGui::Spacing();
  ImGui::Text("Ignore Targeting:");
  ImGui::Checkbox("Survivors", &Vars::Ignore::IgnoreTeammates);
  ImGui::Checkbox("Common Infected", &Vars::Ignore::IgnoreCommon);
  if (ImGui::TreeNode("Special Infected")) {
    ImGui::Checkbox("Smoker", &Vars::Hitscan::IgnoreSmoker);
    ImGui::Checkbox("Boomer", &Vars::Hitscan::IgnoreBoomer);
    ImGui::Checkbox("Hunter", &Vars::Hitscan::IgnoreHunter);
    ImGui::Checkbox("Spitter", &Vars::Hitscan::IgnoreSpitter);
    ImGui::Checkbox("Jockey", &Vars::Hitscan::IgnoreJockey);
    ImGui::Checkbox("Charger", &Vars::Hitscan::IgnoreCharger);
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("Bosses")) {
    ImGui::Checkbox("Tank", &Vars::Hitscan::IgnoreTank);
    ImGui::Checkbox("Witch", &Vars::Hitscan::IgnoreWitch);
    ImGui::TreePop();
  }
  ImGui::EndChild();
  ImGui::Columns(1);
}

//=============================================================================
// TAB: ESP (REFACTORED)
//=============================================================================
void Tab_ESP() {
  ImGui::Columns(2, nullptr, false);

  // Column 1: Survivors & Common
  ImGui::BeginChild("ESPSurv", ImVec2(0, 480), true);
  SectionHeader("Survivors & Common");
  DrawESPGroup("Active Survivors", Vars::ESP::Survivor, 100,
               false); // No distance
  DrawESPGroup("Common Infected", Vars::ESP::Common,
               101); // With distance

  ImGui::Spacing();
  ImGui::Text("Global Settings");
  ImGui::Checkbox("Ignore Teammates (ESP/Chams)",
                  &Vars::Ignore::IgnoreTeammates);
  ImGui::PushItemWidth(-1);
  ImGui::SliderFloat("Max Distance", &Vars::ESP::ItemsMaxDistance, 0.0f,
                     5000.0f, "%.0f");
  ImGui::PopItemWidth();
  ImGui::EndChild();

  ImGui::NextColumn();

  // Column 2: Special Infected
  ImGui::BeginChild("ESPInf", ImVec2(0, 480), true);
  SectionHeader("Special Infected");

  if (ImGui::CollapsingHeader("Special Infected (Standard)",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    DrawESPGroup("Enable Standard SI", Vars::ESP::Infected, 200);
    DrawESPGroup("Ghost Mode (Spawning)", Vars::ESP::Ghost, 203,
                 false); // No distance for ghosts
  }

  if (ImGui::CollapsingHeader("Bosses", ImGuiTreeNodeFlags_DefaultOpen)) {
    DrawESPGroup("Tank", Vars::ESP::Tank, 201, false); // No distance
    ImGui::Indent(10.0f);
    ImGui::Checkbox("Tank Frustration Bar", &Vars::ESP::TankFrustration);
    ImGui::Unindent(10.0f);
    ImGui::Spacing();

    DrawESPGroup("Witch", Vars::ESP::Witch, 202, false); // No distance
  }

  ImGui::EndChild();
  ImGui::Columns(1);
}

//=============================================================================
// TAB: ITEMS
//=============================================================================
void Tab_Items() {
  ImGui::Columns(2, nullptr, false);
  ImGui::BeginChild("ItemsGen", ImVec2(0, 480), true);
  SectionHeader("World Items ESP");
  ImGui::Checkbox("Enable Items ESP", &Vars::ESP::ItemsEnabled);
  ImGui::SliderFloat("Max Distance", &Vars::ESP::ItemsMaxDistance, 0.0f,
                     3000.0f, "%.0f");
  ImGui::EndChild();
  ImGui::NextColumn();
  ImGui::BeginChild("ItemsFilt", ImVec2(0, 480), true);
  SectionHeader("Item Filters");
  ImGui::Checkbox("Weapons", &Vars::ESP::ItemsWeapons);
  ImGui::Checkbox("Medkits", &Vars::ESP::ItemsMedkits);
  ImGui::Checkbox("Pills", &Vars::ESP::ItemsPills);
  ImGui::Checkbox("Adrenaline", &Vars::ESP::ItemsAdrenaline);
  ImGui::Checkbox("Defib", &Vars::ESP::ItemsDefib);
  ImGui::Checkbox("Props (Gas, Propane)", &Vars::ESP::ItemsProps);
  ImGui::EndChild();
  ImGui::Columns(1);
}

//=============================================================================
// TAB: VISUALS (FIXED CRASH)
//=============================================================================
void Tab_Visuals() {
  ImGui::Columns(2, nullptr, false);

  // Column 1: Camera & Effects
  ImGui::BeginChild("VisCam", ImVec2(0, 480), true);
  SectionHeader("Camera & Effects");

  ImGui::Text("Removals");
  ImGui::Checkbox("Remove Boomer Vomit", &Vars::Removals::BoomerVisual);
  ImGui::Checkbox("Remove Visual Recoil", &Vars::Removals::NoVisualRecoil);
  ImGui::Checkbox("Remove Spread (Visual)", &Vars::Removals::VisualSpread);
  ImGui::Checkbox("Disable Post-Processing",
                  &Vars::Removals::DisablePostProcessing);

  ImGui::Spacing();
  ImGui::Text("Camera");
  ImGui::Checkbox("ThirdPerson Mode", &Vars::Misc::ThirdPerson);
  ImGui::SameLine(ImGui::GetContentRegionAvail().x - 40);
  InputKeybind("##TPKey", Vars::Misc::ThirdPersonKey);

  ImGui::PushItemWidth(-1);
  ImGui::SliderInt("Distance", &Vars::Misc::ThirdPersonDistance, 0, 500);
  ImGui::SliderInt("Height Offset", &Vars::Misc::CameraHeight, -100, 100);
  ImGui::SliderInt("Horizontal Offset", &Vars::Misc::CameraHorizontal, -100,
                   100);
  ImGui::SliderInt("Vertical Offset", &Vars::Misc::CameraVertical, -100, 100);
  ImGui::PopItemWidth();

  ImGui::EndChild(); // Closes "VisCam"

  ImGui::NextColumn();

  // Fixed: Added BeginChild so EndChild is valid, or removed EndChild.
  // Best practice is to have a child here for layout consistency.
  ImGui::BeginChild("VisRight", ImVec2(0, 480), true);
  SectionHeader("Chams");
  ImGui::Text("Player / Survivors");
  ImGui::Checkbox("Survivors", &Vars::Chams::Survivors);
  if (Vars::Chams::Survivors) {
    ImGui::SameLine();
    ImGui::Text("Color:");
    ImGui::SameLine();
    float playerCol[4] = {Vars::Chams::PlayerColor.r() / 255.f,
                          Vars::Chams::PlayerColor.g() / 255.f,
                          Vars::Chams::PlayerColor.b() / 255.f,
                          Vars::Chams::PlayerColor.a() / 255.f};
    if (ImGui::ColorEdit4("##PlayerChams", playerCol,
                          ImGuiColorEditFlags_NoInputs |
                              ImGuiColorEditFlags_AlphaBar)) {
      Vars::Chams::PlayerColor = Color(playerCol[0] * 255, playerCol[1] * 255,
                                       playerCol[2] * 255, playerCol[3] * 255);
    }
  }

  ImGui::Text("Player Infected (PVP)");
  ImGui::Checkbox("Infected Players", &Vars::Chams::InfectedPlayers);
  // Uses distinct bool now.
  ImGui::SameLine();
  ImGui::Text("Color:");
  ImGui::SameLine();
  float plrInfCol[4] = {Vars::Chams::PlayerInfectedColor.r() / 255.f,
                        Vars::Chams::PlayerInfectedColor.g() / 255.f,
                        Vars::Chams::PlayerInfectedColor.b() / 255.f,
                        Vars::Chams::PlayerInfectedColor.a() / 255.f};
  if (ImGui::ColorEdit4("##PlrInfChams", plrInfCol,
                        ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_AlphaBar)) {
    Vars::Chams::PlayerInfectedColor =
        Color(plrInfCol[0] * 255, plrInfCol[1] * 255, plrInfCol[2] * 255,
              plrInfCol[3] * 255);
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  ImGui::Text("Infected & Bosses (Chams)");

  auto ChamsColorPicker = [](const char *label, bool *enabled, Color *color) {
    ImGui::Checkbox(label, enabled);
    ImGui::SameLine();
    ImGui::PushID(label);
    float col[4] = {color->r() / 255.f, color->g() / 255.f, color->b() / 255.f,
                    color->a() / 255.f};
    if (ImGui::ColorEdit4("##Color", col,
                          ImGuiColorEditFlags_NoInputs |
                              ImGuiColorEditFlags_AlphaBar)) {
      *color = Color(col[0] * 255, col[1] * 255, col[2] * 255, col[3] * 255);
    }
    ImGui::PopID();
  };

  ChamsColorPicker("Common Infected", &Vars::Chams::Infected,
                   &Vars::Chams::InfectedColor);
  ChamsColorPicker("Tank", &Vars::Chams::Tank, &Vars::Chams::TankColor);
  ChamsColorPicker("Witch", &Vars::Chams::Witch, &Vars::Chams::WitchColor);

  if (ImGui::TreeNode("Special Infected")) {
    ChamsColorPicker("Boomer", &Vars::Chams::Boomer, &Vars::Chams::BoomerColor);
    ChamsColorPicker("Hunter", &Vars::Chams::Hunter, &Vars::Chams::HunterColor);
    ChamsColorPicker("Smoker", &Vars::Chams::Smoker, &Vars::Chams::SmokerColor);
    ChamsColorPicker("Jockey", &Vars::Chams::Jockey, &Vars::Chams::JockeyColor);
    ChamsColorPicker("Charger", &Vars::Chams::Charger,
                     &Vars::Chams::ChargerColor);
    ChamsColorPicker("Spitter", &Vars::Chams::Spitter,
                     &Vars::Chams::SpitterColor);
    ImGui::TreePop();
  }

  ImGui::EndChild(); // Closes "VisRight"

  ImGui::Columns(1);
}

//=============================================================================
// TAB: MOVEMENT
//=============================================================================
void Tab_Movement() {
  ImGui::Columns(2, nullptr, false);

  // Column 1: Movement Refinement
  ImGui::BeginChild("MoveRefine", ImVec2(0, 480), true);
  SectionHeader("Movement Refinement");
  ImGui::Checkbox("BunnyHop (Legit)", &Vars::Misc::Bunnyhop);
  ImGui::PushItemWidth(-1);
  ImGui::SliderInt("Bhop Chance", &Vars::Misc::BhopChance, 0, 100);
  ImGui::PopItemWidth();
  ImGui::Checkbox("Fake Lag (Legit)", &Vars::Misc::FakeLag);
  ImGui::PushItemWidth(-1);
  ImGui::SliderInt("Lag Factor", &Vars::Misc::FakeLagFactor, 0, 14);
  ImGui::PopItemWidth();
  ImGui::Checkbox("Human Randomizer", &Vars::Misc::HumanRandomizer);
  ImGui::Checkbox("AutoStrafe (Smooth)", &Vars::Misc::AutoStrafe);
  ImGui::PushItemWidth(-1);
  ImGui::SliderFloat("Smooth Factor", &Vars::Misc::AutoStrafeSmooth, 1.0f,
                     10.0f, "%.2f");
  ImGui::PopItemWidth();
  ImGui::Checkbox("Edge Jump Assist", &Vars::Misc::EdgeJump);
  ImGui::SameLine();
  InputKeybind("##EdgeJumpKey", Vars::Misc::EdgeJumpKey);
  ImGui::Checkbox("Mini Jump (Tactical)", &Vars::Misc::MiniJump);
  ImGui::Checkbox("Jump Bug (Exploit)", &Vars::Misc::JumpBug);
  ImGui::Checkbox("EdgeBug (No Fall Damage)", &Vars::Misc::EdgeBug);

  ImGui::Spacing();
  ImGui::Checkbox("AutoShove Logic", &Vars::Misc::AutoShove);
  ImGui::PushItemWidth(-1);
  ImGui::SliderInt("Shove Delay", &Vars::Misc::AutoShoveDelay, 0, 1000,
                   "%d ms");
  ImGui::PopItemWidth();
  ImGui::EndChild();

  ImGui::NextColumn();
  ImGui::BeginChild("MoveFilters", ImVec2(0, 480), true);
  SectionHeader("AutoShove Filters");
  ImGui::Checkbox("Smoker", &Vars::Misc::AutoShoveSmoker);
  ImGui::Checkbox("Boomer", &Vars::Misc::AutoShoveBoomer);
  ImGui::Checkbox("Hunter", &Vars::Misc::AutoShoveHunter);
  ImGui::Checkbox("Common Infected", &Vars::Misc::AutoShoveCommon);
  ImGui::Checkbox("Witch", &Vars::Misc::AutoShoveWitch);
  ImGui::Checkbox("Jockey", &Vars::Misc::AutoShoveJockey);
  ImGui::Checkbox("Charger", &Vars::Misc::AutoShoveCharger);
  ImGui::Checkbox("Tank", &Vars::Misc::AutoShoveTank);
  ImGui::EndChild();
  ImGui::Columns(1);
}

//=============================================================================
// TAB: HUD
//=============================================================================
void Tab_HUD() {
  ImGui::Columns(2, nullptr, false);
  ImGui::BeginChild("HUDGen", ImVec2(0, 480), true);
  SectionHeader("Spectator List");
  ImGui::Checkbox("Spectator List", &Vars::Visuals::SpectatorList);
  ImGui::EndChild();
  ImGui::NextColumn();
  ImGui::BeginChild("HUDCross", ImVec2(0, 480), true);
  SectionHeader("Crosshair");
  ImGui::Text("(Coming soon)");
  ImGui::EndChild();
  ImGui::Columns(1);
}

//=============================================================================
// TAB: MISC
//=============================================================================
void Tab_Misc() {
  ImGui::Columns(2, nullptr, false);
  ImGui::BeginChild("MiscGen", ImVec2(0, 480), true);
  SectionHeader("General");
  ImGui::Checkbox("Disable Interpolation", &Vars::Misc::DisableInterp);
  ImGui::Checkbox("Sequence Freezing", &Vars::Misc::SequenceFreezing);
  ImGui::SameLine(ImGui::GetContentRegionAvail().x - 50);
  InputKeybind("##SFKey", Vars::Misc::Key);
  ImGui::EndChild();
  ImGui::NextColumn();
  ImGui::BeginChild("MiscOther", ImVec2(0, 480), true);
  SectionHeader("Other");
  ImGui::Checkbox("Name Stealer", &Vars::Misc::Namestealer);
  ImGui::EndChild();
  ImGui::Columns(1);
}

//=============================================================================
// TAB: CONFIG
//=============================================================================
void Tab_Config() {
  ImGui::BeginChild("ConfigMain", ImVec2(-1, 480), true);
  SectionHeader("Profiles & Presets");

  static char configName[64] = "";
  ImGui::PushItemWidth(-1);
  ImGui::InputTextWithHint("##NewConfig", "New Profile Name", configName, 64);
  ImGui::PopItemWidth();

  ImGui::Spacing();
  if (ImGui::Button("Save Current Settings", ImVec2(-1, 30))) {
    if (strlen(configName) > 0) {
      Config::Save(configName);
      memset(configName, 0, 64);
    }
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  SectionHeader("Available Profiles");
  static std::vector<std::string> configs;
  static float lastRefreshed = 0.0f;
  if (ImGui::GetTime() - lastRefreshed > 2.0f) {
    configs = Config::GetConfigs();
    lastRefreshed = ImGui::GetTime();
  }

  static int selectedConfig = -1;
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
  if (ImGui::BeginListBox("##ConfigList", ImVec2(-1, 250))) {
    for (int i = 0; i < (int)configs.size(); i++) {
      const bool is_selected = (selectedConfig == i);
      if (ImGui::Selectable(configs[i].c_str(), is_selected)) {
        selectedConfig = i;
        strcpy_s(configName, configs[i].c_str());
      }
      if (is_selected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndListBox();
  }
  ImGui::PopStyleColor();

  ImGui::Spacing();
  if (selectedConfig >= 0 && selectedConfig < (int)configs.size()) {
    if (ImGui::Button("Load Profile", ImVec2(100, 30))) {
      Config::Load(configs[selectedConfig]);
    }
    ImGui::SameLine();
    if (ImGui::Button("Delete Profile", ImVec2(100, 30))) {
      Config::Delete(configs[selectedConfig]);
      selectedConfig = -1;
      configs = Config::GetConfigs();
    }
  }

  ImGui::SetCursorPosY(440);
  ImGui::Separator();
  if (ImGui::Button("Unload Cheat", ImVec2(-1, 30))) {
  }
  ImGui::EndChild();
}

void Handle() {
  if (!g_Menu.menuOpen && ImGui::GetStyle().Alpha > 0.f) {
    float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
    float alpha = ImGui::GetStyle().Alpha - fc / 255.f;
    ImGui::GetStyle().Alpha = (alpha < 0.f)     ? 0.f
                              : (alpha > 0.95f) ? 0.95f
                                                : alpha;
  }
  if (g_Menu.menuOpen && ImGui::GetStyle().Alpha < 1.f) {
    float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
    float alpha = ImGui::GetStyle().Alpha + fc / 255.f;
    ImGui::GetStyle().Alpha = (alpha < 0.f)     ? 0.f
                              : (alpha > 0.95f) ? 0.95f
                                                : alpha;
  }
}

void CMenu::Render(IDirect3DDevice9 *pDevice) {
  static bool bInitImGui = false;
  pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
  pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
  pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
  pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

  if (!bInitImGui) {
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(FindWindowW(_(L"Valve001"), 0));
    ImGui_ImplDX9_Init(pDevice);

    auto &io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange |
                     ImGuiConfigFlags_NavEnableKeyboard;

    ImFontConfig font_config;
    font_config.OversampleH = 1;
    font_config.OversampleV = 1;
    font_config.PixelSnapH = 1;
    font_config.FontDataOwnedByAtlas = false;
    static const ImWchar ranges[] = {0x0020, 0x00FF, 0x0400, 0x044F, 0};
    name = io.Fonts->AddFontFromMemoryTTF((void *)MuseoFont, sizeof(MuseoFont),
                                          18.0f, &font_config, ranges);
    font = io.Fonts->AddFontFromMemoryTTF((void *)MuseoFont, sizeof(MuseoFont),
                                          14.0f, &font_config, ranges);

    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF((void *)fa_solid_900, sizeof(fa_solid_900),
                                   14.0f, &icons_config, icons_ranges);

    bInitImGui = true;
  }

  if (GetAsyncKeyState(VK_INSERT) & 1) {
    I::VGuiSurface->SetCursorAlwaysVisible(menuOpen = !menuOpen);
    g_Menu.flTimeOnChange = I::EngineClient->GetTimescale();
  }

  g_Menu.m_flFadeElapsed =
      I::EngineClient->GetTimescale() - g_Menu.flTimeOnChange;

  ImGui_ImplDX9_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  ImGui::GetIO().MouseDrawCursor = menuOpen;
  Handle();

  Features::Visuals::SpectatorList::Render();

  if (menuOpen) {
    ZeniiwareStyle();
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
    ImGui::SetNextWindowSize(MenuSize);
    ImGui::Begin("##ZeniiwareMenu", NULL,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    {
      auto draw = ImGui::GetWindowDrawList();
      auto p = ImGui::GetWindowPos();
      auto s = ImGui::GetWindowSize();

      draw->AddRectFilled(p, ImVec2(p.x + s.x, p.y + s.y),
                          ImColor(15, 15, 15, 250), 0);
      draw->AddRectFilled(p, ImVec2(p.x + s.x, p.y + 35),
                          ImColor(18, 18, 18, 255), 0);
      draw->AddLine(ImVec2(p.x, p.y + 35), ImVec2(p.x + s.x, p.y + 35),
                    ImColor(40, 40, 40, 255));

      ImGui::PushFont(name);
      draw->AddText(ImVec2(p.x + 12, p.y + 8), ImColor(255, 255, 255, 255),
                    "zenii");
      draw->AddText(ImVec2(p.x + 12 + ImGui::CalcTextSize("zenii").x, p.y + 8),
                    ImColor(217, 25, 50, 255), "ware");
      ImGui::PopFont();

      ImGui::PushFont(font);
      ImGui::SetCursorPos(ImVec2(130, 5));
      const char *tabNames[] = {
          ICON_FA_CROSSHAIRS " Aimbot", ICON_FA_EYE " ESP",
          ICON_FA_BOX " Items",         ICON_FA_PALETTE " Visuals",
          ICON_FA_RUNNING " Movement",  ICON_FA_TV " HUD",
          ICON_FA_COGS " Misc",         ICON_FA_SAVE " Config"};

      for (int i = 0; i < 8; i++) {
        bool selected = (tab == i);
        ImGui::PushStyleColor(ImGuiCol_Button,
                              selected ? ImVec4(0.20f, 0.20f, 0.20f, 1.0f)
                                       : ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              ImVec4(0.20f, 0.20f, 0.20f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        if (ImGui::Button(tabNames[i], ImVec2(0, 25))) {
          tab = i;
        }
        ImGui::PopStyleColor(3);
        if (i < 7)
          ImGui::SameLine();
      }
      ImGui::PopFont();

      ImGui::SetCursorPos(ImVec2(5, 42));
      ImGui::BeginChild("Content", ImVec2(s.x - 10, s.y - 47), false);
      ImGui::PushFont(font);

      switch (tab) {
      case 0:
        Tab_Aimbot();
        break;
      case 1:
        Tab_ESP();
        break;
      case 2:
        Tab_Items();
        break;
      case 3:
        Tab_Visuals();
        break;
      case 4:
        Tab_Movement();
        break;
      case 5:
        Tab_HUD();
        break;
      case 6:
        Tab_Misc();
        break;
      case 7:
        Tab_Config();
        break;
      }

      ImGui::PopFont();
      ImGui::EndChild();
    }
    ImGui::End();
  }

  ImGui::EndFrame();
  ImGui::Render();
  ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
  pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
}