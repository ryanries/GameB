#include "miniz.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define OPERATION_NONE		0
#define OPERATION_ADD		1
#define OPERATION_EXTRACT	2

static bool FileExists(const char *FileName)
{
    struct stat file_stat;
    return stat(FileName, &file_stat) == 0 && S_ISREG(file_stat.st_mode);
}

static char *TrimPath(char *FileName)
{
	char *trimmed = FileName;
	for (char *i = FileName; *i; i++) {
		if (*i == '/') {
			trimmed = i + 1;
		}
	}
	return trimmed;
}

int main(int argc, char *argv[])
{
    char* ArchiveName = NULL;
	char* FileName = NULL;
	int Operation = OPERATION_NONE;

	if (argc != 4)
	{
		printf("Adds or extracts files from a compressed archive.\n");
		printf("Usage: myminiz.exe <archive> <[+|-]> <filename>\n");
		return 0;
	}

	ArchiveName = argv[1];
	FileName = argv[3];

	if (strcmp(argv[2], "+") == 0)
	{
		Operation = OPERATION_ADD;
	}
	else if (strcmp(argv[2], "-") == 0)
	{
		Operation = OPERATION_EXTRACT;
	}
	else
	{
		printf("Adds or extracts files from a compressed archive.\n");
		printf("Usage: myminiz.exe <archive> <[+|-]> <filename>\n");
		return(0);
	}

    if (Operation == OPERATION_ADD)
    {
        int fd = open(FileName, O_RDONLY);
        if (fd == -1)
        {
            printf("Error opening %s: %s\n", FileName, strerror(errno));
            return -1;
        }
        printf("[+] File %s opened for reading.\n", FileName);
        struct stat file_stat;
        if (fstat(fd, &file_stat) == -1)
        {
            perror("fstat failed");
            return -1;
        }
        void *FileBuffer = malloc(file_stat.st_size);
        if (FileBuffer == NULL)
        {
            puts("malloc failed");
            return -1;
        }
        ssize_t bytes_read = read(fd, FileBuffer, file_stat.st_size);
        close(fd);
        if (bytes_read != file_stat.st_size)
        {
            puts("ERROR: Bytes read into memory did not match file size!");
            return -1;
        }
		FileName = TrimPath(FileName);
		printf("[+] FileName: %s\n", FileName);
        if ((mz_zip_add_mem_to_archive_file_in_place(ArchiveName, FileName, FileBuffer, file_stat.st_size, "", 1, MZ_BEST_COMPRESSION)) == MZ_FALSE)
		{			
			printf("ERROR: Failed to add file %s to archive %s!\n", FileName, ArchiveName);
			return -1;
		}
        free(FileBuffer);
        printf("[+] File %s successfully added to archive %s.\n", FileName, ArchiveName);
    }
    else if (Operation == OPERATION_EXTRACT)
    {
        bool FileFoundInArchive = false;
        mz_zip_archive ZipArchive = { 0 };
        mz_zip_error MZError = MZ_ZIP_NO_ERROR;
        if (FileExists(ArchiveName) == false)
        {
            printf("ERROR: Archive %s does not exist!\n", ArchiveName);
            return -1;
        }
        if ((mz_zip_reader_init_file(&ZipArchive, ArchiveName, 0)) == MZ_FALSE)
		{
			MZError = mz_zip_get_last_error(&ZipArchive);
			printf("ERROR: mz_zip_reader_init_file failed with error code %d!\n", MZError);
			return(MZError);
		}

		printf("[-] Archive %s opened for reading.\n", ArchiveName);

        for (int FileIndex = 0; FileIndex < (int)mz_zip_reader_get_num_files(&ZipArchive); FileIndex++)
		{
			mz_zip_archive_file_stat CompressedFileStatistics = { 0 };			

			if (mz_zip_reader_file_stat(&ZipArchive, FileIndex, &CompressedFileStatistics) == MZ_FALSE)
			{
				MZError = mz_zip_get_last_error(&ZipArchive);
				printf("ERROR: mz_zip_reader_file_stat failed with error code %d!\n", MZError);
				return(MZError);
			}

			if (strcmp(CompressedFileStatistics.m_filename, FileName) == 0)
			{
				FileFoundInArchive = true;

				printf("[-] File %s found in archive %s.\n", FileName, ArchiveName);

				if ((mz_zip_reader_extract_to_file(&ZipArchive, FileIndex, FileName, 0)) == MZ_FALSE)
				{
					MZError = mz_zip_get_last_error(&ZipArchive);

					printf("ERROR: mz_zip_reader_extract_to_file failed with error code %d!\n", MZError);

					return(MZError);
				}
				else
				{
					printf("[-] Successfully extracted file %s.\n", FileName);
				}
				
				break;
			}
		}

        if (FileFoundInArchive == false)
		{
			printf("ERROR: File %s not found in archive %s!\n", FileName, ArchiveName);
		}
    }
    else
	{
		printf("ERROR: No operation specified!\n");
	}
    return 0;
}
