#include <discord_rpc.h>
#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Interfaces.hpp>
#include <GarrysMod/Lua/LuaInterface.h>
#include "GameEventListener.h"
#include <thread>

namespace globals {
	const char* hostname;
	const char* mapname;
	const char* gamemode;
	bool discord_initialized;
	bool inmenu = true;

	IGameEventManager2* event_mgr;

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

	static void init() {
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

		globals::discord_initialized = true;

		globals::lua_interface_g->Msg("vltg_ds::discord::init() -> done\n");
	}

	static void update_rpc() {
		if (!globals::discord_initialized) {
			init();
			return;
		}

		Discord_ClearPresence();

		char details[128];
		char state[128];
		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));
		snprintf(details, sizeof(details), "Playing on %s", globals::hostname);
		snprintf(state, sizeof(state), "Map: %s | Gamemode: %s", globals::mapname, globals::gamemode);
		discordPresence.details = globals::inmenu ? "In Menu" : details;
		discordPresence.state = globals::inmenu ? "" : state;
		discordPresence.startTimestamp = 0;
		discordPresence.endTimestamp = time(0) + 5 * 60;
		discordPresence.largeImageKey = "garrysmod";
		discordPresence.instance = 0;

		globals::lua_interface_g->Msg("vltg_ds::discord::update_rpc() -> done\n");

		Discord_UpdatePresence(&discordPresence);
	}
}

class vltg_ds_connectlistener : IGameEventListener2 {
public:
	void FireGameEvent(IGameEvent* event) override {
		if (strncmp(event->GetName(), "server_spawn", 16) == 0) {
			globals::lua_interface_g->Msg("vltg_ds::firegameevent() -> server_spawn: %s\n", event->GetString("hostname"));

			globals::inmenu = false;
			globals::hostname = event->GetString("hostname");
			globals::mapname = event->GetString("mapname");
			
			globals::lua_g->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
			globals::lua_g->GetField(-1, "engine");
			globals::lua_g->GetField(-1, "ActiveGamemode");
			globals::lua_g->Call(0, 1);

			globals::gamemode = globals::lua_g->GetString(-1);

			globals::lua_g->Pop(3);

			discord::update_rpc();

			return;
		}

		if (strncmp(event->GetName(), "client_disconnect", 24) == 0 && event->GetString("message")[0] == '\0') {
			globals::lua_interface_g->Msg("vltg_ds::firegameevent() -> client_disconnect\n");

			globals::inmenu = true;

			discord::update_rpc();

			return;
		}
	}
};

vltg_ds_connectlistener* event_listener;

static SourceSDK::FactoryLoader engine_loader("engine");

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

	static void init() {
		globals::lua_interface_g->Msg("vltg_ds::init() -> done: event_listener: 0x%p\n", event_listener);
		discord::init();
		discord::update_rpc();
	}
}

GMOD_MODULE_OPEN() {
	globals::lua_g = LUA;
	globals::lua_interface_g = (GarrysMod::Lua::CLuaInterface*)LUA;
	globals::event_mgr = (IGameEventManager2*)engine_loader.GetFactory()(INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr);
	if (!globals::event_mgr) {
		globals::lua_g->ThrowError("failed to initialize event_mgr\n");
	}
	event_listener = new vltg_ds_connectlistener();
	globals::event_mgr->AddListener((IGameEventListener2*)event_listener, "server_spawn", false);
	globals::event_mgr->AddListener((IGameEventListener2*)event_listener, "client_disconnect", false);

	module::pre_init();
	module::init();
	return 1;
}

GMOD_MODULE_CLOSE() {
	module::deinit();
	return 0;
}
