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

// copy file
#include <fcntl.h>

// copy file (2)
// #include <iostream>	// ??
// #include <cstdio>    // fopen, fclose, fread, fwrite, BUFSIZ
// #include <ctime>
// using namespace std;



// #define BUFFER_SIZE 4096
#define PATH_SIZE 256

long	doc_img_size;
char	*doc_html,
			*doc_img,
			archive[PATH_SIZE],
			destination[PATH_SIZE],
			title[PATH_SIZE];

void get_title(char *path){
	size_t count = 0;
	title[count] = 0x00;
	for(size_t i = 0; (path[i] != 0x00) && (path[i] != 0x2e); ++i){
		if(path[i] == 0x2f){
			count = 0;
			title[count] = 0x00;
			continue;
		}
		title[count++] = path[i];
	}
	if(count)
		title[count] = 0x00;
	else{
		printf("error file name");
		exit(EXIT_FAILURE);
	}
}

void create_dir(char *path){
  if (!mkdir(path, 0755))return;
  if(errno == EEXIST)
		printf("\"%s\" exists! Aborting...\n", path);
  exit(EXIT_FAILURE);
}

// https://stackoverflow.com/questions/10195343/
// copy-a-file-in-a-sane-safe-and-efficient-way
void cp(char *from, char *to) {
	char tmp_to[PATH_SIZE];
	strcpy(tmp_to, destination);
	strcat(tmp_to, to);
  // BUFSIZE default is 8192 bytes
  // BUFSIZE of 1 means one chareter at time
  // good values should fit to blocksize, like 1024 or 4096
  // higher values reduce number of system calls
  // size_t BUFFER_SIZE = 4096;

  char buf[BUFSIZ];
  size_t size;

  FILE* source = fopen(from, "rb");
  FILE* dest = fopen(tmp_to, "wb");

	if(!source){
		printf("can't open: %s", from);
		exit(EXIT_FAILURE);
	}
	if(!dest){
		printf("can't write: %s", to);
		exit(EXIT_FAILURE);
	}

  // clean and more secure
  // feof(FILE* stream) returns non-zero if the end of file indicator for stream is set

  while ((size = fread(buf, 1, BUFSIZ, source))) {
  	fwrite(buf, 1, size, dest);
  }

  fclose(source);
  fclose(dest);
}



void img(const char *image){
	char tmp_buff[30];
	snprintf(tmp_buff, 30, "<img src=\"images/%s\"/>\n", image);
	strcat(doc_img, tmp_buff);
}

void html_ready(){
	char	*tmp_buff,
				tmp_path[PATH_SIZE];

	FILE *fp = fopen("template/index.html", "r");
	if(!fp){
		printf("missing template/index.html");
		exit(EXIT_FAILURE);
	}
 
	// Get the number of bytes 
	fseek(fp, 0L, SEEK_END);
	long numbytes = ftell(fp);
	 
	// reset the file position indicator to 
	// the beginning of the file 
	fseek(fp, 0L, SEEK_SET);	
	 
	// grab sufficient memory for the
	// buffer to hold the text
	tmp_buff = (char*)calloc(numbytes, sizeof(char));
	 
	// memory error
	if(tmp_buff == NULL){
		printf("not sufficient memory");
		exit(EXIT_FAILURE);
	}
	 
	// copy all the text into the buffer
	fread(tmp_buff, sizeof(char), numbytes, fp);
	fclose(fp);
	 
	// confirm we have read the file by
	// outputing it to the console 
	// printf("The file called test.dat contains this text\n\n%s", tmp_buff);
	 



	// fgets(buff, 1024, fp);
	// fread(tmp_buff, BUFFER_SIZE, 1, fp);
	// fclose(fp);

	numbytes = numbytes + doc_img_size;
	doc_html = (char*)calloc(numbytes, sizeof(char));
	if(doc_html == NULL){
		printf("not sufficient memory");
		exit(EXIT_FAILURE);
	}
	snprintf(doc_html, numbytes, tmp_buff, title, doc_img);

	// free the memory we used for the buffer 
	free(tmp_buff);
	free(doc_img);


	// html write
	strcpy(tmp_path, destination);
	strcat(tmp_path, "index.html");

	fp = fopen(tmp_path, "w");
	fwrite(doc_html, sizeof(char), strlen(doc_html), fp);
	fclose(fp);
	free(doc_html);
}


