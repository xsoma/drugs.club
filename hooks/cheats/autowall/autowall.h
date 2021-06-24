#pragma once
#include "..\..\includes.hpp"

class weapon_info_t;
class weapon_t;

class c_autowall : public singleton< c_autowall > {
public:
	struct returninfo_t {
		int m_damage;
		bool valid = false;
		int m_hitgroup;
		int m_hitbox;
		int m_penetration_count;
		bool m_did_penetrate_wall;
		float m_thickness;
		Vector m_end;
		IClientEntity * m_hit_entity;
		bool visible = false;
		returninfo_t(int damage, int hitgroup, int hitbox, int penetration_count, bool did_penetrate_wall, float thickness, bool visible, IClientEntity * hit_entity) {
			m_damage = damage;
			 valid = true;
			 this->visible = visible;
			m_hitgroup = hitgroup;
			m_hitbox = hitbox;
			m_penetration_count = penetration_count;
			m_did_penetrate_wall = did_penetrate_wall;
			m_thickness = thickness;
			m_hit_entity = hit_entity;
		}
	};

	struct firebulletdata_t {
		Vector m_start;
		Vector m_end;
		Vector m_current_position;
		Vector m_direction;

		ITraceFilter * m_filter;
		trace_t m_enter_trace;

		float m_thickness;
		float m_current_damage;
		int m_penetration_count;
	};

	returninfo_t calculate_return_info(Vector start, Vector end, IClientEntity* from_entity = nullptr, IClientEntity* to_entity = nullptr, int specific_hitgroup = -1);
	void scale_damage(player_t* e, weapon_info_t* weapon_info, int hitgroup, float& current_damage);
	bool handle_bullet_penetration(weapon_info_t* info, firebulletdata_t& data);
	bool trace_to_exit(trace_t* enter_trace, Vector start, Vector dir, trace_t * exit_trace);

	void trace_line(Vector& start, Vector & end, unsigned int mask, IClientEntity* ignore, trace_t* trace);
	void clip_trace_to_player(Vector& start, Vector& end, player_t* e, unsigned int mask, ITraceFilter* filter, trace_t* old_trace);
	bool is_breakable_entity(IClientEntity* e);
};