#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define PATH_MAX_LEN 256
#define PATH_PREFIX "/sys/class/gpio/gpio"
#define PATH_DIRECTION_SUFFIX "/direction"
#define PATH_VALUE_SUFFIX "/value"

enum GpioDirection {Output, Input};

int gpio_pin_export(char* pin_number) {
	int fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd == -1) {
		perror("Unable to open /sys/class/gpio/export");
		return -1;
	}

	if (write(fd, pin_number, 2) != 2) {
		perror("Error writing to /sys/class/gpio/export");
		return -2;
	}

	close(fd);
	return 0;
}

int gpio_pin_unexport(char* pin_number) {
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd == -1) {
		perror("Unable to open /sys/class/gpio/unexport");
		exit(1);
	}

	if (write(fd, pin_number, 2) != 2) {
		perror("Error writing to /sys/class/gpio/unexport");
		exit(1);
	}

	close(fd);

}

int gpio_pin_direction(char* pin_number, enum GpioDirection direction) {
	char* gpio_path_string = calloc(PATH_MAX_LEN, sizeof(char*));
	
	strncat(gpio_path_string, PATH_PREFIX, strlen(PATH_PREFIX));
	strncat(gpio_path_string, pin_number, strlen(pin_number));
	strncat(gpio_path_string, PATH_DIRECTION_SUFFIX, strlen(PATH_DIRECTION_SUFFIX));

	int fd = open(gpio_path_string, O_WRONLY);

	free(gpio_path_string);

	if (fd == -1) {
		perror("Unable to open gpio direction file");
		return -1;
	}

	char* direction_string = calloc(4, sizeof(char*));
	switch(direction) {
		case Output:
			direction_string = "out";
			break;
		case Input:
		default:
			direction_string = "in";
	}

	if (write(fd, direction_string, 3) != 3) {
		perror("Error writing to gpio direction file");
		return -2;
	}

	close(fd);
	return 0;
}

int gpio_pin_value_fd(char* pin_number, enum GpioDirection direction) {
	char* gpio_path_string = calloc(PATH_MAX_LEN, sizeof(char*));
	
	strncat(gpio_path_string, PATH_PREFIX, strlen(PATH_PREFIX));
	strncat(gpio_path_string, pin_number, strlen(pin_number));
	strncat(gpio_path_string, PATH_VALUE_SUFFIX, strlen(PATH_VALUE_SUFFIX));
	
	int fd = -1; 
	switch(direction) {
		case Output:
			fd = open(gpio_path_string, O_WRONLY);
			break;
		case Input:
		default:
			fd = open(gpio_path_string, O_RDONLY);
	}
	
	free(gpio_path_string);
	
	if (fd == -1) {
		perror("Unable to open gpio value file");
		return -1;
	}
	return fd;
}
int read_file_value(int fd) {
	lseek(fd, 0, SEEK_SET);
	char buffer[32] = {0};
	size_t br = read(fd, buffer, 32);
	return atoi(buffer);
}
int main(int argc, char* argv[]) {
	gpio_pin_export("22");
	gpio_pin_export("23");
	gpio_pin_export("24");
	
	gpio_pin_direction("22", Input);
	gpio_pin_direction("23", Output);
	gpio_pin_direction("24", Output);

	int fd_btn = gpio_pin_value_fd("22", Input); 
	int fd_led1 = gpio_pin_value_fd("24", Output); 
	int fd_led2  = gpio_pin_value_fd("23", Output);
	char buff[10] = { 0 };
	if (write(fd_led1, "1", 1) != 1) {
		perror("Error writing to fd_led1 value");
		exit(1);
	}
	if (write(fd_led2, "0", 1) != 1) {
		perror("Error writing to fd_led2 value");
		exit(1);
	}

	int prev_value = 0;
	while (1) {
		usleep(100000);
		int value = read_file_value(fd_btn);
		if (value == 1 && prev_value == 0) {
			if (write(fd_led2, "1", 1) != 1) {
				perror("Error writing to fd_led2 value");
				exit(1);
			}

			char* cmd = "/usr/bin/camera_handler";

			FILE *pipe = popen(cmd, "r");

			if (!pipe) {
				return -1;
			};

			pclose(pipe);

			if (write(fd_led2, "0", 1) != 1) {
				perror("Error writing to fd_led2 value");
				exit(1);
			}
		
		}
		prev_value = value;
	}

	close(fd_btn);
	close(fd_led1);
	close(fd_led2);

	gpio_pin_unexport("22");
	gpio_pin_unexport("23");
	gpio_pin_unexport("24");

	return 0;
}
