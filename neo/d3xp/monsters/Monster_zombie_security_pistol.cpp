// Monster_zombie_security_pistol.cpp
//


#pragma hdrstop
#include "precompiled.h"
#include "../Game_local.h"

CLASS_DECLARATION( rvmMonsterZombie, rvmMonsterZombieSecurityPistol )
END_CLASS

#define ZSECP_RUNDISTANCE				192
#define ZSECP_WALKTURN					65
#define ZSECP_DODGE_RATE				3
#define ZSECP_ATTACK_DELAY_MIN			0.5
#define ZSECP_ATTACK_DELAY_MAX			2
#define ZSECP_ATTACK_MAX_LENGTH			3
#define ZSECP_ATTACK_MIN_LENGTH			1
#define ZSECP_WAIT_MIN_LENGTH			0.3
#define ZSECP_WAIT_MAX_LENGTH			1.5
#define ZSECP_CROUCH_ATTACK_MAX_LENGTH	15
#define ZSECP_CROUCH_ATTACK_MIN_LENGTH	3
#define ZSECP_STAND_ATTACK_MAX_LENGTH	15
#define ZSECP_STAND_ATTACK_MIN_LENGTH	3
#define ZSECP_ATTACK_MIN_SHOOT_TIME		0.5
#define ZSECP_NOFOVTIME					4

#define ATTACK_ZSECP_CROUCHFIRE			ATTACK_SPECIAL1

/*
=====================
rvmMonsterZombieSecurityPistol::Init
=====================
*/
void rvmMonsterZombieSecurityPistol::Init( void )
{
	combat_node = NULL;

	// start out with a 50/50 chance of stand vs. crouch attacks.
	zsecp_num_stand_attacks = 1;
	zsecp_num_crouch_attacks = 1;

	fire.LinkTo( scriptObject, "fire" );
	crouch_fire.LinkTo( scriptObject, "crouch_fire" );
	run_attack.LinkTo( scriptObject, "run_attack" );
	nextDodge.LinkTo( scriptObject, "nextDodge" );
	nextAttack.LinkTo( scriptObject, "nextAttack" );
	nextNoFOVAttack.LinkTo( scriptObject, "nextNoFOVAttack" );
}

/*
=====================
rvmMonsterZombieSecurityPistol::AI_Begin
=====================
*/
void rvmMonsterZombieSecurityPistol::AI_Begin( void )
{
	run_distance = ZSECP_RUNDISTANCE;
	walk_turn = ZSECP_WALKTURN;
	run_attack = GetIntKey( "run_attack" );

	SetState( "state_Begin" );
}

/*
=====================
rvmMonsterZombieSecurityPistol::state_Begin
=====================
*/
stateResult_t rvmMonsterZombieSecurityPistol::state_Begin( stateParms_t* parms )
{
	fire = false;
	crouch_fire = false;
	Event_AnimState( ANIMCHANNEL_TORSO, "Torso_Idle", 0 );
	Event_AnimState( ANIMCHANNEL_LEGS, "Legs_Idle", 0 );

	Event_SetMoveType( MOVETYPE_ANIM );
	SetState( "state_Idle" );
	return SRESULT_DONE;
}


/*
=====================
rvmMonsterZombieSecurityPistol::state_Idle
=====================
*/
stateResult_t rvmMonsterZombieSecurityPistol::state_Idle( stateParms_t* parms )
{
	if( parms->stage == 0 )
	{
		if( wait_for_enemy( parms ) == SRESULT_DONE )
		{
			parms->stage = 1;
		}

		return SRESULT_WAIT;
	}

	nextAttack = 0;
	nextNoFOVAttack = 0;
	nextDodge = gameLocal.RandomTime( ZSECP_DODGE_RATE );

	Event_SetState( "state_Combat" );
	return SRESULT_DONE;
}

