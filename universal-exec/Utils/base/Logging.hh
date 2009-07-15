#ifndef LOGGING_HH
#define LOGGING_HH

class Logging {
	public:
		enum LogType {
			ERROR             = 0,  
			WARNING           = 1,  
                        INFO              = 2,
		};

		static int ENABLE_LOGGING;
		static int ENABLE_E_PROMPT;
		static int ENABLE_W_PROMPT;
		static int NEW_LOG_SESSION;
		static char * LOG_TIME;
		static char * FILE_NAME;

		static void set_log_file_name(char * file);
                static void print_to_log(const char * fullmsg);
		static int handle_message(int msg_type, const char * file, int line, const char * msg);
	private:
		static int print_error(char * fullmsg);
		static int print_warning(char * fullmsg);
		static void print_unknown(char * fullmsg);		
		static void prompt_user();
		static void print_stack();
		static char* get_msg_type(int msg);
                static void set_date_time();
};

#endif /* _LOGGING_HH */





