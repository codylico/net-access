
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * \brief Get a line of text from `stdin`.
 * \param[out] ptr string pointer to receive the text
 * \return zero on success, `-5` on empty array,
 *   `-2` on failed allocation
 */
static int na_getline(char** ptr);
/**
 * \brief Help text.
 */
static char const na_help_text[] =
  "Commands:\n"
  "Help................ print this help text\n"
  "Quit................ quit this program\n";

int na_getline(char** recv_ptr){
  char buf[64];
  char* long_ptr = NULL;
  char* fgets_res;
  size_t current_length = 0;
  int result = 0;
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
      } else {
        fputs("Unknown command.\n",stderr);
      }
    }
  } while (fgets_result != -5);
  return EXIT_SUCCESS;
}
