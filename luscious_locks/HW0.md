# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs241.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".
```C
#include <unistd.h>
int main() {
	write(1, "Hi! My name is Ziye Tang\n", 25);
	return 0;
}
```

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```
   
   ```C
   void write_triangle(int n) {
	   int i;
	   for(i = 1; i<=n; i++) {
		   int j;
		   for(j = 1; j<=i; j++) {
			   write(STDERR_FILENO, "*",1);
		   }
		   write(STDERR_FILENO, "\n", 1);
	   }
   }
   ```


   
   
### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).
   
   ```C
   #include <fcntl.h>
   #include <sys/stat.h>
   #include <sys/types.h>
   #include <unistd.h>
   int main() {
	int filedes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	write(filedes, "Hello, World!", 13);
	close(filedes);
	return 0;
   }
   ```


### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!
   ```C
   int main() {
	close(1);
	int filedes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
	printf("Hello, World!");
	close(filedes);
	return 0;
   }
   ```

5. What are some differences between `write()` and `printf()`?

   One difference is that `printf()` write to stdout, and only after closing stdout can we use `printf()` to write to other files. `write()` can write to any file with a valid 
   file name I give to it. 
   Another difference is that, we do not need to give the length of string in `printf()`.


## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?	
   
   `At least 8 bits.`


2. How many bytes are there in a `char`?   
   
   `1 byte.`


3. How many bytes the following are on your machine? 
   - `int`, `double`, `float`, `long`, and `long long`
   
   
   `int`: 4  
   
   `double`: 8
   
   `float`: 4
   
   `long`: 8
   
   `long long`: 8


### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?


   `0x7fbd9d50`


5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?


   `*(data + 3)`



### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
   ```C
   char *ptr = "hello";
   *ptr = 'J';
   ```
Because the memory pointed by "ptr" is only valid for reading but not for writing.

7. What does `sizeof("Hello\0World")` return?	
   
   `12`


8. What does `strlen("Hello\0World")` return?	
   
   `5`


9. Give an example of X such that `sizeof(X)` is 3.   
   
   `"OH"`


10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.  
   
    `*int` 

## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?

   
   The first way is to simply check argc.
   Also the length is equal to the number of strings you type in the terminal (separated by space) plus one.
   

2. What does `argv[0]` represent?   
   
   `./program`
   
### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

   they are stored on the stack

### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?

   `sizeof(ptr)` = 8
   
   `sizeof(array)` = 5
   
   `sizeof(ptr)` is the the size of a pointer. But although array can be used as a pointer, when we use sizeof(array) it means the size of this whole array of character. And   
   since   there are totally 5 characters in "Hello", `sizeof(array)` is equal to 5.

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?	
  
   `stack`

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?

   I can put the data into heap by using malloc(), and I can assign how many byte I need inside malloc.

2. What are the differences between heap and stack memory? 

   Data allocation and deallocation are done automatically in stack by compiler instruction, which is done by the programmer in Heap.

3. Are there other kinds of memory in a process?   
   
   static, environment.
   
4. Fill in the blank: "In a good C program, for every malloc, there is a _free__".
### Heap allocation gotchas
5. What is one reason `malloc` can fail?  

   There is not enough space to allocate memory.

6. What are some differences between `time()` and `ctime()`? 

   `time()` return a `time_t` variable, but `ctime()` return an `array` of character.

7. What is wrong with this code snippet? 
```C
free(ptr);
free(ptr);
```

    It repeatedly free the same memory.
  
8. What is wrong with this code snippet? 
```C
free(ptr);
printf("%s\n", ptr);
```

    It is trying to get access to a freed memory.

9. How can one avoid the previous two mistakes?  
   
   By doing `ptr = NULL"`after `free(ptr)`.
### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).
```C
struct Person {
	char* name;
	int age;
	struct Person* (*friends)[];
};

typedef struct Person person_t;
```
11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.
```C
person_t* p1 = malloc(12);
person_t* p2 = malloc(12);
p1 -> name = "Agent Smith";
p1 -> age = 128;  
p2 -> name = "Sonny Moore";
p2 -> age = 256;  
*(*(p1 -> friends)) = p2;
*(*(p2 -> friends)) = p1;
```
### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).
```C
person_t* create(char* aname, int aage) {
	person_t* p = malloc(12);
	p -> name = strdup(aname);
	p -> age = aage;
	return p;
}
```
13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.
```C
void destroy(person_t *p) {
	free(p->name);
	free(p->friends);
	memset(p, 0, sizeof(12));
	free(p);
}
```
## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?                                                                           

   `getchar()` for getting characters and putchar() for writing characters.

2. Name one issue with `gets()`.

   If the input gives more characters than decleared, then contents of other variables might be corrupted.

### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".
```C
char* data = "Hello 5 World";
char hello[10];
char world[10];
int num;
sscanf(data, "%s %d %s", hello, &num, world);
```
### `getline` is useful
4. What does one need to define before including `getline()`?

   we need to define a string (a pointer to a character) we want to hold the data in the line and we also need to define the size of the string.

5. Write a C program to print out the content of a file line-by-line using `getline()`.
```C
int main() {
	FILE* fp = fopen("data.txt", "r");
	char* buffer = NULL;
	size_t capacity = 0;
	
	while(getline(&buffer, &capacity, fp) != -1) {
		printf("%s",buffer);
		
	}
	free(buffer);
	return 0;
	
}
```

## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?   
   
   `gcc -g` 

2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.

   instead of doing `make`, we should do `make debug`

3. Are tabs or spaces used to indent the commands after the rule in a Makefile?

   Tab is used at the beginning of command.

4. What does `git commit` do? What's a `sha` in the context of git?

   The `git commi`t command captures a snapshot of the project's currently staged changes. `SHA` is short for Simple Hashing Algorithm, which takes some data as input and
   generate a unique 40-character string. SHA is useful for identification and integrity checking of all file objects and commits.

5. What does `git log` show you?  
   
   It shows all my commit history.
   
7. What does `git status` tell you and how would the contents of `.gitignore` change its output?

   It shows the current branch I am on and if there are any changes I made that are still not added, commited, or pushed. When I create a new file in my local repo, git status
   would show me that it is a untracke file and needs to be added. But if I put the file name in .gitignore, then it would not be shown in untracked file anymore.

7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?

   The git push command is used to upload local repository content to a remote repository. The commit message doesn't show bugs in what files are fixed.

8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?

   It means git cannot commit your changes to the remote repository. I can do "git fetch origin" and then "git merge MY_BRANCH_NAME" to fix the error. 
   Another way is to do "git pull origin MY_BRANCH_NAME".

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
