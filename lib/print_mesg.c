extern void write(const char *buf, const int idx);
int print_mesg(const char *buf, const int idx)
{
	write(buf, idx);
	return 0;
}
