// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

void write(const char *buf, const int idx);

int print_mesg(const char *buf, const int idx)
{
	write(buf, idx);
	return 0;
}
