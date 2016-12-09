#include "ipHandling.h"

char* initIp (parsedURL* url) {
  if (url->host == NULL) {
    return NULL;
  }

  struct hostent* h;

  if ((h=gethostbyname(url->host)) == NULL) {
      herror("gethostbyname");
      return NULL;
  }

  url->ip = inet_ntoa(*((struct in_addr *)h->h_addr));

  if (url->ip != NULL) return url->ip;
  else return NULL;
}
