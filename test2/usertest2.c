
int print_mesg(const char *a, const int idx);
void exit(const int );
void main(void)
{
	int i=0;
	const char *a="I am worker2!!\r\n";
	for (i=0 ; i<10 ; i++) {
		print_mesg(a,3);
	}
	exit(3);
}
