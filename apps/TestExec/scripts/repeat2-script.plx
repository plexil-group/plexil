<PLEXILScript>
  <InitialState>

    <State name="x" type="real">
      <Value>0</Value>
    </State>
  </InitialState>

  <Script>

    <CommandAck name="foo" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>

    <State name="x" type="real">
      <Value>1</Value>
    </State>

    <CommandAck name="foo" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>


  </Script>
</PLEXILScript>
