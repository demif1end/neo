#include "cbase.h"
#include "weapon_tachi.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponTachi, DT_WeaponTachi)

BEGIN_NETWORK_TABLE(CWeaponTachi, DT_WeaponTachi)
DEFINE_NEO_BASE_WEP_NETWORK_TABLE

#ifdef CLIENT_DLL
RecvPropTime(RECVINFO(m_flSoonestFiremodeSwitch)),
RecvPropBool(RECVINFO(m_bIsPrimaryFireMode)),
#else
SendPropTime(SENDINFO(m_flSoonestFiremodeSwitch)),
SendPropBool(SENDINFO(m_bIsPrimaryFireMode)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponTachi)
DEFINE_NEO_BASE_WEP_PREDICTION

DEFINE_PRED_FIELD(m_flSoonestFiremodeSwitch, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bIsPrimaryFireMode, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponTachi)

LINK_ENTITY_TO_CLASS(weapon_tachi, CWeaponTachi);

#ifdef GAME_DLL
BEGIN_DATADESC(CWeaponTachi)
DEFINE_FIELD(m_flSoonestFiremodeSwitch, FIELD_TIME),
DEFINE_FIELD(m_bIsPrimaryFireMode, FIELD_BOOLEAN),
END_DATADESC()
#endif

PRECACHE_WEAPON_REGISTER(weapon_tachi);

CWeaponTachi::CWeaponTachi()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flSoonestFiremodeSwitch = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1 = 24;
	m_fMaxRange1 = 1500;
	m_fMinRange2 = 24;
	m_fMaxRange2 = 200;

	m_bFiresUnderwater = true;
	m_bIsPrimaryFireMode = true;
}

void CWeaponTachi::SwitchFireMode( void )
{
	if (m_flSoonestFiremodeSwitch > gpGlobals->curtime)
	{
		return;
	}

	m_bIsPrimaryFireMode = !m_bIsPrimaryFireMode;

#ifdef CLIENT_DLL
	// NEO TODO (Rain): fire mode indicator
	Msg("Fire mode: %s\n", m_bIsPrimaryFireMode ? "primary" : "alt");
#endif

	WeaponSound( SPECIAL1 );
	SendWeaponAnim( ACT_VM_DRAW_SPECIAL );
}

void CWeaponTachi::ForceSetFireMode( bool bPrimaryMode, bool bPlaySound,
	float flSoonestSwitch )
{
	m_bIsPrimaryFireMode = bPrimaryMode;
	m_flSoonestFiremodeSwitch = flSoonestSwitch;

	if (bPlaySound)
	{
		WeaponSound( SPECIAL1 );
	}
}

void CWeaponTachi::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
	{
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
	{
		return;
	}

	if ( (pOwner->m_nButtons & IN_ATTACK2) && (!(pOwner->m_afButtonLast & IN_ATTACK2)) )
	{
		if (m_flSoonestFiremodeSwitch < gpGlobals->curtime)
		{
			SwitchFireMode();
#define TACHI_FASTEST_FIREMODE_SWITCH_TIME 0.2f
			m_flSoonestFiremodeSwitch = gpGlobals->curtime + TACHI_FASTEST_FIREMODE_SWITCH_TIME;
			m_flSoonestAttack = gpGlobals->curtime + TACHI_FASTEST_FIREMODE_SWITCH_TIME;
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponTachi::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

	viewPunch.x = SharedRandomFloat( "tachipax", 0.25f, 0.5f );
	viewPunch.y = SharedRandomFloat( "tachipay", -0.6f, 0.6f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}
