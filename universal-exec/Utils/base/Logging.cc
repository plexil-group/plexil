#include "Logging.hh"
#include "Error.hh"
#include <execinfo.h>
#include <iostream>
#include <fstream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int Logging::ENABLE_LOGGING = 0; // enable messages to log file
int Logging::ENABLE_E_PROMPT = 0; // enable error prompt messages
int Logging::ENABLE_W_PROMPT = 0; // enable warning prompt messages

char * Logging::LOG_TIME = "";
int Logging::NEW_LOG_SESSION = 1;
char * Logging::FILE_NAME = "";

int Logging::handle_message(int msg_type, const char * file, int line, const char * msg) {
        char fullmsg[1024];
        sprintf(fullmsg, "%s: %s:%i: %s", get_msg_type(msg_type), file, line, msg);

	switch (msg_type) {
	case ERROR:
		print_error(fullmsg);
		break;
	case WARNING:
	        print_warning(fullmsg);
		break;
        default:
		print_unknown(fullmsg);
		break;
	}

	return 0;
}

int Logging::print_error(char * fullmsg) {
	// write message to console
        Error::getStream() << fullmsg << "\n";

	// write message to logfile if enabled
	if (Logging::ENABLE_LOGGING)
		print_to_log(fullmsg);

	// prompt user
	if (Logging::ENABLE_E_PROMPT)
                prompt_user();

	return 0;
}

int Logging::print_warning(char * fullmsg) {
	// write message to console
        Error::getStream() << fullmsg << "\n";

	// write message to logfile if enabled
	if (Logging::ENABLE_LOGGING)
		print_to_log(fullmsg);

	if (Logging::ENABLE_W_PROMPT)
		prompt_user();

	return 0;
}

void Logging::print_unknown(char * fullmsg) {
	// write message to console
        Error::getStream() << fullmsg << "\n";

	if (Logging::ENABLE_LOGGING)
		print_to_log(fullmsg);
}

void Logging::print_to_log(const char * fullmsg) {
        Logging::set_date_time();
	std::streambuf *psbuf, *backup;
	std::ofstream filestr(Logging::FILE_NAME, std::ios::app);
	backup = std::cout.rdbuf();
	psbuf = filestr.rdbuf();
	std::cout.rdbuf(psbuf);

	if (NEW_LOG_SESSION) {
		NEW_LOG_SESSION = 0;
		std::cout << "================================================================================\n";
		std::cout << "Logging Session ID (PID): " << getpid() << "\n";
		std::cout << "================================================================================\n";
	}

	std::cout << Logging::LOG_TIME << ": " << fullmsg << "\n";

	std::cout.rdbuf(backup);
	filestr.close();
}

void Logging::prompt_user() {
	do	{
		char buf[16];
		Error::getStream() << " (pid:" << getpid()
		<< ") [E]xit, show [S]tack trace or [P]roceed: ";

		if (isatty(0) && isatty(1))
			fgets(buf, 8, stdin);
		else
			strcpy(buf, "E\n");

		if ((buf[0] == 'E' || buf[0] == 'e') && buf[1] == '\n') {
			Error::getStream() << "aborting...\n";
			exit(0);
		} else if ((buf[0] == 'P' || buf[0] == 'p') && buf[1] == '\n')
			return;
		else if ((buf[0] == 'S' || buf[0] == 's') && buf[1] == '\n') {
			print_stack();
		}

	} while (1);
}

void Logging::set_log_file_name(char * file) {
	if (file == NULL)
		file = "universalexec.log";
	Logging::FILE_NAME = file;
}

char* Logging::get_msg_type(int msg) {
	switch (msg) {
	case ERROR:
		return "ERROR";
	case WARNING:
		return "WARNING";
        default:
		return "UNKNOWN";
	}
}

void Logging::set_date_time() {
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	Logging::LOG_TIME = asctime(timeinfo);
	int i = 0;
	while (Logging::LOG_TIME[i++] != '\n')
		;
	Logging::LOG_TIME[i - 1] = '\0';
}

void Logging::print_stack() {
	void *trace[16];
	char **messages = (char **) NULL;
	int i, trace_size = 0;

	trace_size = backtrace(trace, 16);
	messages = backtrace_symbols(trace, trace_size);
	Error::getStream() << "Execution path:\n";
        if (Logging::ENABLE_LOGGING) print_to_log("Execution path:");
	for (i = 0; i < trace_size; ++i) {
		Error::getStream() << messages[i] << "\n";
                if (Logging::ENABLE_LOGGING) print_to_log(messages[i]);
	}

	free(messages);
}