/*
=====================
rvmMonsterZombieSecurityPistol::check_attacks
=====================
*/
int rvmMonsterZombieSecurityPistol::check_attacks()
{
	float currentTime;
	int attack_flags;
	boolean try_attack;
	boolean reachable;

	attack_flags = 0;

	currentTime = gameLocal.SysScriptTime();
	if( AI_PAIN && ( currentTime >= nextDodge ) )
	{
		if( TestAnimMove( "evade_left" ) )
		{
			attack_flags |= ATTACK_DODGE_LEFT;
		}
		if( TestAnimMove( "evade_right" ) )
		{
			attack_flags |= ATTACK_DODGE_RIGHT;

			// if we can dodge either direction, pick one
			if( attack_flags & ATTACK_DODGE_LEFT )
			{
				if( gameLocal.Random( 100 ) < 50 )
				{
					attack_flags &= ~ATTACK_DODGE_RIGHT;
				}
				else
				{
					attack_flags &= ~ATTACK_DODGE_LEFT;
				}
			}
		}
	}

	combat_node = GetCombatNode();
	if( combat_node )
	{
		attack_flags |= ATTACK_COMBAT_NODE;
	}

	reachable = CanReachEnemy();
	if( AI_ENEMY_IN_FOV && ( !reachable || ( currentTime >= nextAttack ) ) )
	{
		try_attack = true;
	}
	else if( ( gameLocal.SysScriptTime() > nextNoFOVAttack ) && AI_ENEMY_VISIBLE && ( !reachable || ( currentTime >= nextAttack ) ) )
	{
		try_attack = true;
	}
	else if( Touches( GetEnemy() ) )
	{
		try_attack = true;
	}
	else
	{
		try_attack = false;
	}

	if( try_attack )
	{
		// adjust the likelyhood of a crouch vs. stand attack based on the # of each.
		if( gameLocal.Random( zsecp_num_stand_attacks + zsecp_num_crouch_attacks ) < zsecp_num_stand_attacks )
		{
			if( CanHitEnemyFromAnim( "crouch_range_attack_loop" ) )
			{
				attack_flags |= ATTACK_ZSECP_CROUCHFIRE;
			}
			else if( CanHitEnemyFromAnim( "range_attack_loop" ) )
			{
				attack_flags |= ATTACK_MISSILE;
			}
		}
		else if( CanHitEnemyFromAnim( "range_attack_loop" ) )
		{
			attack_flags |= ATTACK_MISSILE;
		}
	}

	return attack_flags;
}

/*
=====================
rvmMonsterZombieSecurityPistol::do_attack
=====================
*/
void rvmMonsterZombieSecurityPistol::do_attack( int attack_flags )
{
	nextNoFOVAttack = gameLocal.SysScriptTime() + ZSECP_NOFOVTIME;
	if( attack_flags & ATTACK_DODGE_LEFT )
	{
		stateThread.SetState( "combat_dodge_left" );
	}
	else if( attack_flags & ATTACK_DODGE_RIGHT )
	{
		//combat_dodge_right();
		stateThread.SetState( "combat_dodge_right" );
	}
	else if( attack_flags & ATTACK_COMBAT_NODE )
	{
		//combat_ainode(combat_node);
		gameLocal.Error( "rvmMonsterZombieSecurityPistol::CombatAINode\n" );
	}
	else if( attack_flags & ATTACK_ZSECP_CROUCHFIRE )
	{
		//crouch_attack();
		stateThread.SetState( "crouch_attack" );
	}
	else if( attack_flags & ATTACK_MISSILE )
	{
		//stand_attack();
		stateThread.SetState( "stand_attack" );
	}
}

