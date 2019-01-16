/*	Copyright (C) 2019 David Leiter
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

struct Entry{
	uint32_t offset;
	uint32_t size;
	char *file_path;
};

struct MemoryArena {
	uint8_t *buffer;
	size_t size;
	size_t allocation_pos;
};

void initMemoryArena(struct MemoryArena *arena, size_t size)
{
	arena->buffer = malloc(size);
	arena->size = size;
	arena->allocation_pos = 0;
}

void destroyemoryArena(struct MemoryArena *arena)
{
	free(arena->buffer);
	arena->size = 0;
}

char *allocateString(struct MemoryArena *arena, size_t size)
{
	char *mem = 0;
	if (arena->allocation_pos + size < arena->size) {
		mem = (char *)&arena->buffer[arena->allocation_pos];
		arena->allocation_pos += size;
	}
	return mem;
}
void createFile(uint8_t *data, size_t size, const char *path)
{
	char buf[2048];
	const char *str = path;
	size_t str_pos = 0;
	size_t buf_pos = 0;
	while (str[str_pos] != 0) {
		if (str[str_pos] == '\\') {
			memcpy(buf + buf_pos, str, str_pos);
			buf[buf_pos + str_pos] = 0;
			mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			str += str_pos + 1;
			buf_pos += str_pos;
			buf[buf_pos] = '/';
			buf_pos++;
			str_pos = 0;
		}
		str_pos++;
	}
	memcpy(buf + buf_pos, str, str_pos);
	buf[buf_pos + str_pos] = 0;
	FILE *f = fopen(buf, "wb");
	fwrite(data, size, 1, f);
	fclose(f);
}
int main(int argc, char **argv)
{
	struct MemoryArena string_arena;
	initMemoryArena(&string_arena, 1024 * 1024 * 100);
	uint8_t *data_buffer = malloc(1024 * 1024 * 100);
	struct Entry *data = malloc(sizeof(*data) * 1024);
	FILE *file = fopen(argv[1], "rb");
	uint32_t version = 0;
	fread(&version, 4, 1, file);
	uint32_t num_files = 0;
	fread(&num_files, 4, 1, file);
	for (int i = 0; i < num_files; i++) {
		uint16_t strlength = 0;
		fread(&strlength, 2, 1, file);
		data[i].file_path = allocateString(&string_arena, strlength + 1);
		fread(data[i].file_path, 1, strlength, file);
		data[i].file_path[strlength] = 0;
		fread(&data[i].offset, 4, 1, file);
		fread(&data[i].size, 4, 1, file);
	}

	printf("Num Files:%d\n", num_files);
	for (int i = 0; i < num_files; i++) {
		fseek(file, data[i].offset, SEEK_SET);
		fread(data_buffer, data[i].size, 1, file);
		createFile(data_buffer, data[i].size, data[i].file_path);
		printf("%d\tOffset: %d\tSize: %d\nPath:%s\n", i, data[i].offset,
		       data[i].size, data[i].file_path);
	}

	free(data);
	free(data_buffer);
	destroyemoryArena(&string_arena);
	return 0;
}
