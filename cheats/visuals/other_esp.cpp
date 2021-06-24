// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "other_esp.h"
#include "..\autowall\autowall.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "..\lagcompensation\local_animations.h"

bool can_penetrate(weapon_t* weapon)
{
	auto weapon_info = weapon->get_csweapon_info();

	if (!weapon_info)
		return false;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	Vector
		start = g_ctx.globals.eye_pos,
		end = start + (direction * 8192.f);

	return c_autowall::get().calculate_return_info(start, end, g_ctx.local()).m_did_penetrate_wall;
}

void otheresp::penetration_reticle()
{
	if (!g_cfg.player.enable)
		return;

	if (!g_cfg.esp.penetration_reticle)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	auto color = Color::Red;

	if (!weapon->is_non_aim() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && can_penetrate(weapon))
		color = Color::Green;

	static int width, height;
	m_engine()->GetScreenSize(width, height);
	
	render::get().rect_filled(width / 2, height / 2 - 1, 1, 3, color);
	render::get().rect_filled(width / 2 - 1, height / 2, 3, 1, color);
}

void otheresp::indicators()
{
	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	int wid, hei;
	m_engine()->GetScreenSize(wid, hei);


	auto normpos = 0;




	if (g_cfg.esp.indicators[INDICATOR_FAKE] && (antiaim::get().type == ANTIAIM_LEGIT || g_cfg.antiaim.type[antiaim::get().type].desync))
	{
		normpos++;
		auto color = Color(130, 20, 20);
		auto animstate = g_ctx.local()->get_animation_state();
		int desync_delta = 1;
		int delta = 1;
		if (animstate && local_animations::get().local_data.animstate)
		{
			auto delta = fabs(math::normalize_yaw(animstate->m_flGoalFeetYaw - local_animations::get().local_data.animstate->m_flGoalFeetYaw));
			auto desync_delta = max(g_ctx.local()->get_max_desync_delta(), 58.0f);

			color = Color(130, 20 + (int)(min(delta / desync_delta, 1.0f) * 150.0f), 20);

		//	render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "FAKE");
			for (auto i = 0; i <= (int)(min(delta / desync_delta, 1.0f) * 50.0f); i++)
			{
				auto side = antiaim::get().desync_angle > 0.0f ? i : -i;
				auto da = antiaim::get().desync_angle > 0.0f ? ">" : "<"  ;
				auto idk = antiaim::get().desync_angle > 0.0f ? +30 : -30;
				if (antiaim::get().type == ANTIAIM_LEGIT)
				{
					side = antiaim::get().desync_angle > 0.0f ? i : -i;
					auto idk = antiaim::get().desync_angle > 0.0f ? -30 : +30;
					//INDICATORFONT_new
				}
				
				render::get().text(fonts[INDICATORFONT_new], ( wid/2 + (idk) ) + (side * 5), hei - (normpos*13), Color(g_cfg.esp.indcolorr.r() + (i * 2), g_cfg.esp.indcolorr.g() + (i * 2), g_cfg.esp.indcolorr.b() + (i * 2), g_cfg.esp.indcolorr.a()), HFONT_CENTERED_Y, da);

			}
			auto type = antiaim::get().type == ANTIAIM_LEGIT ? "REAL" : "FAKE";
			render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, type);
		}



	//	m_indicators.push_back(m_indicator("FAKE", color));
	}










	auto choke_indicator = false;



	if (g_cfg.esp.indicators[INDICATOR_CHOKE] && !fakelag::get().condition && !misc::get().double_tap_enabled && !misc::get().hide_shots_enabled)
	{
		normpos++;

		auto color = Color(25, 255, 25);
		auto idk = antiaim::get().desync_angle > 0.0f ? +30 : -30;
		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "CHOKE");
		for(auto i=0 ; i<= fakelag::get().max_choke ; i++)
		{
			auto side = antiaim::get().desync_angle > 0.0f ? i : -i;
			auto da = antiaim::get().desync_angle > 0.0f ? ">" : "<";

			if (antiaim::get().type == ANTIAIM_LEGIT)
			{
				side = antiaim::get().desync_angle > 0.0f ? i : -i;
				auto idk = antiaim::get().desync_angle > 0.0f ? -30 : +30;
				//INDICATORFONT_new
			}


			
			render::get().text(fonts[INDICATORFONT_new], (wid / 2 + (idk)) + (side * 5), hei - (normpos * 13), Color(g_cfg.esp.indcolorl.r() + (i*2), g_cfg.esp.indcolorl.g()+ (i*2), g_cfg.esp.indcolorl.b() + (i * 2), g_cfg.esp.indcolorl.a()), HFONT_CENTERED_Y, da);
		}



		
		choke_indicator = true;
	}



	if (g_cfg.esp.indicators[INDICATOR_DESYNC_SIDE] && (antiaim::get().type == ANTIAIM_LEGIT && g_cfg.antiaim.desync == 1 || antiaim::get().type != ANTIAIM_LEGIT && g_cfg.antiaim.type[antiaim::get().type].desync == 1) && !antiaim::get().condition(g_ctx.get_command()))
	{
		normpos++;
		auto color = Color(25, 255, 25);
		auto side = antiaim::get().desync_angle > 0.0f ? "RIGHT" : "LEFT";

		if (antiaim::get().type == ANTIAIM_LEGIT)
			side = antiaim::get().desync_angle > 0.0f ? "LEFT" : "RIGHT";



		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, side);
	}



	if (g_cfg.esp.indicators[INDICATOR_DAMAGE] && g_ctx.globals.current_weapon != -1 && key_binds::get().get_key_bind_state(4 + g_ctx.globals.current_weapon) && !weapon->is_non_aim())
	{
		normpos++;
		auto color = Color(25, 255, 25);
		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "DAMAGE");
	
	}

	if (g_cfg.esp.indicators[INDICATOR_SAFE_POINTS] && key_binds::get().get_key_bind_state(3) && !weapon->is_non_aim())
	{
		normpos++;
		auto color = Color(25, 255, 25);
		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "SAFE POINT");
		

	}
	if (g_cfg.esp.indicators[INDICATOR_BODY_AIM] && key_binds::get().get_key_bind_state(22) && !weapon->is_non_aim())
	{
		normpos++;
		auto color = Color(25, 255, 25);
		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "BODY AIM");
	
	}

	if (choke_indicator)
		return;

	if (g_cfg.esp.indicators[INDICATOR_DT] && g_cfg.ragebot.double_tap && g_cfg.ragebot.double_tap_key.key > KEY_NONE && g_cfg.ragebot.double_tap_key.key < KEY_MAX && misc::get().double_tap_key)
	{
		auto color = Color(25, 255, 25);
		normpos++;
		if(!g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().double_tap_enabled && !weapon->is_grenade() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && weapon->can_fire(false))
		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "DT");
	}
	if (g_cfg.esp.indicators[INDICATOR_HS] && g_cfg.antiaim.hide_shots && g_cfg.antiaim.hide_shots_key.key > KEY_NONE && g_cfg.antiaim.hide_shots_key.key < KEY_MAX && misc::get().hide_shots_key)
	{
		auto color = Color(25, 255, 25);
		normpos++;
		if(!g_ctx.local()->m_bGunGameImmunity() && !(g_ctx.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().hide_shots_enabled)
		render::get().text(fonts[ESP], (wid / 2 - 5), hei - (normpos * 13), color, HFONT_CENTERED_Y, "HS");
	}

}

