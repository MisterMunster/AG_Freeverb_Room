#!/usr/bin/env python3
"""
Windowed ASCII Tic-Tac-Toe game.

Cross-platform (Windows / Linux / macOS) using ANSI escape codes.
No external dependencies — runs with standard Python 3.6+.

Build a standalone exe:
    pip install pyinstaller
    pyinstaller --onefile --name tictactoe tictactoe.py
"""

import os
import sys

# ── Platform-specific input handling ────────────────────────────────────────

if os.name == "nt":
    import msvcrt

    def _enable_ansi():
        """Enable ANSI/VT100 escape sequences on Windows 10+."""
        import ctypes
        kernel32 = ctypes.windll.kernel32
        handle = kernel32.GetStdHandle(-11)  # STD_OUTPUT_HANDLE
        mode = ctypes.c_ulong()
        kernel32.GetConsoleMode(handle, ctypes.byref(mode))
        kernel32.SetConsoleMode(handle, mode.value | 0x0004)  # ENABLE_VIRTUAL_TERMINAL_PROCESSING

    def get_key():
        """Read a single keypress on Windows. Returns a string identifier."""
        _enable_ansi()
        ch = msvcrt.getwch()
        if ch in ("\x00", "\xe0"):  # special key prefix
            ch2 = msvcrt.getwch()
            mapping = {"H": "UP", "P": "DOWN", "K": "LEFT", "M": "RIGHT"}
            return mapping.get(ch2, "")
        if ch == "\r":
            return "ENTER"
        return ch
else:
    import tty
    import termios
    import select

    def get_key():
        """Read a single keypress on Unix. Returns a string identifier."""
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
            if ch == "\x1b":
                # Check for escape sequence
                if select.select([sys.stdin], [], [], 0.05)[0]:
                    ch2 = sys.stdin.read(1)
                    if ch2 == "[" and select.select([sys.stdin], [], [], 0.05)[0]:
                        ch3 = sys.stdin.read(1)
                        return {"A": "UP", "B": "DOWN", "C": "RIGHT", "D": "LEFT"}.get(ch3, "")
                return "ESC"
            if ch in ("\r", "\n"):
                return "ENTER"
            return ch
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)


# ── ANSI helpers ────────────────────────────────────────────────────────────

ESC = "\033"
RESET = ESC + "[0m"
BOLD = ESC + "[1m"
DIM = ESC + "[2m"
BLINK = ESC + "[5m"
REVERSE = ESC + "[7m"
CYAN = ESC + "[36m"
MAGENTA = ESC + "[35m"
YELLOW = ESC + "[33m"
GREEN = ESC + "[32m"
RED = ESC + "[31m"
WHITE = ESC + "[37m"


def clear_screen():
    sys.stdout.write(ESC + "[2J")


def move_cursor(row, col):
    sys.stdout.write(ESC + "[{};{}H".format(row + 1, col + 1))


def hide_cursor():
    sys.stdout.write(ESC + "[?25l")


def show_cursor():
    sys.stdout.write(ESC + "[?25h")


def get_terminal_size():
    try:
        cols, rows = os.get_terminal_size()
    except OSError:
        rows, cols = 30, 80
    return rows, cols


def put(row, col, text, *styles):
    move_cursor(row, col)
    prefix = "".join(styles) if styles else ""
    sys.stdout.write(prefix + text + RESET)


def put_center(row, x_start, width, text, *styles):
    col = x_start + (width - len(text)) // 2
    put(row, col, text, *styles)


def flush():
    sys.stdout.flush()


# ── Drawing ─────────────────────────────────────────────────────────────────

WIN_W = 60
WIN_H = 24


def draw_border(sy, sx, w, h, title):
    put(sy, sx, "+" + "=" * (w - 2) + "+", YELLOW, BOLD)
    put(sy + h - 1, sx, "+" + "=" * (w - 2) + "+", YELLOW, BOLD)
    for i in range(1, h - 1):
        put(sy + i, sx, "|", YELLOW, BOLD)
        put(sy + i, sx + 1, " " * (w - 2))
        put(sy + i, sx + w - 1, "|", YELLOW, BOLD)
    tx = sx + (w - len(title)) // 2
    put(sy, tx, title, YELLOW, BOLD)


