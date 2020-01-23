/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

int stricmp(const char *str1, const char *str2)
{
  if (!str1) {
    if (str2 && *str2)
      return -1;
    return 0; /* null equals null or empty string */
  }
  if (!str2) {
    if (*str1)
      return 1;
    return 0; /* null equals empty string */
  }

  char ch1 = *str1++;
  char ch2 = *str2++;
  while (ch1 && ch2) {
    if (ch1 != ch2) {
      /* coerce alpha to upper case */
      if (ch1 >= 'a' && ch1 <= 'z')
        ch1 = ch1 - 0x20;
      if (ch2 >= 'a' && ch2 <= 'z')
        ch2 = ch2 - 0x20;

      if (ch1 > ch2)
        return 1;
      if (ch2 > ch1)
        return -1;
    }

    ch1 = *str1++;
    ch2 = *str2++;
  }

  /* if we got here, either ch1 or ch2 is a terminating null */
  if (ch1 == ch2) /* only if both are null */
    return 0; /* strings are equal */
  if (ch1)
    return 1; /* str1 is longer, therefore greater */
  return -1;
}
