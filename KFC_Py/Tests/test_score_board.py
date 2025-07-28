from score_board import ScoreBoard
from pubsub import Publisher

def test_score_board():
    pub = Publisher()
    score = ScoreBoard()
    pub.subscribe('capture', score)
    pub.notify('capture', {'attacker': 'QW_1', 'victim': 'PB_1', 'cell': (4,4), 'attacker_state': 'move', 'victim_state': 'idle'})
    pub.notify('capture', {'attacker': 'NB_1', 'victim': 'QW_2', 'cell': (2,2), 'attacker_state': 'move', 'victim_state': 'idle'})
    assert score.get_score('W') == 0  # White לא אכל
    assert score.get_score('B') == 10  # Black אכל רץ (1) ומלכה (9)
    # Optional: print for visual check
    score.print_scores()
