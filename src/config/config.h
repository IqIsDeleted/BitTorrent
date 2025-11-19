#ifndef CONFIG_CONFIG_H_
#define CONFIG_CONFIG_H_

#include <linux/limits.h>

typedef enum Mode { SEED = 0, LEECH = 1 } Mode;

typedef struct Config {
  char data_path[PATH_MAX];
  char torrent_path[PATH_MAX];
  Mode mode;
} Config;

/**
 * @brief Print current configuration
 * @param cfg Configuration to print
 */
void print_client_config(const Config* cfg);

/**
 * @brief Initialize config from command line arguments
 * @param cfg Config structure to initialize
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 on success, -1 on error, 1 if help shown
 */
int init_config(Config* cfg, int argc, char** argv);

#endif  // CONFIG_CONFIG_H
