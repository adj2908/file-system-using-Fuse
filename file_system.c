//gcc new.c -o new `pkg-config fuse --cflags --libs`
//mkdir mount
//./new -f mount

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#define SIZE 10
#define STR_SIZE 100

typedef struct file_content
{
	char *file_name;
	char *file_content;
}file_content;

typedef struct file_table
{
	file_content **table;
	int index;
	int size;
}file_table;

typedef struct directory
{
	char *directory_name;
	file_table *data;
}directory;

typedef struct directory_table
{
	directory **table;
	int index;
	int size;
}directory_table;

file_table *files;
directory_table *directories;


file_content *init_file() // initializing and allocation of memory
{
	file_content *temp = (file_content *)malloc(sizeof(file_content));
	temp -> file_name = (char *)malloc(STR_SIZE * sizeof(char));
	temp -> file_content = (char *)malloc(STR_SIZE * sizeof(char));

	return temp;
}

file_table *init_table() // initializing and allocation of memory
{
	int i = 0;
	file_table *temp = (file_table *)malloc(sizeof(file_table));
	temp -> table = (file_content **)malloc(SIZE * sizeof(file_content *));
	temp -> index = 0;
	temp -> size = SIZE;
	for(i = 0;i < SIZE;i++)
	{
		temp -> table[i] = init_file();
	}
	return temp;
}

directory *init_directory() // initializing and allocation of memory
{
	directory *temp = (directory *)malloc(sizeof(directory));
	temp -> directory_name = (char *)malloc(STR_SIZE * sizeof(char));
	temp -> data = init_table();
	return temp;
}

directory_table *init_directory_table() // initializing and allocation of memory
{
	directory_table *temp = (directory_table *)malloc(sizeof(directory_table));
	temp -> table = (directory **)malloc(SIZE * sizeof(directory *));
	temp -> index = 0;
	temp -> size = SIZE;
	int i = 0;
	for(i = 0;i < SIZE;i++)
	{
		temp -> table[i] = init_directory();
	}
	return temp;	
}

int strcpy_2(char *a, char *b)  // strcpy own version
{
	int n = strlen(b);
	if(a == NULL)
	{
		a = (char *)malloc(STR_SIZE * sizeof(char));
	}
	printf("my strcopy called for string : %s of len : %d \n", b, n);
	int i = 0;
	for(i = 0;i < n;i++)
	{
		a[i] = b[i];
	}
	a[i] = '\0';
	return 0;
}

int strncpy_2(char *a, char *b, int n) 
{
	if(a == NULL)
	{
		a = (char *)malloc(STR_SIZE * sizeof(char));
	}
	if(strlen(b) < n)
	{
		return -1;
	}
	printf("my strncopy called for string : %s of len : %d \n", b, n);
	int i = 0;
	for(i = 0;i < n;i++)
	{
		a[i] = b[i];
	}
	a[i] = '\0';	
}

void concatenate(char *a, char *b) // own version of string concatenation
{
	int i = strlen(a);
	int j = strlen(b);
	int index = 0;
	for(index = 0;index < j;index++)
	{
		a[i + index] = b[index];
	}
}

void insert_file(file_table *files, char *filename, char *content) // insert file into the file table
{
	printf("\n%s : %s\n", filename, content);
	if(files -> index >= files -> size)// if the table is full then we increase the size of the table
	{
		printf("HAVE TO INCREASE SIZE OF FILE TABLE\n");
		int size = 2 * files -> size; 
		files -> table = (file_content **)realloc(files, size * sizeof(file_content *)); 
		files -> size = size;
	}
	strcpy_2(files -> table[files->index] -> file_name, filename);
	strcpy_2(files -> table[files->index] -> file_content, content);
	printf("\ninsert_file done : %d : %s : %s\n", files->index, files -> table[files->index] -> file_name, files -> table[files->index] -> file_content);
	files -> index++;
}

void insert_directory(directory_table *temp, char *directory_name) // insert directory into the directory table
{
	if(temp -> index >= temp -> size)
	{
		printf("HAVE TO INCREASE SIZE OF DIRECTORY TABLE\n");
		int size = 2 * temp -> size; 
		temp -> table = (directory **)realloc(temp, size * sizeof(directory *)); 
		temp -> size = size;
	}
	printf("directory_name : %s\n", directory_name);
	strcpy_2(temp -> table[temp -> index] -> directory_name, directory_name);
	printf("insert done of directory : %s\n", temp -> table[temp -> index] -> directory_name);
	temp -> index++;
}

