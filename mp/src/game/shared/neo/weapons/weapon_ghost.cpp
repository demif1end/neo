#include "cbase.h"
#include "weapon_ghost.h"
#include "neo_gamerules.h"

#ifdef CLIENT_DLL
#include "c_neo_player.h"
#else
#include "neo_player.h"
#endif

#ifdef CLIENT_DLL
#include <engine/ivdebugoverlay.h>
#include <engine/IEngineSound.h>
#include "filesystem.h"
#endif

#include "neo_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar cl_neo_ghost_view_distance("cl_neo_ghost_view_distance", "45", FCVAR_REPLICATED, "How far can the ghost user see players in meters.");

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponGhost, DT_WeaponGhost)

BEGIN_NETWORK_TABLE(CWeaponGhost, DT_WeaponGhost)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponGhost)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponGhost)

LINK_ENTITY_TO_CLASS(weapon_ghost, CWeaponGhost);

PRECACHE_WEAPON_REGISTER(weapon_ghost);

CWeaponGhost::CWeaponGhost(void)
{
#ifdef CLIENT_DLL
	m_bHavePlayedGhostEquipSound = false;
	m_bHaveHolsteredTheGhost = false;

	m_flLastGhostBeepTime = 0;
#endif
}

void CWeaponGhost::ItemPreFrame(void)
{
#ifdef CLIENT_DLL
	HandleGhostEquip();
#endif
}

#ifdef CLIENT_DLL
void CWeaponGhost::HandleGhostEquip(void)
{
	if (!m_bHavePlayedGhostEquipSound)
	{
		PlayGhostSound();
		m_bHavePlayedGhostEquipSound = true;
		m_bHaveHolsteredTheGhost = false;
	}
}

// Emit a ghost ping at a refire interval based on distance.
void C_WeaponGhost::TryGhostPing(float closestEnemy)
{
	if (closestEnemy < 0 || closestEnemy > 45)
	{
		return;
	}

	const float frequency = clamp((0.1f * closestEnemy), 1.0f, 3.5f);
	const float deltaTime = gpGlobals->curtime - m_flLastGhostBeepTime;

	if (deltaTime > frequency)
	{
		EmitSound("NeoPlayer.GhostPing");

		m_flLastGhostBeepTime = gpGlobals->curtime;
	}
}

void CWeaponGhost::HandleGhostUnequip(void)
{
	if (!m_bHaveHolsteredTheGhost)
	{
		StopGhostSound();
		m_bHaveHolsteredTheGhost = true;
		m_bHavePlayedGhostEquipSound = false;
	}
}

// Consider calling HandleGhostEquip instead.
void CWeaponGhost::PlayGhostSound(float volume)
{
	auto owner = static_cast<C_BasePlayer*>(GetOwner());
	if (!owner)
	{
		return;
	}

	C_RecipientFilter filter;
	filter.AddRecipient(owner);

	EmitSound_t emitSoundType;
	emitSoundType.m_flVolume = volume;
	emitSoundType.m_pSoundName = "HUD.GhostEquip";
	emitSoundType.m_nFlags |= SND_SHOULDPAUSE | SND_DO_NOT_OVERWRITE_EXISTING_ON_CHANNEL;

	EmitSound(filter, entindex(), emitSoundType);
}

// Consider calling HandleGhostUnequip instead.
void CWeaponGhost::StopGhostSound(void)
{
	StopSound(this->entindex(), "HUD.GhostEquip");
}
#endif

void CWeaponGhost::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();

#ifdef CLIENT_DLL
	HandleGhostUnequip(); // is there some callback, so we don't have to keep calling this?
#endif
}

enum {
	NEO_GHOST_ONLY_ENEMIES = 0,
	NEO_GHOST_ONLY_PLAYABLE_TEAMS,
	NEO_GHOST_ANY_TEAMS
};
ConVar neo_ghost_debug_ignore_teams("neo_ghost_debug_ignore_teams", "0", FCVAR_CHEAT | FCVAR_REPLICATED,
	"Debug ghost team filter. If 0, only ghost the enemy team. If 1, ghost both playable teams. If 2, ghost any team, including spectator and unassigned.", true, 0.0, true, 2.0);
ConVar neo_ghost_debug_spew("neo_ghost_debug_spew", "0", FCVAR_CHEAT | FCVAR_REPLICATED,
	"Whether to spew debug logs to console about ghosting positions and the data PVS/server origin.", true, 0.0, true, 1.0);
ConVar neo_ghost_debug_hudinfo("neo_ghost_debug_hudinfo", "0", FCVAR_CHEAT | FCVAR_REPLICATED,
	"Whether to overlay debug text information to screen about ghosting targets.", true, 0.0, true, 1.0);

void CWeaponGhost::OnPickedUp(CBaseCombatCharacter *pNewOwner)
{
	BaseClass::OnPickedUp(pNewOwner);

	if (pNewOwner)
	{
		auto neoOwner = static_cast<CNEO_Player*>(pNewOwner);
		Assert(neoOwner);

		// Prevent ghoster from sprinting
		if (neoOwner->IsSprinting())
		{
			neoOwner->StopSprinting();
		}

#ifdef GAME_DLL
		CTeamRecipientFilter filter(NEORules()->GetOpposingTeam(neoOwner), true);
		EmitSound_t params;
		params.m_nChannel = CHAN_USER_BASE;
		params.m_pSoundName = "HUD.GhostPickUp";
		EmitSound(filter, neoOwner->entindex(), params);
#endif
	}
}

float CWeaponGhost::DistanceToPos(const Vector& otherPlayerPos)
{
	auto owner = GetOwner();
	if(!owner)
		return false;
	
	const auto dir = owner->EyePosition() - otherPlayerPos;
	return dir.Length2D();
}

float CWeaponGhost::GetGhostRangeInHammerUnits() const
{
	const float maxGhostRangeMeters = cl_neo_ghost_view_distance.GetFloat();
	return (maxGhostRangeMeters / METERS_PER_INCH);
}

bool CWeaponGhost::IsPosWithinViewDistance(const Vector& otherPlayerPos)
{
	float dist;
	return IsPosWithinViewDistance(otherPlayerPos, dist);
}

bool CWeaponGhost::IsPosWithinViewDistance(const Vector& otherPlayerPos, float& dist)
{
	dist = DistanceToPos(otherPlayerPos);
	return dist <= GetGhostRangeInHammerUnits();
}