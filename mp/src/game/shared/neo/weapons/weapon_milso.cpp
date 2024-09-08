#include "cbase.h"
#include "weapon_milso.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponMilso, DT_WeaponMilso)

BEGIN_NETWORK_TABLE(CWeaponMilso, DT_WeaponMilso)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponMilso)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponMilso)

LINK_ENTITY_TO_CLASS(weapon_milso, CWeaponMilso);

PRECACHE_WEAPON_REGISTER(weapon_milso);

CWeaponMilso::CWeaponMilso()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_bFiresUnderwater = true;
}

void CWeaponMilso::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	const QAngle viewPunch{
		SharedRandomFloat("milsopax", 0.25f, 0.5f),
		SharedRandomFloat("milsopay", -0.6f, 0.6f),
		0.0f
	};

	// Add it to the view punch
	pPlayer->ViewPunch(viewPunch);
}
