#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	size_t cap;
	size_t len;
	char *data;
} String;

void *string_grow(String *s, size_t delta) {
	size_t new_cap = s->cap > 0 ? s->cap : delta;
	while (new_cap < s->len + delta) {
		new_cap *= 2;
	}

	if (s->cap < new_cap) {
		s->data = realloc(s->data, sizeof(char) * new_cap);
		if (s->data == NULL) {
			error(EXIT_FAILURE, errno, "error growing String");
		}
		s->cap = new_cap;
	}

	return s;
}

bool read_file(const char *filename, String *s) {
	FILE *file;
	if (strcmp(filename, "-") == 0) {
		file = stdin;
	} else {
		FILE *file = fopen(filename, "r");
		if (file == NULL) {
			return false;
		}
	}

	for (int i = 0; true; i++) {
		string_grow(s, BUFSIZ);
		size_t n = fread(s->data + s->len, sizeof(char), BUFSIZ, file);
		s->len += n;
		if (n == BUFSIZ) {
			continue;
		} else if (ferror(file)) {
			if (file != stdin) {
				fclose(file);
			}
			return false;
		} else {
			break;
		}
	}

	bool ok = file == stdin ? true : !(bool)fclose(file);
	return ok;
}

bool dump_file(const char *filename, String *s, bool append) {
	FILE *file;
	if (strcmp(filename, "-") == 0) {
		file = stdout;
	} else {
		file = fopen(filename, append ? "a" : "w");
		if (file == NULL) {
			return false;
		}
	}

	size_t n = fwrite(s->data, sizeof(char), s->len, file);
	if (n != s->len && ferror(file)) {
		if (file != stdin) {
			fclose(file);
		}
		return false;
	}

	bool ok = file == stdout ? true : !(bool)fclose(file);
	return ok;
}

int main(int argc, char **argv) {
	int opt;
	bool aflag = false;
	while ((opt = getopt(argc, argv, "ah")) != -1) {
		switch (opt) {
		case 'a':
			aflag = true;
			break;
		case 'h':
			printf("Usage: %s [OPTION]... [FILE]...\n", argv[0]);
			printf("Soak up standard input and write to FILE(s).\n");
			printf("\n");
			printf("With no FILE, or when FILE is -, write to standard output.\n");
			printf("\n");
			printf("\t-a\tAppend to file instead of overwriting it\n");
			printf("\t-h\tprint this Help and exit\n");
			return EXIT_SUCCESS;
		default:
			fprintf(stderr, "Usage: %s [OPTION]... [FILE]...\n", argv[0]);
			fprintf(stderr, "try %s -h for help\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	String s;
	if (!read_file("-", &s)) {
		error(EXIT_FAILURE, errno, "error reading from stdin");
	}

	size_t nargs = argc - optind;
	char **args;
	if (nargs < 1) {
		nargs = 1;
		args = (char *[]){"-"};
	} else {
		args = argv + optind;
	}

	for (size_t i = 0; i < nargs; i++) {
		if (!dump_file(args[i], &s, aflag)) {
			error(EXIT_FAILURE, errno, "error writing to file \"%s\"", args[i]);
		}
	}

	return EXIT_SUCCESS;
}
