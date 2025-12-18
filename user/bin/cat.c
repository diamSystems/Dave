#include "types.h"
#include "user.h"
#include "structio.h"
#include "modern.h"

static char buf[512];
static char linebuf[1024];
static int json_mode = 0;

static void
emit_line(const char *source, const char *line)
{
  struct struct_writer w;
  struct_begin(&w, 1);
  if(source)
    struct_field_str(&w, "source", source);
  struct_field_str(&w, "data", line ? line : "");
  struct_end(&w);
}

static void
cat_json(int fd, const char *source)
{
  int n, m = 0;

  while((n = read(fd, buf, sizeof(buf))) > 0){
    for(int i = 0; i < n; i++){
      char c = buf[i];
      if(c == '\n'){
        linebuf[m] = '\0';
        emit_line(source, linebuf);
        m = 0;
      } else if(m < (int)sizeof(linebuf) - 1){
        linebuf[m++] = c;
      } else {
        linebuf[m] = '\0';
        emit_line(source, linebuf);
        m = 0;
      }
    }
  }
  if(m > 0){
    linebuf[m] = '\0';
    emit_line(source, linebuf);
  }
  if(n < 0){
    printf(1, "cat: read error\n");
    exit();
  }
}

static void
cat_plain(int fd)
{
  int n;

  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      printf(1, "cat: write error\n");
      exit();
    }
  }
  if(n < 0){
    printf(1, "cat: read error\n");
    exit();
  }
}

static void
cat_stream(int fd, const char *source)
{
  if(json_mode)
    cat_json(fd, source);
  else
    cat_plain(fd);
}

int
main(int argc, char *argv[])
{
  int fd;
  int idx = modern_consume_flags("cat", argc, argv, 1, &json_mode);

  if(idx >= argc){
    cat_stream(0, "stdin");
    exit();
  }

  for(int i = idx; i < argc; i++){
    if((fd = open(argv[i], 0)) < 0){
      printf(1, "cat: cannot open %s\n", argv[i]);
      exit();
    }
    cat_stream(fd, argv[i]);
    close(fd);
  }
  exit();
}
