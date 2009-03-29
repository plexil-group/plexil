<PLEXILScript>
  <InitialState>
    <State name="car_started" type="bool">
      <Value>false</Value>
    </State>
    <State name="raining" type="bool">
      <Value>true</Value>
    </State>
    <State name="no_passengers" type="bool">
      <Value>true</Value>
    </State>
    <State name="at_school" type="bool">
      <Value>false</Value>
    </State>
    <State name="time" type="real">
      <Value>0.0</Value>
    </State>
  </InitialState>
  <Script>
    <CommandAck name="depress_accelerator" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="depress_accelerator" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="depress_accelerator" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="depress_clutch" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="turn_key" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <State name="car_started" type="bool">
      <Value>true</Value>
    </State>
    <CommandAck name="turn_on_wipers" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="turn_on_lights" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="select_radio" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="drive_a_bit" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="drive_a_bit" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <CommandAck name="drive_a_bit" type="string">
      <Result>COMMAND_SUCCESS</Result>
    </CommandAck>
    <State name="at_school" type="bool">
      <Value>true</Value>
    </State>
  </Script>
</PLEXILScript>
