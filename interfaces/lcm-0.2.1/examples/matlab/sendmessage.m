lc = lcm.lcm.LCM.getSingleton();

msg = lcmtypes.example_t();

msg.timestamp = 0;
msg.position = [1  2  3];
msg.orientation = [1 0 0 0];
msg.ranges = 1:15;
msg.num_ranges = length(msg.ranges);

lc.publish('EXAMPLE', msg);

