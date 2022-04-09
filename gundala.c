#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <curl/curl.h>

#define SRC             "https://pastebin.com/raw/eVjujdc1"
#define SRC_MD5         "6807828dbfab3dfa1301a0cbfc497cab"
#define TARGET_DIR      "./slot/"
#define TARGET_FILE     "./slot/index.html"

char * calc_md5(const char *filename) {
  unsigned char c[MD5_DIGEST_LENGTH];
  int i;
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[1024];
  char *filemd5 = (char*) malloc(33 *sizeof(char));

  FILE *fs = fopen (filename, "rb");
  if (fs == NULL) {
    perror(filename); // TODO: not exist ? restore
    exit(1);
  }
  
  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 1024, fs)) != 0)
  MD5_Update (&mdContext, data, bytes);
  MD5_Final (c,&mdContext);

  for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
    sprintf(&filemd5[i*2], "%02x", (unsigned int)c[i]);
  }

  fclose(fs);
  return filemd5;
}

int restore() {
  CURL* easyhandle = curl_easy_init();
  curl_easy_setopt(easyhandle, CURLOPT_URL, SRC);
  curl_easy_setopt(easyhandle, CURLOPT_HEADER, 0);

  FILE* file = fopen(TARGET_FILE, "w");
  curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, file);
  curl_easy_perform(easyhandle);
  curl_easy_cleanup(easyhandle);
  
  fclose(file);
  return 0;
}
 
int cekidot() {
  // TODO: fix recursive dir?
  /*
  if (access(TARGET_DIR, F_OK) != 0) {
    int r = mkdir(TARGET_DIR, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
    if (r == -1) {
       printf("error creating dir");
       exit(1);
    }
  }
  */

  if (access(TARGET_FILE, F_OK) != 0) {
    restore();
  }   
  return 0; 
}

int main() {
  int f;
  unsigned long fs;

  cekidot();

  char *new_md5 = calc_md5(TARGET_FILE);
  if (!strcmp(SRC_MD5, new_md5)) {
    return 0; // TODO: check config && resident?
  } else {
    restore();
  }
  free(new_md5);

  return 0;
}
