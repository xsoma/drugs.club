// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "prediction_system.h"

void engineprediction::store_netvars()
{
	auto data = &netvars_data[m_clientstate()->iCommandAck % MULTIPLAYER_BACKUP]; //-V807

	data->tickbase = g_ctx.local()->m_nTickBase(); //-V807
	data->m_aimPunchAngle = g_ctx.local()->m_aimPunchAngle();
	data->m_aimPunchAngleVel = g_ctx.local()->m_aimPunchAngleVel();
	data->m_viewPunchAngle = g_ctx.local()->m_viewPunchAngle();
	data->m_vecViewOffset = g_ctx.local()->m_vecViewOffset();
	data->m_duckAmount = g_ctx.local()->m_flDuckAmount();
	data->m_duckSpeed = g_ctx.local()->m_flDuckSpeed();
	data->m_origin = g_ctx.local()->m_vecOrigin();
	data->m_velocity = g_ctx.local()->m_vecVelocity();
	data->m_fall_velocity = g_ctx.local()->m_flFallVelocity();
	data->m_velocity_modifier = g_ctx.local()->m_flVelocityModifier();

}

void engineprediction::restore_netvars()
{
	auto data = &netvars_data[(m_clientstate()->iCommandAck - 1) % MULTIPLAYER_BACKUP]; //-V807

	if (data->tickbase != g_ctx.local()->m_nTickBase()) //-V807
		return;

	auto aim_punch_angle_delta = g_ctx.local()->m_aimPunchAngle() - data->m_aimPunchAngle;
	auto aim_punch_angle_vel_delta = g_ctx.local()->m_aimPunchAngleVel() - data->m_aimPunchAngleVel;
	auto view_punch_angle_delta = g_ctx.local()->m_viewPunchAngle() - data->m_viewPunchAngle;
	auto view_offset_delta = g_ctx.local()->m_vecViewOffset() - data->m_vecViewOffset;
	auto duck_amount = g_ctx.local()->m_flDuckAmount() - data->m_duckAmount;
	const auto velocity_diff = data->m_velocity - g_ctx.local()->m_vecVelocity();
	const auto origin_diff = data->m_origin - g_ctx.local()->m_vecOrigin();

	if (fabs(aim_punch_angle_delta.x) < 0.03125f && fabs(aim_punch_angle_delta.y) < 0.03125f && fabs(aim_punch_angle_delta.z) < 0.03125f)
		g_ctx.local()->m_aimPunchAngle() = data->m_aimPunchAngle;

	if (fabs(aim_punch_angle_vel_delta.x) < 0.03125f && fabs(aim_punch_angle_vel_delta.y) < 0.03125f && fabs(aim_punch_angle_vel_delta.z) < 0.03125f)
		g_ctx.local()->m_aimPunchAngleVel() = data->m_aimPunchAngleVel;

	if (fabs(view_punch_angle_delta.x) < 0.03125f && fabs(view_punch_angle_delta.y) < 0.03125f && fabs(view_punch_angle_delta.z) < 0.03125f)
		g_ctx.local()->m_viewPunchAngle() = data->m_viewPunchAngle;

	if (fabs(view_offset_delta.x) < 0.03125f && fabs(view_offset_delta.y) < 0.03125f && fabs(view_offset_delta.z) < 0.03125f)
		g_ctx.local()->m_vecViewOffset() = data->m_vecViewOffset;

	if (fabs(duck_amount) < 0.03125f)
	{
		g_ctx.local()->m_flDuckAmount() = data->m_duckAmount;
		g_ctx.local()->m_flDuckSpeed() = data->m_duckSpeed;
	}

	if (abs(g_ctx.local()->m_flFallVelocity() - data->m_fall_velocity) <= 0.03125f)
		g_ctx.local()->m_flFallVelocity() = data->m_fall_velocity;

	if (std::abs(g_ctx.local()->m_flVelocityModifier() - data->m_velocity_modifier) <= 0.00625f)
		g_ctx.local()->m_flVelocityModifier() = data->m_velocity_modifier;
}

