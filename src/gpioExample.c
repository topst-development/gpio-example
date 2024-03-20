/*
 * gpio-example.c
 *
 * Copyright (C) 2015 - packt publishing <texier.pj2@gmail.com>
 *
 * <gpio-example> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <gpio-example> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <gpio-example>. If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <linux/types.h>

/*=================================================*/
/*== 		      DEFINE 			 			 ==*/
/*=================================================*/
#define LENGTH 128
#define SYSFS_GPIO_DIR "/sys/class/gpio/"
#define GPIO_PIN 62
#define BUFFER_SIZE 255
#define DEBUG 1
#define VERSION "0.1"
/*=================================================*/
/*== 		      PROTO 			 			 ==*/
/*=================================================*/

typedef enum
{
	LOG_DEBUG = 0,
	LOG_WARN,
	LOG_INFO,
	LOG_ERROR,
} log_level_t;

void log_message(log_level_t level, const char *format, ...);
int unexport(int gpio);
int export(unsigned int gpio);
int dir(int gpio, char *dir);
int getval(int gpio);
int setval(int gpio, int val);
int set_in(int gpio);
int set_out(int gpio);
int gpio_set_edge(int gpio, char *type);
static void wait_bp_state();
void int_quit(int sig);
static void display_help(const char *name);
static int parse_opts(int argc, char *argv[]);

int quit;
int gpio;
int direction;
int value;
int ret_val = 0;
static char *project = "gpio-example - Yocto Project for Raspberry-Pi";
static log_level_t p_loglevel = LOG_INFO;

/*=================================================*/
/*== 		      MAIN 			 				 ==*/
/*=================================================*/
int main(int argc, char *argv[])
{

	quit = 0;

	signal(SIGINT, int_quit);

	if (argc < 2)
	{
		display_help(argv[0]);
		return 0;
	}
	ret_val = parse_opts(argc, argv);
	if (ret_val)
	{
		display_help(argv[0]);
		return ret_val;
	}

	return 0;
}

/*=================================================*/
/*== 		      FUNCTIONS			  			 ==*/
/*=================================================*/

void log_message(log_level_t debug_level, const char *format, ...)
{
	char buffer[BUFFER_SIZE] = {0}, timestamp[BUFFER_SIZE] = {0}, *slevel;
	va_list ap;
	time_t rawtime;
	struct tm *timeinfo;

	if (debug_level >= p_loglevel)
	{
		va_start(ap, format);
		vsnprintf(buffer, BUFFER_SIZE, format, ap);
		va_end(ap);

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(timestamp, BUFFER_SIZE, "%d/%m/%Y %X", timeinfo);

		switch (debug_level)
		{
		case LOG_DEBUG:
			slevel = "DEBUG";
			break;
		case LOG_WARN:
			slevel = "WARNING";
			break;
		case LOG_INFO:
			slevel = "INFO";
			break;
		case LOG_ERROR:
			slevel = "ERROR";
			break;
		}

		fprintf(stderr, "[%s] [%s] %s", timestamp, slevel, buffer);
	}
}

int unexport(int gpio)
{
	char buff[LENGTH];
	FILE *file;

	/* unexport GPIO */
	file = fopen(SYSFS_GPIO_DIR "unexport", "w");
	if (file == NULL)
	{
		printf("unable to open " SYSFS_GPIO_DIR "export\n");
		return -EXIT_FAILURE;
	}
	snprintf(buff, LENGTH, "%d", gpio);
	if (fprintf(file, "%s", buff) != strlen(buff))
	{
		printf("error writing to " SYSFS_GPIO_DIR "export\n");
		fclose(file);
		return -EXIT_FAILURE;
	}
	fclose(file);
	return 0;
}

