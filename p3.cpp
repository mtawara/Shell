//Ryan Nguyen & Mark Tawara
//ryan.nguyen25@uga.edu - mht35915@uga.edu
//lab13

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>

#include <unistd.h>

#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include <fcntl.h>
#include <sys/stat.h>


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::cin;
using std::getline;
using std::vector;
using std::stringstream;

vector< vector<string> > omega;
vector< vector<string> > pipevect;

struct job{
	string status = "";
	int groupID = 0;
	string name = "";
};

vector<job> j;

bool pipelining = false;
bool omegapipe = false;
bool inredir_t = false;
bool outredir_t = false;
bool outredir_a = false;
bool errredir_t = false;
bool errredir_a = false;

bool redirection = false;

struct process{ //vector used for storing arguments
	vector<string> names;
};

string readinput(){
	string strinput = "";
	getline(cin, strinput);
	return strinput;
}

int executable(char ** args){
	pid_t pid;
	pid_t wpid;
	int status;

	pid = fork();
	if(pid == 0){ //in child process

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
//		signal(SIGTSTP,SIG_DFL); <--- broken (stops all other signal handling)
		signal(SIGTTIN,SIG_DFL);
		signal(SIGTTOU,SIG_DFL);
		signal(SIGCHLD,SIG_DFL);
		if(execvp(args[0],args) == -1){
			perror("cp");
		}
		exit(EXIT_FAILURE);
	}else if(pid < 0){

		perror("ef");
	}else{ //in parent process

		do{ //wait for status change from child
			wpid = waitpid(pid, &status, WUNTRACED);
			if(wpid == -1){

			}
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1; //once child process terminates, return 1 to reset loop for next prompt
}

int jobs(vector<job> j){
	cout << "JID\t STATUS\t COMMAND" << endl;
	for(unsigned int i=0;i<j.size();i++){
		cout << j[i].groupID << "\t " << j[i].status << "\t " << j[i].name << endl;
	}
	return 1;
}

void pipeline(vector< vector<string> > a){

/*	unsigned int i = 0;
	int size = a[i].size()+1;	
	char ** args = new char *[size]; //new char ** variable created to store every argument from string
	for(i = 0; i < a.size(); ++i){ //each element copied by index
		args[i] = new char[a[i].size()];
		for(unsignd int j = 0; j < a[i].size(); ++j){
			strcpy(args[i], a[i][j].c_str());
		}
		strcpy(args[i], a[i].c_str());
	}
	
	args[i] = '\0'; //last index must be null terminating
*/

		
}

void close_pipe(int pipefd[2]){
	if (close(pipefd[0]) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	} // if
	if(close(pipefd[1]) == -1) {
		perror("close");
		exit(EXIT_FAILURE);
	} // if	
}

void fpipeline(vector< vector<string> > a){
	
	int pipefd[2];
	pid_t cpid;
//	int status;

	//FIRST PIPE EXECUTION
	if (pipe(pipefd) == -1) { 	//Create pipe
   		 perror("pipe");
		    exit(EXIT_FAILURE);
	  } // if

	if((cpid = fork()) == -1){	//Fork error
		perror("fork");
		exit(EXIT_FAILURE);	
	}else if(cpid == 0){	//Child is reading from the pipe
		if(dup2(pipefd[1], STDOUT_FILENO) == -1){
			perror("dup2");
			exit(EXIT_FAILURE);
		}
		close_pipe(pipefd);

		int size = a[0].size()+1;	
		char ** allArgsOfName1 = new char *[size];
		unsigned int i;
		for(i = 0; i < a[0].size(); ++i){ //each element copied by index
			allArgsOfName1[i] = new char[a[0][i].size()];
			strcpy(allArgsOfName1[i], a[0][i].c_str());
		}
		allArgsOfName1[i] = '\0'; //last index must be null terminating
		
		execvp(allArgsOfName1[0], allArgsOfName1);
	
		for(unsigned int i=0;i<a[0].size()+1;++i) delete[] allArgsOfName1[i];
		delete[] allArgsOfName1;
		exit(EXIT_FAILURE);
	}
	
	
	//SECOND PIPE EXECUTION
	if((cpid = fork()) == -1){
		perror("fork");
		exit(EXIT_FAILURE);
	}else if(cpid==0){
		if(dup2(pipefd[0], STDIN_FILENO) == -1){
			perror("dup2");
			exit(EXIT_FAILURE);
		}
		
		close_pipe(pipefd);
		//PUT NAME2 INTO SEPERATE ARGUMENTS
		unsigned int i = 0;
		int size = a[1].size()+1;	
		char ** allArgsOfName2 = new char *[size];
		for(i = 0; i < a[1].size(); ++i){ //each element copied by index
			allArgsOfName2[i] = new char[a[1][i].size()];
			strcpy(allArgsOfName2[i], a[1][i].c_str());
		}
		allArgsOfName2[i] = '\0'; //last index must be null terminating
		execvp(allArgsOfName2[0], allArgsOfName2);		
		
		for(unsigned int i=0;i<a[1].size()+1;++i) delete[] allArgsOfName2[i];
		delete[] allArgsOfName2;
		exit(EXIT_FAILURE);
	}
/*	else{ //in parent process

		do{ //wait for status change from child
			wpid = waitpid(cpid, &status, WUNTRACED);
			if(wpid == -1){

			}
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
*/	
	close_pipe(pipefd);

	//waitpid(cpid, nullptr, 0);

} // pipeline


void redirect(const char * filename, vector<string> a){
	
	int size = a.size()+1;	
	char ** args = new char *[size]; //new char ** variable created to store every argument from string
	unsigned int i;
	for(i = 0; i < a.size(); ++i){ //each element copied by index
		args[i] = new char[a[i].size()];
		strcpy(args[i], a[i].c_str());
	}
	
	args[i] = '\0'; //last index must be null terminating

	int dupe;
	int fd;
	
	
	pid_t pid;
	pid_t wpid;
	int status;

	pid = fork();
	if(pid == 0){ //in child process

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
//		signal(SIGTSTP,SIG_DFL); <--- broken (stops all other signal handling)
		signal(SIGTTIN,SIG_DFL);
		signal(SIGTTOU,SIG_DFL);
		signal(SIGCHLD,SIG_DFL);
	
		if(inredir_t){
			fd = open(filename, O_RDONLY, 0666);
			if((dupe = dup2(fd, STDIN_FILENO)) == -1){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
		}
		if(outredir_t){
			fd = open(filename, O_RDWR | O_TRUNC | O_CREAT, 0666);
			if((dupe = dup2(fd, STDOUT_FILENO)) == -1){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
		}
		if(outredir_a){
			fd = open(filename, O_RDWR | O_APPEND | O_CREAT, 0666);
			if((dupe = dup2(fd, STDOUT_FILENO)) == -1){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
		}
		if(errredir_t){
			fd = open(filename, O_RDWR | O_TRUNC | O_CREAT, 0666);
			if((dupe = dup2(fd, STDERR_FILENO)) == -1){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
		}
		if(errredir_a){
			fd = open(filename, O_RDWR | O_APPEND | O_CREAT, 0666);
			if((dupe = dup2(fd, STDERR_FILENO)) == -1){
				perror("dup2");
				exit(EXIT_FAILURE);
			}
		}
		
		if(execvp(args[0],args) == -1){
			perror("cp");
		}
		exit(EXIT_FAILURE);
	
	}else if(pid < 0){

		perror("ef");
	}else{ //in parent process

		do{ //wait for status change from child
			wpid = waitpid(pid, &status, WUNTRACED);
			if(wpid == -1){

			}
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	
	int cr = close(fd);
	if(cr == -1){

	}
//	int doit = executable(args);

	
}


char ** splitinput(const char * input){	
	omega.clear();
	vector<string> arguments;
//	vector< vector<string> > omega;
	stringstream ss;
	string substr;
	ss << input;
	
	const char * objfilename = nullptr;
	bool found = false;
	string quote;
	int location;

	while(ss>>substr){ //checks each argument by space delimiter
		if(substr == "|"){
			omegapipe = true;
			pipelining = true;
			omega.push_back(arguments);
//			if(pipelining){
//				//pipeline();
//				pipelining = false;
//			}else {
//				pipelining = true;
//			}
			pipevect.push_back(arguments);
			arguments.clear();
		
		}else if(found == true){
			if(substr.find("\\\"") != string::npos){
				quote = quote + " " + substr;
				while(quote.find("\\\"",0) != string::npos){
					location = quote.find("\\", 0);
					quote.erase(location,1);		
				}
			}else{
				quote = quote + " " + substr;
				found = false;
				quote.erase(quote.end()-1, quote.end());
				arguments.push_back(quote);
			}
				//If NONE of the Job flags are on, continue as normal.
		}else if(substr.find("\"") != string::npos){	// " was found
				if(substr.find("\\\"") != string::npos){	// \ was found
					quote = substr;
					quote.erase(substr.find("\\\""), substr.find("\\\"")+1);
				}else{
				quote = substr;
				quote.erase(0,1);	//Erases the "
				found = true;
				}
		
		}else if(substr == "<"){
			redirection = true;
			inredir_t = true;
		}else if(substr == ">"){
			redirection = true;
			outredir_t = true;
		}else if(substr == ">>"){
			redirection = true;
			outredir_a = true;
		}else if(substr == "e>"){
			redirection = true;
			errredir_t = true;
		}else if(substr == "e>>"){
			redirection = true;
			errredir_a = true;
		}else{ //for purposes of lab, only implements simple processes
//			files.push_back(substr); //put arg into vector
//			arguments.push_back(substr);
		
		//	if(pipelining){
				//pipeline();
		//		pipelining = false;
		//	}
			if(inredir_t){
				objfilename = substr.c_str();
				redirect(objfilename, arguments);
				inredir_t = false;
			}
			if(outredir_t){
				objfilename = substr.c_str();
				redirect(objfilename, arguments);
				outredir_t = false;
			}
			if(outredir_a){
				objfilename = substr.c_str();
				redirect(objfilename, arguments);
				outredir_a = false;
			}
			if(errredir_t){
				objfilename = substr.c_str();
				redirect(objfilename, arguments);
				errredir_t = false;
			}
			if(errredir_a){
				objfilename = substr.c_str();
				redirect(objfilename, arguments);
				errredir_a = false;
			}
		
			arguments.push_back(substr);
			
		}
	}

	omega.push_back(arguments);
	if(pipelining){
		pipevect.push_back(arguments);
		fpipeline(pipevect);
	}

//	int size = files.size()+1;
	int size = omega[0].size()+1;	
	char ** args = new char *[size]; //new char ** variable created to store every argument from string
	unsigned int i;
	for(i = 0; i < omega[0].size(); ++i){ //each element copied by index
		args[i] = new char[omega[0][i].size()];
		strcpy(args[i], omega[0][i].c_str());
	}

	args[i] = '\0'; //last index must be null terminating

	arguments.clear();
	return args;
	
}

/*int executable(char ** args){
	pid_t pid;
	pid_t wpid;
	int status;

	pid = fork();
	if(pid == 0){ //in child process

		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
//		signal(SIGTSTP,SIG_DFL); <--- broken (stops all other signal handling)
		signal(SIGTTIN,SIG_DFL);
		signal(SIGTTOU,SIG_DFL);
		signal(SIGCHLD,SIG_DFL);
		if(execvp(args[0],args) == -1){
			perror("cp");
		}
		exit(EXIT_FAILURE);
	}else if(pid < 0){

		perror("ef");
	}else{ //in parent process

		do{ //wait for status change from child
			wpid = waitpid(pid, &status, WUNTRACED);
			if(wpid == -1){

			}
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1; //once child process terminates, return 1 to reset loop for next prompt
}*/

int mycd(char ** args){
	//if shell does not have a second argument, prompt error
	if(args[1] == nullptr){
		cerr << "cp: destination required" << endl;
	}else{
		if(chdir(args[1]) != 0){
			perror("cp");
		}
	}
	return 1;
}

int myhelp(){
	//information about shell
	cout << "\nRyan's Bad Shell v1.0" << endl;
	cout << "Usage:\n" << endl;
	cout << "\tcd [PATH]\n\t/*Change current directory to PATH*/\n" << endl;
	cout << "\thelp\n\t/*Access the help manual*/\n" << endl;
	cout << "\texit\n\t/*Exit shell*/\n" << endl;
	cout << "Refer to 'man help' for more commands.\n" << endl;
	return 1;
}

int myexit(){
	//returning zero ends loop, program is terminated
	return 0;
}

int gogo(char ** args){
	//if no input, loop to prompt again
	if(args[0] == nullptr){
		return 1;
	}
	
	//checks which command was called
	if(strcmp(args[0], "cd") == 0){
		return mycd(args);
	}else if(strcmp(args[0], "help") == 0){
		return myhelp();
	}else if(strcmp(args[0], "exit") == 0){
		return myexit();
	}else if(strcmp(args[0], "jobs") == 0){
		return jobs(j);
	}

	//if not a built-in command, then run executable file
	if(!redirection && !omegapipe){
		return executable(args);
	}else{
		return 1;
	}
}

int main(int argc, char ** argv){
//	vector<job> j;
	j.push_back(job());
	j[0].groupID = getpgrp();
	
//char * cwd = nullptr;
//int cwdsize;
	const char * cinput;
	char ** args;
	int status;

	//ignore the following signals in shell (parent)
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);
	do{

	//print prompt
//cwd = getcwd(cwd, cwdsize);
//cout << "1730sh:" <<  cwd << "$" << endl;
	cout << "1730sh:" << get_current_dir_name() << "$ ";

	//read line
	string strinput = readinput();
	cinput = strinput.c_str();

	//split input into arguments
	args = splitinput(cinput);

	//execute process or built in
	status = gogo(args);
	
	redirection = false;
	pipelining = false;

	}while(status); //do while status is 1 (always 1 unless exit called)
	
	return EXIT_SUCCESS;

}