void unzip(){
	char buf[BUFSIZ];	// buffer
	struct zip *za;					// zip archive
	struct zip_file *zf;		// zip file
	struct zip_stat sb;			// zip file stat
	int err;								// errors
	char images_path[PATH_SIZE];
	char tmp_path[PATH_SIZE];

	strcpy(images_path, destination);
	strcat(images_path, "images/");

	if ((za = zip_open(archive, 0, &err)) == NULL) {
  	zip_error_to_str(buf, sizeof(buf), err, errno);
    fprintf(stderr, "can't open archive: %s[%s]\n", archive, buf);
    exit(EXIT_FAILURE);
  }

  int index_size = zip_get_num_entries(za, 0);
  if(!index_size){
  	fprintf(stderr, "the archive is empty");
  	if (zip_close(za) == -1)
  		fprintf(stderr, "Can't close archive: %s\n", archive);
  	exit(EXIT_FAILURE);
  }
  doc_img_size = 30L * index_size;
	doc_img = (char*)calloc(doc_img_size, sizeof(char));

	if(doc_img == NULL){
		fprintf(stderr, "not sufficient memory");
		exit(EXIT_FAILURE);
	}

  create_dir(images_path);

	for (int i = 0; i < index_size; i++) {
  	if (zip_stat_index(za, i, 0, &sb)) {
  		printf("File[%s] Line[%d]\n", __FILE__, __LINE__);
  		continue;
  	}
    // printf("Name: [%s], ", sb.name);
    if (sb.name[strlen(sb.name)-1] == 0x2f){
    	strcpy(tmp_path, images_path);
      strcat(tmp_path, sb.name);
      // printf("path: %s", tmp_path);
      create_dir(tmp_path);
      continue;
    }

		img(sb.name);

    zf = zip_fopen_index(za, i, 0);
    if (!zf){
    	fprintf(stderr, "error opening compressed file: %s\n", sb.name);
      continue;
    }

    strcpy(tmp_path, images_path);
    strcat(tmp_path, sb.name);
    int fd = open(tmp_path, O_RDWR | O_TRUNC | O_CREAT, 0644);
    if(fd < 0){
    	fprintf(stderr, "can't create file: %s\n",tmp_path);
      continue;
    }

		int sum = 0;
		while (sum != sb.size) {
    	int len = zip_fread(zf, buf, BUFSIZ);
      if (len < 0){
      	fprintf(stderr, "error writing buffer: %s\n", sb.name);
        continue;
      }
      write(fd, buf, len);
      sum += len;
    }
    close(fd);
    zip_fclose(zf);
  }

  if (zip_close(za) == -1)
  	fprintf(stderr, "Can't close archive: %s\n", archive);

}

int main(int argc, char **argv){
  if (argc != 3) {
    fprintf(stderr, "usage: %s archive destination\n", argv[0]);
    return 1;
  }

  strcpy(archive, argv[1]);	// set archive path

  strcpy(destination, argv[2]);
  size_t i = strlen(destination);
  if(destination[i-1]!=0x2f)
  	destination[i++] = 0x2f;
  destination[i] = 0x00;

  get_title(argv[1]);	// set title
	strcat(destination, title); // set destination
	i = strlen(destination);
	destination[i++] = 0x2f;
	destination[i] = 0x00;

	create_dir(destination);
	unzip();
	
	html_ready();

	// copy rest of the files
	cp("template/bootstrap.bundle.min.js", "bootstrap.bundle.min.js");
	cp("template/bootstrap.bundle.min.js.map", "bootstrap.bundle.min.js.map");
	cp("template/bootstrap.min.css", "bootstrap.min.css");
	cp("template/bootstrap.min.css.map", "bootstrap.min.css.map");
	cp("template/jquery.js","jquery.js");
	cp("template/script.js", "script.js");
	cp("template/stylesheet.css", "stylesheet.css");


	// printf("%s\n", doc_html);

	return 0;
}
