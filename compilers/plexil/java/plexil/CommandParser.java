/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

package plexil;

import java.io.File;
import java.io.PrintStream;

import java.util.Vector;

public class CommandParser
{
    private String[] m_argv;
    
    // Global command line options
    public boolean debug = false;
    public boolean epxOnly = false;
    public boolean indentOutput = false;
    public boolean semanticsOnly = false;
    public boolean syntaxOnly = false;
    public boolean writeEpx = false;

    // Source files
    public Vector<File> sourceFiles = new Vector<File>();

    // Optional: locations for output
    public File outputFile = null;
    public File outputDirectory = null;

    public CommandParser()
    {
    }

    //*
    // Parse the compiler's command line arguments.
    //
    // \param argv The input command line.
    //
    // \return Status to return from main().  0 means no errors
    // detected; 1 means there was an error processing the arguments;
    // 2 denotes a usage error.
    //
    public int parse(String[] argv)
    {
        m_argv = argv;

        int i = 0; // index of arg being processed

        // Parse options
        while (i < m_argv.length) {
            if (m_argv[i].equals("-h") || m_argv[i].equals("--help")) {
                // Print usage message and exit
                usage(System.out);
                return 0;
            }
            else if (m_argv[i].equals("-v") || m_argv[i].equals("--version")) {
                // Print version and exit
                System.out.println("PlexilCompiler version 2.0.0d1");
                return 0;
            }
            else if (m_argv[i].equals("-d") || m_argv[i].equals("--debug")) {
                debug = true;
            }
            else if (m_argv[i].equals("-e") || m_argv[i].equals("--epx-only")) {
                epxOnly = true;
                writeEpx = true;
            }
            else if (m_argv[i].equals("-w") || m_argv[i].equals("--write-epx")) {
                writeEpx = true;
            }
            else if (m_argv[i].equals("-o")) {
                if (outputFile != null) {
                    System.err.println("Error: -o option can only be used once");
                    return 2;
                }
                String outputFilename = m_argv[++i];
                try {
                    outputFile = new File(outputFilename);
                }
                catch (Throwable t) {
                    System.out.println("Error: unable to parse output file name \""
                                       + outputFilename + "\"");
                    return 1;
                }
            }
            else if (m_argv[i].equals("-O")) {
                if (outputDirectory != null) {
                    System.err.println("Error: -O option can only be used once");
                    return 2;
                }
                String outputDirectoryName = m_argv[++i];
                try {
                    outputDirectory = new File(outputDirectoryName);
                }
                catch (Throwable t) {
                    System.out.println("Error: unable to parse output directory name \""
                                       + outputDirectoryName + "\"");
                    return 1;
                }
            }
            else if (m_argv[i].equals("-p") || m_argv[i].equals("--pretty-print")) {
                indentOutput = true;
            }
            else if (m_argv[i].equals("-m") || m_argv[i].equals("--semantics-only")) {
                semanticsOnly = true;
            }
            else if (m_argv[i].equals("-s") || m_argv[i].equals("--syntax-only")) {
                syntaxOnly = true;
            }
            else {
                // Not a recognized option, go on to process input file name(s)
                break;
            }
            ++i;
        }

		if (i >= m_argv.length) {
			// No file name supplied
            System.err.println("Error: no source file(s) supplied");
			usage(System.err);
            return 2;
		}

        while (i < m_argv.length) {
            String sourceFilename = m_argv[i++];
            try {
                sourceFiles.add(new File(sourceFilename));
            }
            catch (Throwable t) {
                    System.out.println("Error: unable to parse source file name \""
                                       + sourceFilename + "\"");
                    return 1;
            }
        }

        // Error checking
        if (outputFile != null && sourceFiles.size() > 1) {
            System.err.println("Error: -o option cannot be used with multiple source file names");
            return 2;
        }
        if (outputDirectory != null) {
            try {
                if (!outputDirectory.isDirectory()) {
                    System.err.println("Error: " + outputDirectory.toString()
                                       + " is not a directory");
                    return 1;
                }
                if (!outputDirectory.canWrite()) {
                    System.err.println("Error: output directory " + outputDirectory.toString()
                                       + " is not writable");
                    return 1;
                }
            }
            catch (Throwable t) {
                System.err.println("Error: unable to check output directory "
                                   + outputDirectory.toString());
                return 1;
            }
        }
        for (File sourceFile : sourceFiles) {
            try {
                if (!sourceFile.exists()) {
                    System.err.println("Error: source file "
                                       + sourceFile.toString() + " does not exist");
                    return 1;
                }
                if (!sourceFile.canRead()) {
                    System.err.println("Error: unable to read source file "
                                       + sourceFile.toString());
                    return 1;
                }
            }
            catch (Throwable t) {
                System.err.println("Error: unable to check source file "
                                   + sourceFile.toString());
                return 1;
            }
        }

        return 0;
    }

