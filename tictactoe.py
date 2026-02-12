#!/usr/bin/env python3
"""Windowed ASCII Tic-Tac-Toe game using curses."""

import curses
import sys

# Window dimensions
WIN_W = 60
WIN_H = 24


def draw_border(stdscr, start_y, start_x, w, h, title, border_attr):
    """Draw a bordered window with a centered title."""
    stdscr.addstr(start_y, start_x, "+" + "=" * (w - 2) + "+", border_attr)
    stdscr.addstr(start_y + h - 1, start_x, "+" + "=" * (w - 2) + "+", border_attr)
    for i in range(1, h - 1):
        stdscr.addstr(start_y + i, start_x, "|", border_attr)
        stdscr.addstr(start_y + i, start_x + w - 1, "|", border_attr)
        # Fill interior with spaces
        stdscr.addstr(start_y + i, start_x + 1, " " * (w - 2))
    tx = start_x + (w - len(title)) // 2
    stdscr.addstr(start_y, tx, title, border_attr)


def center_str(stdscr, y, x_start, width, text, attr=curses.A_NORMAL):
    """Draw text centered within a given width."""
    x = x_start + (width - len(text)) // 2
    stdscr.addstr(y, x, text, attr)


def show_start_screen(stdscr, border_attr):
    """Display the title/start screen. Returns when user presses a key."""
    while True:
        stdscr.erase()
        rows, cols = stdscr.getmaxyx()
        if rows < WIN_H + 2 or cols < WIN_W + 2:
            stdscr.addstr(0, 0, "Terminal too small! Need {}x{}".format(WIN_W + 2, WIN_H + 2))
            stdscr.refresh()
            stdscr.getch()
            continue

        sy = (rows - WIN_H) // 2
        sx = (cols - WIN_W) // 2

        draw_border(stdscr, sy, sx, WIN_W, WIN_H, " TIC-TAC-TOE ", border_attr)

        # ASCII art title
        art = [
            r" _____ _        _____            _____          ",
            r"|_   _(_) ___  |_   _|_ _  ___  |_   _|__   ___ ",
            r"  | | | |/ __|   | |/ _` |/ __|   | |/ _ \ / _ \\",
            r"  | | | | (__    | | (_| | (__    | | (_) |  __/",
            r"  |_| |_|\___|   |_|\__,_|\___|   |_|\___/ \___|",
        ]
        art_y = sy + 2
        title_attr = curses.color_pair(3) | curses.A_BOLD
        for i, line in enumerate(art):
            center_str(stdscr, art_y + i, sx, WIN_W, line, title_attr)

        # Controls section
        ctrl_y = art_y + len(art) + 2
        section_attr = curses.color_pair(4) | curses.A_BOLD
        center_str(stdscr, ctrl_y, sx, WIN_W, "--- CONTROLS ---", section_attr)

        controls = [
            ("Move cursor", "Arrow Keys  or  W A S D"),
            ("Place piece", "Enter  or  Space"),
            ("Restart game", "R"),
            ("Quit", "Q"),
        ]

        key_attr = curses.color_pair(1) | curses.A_BOLD
        label_attr = curses.color_pair(6)
        for i, (label, keys) in enumerate(controls):
            y = ctrl_y + 2 + i
            line = "{:<16s}  {}".format(label, keys)
            lx = sx + (WIN_W - len(line)) // 2
            stdscr.addstr(y, lx, "{:<16s}  ".format(label), label_attr)
            stdscr.addstr(y, lx + 18, keys, key_attr)

        # Players section
        players_y = ctrl_y + 2 + len(controls) + 1
        center_str(stdscr, players_y, sx, WIN_W, "--- PLAYERS ---", section_attr)
        x_attr = curses.color_pair(1) | curses.A_BOLD
        o_attr = curses.color_pair(2) | curses.A_BOLD
        px = sx + WIN_W // 2 - 10
        stdscr.addstr(players_y + 2, px, "Player 1: ", label_attr)
        stdscr.addstr(players_y + 2, px + 10, "X", x_attr)
        stdscr.addstr(players_y + 2, px + 16, "Player 2: ", label_attr)
        stdscr.addstr(players_y + 2, px + 26, "O", o_attr)

        # Prompt
        prompt = "Press any key to start..."
        blink_attr = curses.color_pair(3) | curses.A_BOLD
        center_str(stdscr, sy + WIN_H - 2, sx, WIN_W, prompt, blink_attr)

        stdscr.refresh()
        key = stdscr.getch()
        if key in (ord("q"), ord("Q")):
            return False
        return True


