#include "cbase.h"
#include "weapon_smac.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSMAC, DT_WeaponSMAC)

BEGIN_NETWORK_TABLE(CWeaponSMAC, DT_WeaponSMAC)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponSMAC)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponSMAC)

LINK_ENTITY_TO_CLASS(weapon_smac, CWeaponSMAC);

PRECACHE_WEAPON_REGISTER(weapon_smac);

CWeaponSMAC::CWeaponSMAC()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0;

	m_nNumShotsFired = 0;
}

void CWeaponSMAC::AddViewKick()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	QAngle viewPunch;

	viewPunch.x = SharedRandomFloat("smacpx", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("smacpy", -0.6f, 0.6f);
	viewPunch.z = 0;

	owner->ViewPunch(viewPunch);
}
