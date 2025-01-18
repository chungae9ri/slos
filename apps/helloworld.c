// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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
		if (i >= 0xFFFFFFFF) {
			i = 0;
		} else {
			i++;
		}
	}
}