void otheresp::draw_indicators()
{
	if (!g_ctx.local()->is_alive())
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto h = height - 325;

	for (auto& indicator : m_indicators)
	{
    	auto text_width1 = render::get().text_width(fonts[INDICATORFONT_new], indicator.m_text.c_str());
		
		auto text_height1 = render::get().text_height(fonts[INDICATORFONT_new], indicator.m_text.c_str());
	//	render::get().gradient(27, h- (text_height1/2), text_width1, text_height1, Color(g_cfg.esp.indcolorl.r(), g_cfg.esp.indcolorl.g(), g_cfg.esp.indcolorl.b(), g_cfg.esp.indcolorl.a()), Color(g_cfg.esp.indcolorr.r(), g_cfg.esp.indcolorr.g(), g_cfg.esp.indcolorr.b(), g_cfg.esp.indcolorr.a()), GRADIENT_HORIZONTAL); //GRADIENT_HORIZONTAL

		render::get().text(fonts[INDICATORFONT_new], 27, h, indicator.m_color, HFONT_CENTERED_Y, indicator.m_text.c_str());
		h -= 25;

	
	}

	m_indicators.clear();
}

void otheresp::hitmarker_paint()
{
	if (!g_cfg.esp.hitmarker[0] && !g_cfg.esp.hitmarker[1])
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if (!g_ctx.local()->is_alive())
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if (hitmarker.hurt_time + 0.7f > m_globals()->m_curtime)
	{
		if (g_cfg.esp.hitmarker[0])
		{
			static int width, height;
			m_engine()->GetScreenSize(width, height);

			auto alpha = (int)((hitmarker.hurt_time + 0.7f - m_globals()->m_curtime) * 255.0f);
			hitmarker.hurt_color.SetAlpha(alpha);

			auto offset = 7.0f - (float)alpha / 255.0f * 7.0f;

			render::get().line(width / 2 + 5 + offset, height / 2 - 5 - offset, width / 2 + 12 + offset, height / 2 - 12 - offset, hitmarker.hurt_color);
			render::get().line(width / 2 + 5 + offset, height / 2 + 5 + offset, width / 2 + 12 + offset, height / 2 + 12 + offset, hitmarker.hurt_color);
			render::get().line(width / 2 - 5 - offset, height / 2 + 5 + offset, width / 2 - 12 - offset, height / 2 + 12 + offset, hitmarker.hurt_color);
			render::get().line(width / 2 - 5 - offset, height / 2 - 5 - offset, width / 2 - 12 - offset, height / 2 - 12 - offset, hitmarker.hurt_color);
		}

		if (g_cfg.esp.hitmarker[1])
		{
			Vector world;

			if (math::world_to_screen(hitmarker.point, world))
			{
				auto alpha = (int)((hitmarker.hurt_time + 0.7f - m_globals()->m_curtime) * 255.0f);
				hitmarker.hurt_color.SetAlpha(alpha);

				auto offset = 7.0f - (float)alpha / 255.0f * 7.0f;

				render::get().line(world.x + 5 + offset, world.y - 5 - offset, world.x + 12 + offset, world.y - 12 - offset, hitmarker.hurt_color);
				render::get().line(world.x + 5 + offset, world.y + 5 + offset, world.x + 12 + offset, world.y + 12 + offset, hitmarker.hurt_color);
				render::get().line(world.x - 5 - offset, world.y + 5 + offset, world.x - 12 - offset, world.y + 12 + offset, hitmarker.hurt_color);
				render::get().line(world.x - 5 - offset, world.y - 5 - offset, world.x - 12 - offset, world.y - 12 - offset, hitmarker.hurt_color);
			}
		}
	}
}

