#ifndef _FILE_
#define _FILE_
struct file {
	struct file_system *pfs;
	int fd;
	int pos;
	char name[128];
};

struct file *create_file(int _fd, char *str);
struct file *open_file(char *str);
int close_file(struct file *fp);
int read(struct file *fp, int _n, char *_buf);
int write(struct file *fp, int _n, char * _buf);
void reset(struct file *fp);
void rewrite(struct file *fp);
int eof(struct file *fp);
#endif
