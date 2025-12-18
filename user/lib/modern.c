#include "types.h"
#include "user.h"
#include "structio.h"
#include "modern.h"

static void
modern_plain_version(const char *cmd)
{
  printf(1, "%s (c) %s %d\n", cmd, MODERN_VENDOR, MODERN_YEAR);
}

int
modern_consume_flags(const char *cmd, int argc, char *argv[], int supports_json, int *json_mode)
{
  int i;
  if(json_mode)
    *json_mode = 0;
  for(i = 1; i < argc; i++){
    char *arg = argv[i];
    if(strcmp(arg, "--version") == 0){
      modern_print_version(cmd, json_mode ? *json_mode : 0);
      exit();
    }
    if(supports_json && (strcmp(arg, "-J") == 0 || strcmp(arg, "--json") == 0)){
      if(json_mode)
        *json_mode = 1;
      continue;
    }
    break;
  }
  return i;
}

void
modern_print_version(const char *cmd, int json_mode)
{
  if(!json_mode){
    modern_plain_version(cmd);
    return;
  }
  struct struct_writer w;
  struct_begin(&w, 1);
  struct_field_str(&w, "command", cmd);
  struct_field_str(&w, "vendor", MODERN_VENDOR);
  struct_field_int(&w, "year", MODERN_YEAR);
  struct_end(&w);
}

modern_emit_status(const char *cmd, const char *event, const char *target, int json_mode, const char *status)
{
  if(!json_mode){
    printf(1, "%s: %s %s %s\n", cmd, event ? event : "event", target ? target : "", status ? status : "done");
    return;
  }
  struct struct_writer w;
  struct_begin(&w, 1);
  struct_field_str(&w, "command", cmd);
  struct_field_str(&w, "event", event ? event : "event");
  if(target)
    struct_field_str(&w, "target", target);
  if(status)
    struct_field_str(&w, "status", status);
  struct_end(&w);
}
