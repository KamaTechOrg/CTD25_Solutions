from pubsub import Subscriber

class MoveLog(Subscriber):
    def __init__(self):
        self.moves = []
        self.captures = []
    def update(self, event_type, data):
        if event_type == 'move':
            self.moves.append(data)
        elif event_type == 'capture':
            self.captures.append(data)
    def get_moves(self):
        return self.moves
    def get_captures(self):
        return self.captures
    def print_log(self):
        print("--- Move Log ---")
        for i, move in enumerate(self.moves, 1):
            print(f"{i}. {move['piece']} {move['type']} {move['from']}->{move['to']}")
        print("--- Captures ---")
        for cap in self.captures:
            print(f"{cap['attacker']} captured {cap['victim']} at {cap['cell']}")
