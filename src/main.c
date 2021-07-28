#include <stdio.h>
#include <string.h>

// file listing
#include <sys/types.h>
#include <dirent.h>

// mkdir
#include <sys/stat.h>



#define BUFFER_SIZE 4096
#define PATH_SIZE 256

char destination[PATH_SIZE];

int create_dir(char *path){
    struct stat st = {0};
    if (stat(path, &st) == -1)
    	mkdir(path, 0755);
    else{
    	printf("\"%s\" exists! Aborting...\n", );
    	return 1;
    }
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
	char buff[BUFFER_SIZE];
	static char out_buff[BUFFER_SIZE];

	FILE *fp = fopen("template/index.html", "r");


	// fgets(buff, 1024, fp);
	fread(buff, BUFFER_SIZE, 1, fp);

	// buff[BUFFER_SIZE-1] = 0x00;
	

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




	snprintf(out_buff, BUFFER_SIZE, buff, "title", img(images, count));

	return(out_buff);

}

int main(int argc, char **argv){
	char title[] = "title"

	strcat(destination, title);
	create_dir(destination);
	
	char *out_buff = html_ready(title);


	// printf("%s\n", out_buff);

	return 0;
}
