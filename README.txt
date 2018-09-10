Harsh Patel

This is a FAT12 parser. It takes in 2 arguments which are the disc image and the output
directory

DESIGN:
--------

The basic design of the program is that it calls the functions decodeFat and parseDirectory
which are pretty self explanitory. parseDirectory takes in the file pointer to the image
which should be preset to the start of the directory, and the current path name which is a
string (char*). The function checks upto 16 directory entries (1 sector) then returns 0
if there could be more directory entries (didnt find a '\0'), other wise returns 1. If
a subdirectory is found, then parseDirectory is called starting at that new directory and
with the updated path name. Directory entries are read into structures that are alligned to
retain the information such as filename, ext, size etc. After a file is found, the data is
retrieved using getData and then the file path, size and data are stored in a structure
called file_info_t which. All files are linked in a linked list and then printed out 
to stdout and the added to the directory provided.