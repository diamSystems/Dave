#ifndef MODERN_H
#define MODERN_H

#define MODERN_VENDOR "diam Systems Ltd"
#define MODERN_YEAR 2026

int modern_consume_flags(const char *cmd, int argc, char *argv[], int supports_json, int *json_mode);
void modern_print_version(const char *cmd, int json_mode);
void modern_emit_status(const char *cmd, const char *event, const char *target, int json_mode, const char *status);

#endif // MODERN_H
