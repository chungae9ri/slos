/*
  kernel/core/helloworld.c
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

int print_mesg(const char *buf, const int idx);

void main(void)
{
	const char *a = "hello world!!\n";
	const char *b = "nice to meet you!!\n";
	const char *c = "I am user_0 app!!\n";
	unsigned int i;

	print_mesg(a, 0);
	print_mesg(b, 0);
	print_mesg(c, 0);

	i = 0;
	while (1) {
		if (i>= 0xFFFFFFFF) {
			i = 0;
		} else {
			i++;
		}
	}
}
