#include "../SDK/SDK.h"
#include "../Features/Misc/Misc.h"

MAKE_SIGNATURE(CHudTournamentSetup_OnTick, "client.dll", "48 83 EC ? E8 ? ? ? ? 48 85 C0 74 ? 48 8B C8 E8 ? ? ? ? 48 83 C4 ? C3", 0x0);

MAKE_HOOK(CHudTournamentSetup_OnTick, S::CHudTournamentSetup_OnTick(), void,
	void* rcx)
{
	CALL_ORIGINAL(rcx);

	F::Misc.AutoMvmReadyUp();
} 