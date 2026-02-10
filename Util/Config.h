#pragma once
#include "../Features/Vars.h"
#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>

// Simple Config System
namespace Config {
// Helper to serialize C_ESP_Settings
inline void WriteESPSettings(std::ofstream &out,
                             const C_ESP_Settings &settings) {
  out.write((char *)&settings.Enabled, sizeof(bool));
  out.write((char *)&settings.Box, sizeof(bool));
  out.write((char *)&settings.Name, sizeof(bool));
  out.write((char *)&settings.Healthbar, sizeof(bool));
  out.write((char *)&settings.NumericHP, sizeof(bool));
  out.write((char *)&settings.Distance, sizeof(bool));
  out.write((char *)&settings.Skeleton, sizeof(bool));
  out.write((char *)&settings.VisCheck, sizeof(bool));
  out.write((char *)&settings.MainColor, sizeof(Color));
}

inline void ReadESPSettings(std::ifstream &in, C_ESP_Settings &settings) {
  in.read((char *)&settings.Enabled, sizeof(bool));
  in.read((char *)&settings.Box, sizeof(bool));
  in.read((char *)&settings.Name, sizeof(bool));
  in.read((char *)&settings.Healthbar, sizeof(bool));
  in.read((char *)&settings.NumericHP, sizeof(bool));
  in.read((char *)&settings.Distance, sizeof(bool));
  in.read((char *)&settings.Skeleton, sizeof(bool));
  in.read((char *)&settings.VisCheck, sizeof(bool));
  in.read((char *)&settings.MainColor, sizeof(Color));
}

inline std::vector<std::string> GetConfigs() {
  std::vector<std::string> configs;
  std::string path = "C:\\Zeniiware\\Configs\\";
  std::string search_path = path + "*.cfg";
  CreateDirectoryA(path.c_str(), NULL);

  WIN32_FIND_DATAA fd;
  HANDLE hFind = FindFirstFileA(search_path.c_str(), &fd);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        std::string filename = fd.cFileName;
        size_t lastindex = filename.find_last_of(".");
        if (lastindex != std::string::npos)
          filename = filename.substr(0, lastindex);
        configs.push_back(filename);
      }
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
  }
  return configs;
}

