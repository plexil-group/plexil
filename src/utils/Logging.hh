/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
 *  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Universities Space Research Association nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
                static void print_to_log(char** run_command, int num);                
		static int handle_message(int msg_type, const char * file, int line, const char * msg);
	private:
		static int print_error(char * fullmsg);
		static int print_warning(char * fullmsg);
		static void print_unknown(char * fullmsg);	
		static void prompt_user();
#ifdef PLATFORM_HAS_EXECINFO_H
		static void print_stack();
#endif
		static char* get_msg_type(int msg);
                static void set_date_time();
};

#endif /* _LOGGING_HH */





