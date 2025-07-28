from pubsub import Subscriber

PIECE_VALUES = {
    'P': 1,  # Pawn
    'N': 3,  # Knight
    'B': 3,  # Bishop
    'R': 5,  # Rook
    'Q': 9,  # Queen
    'K': 0   # King (not counted)
}

class ScoreBoard(Subscriber):
    def __init__(self):
        self.scores = {'W': 0, 'B': 0}
        self.captures = {'W': [], 'B': []}
    def update(self, event_type, data):
        if event_type == 'capture':
            victim = data['victim']
            attacker = data['attacker']
            victim_type = victim[:2][0]  # e.g. 'P' from 'PW_1'
            victim_color = victim[1]     # 'W' or 'B'
            attacker_color = attacker[1]
            value = PIECE_VALUES.get(victim_type, 0)
            # Add score to the attacker side
            self.scores[attacker_color] += value
            self.captures[attacker_color].append(victim)
    def get_score(self, color):
        return self.scores[color]
    def print_scores(self):
        print(f"White: {self.scores['W']} | Black: {self.scores['B']}")
        print(f"White captured: {self.captures['W']}")
        print(f"Black captured: {self.captures['B']}")
