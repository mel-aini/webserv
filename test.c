#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	// int	fd[2];
	// if (pipe(fd) == -1)
	// {
	// 	exit(EXIT_FAILURE);
	// }
	// pid_t	pid = fork();
	// if (pid == -1)
	// {
	// 	exit(EXIT_FAILURE);
	// }
	// if (pid == 0)
	// {
	// 	dup2(fd[1], 1);
	// 	close(fd[0]);
	// 	close(fd[1]);
		char* arg[2] = {"test.php", NULL};
		char* env[2] = {"REQUEST_METHOD=GET", NULL};
		execve("cgi/php-cgi", arg, env);
	// }
	// dup2(fd[0], 0);
	// close(fd[0]);
	// close(fd[1]);
	// wait(0);
    return 0;
}
