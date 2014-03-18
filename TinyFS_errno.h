#ifndef TINYFSERRNO
#define TINYFSERRNO

#define FSACCES -1 		//permission denied
#define FSEXIST	-2		//file already exists
#define FSBADFD	-3		//bad file descriptor
#define FSBIG	-4		//file is too big
#define FSISDIR	-5		//file is a directory
#define FSMFILE	-6		//too many open files
#define NODISKMEM -7	//no more disk memory
#define FILENOTWRIT -8	//file has not been written to yet
#define EOFREACH -9		//reached the end of the file
#define FILEOPEN -10	//file already opened
#define DISKNOREAD -11	//disk could not be read
#define DISKNOCREAT -12	//disk could not be created
#define INCOMPWRIT -13	//could not write file contents
#define INCOMPREAD -14	//could not read file contents
#define MAXDISKS -15	//can not mount anymore disks
#define DISKCORRUPT -16	//disk is corrupted
#define NODISK -17		//cannot find disk
#define FILENAMETOOLONG -18 //file name more than 8 chars
#define PLACEHOLDER -19
#endif


/*more erros
file already open
*/