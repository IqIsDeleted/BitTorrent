#include "config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define HELP_MSG "Try '%s --help' for more information.\n"
#define INVALID_MODE_MSG "Error: Invalid mode '%s'. Use 'seed' or 'leech'\n"
#define INVALID_ARGS_MSG "Error: Invalid arguments\n"
#define TORRENT_REQUIRED_MSG "Error: Torrent file is required (-t/--torrent)\n"
#define DATA_REQUIRED_MSG "Error: Data path is required (-d/--data)\n"

static void print_help(const char* program_name) {
  printf(
      "Usage: %s --mode <mode> --torrent <file> --data <path>\n\n"
      "BitTorrent client that can work in seed or leech mode.\n"
      "OPTIONS:\n"
      "  -m, --mode <MODE>        Set operation mode\n"
      "                           MODE can be:\n"
      "                             seed  - Share files with other peers "
      "(default)\n"
      "                             leech - Download files from peers\n\n"
      "  -t, --torrent <FILE>     Path to .torrent file (required)\n"
      "                           Specifies which torrent to process\n\n"
      "  -d, --data <PATH>        Path to data directory (required)\n"
      "                           For seed mode: directory containing files "
      "to share\n"
      "                           For leech mode: directory to save "
      "downloaded files\n\n"
      "  -h, --help               Show this help message and exit\n\n",
      program_name);
}

void print_client_config(const Config* cfg) {
  printf(
      "-- BitTorrent Client Configuration --\n"
      "  Mode:            %s\n"
      "  Torrent file:    %s\n"
      "  Data path:       %s\n"
      "----------------------------------\n",
      cfg->mode == SEED ? "seed" : "leech", cfg->torrent_path, cfg->data_path);
}

int init_config(Config* cfg, int argc, char** argv) {
  if (!cfg) {
    return -1;
  }

  static struct option long_options[] = {{"mode", required_argument, 0, 'm'},
                                         {"torrent", required_argument, 0, 't'},
                                         {"data", required_argument, 0, 'd'},
                                         {"help", no_argument, 0, 'h'},
                                         {0, 0, 0, 0}};

  int opt;

  while ((opt = getopt_long(argc, argv, "m:t:d:h", long_options, NULL)) != -1) {
    switch (opt) {
      case 'm':
        if (strcmp(optarg, "seed") == 0) {
          cfg->mode = SEED;
        } else if (strcmp(optarg, "leech") == 0) {
          cfg->mode = LEECH;
        } else {
          fprintf(stderr, INVALID_MODE_MSG HELP_MSG, optarg, argv[0]);
          return -1;
        }
        break;
      case 't':
        strncpy(cfg->torrent_path, optarg, PATH_MAX - 1);
        break;
      case 'd':
        strncpy(cfg->data_path, optarg, PATH_MAX - 1);
        break;
      case 'h':
        print_help(argv[0]);
        return 1;
      default:
        fprintf(stderr, INVALID_ARGS_MSG HELP_MSG, argv[0]);
        return -1;
    }
  }

  if (strlen(cfg->torrent_path) == 0) {
    fprintf(stderr, TORRENT_REQUIRED_MSG HELP_MSG, argv[0]);
    return -1;
  }

  if (strlen(cfg->data_path) == 0) {
    fprintf(stderr, DATA_REQUIRED_MSG HELP_MSG, argv[0]);
    return -1;
  }

  return 0;
}
