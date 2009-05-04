/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

//
// This file contains a small example main() using the ExecApplication and related classes.
// It reads its interface configuration from an XML file
//


#include "ExecApplication.hh"
#include <iostream>
#include <vector>

void usage(const char* progname)
{
  std::cout << "Usage:\n "
            << progname << " [option]*\n"
            << "Supported options are:\n"
            << " -p <plan filename> (REQUIRED)\n"
            << " -c <config filename>\n"
            << " -d <debug-config filename>\n"
            << " -l <library filename> (multiple -l options are permitted)\n"
            << std::endl;
}

int main(int argc, char** argv)
{
  std::vector<const char*> libraryNames;
  const char* configFilename = NULL;
  const char* debugFilename = NULL;
  const char* planFilename = NULL;

  // if not enough parameters, print usage

  if (argc < 3)
    {
      usage(argv[0]);
      return -1;
    }

  // parse out parameters
   
  for (int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-c") == 0)
        {
          if (configFilename == NULL)
            {
              configFilename = argv[++i];
            }
          else
            {
              std::cout << "ERROR: multiple configuration files specified\n";
              usage(argv[0]);
              return -1;
            }
        }
      else if (strcmp(argv[i], "-d") == 0)
        {
          if (debugFilename == NULL)
            {
              debugFilename = argv[++i];
            }
          else
            {
              std::cout << "ERROR: multiple debug files specified\n";
              usage(argv[0]);
              return -1;
            }
        }
      else if (strcmp(argv[i], "-l") == 0)
        libraryNames.push_back(argv[++i]);
      else if (strcmp(argv[i], "-p") == 0)
        {
          if (planFilename == NULL)
            {
              planFilename = argv[++i];
            }
          else
            {
              std::cout << "ERROR: multiple plan files specified\n";
              usage(argv[0]);
              return -1;
            }
        }
      else
        {
          std::cout << "ERROR: unknown option \"" 
                    << argv[i] 
                    << "\"\n" ;
          usage(argv[0]);
          return -1;
        }
    }

  if (planFilename == NULL)
    {
      std::cout << "ERROR: no plan file specified\n";
      usage(argv[0]);
      return -1;
    }

  return 0;
}
