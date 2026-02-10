#include "entitycache.h"
#include <cstring>

void CEntityCache::Fill() {
  if (!I::EngineClient || !I::ClientEntityList || !I::EngineClient->IsInGame()) {
    Clear();
    return;
  }

  IClientEntity *pLocalEntity =
      I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
  if (!pLocalEntity)
    return;
  C_TerrorPlayer *_pLocal = pLocalEntity->As<C_TerrorPlayer *>();
  // C_BasePlayer* pLocal =
  // gInts.EntList->GetClientEntitySecond(gInts.Engine->GetLocalPlayer());
  if (_pLocal && _pLocal->m_iTeamNum()) {
    m_pLocal = _pLocal;

    IClientEntity *pEntity;
    for (int n = 1; n < (I::ClientEntityList->GetMaxEntities() + 1); n++) {
      if (n == I::EngineClient->GetLocalPlayer())
        continue;
      pEntity = I::ClientEntityList->GetClientEntity(n);

      if (!pEntity || pEntity->IsDormant())
        continue;

      const auto pCC = pEntity->GetClientClass();
      if (!pCC)
        continue;

      switch (pCC->m_ClassID) {
      case Infected: {
        if (!(pEntity->As<C_Infected *>()->m_usSolidFlags() &
              SolidFlags_t::FSOLID_NOT_SOLID) &&
            pEntity->As<C_Infected *>()->m_nSequence() <= 305)
          m_vecGroups[EGroupType::INFECTED].push_back(pEntity);
        break;
      }
      case EClientClass::Boomer:
      case EClientClass::Jockey:
      case EClientClass::Smoker:
      case EClientClass::Hunter:
      case EClientClass::Spitter:
      case EClientClass::Charger: {
        C_TerrorPlayer *pPlayer = pEntity->As<C_TerrorPlayer *>();
        if (!pPlayer)
          break;

        if (pPlayer->IsAlive() && !pPlayer->IsDormant() &&
            pPlayer->m_iTeamNum() == TEAM_INFECTED && !pPlayer->m_isGhost())
          m_vecGroups[EGroupType::SPECIAL_INFECTED].push_back(pEntity);
        break;
      }
      case Tank: {
        C_Tank *pTank = pEntity->As<C_Tank *>();
        if (!pTank)
          break;

        if (pTank->IsAlive() && !pTank->IsDormant() &&
            pTank->m_iTeamNum() == TEAM_INFECTED &&
            (pTank->m_iHealth() <= pTank->m_iMaxHealth()))
          m_vecGroups[EGroupType::SPECIAL_INFECTED].push_back(pEntity);
        break;
      }
      case Witch: {
        if (!(pEntity->As<C_Witch *>()->m_usSolidFlags() &
              SolidFlags_t::FSOLID_NOT_SOLID) &&
            pEntity->As<C_Witch *>()->m_nSequence() <= 70)
          m_vecGroups[EGroupType::WITCH].push_back(pEntity);
        break;
      }
      case CTerrorPlayer:
      case SurvivorBot: {
        if (pEntity->As<C_TerrorPlayer *>()->IsAlive()) {
          m_vecGroups[EGroupType::CTERRORPLAYER].push_back(pEntity);
        }
        break;
      }

      default:
        break;
      }
    }
    UpdateFriends();
  }
}

bool IsPlayerOnSteamFriendList(IClientEntity *pPlayer) {
  player_info_t pi = {};

  if (I::EngineClient->GetPlayerInfo(pPlayer->entindex(), &pi) &&
      pi.friendsid && I::steamfriends002) {
    CSteamID steamID{pi.friendsid, 1, k_EUniversePublic,
                     k_EAccountTypeIndividual};
    return I::steamfriends002->HasFriend(steamID, k_EFriendFlagImmediate);
  }

  return false;
}

void CEntityCache::UpdateFriends() {
  static size_t CurSize, OldSize;
  const auto Players = GetGroup(EGroupType::CTERRORPLAYER);
  CurSize = Players.size();

  if (CurSize != OldSize) {
    for (const auto &Player : Players)
      Friends[Player->entindex()] = IsPlayerOnSteamFriendList(Player);
  }

  OldSize = CurSize;
}

void CEntityCache::Clear() {
  m_pLocal = nullptr;
  m_pLocalWeapon = nullptr;
  m_pObservedTarget = nullptr;

  memset(Friends, 0, sizeof(Friends));

  for (auto &Group : m_vecGroups)
    Group.second.clear();
}

const std::vector<IClientEntity *> &
CEntityCache::GetGroup(const EGroupType &Group) {
  return m_vecGroups[Group];
}
