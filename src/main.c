#include <stdio.h>
#include <string.h>

// file listing
#include <sys/types.h>
// #include <dirent.h>

// mkdir
#include <sys/stat.h>
#include <errno.h>

// exit() ,write, close
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
// #include <time.h>	// clock
// using namespace std;



// #define BUFFER_SIZE 4096
#define PATH_SIZE 256

int img_name_size;
int	doc_img_size;
char	*doc_html,
			*doc_img,
			archive[PATH_SIZE],
			destination[PATH_SIZE],
			title[PATH_SIZE];

int set_title(char *path){
	size_t count = 0;
	title[count] = 0x00;
	for(size_t i = 0; (path[i] != 0x00) && (path[i] != 0x2e); i++){
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
		fprintf(stderr, "error file name");
		return 0;
	}
	return 1;
}

int create_dir(char *path){
  if (!mkdir(path, 0755))return 1;
  if(errno == EEXIST)
		fprintf(stderr, "\"%s\" exists! Aborting...\n", path);
  return 0;
}

// https://stackoverflow.com/questions/10195343/
// copy-a-file-in-a-sane-safe-and-efficient-way
int cp(char *from, char *to) {
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
		fprintf(stderr, "can't open: %s", from);
		return 0;
	}
	if(!dest){
		fprintf(stderr, "can't write: %s", to);
		return 0;
	}

  // clean and more secure
  // feof(FILE* stream) returns non-zero if the end of file indicator for stream is set

  while ((size = fread(buf, 1, BUFSIZ, source))) {
  	fwrite(buf, 1, size, dest);
  }

  fclose(source);
  fclose(dest);
  return 1;
}



void img(const char *image){
	char tmp_buff[img_name_size];
	snprintf(tmp_buff, img_name_size, "<img src=\"images/%s\"/>\n", image);
	strcat(doc_img, tmp_buff);
}

int html_ready(){
	char	*tmp_buff,
				tmp_path[PATH_SIZE];

	FILE *fp = fopen("template/index.html", "r");
	if(!fp){
		fprintf(stderr, "missing template/index.html");
		return 0;
	}
 
	// Get the number of bytes,
	// stat size might also work
	fseek(fp, 0L, SEEK_END);
	long numbytes = ftell(fp);
	 
	// reset seek
	// fseek(fp, 0L, SEEK_SET);
	rewind(fp);	// works same
	 
	// +1 maybe for the null byte
	tmp_buff = (char*)calloc(numbytes+1, sizeof(char));
	 
	// memory error
	if(tmp_buff == NULL){
		fprintf(stderr, "not sufficient memory");
		return 0;
	}

	// fgets(buff, 1024, fp);

	// copy all the text into the buffer
	fread(tmp_buff, sizeof(char), numbytes, fp);
	fclose(fp);

	numbytes = numbytes + doc_img_size;
	doc_html = (char*)calloc(numbytes, sizeof(char));
	if(doc_html == NULL){
		fprintf(stderr, "not sufficient memory");
		return 0;
	}
	snprintf(doc_html, numbytes, tmp_buff, title, doc_img);

	free(tmp_buff);
	free(doc_img);


	// html write
	strcpy(tmp_path, destination);
	strcat(tmp_path, "index.html");

	fp = fopen(tmp_path, "w");
	fwrite(doc_html, sizeof(char), strlen(doc_html), fp);
	fclose(fp);
	free(doc_html);
	return 1;
}


