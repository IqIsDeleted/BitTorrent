#include "progress_bar.h"

#include <stdio.h>
#include <time.h>

#define PROGRESS_BAR_WIDTH 50
#define BYTES_PER_KB 1024.0

#define CLEAR_LINE "\033[K"
#define CARRIAGE_RETURN "\r"
#define SAVE_CURSOR "\033[s"
#define RESTORE_CURSOR "\033[u"
#define MOVE_CURSOR_UP "\033[1A"

typedef struct {
  uint64_t total_bytes;
  uint64_t downloaded_bytes;
  double percentage;
  time_t start_time;
  int is_first_update;
  int is_completed;
} download_state_t;

static void update_percentage(download_state_t* state) {
  if (state->total_bytes == 0) {
    state->percentage = 0.0;
    return;
  }

  state->percentage =
      (double)state->downloaded_bytes * 100.0 / (double)state->total_bytes;
  if (state->percentage > 100.0) {
    state->percentage = 100.0;
  }
}

static int is_download_complete(const download_state_t* state) {
  return state->downloaded_bytes >= state->total_bytes;
}

static void format_file_size(uint64_t bytes, double* size, int* unit_index) {
  *size = (double)bytes;
  *unit_index = 0;

  while (*size >= BYTES_PER_KB && *unit_index < 3) {
    *size /= BYTES_PER_KB;
    (*unit_index)++;
  }
}

static void format_time(double seconds, char* buffer, size_t buffer_size) {
  if (seconds < 60) {
    snprintf(buffer, buffer_size, "%.1fs", seconds);
  } else if (seconds < 3600) {
    int minutes = (int)seconds / 60;
    double secs = seconds - minutes * 60;
    snprintf(buffer, buffer_size, "%dm %.1fs", minutes, secs);
  } else {
    int hours = (int)seconds / 3600;
    int minutes = ((int)seconds % 3600) / 60;
    double secs = seconds - hours * 3600 - minutes * 60;
    snprintf(buffer, buffer_size, "%dh %dm %.1fs", hours, minutes, secs);
  }
}

static void display_progress(const download_state_t* state) {
  printf(SAVE_CURSOR);
  printf(MOVE_CURSOR_UP);
  printf(CLEAR_LINE CARRIAGE_RETURN);

  double downloaded_size, total_size;
  int downloaded_unit, total_unit;

  format_file_size(state->downloaded_bytes, &downloaded_size, &downloaded_unit);
  format_file_size(state->total_bytes, &total_size, &total_unit);

  const char* units[] = {"B", "KB", "MB", "GB"};
  int pos = (int)(state->percentage * PROGRESS_BAR_WIDTH / 100.0);

  printf("[");
  for (int i = 0; i < PROGRESS_BAR_WIDTH; ++i) {
    if (i < pos) {
      printf("▉");
    } else {
      printf(" ");
    }
  }
  printf("] %.1f%%", state->percentage);

  printf(" | %.2f %s / %.2f %s", downloaded_size, units[downloaded_unit],
         total_size, units[total_unit]);

  if (!state->is_first_update) {
    time_t current_time = time(NULL);
    double elapsed_seconds = difftime(current_time, state->start_time);

    if (elapsed_seconds > 0) {
      double speed = (double)state->downloaded_bytes / elapsed_seconds;
      double speed_size;
      int speed_unit;
      format_file_size((uint64_t)speed, &speed_size, &speed_unit);

      char time_buffer[32];
      format_time(elapsed_seconds, time_buffer, sizeof(time_buffer));

      printf(" | %.2f %s/s | %s", speed_size, units[speed_unit], time_buffer);
    }
  }

  printf(RESTORE_CURSOR);
  fflush(stdout);
}

void update_progress_bar(uint64_t total_bytes, uint64_t current_downloaded) {
  static download_state_t state = {0};

  if (state.is_completed) {
    return;
  }

  if (state.total_bytes == 0 && total_bytes > 0) {
    state.total_bytes = total_bytes;
    state.start_time = time(NULL);
    state.is_first_update = 1;

    printf("\n");
    fflush(stdout);
  }

  if (total_bytes == 0) {
    fprintf(stderr, "Error: total_bytes cannot be zero\n");
    return;
  }

  state.downloaded_bytes = current_downloaded;
  update_percentage(&state);

  if (state.is_first_update) {
    state.is_first_update = 0;
  }

  display_progress(&state);

  if (is_download_complete(&state)) {
    state.is_completed = 1;

    time_t end_time = time(NULL);
    double total_seconds = difftime(end_time, state.start_time);

    double total_size;
    int total_unit;
    format_file_size(state.total_bytes, &total_size, &total_unit);

    const char* units[] = {"B", "KB", "MB", "GB"};

    char time_buffer[32];
    format_time(total_seconds, time_buffer, sizeof(time_buffer));

    double avg_speed = (double)state.total_bytes / total_seconds;
    double avg_speed_size;
    int avg_speed_unit;
    format_file_size((uint64_t)avg_speed, &avg_speed_size, &avg_speed_unit);

    printf("\nDownload completed! %.1f%% - %.2f %s downloaded\n",
           state.percentage, total_size, units[total_unit]);
    printf("Time: %s | Average speed: %.2f %s/s\n", time_buffer, avg_speed_size,
           units[avg_speed_unit]);
    fflush(stdout);
  }
}
