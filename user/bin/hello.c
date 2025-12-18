// Hello Shell.

#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "structio.h"
#include "modern.h"

// Parsed command representation
#define EXEC  1
#define REDIR 2
#define PIPE  3
#define LIST  4
#define BACK  5

#define MAXARGS 10
#define HELLO_MAXPATH 128

static int hello_json = 0;
static char hello_path[HELLO_MAXPATH] = "bin";

struct cmd {
  int type;
};

struct execcmd {
  int type;
  char *argv[MAXARGS];
  char *eargv[MAXARGS];
};

struct redircmd {
  int type;
  struct cmd *cmd;
  char *file;
  char *efile;
  int mode;
  int fd;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
  int type;
  struct cmd *cmd;
};

int fork1(void);  // Fork but panics on failure.
void panic(char*);
struct cmd *parsecmd(char*);

static void try_exec(char *cmd, char **argv);
static void hello_print_path(void);
static void hello_set_path(const char *value);
static void hello_help(void);
static void hello_show_prompt(void);
static int handle_builtin(char *buf);
static void trim_newline(char *s);
static char* skipws(char *s);
static void hello_copy(char *dst, const char *src, int dstsz);
static int hello_strncmp(const char *a, const char *b, int n);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit();

  switch(cmd->type){
  default:
    panic("runcmd");

  case EXEC:
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit();
    try_exec(ecmd->argv[0], ecmd->argv);
    printf(2, "hello: exec %s failed\n", ecmd->argv[0]);
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    close(rcmd->fd);
    if(open(rcmd->file, rcmd->mode) < 0){
      printf(2, "open %s failed\n", rcmd->file);
      exit();
    }
    runcmd(rcmd->cmd);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    if(fork1() == 0)
      runcmd(lcmd->left);
    wait();
    runcmd(lcmd->right);
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    if(pipe(p) < 0)
      panic("pipe");
    if(fork1() == 0){
      close(1);
      dup(p[1]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->left);
    }
    if(fork1() == 0){
      close(0);
      dup(p[0]);
      close(p[0]);
      close(p[1]);
      runcmd(pcmd->right);
    }
    close(p[0]);
    close(p[1]);
    wait();
    wait();
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    if(fork1() == 0)
      runcmd(bcmd->cmd);
    break;
  }
  exit();
}

int
getcmd(char *buf, int nbuf)
{
  hello_show_prompt();
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(int argc, char *argv[])
{
  static char buf[100];
  int fd;
  int argi = modern_consume_flags("hello", argc, argv, 1, &hello_json);

  if(argi < argc){
    printf(2, "hello: unexpected arguments\n");
  }

  // Ensure that three file descriptors are open.
  while((fd = open("console", O_RDWR)) >= 0){
    if(fd >= 3){
      close(fd);
      break;
    }
  }

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(handle_builtin(buf))
      continue;
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait();
  }
  exit();
}

void
panic(char *s)
{
  printf(2, "%s\n", s);
  exit();
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//PAGEBREAK!
// Constructors

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = EXEC;
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = REDIR;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->efile = efile;
  cmd->mode = mode;
  cmd->fd = fd;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = PIPE;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
listcmd(struct cmd *left, struct cmd *right)
{
  struct listcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = LIST;
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

struct cmd*
backcmd(struct cmd *subcmd)
{
  struct backcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = BACK;
  cmd->cmd = subcmd;
  return (struct cmd*)cmd;
}
//PAGEBREAK!
// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>&;()";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '(':
  case ')':
  case ';':
  case '&':
  case '<':
    s++;
    break;
  case '>':
    s++;
    if(*s == '>'){
      ret = '+';
      s++;
    }
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;

  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);
struct cmd *nulterminate(struct cmd*);

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    printf(2, "leftovers: %s\n", s);
    panic("syntax");
  }
  nulterminate(cmd);
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parsepipe(ps, es);
  while(peek(ps, es, "&")){
    gettoken(ps, es, 0, 0);
    cmd = backcmd(cmd);
  }
  if(peek(ps, es, ";")){
    gettoken(ps, es, 0, 0);
    cmd = listcmd(cmd, parseline(ps, es));
  }
  return cmd;
}

static void
hello_show_prompt(void)
{
  if(hello_json){
    struct struct_writer w;
    struct_begin(&w, 1);
    struct_field_str(&w, "prompt", "hello");
    struct_end(&w);
  } else {
    printf(2, "hello$ ");
  }
}

static void
trim_newline(char *s)
{
  for(int i = 0; s[i]; i++){
    if(s[i] == '\n' || s[i] == '\r'){
      s[i] = 0;
      return;
    }
  }
}

static char*
skipws(char *s)
{
  while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
    s++;
  return s;
}

static void
hello_emit_status(const char *event, const char *detail)
{
  modern_emit_status("hello", event, detail, hello_json, "ok");
}

static void
hello_print_path(void)
{
  if(hello_json){
    struct struct_writer w;
    struct_begin(&w, 1);
    struct_field_str(&w, "command", "hello");
    struct_field_str(&w, "path", hello_path);
    struct_end(&w);
  } else {
    printf(1, "PATH=%s\n", hello_path);
  }
}

static void
hello_set_path(const char *value)
{
  if(value == 0 || value[0] == 0)
    value = "bin";
  int len = strlen(value);
  if(len >= HELLO_MAXPATH)
    len = HELLO_MAXPATH-1;
  memmove(hello_path, value, len);
  hello_path[len] = 0;
  hello_emit_status("path", hello_path);
}

