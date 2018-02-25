/*
  kernel/core/gic.c general interrupt controller 
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

int main(int argc, char **argv)
{
	int i;
	FILE *outfp, *infp;
	/* user app size should be less than 1MB */
	char buff[1024*1024];
	unsigned int size, cnt = argc-2, padding;

	if (argc < 3) {
		cout << "usage : mkappfs out in1 in2...\n" << endl;
		return 0;
	}

	outfp = fopen(argv[1], "w");
	if (!outfp) return 0;

	/*sprintf(cnt, "%d", argc-2);*/
	fwrite(&cnt, 1, sizeof(unsigned int), outfp);

	for (i = 2; i < argc; i++) {
		infp = fopen(argv[i], "r");
		if (!infp) return 0;

		fseek(infp, 0, SEEK_SET);

		size = 0;
		fseek(outfp, 0, SEEK_END);
		memset(buff, 0x0, sizeof(buff));
		size = fread(buff, sizeof(char), sizeof(buff), infp);
		cout << "read : " << size << endl;
		padding = size % 4;
		if (padding != 0)  {
			padding = 4 - padding;
			while (padding > 0) {
				buff[size] = 0x00;
				padding--;
				size++;
			}	
		}	
		fwrite(&size, 1, sizeof(size), outfp);
		fwrite(buff, size, 1, outfp);
		cout << "write : " << size << endl;
		
		fclose(infp);
	}
	fclose(outfp);

	return 0;
}