int unzip(){
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
    return 0;
  }

  // when the zip is empty
  int index_size = zip_get_num_entries(za, 0);
  if(!index_size){
  	fprintf(stderr, "the archive is empty");
  	if (zip_close(za) == -1)
  		fprintf(stderr, "Can't close archive: %s\n", archive);
  	return 0;
  }
  const char *tmp_char = zip_get_name(za, 0, 0);
  if(tmp_char == NULL)return 0;

  // size of each img element
  img_name_size = 30 + strlen(tmp_char);

  //size of all img elements
  doc_img_size = img_name_size * index_size;
	doc_img = (char*)calloc(doc_img_size, sizeof(char));

	if(doc_img == NULL){
		fprintf(stderr, "not sufficient memory");
		return 0;
	}

  if(!create_dir(destination)) return 0;
  if(!create_dir(images_path)) return 0;

	for (int i = 0; i < index_size; i++) {
  	if (zip_stat_index(za, i, 0, &sb)) {
  		printf("File[%s] Line[%d]\n", __FILE__, __LINE__);
  		continue;
  	}
    if (sb.name[strlen(sb.name)-1] == 0x2f){
    	strcpy(tmp_path, images_path);
      strcat(tmp_path, sb.name);
      if(!create_dir(tmp_path)) return 0;
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

    // should also work with fopen, fwrite, fclose
    // it's taking different header
    int fd = open(tmp_path, O_RDWR | O_TRUNC | O_CREAT, 0644);
    if(fd < 0){
    	fprintf(stderr, "can't create file: %s\n",tmp_path);
      continue;
    }

    // write the file
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
    zip_fclose(zf);	// zip file close
  }

  // will the archive break, if not close?
  // (should be opened as read-only)?
  if (zip_close(za) == -1)
  	fprintf(stderr, "Can't close archive: %s\n", archive);
  return 1;
}

void worker(char *archive_input, char *destination_input){
	// clock_t start, end;
	// start = clock();

  strcpy(archive, archive_input);	// set archive path

  strcpy(destination, destination_input);
  size_t i = strlen(destination);
  if(destination[i-1]!=0x2f)	// add '/' at the end,
  	destination[i++] = 0x2f;	// if not present
  destination[i] = 0x00;	// add '\0' (null byte)

  if(!set_title(archive_input)) return;	// set title
	strcat(destination, title); // set destination
	i = strlen(destination);
	destination[i++] = 0x2f;	// add '/' at the end
	destination[i] = 0x00;	// add '\0' (null byte)

	// unzip all the images
	if(!unzip()) return;
	
	// create the html
	if(!html_ready()) return;

	// copy rest of the files
	if(!cp("template/bootstrap.bundle.min.js", "bootstrap.bundle.min.js"))return;
	if(!cp("template/bootstrap.bundle.min.js.map", "bootstrap.bundle.min.js.map"))return;
	if(!cp("template/bootstrap.min.css", "bootstrap.min.css"))return;
	if(!cp("template/bootstrap.min.css.map", "bootstrap.min.css.map"))return;
	if(!cp("template/jquery.js","jquery.js"))return;
	if(!cp("template/script.js", "script.js"))return;
	if(!cp("template/stylesheet.css", "stylesheet.css"))return;

  printf("done: %s\n", title);

	// end = clock();

  // printf("CLOCKS_PER_SEC: %ld\n", CLOCKS_PER_SEC);
  // printf("CPU-TIME END - START: %ld\n", end - start);
  // printf("TIME(SEC): %ld\n", (end - start) / CLOCKS_PER_SEC);
  // printf("---------------------------\n");
}

int main(int argc, char **argv){
	// clock_t start, end;
	// start = clock();

  if (argc < 3) {
    fprintf(stderr, "usage: %s archive destination\n", argv[0]);
    return 1;
  }

  for (size_t i = 1; i < argc-1; i++){
  	worker(argv[i], argv[argc-1]);
  }
/*
	end = clock();

	printf("Result:\n");
  printf("CLOCKS_PER_SEC: %ld\n", CLOCKS_PER_SEC);
  printf("CPU-TIME START: %ld\n", start);
  printf("CPU-TIME END: %ld\n", end);
  // printf("CPU-TIME END - START: %ld\n", end - start);
  printf("TIME(SEC): %ld\n", (end - start) / CLOCKS_PER_SEC);
*/
	return 0;
}
