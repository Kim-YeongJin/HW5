#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/resource.h>
#include <signal.h>
#include <fcntl.h>
#include <utmp.h>
#include <time.h>
#include <string.h>

static int 	logsize, old_logsize;

void check_log(){

	FILE 	*wtmp, *cse;
	char	time[40];
	char	log_data[100];
	
	if((wtmp = fopen("/var/log/wtmp", "rb")) < 0){
		fprintf(stderr, "can't open wtmp");
		return;
	}

	fseek(wtmp, 0, SEEK_END);
	logsize = ftell(wtmp) / sizeof(struct utmp);

	struct 	utmp	log[logsize];

	fseek(wtmp, 0, SEEK_SET);	

	if(chdir("/home/kyj/tmp") < 0){
		fprintf(stderr, "can't change directory to /home/kyj/tmp");
		return;
	}

	if((cse = fopen("cse.txt", "a")) < 0){
		fprintf(stderr, "can't open cse.txt");
		return;
	}
		
	if(logsize != old_logsize){
		fread(&log, sizeof(struct utmp), logsize, wtmp);
		for(int i=old_logsize; i<logsize; i++) {
			time[0] = '\0';

			time_t t = (time_t) log[i].ut_tv.tv_sec;
			struct tm time_info = {0};
			time_info = *localtime(&t);
        		strftime(time, sizeof(struct tm), "%Y-%m-%d / %H-%M-%S", &time_info);

			if(log[i].ut_type == USER_PROCESS){
				sprintf(log_data, "%s LOGIN TIME : %s\n", log[i].ut_user, time);
				fwrite(log_data, strlen(log_data), 1, cse);
				syslog(LOG_INFO, log_data);
			}
			if(log[i].ut_type == DEAD_PROCESS){
				sprintf(log_data, "LOG OUT TIME : %s\n", time);
				fwrite(log_data, strlen(log_data), 1, cse);
				syslog(LOG_INFO, log_data);
			}
		}
		old_logsize = logsize;
	}

	fclose(wtmp);
	fclose(cse);

}


int main(void){

	pid_t pid, sid;

	pid = fork();
	if(pid < 0)
		exit(EXIT_FAILURE);
	if(pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	sid = setsid();

	if(sid < 0)
		exit(EXIT_FAILURE);

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

        openlog("201424423_YeongJin_Kim", LOG_CONS, LOG_DAEMON);



	while(1){
		check_log();
		sleep(5);
	}

	return 0;
}
