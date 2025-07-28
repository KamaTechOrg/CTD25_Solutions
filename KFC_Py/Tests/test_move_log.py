from move_log import MoveLog
from pubsub import Publisher

def test_move_log():
    pub = Publisher()
    log = MoveLog()
    pub.subscribe('move', log)
    pub.subscribe('capture', log)
    pub.notify('move', {'piece': 'QW', 'from': (1,1), 'to': (2,2), 'type': 'move', 'player': 1})
    pub.notify('move', {'piece': 'PB', 'from': (6,4), 'to': (5,4), 'type': 'move', 'player': 2})
    pub.notify('capture', {'attacker': 'QW', 'victim': 'PB', 'cell': (5,4), 'attacker_state': 'move', 'victim_state': 'idle'})
    assert len(log.get_moves()) == 2
    assert len(log.get_captures()) == 1
    assert log.get_moves()[0]['piece'] == 'QW'
    assert log.get_captures()[0]['victim'] == 'PB'
    # Optional: print log for visual check
    log.print_log()
