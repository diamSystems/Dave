#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "structio.h"
#include "modern.h"

#define MAX_LINES 512
#define MAX_COLS  160
#define MAX_FILE  128

struct line {
  int number;
  char text[MAX_COLS];
};

static struct line buffer[MAX_LINES];
static int line_count = 0;
static int dirty = 0;
static char current_file[MAX_FILE];
static int json_mode = 0;

static void editor_prompt(void);
static void emit_status(const char *event, const char *detail);
static void trim(char *s);
static char* skipws(char *s);
static int parse_int_token(char **s, int *ok);
static int begin_with_digit(const char *s);
static void to_upper(char *dst, const char *src, int max);
static int command_is(const char *cmd, const char *word);
static void print_help(void);
static void handle_line_input(char *s);
static void handle_command(char *s);
static void list_lines(int start, int end);
static void upsert_line(int number, const char *text, int mark_dirty);
static void delete_line_number(int number, int mark_dirty);
static int find_insert_index(int number);
static void save_file(const char *name);
static void load_file(const char *name);
static void set_filename(const char *name);
static void clear_buffer(void);
static void editor_copy(char *dst, const char *src, int max);
static void emit_error(const char *msg);

int
main(int argc, char *argv[])
{
  int argi = modern_consume_flags("hello-edit", argc, argv, 1, &json_mode);

  if(argi < argc)
    set_filename(argv[argi]);

  if(current_file[0])
    load_file(current_file);

  char buf[256];
  while(1){
    editor_prompt();
    if(gets(buf, sizeof(buf)) == 0)
      break;
    trim(buf);
    char *line = skipws(buf);
    if(line[0] == 0)
      continue;

    if(begin_with_digit(line)){
      handle_line_input(line);
      continue;
    }

    if(line[0] == 'q' && line[1] == '!' && line[2] == 0){
      emit_status("quit", "forced");
      break;
    }

    if((line[0] == 'q' || line[0] == 'Q') && line[1] == 0){
      if(dirty){
        emit_error("buffer modified (use Q!)");
        continue;
      }
      emit_status("quit", "clean");
      break;
    }

    handle_command(line);
  }

  exit();
}

static void
editor_prompt(void)
{
  if(json_mode){
    struct struct_writer w;
    struct_begin(&w, 1);
    struct_field_str(&w, "prompt", "hello-edit");
    struct_field_str(&w, "file", current_file);
    struct_field_int(&w, "lines", line_count);
    struct_field_int(&w, "dirty", dirty);
    struct_end(&w);
  } else {
    printf(1, "edit(%s%s) > ", current_file[0] ? current_file : "(new)", dirty ? "*" : "");
  }
}

static void
emit_status(const char *event, const char *detail)
{
  modern_emit_status("hello-edit", event, detail, json_mode, dirty ? "dirty" : "clean");
}

static void
print_help(void)
{
  printf(1, "hello-edit BASIC-style usage:\n");
  printf(1, "  <num> text   set a line (empty text deletes)\n");
  printf(1, "  LIST [a [b]] list program lines\n");
  printf(1, "  DELETE n     delete line n\n");
  printf(1, "  NEW          clear buffer\n");
  printf(1, "  OPEN file    load file (line numbers optional)\n");
  printf(1, "  WRITE [file] save program\n");
  printf(1, "  HELP         this text\n");
  printf(1, "  Q or Q!      quit (Q! ignores unsaved edits)\n");
}

static void
handle_line_input(char *s)
{
  int ok = 0;
  int number = parse_int_token(&s, &ok);
  if(!ok || number <= 0){
    emit_error("invalid line number");
    return;
  }
  char *text = skipws(s);
  if(text[0] == 0){
    delete_line_number(number, 1);
  } else {
    upsert_line(number, text, 1);
  }
}

static void
handle_command(char *s)
{
  char cmd[16];
  to_upper(cmd, s, sizeof(cmd));
  char *args = s;
  while(*args && *args != ' ' && *args != '\t')
    args++;
  args = skipws(args);

  if(command_is(cmd, "HELP") || command_is(cmd, "H")){
    print_help();
    return;
  }

  if(command_is(cmd, "LIST") || command_is(cmd, "L")){
    int ok = 0;
    if(args[0] == 0){
      list_lines(-1, -1);
      return;
    }
    int start = parse_int_token(&args, &ok);
    if(!ok){
      emit_error("invalid start line");
      return;
    }
    args = skipws(args);
    if(args[0] == 0){
      list_lines(start, start);
      return;
    }
    int end = parse_int_token(&args, &ok);
    if(!ok){
      emit_error("invalid end line");
      return;
    }
    list_lines(start, end);
    return;
  }

  if(command_is(cmd, "DELETE") || command_is(cmd, "D")){
    int ok = 0;
    int number = parse_int_token(&args, &ok);
    if(!ok){
      emit_error("usage: DELETE <line>");
      return;
    }
    delete_line_number(number, 1);
    return;
  }

  if(command_is(cmd, "NEW")){
    clear_buffer();
    emit_status("new", "buffer");
    return;
  }

  if(command_is(cmd, "OPEN") || command_is(cmd, "O")){
    if(args[0] == 0){
      emit_error("usage: OPEN <file>");
      return;
    }
    load_file(args);
    return;
  }

  if(command_is(cmd, "WRITE") || command_is(cmd, "W") || command_is(cmd, "SAVE")){
    const char *name = args[0] ? args : current_file;
    if(name[0] == 0){
      emit_error("no filename");
      return;
    }
    save_file(name);
    return;
  }

  emit_error("unknown command (type HELP)");
}