int export(unsigned int gpio)
{
	char buff[LENGTH];
	FILE *file;

	/* export GPIO */
	file = fopen(SYSFS_GPIO_DIR "export", "w");
	if (file == NULL)
	{
		printf("unable to open " SYSFS_GPIO_DIR "export\n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	snprintf(buff, LENGTH, "%d", gpio);
	if (fprintf(file, "%s", buff) != strlen(buff))
	{
		printf("error writing to " SYSFS_GPIO_DIR "export\n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	fclose(file);
	return 0;
}

int dir(int gpio, char *dir)
{
	char buff[LENGTH];
	FILE *file;

	snprintf(buff, LENGTH, SYSFS_GPIO_DIR "gpio%d/direction", gpio);
	file = fopen(buff, "w");
	if (file == NULL)
	{
		printf("unable to open %s\n", buff);
		fclose(file);
		exit(EXIT_FAILURE);
	}
	snprintf(buff, LENGTH, "%s", dir);
	if (fprintf(file, "%s", buff) != strlen(buff))
	{
		printf("error writing\n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	fclose(file);
	return 0;
}

int getval(int gpio)
{
	char buff[LENGTH];
	FILE *file;
	int value = 0;

	snprintf(buff, LENGTH, SYSFS_GPIO_DIR "gpio%d/value", gpio);
	file = fopen(buff, "r");
	if (file == NULL)
	{
		printf("unable to open %s\n", buff);
		fclose(file);
		exit(EXIT_FAILURE);
	}
	if (fscanf(file, "%d", &value) != 1)
	{
		printf("error reading\n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	fclose(file);
	return value;
}

int setval(int gpio, int val)
{
	char buff[LENGTH];
	FILE *file;

	snprintf(buff, LENGTH, SYSFS_GPIO_DIR "gpio%d/value", gpio);
	file = fopen(buff, "w");
	if (file == NULL)
	{
		printf("unable to open %s\n", buff);
		fclose(file);
		exit(EXIT_FAILURE);
	}
	snprintf(buff, LENGTH, "%d", val);
	if (fprintf(file, "%s", buff) != strlen(buff))
	{
		printf("error writing\n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	fclose(file);
	return 0;
}

int set_in(int gpio)
{
	/** set direction */
	dir(gpio, "in");

	return 0;
}

int set_out(int gpio)
{
	/** set direction */
	dir(gpio, "out");

	return 0;
}

int gpio_set_edge(int gpio, char *type)
{
	char buff[LENGTH];
	FILE *file;

	snprintf(buff, LENGTH, SYSFS_GPIO_DIR "gpio%d/edge", gpio);
	file = fopen(buff, "w");
	if (file == NULL)
	{
		printf("unable to open %s\n", buff);
		exit(EXIT_FAILURE);
	}
	snprintf(buff, LENGTH, "%s", type);
	if (fprintf(file, "%s", buff) != strlen(buff))
	{
		printf("error writing\n");
		exit(EXIT_FAILURE);
	}
	fclose(file);
	return 0;
}

static void wait_bp_state()
{
	int fd;
	int val;
	fd_set fds;
	char file[LENGTH];
	char buffer[2];

	/*
	 * Set gpio on rising edge
	 *
	 *      ------------
	 *      ^	      |
	 * -----|	  	  |--------
	 */

	ret_val = gpio_set_edge(GPIO_PIN, "rising");
	if (ret_val)
	{
		printf("unable to set edge of %d\n", GPIO_PIN);
		return;
	}

	snprintf(file, LENGTH, SYSFS_GPIO_DIR "gpio%d/value", GPIO_PIN);
	fd = open(file, O_RDWR);
	if (fd == -1)
	{
		printf("unable to open %s\n", file);
		return;
	}

	while (quit != 1)
	{
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		if (select(fd + 1, NULL, NULL, &fds, NULL) < 0)
		{
			perror("select");
			break;
		}

		lseek(fd, 0, SEEK_SET);
		if (read(fd, &buffer, 2) != 2)
		{
			perror("read");
			break;
		}

		buffer[1] = '\0';

		val = strcmp(buffer, "1");
		if (val == 0)
		{
			printf("Current GPIO status low\n");
		}
		else
		{
			printf("Current GPIO status high\n");
		}
	}
	close(fd);
}

void int_quit(int sig)
{
	/* Ignore the ctrl+c sequence for the system */
	signal(sig, SIG_IGN);
	printf("Exiting...\n");
	quit = 1;
}

static void display_help(const char *name)
{
	fprintf(stderr, "This example show how to used the GPIO from sysfs interface\n\n");
	fprintf(stderr, "usage: %s options...\n", name);
	fprintf(stderr, " options:\n");
	fprintf(stderr, " -g --gpio=<num>\n");
	fprintf(stderr, " -i --input\n");
	fprintf(stderr, " -o --out\n");
	fprintf(stderr, " -r --raspberryPI display raspberry PI Map \n");
	fprintf(stderr, " -h --help Prints this help\n\n");
	fprintf(stderr, "Example: %s -g 82 -o 1 \n\n", name);
}

static void display_raspberryPiMap(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "                      +--------+ 		\n");
	fprintf(stderr, "                  3P3-|-1    2-|-5P0	\n");
	fprintf(stderr, "   I2C_SDA(15)/GPIO82-|-3    4-|-5P0 	\n");
	fprintf(stderr, "   I2C_SCL(15)/GPIO81-|-5    6-|-GND	\n");
	fprintf(stderr, "               GPIO83-|-7    8-|-GPIO87/UT_TXD(22) \n");
	fprintf(stderr, "                  GND-|-9   10-|-GPIO88/UT_RXD(22) \n");
	fprintf(stderr, "               GPIO84-|-11  12-|-GPIO89 \n");
	fprintf(stderr, "               GPIO85-|-13  14-|-GND	\n");
	fprintf(stderr, "               GPIO86-|-15  16-|-GPIO90 \n");
	fprintf(stderr, "                  3P3-|-17  18-|-GPIO65	\n");
	fprintf(stderr, " SPIO_MOSI(15)/GPIO63-|-19  20-|-GND 		\n");
	fprintf(stderr, " SPIO_MISO(15)/GPIO64-|-21  22-|-GPIO66	\n");
	fprintf(stderr, " SPIO_SCLK(15)/GPIO61-|-23  24-|-GPIO62/SPIO_CS0(15)\n");
	fprintf(stderr, "                  GND-|-25  26-|-GPIO67/SPIO_CS1(16)\n");
	fprintf(stderr, "            RESERVED0-|-27  28-|-RESERVED1 \n");
	fprintf(stderr, "              GPIO112-|-29  30-|-GND\n");
	fprintf(stderr, "              GPIO113-|-31  32-|-GPIO115/PDM_OUT(69)\n");
	fprintf(stderr, "  PDM_OUT(68)/GPIO114-|-33  34-|-GND \n");
	fprintf(stderr, "SPI1_MISO(22)/GPIO121-|-35  36-|-GPIO119/SPI1_CS0(22)\n");
	fprintf(stderr, "              GPIO117-|-37  38-|-GPIO120/SPI1_MOSI(22)\n");
	fprintf(stderr, "                  GND-|-39  40-|-GPIO118/SPI1_SCLK(22)\n");
	fprintf(stderr, "                      +--------+ \n");
	fprintf(stderr, "\n");
}

static int parse_opts(int argc, char *argv[])
{

	int long_index = 0;
	int opt;

	static struct option option[] =
		{
			{"help", no_argument, NULL, 'h'},
			{"raspberryPi", no_argument, NULL, 'r'},
			{"gpio", no_argument, NULL, 'g'},
			{"input", no_argument, NULL, 'i'},
			{"out", no_argument, NULL, 'o'},
			{"value", no_argument, NULL, 'v'},
			{0, 0, 0, 0}};

	gpio = -1;
	direction = -1;
	value = -1;

	while ((opt = getopt_long(argc, argv, "rhg:io:", option, &long_index)) >= 0)
	{
		switch (opt)
		{
		case 'r':
			display_raspberryPiMap();
			exit(EXIT_SUCCESS);
		case 'h':
			display_help(argv[0]);
			exit(EXIT_SUCCESS);
		case 'g':
			gpio = atoi(optarg);
			printf("gpio %d ", gpio);
			if(gpio > 0) {
				export(gpio);
			}
			else {
				return 1;
			}
			break;
		case 'i':
			direction = 1;
			printf("in ");
			if (gpio > 0) {
				set_in(gpio);
				value = getval(gpio);
				printf("%d\n", value);
			}
			else {
				return 1;
			}
			break;
		case 'o':
			direction = 2;
			value = atoi(optarg);
			printf("out ");
			if (gpio > 0 && direction == 2 && value >= 0) {
				set_out(gpio);
				setval(gpio, value);
				printf("%d\n", value);
			}
			else {
				return 1;
			}
			break;
		default:
			fprintf(stderr, "[ERROR] %s: Bad option. -h for help\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/*unexport*/
	if(gpio > 0) {
		unexport(gpio);
	}

	return 0;
}