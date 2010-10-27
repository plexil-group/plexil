File: $SVNROOT/examples/multi-exec/agents/tests/t5-readme.txt
Date: Tue Oct 26 19:53:40 2010

In t5-sim.pli, if the "command" node is not wrapped in something
(e.g., command-node or action, possibly others), the translation
process appears to throw it away.  

% cd plexil/examples/multi-exec/agents/test
% make
% xterm ipc &
% xterm -v -p t5-sim.plx -c sim.xml
% plexilexec -v -b -p t5-cmd.plx -c cmd.xml

or

% cd plexil/examples/multi-exec/agents/test
% run-agents t5-sim t5-cmd [-v [-p]]

Specifically,

    (on-message "Quit"
      (action "Foo"
        (command "pprint" "Quitting")))))

translates (in part) to

                        <NodeBody>
                           <NodeList>
                              <Node NodeType="Command">
                                 <NodeId>Foo</NodeId>
                                 <NodeBody>
                                    <Command>
                                       <Name>
                                          <StringValue>pprint</StringValue>
                                       </Name>
                                       <Arguments>
                                          <StringValue>Quitting</StringValue>
                                       </Arguments>
                                    </Command>
                                 </NodeBody>
                              </Node>
                           </NodeList>
                        </NodeBody>

and prints "Quitting" as expected, but

    (on-message "Quit"
      (command "pprint" "Quitting"))

translates the same stuff into just

                       <NodeBody/>

which obviously doesn't print "Quitting".

As I read the Plexilisp Reference Manual, it seems reasonable to use
the latter idiom:

   (on-message message [ action ] )
