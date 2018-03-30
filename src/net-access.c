
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * \brief Get a line of text from `stdin`.
 * \param[out] ptr string pointer to receive the text
 * \return zero on success, `-5` on empty array,
 *   `-2` on failed allocation
 */
static int na_getline(char** ptr);
/**
 * \brief Check a process for termination.
 * \param[in,out] pid the ID of the process to check
 * \return nonzero if the process is still running,
 *   zero otherwise
 */
static int na_check_process(pid_t *pid);
/**
 * \brief Start a process.
 * \param[out] pid the ID of the process to start
 * \param argv program arguments
 * \return zero on fork success
 */
static int na_start_process(pid_t *pid, const char** argv);
/**
 * \brief Terminate a process.
 * \param[in,out] pid the ID of the process to terminate
 * \return nonzero if the process is still running,
 *   zero otherwise
 */
static int na_finish_process(pid_t *pid);
/**
 * \brief Help text.
 */
static char const na_help_text[] =
  "Commands:\n"
  "(h)  help ............... print this help text\n"
  "(q)  quit ............... quit this program\n"
  "(+w) +wpa_supplicant .... start wpa_supplicant\n"
  "(?w) ?wpa_supplicant .... check wpa_supplicant process status\n"
  "(-w) -wpa_supplicant .... terminate wpa_supplicant process\n"
  "(+g) +wpa_gui ........... start wpa_gui\n"
  "(?g) ?wpa_gui ........... check wpa_gui process status\n"
  "(-g) -wpa_gui ........... terminate wpa_gui process\n"
  "(+d) +dhclient .......... start dhclient to acquire and lease an IP address\n"
  "(~d) ~dhclient .......... start dhclient to release an IP address\n"
  "(?d) ?dhclient .......... check dhclient process status\n"
  "(-d) -dhclient .......... terminate dhclient process\n";
/**
 * \brief Process ID for `wpa_supplicant`.
 */
static pid_t supplicant_pid = -1;
/**
 * \brief Process ID for `wpa_gui`.
 */
static pid_t gui_pid = -1;
/**
 * \brief Process ID for `dhclient`.
 */
static pid_t dhclient_pid = -1;

int na_getline(char** recv_ptr){
  char buf[64];
  char* long_ptr = NULL;
  char* fgets_res;
  size_t current_length = 0;
  int result = 0;
  fprintf(stderr,"> ");
  fflush(NULL);
  while ((fgets_res = fgets(buf,64,stdin)) == buf){
    size_t len = strlen(buf);
    char* new_ptr = (char*)realloc(long_ptr,sizeof(char)*current_length+len+1);
    if (new_ptr != NULL){
      long_ptr = new_ptr;
      memcpy(long_ptr+current_length,buf,len);
      current_length += len;
      long_ptr[current_length] = 0;
    } else {
      free(long_ptr);
      long_ptr = NULL;
      result = -2;
      break;
    }
    if (len < 63) break;
  }
  if (long_ptr != NULL && current_length > 0){
    if (long_ptr[current_length-1] == '\n')
      long_ptr[current_length-1] = 0;
  }
  *recv_ptr = long_ptr;
  if (fgets_res == 0) result = -5;
  return result;
}
int na_check_process(pid_t* pid){
  if (*pid < 0) return 0;
  else {
    int wstatus;
    pid_t new_pid = waitpid(*pid,&wstatus,WNOHANG);
    if (new_pid == 0) return 1;
    else if (new_pid == *pid){
      if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)){
        if (WIFEXITED(wstatus)){
          fprintf(stderr,"Process %i exited with result %i.\n",
              (int)*pid, (int)WEXITSTATUS(wstatus));
        } else {
          fprintf(stderr,"Process %i terminated by signal %i.\n",
              (int)*pid, (int)WTERMSIG(wstatus));
        }
        *pid = -1;
        return 0;
      }
    }
  }
}
int na_start_process(pid_t *pid, const char** argv){
  pid_t cross_pid;
  if (*pid >= 0) return -3;
  cross_pid = fork();
  if (cross_pid == -1){
    int error_result = errno;
    fprintf(stderr,"Failed to fork child process:\n\t%s\n",
        strerror(error_result));
    return -6;
  } else if (cross_pid == 0){
    /* child process */
    int exec_result = execv(argv[0],(char *const*)argv);
    if (exec_result == -1){
      int error_result = errno;
      fprintf(stderr,"Failed to execute process:\n\t%s\n",
        strerror(error_result));
    }
    exit(EXIT_FAILURE);
  } else {
    /* parent process */
    *pid = cross_pid;
    fprintf(stderr,"Process %i has started.\n",(int)*pid);
    return 0;
  }
}
int na_finish_process(pid_t *pid){
  int kill_result;
  if (*pid < 0) return 0;
  kill_result = kill(*pid,SIGTERM);
  if (kill_result == -1){
    int error_result = errno;
    fprintf(stderr,"Failed to terminate process %i:\n\t%s\n",
      (int)*pid, strerror(error_result));
    return -1;
  } else {
    /* update status */
    int wstatus;
    pid_t new_pid = waitpid(*pid,&wstatus,0);
    if (new_pid == 0) return 1;
    else if (new_pid == *pid){
      if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)){
        if (WIFEXITED(wstatus)){
          fprintf(stderr,"Process %i exited with result %i.\n",
              (int)*pid, (int)WEXITSTATUS(wstatus));
        } else {
          fprintf(stderr,"Process %i terminated by signal %i.\n",
              (int)*pid, (int)WTERMSIG(wstatus));
        }
        *pid = -1;
        return 0;
      }
    }
  }
}

