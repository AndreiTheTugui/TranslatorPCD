#import "stlvector.h"
#import "stlstring.h"

int ns__upload(struct soap *soap, char *fileName, char *fileContent, char **response);
int ns__download(struct soap *soap, char *fileName, char **fileContent);
