#include "parser.h"

/**
 * Where to find the parser modules.
 * Can be overridden by env variable NMEA_PARSER_PATH
 */
#define PARSER_PATH "/usr/lib/nmea/"

static inline int
_get_so_files(const char *path, char **files)
{
	int len, i = 0;
	DIR *d;
	struct dirent *dir;

	d = opendir(path);
	if (NULL == d) {
		return -1;
	}

	while (NULL != (dir = readdir(d))) {
		if (dir->d_name == NULL || dir->d_name[0] == '.') {
			continue;
		}

		len = strlen(dir->d_name);
		if (len < 3) {
			continue;
		}

		if (0 != strncmp(dir->d_name + len - 3, ".so", 3)) {
			continue;
		}

		char *name = malloc(255);
		if (NULL == name) {
			return -1;
		}

		strcpy(name, path);
		strcat(name, dir->d_name);
		files[i++] = name;
	}

	return i;
}

nmea_parser_module_s *
nmea_init_parser(const char *filename)
{
	nmea_parser_module_s *parser;

	/* Allocate parser struct */
	parser = malloc(sizeof (nmea_parser_module_s));
	if (NULL == parser) {
		return (nmea_parser_module_s *) NULL;
	}

	void *plugin = dlopen(filename, RTLD_NOW);
	if (NULL == plugin) {
		return (nmea_parser_module_s *) NULL;
	}

 	parser->handle = plugin;

	init_f init = dlsym(plugin, "init");
	if (NULL == init) {
		return (nmea_parser_module_s *) NULL;
	}

	parser->allocate_data = dlsym(plugin, "allocate_data");
	if (NULL == parser->allocate_data) {
		return (nmea_parser_module_s *) NULL;
	}

	parser->set_default = dlsym(plugin, "set_default");
	if (NULL == parser->set_default) {
		return (nmea_parser_module_s *) NULL;
	}

	parser->free_data = dlsym(plugin, "free_data");
	if (NULL == parser->free_data) {
		return (nmea_parser_module_s *) NULL;
	}

	parser->parse = dlsym(plugin, "parse");
	if (NULL == parser->parse) {
		return (nmea_parser_module_s *) NULL;
	}

	if (-1 == init((nmea_parser_s *) parser)) {
		return (nmea_parser_module_s *) NULL;
	}

	return parser;
}

int
nmea_load_parsers()
{
	int i;
	char *files[255];
	char *parser_path;
	nmea_parser_module_s *parser;

	/* Get list of so files */
	parser_path = getenv("NMEA_PARSER_PATH");
	if (NULL == parser_path) {
		/* Use default path */
		parser_path = strdup(PARSER_PATH);
	} else {
		parser_path = strdup(parser_path);
	}

	n_parsers = _get_so_files(parser_path, files);
	free(parser_path);
	if (1 > n_parsers) {
		return -1;
	}

	/* Allocate parsers array */
	parsers = malloc((sizeof (nmea_parser_module_s *)) * n_parsers);
	if (NULL == parsers) {
		return (nmea_parser_module_s *) NULL;
	}
	memset(parsers, 0, (sizeof (nmea_parser_module_s *)) * n_parsers);

	i = n_parsers;
	while (i-- > 0) {
		parser = nmea_init_parser(files[i]);
		free(files[i]);

		if (NULL == parser) {
			return -1;
		}

		parsers[(int) parser->parser.type - 1] = parser;
	}

	return n_parsers;
}

void
nmea_unload_parsers()
{
	int i = 0;
	nmea_parser_module_s *parser;

	while (i < n_parsers) {
		parser = parsers[i++];
		if (NULL == parser) {
			continue;
		}

		dlclose(parser->handle);
		free(parser);
	}
}

nmea_parser_module_s *
nmea_get_parser_by_type(nmea_t type)
{
	return parsers[(int) type - 1];
}

nmea_parser_module_s *
nmea_get_parser_by_sentence(const char *sentence)
{
	int i = 0;
	nmea_parser_module_s *parser;

	while (i < n_parsers) {
		parser = parsers[i++];
		if (NULL == parser) {
			continue;
		}

		if (0 == strncmp(sentence + 1, parser->parser.type_word, NMEA_PREFIX_LENGTH)) {
			return parser;
		}
	}

	return (nmea_parser_module_s *) NULL; 
}