def main(stdscr):
    curses.curs_set(0)
    stdscr.keypad(True)

    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(1, curses.COLOR_CYAN, -1)     # X color
    curses.init_pair(2, curses.COLOR_MAGENTA, -1)   # O color
    curses.init_pair(3, curses.COLOR_YELLOW, -1)    # border / highlight
    curses.init_pair(4, curses.COLOR_GREEN, -1)     # section headers
    curses.init_pair(5, curses.COLOR_RED, -1)       # draw message
    curses.init_pair(6, curses.COLOR_WHITE, -1)     # grid lines

    border_attr = curses.color_pair(3) | curses.A_BOLD

    # Show start screen
    if not show_start_screen(stdscr, border_attr):
        return

    board = [" "] * 9
    cursor = 4
    current_player = "X"
    game_over = False
    winner = None
    winning_cells = []
    scores = {"X": 0, "O": 0, "Draw": 0}

    def check_winner():
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

    def draw_game(stdscr):
        stdscr.erase()
        rows, cols = stdscr.getmaxyx()

        if rows < WIN_H + 2 or cols < WIN_W + 2:
            stdscr.addstr(0, 0, "Terminal too small! Need {}x{}".format(WIN_W + 2, WIN_H + 2))
            stdscr.refresh()
            return

        sy = (rows - WIN_H) // 2
        sx = (cols - WIN_W) // 2

        draw_border(stdscr, sy, sx, WIN_W, WIN_H, " TIC-TAC-TOE ", border_attr)

        # Scoreboard
        score_line = "X: {}   O: {}   Draw: {}".format(scores["X"], scores["O"], scores["Draw"])
        center_str(stdscr, sy + 2, sx, WIN_W, score_line, curses.A_BOLD)

        # Current player
        if not game_over:
            turn_str = "Player {}'s turn".format(current_player)
            attr = curses.color_pair(1) if current_player == "X" else curses.color_pair(2)
            center_str(stdscr, sy + 3, sx, WIN_W, turn_str, attr | curses.A_BOLD)

        # Board
        cell_w = 11
        cell_h = 3
        board_w = cell_w * 3 + 4
        board_h = cell_h * 3 + 2
        by = sy + 5
        bx = sx + (WIN_W - board_w) // 2

        grid_attr = curses.color_pair(6)

        for row in range(3):
            for col in range(3):
                idx = row * 3 + col
                cy = by + row * (cell_h + 1)
                cx = bx + col * (cell_w + 1) + (1 if col > 0 else 0)

                is_cursor = (idx == cursor and not game_over)
                is_winning = idx in winning_cells

                for dy in range(cell_h):
                    for dx in range(cell_w):
                        attr = curses.A_NORMAL
                        if is_cursor:
                            attr |= curses.A_REVERSE
                        stdscr.addstr(cy + dy, cx + dx, " ", attr)

                piece = board[idx]
                if piece == "X":
                    pa = curses.color_pair(1) | curses.A_BOLD
                    if is_cursor:
                        pa |= curses.A_REVERSE
                    if is_winning:
                        pa |= curses.A_BLINK
                    px = cx + (cell_w - 7) // 2
                    py = cy
                    stdscr.addstr(py,     px, "\\     /", pa)
                    stdscr.addstr(py + 1, px, "   X   ", pa)
                    stdscr.addstr(py + 2, px, "/     \\", pa)
                elif piece == "O":
                    pa = curses.color_pair(2) | curses.A_BOLD
                    if is_cursor:
                        pa |= curses.A_REVERSE
                    if is_winning:
                        pa |= curses.A_BLINK
                    px = cx + (cell_w - 7) // 2
                    py = cy
                    stdscr.addstr(py,     px, " OOOOO ", pa)
                    stdscr.addstr(py + 1, px, "OO   OO", pa)
                    stdscr.addstr(py + 2, px, " OOOOO ", pa)
                else:
                    if is_cursor:
                        ha = curses.A_REVERSE | curses.A_DIM
                    else:
                        ha = curses.A_DIM
                    hx = cx + cell_w // 2
                    hy = cy + cell_h // 2
                    stdscr.addstr(hy, hx, str(idx + 1), ha)

            if row < 2:
                sep_y = by + (row + 1) * cell_h + row
                for dx in range(board_w):
                    stdscr.addstr(sep_y, bx + dx, "-", grid_attr)

        for col in range(1, 3):
            sep_x = bx + col * (cell_w + 1) - 1
            for row in range(3):
                for dy in range(cell_h):
                    ry = by + row * (cell_h + 1) + dy
                    stdscr.addstr(ry, sep_x, "|", grid_attr)
            for row in range(2):
                sep_y = by + (row + 1) * cell_h + row
                stdscr.addstr(sep_y, sep_x, "+", grid_attr)

        # Status area
        msg_y = by + board_h + 1
        if game_over:
            if winner == "Draw":
                msg = "It's a Draw!"
                ma = curses.color_pair(5) | curses.A_BOLD
            else:
                msg = "Player {} Wins!".format(winner)
                ma = curses.color_pair(4) | curses.A_BOLD
            center_str(stdscr, msg_y, sx, WIN_W, msg, ma)
            center_str(stdscr, msg_y + 1, sx, WIN_W,
                       "[R] Restart  |  [Q] Quit", curses.A_DIM)
        else:
            center_str(stdscr, msg_y, sx, WIN_W,
                       "Arrows/WASD: move | Enter/Space: place | Q: quit",
                       curses.A_DIM)

        stdscr.refresh()

    while True:
        draw_game(stdscr)
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

        if key in (ord("q"), ord("Q")):
            break

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