int main(int argc, char**argv){
  int fgets_result;
  do {
    char *line_string;
    fgets_result = na_getline(&line_string);
    if (line_string != NULL){
      /* process the command */
      if (line_string[0] == '#'){
        /* ignore */
      } else if (strcmp("q",line_string) == 0
      ||  strcmp("quit",line_string) == 0)
      {
        fprintf(stderr,"Quitting...\n");
        fgets_result = -5;
      } else if (strcmp("h",line_string) == 0
      ||  strcmp("?",line_string) == 0
      ||  strcmp("help",line_string) == 0)
      {
        fputs(na_help_text,stderr);
      } else if (strcmp("+wpa_supplicant",line_string) == 0
      ||  strcmp("+w",line_string) == 0)
      {
        int check_result = na_check_process(&supplicant_pid);
        if (check_result){
          fprintf(stderr,"wpa_supplicant may already be running.\n");
        } else {
          int run_result;
          char const* start_array[] = {
            "/usr/sbin/wpa_supplicant",
            "-iwlan0",
            "-c/etc/wpa_supplicant.conf",
            NULL
          };
          free(line_string);
          line_string = NULL;
          run_result = na_start_process(&supplicant_pid,start_array);
          if (run_result < 0){
            fprintf(stderr,"Failed to start wpa_supplicant.\n");
          }
        }
      } else if (strcmp("?wpa_supplicant",line_string) == 0
      ||  strcmp("?w",line_string) == 0)
      {
        int check_result = na_check_process(&supplicant_pid);
        if (check_result){
          fprintf(stderr,"wpa_supplicant is running.\n");
        } else {
          fprintf(stderr,"wpa_supplicant is not running.\n");
        }
      } else if (strcmp("-wpa_supplicant",line_string) == 0
      ||  strcmp("-w",line_string) == 0)
      {
        int finish_result = na_finish_process(&supplicant_pid);
        if (finish_result){
          fprintf(stderr,"wpa_supplicant is running.\n");
        } else {
          fprintf(stderr,"wpa_supplicant is not running.\n");
        }
      } else if (strcmp("+wpa_gui",line_string) == 0
      ||  strcmp("+g",line_string) == 0)
      {
        int check_result = na_check_process(&gui_pid);
        if (check_result){
          fprintf(stderr,"wpa_gui may already be running.\n");
        } else {
          int run_result;
          char const* start_array[] = {
            "/usr/bin/wpa_gui",
            NULL
          };
          free(line_string);
          line_string = NULL;
          run_result = na_start_process(&gui_pid,start_array);
          if (run_result < 0){
            fprintf(stderr,"Failed to start wpa_gui.\n");
          }
        }
      } else if (strcmp("?wpa_gui",line_string) == 0
      ||  strcmp("?g",line_string) == 0)
      {
        int check_result = na_check_process(&gui_pid);
        if (check_result){
          fprintf(stderr,"wpa_gui is running.\n");
        } else {
          fprintf(stderr,"wpa_gui is not running.\n");
        }
      } else if (strcmp("-wpa_gui",line_string) == 0
      ||  strcmp("-g",line_string) == 0)
      {
        int finish_result = na_finish_process(&gui_pid);
        if (finish_result){
          fprintf(stderr,"wpa_gui is running.\n");
        } else {
          fprintf(stderr,"wpa_gui is not running.\n");
        }
      } else if (strcmp("+dhclient",line_string) == 0
      ||  strcmp("+d",line_string) == 0)
      {
        int check_result = na_check_process(&dhclient_pid);
        if (check_result){
          fprintf(stderr,"dhclient may already be running.\n");
        } else {
          int run_result;
          char const* start_array[] = {
            "/sbin/dhclient",
            "-v",
            "wlan0",
            NULL
          };
          free(line_string);
          line_string = NULL;
          run_result = na_start_process(&dhclient_pid,start_array);
          if (run_result < 0){
            fprintf(stderr,"Failed to start dhclient.\n");
          }
        }
      } else if (strcmp("~dhclient",line_string) == 0
      ||  strcmp("~d",line_string) == 0)
      {
        int check_result = na_check_process(&dhclient_pid);
        if (check_result){
          fprintf(stderr,"dhclient may already be running.\n");
        } else {
          int run_result;
          char const* start_array[] = {
            "/sbin/dhclient",
            "-v",
            "-r",
            "wlan0",
            NULL
          };
          free(line_string);
          line_string = NULL;
          run_result = na_start_process(&dhclient_pid,start_array);
          if (run_result < 0){
            fprintf(stderr,"Failed to start dhclient.\n");
          }
        }
      } else if (strcmp("?dhclient",line_string) == 0
      ||  strcmp("?d",line_string) == 0)
      {
        int check_result = na_check_process(&dhclient_pid);
        if (check_result){
          fprintf(stderr,"dhclient is running.\n");
        } else {
          fprintf(stderr,"dhclient is not running.\n");
        }
      } else if (strcmp("-dhclient",line_string) == 0
      ||  strcmp("-d",line_string) == 0)
      {
        int finish_result = na_finish_process(&dhclient_pid);
        if (finish_result){
          fprintf(stderr,"dhclient is running.\n");
        } else {
          fprintf(stderr,"dhclient is not running.\n");
        }
      } else if (line_string[0] != 0){
        fputs("Unknown command.\n",stderr);
      }
      free(line_string);
    }
  } while (fgets_result != -5);
  na_finish_process(&dhclient_pid);
  na_finish_process(&gui_pid);
  na_finish_process(&supplicant_pid);
  return EXIT_SUCCESS;
}
