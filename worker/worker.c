
void exit(const int);
int print_mesg(const char *a, const int idx);

void main(void)
{
	char sleepdur=10;
	const char *a="I am user worker!! \r\n";
	int cnt=0;

	while(1) {
		print_mesg(a,0);
	 	sleep(&sleepdur, 0); /* arg0*100 : sleep time 100msec step, arg1 : idx*/
		/* doing something in here */
		while(1) {
			cnt++;
			if(cnt >=10000) {
				cnt=0;
				break;
			}
		}
	}
	exit(0);
}
