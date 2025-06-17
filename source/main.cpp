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
	bool discord_initialized;
	bool update_needed;
	bool inmenu;

	GarrysMod::Lua::ILuaBase* lua_g;
	GarrysMod::Lua::CLuaInterface* lua_interface_g;
}

namespace discord {
	static void handle_disconnected(int error_code, const char* msg) {
		globals::lua_interface_g->Msg("vltg_ds::discord::handle_disconnected() -> disconnected from discord with msg %s and error_code %d\n", msg, error_code);
		globals::discord_initialized = false;

		return;
	}

	static void handle_errored(int error_code, const char* msg) {
		globals::lua_interface_g->Msg("vltg_ds::discord::handle_errored() -> got a error from discord with msg %s and error_code %d\n", msg, error_code);

		return;
	}

	static void handle_joingame(const char* join_secret) {
		return;
	}

	static void handle_joinrequest(const DiscordUser* usr) {
		// nah
		Discord_Respond(usr->userId, DISCORD_REPLY_NO);

		return;
	}

	static void handle_ready(const DiscordUser* usr) {
		globals::lua_interface_g->Msg("vltg_ds::discord::handle_ready() -> connected to discord with user %s\n", usr->username);
		globals::discord_initialized = true;

		return;
	}

	static void handle_spectate(const char* spectate_secret) {
		return;
	}

	static void update_rpc() {
		if (!globals::discord_initialized || !globals::update_needed) {
			return;
		}

		Discord_ClearPresence();

		char buffer[256];
		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));
		discordPresence.state = "Playing Garry's Mod";
		sprintf(buffer, "Server Name: %s | Map Name: %s | Gamemode: %s | Player Count: %d | Max Players: %d",
			globals::hostname,
			globals::mapname,
			globals::gamemode,
			globals::player_count,
			globals::max_players
		);
		discordPresence.details = buffer;
		discordPresence.startTimestamp = 0;
		discordPresence.endTimestamp = time(0) + 5 * 60;
		discordPresence.largeImageKey = "garrysmod";
		discordPresence.instance = 0;

		Discord_UpdatePresence(&discordPresence);

		globals::update_needed = false;
	}

	void init() {
		static const char* ds_client_id = "1384291447594811534";
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
		handlers.disconnected = handle_disconnected;
		handlers.errored = handle_errored;
		handlers.joinGame = handle_joingame;
		handlers.ready = handle_ready;
		handlers.spectateGame = handle_spectate;
		handlers.joinRequest = handle_joinrequest;

		Discord_Initialize(ds_client_id, &handlers, 1, NULL);

		std::thread([]() {
			while (true) {
				Discord_RunCallbacks();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}).detach();

		globals::lua_interface_g->Msg("vltg_ds::discord::init() -> done\n");
	}
}

namespace module {
	static void pre_init() {
		globals::lua_g->CreateTable();

		globals::lua_g->PushString("vltg_ds");
		globals::lua_g->SetField(-2, "Version");

		globals::lua_g->PushNumber(010000);
		globals::lua_g->SetField(-2, "VersionNum");
	}

	static void deinit() {
		globals::lua_g->PushNil();
		globals::lua_g->SetField(GarrysMod::Lua::INDEX_GLOBAL, "vltg_ds");
	}

	void get_static_values() {
		{
			globals::lua_g->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			globals::lua_g->GetField(-1, "game");
			globals::lua_g->GetField(-1, "GetMap");
			globals::lua_g->Call(0, 1);

			globals::mapname = globals::lua_g->GetString(-1);

			globals::lua_g->Pop(3);
		}

		if (globals::mapname == "menu") {
			globals::inmenu = true;
			return;
		}

		globals::inmenu = false;

		{
			globals::lua_g->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			globals::lua_g->GetField(-1, "GetHostName");
			globals::lua_g->Call(0, 1);

			const char* l_name = globals::lua_g->GetString(-1);
			if (l_name != globals::hostname) {
				globals::update_needed = true;
			}

			globals::hostname = l_name;

			globals::lua_g->Pop(2);
		}

		{
			globals::lua_g->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			globals::lua_g->GetField(-1, "engine");
			globals::lua_g->GetField(-1, "ActiveGamemode");
			globals::lua_g->Call(0, 1);

			globals::gamemode = globals::lua_g->GetString(-1);

			globals::lua_g->Pop(3);
		}

		{
			globals::lua_g->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			globals::lua_g->GetField(-1, "game");
			if (!globals::lua_g->IsType(-1, GarrysMod::Lua::Type::Table)) {
				globals::lua_g->Pop(2);
				return;
			}
			globals::lua_g->GetField(-1, "MaxPlayers");
			if (!globals::lua_g->IsType(-1, GarrysMod::Lua::Type::Function)) {
				globals::lua_g->Pop(2);
				return;
			}
			globals::lua_g->Call(0, 1);

			globals::max_players = globals::lua_g->GetNumber(-1);

			globals::lua_g->Pop(3);
		}
	}

	void get_dynamic_values() {
		{
			globals::lua_g->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			globals::lua_g->GetField(-1, "player");
			if (!globals::lua_g->IsType(-1, GarrysMod::Lua::Type::Table)) {
				globals::lua_g->Pop(2);
				return;
			}
			globals::lua_g->GetField(-1, "GetCount");
			if (!globals::lua_g->IsType(-1, GarrysMod::Lua::Type::Function)) {
				globals::lua_g->Pop(2);
				return;
			}
			globals::lua_g->Call(0, 1);

			int l_count = globals::lua_g->GetNumber(-1);
			// to avoid unpleasant rpc "flickering" in the discord profile
			if (l_count != globals::player_count) {
				globals::update_needed = true;
			}

			globals::player_count = l_count;

			globals::lua_g->Pop(3);
		}
	}

	LUA_FUNCTION_STATIC(update) {
		if (globals::inmenu) {
			get_static_values();
			return 0;
		}


		get_static_values();
		get_dynamic_values();
		discord::update_rpc();
		return 0;
	}

	static void init() {
		globals::lua_g->PushCFunction(update);
		globals::lua_g->SetField(-2, "update");
		globals::lua_g->SetField(GarrysMod::Lua::INDEX_GLOBAL, "vltg_ds");

		get_static_values();
		discord::init();

		globals::lua_interface_g->Msg("vltg_ds::init() -> done: %s | %s | %s | %d | %d\n", globals::mapname, globals::hostname, globals::gamemode, globals::max_players, globals::player_count);
	}
}

GMOD_MODULE_OPEN() {
	globals::lua_g = LUA;
	globals::lua_interface_g = (GarrysMod::Lua::CLuaInterface*)LUA;

	module::pre_init();
	module::init();
	return 1;
}

GMOD_MODULE_CLOSE() {
	module::deinit();
	return 0;
}