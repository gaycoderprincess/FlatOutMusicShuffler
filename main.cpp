#include <windows.h>
#include <ctime>
#include <thread>
#include <random>
#include <toml++/toml.hpp>
#include "nya_commonhooklib.h"
#include "../nya-common-fouc/fo2versioncheck.h"

bool bVanillaSongRotation = false;
bool bAsync = true;

class PlayerHost {
public:
	uint8_t _0[0x1E24C];
	uint32_t nRaceTime; // +1E24C
};
auto& pPlayerHost = *(PlayerHost**)0x68B7C0;

class GameFlow {
public:
	uint8_t _0[0x2828];
	PlayerHost* pHost; // +2828
	uint8_t _282C[0xC4];
	void* pMenuInterface; // +28F0
};
auto& pGameFlow = *(GameFlow**)0x6A7CE0;

auto& nMusicPopupTimeOffset = *(int*)0x6BFFEC;
auto& nCurrentPlaylistSongID = *(int*)0x6BFF68;
auto& pMusicPlaylistStart = *(uintptr_t*)0x6C0130;
auto& pMusicPlaylistEnd = *(uintptr_t*)0x6C012C;

auto GetFMODSound = (void*(*)(int))0x5FF77B;
auto StartSelectedSong = (char(*)())0x411350;

std::mt19937 musicRNG;
int GetRandom(int max) {
	std::uniform_int_distribution<int> uni(0, max - 1);
	return uni(musicRNG);
}

void SelectNewSong() {
	auto numSongs = (pMusicPlaylistStart - pMusicPlaylistEnd) / 84;
	if (numSongs < 2) return;

	auto songId = GetRandom(numSongs);
	while (songId == nCurrentPlaylistSongID) songId = GetRandom(numSongs);
	nCurrentPlaylistSongID = songId;
}

bool bLoadingSong = false;
void OnSongEnd() {
	if (pGameFlow && !pGameFlow->pMenuInterface && pGameFlow->pHost) { // don't play race songs in menus
		if (!bVanillaSongRotation) SelectNewSong();
		StartSelectedSong();
		nMusicPopupTimeOffset = pGameFlow->pHost->nRaceTime;
	}
	else StartSelectedSong();
	bLoadingSong = false;
}

auto sub_5FFE32_orig = (char(__stdcall*)(int))0x5FFE32;
char __stdcall MusicLoop(int soundId) {
	// sound ptr is null after the song ends
	if (!GetFMODSound(soundId) && !bLoadingSong) {
		if (bAsync) {
			bLoadingSong = true;
			std::thread(OnSongEnd).detach();
		}
		else OnSongEnd();
	}

	return sub_5FFE32_orig(soundId);
}

auto sub_5FF338_orig = (void*(__stdcall*)(void*, uint32_t, int, size_t))0x5FF338;
void* __stdcall ResetSongPopupTimer(void* data, uint32_t flags, int a3, size_t size) {
	nMusicPopupTimeOffset = 0;
	return sub_5FF338_orig(data, flags, a3, size);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			DoFlatOutVersionCheck(FO2Version::FO1_1_1);

			auto config = toml::parse_file("FlatOutMusicShuffler_gcp.toml");
			bVanillaSongRotation = config["main"]["sequential"].value_or(false);
			bAsync = config["main"]["async"].value_or(true);

			musicRNG = std::mt19937(time(nullptr));

			NyaHookLib::Patch(0x410CF8 + 1, 0x88000); // remove the loop flag
			sub_5FFE32_orig = (char(__stdcall*)(int))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x410EC6, MusicLoop);
			sub_5FF338_orig = (void*(__stdcall*)(void*, uint32_t, int, size_t))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x410CFE, ResetSongPopupTimer);
		} break;
		default:
			break;
	}
	return TRUE;
}