void delete(file_table *files, int index) // removing the entry from the file table
{
	int i = index;
	for(i = index;i < files -> index;i++)
	{
		files -> table[i] = files -> table[i + 1];
	}
	files -> index--;
}

void delete_dir(directory_table *temp,int index)
{
	int i= index;
	for(i=index;i<temp ->index;i++)
	{
		temp -> table[i] = temp -> table[i + 1];
	}
	temp -> index--;
}
 
static int do_getattr( const char *path, struct stat *st )
{
	printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );
	
	// GNU's definitions of the attributes (http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
	// 		st_uid: 	The user ID of the file’s owner.
	//		st_gid: 	The group ID of the file.
	//		st_atime: 	This is the last access time for the file.
	//		st_mtime: 	This is the time of the last modification to the contents of the file.
	//		st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	//		st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon 
	//						as no process still holds it open. Symbolic links are not counted in the total.
	//		st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
	
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	//st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	//st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	char *name = (char *)malloc(STR_SIZE * sizeof(char));
	char *file = (char *)malloc(STR_SIZE * sizeof(char));
	strcpy_2(name, (char *)path);
	strcpy_2(file, (char *)path);
	file = file + 1;
	int i = 0;
	printf("do_getattr : name : %s\n", name);
	for(i = 0;i < directories -> index;i++)
	{
		printf("directory comparision of %s with %s\n\n", name, directories -> table[i] -> directory_name);
		if ( strcmp(name, directories -> table[i] -> directory_name ) == 0)
		{
			st->st_mode = S_IFDIR | 0755;
			st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
			return 0;
		}
	}
	for(i = 0;i < files -> index;i++)
	{
		printf("file comparision of %s with %s\n\n", file, files -> table[i] -> file_name);
		if ( strcmp(file, files -> table[i] -> file_name) == 0)
		{
			st->st_mode = S_IFREG | 0777;
			st->st_nlink = 1;
			st->st_size = 2048;
			return 0;
		}
	}
	if(strcmp(path, "") == 0)
	{
		return 0;
	}
	for(i = 0;i < directories -> index;i++)
	{
		if ( strcmp(name, directories -> table[i] -> directory_name ) == 0)
		{
			printf("directory found : %s\n", directories -> table[i] -> directory_name);
			return 0;
		}
	}
	for(i = 0;i < files -> index;i++)
	{
		if ( strcmp(file, files -> table[i] -> file_name) == 0)
		{
			printf("file found : %s\n", files -> table[i] -> file_name);
			return 0;
		}
	}

	
	return -ENOENT;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	printf( "--> Getting The List of Files of %s\n", path );
	
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	char *temp = (char *)malloc(STR_SIZE * sizeof(char));
	strcpy_2(temp, (char *)path);
	temp = temp + 1;
	int n = strlen(temp);
	printf("\nDO READ DIR : path : %s && temp : %s\n", path, temp);
	int i = 0;	
	if(strcmp(path, "/") == 0)
	{
		for(i = 0;i < files -> index;i++)
		{
			printf("\nfiller : %s : %s\n", files -> table[i] -> file_name, files -> table[i] -> file_content);
			filler( buffer, files -> table[i] -> file_name, NULL, 0 );
		}
		for(i = 0;i < directories -> index;i++)
		{
			printf("filler : directory : %s\n", directories -> table[i] -> directory_name);
			filler( buffer, directories -> table[i] -> directory_name, NULL, 0 );
		}
	}
	else
	{
		char *directory_name = (char *)malloc(STR_SIZE * sizeof(char));
		for(i = 0;i < files -> index;i++)
		{
			printf("file_name : %s\n", files -> table[i] -> file_name);
			int flag = strncpy_2(directory_name, files -> table[i] -> file_name, n);
			printf("directory_name : %s\n", directory_name);
			if(flag != -1 && (strcmp(directory_name, temp) == 0))
			{
				printf("\nfiller : %s : %s\n", files -> table[i] -> file_name, files -> table[i] -> file_content);
				filler( buffer, files -> table[i] -> file_name, NULL, 0 );
			}

		}
		for(i = 0;i < directories -> index;i++)
		{
			printf("full directory_name : %s\n", directories -> table[i] -> directory_name);
			int flag = strncpy_2(directory_name, directories -> table[i] -> directory_name, n + 1);
			printf("directory_name : %s\n", directory_name);
			if(flag != -1 && (strcmp(directory_name, temp) == 0))
			{
				printf("filler : directory : %s\n", directories -> table[i] -> directory_name);
				filler( buffer, directories -> table[i] -> directory_name, NULL, 0 );
			}
		}
	}
	return 0;
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	path = path + 1;
	printf( "--> Trying to read %s, %lu, %lu\n", path, offset, size );
	char selectedText[100];
	int i = 0;	
	for(i = 0;i < files -> index;i++)
	{
		char *a = (char *)malloc((SIZE + 1) * sizeof(char));
		a = files -> table[i] -> file_name;
		printf("\nREAD : path : %s : a : %s\n", path, a);
		if(strcmp(path, a) == 0)
		{
			printf("MATCH FOUND\n");
			strcpy_2(selectedText, files -> table[i] -> file_content);
			break;
		}
	}
	if(selectedText == NULL)
	{ 
		return -1;
	}

	printf("selectedText : %s\n", selectedText);
	memcpy( buffer, selectedText + offset, size );
	printf("memcpy done\n");	
	return strlen( selectedText ) - offset;
}