static void
list_lines(int start, int end)
{
  if(line_count == 0){
    printf(1, "(empty)\n");
    return;
  }
  for(int i = 0; i < line_count; i++){
    int number = buffer[i].number;
    if(start >= 0 && number < start)
      continue;
    if(end >= 0 && number > end)
      continue;
    printf(1, "%d %s\n", number, buffer[i].text);
  }
}

static int
find_insert_index(int number)
{
  int i = 0;
  while(i < line_count && buffer[i].number < number)
    i++;
  return i;
}

static void
upsert_line(int number, const char *text, int mark_dirty)
{
  if(line_count >= MAX_LINES){
    emit_error("buffer full");
    return;
  }
  int idx = find_insert_index(number);
  if(idx < line_count && buffer[idx].number == number){
    editor_copy(buffer[idx].text, text, MAX_COLS);
  } else {
    for(int i = line_count; i > idx; i--)
      buffer[i] = buffer[i-1];
    buffer[idx].number = number;
    editor_copy(buffer[idx].text, text, MAX_COLS);
    line_count++;
  }
  if(mark_dirty)
    dirty = 1;
}

static void
delete_line_number(int number, int mark_dirty)
{
  int idx = find_insert_index(number);
  if(idx >= line_count || buffer[idx].number != number)
    return;
  for(int i = idx; i < line_count-1; i++)
    buffer[i] = buffer[i+1];
  line_count--;
  if(mark_dirty)
    dirty = 1;
}

static void
save_file(const char *name)
{
  int fd = open(name, O_CREATE|O_WRONLY);
  if(fd < 0){
    emit_error("cannot write file");
    return;
  }
  for(int i = 0; i < line_count; i++){
    char numbuf[16];
    int len = 0;
    int n = buffer[i].number;
    char tmp[16];
    int t = 0;
    if(n == 0)
      tmp[t++] = '0';
    while(n > 0 && t < (int)sizeof(tmp)){
      tmp[t++] = '0' + (n % 10);
      n /= 10;
    }
    while(t-- > 0)
      numbuf[len++] = tmp[t];
    numbuf[len++] = ' ';
    write(fd, numbuf, len);
    write(fd, buffer[i].text, strlen(buffer[i].text));
    write(fd, "\n", 1);
  }
  close(fd);
  dirty = 0;
  set_filename(name);
  emit_status("write", name);
}

static void
load_file(const char *name)
{
  int fd = open(name, O_RDONLY);
  clear_buffer();
  if(fd < 0){
    printf(1, "hello-edit: new file %s\n", name);
    set_filename(name);
    emit_status("open", name);
    return;
  }

  char ch;
  char linebuf[MAX_COLS];
  int pos = 0;
  int last = 0;
  while(read(fd, &ch, 1) == 1){
    if(ch == '\r')
      continue;
    if(ch == '\n'){
      linebuf[pos] = 0;
      char *line = linebuf;
      char *tmp = line;
      int ok = 0;
      int number = parse_int_token(&tmp, &ok);
      if(ok)
        line = skipws(tmp);
      else
        number = (last += 10);
      if(number > last)
        last = number;
      upsert_line(number, line, 0);
      pos = 0;
      continue;
    }
    if(pos < MAX_COLS-1)
      linebuf[pos++] = ch;
  }
  if(pos > 0){
    linebuf[pos] = 0;
    char *line = linebuf;
    char *tmp = line;
    int ok = 0;
    int number = parse_int_token(&tmp, &ok);
    if(ok)
      line = skipws(tmp);
    else
      number = last + 10;
    upsert_line(number, line, 0);
  }
  close(fd);
  dirty = 0;
  set_filename(name);
  emit_status("open", name);
}

static void
set_filename(const char *name)
{
  editor_copy(current_file, name, sizeof(current_file));
}

static void
clear_buffer(void)
{
  line_count = 0;
  dirty = 0;
}

static void
editor_copy(char *dst, const char *src, int max)
{
  if(max <= 0)
    return;
  int i = 0;
  while(i < max - 1 && src && src[i]){
    dst[i] = src[i];
    i++;
  }
  dst[i] = 0;
}

static void
emit_error(const char *msg)
{
  if(json_mode)
    modern_emit_status("hello-edit", "error", msg, 1, "error");
  else
    printf(1, "hello-edit: %s\n", msg);
}

static char*
skipws(char *s)
{
  while(*s == ' ' || *s == '\t')
    s++;
  return s;
}

static void
trim(char *s)
{
  int n = strlen(s);
  while(n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1] == ' ' || s[n-1] == '\t'))
    s[--n] = 0;
}

static int
begin_with_digit(const char *s)
{
  return *s >= '0' && *s <= '9';
}

static int
parse_int_token(char **s, int *ok)
{
  char *p = *s;
  int n = 0;
  int any = 0;
  while(*p >= '0' && *p <= '9'){
    n = n*10 + *p - '0';
    p++;
    any = 1;
  }
  *s = p;
  if(ok)
    *ok = any;
  return n;
}

static void
to_upper(char *dst, const char *src, int max)
{
  int i = 0;
  while(src[i] && src[i] != ' ' && src[i] != '\t' && i < max-1){
    char c = src[i];
    if(c >= 'a' && c <= 'z')
      c -= 32;
    dst[i++] = c;
  }
  dst[i] = 0;
}

static int
command_is(const char *cmd, const char *word)
{
  return strcmp(cmd, word) == 0;
}
