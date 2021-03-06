#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <nmea.h>
#include <nmea/gpgll.h>
#include <nmea/gpgga.h>

int
main(void)
{
	int gps_fd;
	int read_bytes, total_bytes = 0;
	char *buffer, *start, *end;

	buffer = malloc(4096);
	if (NULL == buffer) {
		perror("malloc buffer");
		exit(EXIT_FAILURE);
	}

	gps_fd = 0; // stdin

	while (1) {
		read_bytes = read(gps_fd, buffer + total_bytes, 20);
		if (-1 == read_bytes) {
			perror("read ttyUSB0");
			exit(EXIT_FAILURE);
		}
		total_bytes += read_bytes;

		/* find start (a dollar $ign) */
		start = memchr(buffer, '$', total_bytes);
		if (NULL == start) {
			total_bytes = 0;
			continue;
		}

		/* find end of line */
		end = memchr(start, NMEA_END_CHAR_1, total_bytes - (start - buffer));
		if (NULL == end || NMEA_END_CHAR_2 != *(++end)) {
			continue;
		}

		/* handle data */
		char buf[255];
		nmea_s *data;

		data = nmea_parse(start, end - start + 1, 0);
		if (NULL == data) {
			printf("Could not parse sentence\n");
			continue;
		}

		if (0 < data->errors) {
			printf("WARN: The sentence struct contains parse errors!\n");
		}

		if (NMEA_GPGGA == data->type) {
			printf("GPGGA sentence\n");
			nmea_gpgga_s *gpgga = (nmea_gpgga_s *) data;
			printf("Number of satellites: %d\n", gpgga->n_satellites);
			printf("Altitude: %d %c\n", gpgga->altitude, gpgga->altitude_unit);
		}

		if (NMEA_GPGLL == data->type) {
			printf("GPGLL sentence\n");
			nmea_gpgll_s *pos = (nmea_gpgll_s *) data;
			printf("Longitude:\n");
			printf("  Degrees: %d\n", pos->longitude.degrees);
			printf("  Minutes: %f\n", pos->longitude.minutes);
			printf("  Cardinal: %c\n", (char) pos->longitude.cardinal);
			printf("Latitude:\n");
			printf("  Degrees: %d\n", pos->latitude.degrees);
			printf("  Minutes: %f\n", pos->latitude.minutes);
			printf("  Cardinal: %c\n", (char) pos->latitude.cardinal);
			strftime(buf, sizeof(buf), "%H:%M:%S", &pos->time);
			printf("Time: %s\n", buf);
		}

		nmea_free(data);

		/* buffer empty? */
		if (end == buffer + total_bytes) {
			total_bytes = 0;
			continue;
		}

		/* copy rest of buffer to beginning */
		if (buffer != memmove(buffer, end, total_bytes - (end - buffer))) {
			total_bytes = 0;
			continue;
		}

		total_bytes -= end - buffer;
	}

	close(gps_fd);
	return 0;
}