void otheresp::damage_marker_paint()
{
	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		if (damage_marker[i].hurt_time + 2.0f > m_globals()->m_curtime)
		{
			Vector screen;

			if (!math::world_to_screen(damage_marker[i].position, screen))
				continue;

			auto alpha = (int)((damage_marker[i].hurt_time + 2.0f - m_globals()->m_curtime) * 127.5f);
			damage_marker[i].hurt_color.SetAlpha(alpha);

			render::get().text(fonts[DAMAGE_MARKER], screen.x, screen.y, damage_marker[i].hurt_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "%i", damage_marker[i].damage);
		}
	}
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device);

void otheresp::spread_crosshair(LPDIRECT3DDEVICE9 device)
{
	if (!g_cfg.player.enable)
		return;

	if (!g_cfg.esp.show_spread)
		return;

	if (!g_ctx.local()->is_alive())
		return;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (weapon->is_non_aim())
		return;

	int w, h;
	m_engine()->GetScreenSize(w, h);

	draw_circe((float)w * 0.5f, (float)h * 0.5f, g_ctx.globals.inaccuracy * 500.0f, 50, D3DCOLOR_RGBA(g_cfg.esp.show_spread_color.r(), g_cfg.esp.show_spread_color.g(), g_cfg.esp.show_spread_color.b(), g_cfg.esp.show_spread_color.a()), D3DCOLOR_RGBA(0, 0, 0, 0), device);
}

void draw_circe(float x, float y, float radius, int resolution, DWORD color, DWORD color2, LPDIRECT3DDEVICE9 device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2 = nullptr;
	std::vector <CUSTOMVERTEX2> circle(resolution + 2);

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0.0f;

	circle[0].rhw = 1.0f;
	circle[0].color = color2;

	for (auto i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - radius * cos(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - radius * sin(D3DX_PI * ((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0.0f;

		circle[i].rhw = 1.0f;
		circle[i].color = color;
	}

	device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX2), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, nullptr); //-V107

	if (!g_pVB2)
		return;

	void* pVertices;

	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX2), (void**)&pVertices, 0); //-V107
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX2));
	g_pVB2->Unlock();

	device->SetTexture(0, nullptr);
	device->SetPixelShader(nullptr);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX2));
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);

	g_pVB2->Release();
}

void otheresp::automatic_peek_indicator()
{
	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	static auto position = ZERO;

	if (!g_ctx.globals.start_position.IsZero())
		position = g_ctx.globals.start_position;

	if (position.IsZero())
		return;

	static auto alpha = 0.0f;

	if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(18) || alpha)
	{
		if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(18))
			alpha += 3.0f * m_globals()->m_frametime;
		else
			alpha -= 3.0f * m_globals()->m_frametime;

		alpha = math::clamp(alpha, 0.0f, 1.0f);
		render::get().Draw3DFilledCircle(position, 25.0f, g_ctx.globals.fired_shot ? Color(30, 240, 30, (int)(alpha * 255.0f)) : Color(240, 30, 30, (int)(alpha * 255.0f)));

		Vector screen;

		if (math::world_to_screen(position, screen))
		{
			static auto offset = 30.0f;

			if (!g_ctx.globals.fired_shot)
			{
				static auto switch_offset = false;

				if (offset <= 30.0f || offset >= 55.0f)
					switch_offset = !switch_offset;

				offset += switch_offset ? 22.0f * m_globals()->m_frametime : -22.0f * m_globals()->m_frametime;
				offset = math::clamp(offset, 30.0f, 55.0f);

				
			
			}
		}
	}
}