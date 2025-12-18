#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "structio.h"
#include "modern.h"

static int json_mode = 0;

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

static void
basename(char *dst, int dstsz, const char *path)
{
  const char *start = path + strlen(path);
  const char *p = start;
  while(p > path && *(p-1) == '/')
    p--;
  start = p;
  while(start > path && *(start-1) != '/')
    start--;
  int len = p - start;
  if(len >= dstsz)
    len = dstsz - 1;
  memmove(dst, start, len);
  dst[len] = '\0';
  if(len == 0 && dstsz > 1)
    strcpy(dst, ".");
}

static const char*
typestr(short type)
{
  switch(type){
  case T_DIR: return "dir";
  case T_FILE: return "file";
  case T_DEV: return "device";
  default: return "unknown";
  }
}

static void
emit_entry(const char *path, const char *name, struct stat *st)
{
  if(!json_mode){
    printf(1, "%s %d %d %d\n", fmtname((char*)path), st->type, st->ino, st->size);
    return;
  }

  struct struct_writer w;
  struct_begin(&w, 1);
  struct_field_str(&w, "path", path);
  struct_field_str(&w, "name", name);
  struct_field_str(&w, "type", typestr(st->type));
  struct_field_int(&w, "inode", st->ino);
  struct_field_int(&w, "size", st->size);
  struct_end(&w);
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    {
      char namebuf[DIRSIZ+1];
      basename(namebuf, sizeof(namebuf), path);
      emit_entry(path, json_mode ? namebuf : fmtname(path), &st);
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
      emit_entry(buf, p, &st);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i = modern_consume_flags("ls", argc, argv, 1, &json_mode);

  if(i >= argc){
    ls(".");
    exit();
  }
  for(; i<argc; i++)
    ls(argv[i]);
  exit();
}
