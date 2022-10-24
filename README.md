# Farm project
__Request__: make a C program named _farm_ that implements the communication scheme between processes and threads shown below. 

![farm](https://user-images.githubusercontent.com/89905733/197409107-9e1dc194-35f8-4a9f-a1ef-dad46cf9ff55.png)


## Instructions
_farm_ is a program composed by two processes: they are named _MasterWorker_ and _Collector_.
_MasterWorker_ is a multi-threaded process composed by one _Master_ thread and 'n' (generic number) _Worker_ thread.
The input program is: a binary file list containing long numbers, and a certain number of optional arguments ('-n', '-q' and '-t').
_MasterWorker_ process generates the _Collector_ process. They communicate via a socket connection AF_LOCAL (AF_UNIX). Students can choose which one between _MasterWorker_ and _Collector_ is the master process for socket connection and also how many socket connections to use (one or more). The socket file "farm.sck" used for AF_LOCAL connection must be created in the project's directory and it must be deleted when program execution ends. <br />

### Components
* _MasterWorker_ process reads function _main_ arguments one by one and checks if they are regular files, then gives each file name (possibly with parameters) to one of Worker threads by using a shared concurrent queue. _Worker_ thread must read his input file, calculate a value using file's content, and send the result with the file name to _Collector_ process by the previously established socket connection. <br />

* _Collector_ process awaits results from _Worker_ threads and prints them on standard output:
<pre>
  result1   filename1
  result2   filename2
  result3   filename3
</pre>

* Each _Worker_ calculates: <br />
![Cattura](https://user-images.githubusercontent.com/89905733/197415269-f24d26c2-e594-46b6-a2a6-ae72e2c17418.PNG) <br />
where N is the number of long numbers in file. <br />

### Optional arguments
_MasterWorker_ input could also include three optional arguments: add <br />
* '-n' to choose how many _Worker_ thread to create by _MasterWorker_
* '-q' to choose queue length
* '-t' to choose a delay between requests from _Master_ thread to _Worker_ threads (default value: 0)

### Signal handling
_MasterWorker_ process must handle SIGHUP, SIGINT, SIGQUIT and SIGTERM. When one of these signals is received, the process must complete all tasks in the queue, then awaits the _Collector_ process ending and deletes the socket file.
_Collector_ process masks the _MasterWorker_ process handled signals. SIGPIPE signal must be appropiately handled by both processess.

#### Note
File size is not a specified value. Assume that file name length is not more than 255 characters.

#### Material provided for the project
* a pdf file with these instruction in Italian 
* _generafile.c_: generates files
* _test.sh_: some tests that the program must pass

##### _This README.md is written and translated by me from the original pdf with project instructions._
