// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

package plexil;

public class Diagnostic
{
    private Severity m_severity;
    private String m_message;
    private PlexilTreeNode m_location;
    private String m_file;

    public Diagnostic(String file, PlexilTreeNode location, String message, Severity severity)
    {
        m_file = file;
        m_location = location;
        m_message = message;
        m_severity = severity;
    }

    public String toString()
    {
        StringBuilder s = new StringBuilder(m_severity.toString());
        s.append(": ");
        if (m_file != null) {
            s.append(m_file);
            s.append(":");
        }
        if (m_location != null) {
            s.append(String.valueOf(m_location.getLine()));
            s.append(':');
            s.append(String.valueOf(m_location.getCharPositionInLine()));
        }
        if (m_file != null || m_location != null) {
            s.append(": ");
        }
        s.append(m_message);
        return s.toString();
    }

    public int severity() { return m_severity.rank; }

}
