#!/usr/bin/env python3
"""Windowed ASCII Tic-Tac-Toe game using curses."""

import curses
import sys


def main(stdscr):
    curses.curs_set(0)
    stdscr.keypad(True)

    # Init color pairs
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(1, curses.COLOR_CYAN, -1)    # X color
    curses.init_pair(2, curses.COLOR_MAGENTA, -1)  # O color
    curses.init_pair(3, curses.COLOR_YELLOW, -1)   # border / highlight
    curses.init_pair(4, curses.COLOR_GREEN, -1)    # win message
    curses.init_pair(5, curses.COLOR_RED, -1)      # draw message
    curses.init_pair(6, curses.COLOR_WHITE, -1)    # grid lines

    board = [" "] * 9
    cursor = 4  # start in center
    current_player = "X"
    game_over = False
    winner = None
    winning_cells = []
    scores = {"X": 0, "O": 0, "Draw": 0}

    def check_winner():
        lines = [
            (0, 1, 2), (3, 4, 5), (6, 7, 8),  # rows
            (0, 3, 6), (1, 4, 7), (2, 5, 8),  # cols
            (0, 4, 8), (2, 4, 6),              # diags
        ]
        for a, b, c in lines:
            if board[a] == board[b] == board[c] != " ":
                return board[a], [a, b, c]
        if " " not in board:
            return "Draw", []
        return None, []

    def draw(stdscr):
        stdscr.erase()
        rows, cols = stdscr.getmaxyx()

        # Window dimensions
        win_w = 39
        win_h = 21
        if rows < win_h + 2 or cols < win_w + 2:
            stdscr.addstr(0, 0, "Terminal too small! Need {}x{}".format(win_w + 2, win_h + 2))
            stdscr.refresh()
            return

        # Center the window
        start_y = (rows - win_h) // 2
        start_x = (cols - win_w) // 2

        border_attr = curses.color_pair(3) | curses.A_BOLD

        # Draw outer border
        # Top border
        stdscr.addstr(start_y, start_x, "+" + "-" * (win_w - 2) + "+", border_attr)
        # Bottom border
        stdscr.addstr(start_y + win_h - 1, start_x, "+" + "-" * (win_w - 2) + "+", border_attr)
        # Side borders
        for i in range(1, win_h - 1):
            stdscr.addstr(start_y + i, start_x, "|", border_attr)
            stdscr.addstr(start_y + i, start_x + win_w - 1, "|", border_attr)

        # Title
        title = " TIC-TAC-TOE "
        tx = start_x + (win_w - len(title)) // 2
        stdscr.addstr(start_y, tx, title, border_attr)

        # Scoreboard
        score_line = "X: {}   O: {}   Draw: {}".format(scores["X"], scores["O"], scores["Draw"])
        sx = start_x + (win_w - len(score_line)) // 2
        stdscr.addstr(start_y + 2, sx, score_line, curses.A_BOLD)

        # Current player indicator
        if not game_over:
            turn_str = "Player {}'s turn".format(current_player)
            attr = curses.color_pair(1) if current_player == "X" else curses.color_pair(2)
            tx2 = start_x + (win_w - len(turn_str)) // 2
            stdscr.addstr(start_y + 3, tx2, turn_str, attr | curses.A_BOLD)

        # Draw the board
        # Board is 3x3 cells, each cell is 9 wide x 3 tall
        cell_w = 9
        cell_h = 3
        board_w = cell_w * 3 + 4  # 4 for separators (2 vertical lines)
        board_h = cell_h * 3 + 2  # 2 for horizontal lines
        board_start_y = start_y + 5
        board_start_x = start_x + (win_w - board_w) // 2

        grid_attr = curses.color_pair(6)

        for row in range(3):
            for col in range(3):
                idx = row * 3 + col
                cy = board_start_y + row * (cell_h + 1)
                cx = board_start_x + col * (cell_w + 1) + (1 if col > 0 else 0)

                is_cursor = (idx == cursor and not game_over)
                is_winning = idx in winning_cells

                # Draw cell content
                piece = board[idx]
                for dy in range(cell_h):
                    for dx in range(cell_w):
                        ch = " "
                        attr = curses.A_NORMAL

                        if is_cursor:
                            attr |= curses.A_REVERSE

                        stdscr.addstr(cy + dy, cx + dx, ch, attr)

                # Draw the piece (big ASCII art letters)
                if piece == "X":
                    piece_attr = curses.color_pair(1) | curses.A_BOLD
                    if is_cursor:
                        piece_attr |= curses.A_REVERSE
                    if is_winning:
                        piece_attr |= curses.A_BLINK
                    # 3-line X in center of cell
                    px = cx + (cell_w - 5) // 2
                    py = cy
                    stdscr.addstr(py,     px, "\\   /", piece_attr)
                    stdscr.addstr(py + 1, px, "  X  ", piece_attr)
                    stdscr.addstr(py + 2, px, "/   \\", piece_attr)
                elif piece == "O":
                    piece_attr = curses.color_pair(2) | curses.A_BOLD
                    if is_cursor:
                        piece_attr |= curses.A_REVERSE
                    if is_winning:
                        piece_attr |= curses.A_BLINK
                    px = cx + (cell_w - 5) // 2
                    py = cy
                    stdscr.addstr(py,     px, " OOO ", piece_attr)
                    stdscr.addstr(py + 1, px, "O   O", piece_attr)
                    stdscr.addstr(py + 2, px, " OOO ", piece_attr)
                else:
                    # Empty cell – show position number hint
                    if is_cursor:
                        hint_attr = curses.A_REVERSE | curses.A_DIM
                    else:
                        hint_attr = curses.A_DIM
                    hx = cx + cell_w // 2
                    hy = cy + cell_h // 2
                    stdscr.addstr(hy, hx, str(idx + 1), hint_attr)

            # Draw horizontal separator after each row except the last
            if row < 2:
                sep_y = board_start_y + (row + 1) * cell_h + row
                for dx in range(board_w):
                    stdscr.addstr(sep_y, board_start_x + dx, "-", grid_attr)

        # Draw vertical separators
        for col in range(1, 3):
            sep_x = board_start_x + col * (cell_w + 1) - 1
            for row in range(3):
                for dy in range(cell_h):
                    ry = board_start_y + row * (cell_h + 1) + dy
                    stdscr.addstr(ry, sep_x, "|", grid_attr)
            # Intersections
            for row in range(2):
                sep_y = board_start_y + (row + 1) * cell_h + row
                stdscr.addstr(sep_y, sep_x, "+", grid_attr)

        # Game over message
        msg_y = board_start_y + board_h + 1
        if game_over:
            if winner == "Draw":
                msg = "It's a Draw!"
                msg_attr = curses.color_pair(5) | curses.A_BOLD
            else:
                msg = "Player {} Wins!".format(winner)
                msg_attr = curses.color_pair(4) | curses.A_BOLD
            mx = start_x + (win_w - len(msg)) // 2
            stdscr.addstr(msg_y, mx, msg, msg_attr)
            restart_msg = "Press [R] to restart  |  [Q] to quit"
            rx = start_x + (win_w - len(restart_msg)) // 2
            stdscr.addstr(msg_y + 1, rx, restart_msg, curses.A_DIM)
        else:
            help_msg = "Arrow keys / WASD: move  |  Enter/Space: place  |  Q: quit"
            hx = start_x + (win_w - len(help_msg)) // 2
            stdscr.addstr(msg_y, hx, help_msg, curses.A_DIM)

        stdscr.refresh()

    while True:
        draw(stdscr)
        key = stdscr.getch()

        if game_over:
            if key in (ord("q"), ord("Q")):
                break
            if key in (ord("r"), ord("R")):
                board = [" "] * 9
                cursor = 4
                current_player = "X"
                game_over = False
                winner = None
                winning_cells = []
            continue

        # Quit
        if key in (ord("q"), ord("Q")):
            break

        # Movement
        row, col = divmod(cursor, 3)
        if key in (curses.KEY_UP, ord("w"), ord("W")):
            row = (row - 1) % 3
        elif key in (curses.KEY_DOWN, ord("s"), ord("S")):
            row = (row + 1) % 3
        elif key in (curses.KEY_LEFT, ord("a"), ord("A")):
            col = (col - 1) % 3
        elif key in (curses.KEY_RIGHT, ord("d"), ord("D")):
            col = (col + 1) % 3
        elif key in (curses.KEY_ENTER, 10, 13, ord(" ")):
            if board[cursor] == " ":
                board[cursor] = current_player
                result, cells = check_winner()
                if result:
                    game_over = True
                    winner = result
                    winning_cells = cells
                    scores[result] = scores.get(result, 0) + 1
                else:
                    current_player = "O" if current_player == "X" else "X"

        cursor = row * 3 + col


if __name__ == "__main__":
    try:
        curses.wrapper(main)
    except KeyboardInterrupt:
        sys.exit(0)