/*
=====================
rvmMonsterZombieSecurityPistol::stand_attack
=====================
*/
stateResult_t rvmMonsterZombieSecurityPistol::stand_attack( stateParms_t* parms )
{
	//float endtime;
	float left;
	float right;

	if( parms->stage == 0 )
	{
		zsecp_num_stand_attacks++;

		if( gameLocal.Random( 10 ) < 6 )
		{
			parms->param2 = run_attack;
		}
		else
		{
			parms->param2 = false;
		}

		fire = true;
		attackTime = gameLocal.RandomDelay( ZSECP_STAND_ATTACK_MIN_LENGTH, ZSECP_STAND_ATTACK_MAX_LENGTH );
		parms->stage = 1;
		return SRESULT_WAIT;
	}

	if( parms->stage == 1 )
	{
		if( AI_ENEMY_VISIBLE )
		{
			parms->stage = 2;
			parms->param1 = gameLocal.RandomDelay( 0.5, 1 ); // endtime
		}

		parms->stage = 3;
		return SRESULT_WAIT;
	}

	if( parms->stage == 2 )
	{
		float endtime = parms->param1;
		if( gameLocal.SysScriptTime() < endtime )
		{
			if( gameLocal.InfluenceActive() )
			{
				fire = false;
				parms->stage = 1; // break
			}

			Event_LookAtEnemy( 1.0f );
			if( !parms->param2 || ( EnemyRange() < 90 ) )
			{
				Event_FaceEnemy();
			}
			else if( !EntityInAttackCone( GetEnemy() ) )
			{
				attackTime = 0;
				parms->stage = 1; // break
			}
			else
			{
				Event_MoveToEnemy();
			}

			if( !CanHitEnemyFromAnim( "range_attack_loop" ) )
			{
				parms->stage = 1; // break
			}
			if( AI_PAIN )
			{
				int attack_flags = check_attacks();
				if( attack_flags & ( ATTACK_DODGE_LEFT | ATTACK_DODGE_RIGHT ) )
				{
					fire = false;
					do_attack( attack_flags );
					return SRESULT_DONE;
				}
			}
		}
		else
		{
			if( gameLocal.InfluenceActive() )
			{
				parms->stage = 3;
				return SRESULT_WAIT;
			}

			if( gameLocal.SysScriptTime() > attackTime )
			{
				parms->stage = 3;
				return SRESULT_WAIT;
			}

			if( !AI_ENEMY_VISIBLE || !CanHitEnemyFromAnim( "range_attack_loop" ) )
			{
				parms->stage = 3;
				return SRESULT_WAIT;
			}

			left = TestAnimMove( "step_left" );
			right = TestAnimMove( "step_right" );
			if( left && right )
			{
				if( gameLocal.Random( 100 ) < 50 )
				{
					left = false;
				}
				else
				{
					right = false;
				}
			}

			if( left )
			{
				Event_AnimState( ANIMCHANNEL_LEGS, "Legs_StepLeft", 4 );
				parms->stage = 4;
				return SRESULT_WAIT;
			}
			else if( right )
			{
				Event_AnimState( ANIMCHANNEL_LEGS, "Legs_StepRight", 4 );
				parms->stage = 5;
				return SRESULT_WAIT;
			}
		}

		return SRESULT_WAIT;
	}

	if( parms->stage == 4 )
	{
		if( InAnimState( ANIMCHANNEL_LEGS, "Legs_StepLeft" ) )
		{
			Event_LookAtEnemy( 1 );
			return SRESULT_WAIT;
		}

		parms->stage = 2;

		return SRESULT_WAIT;
	}

	if( parms->stage == 5 )
	{
		if( InAnimState( ANIMCHANNEL_LEGS, "Legs_StepRight" ) )
		{
			Event_LookAtEnemy( 1 );
			return SRESULT_WAIT;
		}

		parms->stage = 2;

		return SRESULT_WAIT;
	}


	if( parms->stage == 3 )
	{
		fire = false;
		if( InAnimState( ANIMCHANNEL_TORSO, "Torso_RangeAttack" ) )
		{
			return SRESULT_WAIT;
		}

		goto done;
	}
done:
	// don't attack for a bit
	nextAttack = gameLocal.RandomDelay( ZSECP_ATTACK_DELAY_MIN, ZSECP_ATTACK_DELAY_MAX );
	nextNoFOVAttack = gameLocal.SysScriptTime() + ZSECP_NOFOVTIME;
	return SRESULT_DONE;
}

