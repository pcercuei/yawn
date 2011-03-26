
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>

/* By default, we kick every second. */
#define DEFAULT_KICK_PERIOD 1

#define DEFAULT_WDT_DEVICE "/dev/watchdog"

static int fd;

/* Handles the SIGUSR1 signal.
 * This signal will send the magic 'V' character to stop
 * the watchdog device. */
static void close_and_quit(int sig)
{
	char magic = 'V';
	int ret;
	(void)sig;

	do {
		ret = write(fd, &magic, sizeof(magic));
		if (ret < 0) {
			perror("Unable to send magic \'V\' character");
			break;
		}
	} while (!ret);

	close(fd);
	exit(0);
}

static void show_usage()
{
	printf("Usage: yawn [OPTIONS...]\n\n"
				"\t-h, --help\t\tDisplay this help and exit.\n"
				"\t-d, --device\t\tSelect the watchdog device (default: \"/dev/watchdog\").\n"
				"\t-k, --kick-period\tPeriod (in seconds) between two kicks (default: 1).\n"
				"\t-t, --timeout\t\tNumber of seconds before the watchdog resets the device.\n");
}

int main(int argc, char **argv)
{
	unsigned int kick_period = DEFAULT_KICK_PERIOD;
	unsigned int timeout = 0;
	char *device = DEFAULT_WDT_DEVICE;

	for (; argc > 1; argc--, argv++) {
		if (!strcmp(argv[1], "--kick-rate")
					|| !strcmp(argv[1], "-k")) {
			argc--;
			argv++;
			kick_period = (unsigned int)atoi(argv[1]);
		}

		else if (!strcmp(argv[1], "--timeout")
					|| !strcmp(argv[1], "-t")) {
			argc--;
			argv++;
			timeout = (unsigned int)atoi(argv[1]);
		}

		else if (!strcmp(argv[1], "--device")
					|| !strcmp(argv[1], "-d")) {
			argc--;
			argv++;
			device = argv[1];
		}

		else if (!strcmp(argv[1], "--help")
					|| !strcmp(argv[1], "-h")) {
			show_usage();
			return 0;
		}
		
		else {
			printf("Unknown option: %s.\n\n", argv[1]);
			show_usage();
			return 1;
		}
	}

	fd = open(device, O_RDWR);
	if (fd < 0) {
		perror("Unable to open watchdog dev file");
		return 1;
	}

	/* If the user specified the --timeout option, we supply the new
	 * value to the watchdog driver. */
	if (timeout && ioctl(fd, WDIOC_SETTIMEOUT, &timeout) < 0) {
		perror("Unable to set watchdog timeout");
		return 1;
	}

	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = close_and_quit;
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror("Sigaction");
		return 1;
	}
	
	for (;;) {
		ioctl(fd, WDIOC_KEEPALIVE, 0);
		sleep(kick_period);
	}

	/* Never reached */
	return 0;
}
