#ifndef __FSDATA_H__
#define __FSDATA_H__

struct fsdata_file {
  const struct fsdata_file *next;
  const unsigned char *name;
  const unsigned char *data;
  const int len;
  const char flag;
};

struct fsdata_file_noconst {
  struct fsdata_file *next;
  char *name;
  char *data;
  int len;
};

#endif /* __FSDATA_H__ */