inline void Save(const std::string &name) {
  std::string path = "C:\\Zeniiware\\Configs\\" + name + ".cfg";
  std::ofstream out(path, std::ios::binary);
  if (!out.is_open())
    return;

  // Hitscan
  out.write((char *)&Vars::Hitscan::bEnable, sizeof(bool));
  out.write((char *)&Vars::Hitscan::bAutoShoot, sizeof(bool));
  out.write((char *)&Vars::Hitscan::bAutoPistol, sizeof(bool));
  out.write((char *)&Vars::Hitscan::bKey, sizeof(bool));
  out.write((char *)&Vars::Hitscan::bSilentAim, sizeof(bool));
  out.write((char *)&Vars::Hitscan::iFov, sizeof(int));
  out.write((char *)&Vars::Hitscan::iHitbox, sizeof(int));
  out.write((char *)&Vars::Hitscan::AimKey.m_Var, sizeof(int));
  out.write((char *)&Vars::Hitscan::MultiPointIfNoHitboxesAreVisible,
            sizeof(bool));
  out.write((char *)&Vars::Hitscan::AimFovCircle, sizeof(bool));
  out.write((char *)&Vars::Hitscan::bSmoothing, sizeof(bool));
  out.write((char *)&Vars::Hitscan::fSmoothFactor, sizeof(float));
  out.write((char *)&Vars::Hitscan::IgnoreWalls, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreSmoker, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreBoomer, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreHunter, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreSpitter, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreJockey, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreCharger, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreTank, sizeof(bool));
  out.write((char *)&Vars::Hitscan::IgnoreWitch, sizeof(bool));

  // Backtrack & Exploits
  out.write((char *)&Vars::Backtrack::Enabled, sizeof(bool));
  out.write((char *)&Vars::Backtrack::BacktrackMS, sizeof(int));
  out.write((char *)&Vars::Exploits::Speedhack, sizeof(bool));
  out.write((char *)&Vars::Exploits::Key.m_Var, sizeof(int));
  out.write((char *)&Vars::Exploits::SpeedHackValue, sizeof(int));

  // Misc
  out.write((char *)&Vars::Misc::Namestealer, sizeof(bool));
  out.write((char *)&Vars::Misc::Bunnyhop, sizeof(bool));
  out.write((char *)&Vars::Misc::BhopChance, sizeof(int));
  out.write((char *)&Vars::Misc::HumanRandomizer, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoStrafe, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoStrafeSmooth, sizeof(float));
  out.write((char *)&Vars::Misc::EdgeJump, sizeof(bool));
  out.write((char *)&Vars::Misc::MiniJump, sizeof(bool));
  out.write((char *)&Vars::Misc::JumpBug, sizeof(bool));
  out.write((char *)&Vars::Misc::EdgeBug, sizeof(bool));
  out.write((char *)&Vars::Misc::SequenceFreezing, sizeof(bool));
  out.write((char *)&Vars::Misc::DisableInterp, sizeof(bool));
  out.write((char *)&Vars::Misc::Key.m_Var, sizeof(int));
  out.write((char *)&Vars::Misc::AutoShove, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveDelay, sizeof(int));
  out.write((char *)&Vars::Misc::AutoShoveCommon, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveTank, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveHunter, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveSmoker, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveJockey, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveCharger, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveBoomer, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveSpitter, sizeof(bool));
  out.write((char *)&Vars::Misc::AutoShoveWitch, sizeof(bool));
  out.write((char *)&Vars::Misc::ThirdPerson, sizeof(bool));
  out.write((char *)&Vars::Misc::ThirdPersonDistance, sizeof(int));
  out.write((char *)&Vars::Misc::CameraHeight, sizeof(int));
  out.write((char *)&Vars::Misc::CameraHorizontal, sizeof(int));
  out.write((char *)&Vars::Misc::CameraVertical, sizeof(int));
  out.write((char *)&Vars::Misc::ThirdPersonKey.m_Var, sizeof(int));
  out.write((char *)&Vars::Misc::TeleportKey.m_Var, sizeof(int));

  // Removals
  out.write((char *)&Vars::Removals::NoSpread, sizeof(bool));
  out.write((char *)&Vars::Removals::NoRecoil, sizeof(bool));
  out.write((char *)&Vars::Removals::NoVisualRecoil, sizeof(bool));
  out.write((char *)&Vars::Removals::BoomerVisual, sizeof(bool));
  out.write((char *)&Vars::Removals::VisualSpread, sizeof(bool));
  out.write((char *)&Vars::Removals::DisablePostProcessing, sizeof(bool));

  // ESP - Updated to use Structs
  WriteESPSettings(out, Vars::ESP::Survivor);
  WriteESPSettings(out, Vars::ESP::Infected);
  WriteESPSettings(out, Vars::ESP::Common);
  WriteESPSettings(out, Vars::ESP::Tank);
  WriteESPSettings(out, Vars::ESP::Witch);

  out.write((char *)&Vars::ESP::TankFrustration, sizeof(bool)); // New

  // Item ESP
  out.write((char *)&Vars::ESP::ItemsEnabled, sizeof(bool));
  out.write((char *)&Vars::ESP::ItemsMaxDistance, sizeof(float));
  out.write((char *)&Vars::ESP::ItemsWeapons, sizeof(bool));
  out.write((char *)&Vars::ESP::ItemsMedkits, sizeof(bool));
  out.write((char *)&Vars::ESP::ItemsPills, sizeof(bool));
  out.write((char *)&Vars::ESP::ItemsAdrenaline, sizeof(bool));
  out.write((char *)&Vars::ESP::ItemsDefib, sizeof(bool));
  out.write((char *)&Vars::ESP::ItemsProps, sizeof(bool));

  // Ignore
  out.write((char *)&Vars::Ignore::IgnoreWitchUntilStartled, sizeof(bool));
  out.write((char *)&Vars::Ignore::IgnoreFriends, sizeof(bool));
  out.write((char *)&Vars::Ignore::IgnoreTeammates, sizeof(bool));
  out.write((char *)&Vars::Ignore::IgnoreCommon, sizeof(bool));

  // Chams
  out.write((char *)&Vars::Chams::Survivors, sizeof(bool));
  out.write((char *)&Vars::Chams::InfectedPlayers, sizeof(bool));
  out.write((char *)&Vars::Chams::Infected, sizeof(bool));
  out.write((char *)&Vars::Chams::Tank, sizeof(bool));
  out.write((char *)&Vars::Chams::Witch, sizeof(bool));
  out.write((char *)&Vars::Chams::Boomer, sizeof(bool));
  out.write((char *)&Vars::Chams::Hunter, sizeof(bool));
  out.write((char *)&Vars::Chams::Smoker, sizeof(bool));
  out.write((char *)&Vars::Chams::Jockey, sizeof(bool));
  out.write((char *)&Vars::Chams::Charger, sizeof(bool));
  out.write((char *)&Vars::Chams::Spitter, sizeof(bool));

  out.write((char *)&Vars::Chams::PlayerColor, sizeof(Color));
  out.write((char *)&Vars::Chams::PlayerInfectedColor, sizeof(Color));
  out.write((char *)&Vars::Chams::InfectedColor, sizeof(Color));
  out.write((char *)&Vars::Chams::TankColor, sizeof(Color));
  out.write((char *)&Vars::Chams::WitchColor, sizeof(Color));
  out.write((char *)&Vars::Chams::BoomerColor, sizeof(Color));
  out.write((char *)&Vars::Chams::HunterColor, sizeof(Color));
  out.write((char *)&Vars::Chams::SmokerColor, sizeof(Color));
  out.write((char *)&Vars::Chams::JockeyColor, sizeof(Color));
  out.write((char *)&Vars::Chams::ChargerColor, sizeof(Color));
  out.write((char *)&Vars::Chams::SpitterColor, sizeof(Color));

  out.close();
}