/*
=====================
rvmMonsterZombieSecurityPistol::crouch_attack
=====================
*/
stateResult_t rvmMonsterZombieSecurityPistol::crouch_attack( stateParms_t* parms )
{
	if( parms->stage == 0 )
	{
		zsecp_num_crouch_attacks++;

		Event_FaceEnemy();
		crouch_fire = true;

		attackTime = gameLocal.RandomDelay( ZSECP_CROUCH_ATTACK_MIN_LENGTH, ZSECP_CROUCH_ATTACK_MAX_LENGTH );
		parms->stage = 1;
		return SRESULT_WAIT;
	}

	if( parms->stage == 1 )
	{
		if( AI_ENEMY_VISIBLE )
		{
			if( gameLocal.InfluenceActive() )
			{
				crouch_fire = false;
				parms->stage = 2; // break
				return SRESULT_WAIT;
			}
			if( gameLocal.SysScriptTime() > attackTime )
			{
				parms->stage = 2; // break
				return SRESULT_WAIT;
			}

			if( AI_PAIN )
			{
				int attack_flags = check_attacks();
				if( attack_flags & ( ATTACK_DODGE_LEFT | ATTACK_DODGE_RIGHT ) )
				{
					crouch_fire = false;
					Event_SetAnimPrefix( "" );
					do_attack( attack_flags );
					stateThread.PostState( "state_Combat" );
					return SRESULT_DONE;
				}
			}

			Event_LookAtEnemy( 1 );
			if( !CanHitEnemyFromAnim( "range_attack_loop" ) )
			{
				parms->stage = 2; // break
				return SRESULT_WAIT;
			}
		}
		else
		{
			parms->stage = 2;
		}

		return SRESULT_WAIT;
	}

	if( parms->stage == 2 )
	{
		crouch_fire = false;
		if( InAnimState( ANIMCHANNEL_TORSO, "Torso_CrouchAttack" ) )
		{
			return SRESULT_WAIT;
		}
		goto done;
	}
done:
	// don't attack for a bit
	nextAttack = gameLocal.RandomDelay( ZSECP_ATTACK_DELAY_MIN, ZSECP_ATTACK_DELAY_MAX );
	nextNoFOVAttack = gameLocal.SysScriptTime() + ZSECP_NOFOVTIME;

	return SRESULT_DONE;
}


/*
=====================
rvmMonsterZombieSecurityPistol::combat_dodge_left
=====================
*/
stateResult_t rvmMonsterZombieSecurityPistol::combat_dodge_left( stateParms_t* parms )
{
	if( parms->stage == 0 )
	{
		Event_StopMove();
		Event_FaceEnemy();
		Event_AnimState( ANIMCHANNEL_LEGS, "Legs_DodgeLeft", 2 );
		SetWaitState( "strafe" );
		parms->stage = 1;
		return SRESULT_WAIT;
	}

	if( waitState != "" )
	{
		return SRESULT_WAIT;
	}

	parms->stage = 2;

	nextDodge = gameLocal.DelayTime( ZSECP_DODGE_RATE );
	return SRESULT_DONE;
}

/*
=====================
rvmMonsterZombieSecurityPistol::combat_dodge_right
=====================
*/
stateResult_t rvmMonsterZombieSecurityPistol::combat_dodge_right( stateParms_t* parms )
{
	if( parms->stage == 0 )
	{
		Event_StopMove();
		Event_FaceEnemy();
		Event_AnimState( ANIMCHANNEL_LEGS, "Legs_DodgeRight", 2 );
		SetWaitState( "strafe" );
		parms->stage = 1;
		return SRESULT_WAIT;
	}

	if( waitState != "" )
	{
		return SRESULT_WAIT;
	}

	parms->stage = 2;

	nextDodge = gameLocal.DelayTime( ZSECP_DODGE_RATE );
	return SRESULT_DONE;
}