#include <3ds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <cstring>
// #include "common/gl2d.h"

#include "ndsheaderbanner.h"
#include "module_params.h"

extern sNDSBannerExt ndsBanner;

// Needed to test if homebrew
char tidBuf[4];

/**
 * Get the title ID.
 * @param ndsFile DS ROM image.
 * @param buf Output buffer for title ID. (Must be at least 4 characters.)
 * @return 0 on success; non-zero on error.
 */
int grabTID(FILE *ndsFile, char *buf)
{
	fseek(ndsFile, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	size_t read = fread(buf, 1, 4, ndsFile);
	return !(read == 4);
}

/**
 * Get SDK version from an NDS file.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 0 on success; non-zero on error.
 */
u32 getSDKVersion(FILE *ndsFile)
{
	sNDSHeaderExt NDSHeader;
	fseek(ndsFile, 0, SEEK_SET);
	fread(&NDSHeader, 1, sizeof(NDSHeader), ndsFile);
	if (NDSHeader.arm7destination >= 0x037F8000 || grabTID(ndsFile, tidBuf) != 0)
		return 0;
	return getModuleParams(&NDSHeader, ndsFile)->sdk_version;
}

/**
 * Check if NDS game has AP.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return true on success; false if no AP.
 */
bool checkRomAP(FILE *ndsFile)
{
	char game_TID[5];
	grabTID(ndsFile, game_TID);
	game_TID[4] = 0;

	// Check for SDK4-5 ROMs that don't have AP measures.
	if ((strcmp(game_TID, "AZLJ") == 0)     // Girls Mode (JAP version of Style Savvy)
	|| (strcmp(game_TID, "YEEJ") == 0)      // Inazuma Eleven (J)
	|| (strncmp(game_TID, "VSO", 3) == 0)   // Sonic Classic Collection
	|| (strncmp(game_TID, "B2D", 3) == 0)   // Doctor Who: Evacuation Earth
	|| (strcmp(game_TID, "BRFP") == 0))    // Rune Factory 3 - A Fantasy Harvest Moon
	{
		return false;
	}
	else
	// Check for ROMs that have AP measures.
	if ((strncmp(game_TID, "B", 1) == 0)
	|| (strncmp(game_TID, "T", 1) == 0)
	|| (strncmp(game_TID, "V", 1) == 0)) {
		return true;
	} else {
		static const char ap_list[][4] = {
			"ABT",	// Bust-A-Move DS
			"YHG",	// Houkago Shounen
			"YWV",	// Taiko no Tatsujin DS: Nanatsu no Shima no Daibouken!
			"AS7",	// Summon Night: Twin Age
			"YFQ",	// Nanashi no Geemu
			"AFX",	// Final Fantasy Crystal Chronicles: Ring of Fates
			"YV5",	// Dragon Quest V: Hand of the Heavenly Bride
			"CFI",	// Final Fantasy Crystal Chronicles: Echoes of Time
			"CCU",	// Tomodachi Life
			"CLJ",	// Mario & Luigi: Bowser's Inside Story
			"YKG",	// Kindgom Hearts: 358/2 Days
			"COL",	// Mario & Sonic at the Olympic Winter Games
			"C24",	// Phantasy Star 0
			"AZL",	// Style Savvy
			"CS3",	// Sonic and Sega All Stars Racing
			"IPK",	// Pokemon HeartGold Version
			"IPG",	// Pokemon SoulSilver Version
			"YBU",	// Blue Dragon: Awakened Shadow
			"YBN",	// 100 Classic Books
			"YDQ",	// Dragon Quest IX: Sentinels of the Starry Skies
			"C3J",	// Professor Layton and the Unwound Future
			"IRA",	// Pokemon Black Version
			"IRB",	// Pokemon White Version
			"CJR",	// Dragon Quest Monsters: Joker 2
			"YEE",	// Inazuma Eleven
			"UZP",	// Learn with Pokemon: Typing Adventure
			"IRE",	// Pokemon Black Version 2
			"IRD",	// Pokemon White Version 2
		};

		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(ap_list)/sizeof(ap_list[0]); i++) {
			if (memcmp(game_TID, ap_list[i], 3) == 0) {
				// Found a match.
				return true;
				break;
			}
		}

	}
	
	return false;
}

// bnriconframeseq[]
static u16 bnriconframeseq[41][64] = {0x0000};

// bnriconisDSi[]
bool isDirectory[40] = {false};
bool bnrSysSettings[41] = {false};
int bnrRomType[41] = {0};
bool bnriconisDSi[41] = {false};
int bnrWirelessIcon[41] = {0}; // 0 = None, 1 = Local, 2 = WiFi
bool isDSiWare[41] = {true};
int isHomebrew[41] = {0}; // 0 = No, 1 = Yes with no DSi-Extended header, 2 = Yes with DSi-Extended header

static u16 bannerDelayNum[41] = {0x0000};
int currentbnriconframeseq[41] = {0};

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence(int iconnum)
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[iconnum][i] = ndsBanner.dsi_seq[i];
	}
	currentbnriconframeseq[iconnum] = 0;
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence(int iconnum)
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[iconnum][i] = 0x0000;
	}
	currentbnriconframeseq[iconnum] = 0;
}
