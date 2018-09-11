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

int stricmp(const char *s1, const char *s2)
{
  if (!s1) {
    if (s2 && *s2)
      return -1;
    return 0; /* null equals null or empty string */
  }
  if (!s2) {
    if (*s1)
      return 1;
    return 0; /* null equals empty string */
  }

  char c1 = *s1++;
  char c2 = *s2++;
  while (c1 && c2) {
    if (c1 != c2) {
      /* coerce alpha to upper case */
      if (c1 >= 'a' && c1 <= 'z')
        c1 = c1 - 0x20;
      if (c2 >= 'a' && c2 <= 'z')
        c2 = c2 - 0x20;

      if (c1 > c2)
        return 1;
      if (c2 > c1)
        return -1;
    }

    c1 = *s1++;
    c2 = *s2++;
  }

  /* if we got here, either c1 or c2 is a terminating null */
  if (c1 == c2) /* only if both are null */
    return 0; /* strings are equal */
  if (c1)
    return 1; /* s1 is longer, therefore greater */
  return -1;
}
