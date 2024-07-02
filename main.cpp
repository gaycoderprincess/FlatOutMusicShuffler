#include <windows.h>
#include <ctime>
#include <toml++/toml.hpp>
#include "nya_commonhooklib.h"

bool bVanillaSongRotation = false;

struct tRaceStruct {
	uint8_t _0[0x1E24C];
	uint32_t nCurrentTick; // 0x1E24C
};

struct tGameMain {
	uint8_t _0[0x2828];
	tRaceStruct* pRaceStruct; // 0x2828
	uint8_t _282C[0xC4];
	void* pMenuInterface; // 0x28F0
};
auto& pGame = *(tGameMain**)0x6A7CE0;

auto& nMusicPopupTimeOffset = *(int*)0x6BFFEC;
auto& nCurrentPlaylistSongID = *(int*)0x6BFF68;
auto& pMusicPlaylistStart = *(uintptr_t*)0x6C0130;
auto& pMusicPlaylistEnd = *(uintptr_t*)0x6C012C;

auto GetFMODSound = (void*(*)(int))0x5FF77B;
auto StartSelectedSong = (char(*)())0x411350;

void SelectNewSong() {
	auto numSongs = (pMusicPlaylistStart - pMusicPlaylistEnd) / 84;
	if (numSongs < 2) return;

	auto songId = time(nullptr) % numSongs;
	while (songId == nCurrentPlaylistSongID) songId = time(nullptr) % numSongs;
	nCurrentPlaylistSongID = songId;
}

auto sub_5FFE32_orig = (char(__stdcall*)(int))0x5FFE32;
char __stdcall MusicLoop(int soundId) {
	// sound ptr is null after the song ends
	if (!GetFMODSound(soundId)) {
		if (pGame && !pGame->pMenuInterface && pGame->pRaceStruct) { // don't play race songs in menus
			if (!bVanillaSongRotation) SelectNewSong();
			nMusicPopupTimeOffset = pGame->pRaceStruct->nCurrentTick;
		}
		StartSelectedSong();
	}

	return sub_5FFE32_orig(soundId);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x1E829F) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.1 (.exe size of 2822144 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			auto config = toml::parse_file("FlatOutMusicShuffler_gcp.toml");
			bVanillaSongRotation = config["main"]["sequential"].value_or(false);

			NyaHookLib::Patch(0x410CF8 + 1, 0x88000); // remove the loop flag
			sub_5FFE32_orig = (char(__stdcall*)(int))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x410EC6, MusicLoop);
		} break;
		default:
			break;
	}
	return TRUE;
}