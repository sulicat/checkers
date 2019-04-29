import os
import subprocess


board_index = [ [0, 32, 0, 31, 0, 30, 0, 29],
                [28, 0, 27, 0, 26, 0, 25, 0],
                [0, 24, 0, 23, 0, 22, 0, 21],
                [20, 0, 19, 0, 18, 0, 17, 0],
                [0, 16, 0, 15, 0, 14, 0, 13],
                [12, 0, 11, 0, 10, 0, 9, 0],
                [0, 8, 0, 7, 0, 6, 0, 5],
                [4, 0, 3, 0, 2, 0, 1, 0] ]

board = [ [0, 3, 0, 3, 0, 0, 0, 0],
          [3, 0, 3, 0, 3, 0, 0, 0],
          [0, 0, 0, 0, 0, 0, 0, 0],
          [0, 0, 0, 0, 0, 0, 0, 0],
          [0, 0, 0, 0, 0, 0, 0, 0],
          [0, 0, 0, 0, 0, 0, 0, 0],
          [0, 0, 0, 4, 0, 0, 0, 0],
          [1, 0, 2, 0, 1, 0, 0, 0] ]

FEN = {
    "B":[],
    "W":[],
    "BK":[],
    "BW":[]
}

FEN_str = ""
for r, row in enumerate(board):
    for c, item in enumerate(row):
        if( board_index[r][c] != 0 ):
            # black reg
            if( item == 1 ):
                FEN["B"].append(str(board_index[r][c]))
            # black King
            elif( item == 2 ):
                FEN["B"].append("K" + str(board_index[r][c]))
            # white reg
            elif( item == 3 ):
                FEN["W"].append(str(board_index[r][c]))
            # white King
            elif( item == 4 ):
                FEN["W"].append("K" + str(board_index[r][c]))


if len(FEN["B"]) > 0:
    FEN_str += "B:B"
    for index, p in enumerate(FEN["B"]):
        FEN_str += p
        if( index != len(FEN["B"])-1 ):
            FEN_str += ","


if len(FEN["W"]) > 0:
    FEN_str += ":W"
    for index, p in enumerate(FEN["W"]):
        FEN_str += p
        if( index != len(FEN["W"])-1 ):
            FEN_str += ","


print( FEN_str )

cmd = ['./ponder']
cmd.append( FEN_str )
os.chdir("checkers")
print( subprocess.check_output(cmd) )