void engineprediction::update_incoming_sequences() {
	if (!m_clientstate()->pNetChannel)
		return;

	if (m_sequence.empty() || m_clientstate()->pNetChannel->m_nInSequenceNr > m_sequence.front().m_seq) {
		// store new stuff.
		m_sequence.emplace_front(m_globals()->m_realtime, m_clientstate()->pNetChannel->m_nInReliableState, m_clientstate()->pNetChannel->m_nInSequenceNr);
	}

	// do not save too many of these.
	while (m_sequence.size() > 2048)
		m_sequence.pop_back();
}

void engineprediction::setup()
{
	if (prediction_data.prediction_stage != SETUP)
		return;

	if (!prediction_data.sv_footsteps)
		prediction_data.sv_footsteps = m_cvar()->FindVar(crypt_str("sv_footsteps"));

	if (!prediction_data.sv_min_jump_landing_sound)
		prediction_data.sv_min_jump_landing_sound = m_cvar()->FindVar(crypt_str("sv_min_jump_landing_sound"));

	backup_data.flags = g_ctx.local()->m_fFlags(); //-V807
	backup_data.velocity = g_ctx.local()->m_vecVelocity();

	backup_data.sv_footsteps_backup = *(float*)(uintptr_t(prediction_data.sv_footsteps) + 0x2C);
	backup_data.sv_min_jump_landing_sound_backup = *(float*)(uintptr_t(prediction_data.sv_min_jump_landing_sound) + 0x2C);

	prediction_data.old_curtime = m_globals()->m_curtime; //-V807
	prediction_data.old_frametime = m_globals()->m_frametime;

	m_globals()->m_curtime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);
	m_globals()->m_frametime = m_prediction()->EnginePaused ? 0.0f : m_globals()->m_intervalpertick;

	prediction_data.prediction_stage = PREDICT;
}

