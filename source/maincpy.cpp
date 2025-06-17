#include <discord_rpc.h>
#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Interfaces.hpp>
#include <GarrysMod/Lua/LuaInterface.h>
#include <thread>

namespace globals {
	const char* hostname;
	const char* mapname;
	const char* gamemode;
	int player_count;
	int max_players;

	GarrysMod::Lua::ILuaBase* lua;
	GarrysMod::Lua::CLuaInterface* lua_interface;
}

namespace discord {
	static void handle_disconnected(int error_code, const char* msg) {
		globals::lua_interface->Msg("vltg_ds::discord::handle_disconnected() -> disconnected from discord: %d | %s\n", error_code, msg);
		return;
	}

	static void handle_errored(int error_code, const char* msg) {
		globals::lua_interface->Msg("vltg_ds::discord::handle_errored() -> got a error from discord: %d | %s\n", error_code, msg);
		return;
	}

	static void handle_joingame(const char* join_secret) {
		return;
	}

	static void handle_ready(const DiscordUser* usr) {
		globals::lua_interface->Msg("vltg_ds::discord::handle_ready() -> connected to discord. username: %s\n", usr->username);
		return;
	}

	static void handle_spectate(const char* spectate_secret) {
		return;
	}

	void init() {
		const char* ds_client_id = "1384291447594811534";
		DiscordEventHandlers handlers = { 0 };
		handlers.disconnected = handle_disconnected;
		handlers.errored = handle_errored;
		handlers.joinGame = handle_joingame;
		handlers.ready = handle_ready;
		handlers.spectateGame = handle_spectate;

		Discord_Initialize(ds_client_id, &handlers, 0, "");
	}
}

namespace module {
	GarrysMod::Lua::ILuaBase* lua_b;
	GarrysMod::Lua::CLuaInterface* lua_interface_b;

	static void pre_init(GarrysMod::Lua::ILuaBase* lua_base) {
		globals::lua = lua_base;
		globals::lua_interface = (GarrysMod::Lua::CLuaInterface*)lua_base;
		lua_b = lua_base;
		lua_interface_b = (GarrysMod::Lua::CLuaInterface*)lua_base;

		lua_b->CreateTable();

		lua_b->PushString("vltg_ds");
		lua_b->SetField(-2, "Version");

		lua_b->PushNumber(010000);
		lua_b->SetField(-2, "VersionNum");
	}

	static void deinit() {
		lua_b->PushNil();
		lua_b->SetField(GarrysMod::Lua::INDEX_GLOBAL, "vltg_ds");
	}

	void get_static_values() {
		{
			lua_b->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			lua_b->GetField(-1, "GetHostName");
			lua_b->Call(0, 1);

			globals::hostname = lua_b->GetString(-1);

			lua_b->Pop(2);
		}

		{
			lua_b->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			lua_b->GetField(-1, "game");
			lua_b->GetField(-1, "GetMap");
			lua_b->Call(0, 1);

			globals::mapname = lua_b->GetString(-1);

			lua_b->Pop(3);
		}

		{
			lua_b->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			lua_b->GetField(-1, "engine");
			lua_b->GetField(-1, "ActiveGamemode");
			lua_b->Call(0, 1);

			globals::gamemode = lua_b->GetString(-1);

			lua_b->Pop(3);
		}

		{
			lua_b->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			lua_b->GetField(-1, "game");
			if (!lua_b->IsType(-1, GarrysMod::Lua::Type::Table)) {
				lua_b->Pop(2);
				return;
			}
			lua_b->GetField(-1, "MaxPlayers");
			if (!lua_b->IsType(-1, GarrysMod::Lua::Type::Function)) {
				lua_b->Pop(2);
				return;
			}
			lua_b->Call(0, 1);

			globals::max_players = lua_b->GetNumber(-1);

			lua_b->Pop(3);
		}

		lua_interface_b->Msg("vltg_ds::get_static_values() -> mapname: %s, hostname: %s, gamemode: %s, max_players: %d\n", globals::mapname, globals::hostname, globals::gamemode, globals::max_players);
	}

	void get_dynamic_values() {
		{
			lua_b->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			lua_b->GetField(-1, "player");
			if (!lua_b->IsType(-1, GarrysMod::Lua::Type::Table)) {
				lua_b->Pop(2);
				return;
			}
			lua_b->GetField(-1, "GetCount");
			if (!lua_b->IsType(-1, GarrysMod::Lua::Type::Function)) {
				lua_b->Pop(2);
				return;
			}
			lua_b->Call(0, 1);

			globals::player_count = lua_b->GetNumber(-1);

			lua_b->Pop(3);
		}

		lua_interface_b->Msg("vltg_ds::get_dynamic_values() -> player_count: %d\n", globals::player_count);
	}

	LUA_FUNCTION_STATIC(update) {
		lua_interface_b->Msg("vltg_ds::update() -> getting dynamic values\n");
		get_dynamic_values();
		return 0;
	}

	static void init() {
		lua_b->PushCFunction(update);
		lua_b->SetField(-2, "update");
		lua_b->SetField(GarrysMod::Lua::INDEX_GLOBAL, "vltg_ds");

		lua_interface_b->Msg("vltg_ds::init() -> getting server info\n");

		get_static_values();
		get_dynamic_values();

		// discord::init();
	}
}

GMOD_MODULE_OPEN() {
	module::pre_init(LUA);
	module::init();
	return 1;
}

GMOD_MODULE_CLOSE() {
	module::deinit();
	return 0;
}