inline void Load(const std::string &name) {
  std::string path = "C:\\Zeniiware\\Configs\\" + name + ".cfg";
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open())
    return;

  // Hitscan
  in.read((char *)&Vars::Hitscan::bEnable, sizeof(bool));
  in.read((char *)&Vars::Hitscan::bAutoShoot, sizeof(bool));
  in.read((char *)&Vars::Hitscan::bAutoPistol, sizeof(bool));
  in.read((char *)&Vars::Hitscan::bKey, sizeof(bool));
  in.read((char *)&Vars::Hitscan::bSilentAim, sizeof(bool));
  in.read((char *)&Vars::Hitscan::iFov, sizeof(int));
  in.read((char *)&Vars::Hitscan::iHitbox, sizeof(int));
  in.read((char *)&Vars::Hitscan::AimKey.m_Var, sizeof(int));
  in.read((char *)&Vars::Hitscan::MultiPointIfNoHitboxesAreVisible,
          sizeof(bool));
  in.read((char *)&Vars::Hitscan::AimFovCircle, sizeof(bool));
  in.read((char *)&Vars::Hitscan::bSmoothing, sizeof(bool));
  in.read((char *)&Vars::Hitscan::fSmoothFactor, sizeof(float));
  in.read((char *)&Vars::Hitscan::IgnoreWalls, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreSmoker, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreBoomer, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreHunter, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreSpitter, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreJockey, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreCharger, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreTank, sizeof(bool));
  in.read((char *)&Vars::Hitscan::IgnoreWitch, sizeof(bool));

  // Backtrack
  in.read((char *)&Vars::Backtrack::Enabled, sizeof(bool));
  in.read((char *)&Vars::Backtrack::BacktrackMS, sizeof(int));

  // Exploits
  in.read((char *)&Vars::Exploits::Speedhack, sizeof(bool));
  in.read((char *)&Vars::Exploits::Key.m_Var, sizeof(int));
  in.read((char *)&Vars::Exploits::SpeedHackValue, sizeof(int));

  // Misc
  in.read((char *)&Vars::Misc::Namestealer, sizeof(bool));
  in.read((char *)&Vars::Misc::Bunnyhop, sizeof(bool));
  in.read((char *)&Vars::Misc::BhopChance, sizeof(int));
  in.read((char *)&Vars::Misc::HumanRandomizer, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoStrafe, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoStrafeSmooth, sizeof(float));
  in.read((char *)&Vars::Misc::EdgeJump, sizeof(bool));
  in.read((char *)&Vars::Misc::MiniJump, sizeof(bool));
  in.read((char *)&Vars::Misc::JumpBug, sizeof(bool));
  in.read((char *)&Vars::Misc::EdgeBug, sizeof(bool));
  in.read((char *)&Vars::Misc::SequenceFreezing, sizeof(bool));
  in.read((char *)&Vars::Misc::DisableInterp, sizeof(bool));
  in.read((char *)&Vars::Misc::Key.m_Var, sizeof(int));
  // Autoshove
  in.read((char *)&Vars::Misc::AutoShove, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveDelay, sizeof(int));
  in.read((char *)&Vars::Misc::AutoShoveCommon, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveTank, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveHunter, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveSmoker, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveJockey, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveCharger, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveBoomer, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveSpitter, sizeof(bool));
  in.read((char *)&Vars::Misc::AutoShoveWitch, sizeof(bool));

  // Camera
  in.read((char *)&Vars::Misc::ThirdPerson, sizeof(bool));
  in.read((char *)&Vars::Misc::ThirdPersonDistance, sizeof(int));
  in.read((char *)&Vars::Misc::CameraHeight, sizeof(int));
  in.read((char *)&Vars::Misc::CameraHorizontal, sizeof(int));
  in.read((char *)&Vars::Misc::CameraVertical, sizeof(int));
  in.read((char *)&Vars::Misc::ThirdPersonKey.m_Var, sizeof(int));
  in.read((char *)&Vars::Misc::TeleportKey.m_Var, sizeof(int));

  // Removals
  in.read((char *)&Vars::Removals::NoSpread, sizeof(bool));
  in.read((char *)&Vars::Removals::NoRecoil, sizeof(bool));
  in.read((char *)&Vars::Removals::NoVisualRecoil, sizeof(bool));
  in.read((char *)&Vars::Removals::BoomerVisual, sizeof(bool));
  in.read((char *)&Vars::Removals::VisualSpread, sizeof(bool));
  in.read((char *)&Vars::Removals::DisablePostProcessing,
          sizeof(bool)); // Added

  // ESP - Use functions
  ReadESPSettings(in, Vars::ESP::Survivor);
  ReadESPSettings(in, Vars::ESP::Infected);
  ReadESPSettings(in, Vars::ESP::Common);
  ReadESPSettings(in, Vars::ESP::Tank);
  ReadESPSettings(in, Vars::ESP::Witch);

  in.read((char *)&Vars::ESP::TankFrustration, sizeof(bool));

  // Item ESP
  in.read((char *)&Vars::ESP::ItemsEnabled, sizeof(bool));
  in.read((char *)&Vars::ESP::ItemsMaxDistance, sizeof(float));
  in.read((char *)&Vars::ESP::ItemsWeapons, sizeof(bool));
  in.read((char *)&Vars::ESP::ItemsMedkits, sizeof(bool));
  in.read((char *)&Vars::ESP::ItemsPills, sizeof(bool));
  in.read((char *)&Vars::ESP::ItemsAdrenaline, sizeof(bool));
  in.read((char *)&Vars::ESP::ItemsDefib, sizeof(bool));
  in.read((char *)&Vars::ESP::ItemsProps, sizeof(bool));

  // Ignore
  in.read((char *)&Vars::Ignore::IgnoreWitchUntilStartled, sizeof(bool));
  in.read((char *)&Vars::Ignore::IgnoreFriends, sizeof(bool));
  in.read((char *)&Vars::Ignore::IgnoreTeammates, sizeof(bool));
  in.read((char *)&Vars::Ignore::IgnoreCommon, sizeof(bool));

  // Chams
  in.read((char *)&Vars::Chams::Survivors, sizeof(bool));
  in.read((char *)&Vars::Chams::InfectedPlayers, sizeof(bool));
  in.read((char *)&Vars::Chams::Infected, sizeof(bool));
  in.read((char *)&Vars::Chams::Tank, sizeof(bool));
  in.read((char *)&Vars::Chams::Witch, sizeof(bool));
  in.read((char *)&Vars::Chams::Boomer, sizeof(bool));
  in.read((char *)&Vars::Chams::Hunter, sizeof(bool));
  in.read((char *)&Vars::Chams::Smoker, sizeof(bool));
  in.read((char *)&Vars::Chams::Jockey, sizeof(bool));
  in.read((char *)&Vars::Chams::Charger, sizeof(bool));
  in.read((char *)&Vars::Chams::Spitter, sizeof(bool));

  in.read((char *)&Vars::Chams::PlayerColor, sizeof(Color));
  in.read((char *)&Vars::Chams::PlayerInfectedColor, sizeof(Color));
  in.read((char *)&Vars::Chams::InfectedColor, sizeof(Color));
  in.read((char *)&Vars::Chams::TankColor, sizeof(Color));
  in.read((char *)&Vars::Chams::WitchColor, sizeof(Color));
  in.read((char *)&Vars::Chams::BoomerColor, sizeof(Color));
  in.read((char *)&Vars::Chams::HunterColor, sizeof(Color));
  in.read((char *)&Vars::Chams::SmokerColor, sizeof(Color));
  in.read((char *)&Vars::Chams::JockeyColor, sizeof(Color));
  in.read((char *)&Vars::Chams::ChargerColor, sizeof(Color));
  in.read((char *)&Vars::Chams::SpitterColor, sizeof(Color));

  in.close();
}

inline void Delete(const std::string &name) {
  std::string path = "C:\\Zeniiware\\Configs\\" + name + ".cfg";
  DeleteFileA(path.c_str());
}
} // namespace Config