    public void usage(PrintStream s)
    {
        s.println("Usage:  PlexilCompiler [options] [sourcefile]* ...");
        s.println(" Options for information (no sourcefile required): ");
        s.println("  -h, -help, --help        Prints this message and exits");
        s.println("  -v, -version, --version  Prints version number and exits");
        s.println(" Options for output control: ");
        s.println("  -O <dirname>             Writes output files to dirname (default: same as source)");
        s.println("  -o <filename>            Writes output to filename");
        s.println("  -p, --pretty-print       Format XML output for readability");
        s.println("  -w, --write-epx          Write an Extended Plexil XML output file");
        s.println(" Options primarily for compiler testing:");
        s.println("  -d, --debug              Enable debug output to standard-error stream");
        s.println("  -e, --epx-only           Write EPX file, but do not translate to Core Plexil");
        s.println("  -m, --semantics-only     Perform syntax and semantic checks, but do not generate output");
        s.println("  -s, --syntax-only        Perform surface syntax parsing only");
    }

    //*
    // Get the next input file.
    //
    // \return The next file to be processed, or null if all have been processed.
    //
    public File nextInputFile()
    {
        if (sourceFiles.isEmpty()) {
            return null;
        }
        try {
            File result = sourceFiles.firstElement();
            sourceFiles.remove(0);
            return result;
        }
        catch (Throwable t) {
            return null;
        }
    }

    private String replaceFileExtension(String fname, String ext)
    {
        String result = null;
        int dot = fname.lastIndexOf('.');
        if (dot >= 0) {
            result = fname.substring(0, dot);
        }
        else {
            result = fname;
        }
        return result.concat(".").concat(ext);
    }

    //*
    // Get the output filename given the input filename and extension.
    //
    // \param infile The input filename.
    //
    // \param ext The extension desired for the output file.
    //
    // \return The output filename.  Will be null if command line
    // options dictate no output file is created.
    //
    public File getOutputFile(File infile, String ext)
    {
        if (syntaxOnly || semanticsOnly) {
            return null;
        }
        if ("epx".equals(ext) && !writeEpx && !epxOnly) {
            return null;
        }
        File resultDirectory = null;
        String resultName = null;
        if (outputFile != null) {
            resultDirectory = outputFile.getParentFile();
            if ("epx".equals(ext)) {
                resultName = replaceFileExtension(outputFile.getName(), ext);
            }
            else {
                resultName = outputFile.getName();
            }
        }
        else {
            resultDirectory = infile.getParentFile();
            resultName = replaceFileExtension(infile.getName(), ext);
        }

        if (outputDirectory != null) {
            if (resultDirectory.isAbsolute()) {
                // Just replace the result directory
                resultDirectory = outputDirectory;
            }
            else {
                // Prepend the output directory on the result directory
                resultDirectory = new File(outputDirectory, resultDirectory.getPath());
            }
        }

        return new File(resultDirectory, resultName);
    }

}
