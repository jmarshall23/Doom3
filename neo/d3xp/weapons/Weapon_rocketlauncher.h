// Weapon_rocketlauncher.h
//

#pragma once

class rvmWeaponRocketLauncher : public rvmWeaponObject
{
public:
	CLASS_PROTOTYPE( rvmWeaponRocketLauncher );

	virtual void			Init( idWeapon* weapon );

	stateResult_t			Raise( stateParms_t* parms );
	stateResult_t			Lower( stateParms_t* parms );
	stateResult_t			Idle( stateParms_t* parms );
	stateResult_t			Fire( stateParms_t* parms );
	stateResult_t			Reload( stateParms_t* parms );
private:
	void					UpdateSkin();

	float		spread;
	idStr		skin_invisible;
};