#include <stdio.h>
#include <string.h>

// file listing
#include <sys/types.h>
#include <dirent.h>

// mkdir
#include <sys/stat.h>
#include <errno.h>

// exit()
#include <unistd.h>

// malloc, free
#include <stdlib.h>

// libzip
#include <zip.h>



#define BUFFER_SIZE 4096
#define PATH_SIZE 256


typedef struct document{
	char html[BUFFER_SIZE], img[BUFFER_SIZE];
} document;


char destination[PATH_SIZE];

void freeArray(char **p, size_t n){
  for(size_t i = 0 ; i < n ; i++)
  	free(p[i]);
  free(p);
}

void create_dir(char *path){
  if (!mkdir(path, 0755))return;
  if(errno == EEXIST)
		printf("\"%s\" exists! Aborting...\n", path);
  exit(1);
}

char *img(char (*list)[PATH_SIZE], size_t number){
	
	static char out[BUFFER_SIZE];
	char tmp[PATH_SIZE];

	for(size_t i = 0; i < number; ++i){
		snprintf(tmp, PATH_SIZE, "<img src=\"%s\"/>\n", list[i]);
		strcat(out, tmp);
	}
	return out;
}

char *html_ready(char *title){
	document doc;
	static char out_buff[BUFFER_SIZE];

	FILE *fp = fopen("template/index.html", "r");


	// fgets(buff, 1024, fp);
	fread(doc.html, BUFFER_SIZE, 1, fp);
	fclose(fp);

	// buff[BUFFER_SIZE-1] = 0x00;
	
	printf("doc.html: [%s]\n", doc.html);
	size_t count = 0;

	DIR *dir = opendir("template/images");
	struct dirent *files;
	while((files = readdir(dir))){
  	char *f_name = files->d_name;

		// (f_name[0] != 0x2e)
  	if((files->d_type == DT_REG) && 
  		((strstr(f_name, "jpg"))||(strstr(f_name, "png"))))
  		count++;
  }

  char images[count][PATH_SIZE];
  size_t i = 0;
  
	rewinddir(dir);
	while((files = readdir(dir))){
  	char *f_name = files->d_name;

  	if((files->d_type == DT_REG) && 
  		((strstr(f_name, "jpg"))||(strstr(f_name, "png"))))
  		strcpy(images[i++], f_name);
  }

	closedir(dir);




	snprintf(out_buff, BUFFER_SIZE, doc.html, title, img(images, count));

	return(out_buff);

}


void unzip(char zip_file){
	// char buf[BUFFER_SIZE];	// buffer
	// struct zip *za;					// zip archive
	// struct zip_file *zf;		// zip file
	// struct zip_stat sb;			// zip file stat
	// int err;								// errors
	// int i, len;
	// int fd;
	// long long sum;

/*	if ((za = zip_open(zip_file, 0, &err)) == NULL) {
  	zip_error_to_str(buf, sizeof(buf), err, errno);
    fprintf(stderr, "can't open zip archive `%s': %s\n", argv[1], buf);
    exit(EXIT_FAILURE);
  }
*/

	return;
}

int main(int argc, char **argv){
  if (argc != 3) {
    fprintf(stderr, "usage: %s archive destination\n", argv[0]);
    return 1;
  }




  strcpy(destination, argv[2]);
  size_t i = strlen(destination);
  if(destination[i-1]!=0x2f)
  	destination[i] = 0x2f;

  char title[PATH_SIZE];
	for(i = 0; (argv[1][i] != 0x00) && (argv[1][i] != 0x2e); i++)
		title[i] = argv[1][i];
	title[++i] = 0x00;

	strcat(destination, title);
	// create_dir(destination);

	// printf("dest: %s", destination);
	
	char *out_buff = html_ready(title);




	printf("%s\n", out_buff);

	return 0;
}
