#ifndef _FILE_
#define _FILE_
struct file {
	int fd;
	int pos;
	char name[128];
};

struct file *create_file(int _fd, char *str);
int read(struct file *fp, int _n, char *_buf);
int write(struct file *fp, int _n, unsigned char * _buf);
void reset(struct file *fp);
void rewrite(struct file *fp);
int eof(struct file *fp);
#endif
