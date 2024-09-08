#include "cbase.h"
#include "weapon_jitte.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponJitte, DT_WeaponJitte)

BEGIN_NETWORK_TABLE(CWeaponJitte, DT_WeaponJitte)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponJitte)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponJitte)

LINK_ENTITY_TO_CLASS(weapon_jitte, CWeaponJitte);

PRECACHE_WEAPON_REGISTER(weapon_jitte);

CWeaponJitte::CWeaponJitte()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0;

	m_nNumShotsFired = 0;
}

void CWeaponJitte::AddViewKick()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	QAngle viewPunch;

	viewPunch.x = SharedRandomFloat("jittepx", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("jittepy", -0.6f, 0.6f);
	viewPunch.z = 0;

	owner->ViewPunch(viewPunch);
}
