#include "cbase.h"
#include "weapon_ghost.h"
#include "neo_gamerules.h"

#ifdef CLIENT_DLL
#include <engine/ivdebugoverlay.h>
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponGhost, DT_WeaponGhost)

BEGIN_NETWORK_TABLE(CWeaponGhost, DT_WeaponGhost)
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bShouldShowEnemies)),
RecvPropArray(RecvPropVector(RECVINFO(m_rvPlayerPositions[0])), m_rvPlayerPositions),
#else
SendPropBool(SENDINFO(m_bShouldShowEnemies)),
SendPropArray(SendPropVector(SENDINFO_ARRAY(m_rvPlayerPositions), -1, SPROP_COORD_MP_LOWPRECISION | SPROP_CHANGES_OFTEN, MIN_COORD_FLOAT, MAX_COORD_FLOAT), m_rvPlayerPositions),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponGhost)
DEFINE_PRED_FIELD(m_rvPlayerPositions, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(weapon_ghost, CWeaponGhost);
PRECACHE_WEAPON_REGISTER(weapon_ghost);

#ifdef GAME_DLL
acttable_t CWeaponGhost::m_acttable[] =
{
	{ ACT_IDLE,				ACT_IDLE_PISTOL,				false },
	{ ACT_RUN,				ACT_RUN_PISTOL,					false },
	{ ACT_CROUCHIDLE,		ACT_HL2MP_IDLE_CROUCH_PISTOL,	false },
	{ ACT_WALK_CROUCH,		ACT_HL2MP_WALK_CROUCH_PISTOL,	false },
	{ ACT_RANGE_ATTACK1,	ACT_RANGE_ATTACK_PISTOL,		false },
	{ ACT_RELOAD,			ACT_RELOAD_PISTOL,				false },
	{ ACT_JUMP,				ACT_HL2MP_JUMP_PISTOL,			false },
};
IMPLEMENT_ACTTABLE(CWeaponGhost);
#endif

CWeaponGhost::CWeaponGhost(void)
{
#ifdef CLIENT_DLL
	rootGhostPanel = NULL;
#else
	// This is just always on for now.
	// Not sure if there's a reason to ever disable ghosting,
	// but might as well have the option.
	SetShowEnemies(true);
#endif
}

#ifdef CLIENT_DLL
CWeaponGhost::~C_WeaponGhost(void)
{
	if (rootGhostPanel)
	{
		rootGhostPanel->DeletePanel();
	}
}
#endif

void CWeaponGhost::Spawn(void)
{
	BaseClass::Spawn();

#ifdef CLIENT_DLL
	rootGhostPanel = new vgui::Panel();
	rootGhostPanel->SetAlpha(255);
	rootGhostPanel->SetVisible(true);
	rootGhostPanel->SetEnabled(false);
	rootGhostPanel->SetPostChildPaintEnabled(true);

	char formatBuff[21];
	const int playerPosArrSize = sizeof(m_rvPlayerPositions) / sizeof(m_rvPlayerPositions[0]);
	for (int i = 1; i <= playerPosArrSize; i++)
	{
		V_sprintf_safe(formatBuff, "GhostHUD_ClientIdx%i", i);
		vgui::ImagePanel *img = new vgui::ImagePanel(rootGhostPanel, formatBuff);

#ifdef _WIN32
		img->SetImage(vgui::scheme()->GetImage("vgui\\hud\\ctg\\g_beacon_enemy", false));
#elif defined(LINUX)
		img->SetImage(vgui::scheme()->GetImage("vgui/hud/ctg/g_beacon_enemy", false));
#else
#error Unimplemented
#endif

		img->SetAutoDelete(true);
		img->SetSize(256, 256);
		img->SetFgColor(Color(255, 0, 0));
		img->SetEnabled(false);
		img->SetVisible(false);
	}
#endif
}

inline void CWeaponGhost::ZeroGhostedPlayerLocArray(void)
{
#ifdef CLIENT_DLL
	for (int i = 0; i < m_rvPlayerPositions->Length(); i++)
	{
		m_rvPlayerPositions[i].Zero();
	}
#else

	for (int i = 0; i < m_rvPlayerPositions.Count(); i++)
	{
		m_rvPlayerPositions.Set(i, Vector(0, 0, 0));
	}
	NetworkStateChanged();
#endif
}

void CWeaponGhost::ItemPreFrame(void)
{
	if (m_bShouldShowEnemies)
	{
#ifdef CLIENT_DLL
		// Only show enemies if we are ghosting
		ShowEnemies();
		rootGhostPanel->Paint();
#else
		// We only need to update this while someone is ghosting
		UpdateNetworkedEnemyLocations();
#endif
	}
}

void CWeaponGhost::ItemHolsterFrame(void)
{
	BaseClass::ItemHolsterFrame();
}

void CWeaponGhost::PrimaryAttack(void)
{
}

#ifdef GAME_DLL
void CWeaponGhost::SetShowEnemies(bool enabled)
{
	m_bShouldShowEnemies.GetForModify() = enabled;
}
#endif

enum {
	NEO_GHOST_ONLY_ENEMIES = 0,
	NEO_GHOST_ONLY_PLAYABLE_TEAMS,
	NEO_GHOST_ANY_TEAMS
};
extern ConVar neo_ghost_debug_ignore_teams("neo_ghost_debug_ignore_teams", "2", FCVAR_CHEAT | FCVAR_REPLICATED,
	"Debug ghost team filter. If 0, only ghost the enemy team. If 1, ghost both playable teams. If 2, ghost any team, including spectator and unassigned.", true, 0.0, true, 2.0);

#ifdef CLIENT_DLL
// Purpose: Iterate through all enemies and give ghoster their position info,
// either via client's own PVS information or networked by the server when needed.
void CWeaponGhost::ShowEnemies(void)
{
	C_NEO_Player *player = (C_NEO_Player*)GetOwner();
	if (!player)
	{
		return;
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		HideBeacon(i);

		auto otherPlayer = ToBasePlayer( ClientEntityList().GetEnt( i ) );
		//auto otherPlayer = UTIL_PlayerByIndex(i);

		// Only ghost valid clients that aren't ourselves
		if (!otherPlayer || otherPlayer == player)
		{
			continue;
		}

		if (otherPlayer->GetTeamNumber() != TEAM_JINRAI && otherPlayer->GetTeamNumber() != TEAM_NSF)
		{
			// We don't want to ghost spectators or unassigned players
			if (neo_ghost_debug_ignore_teams.GetInt() < NEO_GHOST_ANY_TEAMS)
			{
				continue;
			}
		}

		if (player->GetTeamNumber() == otherPlayer->GetTeamNumber())
		{
			// We don't want to ghost our own team
			if (neo_ghost_debug_ignore_teams.GetInt() < NEO_GHOST_ONLY_PLAYABLE_TEAMS)
			{
				continue;
			}
		}

		// If it's in my PVS already
		if (otherPlayer->IsVisible())
		{
			DevMsg("Ghosting enemy from my PVS: %f %f %f\n",
				otherPlayer->GetAbsOrigin().x,
				otherPlayer->GetAbsOrigin().y,
				otherPlayer->GetAbsOrigin().z);
			
			Debug_ShowPos(otherPlayer->GetAbsOrigin());

			ShowBeacon(i, otherPlayer->GetAbsOrigin());
		}
		// Else, the server will provide us with this enemy's position info
		else
		{
			DevMsg("Ghosting enemy from server pos: %f %f %f\n",
				m_rvPlayerPositions[i].x,
				m_rvPlayerPositions[i].y,
				m_rvPlayerPositions[i].z);

			Debug_ShowPos(m_rvPlayerPositions[i]);

			ShowBeacon(i, m_rvPlayerPositions[i]);
		}
	}
}

inline void CWeaponGhost::HideBeacon(int panelIndex)
{
	rootGhostPanel->GetChild(panelIndex)->SetVisible(false);
}

inline void CWeaponGhost::ShowBeacon(int panelIndex, const Vector &pos)
{
	int x, y;
	GetVectorInScreenSpace(pos, x, y); // this is pixels from top-left

	//DevMsg("x %d y %d\n", x, y);

	rootGhostPanel->GetChild(panelIndex)->SetPos(x, y);
	rootGhostPanel->GetChild(panelIndex)->SetVisible(true);
	rootGhostPanel->GetChild(panelIndex)->Paint();

	//Assert(rootGhostPanel->GetChild(panelIndex)->IsFullyVisible());
}

void CWeaponGhost::Debug_ShowPos(const Vector &pos)
{
	int x, y;
	GetVectorInScreenSpace(pos, x, y);

	debugoverlay->AddTextOverlay(pos, 0.002f, "GHOST TARGET");
}
#endif

#ifdef GAME_DLL
// Purpose: Send enemy player locations to clients for ghost usage outside their PVS.
//
// NEO TODO/FIXME 1 (Rain): We should only send enemy position info to the ghoster
//
// NEO TODO/FIXME 2 (Rain): This stuff will get networked once per ghost;
// this can be inefficient in the unlikely event of multiple ghosts at play at once.
//
// NEO TODO/FIXME 3 (Rain): We don't check for distance (45m) yet - all enemy positions are revealed.
// This is also true for the PVS method.
void CWeaponGhost::UpdateNetworkedEnemyLocations(void)
{
	// FIXME/HACK: we never deallocate, move this to class member variable
	const int pvsMaxSize = (engine->GetClusterCount() / 8);
	Assert(pvsMaxSize > 0);
	static unsigned char *pvs = new unsigned char[pvsMaxSize];

	CNEO_Player *player = (CNEO_Player*)GetOwner();
	if (!player)
	{
		return;
	}

	const int cluster = engine->GetClusterForOrigin(player->GetAbsOrigin());
	const int pvsSize = engine->GetPVSForCluster(cluster, pvsMaxSize, pvs);
	Assert(pvsSize > 0);

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CNEO_Player *otherPlayer = (CNEO_Player*)UTIL_PlayerByIndex(i);

		// We're only interested in valid players that aren't the ghost owner.
		if (!otherPlayer || otherPlayer == player)
		{
			continue;
		}

		// If the other player is already in ghoster's PVS, we can skip them.
		else if (engine->CheckOriginInPVS(otherPlayer->GetAbsOrigin(), pvs, pvsSize))
		{
			continue;
		}

		vec_t absPos[3] = { otherPlayer->GetAbsOrigin().x, otherPlayer->GetAbsOrigin().y, otherPlayer->GetAbsOrigin().z };

		m_rvPlayerPositions.Set(i, otherPlayer->GetAbsOrigin());
		m_rvPlayerPositions.GetForModify(i).CopyToArray(absPos);
	}
}
#endif
