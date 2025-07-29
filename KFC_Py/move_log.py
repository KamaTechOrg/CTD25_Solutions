class ScoreBoardDisplay:
    def __init__(self):
        self.scores = {1: 0, 2: 0}
    def update(self, event_type, data):
        if event_type == 'capture':
            attacker = data.get('attacker')
            if attacker and attacker[1] == 'W':
                self.scores[1] += self._piece_value(data.get('victim', ''))
            elif attacker and attacker[1] == 'B':
                self.scores[2] += self._piece_value(data.get('victim', ''))
    def _piece_value(self, piece_id):
        # Return chess piece value
        if piece_id.startswith('P'): return 1
        if piece_id.startswith('N'): return 3
        if piece_id.startswith('B'): return 3
        if piece_id.startswith('R'): return 5
        if piece_id.startswith('Q'): return 9
        return 0
    def draw(self, bg, side='left', y=40):
        bw, bh = bg.img.shape[1], bg.img.shape[0]
        font_size = min(bw, bh) / 900
        margin = 10
        player = 1 if side == 'left' else 2
        score = self.scores[player]
        x = margin if side == 'left' else bw - 300
        color = (255,255,255,255) if side == 'left' else (0,0,0,255)
        bg.put_text(f"SCORE: {score}", x, y, font_size*1.2, color=color, thickness=3)
class MoveLogDisplay:
    def __init__(self, move_log):
        self.move_log = move_log

    def draw(self, bg, side='left', y_offset=None):
        # Draw up to 10 moves for the given side (player)
        bw, bh = bg.img.shape[1], bg.img.shape[0]
        font_size = min(bw, bh) / 900
        thickness = 2
        margin = 10
        line_height = int(32 * font_size) + 2
        max_lines = 10
        player = 1 if side == 'left' else 2
        # Filter moves by player
        moves = [m for m in self.move_log.get_moves() if m.get('player') == player]
        lines = []
        for i, move in enumerate(moves[-max_lines:], 1):
            piece = move.get('piece', '')
            move_type = move.get('type', '')
            from_cell = move.get('from', '')
            to_cell = move.get('to', '')
            time_str = move.get('time', '')
            lines.append(f"{i}. {piece} {move_type} {from_cell}->{to_cell} [{time_str}]")
        x = margin if side == 'left' else bw - 300
        if y_offset is None:
            y_offset = margin*2 + 100
        for i, line in enumerate(lines):
            y = y_offset + i*line_height
            color = (255,255,255,255) if side == 'left' else (0,0,0,255)
            bg.put_text(line, x, y, font_size, color=color, thickness=thickness)
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
            time_str = move.get('time', '')
            print(f"{i}. {move['piece']} {move['type']} {move['from']}->{move['to']} [{time_str}]")
        print("--- Captures ---")
        for cap in self.captures:
            print(f"{cap['attacker']} captured {cap['victim']} at {cap['cell']}")
