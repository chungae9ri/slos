
int print_mesg(const char *a, const int idx);
void exit(const int );

void main(void)
{
	const char *a="hello world!!\r\n";
	const char *b="nice to meet you!!\r\n";
	print_mesg(a,1);
	print_mesg(b,1);
	exit(1);
}
