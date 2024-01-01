#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
	char* arg[3] = {"cgi/php-cgi", "cgi/test", NULL};   
	char* env[3] = {"REQUEST_METHOD=GET", "SCRIPT_NAME=public/html/index.html", NULL};
	int r = execve(arg[0], arg, env);
	perror("execve:");
	printf("%d\n", r);
    return 0;
}
