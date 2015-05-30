
void exit(const int );
void main(void)
{
	int i=0;
	const char *a="I am worker1!!\r\n";
	for (i=0 ; i<20 ; i++) {
		print_mesg(a,2);
	}
	exit(2);
}