void engineprediction::predict(CUserCmd* m_pcmd)
{
	if (prediction_data.prediction_stage != PREDICT)
		return;

	bool prediction_need_to_recount = false;

	if (m_clientstate()->iDeltaTick > 0) {
		m_prediction()->Update(
			m_clientstate()->iDeltaTick,
			m_clientstate()->iDeltaTick > 1,
			m_clientstate()->nLastCommandAck,
			m_clientstate()->nLastOutgoingCommand + m_clientstate()->iChokedCommands);
	}

	if (prediction_need_to_recount) { // predict recount compression.
		*(int*)((uintptr_t)m_prediction() + 0x1C) = 0;
		*(bool*)((uintptr_t)m_prediction() + 0x24) = true;
	}

	// backup footsteps.
	const auto backup_footsteps = backup_data.sv_footsteps_backup;
	float value = 0.0f;
	if (prediction_data.sv_footsteps)
		*(float*)(uintptr_t(prediction_data.sv_footsteps) + 0x2C) = (uint32_t)prediction_data.sv_footsteps ^ uint32_t(value);

	if (prediction_data.sv_min_jump_landing_sound)
		*(float*)(uintptr_t(prediction_data.sv_min_jump_landing_sound) + 0x2C) = (uint32_t)prediction_data.sv_min_jump_landing_sound ^ 0x7F7FFFFF;

	int m_nimpulse = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_nImpulse"));
	int get_buttons = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_nButtons"));
	int get_buttons_last = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_afButtonLast"));
	int get_buttons_pressed = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_afButtonPressed"));
	int get_buttons_released = util::find_in_datamap(g_ctx.local()->GetPredDescMap(), crypt_str("m_afButtonReleased"));

	prediction_data.m_nServerCommandsAcknowledged = *(int*)(uintptr_t(m_prediction()) + 0x20);
	prediction_data.m_bInPrediction = *(bool*)(uintptr_t(m_prediction()) + 8);

	*reinterpret_cast<CUserCmd**>(reinterpret_cast<uintptr_t>(g_ctx.local()) + 0x3288) = m_pcmd;

	if (!prediction_data.prediction_random_seed)
		prediction_data.prediction_random_seed = *reinterpret_cast <int**> (util::FindSignature(crypt_str("client.dll"), crypt_str("A3 ? ? ? ? 66 0F 6E 86")) + 0x1);

	*prediction_data.prediction_random_seed = MD5_PseudoRandom(m_pcmd->m_command_number) & INT_MAX;

	if (!prediction_data.prediction_player)
		prediction_data.prediction_player = *reinterpret_cast <int**> (util::FindSignature(crypt_str("client.dll"), crypt_str("89 35 ? ? ? ? F3 0F 10 48")) + 0x2);

	*prediction_data.prediction_player = reinterpret_cast <int> (g_ctx.local());

	auto buttons_forced = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(g_ctx.local()) + 0x3334);
	auto buttons_disabled = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(g_ctx.local()) + 0x3330);

	m_pcmd->m_buttons |= buttons_forced;
	m_pcmd->m_buttons &= ~buttons_disabled;

	m_gamemovement()->StartTrackPredictionErrors(g_ctx.local()); //-V807
	m_movehelper()->set_host(g_ctx.local());

	CMoveData move_data;
	memset(&move_data, 0, sizeof(CMoveData));

	if (m_pcmd->m_weaponselect) {
		auto weapon = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntity(m_pcmd->m_weaponselect));
		if (weapon) {
			auto weapon_data = weapon->get_csweapon_info();
			weapon_data ? g_ctx.local()->select_item(weapon_data->szWeaponName, m_pcmd->m_weaponsubtype) : 0;
		}
	}

	auto vehicle_handle = g_ctx.local()->m_hVehicle();
	auto vehicle = vehicle_handle.IsValid() ? reinterpret_cast<entity_t*>(vehicle_handle.Get()) : nullptr;

	if (m_pcmd->m_impulse
		&& (!vehicle || g_ctx.local()->using_standard_weapons_in_vechile()))
		m_nimpulse = m_pcmd->m_impulse;

	auto buttons = m_pcmd->m_buttons;
	auto buttons_changed = buttons ^ get_buttons;

	get_buttons_last = get_buttons;
	get_buttons = buttons;
	get_buttons_pressed = buttons_changed & buttons;
	get_buttons_released = buttons_changed & ~buttons;

	m_prediction()->CheckMovingGround(g_ctx.local(), m_globals()->m_frametime);

	g_ctx.local()->physics_run_think(0) ? g_ctx.local()->pre_think() : 0;

	if (g_ctx.local()->get_next_think_tick()
		&& g_ctx.local()->get_next_think_tick() != -1
		&& g_ctx.local()->get_next_think_tick() <= g_ctx.local()->m_nTickBase()) {
		g_ctx.local()->get_next_think_tick() = -1;
		g_ctx.local()->think();
	}

	m_prediction()->SetupMove(g_ctx.local(), m_pcmd, m_movehelper(), &move_data);
	m_gamemovement()->ProcessMovement(g_ctx.local(), &move_data);
	m_prediction()->FinishMove(g_ctx.local(), m_pcmd, &move_data);

	m_movehelper()->process_impacts();

	m_gamemovement()->FinishTrackPredictionErrors(g_ctx.local());
	m_movehelper()->set_host(nullptr);

	if (prediction_data.sv_footsteps)
		*(float*)(uintptr_t(prediction_data.sv_footsteps) + 0x2C) = backup_footsteps;

	auto viewmodel = g_ctx.local()->m_hViewModel().Get();

	if (viewmodel)
	{
		viewmodel_data.weapon = viewmodel->m_hWeapon().Get();

		viewmodel_data.viewmodel_index = viewmodel->m_nViewModelIndex();
		viewmodel_data.sequence = viewmodel->m_nSequence();
		viewmodel_data.animation_parity = viewmodel->m_nAnimationParity();

		viewmodel_data.cycle = viewmodel->m_flCycle();
		viewmodel_data.animation_time = viewmodel->m_flAnimTime();
	}

	prediction_data.prediction_stage = FINISH;
}

void engineprediction::finish()
{
	if (prediction_data.prediction_stage != FINISH)
		return;

	*prediction_data.prediction_random_seed = -1;
	*prediction_data.prediction_player = 0;

	*(int*)(uintptr_t(m_prediction()) + 0x20) = prediction_data.m_nServerCommandsAcknowledged; //m_bPreviousAckHadErrors
	*(bool*)(uintptr_t(m_prediction()) + 8) = prediction_data.m_bInPrediction; // m_bInPrediction

	m_globals()->m_curtime = prediction_data.old_curtime;
	m_globals()->m_frametime = prediction_data.old_frametime;
}