def show_start_screen():
    """Draw the start screen. Returns True to play, False to quit."""
    while True:
        clear_screen()
        rows, cols = get_terminal_size()
        if rows < WIN_H + 2 or cols < WIN_W + 2:
            put(0, 0, "Terminal too small! Need {}x{}. Resize and press a key.".format(WIN_W + 2, WIN_H + 2))
            flush()
            get_key()
            continue

        sy = (rows - WIN_H) // 2
        sx = (cols - WIN_W) // 2

        draw_border(sy, sx, WIN_W, WIN_H, " TIC-TAC-TOE ")

        art = [
            " _____ _        _____            _____",
            "|_   _(_) ___  |_   _|_ _  ___  |_   _|__   ___",
            "  | | | |/ __|   | |/ _` |/ __|   | |/ _ \\ / _ \\\\",
            "  | | | | (__    | | (_| | (__    | | (_) |  __/",
            "  |_| |_|\\___|   |_|\\__,_|\\___|   |_|\\___/ \\___|",
        ]
        for i, line in enumerate(art):
            put_center(sy + 2 + i, sx, WIN_W, line, YELLOW, BOLD)

        ctrl_y = sy + 9
        put_center(ctrl_y, sx, WIN_W, "--- CONTROLS ---", GREEN, BOLD)

        controls = [
            ("Move cursor", "Arrow Keys  or  W A S D"),
            ("Place piece", "Enter  or  Space"),
            ("Restart game", "R"),
            ("Quit", "Q  or  Esc"),
        ]
        for i, (label, keys) in enumerate(controls):
            y = ctrl_y + 2 + i
            lx = sx + (WIN_W - 42) // 2
            put(y, lx, "{:<16s}".format(label), WHITE)
            put(y, lx + 18, keys, CYAN, BOLD)

        players_y = ctrl_y + 2 + len(controls) + 1
        put_center(players_y, sx, WIN_W, "--- PLAYERS ---", GREEN, BOLD)
        px = sx + WIN_W // 2 - 10
        put(players_y + 2, px, "Player 1: ", WHITE)
        put(players_y + 2, px + 10, "X", CYAN, BOLD)
        put(players_y + 2, px + 16, "Player 2: ", WHITE)
        put(players_y + 2, px + 26, "O", MAGENTA, BOLD)

        put_center(sy + WIN_H - 2, sx, WIN_W, "Press any key to start...", YELLOW, BOLD)
        flush()

        key = get_key()
        if key in ("q", "Q", "ESC"):
            return False
        return True


