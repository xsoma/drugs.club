#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

enum Prediction_stage
{
	SETUP,
	PREDICT,
	FINISH
};

class engineprediction : public singleton <engineprediction>
{
	struct Netvars_data
	{
		int tickbase = INT_MIN;

		Vector m_aimPunchAngle = ZERO;
		Vector m_aimPunchAngleVel = ZERO;
		Vector m_viewPunchAngle = ZERO;
		Vector m_vecViewOffset = ZERO;
		Vector m_origin = ZERO;
		Vector m_velocity = ZERO;


		float m_duckAmount = 0.f;
		float m_duckSpeed = 0.f;
		float m_fall_velocity = 0.f;
		float m_velocity_modifier = 0.f;
	};

	struct Backup_data
	{
		int flags = 0;
		Vector velocity = ZERO;
		float sv_footsteps_backup = 0.0f;
		float sv_min_jump_landing_sound_backup = 0.0f;
	};

	class m_nsequence {
	public:
		float m_time;
		int   m_state;
		int   m_seq;

	public:
		__forceinline m_nsequence() : m_time{ }, m_state{ }, m_seq{ } {};
		__forceinline m_nsequence(float time, int state, int seq) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
	};

	struct Prediction_data
	{
		void reset()
		{
			prediction_stage = SETUP;
			old_curtime = 0.0f;
			old_frametime = 0.0f;
		}

		Prediction_stage prediction_stage = SETUP;
		float old_curtime = 0.0f;
		float old_frametime = 0.0f;
		int* prediction_random_seed = nullptr;
		int* prediction_player = nullptr;
		ConVar* sv_footsteps = nullptr; // m_cvar()->FindVar(m_xor_str("sv_footsteps"))
		ConVar* sv_min_jump_landing_sound = nullptr; // m_cvar()->FindVar(m_xor_str("sv_min_jump_landing_sound"))
		int m_nServerCommandsAcknowledged;
		bool m_bInPrediction;
	};

	struct Viewmodel_data
	{
		weapon_t* weapon = nullptr;

		int viewmodel_index = 0;
		int sequence = 0;
		int animation_parity = 0;

		float cycle = 0.0f;
		float animation_time = 0.0f;
	};
public:
	Netvars_data netvars_data[MULTIPLAYER_BACKUP];
	Backup_data backup_data;
	Prediction_data prediction_data;
	Viewmodel_data viewmodel_data;
	std::deque< m_nsequence > m_sequence;

	void store_netvars();
	void restore_netvars();
	void update_incoming_sequences();
	void setup();
	void predict(CUserCmd* m_pcmd);
	void finish();
};