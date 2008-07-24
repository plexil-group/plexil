<PLEXILScript>
  <InitialState>
  </InitialState>
  <Script>

    <!-- int state value, to be sure i didn't break them -->

    <State name="stateInteger" type="int">
      <Value>100</Value>
    </State>

    <!-- Command returning array of bool -->

    <Command name="boolArrayCommand" type="bool-array">
      <Result>1</Result>
      <Result>1</Result>
      <Result>1</Result>
    </Command>

    <CommandAck name="boolArrayCommand" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>

    <!-- Function returning array of string -->
    
    <FunctionCall name="stringArrayFunction" type="string-array">
      <Result>new string 1</Result>
      <Result>new string 2</Result>
      <Result>new strign 3</Result>
    </FunctionCall>

  </Script>
</PLEXILScript>