def draw_game(board, cursor, current_player, game_over, winner, winning_cells, scores):
    clear_screen()
    rows, cols = get_terminal_size()

    if rows < WIN_H + 2 or cols < WIN_W + 2:
        put(0, 0, "Terminal too small! Need {}x{}".format(WIN_W + 2, WIN_H + 2))
        flush()
        return

    sy = (rows - WIN_H) // 2
    sx = (cols - WIN_W) // 2

    draw_border(sy, sx, WIN_W, WIN_H, " TIC-TAC-TOE ")

    # Scoreboard
    score_line = "X: {}   O: {}   Draw: {}".format(scores["X"], scores["O"], scores["Draw"])
    put_center(sy + 2, sx, WIN_W, score_line, BOLD)

    # Current player
    if not game_over:
        turn_str = "Player {}'s turn".format(current_player)
        color = CYAN if current_player == "X" else MAGENTA
        put_center(sy + 3, sx, WIN_W, turn_str, color, BOLD)

    # Board
    cell_w = 11
    cell_h = 3
    board_w = cell_w * 3 + 4
    board_h = cell_h * 3 + 2
    by = sy + 5
    bx = sx + (WIN_W - board_w) // 2

    for row in range(3):
        for col in range(3):
            idx = row * 3 + col
            cy = by + row * (cell_h + 1)
            cx = bx + col * (cell_w + 1) + (1 if col > 0 else 0)

            is_cursor = (idx == cursor and not game_over)
            is_winning = idx in winning_cells

            # Draw cell background
            for dy in range(cell_h):
                bg = REVERSE if is_cursor else ""
                put(cy + dy, cx, " " * cell_w, bg)

            piece = board[idx]
            if piece == "X":
                styles = [CYAN, BOLD]
                if is_cursor:
                    styles.append(REVERSE)
                if is_winning:
                    styles.append(BLINK)
                px = cx + (cell_w - 7) // 2
                put(cy,     px, "\\     /", *styles)
                put(cy + 1, px, "   X   ", *styles)
                put(cy + 2, px, "/     \\", *styles)
            elif piece == "O":
                styles = [MAGENTA, BOLD]
                if is_cursor:
                    styles.append(REVERSE)
                if is_winning:
                    styles.append(BLINK)
                px = cx + (cell_w - 7) // 2
                put(cy,     px, " OOOOO ", *styles)
                put(cy + 1, px, "OO   OO", *styles)
                put(cy + 2, px, " OOOOO ", *styles)
            else:
                style = [DIM]
                if is_cursor:
                    style.append(REVERSE)
                put(cy + cell_h // 2, cx + cell_w // 2, str(idx + 1), *style)

        # Horizontal separator
        if row < 2:
            sep_y = by + (row + 1) * cell_h + row
            put(sep_y, bx, "-" * board_w, WHITE)

    # Vertical separators
    for col in range(1, 3):
        sep_x = bx + col * (cell_w + 1) - 1
        for row in range(3):
            for dy in range(cell_h):
                ry = by + row * (cell_h + 1) + dy
                put(ry, sep_x, "|", WHITE)
        for row in range(2):
            sep_y = by + (row + 1) * cell_h + row
            put(sep_y, sep_x, "+", WHITE)

    # Status area
    msg_y = by + board_h + 1
    if game_over:
        if winner == "Draw":
            put_center(msg_y, sx, WIN_W, "It's a Draw!", RED, BOLD)
        else:
            put_center(msg_y, sx, WIN_W, "Player {} Wins!".format(winner), GREEN, BOLD)
        put_center(msg_y + 1, sx, WIN_W, "[R] Restart  |  [Q] Quit", DIM)
    else:
        put_center(msg_y, sx, WIN_W, "Arrows/WASD: move | Enter/Space: place | Q: quit", DIM)

    flush()


# ── Game logic ──────────────────────────────────────────────────────────────

def check_winner(board):
    lines = [
        (0, 1, 2), (3, 4, 5), (6, 7, 8),
        (0, 3, 6), (1, 4, 7), (2, 5, 8),
        (0, 4, 8), (2, 4, 6),
    ]
    for a, b, c in lines:
        if board[a] == board[b] == board[c] != " ":
            return board[a], [a, b, c]
    if " " not in board:
        return "Draw", []
    return None, []


def main():
    if os.name == "nt":
        _enable_ansi()

    hide_cursor()
    try:
        if not show_start_screen():
            return

        board = [" "] * 9
        cursor = 4
        current_player = "X"
        game_over = False
        winner = None
        winning_cells = []
        scores = {"X": 0, "O": 0, "Draw": 0}

        while True:
            draw_game(board, cursor, current_player, game_over, winner, winning_cells, scores)
            key = get_key()

            if game_over:
                if key in ("q", "Q", "ESC"):
                    break
                if key in ("r", "R"):
                    board = [" "] * 9
                    cursor = 4
                    current_player = "X"
                    game_over = False
                    winner = None
                    winning_cells = []
                continue

            if key in ("q", "Q", "ESC"):
                break

            row, col = divmod(cursor, 3)
            if key in ("UP", "w", "W"):
                row = (row - 1) % 3
            elif key in ("DOWN", "s", "S"):
                row = (row + 1) % 3
            elif key in ("LEFT", "a", "A"):
                col = (col - 1) % 3
            elif key in ("RIGHT", "d", "D"):
                col = (col + 1) % 3
            elif key in ("ENTER", " "):
                if board[cursor] == " ":
                    board[cursor] = current_player
                    result, cells = check_winner(board)
                    if result:
                        game_over = True
                        winner = result
                        winning_cells = cells
                        scores[result] = scores.get(result, 0) + 1
                    else:
                        current_player = "O" if current_player == "X" else "X"

            cursor = row * 3 + col
    finally:
        show_cursor()
        clear_screen()
        move_cursor(0, 0)
        flush()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        show_cursor()
        clear_screen()
        move_cursor(0, 0)
        flush()
        sys.exit(0)
