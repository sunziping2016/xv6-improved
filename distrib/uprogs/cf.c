#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fcntl.h"
#include "xv6/fs.h"

#define bufSize 128

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf(2, "Less or more arguments!\n");
		exit();
	}
	int tfp, buflen = 1;
	char tfname[bufSize];
	char buffer[bufSize] = "\0";
	strcpy(tfname, argv[1]);
	if ((tfp = open(tfname, O_RDONLY)) >= 0) {
		printf(2, "ERROR: file already exists!\n");
		close(tfp);
		exit();
	}
	if ((tfp = open(tfname, O_WRONLY | O_CREATE)) < 0) {
		printf(2, "ERROP: creating new file failed!\n");
		close(tfp);
		exit();
	}
	while (buflen > 0) {
		buflen = strlen(buffer);
		write(tfp, buffer, buflen);
		gets(buffer, bufSize);
		buflen = strlen(buffer);
	}
	close(tfp);
	exit();
}