lc = lcm.lcm.LCM.getSingleton();
aggregator = lcm.lcm.MessageAggregator();

lc.subscribe('EXAMPLE', aggregator);

while true
    disp waiting
    millis_to_wait = 1000;
    msg = aggregator.getNextMessage(millis_to_wait);
    if length(msg) > 0
        break
    end
end

disp(sprintf('channel of received message: %s', char(msg.channel)))
disp(sprintf('raw bytes of received message:'))
disp(sprintf('%d ', msg.data'))

m = lcmtypes.example_t(msg.data);

disp(sprintf('decoded message:\n'))
disp([ 'timestamp:   ' sprintf('%d ', m.timestamp) ])
disp([ 'position:    ' sprintf('%f ', m.position) ])
disp([ 'orientation: ' sprintf('%f ', m.orientation) ])
disp([ 'ranges:      ' sprintf('%f ', m.ranges) ])