static int do_truncate(const char *path, off_t size)
{
	return size;
}

static int do_mkdir(const char* path, mode_t mode)
{
	//path = path + 1;
	printf("\nmake directory was called with name : %s\n", path);
	insert_directory(directories, (char *)path);
	return 0;
}


static int do_write(const char *path, const char *buffer, size_t size, off_t offset,struct fuse_file_info *fi)
{
	path = path + 1;
	char selectedText[100];
	printf("write : path : %s : new buffer : %s\n", path, selectedText);
	strcpy_2(selectedText, (char *)buffer);
	int i = 0;	
	for(i = 0;i < files -> index;i++)
	{
		char *a = (char *)malloc((STR_SIZE + 1) * sizeof(char));
		//strcat(a, "/");
		strcpy_2(a, files -> table[i] -> file_name);
		printf("WRITE : path : %s && a : %s\n\n", path, a);
		if(strcmp(path, a) == 0)
		{
			delete(files, i);
			//strcpy_2(files -> table[i] -> file_content, selectedText);
			printf("\ndeleted old data\n");
			break;
		}
	}
	insert_file(files, (char *)path, (char *)buffer);
	printf("OUTSIDE LOOP string written : %s\n", files -> table[files -> index - 1] -> file_content);
	return size;
}

static int do_open(const char *path, struct fuse_file_info *info)
{
	path = path + 1;
	printf( "--> Trying to open %s\n", path);
	char selectedText[100];
	int i = 0;	
	for(i = 0;i < files -> index;i++)
	{
		char *a = (char *)malloc((SIZE + 1) * sizeof(char));
		a = files -> table[i] -> file_name;
//		concatenate(a, files -> table[i] -> file_name);
		printf("\npath : %s : a : %s\n", path, a);
		if(strcmp(path, a) == 0)
		{
//			strcpy_2(selectedText, files -> table[i] -> file_content);
			break;
		}
	}
	if(i == files -> index)
	{ 
		return -ENOENT;
	}

	if ((info->flags & O_ACCMODE) != O_RDONLY)
	{
		return -EACCES;
	}
	return 0;
}

static int do_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	path = path + 1;
	insert_file(files, (char *)path, (char *)"");
	return 0;
}

static int do_remove(const char* path)
{
	path = path + 1;
	int i = 0;
	for(i = 0;i < files -> index;i++)
	{
		if(strcmp(path, files -> table[i] -> file_name) == 0)
		{
			break;
		}
	}
	if(i < files -> index)
	{
		delete(files, i);
	}
	else
	{
		return -1;
	}
	return 0;
}

static int do_rmdir(const char *path)
{
	path = path + 1;
	int i = 0;
	for(i = 0;i < directories -> index;i++)
	{
		if(strcmp(path, directories -> table[i] -> directory_name) == 0)
		{
			break;
		}
	}
	if(i < directories -> index)
	{
		delete_dir(directories, i);
	}
	else
	{
		return -1;
	}
	return 0;
} 

static int do_rename(const char *path, const char *new)
{
	printf("\n\n %s to %s\n\n", path, new);
	return 0;
}



static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read	= do_read,
    .write      = do_write,
    .truncate   = do_truncate,
    .mkdir      = do_mkdir,
    .rmdir	= do_rmdir,
    .create     = do_create,
    .unlink     = do_remove,
    .open       = do_open,
//  .rename     = do_rename,
};


int main( int argc, char *argv[] )
{
	files = init_table();
	directories = init_directory_table();
	insert_file(files, "f1", "hello f1");
	insert_file(files, "f2", "hello_ssup f2");
	insert_directory(directories, "/");
	insert_directory(directories, ".");
	insert_directory(directories, "/hello");
	return fuse_main( argc, argv, &operations, NULL );
}
