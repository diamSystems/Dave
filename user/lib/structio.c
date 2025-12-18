#include "types.h"
#include "user.h"
#include "structio.h"

static void
write_str(int fd, const char *s)
{
  if(s)
    write(fd, s, strlen(s));
}

static void
write_escaped(int fd, const char *s)
{
  static const char *hex = "0123456789abcdef";
  char buf[6];

  for(; *s; s++){
    char c = *s;
    switch(c){
    case '"':
      write(fd, "\\\"", 2);
      break;
    case '\\':
      write(fd, "\\\\", 2);
      break;
    case '\n':
      write(fd, "\\n", 2);
      break;
    case '\r':
      write(fd, "\\r", 2);
      break;
    case '\t':
      write(fd, "\\t", 2);
      break;
    default:
      if((uchar)c < 0x20){
        buf[0] = '\\';
        buf[1] = 'u';
        buf[2] = '0';
        buf[3] = '0';
        buf[4] = hex[(c >> 4) & 0xf];
        buf[5] = hex[c & 0xf];
        write(fd, buf, sizeof(buf));
      } else {
        write(fd, &c, 1);
      }
      break;
    }
  }
}

static void
begin_field(struct struct_writer *w)
{
  if(w->need_comma)
    write(w->fd, ",", 1);
  else
    w->need_comma = 1;
}

void
struct_begin(struct struct_writer *w, int fd)
{
  w->fd = fd;
  w->need_comma = 0;
  write(fd, "{", 1);
}

void
struct_field_str(struct struct_writer *w, const char *key, const char *value)
{
  begin_field(w);
  write(w->fd, "\"", 1);
  write_str(w->fd, key ? key : "");
  write(w->fd, "\":\"", 3);
  write_escaped(w->fd, value ? value : "");
  write(w->fd, "\"", 1);
}

void
struct_field_int(struct struct_writer *w, const char *key, int value)
{
  begin_field(w);
  write(w->fd, "\"", 1);
  write_str(w->fd, key ? key : "");
  write(w->fd, "\":", 2);
  printf(w->fd, "%d", value);
}

void
struct_end(struct struct_writer *w)
{
  write(w->fd, "}\n", 2);
}
