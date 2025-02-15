// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup mkfs
 * @{
 *
 * @file
 *
 * @brief Build ramdisk image with user applications.
 * Format is user_app_number(4bytes) + user_app_0_size(4bytes) + user_app_0_bin + pad + ...
 *
 */

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

int main(int argc, char **argv)
{
	int i;
	/* User app size max size 1MB */
	char buff[1024 * 1024];
	uint32_t app_cnt = argc - 2;
	uint32_t padding;

	if (argc < 3) {
		cout << "usage : mkappfs out in1 in2...\n" << endl;
		return -1;
	}

	filesystem::path ramdisk_file_path = argv[1];
	if (!filesystem::exists(ramdisk_file_path)) {
		filesystem::remove(ramdisk_file_path);
	}

	ofstream ramdisk_file_os(ramdisk_file_path, ios::binary);
	ramdisk_file_os.write(reinterpret_cast<const char*>(&app_cnt), sizeof(app_cnt));

	for (i = 2; i < argc; i++) {
		filesystem::path app_path = argv[i];
		if (!filesystem::exists(app_path)) {
			cout << app_path << " doesn't exists" << endl;
			return -1;
		}

		ifstream app_is(app_path, ios::binary);
		app_is.read(reinterpret_cast<char *>(buff), 1024 * 1024);
		uint32_t bytes_read = static_cast<uint32_t>(app_is.gcount());
		cout << "bytes read: " << bytes_read << endl;

		padding = bytes_read % 4;
		if (padding) {
			padding = 4 - padding;
			while (padding-- > 0) {
				buff[bytes_read++] = 0x00;
			}
		}

		ramdisk_file_os.write(reinterpret_cast<const char *>(&bytes_read), sizeof(bytes_read));
		ramdisk_file_os.write(reinterpret_cast<const char *>(buff), bytes_read);
	}

	return 0;
}

/**
 * @}
 *
 */
