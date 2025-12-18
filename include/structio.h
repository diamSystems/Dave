#ifndef STRUCTIO_H
#define STRUCTIO_H

struct struct_writer {
  int fd;
  int need_comma;
};

void struct_begin(struct struct_writer *w, int fd);
void struct_field_str(struct struct_writer *w, const char *key, const char *value);
void struct_field_int(struct struct_writer *w, const char *key, int value);
void struct_end(struct struct_writer *w);

#endif // STRUCTIO_H
