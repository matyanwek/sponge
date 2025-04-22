#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
	size_t cap;
	size_t len;
	char *data;
} StringBuilder;

StringBuilder *sb_init() {
	StringBuilder *sb = malloc(sizeof(StringBuilder));
	if (sb == NULL) {
		char *msg = strerror(errno);
		fprintf(stderr, "%s\n", msg);
		exit(EXIT_FAILURE);
	}

	sb->cap = 0;
	sb->len = 0;
	return sb;
}

void sb_del(StringBuilder *sb) {
	free(sb->data);
	free(sb);
}

void *sb_extend(StringBuilder *sb, const char *s, size_t len) {
	if (sb->cap < sb->len + len) {
		sb->cap = sb->len + len;
		sb->data = realloc(sb->data, sb->cap * sizeof(char));
		if (sb->data == NULL) {
			char *msg = strerror(errno);
			fprintf(stderr, "%s\n", msg);
			exit(EXIT_FAILURE);
		}
	}

	memcpy(sb->data + sb->len, s, len);
	sb->len += len;

	return sb;
}

void *load_stdin(StringBuilder *sb) {
	char buf[BUFSIZ];

	for (;;) {
		size_t n = fread(buf, sizeof(char), BUFSIZ, stdin);
		sb_extend(sb, buf, n);

		if (n != BUFSIZ * sizeof(char)) {
			if (ferror(stdin)) {
				fprintf(stderr, "error reading STDIN\n");
				exit(EXIT_FAILURE);
			}
			break;
		}
	}

	return sb;
}

bool dump(char *filename, char *write_mode, StringBuilder *sb) {
	FILE *file;

	if (strcmp(filename, "-") == 0) {
		file = stdout;
	} else {
		file = fopen(filename, write_mode);
		if (file == NULL) {
			char *msg = strerror(errno);
			fprintf(stderr, "error opening file '%s': %s\n", filename, msg);
			return false;
		}
	}

	bool ok = true;
	size_t n = fwrite(sb->data, sizeof(char), sb->len, file);
	if (n != sb->len && ferror(file)) {
		fprintf(stderr, "error writing to file '%s'\n", filename);
		ok = false;
	}

	if (strcmp(filename, "-") != 0) {
		if (fclose(file) != 0) {
			char *msg = strerror(errno);
			fprintf(stderr, "error closing file '%s': %s\n", filename, msg);
			ok = false;
		}
	}

	return ok;
}

int main(int argc, char **argv) {
	int opt;
	char *write_mode = "w";
	while ((opt = getopt(argc, argv, "ah")) != -1) {
		switch (opt) {
		case 'a':
			write_mode = "a";
			break;
		case 'h':
			printf("Usage: %s [OPTION]... [FILE]...\n", argv[0]);
			printf("Soak up standard input and write to FILE(s).\n");
			printf("\n");
			printf("With no FILE, or when file is -, write to standard output.\n");
			printf("\n");
			printf("\t-a\tAppend to file instead of overwriting it\n");
			printf("\t-h\tprint this Help and exit\n");
			exit(EXIT_SUCCESS);
		default: /* '?' */
			fprintf(stderr, "Usage: %s [OPTION]... [FILE]...\n", argv[0]);
			fprintf(stderr, "try %s -h for help\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	StringBuilder *sb = sb_init();
	load_stdin(sb);

	if (optind == argc) {
		// no args; dump to stdout and return
		return dump("-", write_mode, sb) ? 0 : 1;
	}

	int ret = EXIT_SUCCESS;
	for (int i = optind; i < argc; i++) {
		bool ok = dump(argv[i], write_mode, sb);
		if (!ok) {
			ret = EXIT_FAILURE;
		}
	}

	return ret;
}
