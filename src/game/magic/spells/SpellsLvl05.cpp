/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game/magic/spells/SpellsLvl05.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "game/Damage.h"
#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/spells/Spells05.h"
#include "physics/Collisions.h"
#include "scene/GameSound.h"
#include "scene/Interactive.h"

void RuneOfGuardingSpell::Launch(long duration)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_RUNE_OF_GUARDING, m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING);
	m_exist = true;
	m_tolive = (duration > -1) ? duration : 99999999;
	
	CRuneOfGuarding * effect = new CRuneOfGuarding();
	effect->Create(entities[m_caster]->pos, 0);
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void RuneOfGuardingSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
		CRuneOfGuarding * pCRG=(CRuneOfGuarding *)m_pSpellFx;

		if (pCRG)
		{
			EERIE_SPHERE sphere;
			sphere.origin = pCRG->eSrc;
			sphere.radius=std::max(m_caster_level*15.f,50.f);

			if (CheckAnythingInSphere(&sphere,m_caster,CAS_NO_SAME_GROUP | CAS_NO_BACKGROUND_COL | CAS_NO_ITEM_COL| CAS_NO_FIX_COL | CAS_NO_DEAD_COL))
			{
				ARX_BOOMS_Add(pCRG->eSrc);
				LaunchFireballBoom(&pCRG->eSrc,(float)m_caster_level);
				DoSphericDamage(&pCRG->eSrc,4.f*m_caster_level,30.f*m_caster_level,DAMAGE_AREA,DAMAGE_TYPE_FIRE | DAMAGE_TYPE_MAGICAL,m_caster);
				m_tolive=0;
				ARX_SOUND_PlaySFX(SND_SPELL_RUNE_OF_GUARDING_END, &sphere.origin);
			}
		}
	}
}

void LevitateSpell::Launch(long duration, long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_LEVITATE, m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(m_caster == 0) {
		m_target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_START, &entities[m_target]->pos);
	m_exist = true;
	m_tolive = (duration > -1) ? duration : 2000000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	CLevitate * effect = new CLevitate();
	
	Vec3f target;
	if(m_target == 0) {
		target = player.pos + Vec3f(0.f, 150.f, 0.f);
		m_tolive = 200000000;
		player.levitate = true;
	} else {
		target = entities[m_target]->pos;
	}
	
	effect->Create(16, 50.f, 100.f, 80.f, &target, m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
	
	m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_LOOP,
	                                       &entities[m_target]->pos, 0.7f,
	                                       ARX_SOUND_PLAY_LOOPED);
	
	ARX_SPELLS_AddSpellOn(m_target, i);
}

void LevitateSpell::End(size_t i)
{
	ARX_SOUND_Stop(m_snd_loop);
	ARX_SOUND_PlaySFX(SND_SPELL_LEVITATE_END, &entities[m_target]->pos);
	ARX_SPELLS_RemoveSpellOn(m_target, i);
	
	if(m_target == 0)
		player.levitate = false;
}

void LevitateSpell::Update(float timeDelta)
{
	CLevitate *pLevitate=(CLevitate *)m_pSpellFx;
	Vec3f target;

	if(m_target == 0) {
		target.x=player.pos.x;
		target.y=player.pos.y+150.f;
		target.z=player.pos.z;
		player.levitate = true;
	} else {
		target.x = entities[m_caster]->pos.x;
		target.y = entities[m_caster]->pos.y;
		target.z = entities[m_caster]->pos.z;
	}

	pLevitate->ChangePos(&target);
		
	CSpellFx *pCSpellFX = m_pSpellFx;

	if(pCSpellFX) {
		pCSpellFX->Update(timeDelta);
		pCSpellFX->Render();
	}
	ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
}

void CurePoisonSpell::Launch(long i)
{
	if(m_caster == 0) {
		m_target = 0;
	}
	
	float cure = m_caster_level * 10;
	if(m_target == 0) {
		player.poison -= std::min(player.poison, cure);
		ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON);
	} else if (ValidIONum(m_target)) {
		Entity * io = entities[m_target];
		if(io->ioflags & IO_NPC) {
			io->_npcdata->poisonned -= std::min(io->_npcdata->poisonned, cure);
		}
		ARX_SOUND_PlaySFX(SND_SPELL_CURE_POISON, &io->pos);
	}
	
	m_exist = true;
	m_timcreation = (unsigned long)(arxtime);
	m_tolive = 3500;
	
	CCurePoison * effect = new CCurePoison();
	effect->spellinstance = i;
	effect->Create();
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void CurePoisonSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}

void RepelUndeadSpell::Launch(long duration, long i)
{
	long iCancel = ARX_SPELLS_GetInstanceForThisCaster(SPELL_REPEL_UNDEAD, m_caster);
	if(iCancel > -1) {
		spells[iCancel].m_tolive = 0;
	}
	
	if(m_caster == 0) {
		m_target = 0;
	}
	
	ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD, &entities[m_target]->pos);
	if(m_target == 0) {
		m_snd_loop = ARX_SOUND_PlaySFX(SND_SPELL_REPEL_UNDEAD_LOOP,
		                                       &entities[m_target]->pos, 1.f,
		                                       ARX_SOUND_PLAY_LOOPED);
	}
	
	m_exist = true;
	m_tolive = (duration > -1) ? duration : 20000000;
	m_bDuration = true;
	m_fManaCostPerSecond = 1.f;
	
	CRepelUndead * effect = new CRepelUndead();
	effect->spellinstance = i;
	effect->Create(player.pos, MAKEANGLE(player.angle.getPitch()));
	effect->SetDuration(m_tolive);
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void RepelUndeadSpell::End()
{
	if(lightHandleIsValid(m_longinfo_light)) {
		EERIE_LIGHT * light = lightHandleGet(m_longinfo_light);
		
		light->duration = 200;
		light->time_creation = (unsigned long)(arxtime);
	}
	m_longinfo_light = -1;
	
	ARX_SOUND_Stop(m_snd_loop);
}

void RepelUndeadSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();

		if (m_target == 0)
			ARX_SOUND_RefreshPosition(m_snd_loop, entities[m_target]->pos);
	}
}

void PoisonProjectileSpell::Launch(long i)
{
	ARX_SOUND_PlaySFX(SND_SPELL_POISON_PROJECTILE_LAUNCH,
	                  &m_caster_pos);
	
	m_exist = true;
	m_tolive = 900000000; // TODO probably never read
	
	long level = std::max(long(m_caster_level), 1l);
	CMultiPoisonProjectile * effect = new CMultiPoisonProjectile(level);
	effect->spellinstance = i;
	effect->SetDuration(8000ul);
	float ang;
	if(m_caster == 0) {
		ang = player.angle.getPitch();
	} else {
		ang = entities[m_caster]->angle.getPitch();
	}
	effect->Create(Vec3f_ZERO, MAKEANGLE(ang));
	m_pSpellFx = effect;
	m_tolive = effect->GetDuration();
}

void PoisonProjectileSpell::Update(float timeDelta)
{
	if(m_pSpellFx) {
		m_pSpellFx->Update(timeDelta);
		m_pSpellFx->Render();
	}
}