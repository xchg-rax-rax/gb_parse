#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "gb_rom_header.h"

// function that the bootstrap rom uses to compute the header checksum
// weird pseudo code so likely wrong
uint8_t compute_header_checksum(gb_rom_header* header) {
    uint8_t x = 0;
    for (int i = 0x34; i < 0x4C; i++) {
        x -= *(((uint8_t*)header) + i) - 1;
    }
    return x;
}

// kay so the 2 byte new licensee code are fucking batshit
// this fixes them and makes them line up with all the docs
// no idea who to blame for this particular fuckery 
uint8_t trasnlate_new_licensee_code(uint16_t raw_new_licensee_code) {
    return ((raw_new_licensee_code & 0x0F) << 4) + ((raw_new_licensee_code & 0x0F00) >> 8);
}

int main(int argc, char**argv) {
    if (argc != 2) {
        printf("Usage: %s <file path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // open file
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        printf("[!] Could not open file: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // memory map file
    gb_rom_header* header =  (gb_rom_header*) (mmap(0,0x50,PROT_READ, MAP_PRIVATE, fd, 0) + GB_HEADER_BASE_OFFSET);
    if (header == MAP_FAILED) {
        int error = errno;
        printf("[!] Could not map file into memory: %s\n", strerror(error));
        return EXIT_FAILURE;
    }    
    // print vanity banner OwO
    printf("\033[0;31m  ▄████  ▄▄▄▄       ██▓███   ▄▄▄       ██▀███    ██████ ▓█████ \n ██▒ ▀█▒▓█████▄    ▓██░  ██▒▒████▄    ▓██ ▒ ██▒▒██    ▒ ▓█   ▀ \n▒██░▄▄▄░▒██▒ ▄██   ▓██░ ██▓▒▒██  ▀█▄  ▓██ ░▄█ ▒░ ▓██▄   ▒███   \n░▓█  ██▓▒██░█▀     ▒██▄█▓▒ ▒░██▄▄▄▄██ ▒██▀▀█▄    ▒   ██▒▒▓█  ▄ \n░▒▓███▀▒░▓█  ▀█▓   ▒██▒ ░  ░ ▓█   ▓██▒░██▓ ▒██▒▒██████▒▒░▒████▒\n ░▒   ▒ ░▒▓███▀▒   ▒▓▒░ ░  ░ ▒▒   ▓▒█░░ ▒▓ ░▒▓░▒ ▒▓▒ ▒ ░░░ ▒░ ░\n  ░   ░ ▒░▒   ░    ░▒ ░       ▒   ▒▒ ░  ░▒ ░ ▒░░ ░▒  ░ ░ ░ ░  ░\n░ ░   ░  ░    ░    ░░         ░   ▒     ░░   ░ ░  ░  ░     ░   \n      ░  ░                        ░  ░   ░           ░     ░  ░\n              ░                                                \n\033[0;36m");  
    
    printf("Hell of simulation, which is no longer one of torture, but of subtle, maleficent, \nelusive twisting of meaning... ― Jean Baudrillard, Simulacra and Simulation\n\n");

    // check if new or old cartridge format
    uint8_t new_catridge = (USE_NEW_LICENSEE_CODE == header->old_licensee_code);

    char game_title[17];
    if (new_catridge) {
        strncpy(game_title, header->game_title, 11);
        game_title[11] = '\0';
    }
    else {
        strncpy(game_title, header->game_title, 16);
        game_title[16] = '\0';
    }

    printf("Game Title:\t\t%s\n", game_title);

    printf("Entry Point:\t\t0x%04x\n", header->entry_point_addr);

    // display nintendo logo bytes and simultaneous verify them
    printf("Nintendo Logo:\t\t");
    uint8_t correct_bytes = 0;
    for (uint8_t i = 0; i < 0x30; i++) {
        printf("%02x ",header->nintendo_logo[i]);
        if (header->nintendo_logo[i] == NINTENDO_LOGO[i]) {
            correct_bytes++;
        }
        if (i % 0x10 == 0xF) {
            printf("\n              \t\t");
        }
    }

    printf("\nLogo Correct:\t\t%s\n",  correct_bytes == 0x30 ? "Yes" : "No" );
    if (new_catridge) {
        uint32_t manufacturer_code = *((uint32_t*)(&header->game_title)+11);
        printf("Manufacturer Code:\t%08x\n", manufacturer_code);
        uint8_t cgb_flag = *((uint8_t*)(&header->game_title)+15);
        printf("CBG Code:\t\t%02x ", cgb_flag);
        switch(cgb_flag) {
            case CGB_OFF:
                printf("(CGB off)\n");
                break;
            case CGB_SUPPORTED:
                printf("(CGB supported)\n");
                break;
            case CGB_ONLY:
                printf("(CGB only)\n");
                break;
            default:
                printf("(Unknown)\n");
                break;
        }
    }
    
    uint8_t licensee_code = new_catridge ? trasnlate_new_licensee_code(header->new_licensee_code) : header->old_licensee_code;
    printf("Licensee:\t\t");
    if (new_catridge) {
        switch(licensee_code) {
            case LICENSEE_NONE:
                printf("None\n");
                break;
            case LICENSEE_NINTENDO_RND:
                printf("Nintendo R&D\n");
                break;
            case LICENSEE_CAPCOM:
                printf("Capcom\n");
                break;
            case LICENSEE_EA_1:
            case LICENSEE_EA_2:
                printf("Electronic Arts\n");
                break;
            case LICENSEE_HUDSON_SOFT:
            case LICENSEE_HUDSON:
                printf("Hudson Soft\n");
                break;
            case LICENSEE_B_AI:
                printf("B-ai\n");
                break;
            case LICENSEE_KSS:
                printf("KSS\n");
                break;
            case LICENSEE_POW:
                printf("POW\n");
                break;
            case LICENSEE_PCM_COMPLETE:
                printf("PCM Complete\n");
                break;
            case LICENSEE_SAN_X:
                printf("San X\n");
                break;
            case LICENSEE_KEMCO_JP:
                printf("Kemco\n");
                break;
            case LICENSEE_SETA:
                printf("SETA Corporation\n");
                break;
            case LICENSEE_VIACOM:
                printf("Viacom\n"); 
                break;
            case LICENSEE_NINTENDO:
                printf("Nintendo\n");
                break;
            case LICENSEE_BANDAI:
                printf("Bandai\n");
                break;
            case LICENSEE_OCEAN_ACCLAIM_1:
            case LICENSEE_OCEAN_ACCLAIM_2:
                printf("Ocean Software & Acclaim Games\n");
                break;
            case LICENSEE_KONAMI_1:
            case LICENSEE_KONAMI_2:
                printf("Konmai\n");
                break;
            case LICENSEE_HECTOR:
                printf("Hect (Hector Playing Interface)\n");
                break;
            case LICENSEE_TAITO:
                printf("Taito\n");
                break;
            case LICENSEE_BANPRESTO:
                printf("Banpresto\n");
                break;
            case LICENSEE_UBISOFT:
                printf("Ubisoft\n");
                break;
            case LICENSEE_ATLUS:
                printf("Atlus\n");
                break;
            case LICENSEE_MALIBU:
                printf("Malibu Games\n");
                break;
            case LICENSEE_ANGEL:
                printf("Angel (Bandai)\n");
                break;
            case LICENSEE_BULLET_PROOF:
                printf("Bullet Proof Software\n");
                break;
            case LICENSEE_IREM:
                printf("Irem Software Engineering\n");
                break;
            case LICENSEE_ABSOLUTE:
                printf("Absolute Entertainment\n");
                break;
            case LICENSEE_ACCLAIM:
                printf("Acclaim Games\n");
                break;
            case LICENSEE_ACTIVISION:
                printf("Activation\n");
                break;
            case LICENSEE_AMERICAN_SAMMY:
                printf("Sammy Corporation\n");
                break;
            case LICENSEE_HITECH_ENTERTAINMENT:
                printf("Hi Tech Entertainment\n");
                break;
            case LICENSEE_LJN:
                printf("LJN Toys\n");
                break;
            case LICENSEE_MATCHBOX:
                printf("Matchbox\n");
                break;
            case LICENSEE_MATTEL:
                printf("Mattel\n");
                break;
            case LICENSEE_MILTON_BRADLEY:
                printf("Milton Bradley Company\n");
                break;
            case LICENSEE_TITUS:
                printf("Titus Interactive\n");
                break;
            case LICENSEE_VIRGIN:
                printf("Virgin\n");
                break;
            case LICENSEE_LUCASARTS:
                printf("Lucasarts\n");
                break;
            case LICENSEE_OCEAN:
                printf("Ocean Software\n");
                break;
            case LICENSEE_INFOGRAMES:
                printf("Infogrames\n");
                break;
            case LICENSEE_INTERPLAY:
                printf("Interplay\n");
                break;
            case LICENSEE_BRODERBUND:
                printf("Broderbund\n");
                break;
            case LICENSEE_SCULPTURED:
                printf("Sculptured Software\n");
                break;
            case LICENSEE_SCI:
                printf("SCi Games\n");
                break;
            case LICENSEE_THQ:
                printf("THQ\n");
                break;
            case LICENSEE_ACCOLADE:
                printf("Accolade\n");
                break;
            case LICENSEE_MISAWA:
                printf("Misawa Entertainment\n");
                break;
            case LICENSEE_LOZC:
                printf("LOZC/G Amusements\n");
                break;
            case LICENSEE_TOKUMA_SHOTEN_INTERMEDIA:
                printf("Tokuma Shoten Intermedia\n");
                break;
            case LICENSEE_TUSKUDA_ORIGINAL:
                printf("Tsukuda Original\n");
                break;
            case LICENSEE_CHUNSOFT:
                printf("Chunsoft\n");
                break;
            case LICENSEE_VIDEO_SYSTEM:
                printf("Video System\n");
                break;
            case LICENSEE_VARIE:
                printf("Varie\n");
                break;
            case LICENSEE_YONEZAWA_SPAL:
                printf("Yonezawa / S’Pal\n");
                break;
            case LICENSEE_KANEKO:
                printf("Kaneko\n");
                break;
            case LICENSEE_PACK_IN_SOFT:
                printf("Pack-In-Soft\n");
                break;
            case LICENSEE_KONAMI_YU_GI_OH:
                printf("Konmai (Yogi Oh)\n");
                break;
            default:
                printf("Unknown (0x%02x)\n", licensee_code);
        }
    }
    else {
        switch(licensee_code){
            case OLD_LICENSEE_NONE:
                printf("None\n");
                break;
            case OLD_LICENSEE_NINTENDO_1:
            case OLD_LICENSEE_NINTENDO_2:
                printf("Nintendo\n");
                break;
            case OLD_LICENSEE_CAPCOM_1:
            case OLD_LICENSEE_CAPCOM_2:
                printf("Capcom\n");
                break;
            case OLD_LICENSEE_HOTB:
                printf("Hotb\n");
                break;
            case OLD_LICENSEE_JALECO_1:
            case OLD_LICENSEE_JALECO_2:
                printf("Jaleco\n");
                break;
            case OLD_LICENSEE_COCONUTS:
                printf("Coconuts\n");
                break;
            case OLD_LICENSEE_ELITE_SYSTEMS_1:
            case OLD_LICENSEE_ELITE_SYSTEMS_2:
                printf("Elite Systems\n");
                break;
            case OLD_LICENSEE_EA_1:
            case OLD_LICENSEE_EA_2:
                printf("Electronic Arts\n");
                break;
            case OLD_LICENSEE_HUDSONSOFT:
                printf("Hudsonsoft\n");
                break;
            case OLD_LICENSEE_ITC_ENTERTAINMENT:
                printf("Itc Entertainment\n");
                break;
            case OLD_LICENSEE_YANOMAN:
                printf("Yanoman\n");
                break;
            case OLD_LICENSEE_CLARY:
                printf("Clary\n");
                break;
            case OLD_LICENSEE_VIRGIN_1:
            case OLD_LICENSEE_VIRGIN_2:
                printf("Virgin\n");
                break;
            case OLD_LICENSEE_PCM_COMPLETE:
                printf("Pcm Complete\n");
                break;
            case OLD_LICENSEE_SANX:
                printf("Sanx\n");
                break;
            case OLD_LICENSEE_KOTOBUKI_SYSTEMS:
                printf("Kotobuki Systems\n");
                break;
            case OLD_LICENSEE_SETA:
                printf("Seta\n");
                break;
            case OLD_LICENSEE_INFOGRAMES_1:
            case OLD_LICENSEE_INFOGRAMES_2:
                printf("Infogrames\n");
                break;
            case OLD_LICENSEE_BANDAI_1:
            case OLD_LICENSEE_BANDAI_2:
            case OLD_LICENSEE_BANDAI_3:
                printf("Bandai\n");
                break;
            case OLD_LICENSEE_KONAMI_1:
            case OLD_LICENSEE_KONAMI_2:
                printf("Konami\n");
                break;
            case OLD_LICENSEE_HECTOR:
                printf("Hector\n");
                break;
            case OLD_LICENSEE_GREMLIN:
                printf("Gremlin\n");
                break;
            case OLD_LICENSEE_UBI_SOFT:
                printf("Ubi Soft\n");
                break;
            case OLD_LICENSEE_ATLUS_1:
            case OLD_LICENSEE_ATLUS_2:
                printf("Atlus\n");
                break;
            case OLD_LICENSEE_MALIBU_1:
            case OLD_LICENSEE_MALIBU_2:
                printf("Malibu\n");
                break;
            case OLD_LICENSEE_ANGEL_1:
            case OLD_LICENSEE_ANGEL_2:
                printf("Angel\n");
                break;
            case OLD_LICENSEE_SPECTRUM_HOLOBY:
                printf("Spectrum Holoby\n");
                break;
            case OLD_LICENSEE_IREM:
                printf("Irem\n");
                break;
            case OLD_LICENSEE_US_GOLD:
                printf("Us Gold\n");
                break;
            case OLD_LICENSEE_ABSOLUTE:
                printf("Absolute\n");
                break;
            case OLD_LICENSEE_ACCLAIM_1:
            case OLD_LICENSEE_ACCLAIM_2:
                printf("Acclaim\n");
                break;
            case OLD_LICENSEE_ACTIVISION:
                printf("Activision\n");
                break;
            case OLD_LICENSEE_AMERICAN_SAMMY:
                printf("American Sammy\n");
                break;
            case OLD_LICENSEE_GAMETEK:
                printf("Gametek\n");
                break;
            case OLD_LICENSEE_PARK_PLACE:
                printf("Park Place\n");
                break;
            case OLD_LICENSEE_MATCHBOX:
                printf("Matchbox\n");
                break;
            case OLD_LICENSEE_MILTON_BRADLEY:
                printf("Milton Bradley\n");
                break;
            case OLD_LICENSEE_MINDSCAPE:
                printf("Mindscape\n");
                break;
            case OLD_LICENSEE_ROMSTAR:
                printf("Romstar\n");
                break;
            case OLD_LICENSEE_NAXAT_SOFT_1:
            case OLD_LICENSEE_NAXAT_SOFT_2:
                printf("Naxat Soft\n");
                break;
            case OLD_LICENSEE_TRADEWEST:
                printf("Tradewest\n");
                break;
            case OLD_LICENSEE_TITUS:
                printf("Titus\n");
                break;
            case OLD_LICENSEE_OCEAN:
                printf("Ocean\n");
                break;
            case OLD_LICENSEE_ELECTRO_BRAIN:
                printf("Electro Brain\n");
                break;
            case OLD_LICENSEE_INTERPLAY:
                printf("Interplay\n");
                break;
            case OLD_LICENSEE_BRODERBUND_1:
            case OLD_LICENSEE_BRODERBUND_2:
                printf("Broderbund\n");
                break;
            case OLD_LICENSEE_SCULPTERED_SOFT:
                printf("Sculptered Soft\n");
                break;
            case OLD_LICENSEE_THE_SALES_CURVE:
                printf("The Sales Curve\n");
                break;
            case OLD_LICENSEE_THQ:
                printf("Thq\n");
                break;
            case OLD_LICENSEE_ACCOLADE:
                printf("Accolade\n");
                break;
            case OLD_LICENSEE_TRIFFIX_ENTERTAINMENT:
                printf("Triffix Entertainment\n");
                break;
            case OLD_LICENSEE_MICROPROSE:
                printf("Microprose\n");
                break;
            case OLD_LICENSEE_MISAWA_ENTERTAINMENT:
                printf("Misawa Entertainment\n");
                break;
            case OLD_LICENSEE_LOZC:
                printf("Lozc\n");
                break;
            case OLD_LICENSEE_BULLETPROOF_SOFTWARE:
                printf("Bulletproof Software\n");
                break;
            case OLD_LICENSEE_VIC_TOKAI:
                printf("Vic Tokai\n");
                break;
            case OLD_LICENSEE_APE:
                printf("Ape\n");
                break;
            case OLD_LICENSEE_I_MAX:
                printf("I'Max\n");
                break;
            case OLD_LICENSEE_CHUN_SOFT:
                printf("Chun Soft\n");
                break;
            case OLD_LICENSEE_VIDEO_SYSTEM:
                printf("Video System\n");
                break;
            case OLD_LICENSEE_TSUBURAVA:
                printf("Tsuburava\n");
                break;
            case OLD_LICENSEE_YONEZAWA_S_PAL:
                printf("Yonezawa S'Pal\n");
                break;
            case OLD_LICENSEE_KANEKO:
                printf("Kaneko\n");
                break;
            case OLD_LICENSEE_ARC:
                printf("Arc\n");
                break;
            case OLD_LICENSEE_NIHON_BUSSAN:
                printf("Nihon Bussan\n");
                break;
            case OLD_LICENSEE_TECMO:
                printf("Tecmo\n");
                break;
            case OLD_LICENSEE_IMAGINEER:
                printf("Imagineer\n");
                break;
            case OLD_LICENSEE_HORI_ELECTRIC:
                printf("Hori Electric\n");
                break;
            case OLD_LICENSEE_KAWADA:
                printf("Kawada\n");
                break;
            case OLD_LICENSEE_TECHNOS_JAPAN:
                printf("Technos Japan\n");
                break;
            case OLD_LICENSEE_TOEI_ANIMATION:
                printf("Toei Animation\n");
                break;
            case OLD_LICENSEE_TOHO:
                printf("Toho\n");
                break;
            case OLD_LICENSEE_NAMCO:
                printf("Namco\n");
                break;
            case OLD_LICENSEE_ASCII_OR_NEXOFT:
                printf("Ascii Or Nexoft\n");
                break;
            case OLD_LICENSEE_ENIX:
                printf("Enix\n");
                break;
            case OLD_LICENSEE_HAL:
                printf("Hal\n");
                break;
            case OLD_LICENSEE_SNK:
                printf("Snk\n");
                break;
            case OLD_LICENSEE_PONY_CANYON_1:
            case OLD_LICENSEE_PONY_CANYON_2:
                printf("Pony Canyon\n");
                break;
            case OLD_LICENSEE_CULTURE_BRAIN:
                printf("Culture Brain\n");
                break;
            case OLD_LICENSEE_SUNSOFT:
                printf("Sunsoft\n");
                break;
            case OLD_LICENSEE_SONY_IMAGESOFT:
                printf("Sony Imagesoft\n");
                break;
            case OLD_LICENSEE_SAMMY:
                printf("Sammy\n");
                break;
            case OLD_LICENSEE_TAITO_1:
            case OLD_LICENSEE_TAITO_2:
                printf("Taito\n");
                break;
            case OLD_LICENSEE_KEMCO_1:
            case OLD_LICENSEE_KEMCO_2:
                printf("Kemco\n");
                break;
            case OLD_LICENSEE_SQUARESOFT:
                printf("Squaresoft\n");
                break;
            case OLD_LICENSEE_TOKUMA_SHOTEN_INTERMEDIA_1:
            case OLD_LICENSEE_TOKUMA_SHOTEN_INTERMEDIA_2:
                printf("Tokuma Shoten Intermedia\n");
                break;
            case OLD_LICENSEE_DATA_EAST:
                printf("Data East\n");
                break;
            case OLD_LICENSEE_TONKIN_HOUSE:
                printf("Tonkin House\n");
                break;
            case OLD_LICENSEE_KOEI:
                printf("Koei\n");
                break;
            case OLD_LICENSEE_UFL:
                printf("Ufl\n");
                break;
            case OLD_LICENSEE_ULTRA:
                printf("Ultra\n");
                break;
            case OLD_LICENSEE_VAP:
                printf("Vap\n");
                break;
            case OLD_LICENSEE_USE:
                printf("Use\n");
                break;
            case OLD_LICENSEE_MELDAC:
                printf("Meldac\n");
                break;
            case OLD_LICENSEE_SOFEL:
                printf("Sofel\n");
                break;
            case OLD_LICENSEE_QUEST:
                printf("Quest\n");
                break;
            case OLD_LICENSEE_SIGMA_ENTERPRISES:
                printf("Sigma Enterprises\n");
                break;
            case OLD_LICENSEE_ASK_KODANSHA:
                printf("Ask Kodansha\n");
                break;
            case OLD_LICENSEE_COPYA_SYSTEMS:
                printf("Copya Systems\n");
                break;
            case OLD_LICENSEE_BANPRESTO_1:
            case OLD_LICENSEE_BANPRESTO_2:
            case OLD_LICENSEE_BANPRESTO_3:
                printf("Banpresto\n");
                break;
            case OLD_LICENSEE_TOMY:
                printf("Tomy\n");
                break;
            case OLD_LICENSEE_LJN_1:
            case OLD_LICENSEE_LJN_2:
                printf("Ljn\n");
                break;
            case OLD_LICENSEE_NCS:
                printf("Ncs\n");
                break;
            case OLD_LICENSEE_HUMAN:
                printf("Human\n");
                break;
            case OLD_LICENSEE_ALTRON:
                printf("Altron\n");
                break;
            case OLD_LICENSEE_TOWACHIKI:
                printf("Towachiki\n");
                break;
            case OLD_LICENSEE_UUTAKA:
                printf("Uutaka\n");
                break;
            case OLD_LICENSEE_VARIE_1:
            case OLD_LICENSEE_VARIE_2:
                printf("Varie\n");
                break;
            case OLD_LICENSEE_EPOCH:
                printf("Epoch\n");
                break;
            case OLD_LICENSEE_ATHENA:
                printf("Athena\n");
                break;
            case OLD_LICENSEE_ASMIK:
                printf("Asmik\n");
                break;
            case OLD_LICENSEE_NATSUME:
                printf("Natsume\n");
                break;
            case OLD_LICENSEE_KING_RECORDS:
                printf("King Records\n");
                break;
            case OLD_LICENSEE_EPIC_SONY_RECORDS:
                printf("Epic Sony Records\n");
                break;
            case OLD_LICENSEE_IGS:
                printf("Igs\n");
                break;
            case OLD_LICENSEE_A_WAVE:
                printf("A Wave\n");
                break;
            case OLD_LICENSEE_EXTREME_ENTERTAINMENT:
                printf("Extreme Entertainment\n");
                break;
            default:
                printf("Unknown\n");
        }
    }

    printf("RAM Size:\t\t");
    switch(header->ram_size_code) {
        case RAM_SIZE_NONE:
            printf("0KB\n");
            break;
        case RAM_SIZE_8KB:
            printf("8KB\n");
            break;
        case RAM_SIZE_32KB:
            printf("32KB\n");
            break;
        case RAM_SIZE_64KB:
            printf("64KB\n");
            break;
        case RAM_SIZE_128KB:
            printf("128KB\n");
            break;
        default:
            printf("Unknown\n");
    }

    printf("Region:\t\t\t");
    switch(header->destination_code) {
        case DESTINATION_JP:
            printf("Japan\n");
            break;
        case DESTINATION_NON_JP:
            printf("Non-Japan\n");
            break;
        default:
            printf("Unknown\n");
    }

    printf("Mask ROM Version:\t0x%x\n", header->mask_rom_version_number);
    printf("Header Checksum:\t0x%x\n", header->header_checksum);
    printf("Global Checksum:\t0x%x\n", header->global_checksum);

    
    return EXIT_SUCCESS;
}
