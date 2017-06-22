#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"
#include "xv6/fcntl.h"
#include "xv6/fs.h"

#define bufsize 128

int NP_fname(char *file_path, int len) {
	int result = len;
	while (result) {
		if (file_path[result - 1] == '/') {
			return result;
		}
		result--;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf(2, "Less or more arguments!\n");
		exit();
	}
	int srcfp, dstfp, srclen, dstlen, buflen;
	char srcfname[bufsize];
	char dstfname[bufsize];
	char buffer[bufsize];
	strcpy(srcfname, argv[1]);
	srclen = strlen(srcfname);
	strcpy(dstfname, argv[2]);
	dstlen = strlen(dstfname);
	if ((srcfp = open(srcfname, O_RDONLY)) < 0) {
		printf(2, "ERROR: cannot open source file!\n");
		close(srcfp);
		exit();
	}
	struct stat srcstat;
	fstat(srcfp, &srcstat);
	if (srcstat.type == T_DIR) {
		printf(2, "ERROR: only files can be moved!\n");
		close(srcfp);
		exit();
	}
	if (dstfname[dstlen - 1] != '/') {
		printf(2, "ERROR: the destination must be a dictionary end by '/'\n");
		exit();
	}
	strcpy(&(dstfname[dstlen]), &(srcfname[NP_fname(srcfname, srclen)]));
	if ((dstfp = open(dstfname, O_RDONLY)) >= 0) {
		printf(2, "ERROR: destination has been used!\n");
		close(srcfp);
		close(dstfp);
		exit();
	}
	if ((dstfp = open(dstfname, O_WRONLY | O_CREATE)) < 0) {
		printf(2, "ERROR: cannot open the destination file!\n");
		close(srcfp);
		close(dstfp);
		exit();
	}
	while((buflen = read(srcfp, buffer, bufsize)) > 0) {
		write(dstfp, buffer, buflen);
	}
	close(srcfp);
	close(dstfp);
	exit();
}