static void
hello_help(void)
{
  if(hello_json){
    struct struct_writer w;
    struct_begin(&w, 1);
    struct_field_str(&w, "command", "hello");
    struct_field_str(&w, "help",
      "Built-ins: cd <dir>, path [set <paths>], help, version");
    struct_end(&w);
  } else {
    printf(1, "Hello Shell commands:\n");
    printf(1, "  cd <dir>        change directory\n");
    printf(1, "  path            show command search path\n");
    printf(1, "  path set X      set path (colon separated)\n");
    printf(1, "  help            show this message\n");
    printf(1, "  version         show version info\n");
  }
}

static void
append_exec_path(char *dst, int dstsz, const char *prefix, const char *cmd)
{
  if(prefix == 0 || prefix[0] == 0){
    hello_copy(dst, cmd, dstsz);
    return;
  }
  int n = strlen(prefix);
  if(n >= dstsz)
    n = dstsz - 1;
  memmove(dst, prefix, n);
  int need_slash = (n > 0 && dst[n-1] != '/');
  if(need_slash && n < dstsz - 1)
    dst[n++] = '/';
  dst[n] = 0;
  if(n < dstsz - 1)
    hello_copy(dst + n, cmd, dstsz - n);
}

static void
try_exec(char *cmd, char **argv)
{
  if(strchr(cmd, '/')){
    exec(cmd, argv);
    return;
  }

  char pathbuf[HELLO_MAXPATH];
  hello_copy(pathbuf, hello_path, sizeof(pathbuf));
  char *segment = pathbuf;
  while(1){
    char *end = segment;
    while(*end && *end != ':')
      end++;
    char saved = *end;
    *end = 0;
    char full[128];
    append_exec_path(full, sizeof(full), segment, cmd);
    exec(full, argv);
    *end = saved;
    if(*end == 0)
      break;
    segment = end + 1;
  }

  // Last resort: try the raw command name.
  exec(cmd, argv);
}

static int
handle_builtin(char *buf)
{
  trim_newline(buf);
  char *s = skipws(buf);
  if(*s == 0)
    return 1;

  if(s[0] == 'c' && s[1] == 'd' && (s[2] == 0 || s[2] == ' ' || s[2] == '\t')){
    char *arg = skipws(s+2);
    if(*arg == 0){
      printf(2, "hello: cd requires a path\n");
    } else if(chdir(arg) < 0){
      printf(2, "hello: cannot cd %s\n", arg);
    }
    return 1;
  }

  if(strcmp(s, "help") == 0){
    hello_help();
    return 1;
  }

  if(strcmp(s, "version") == 0 || strcmp(s, "--version") == 0){
    modern_print_version("hello", hello_json);
    return 1;
  }

  if(hello_strncmp(s, "path", 4) == 0 && (s[4] == 0 || s[4] == ' ' || s[4] == '\t')){
    char *arg = skipws(s+4);
    if(*arg == 0){
      hello_print_path();
    } else if(hello_strncmp(arg, "set", 3) == 0 && (arg[3] == 0 || arg[3] == ' ' || arg[3] == '\t')){
      char *val = skipws(arg+3);
      hello_set_path(val);
    } else {
      printf(2, "hello: usage path [set <paths>]\n");
    }
    return 1;
  }

  return 0;
}

static void
hello_copy(char *dst, const char *src, int dstsz)
{
  if(dstsz <= 0)
    return;
  int i = 0;
  for(; i < dstsz - 1 && src && src[i]; i++)
    dst[i] = src[i];
  dst[i] = 0;
}

static int
hello_strncmp(const char *a, const char *b, int n)
{
  for(int i = 0; i < n; i++){
    char ca = a[i];
    char cb = b[i];
    if(ca != cb)
      return ca - cb;
    if(ca == 0)
      return 0;
  }
  return 0;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a')
      panic("missing file for redirection");
    switch(tok){
    case '<':
      cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
      break;
    case '>':
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    case '+':  // >>
      cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
      break;
    }
  }
  return cmd;
}

struct cmd*
parseblock(char **ps, char *es)
{
  struct cmd *cmd;

  if(!peek(ps, es, "("))
    panic("parseblock");
  gettoken(ps, es, 0, 0);
  cmd = parseline(ps, es);
  if(!peek(ps, es, ")"))
    panic("syntax - missing )");
  gettoken(ps, es, 0, 0);
  cmd = parseredirs(cmd, ps, es);
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;

  if(peek(ps, es, "("))
    return parseblock(ps, es);

  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|)&;")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a')
      panic("syntax");
    cmd->argv[argc] = q;
    cmd->eargv[argc] = eq;
    argc++;
    if(argc >= MAXARGS)
      panic("too many args");
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  cmd->eargv[argc] = 0;
  return ret;
}

// NUL-terminate all the counted strings.
struct cmd*
nulterminate(struct cmd *cmd)
{
  int i;
  struct backcmd *bcmd;
  struct execcmd *ecmd;
  struct listcmd *lcmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    return 0;

  switch(cmd->type){
  case EXEC:
    ecmd = (struct execcmd*)cmd;
    for(i=0; ecmd->argv[i]; i++)
      *ecmd->eargv[i] = 0;
    break;

  case REDIR:
    rcmd = (struct redircmd*)cmd;
    nulterminate(rcmd->cmd);
    *rcmd->efile = 0;
    break;

  case PIPE:
    pcmd = (struct pipecmd*)cmd;
    nulterminate(pcmd->left);
    nulterminate(pcmd->right);
    break;

  case LIST:
    lcmd = (struct listcmd*)cmd;
    nulterminate(lcmd->left);
    nulterminate(lcmd->right);
    break;

  case BACK:
    bcmd = (struct backcmd*)cmd;
    nulterminate(bcmd->cmd);
    break;
  }
  return cmd;
}
