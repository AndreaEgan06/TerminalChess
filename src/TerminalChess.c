// This is a simple chess program that runs in the terminal.
// Throughout the codebase, a move refers to a half-move, or ply.
// In proper chess terminology a move involves a turn from white
// and a turn from black, but we will assume a move refers to one
// turn by either white or black.
// @author Andrea Egan 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/// The maximum amount of characters a move can be. #MOVE_RELATED
#define MAX_MOVE_SIZE 10

/// The size of a chess board is 8 x 8. #BOARD_RELATED
#define BOARD_SIZE 8

/// How many haracters can fit in a buffer. #IO_RELATED
#define BUFFER_SIZE 100

/// The string representing the white player. #IO_RELATED
#define WHITE_STR "White"

/// The string represenitng the black player. #IO_RELATED
#define BLACK_STR "Black"

/// This macro passes the given arguments to printf
/// and gets user input from stdin into a char array called buffer.
/// #IO_RELATED
#define GET_INPUT(...)                                  \
    printf(__VA_ARGS__);                                \
    fgets(buffer, BUFFER_SIZE, stdin);                  \
    for (int i = 0; i < BUFFER_SIZE; ++i) {             \
        if (buffer[i] == '\n') {                        \
            buffer[i] = '\0';                           \
            break;                                      \
        }                                               \
    }                                                   \
    c = 0;                                              \
    while (isspace(buffer[c]) && c < BUFFER_SIZE) ++c;  \
    if (c >= BUFFER_SIZE) continue;

/// This macro asserts that a certain condition is true.
/// If the condition is false then the given error message is printed
/// to stderr, the variable exitValue is set to -1,
/// and the program goes to the exit label.
/// #DESING_PATTERN
#define ASSERT(condition, errorMessage)                 \
    if (!(condition)) {                                 \
        fprintf(stderr, "\n[ERROR] %s\n", errorMessage);\
        exitValue = -1;                                 \
        goto exit;                                      \
    }

/// This macro compares two Positions (possibly KingPositions),
/// and returns whether they are a knight's move apart.
/// @param pos1 The first position to compare
/// @param pos2 The second position to compare
/// #POSITION_RELATED
#define isKnightMove(pos1, pos2) ((abs((pos1).row - (pos2).row) == 2 * abs((pos1).col - (pos2).col)) || (abs((pos1).col - (pos2).col) == 2 * abs((pos1).row - (pos2).row)))

/// This macro compares two Positions (possibly KingPositions),
/// and returns whether they refer to the same position.
/// @param pos1 The first position to compare
/// @param pos2 The second position to compare
/// #POSITION_RELATED
#define comparePositions(pos1, pos2) ((pos1).col == (pos2).col && (pos1).row == (pos2).row)

/// This macro returns whether or not a character representation of a piece
/// is the same color was would be indicated by the given boolean.
/// @param piece The piece (char) to compare
/// @param isWhite If this piece should be compared against WHITE
/// #PIECE_RELATED
#define hasSameColor(piece, isWhite) strchr((isWhite) ? "PRNBQK" : "prnbqk", (piece))

/// This macro returns the equivalent Position from a given KingPosition.
/// @param kingPos the KingPosition to get the equivalent Position from
/// #POSITION_RELATED
#define getRegPos(kingPos) (Position){(kingPos).row, (kingPos).col}

/// This macro get the piece (char) from the board at a given position
/// @param pos the position to get from the board
/// #POSITION_RELATED
/// #BOARD_RELATED
/// #PIECE_RELATED
#define boardAt(pos) board[(pos).row][(pos).col]

/// @brief A Board is a 2D array of pieces (chars). #BOARD_RELATED
typedef char (*Board)[BOARD_SIZE];

/// The size of a chunk. #IO_RELATED
#define CHUNK_SIZE 50

/// @brief A chunk is a fixed-length array of string representations of moves made.
///        Each string has a fixed maximum size.
/// #LOGGING_RELATED
typedef char Chunk[CHUNK_SIZE][MAX_MOVE_SIZE];

/// @brief This enum represents all of the different statuses that a game of chess
///        could be in.
/// #GAME_STATUS_RELATED
typedef enum {
    WHITE,              ///< It is the WHITE player's turn.
    BLACK,              ///< It is the BLACK player's turn.
    WHITE_WON,          ///< The WHITE player won the game.
    DRAWBYPLAYER,       ///< The game was drawn by a player.
    DRAWBYREPETITION,   ///< The game was drawn by repetition.
    DRAWBY50MOVERULE,   ///< The game was drawn by the 50 move rule.
    DRAWBYMATERIAL,     ///< The game was drawn by insufficient material.
    STALEMATE,          ///< The game ended in stalemate.
    BLACK_WON           ///< The BLACK player won the game.
} GameStatus;

/// @brief This enum defines the typed of pieces that can be placed on a
///        chess board. Each value is set equal to the character representation
///        of the white variant of the piece. (upper case pieces represent
///        WHITE, whereas lower case pieces represent BLACK)
/// #PIECE_RELATED
typedef enum {
    ROOK = 'R',     ///< A WHITE Rook
    KNIGHT = 'N',   ///< A WHITE Knight
    BISHOP = 'B',   ///< A WHITE Bishop
    QUEEN = 'Q',    ///< A WHITE Queen
    KING = 'K',     ///< A WHITE King
    PAWN = 'P'      ///< A WHITE pawn
} PieceType;

/// @brief This enum defines the types of searches that can be made when
///        processing a chess move such as finding pieces or verifying
///        if a move was legal.
/// #SEARCH_RELATED
typedef enum {
    FINDPIECES,
    ISLEGALMOVE
} SearchType;

/// @brief This enum describes the types of moves that
///        can be made by a player.
/// #MOVE_RELATED
/// #GAME_STATUS_RELATED
typedef enum {
    NORMALMOVE,     ///< The move was a capture or normal piece move
    DOUBLEPAWNMOVE, ///< The move was a double pawn move
    ENPEASANT,      ///< The move was en passant
    PROMOTION,      ///< The move was a promotion
    CASTLESHORT,    ///< The move was short castle
    CASTLELONG,     ///< The move was long castle
    PLAYERDRAW,     ///< The move was a player drawing the game
    RESIGN          ///< The move was a player resigning the game
} MoveType;


typedef struct {
    short int row;
    short int col;
} Position;

typedef struct {
    bool canCastleShort : 1;
    bool canCastleLong  : 1;
    unsigned short int row : 3;
    unsigned short int col : 3;
} KingPosition;

/// @brief A piece couple contains two sqaures from the board
///        inside of a single character.
/// #REMOVE_FROM_EXISTENCE
typedef struct {
    unsigned char p1 : 4;
    unsigned char p2 : 4;
} PieceCouple;

/// @brief A BoardPosition is an array of PieceCouples?
typedef PieceCouple BoardPosition[BOARD_SIZE * BOARD_SIZE / 2];

typedef struct {
    MoveType type;
    Position origin;
    Position destination;
    char pieceMoved;
    bool captures;
    char promotionPiece;
} Move;

/// @brief A GameLog contains a text-based log of all the moves made
///        during a game.
typedef struct {
    Chunk *log;         ///< The array of @linkplain Chunks that make up the gamelog.
    int chunkCount;     ///< How many chunks are in the log array.
    int moveCounter;    ///< How many moves have been made.
} GameLog;

typedef struct {
    Board board;
    KingPosition whiteKing;
    KingPosition blackKing;
    GameStatus status;
    Move move;
    BoardPosition positions[100];
    unsigned short int movesWithoutCaptures;
    unsigned short int moveCounter;
} GameState;

Board initializeBoard(void);
void printBoard(Board board);
bool parseFEN(const char *fenStr, GameState *state);
void loadPosition(GameState *state);
void exportPosition(const GameState *state);
void getMove(char *buffer, GameState *state, bool *specifyRow, bool *specifyCol);
bool getLineOfSight(const Position pos1, const Position pos2, Position *list, unsigned short int *count);
bool hasClearSight(Board board, const Position pos1, const Position pos2);
bool search(PieceType pieceType, SearchType searchType, Position pos, Position dest, char piece, Board board, Position *candidates, unsigned short int *count, const KingPosition *kingPos);
bool searchBoard(PieceType type, SearchType SearchType, Position pos, char piece, Board board, Position *candidates, unsigned short int *count, const KingPosition *kingPos);
bool canMoveTo(Board board, Position target, bool isWhite, KingPosition kingPos, const Move previousMove);
bool isCheckmate(Board board, KingPosition oppKingPos, Position *checkers, const short int checkerCount, const Move previousMove);
BoardPosition *convertBoardPosition(GameState *state);
void updateGameStatus(GameState *state, bool *isCheck);
bool hasSufficientMaterial(Board board);
bool isStalemate(Board board, KingPosition kingPos);
bool compareBoardPositions(const BoardPosition *old, const BoardPosition *new);
bool isInCheck(Board board, const Position pos, bool isWhite, const Move previousMove, Position *attackers, unsigned short int *attackerCount);
bool isPossibleMove(Board board, const Move move, const KingPosition *ownKingPos);
void makeMove(Board board, const Move move);
bool setMoveToCastle(Move *move, const MoveType type, const KingPosition kingPos);
bool validateMove(char *move, GameState *state, bool *specifyRow, bool *specifyCol);
bool findDestination(char *rawMove, Move *move, unsigned short int *destinationIndex);
bool initializeGameLog(GameLog *game);
bool resize(GameLog *game);
bool isRow(const int c) { return (0 <= c && c < BOARD_SIZE) || ('1' <= c && c < BOARD_SIZE + '1'); }
bool isCol(const int c) { return (0 <= c && c < BOARD_SIZE) || ('a' <= c && c < BOARD_SIZE + 'a'); }
bool logMove(GameLog *game, GameState *state, const bool isCheck, const bool specifyRow, const bool specifyCol);
void createGameFile(GameLog *game, GameStatus status);

int main(void) {
    int exitValue = 0, c = 0;
    char buffer[BUFFER_SIZE];
    bool recording = false;
    GameLog gameLog = {0};
    GameState state = {
        .whiteKing = { true, true, 7, 4 },
        .blackKing = { true, true, 0, 4 },
        .board = initializeBoard(),
        .move = { .pieceMoved = 'k' },
        .moveCounter = 1
    };

    puts("--------------------------------\nWelcome to chess!\n--------------------------------\n\nTo load a position from FEN notation, type \"load\".\nTo start a game, type \"start\".\nAt any point during the game, typing \"export\" will generate the FEN notation for the current position.\n");
    while (true) {
        GET_INPUT("Do you want to load a position or start the game? ")
        if (strcmp(&buffer[c], "start") == 0) break;
        if (strcmp(&buffer[c], "load") != 0) continue;
        loadPosition(&state);
        break;
    }
    while (true) {
        GET_INPUT("Would you like to record the game? (y|n) ")
        recording = tolower(buffer[c]) == 'y';
        if (recording || tolower(buffer[c] == 'n')) break;
    }
    ASSERT(!recording || initializeGameLog(&gameLog), "Could not start recording the game.")
    convertBoardPosition(&state);

    do {
        bool isCheck = false, specifyRow = false, specifyCol = false;
        ++state.moveCounter;
        printBoard(state.board);
        getMove(buffer, &state, &specifyRow, &specifyCol);
        makeMove(state.board, state.move);
        updateGameStatus(&state, &isCheck);
        if (recording) ASSERT(logMove(&gameLog, &state, isCheck, specifyRow, specifyCol), "Unable to record the move.")
    } while (state.status == WHITE || state.status == BLACK);

    if (state.move.type != PLAYERDRAW && state.move.type != RESIGN) printBoard(state.board);
    switch (state.status) {
        case DRAWBYPLAYER:
            puts("It's a draw!");
            break;
        case DRAWBY50MOVERULE:
            puts("No pieces have been captured and no pawns have been moved for the last 50 moves.\nIt's a draw!");
            break;
        case DRAWBYMATERIAL:
            puts("There is insufficient material for either side to win.\nIt's a draw!");
            break;
        case DRAWBYREPETITION:
            puts("The same position has been reached for the third time.\nIt's a draw!");
            break;
        case STALEMATE:
            puts("It's stalemate!");
            break;
        default:
            printf("%s wins!\n", state.status == WHITE_WON ? WHITE_STR : BLACK_STR);
    }
    if (recording) createGameFile(&gameLog, state.status);

exit:
    if (recording) free(gameLog.log);
    free(state.board);
    return exitValue;
}

Board initializeBoard(void) {
    Board board = malloc(sizeof(char) * BOARD_SIZE * BOARD_SIZE);
    char boardString[BOARD_SIZE * BOARD_SIZE] = {
        'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r',
        'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
        'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P',
        'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'
    };
    for (int i = 0; i < BOARD_SIZE; ++i)
        for (int j = 0; j < BOARD_SIZE; ++j)
            board[i][j] = boardString[i * BOARD_SIZE + j];
    return board;
}

void printBoard(Board board) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            switch (board[i][j]) {
                case 'R':
                    printf("♜");
                    break;
                case 'r':
                    printf("♖");
                    break;
                case 'N':
                    printf("♞");
                    break;
                case 'n':
                    printf("♘");
                    break;
                case 'B':
                    printf("♝");
                    break;
                case 'b':
                    printf("♗");
                    break;
                case 'Q':
                    printf("♛");
                    break;
                case 'q':
                    printf("♕");
                    break;
                case 'K':
                    printf("♚");
                    break;
                case 'k':
                    printf("♔");
                    break;
                case 'P':
                    printf("♟");
                    break;
                case 'p':
                    printf("♙");
                    break;
                default:
                    printf((i + j) % 2 == 0 ? "■" : "□");
            }
        }
        putchar('\n');
    }
}

bool parseFEN(const char *const restrict fenStr, GameState *const restrict state) {
    const char validPieces[22] = "rnbqkpRNBQKP12345678/";
    unsigned short int iter = 0, i = 0, j = 0;
    bool seenWhiteKing = false, seenBlackKing = false;
    while (strchr(validPieces, fenStr[iter])) {
        if (i >= BOARD_SIZE || j > BOARD_SIZE) return false;
        if (isalpha(fenStr[iter])) {
            if (fenStr[iter] == 'K') {
                if (seenWhiteKing) return false;
                seenWhiteKing = true;
                state->whiteKing.row = i;
                state->whiteKing.col = j;
            } else if (fenStr[iter] == 'k') {
                if (seenBlackKing) return false;
                seenBlackKing = true;
                state->blackKing.row = i;
                state->blackKing.col = j;
            }
            state->board[i][j++] = fenStr[iter];
        } else if (isdigit(fenStr[iter])) {
            for (int k = 0; k < fenStr[iter] - '0'; ++k) {
                state->board[i][j++] = ' ';
                if (j > BOARD_SIZE) return false;
            }
        } else {
            ++i;
            j = 0;
        }
        ++iter;
    }
    if (i != BOARD_SIZE - 1 || j != BOARD_SIZE) return false;
    if (fenStr[iter++] != ' ') return false;
    switch(fenStr[iter++]) {
        case 'w':
            state->status = WHITE;
            break;
        case 'b':
            state->status = BLACK;
            break;
        default:
            return false;
    }
    if (fenStr[iter++] != ' ') return false;
    bool seenBlack = false;
    if (fenStr[iter] != '-') {
        bool shouldBreak = false;
        for (i = 0; i < 4; ++i) {
            switch(fenStr[iter]) {
                case 'K':
                    if (seenBlack) return false;
                    state->whiteKing.canCastleShort = true;
                    break;
                case 'Q':
                    if (seenBlack) return false;
                    state->whiteKing.canCastleLong = true;
                    break;
                case 'k':
                    seenBlack = true;
                    state->blackKing.canCastleShort = true;
                    break;
                case 'q':
                    seenBlack = true;
                    state->blackKing.canCastleLong = true;
                    break;
                case ' ':
                    shouldBreak = true;
                    break;
                default:
                    return false;
            }
            if (shouldBreak) break;
            ++iter;
        }
    } else ++iter;
    if (fenStr[iter++] != ' ') return false;
    if (fenStr[iter] != '-') {
        char col = fenStr[iter++];
        if (col < 'a' || 'h' < col) return false;
        state->move.origin.col = col - 'a';
        state->move.destination.col = col - 'a';
        char row = fenStr[iter++];
        if (row != '6' && state->status == WHITE) return false;
        if (row != '3' && state->status == BLACK) return false;
        state->move.origin.row = row - '2' + (state->status == BLACK) * 2;
        state->move.destination.row = row - '2' + (state->status == WHITE) * 2;
    } else ++iter;
    if (fenStr[iter++] != ' ') return false;
    for (i = 0; i < 3; ++i) {
        if (!isdigit(fenStr[iter])) return false;
        state->movesWithoutCaptures *= 10;
        state->movesWithoutCaptures += fenStr[iter++] - '0';
        if (fenStr[iter++] == ' ') break;
    }
    if (i == 3) return false;
    Position attackers[16];
    unsigned short int attackerCount = 0;
    if (isInCheck(state->board, state->status == WHITE ? getRegPos(state->whiteKing) : getRegPos(state->blackKing), state->status == WHITE, state->move, attackers, &attackerCount) && isCheckmate(state->board, state->status == WHITE ? state->whiteKing : state->blackKing, attackers, attackerCount, state->move)) state->status = state->status == BLACK ? WHITE_WON : BLACK_WON;
    while (true) {
        if (!isdigit(fenStr[iter])) return false;
        state->moveCounter *= 10;
        state->moveCounter += fenStr[iter] - '0';
        if (fenStr[++iter] != '\n') continue;
        state->moveCounter *= 2;
        return true;
    }
}

void loadPosition(GameState *const restrict state) {
    char buffer[BUFFER_SIZE];
    GameState loadedState = {
        .board = initializeBoard(),
        .move = {0},
    };
    do {
        memset(buffer, 0, BUFFER_SIZE);
        printf("\nPlease enter the FEN notation: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        if (parseFEN(buffer, &loadedState)) break;
        printf("\nThat was not a valid FEN notation.");
    } while (true);
    state->whiteKing = loadedState.whiteKing;
    state->blackKing = loadedState.blackKing;
    state->status = loadedState.status;
    state->move = loadedState.move;
    state->movesWithoutCaptures = loadedState.movesWithoutCaptures;
    state->moveCounter = loadedState.moveCounter;
    for (int i = 0; i < BOARD_SIZE; ++i) for (int j = 0; j < BOARD_SIZE; ++j) state->board[i][j] = loadedState.board[i][j];
    free(loadedState.board);
}

void exportPosition(const GameState *const restrict state) {
    putchar('\n');
    unsigned short int spaceCounter = 0;
    bool canCastle = false;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            char piece = state->board[i][j];
            switch(piece) {
                case ' ':
                    ++spaceCounter;
                    break;
                default:
                    if (spaceCounter > 0) {
                        printf("%d", spaceCounter);
                        spaceCounter = 0;
                    }
                    putchar(piece);
            }
        }
        if (spaceCounter > 0) {
            printf("%d", spaceCounter);
            spaceCounter = 0;
        }
        putchar(i < BOARD_SIZE - 1 ? '/' : ' ');
    }
    printf("%c ", state->status == WHITE ? 'w' : 'b');
    if (state->whiteKing.canCastleShort) { printf("K"); canCastle = true; }
    if (state->whiteKing.canCastleShort) { printf("Q"); canCastle = true; }
    if (state->blackKing.canCastleShort) { printf("k"); canCastle = true; }
    if (state->blackKing.canCastleShort) { printf("q"); canCastle = true; }
    if (!canCastle) putchar('-');
    if (state->move.type == DOUBLEPAWNMOVE) {
        printf(" %c%c ", state->move.destination.col + 'a', '8' - (state->move.destination.row - 1 + (state->move.origin.row == BOARD_SIZE - 2) * 2));
    } else printf(" - ");
    printf("%d %d\n", state->movesWithoutCaptures - 1, state->moveCounter / 2);
    printf("\n%d. %s to move: ", state->moveCounter / 2, state->status == WHITE ? WHITE_STR : BLACK_STR);
}

void getMove(char *const restrict buffer, GameState *const restrict state, bool *const restrict specifyRow, bool *const restrict specifyCol) {
    const unsigned short int moveNumber = state->moveCounter / 2;
    const GameStatus status = state->status;
    int c;
    while (true) {
        GET_INPUT("%d. %s to move: ", moveNumber, status == WHITE ? WHITE_STR : BLACK_STR)
        if (strcmp(&buffer[c], "export") == 0) {
            exportPosition(state);
            return;
        } else if (strcmp(&buffer[c], "draw") == 0) {
            state->move.type = PLAYERDRAW;
            return;
        } else if (strcmp(&buffer[c], "resign") == 0) {
            state->move.type = RESIGN;
            return;
        } else if (validateMove(buffer, state, specifyRow, specifyCol)) return;
    }
}

// Has a max return value of 6 Positions inside the list
bool getLineOfSight(const Position pos1, const Position pos2, Position *const restrict list, unsigned short int *const restrict count) {
    short int start = pos1.row < pos2.row ? pos1.row : pos2.row, end = pos1.row < pos2.row ? pos2.row : pos1.row;
    if (abs(pos1.row - pos2.row) == abs(pos1.col - pos2.col)) {
        for (++start; start < end; ++start) list[(*count)++] = (Position){start, pos1.col + ((pos1.row - pos2.row)/(pos1.col - pos2.col)) * (start - pos1.row)};
    } else if (pos1.row == pos2.row) {
        start = pos1.col < pos2.col ? pos1.col : pos2.col;
        end = pos1.col < pos2.col ? pos2.col : pos1.col;
        for (++start; start < end; ++start) list[(*count)++] = (Position){pos1.row, start};
    } else if (pos1.col == pos2.col) {
        for (++start; start < end; ++start) list[(*count)++] = (Position){start, pos1.col};
    } else return false;
    return true;
}

bool hasClearSight(Board board, const Position pos1, const Position pos2) {
    Position list[6];
    unsigned short int count = 0;
    if (!getLineOfSight(pos1, pos2, list, &count)) return false;
    for (int i = 0; i < count; ++i) if (boardAt(list[i]) != ' ') return false;
    return true;
}

bool search(const PieceType pieceType, const SearchType searchType, const Position pos, const Position dest, const char piece, const Board board, Position *const restrict candidates, unsigned short int *const restrict count, const KingPosition *const restrict kingPos) {
    if (comparePositions(pos, dest)) return false;
    const bool isWhite = hasSameColor(piece, true);
    Move move = { NORMALMOVE, pos, dest, piece, false, ' ' };
    switch (searchType) {
        case FINDPIECES:
            if ((boardAt(dest) == piece) && ((pieceType == KNIGHT || pieceType == KING) ? true : hasClearSight(board, pos, dest))) candidates[(*count)++] = dest;
            return false;
        case ISLEGALMOVE:
            if (((pieceType == KNIGHT || pieceType == KING) ? true : hasClearSight(board, pos, dest)) && !hasSameColor(boardAt(dest), isWhite)) {
                if (hasSameColor(boardAt(dest), !isWhite) || (pieceType == PAWN && abs(pos.col - dest.col) == 1)) move.captures = true;
                if (pieceType == PAWN && abs(pos.row - dest.row) == 2) move.type = DOUBLEPAWNMOVE;
                if (isPossibleMove(board, move, kingPos)) return true;
            }
            return false;
    }
}

// type = pawn will run ISLEGALMOVE no matter what the given function is
bool searchBoard(const PieceType pieceType, const SearchType searchType, Position pos, const char piece, const Board board, Position *const restrict candidates, unsigned short int *const restrict count, const KingPosition *const restrict kingPos) {
    bool isWhite;
    switch (pieceType) {
        case ROOK:
            for (int i = 0; i < 2; ++i) {
                short int j = 0;
                short int *const row = i == 0 ? &(pos.row) : &j;
                short int *const col = i == 1 ? &(pos.col) : &j;
                for (; j < BOARD_SIZE; ++j) if (search(pieceType, searchType, pos, (Position){ *row, *col }, piece, board, candidates, count, kingPos)) return true;
            }
            return false;
        case KNIGHT:
            for (short int i = pos.row - 2; i <= pos.row + 2; ++i) for (short int j = pos.col - 2; isRow(i) && j <= pos.col + 2; ++j) {
                if (!isCol(j)) continue;
                const Position dest = { i, j };
                if (isKnightMove(dest, pos) && search(pieceType, searchType, pos, dest, piece, board, candidates, count, kingPos)) return true;
            }
            return false;
        case BISHOP:
            for (int i = -1; i < 2; i += 2) for (short int r = 0; r < BOARD_SIZE; ++r) {
                const short int col = pos.col + i * (r - pos.row);
                if (isCol(col) && search(pieceType, searchType, pos, (Position){ r, col }, piece, board, candidates, count, kingPos)) return true;
            }
            return false;
        case QUEEN:
            return searchBoard(ROOK, searchType, pos, piece, board, candidates, count, kingPos) || searchBoard(BISHOP, searchType, pos, piece, board, candidates, count, kingPos);
        case KING:
            for (short int i = pos.row - 1; i <= pos.row + 1; ++i)
                for (short int j = pos.col - 1; isRow(i) && j <= pos.col + 1; ++j)
                    if (isCol(j) && search(pieceType, searchType, pos, (Position){ i, j }, piece, board, candidates, count, kingPos)) return true;
            return false;
        case PAWN:
            isWhite = hasSameColor(piece, true);
            for (int i = pos.row + 2 - isWhite * 4; i != pos.row; i -= 1 - isWhite * 2) {
                if (!isRow(i)) continue;
                for (int j = pos.col - 1; j <= pos.col + 1; ++j) {
                    if (!isCol(j)) continue;
                    const Position dest = { i, j };
                    if (isKnightMove(dest, pos)) continue;
                    if (search(pieceType, ISLEGALMOVE, pos, dest, piece, board, candidates, count, kingPos)) return true;
                }
            }
            return false;
    }
}

bool canMoveTo(Board board, Position target, bool isWhite, KingPosition kingPos, const Move previousMove) {
    // Checks for pawns that can move forward to block
    for (int i = 1; i < 3; ++i) {
        const short int row = target.row + i * (2 * isWhite - 1);
        const char square = board[row][target.col];
        if (square == 'p' - isWhite * 32 && isPossibleMove(board, (Move){ i == 1 ? NORMALMOVE : DOUBLEPAWNMOVE, { row, target.col }, target, square, false, ' ' }, &kingPos)) return true;
        if (target.row != 3 + isWhite) break;
    }
    // Checks for pawns that can en peasant into that square
    if (target.row == 5 - 3 * isWhite && previousMove.type == DOUBLEPAWNMOVE && previousMove.destination.col == target.col)
        for (int i = target.row - 1; i <= target.row + 1; i += 2) {
            if (!isCol(i)) continue;
            const short int row = previousMove.destination.row;
            const char square = board[row][i];
            if (square != 'p' - 32 * isWhite) continue;
            if (isPossibleMove(board, (Move){ ENPEASANT, { row, target.col }, target, square, true, ' ' }, &kingPos)) return true;
        }
    // Search for all other types of pieces
    const int numberOfSearches = 4;
    const PieceType pieceList[numberOfSearches] = { ROOK, KNIGHT, BISHOP, QUEEN };
    for (int i = 0; i < numberOfSearches; ++i) {
        Position candidates[8];
        unsigned short int count = 0;
        searchBoard(pieceList[i], FINDPIECES, target, pieceList[i] + 32 * !isWhite, board, candidates, &count, NULL);
        for (unsigned short int j = 0; j < count; ++j) if (isPossibleMove(board, (Move){ NORMALMOVE, candidates[j], target, pieceList[i] + !isWhite * 32, false, ' ' }, &kingPos)) return true;
    }
    return false;
}

bool isCheckmate(Board board, KingPosition oppKingPos, Position *checkers, const short int checkerCount, const Move previousMove) {
    const bool isWhite = hasSameColor(boardAt(oppKingPos), true);
    if (!checkerCount) return false;
    if (checkerCount == 1) {
        Position saviors[16];
        unsigned short int saviorCount = 0;
        if (isInCheck(board, checkers[0], !isWhite, previousMove, saviors, &saviorCount)) for (int i = 0; i < saviorCount; ++i) if (isPossibleMove(board, (Move){ NORMALMOVE, saviors[i], checkers[0], boardAt(saviors[i]), true, ' ' }, &oppKingPos)) return false;
        if (toupper(boardAt(checkers[0])) != KNIGHT) {
            Position lineOfSight[6];
            unsigned short int squareCount = 0;
            if (!getLineOfSight(checkers[0], getRegPos(oppKingPos), lineOfSight, &squareCount)) return false;
            for (int i = 0; i < squareCount; ++i) if (canMoveTo(board, lineOfSight[i], isWhite, oppKingPos, previousMove)) return false;
        }
    }
    for (int i = oppKingPos.row - 1; i <= oppKingPos.row + 1; ++i)
        for (int j = oppKingPos.col - 1; 0 <= i && i < BOARD_SIZE && j <= oppKingPos.col + 1; ++j)
            if (0 <= j && j < BOARD_SIZE && isPossibleMove(board, (Move){ NORMALMOVE, getRegPos(oppKingPos), { i, j }, 'K' + 32 * !isWhite, false, ' ' }, &oppKingPos)) return false;
    return true;
}

BoardPosition *convertBoardPosition(GameState *state) {
    const char options[16] = { ' ', 'p', 'P', 'r', 'c', 'R', 'C', 'n', 'N', 'b', 'B', 'q', 'Q', 'k', 'K', 'e' };
    const Position corners[4] = { { 0, 0 }, { 0, 7 }, { 7, 0 }, { 7, 7 } };
    const bool castlingRights[4] = { state->blackKing.canCastleLong, state->blackKing.canCastleShort, state->whiteKing.canCastleLong, state->whiteKing.canCastleShort };
    bool seenDest = false;
    const Board board = state->board;
    BoardPosition *const position = &(state->positions[state->movesWithoutCaptures++]);
    for (int i = 0; i < BOARD_SIZE; ++i) for (int j = 0; j < BOARD_SIZE; j += 2) {
        const Position pos[2] = { { i, j },  { i, j + 1 } };
        char p[2] = { boardAt(pos[0]), boardAt(pos[1]) };
        for (int k = 0; k < 15; ++k) for (int l = 0; l < 2; ++l) if (options[k] == p[l]) p[l] = k;
        if (!seenDest) for (int k = 0; k < 2; ++k) {
            if (!comparePositions(state->move.destination, pos[k])) continue;
            if (state->move.type == DOUBLEPAWNMOVE) p[k] = 15;
            seenDest = true;
        }
        (*position)[(i * BOARD_SIZE + j) / 2].p1 = p[0];
        (*position)[(i * BOARD_SIZE + j) / 2].p2 = p[1];
    }
    for (int i = 0; i < 4; ++i) {
        PieceCouple * const pos = &((*position)[(corners[i].row * BOARD_SIZE + corners[i].col) / 2]);
        const unsigned short int offset = i % 2;
        const unsigned char c = offset == 0 ? pos->p1 : pos->p2;
        if ((c != 3 || i != offset || !castlingRights[i]) && (c != 5 || i != 2 + offset || !castlingRights[i])) continue;
        if (offset == 0) { ++pos->p1; } else ++pos->p2;
    }
    return position;
}

bool compareBoardPositions(const BoardPosition * const restrict old, const BoardPosition * const restrict new) {
    for (int i = 0; i < BOARD_SIZE * BOARD_SIZE / 2; ++i) {
        if ((*old)[i].p1 != (*new)[i].p1) return false;
        if ((*old)[i].p2 != (*new)[i].p2) return false;
    }
    return true;
}

void updateGameStatus(GameState *state, bool *isCheck) {
    Position candidates[16];
    unsigned short int count = 0;
    const Board board = state->board;
    GameStatus *status = &(state->status);
    KingPosition *ownKingPos = *status == WHITE ? &(state->whiteKing) : &(state->blackKing);
    const KingPosition oppKingPos = *status == BLACK ? state->whiteKing : state->blackKing;
    const Move move = state->move;
    const unsigned short int *movesWithoutCaptures = &(state->movesWithoutCaptures);
    if (isInCheck(board, getRegPos(oppKingPos), *status == BLACK, move, candidates, &count)) {
        *isCheck = true;
        if (isCheckmate(board, oppKingPos, candidates, count, move)) {
            *status = *status == WHITE ? WHITE_WON : BLACK_WON;
            return;
        }
    }
    if (move.type == PLAYERDRAW) {
        *status = DRAWBYPLAYER;
        return;
    } else if (*movesWithoutCaptures >= 100) {
        *status = DRAWBY50MOVERULE;
        return;
    } else if (move.type == RESIGN) {
        *status = *status == BLACK ? WHITE_WON : BLACK_WON;
        return;
    }
    if (tolower(move.pieceMoved) == 'r') {
        switch (move.origin.col) {
            case 7:
                ownKingPos->canCastleShort = false;
                break;
            case 0:
                ownKingPos->canCastleLong = false;
        }
    } else if (tolower(move.pieceMoved) == 'k') *ownKingPos = (KingPosition){
        .row = move.destination.row,
        .col = move.destination.col,
    };
    BoardPosition *currentPosition = convertBoardPosition(state);
    count = 1;
    for (int i = 0; i < *movesWithoutCaptures - 1 && count < 3; ++i)
        if (compareBoardPositions(&(state->positions[i]), currentPosition)) ++count;
    if (count == 3) {
        *status = DRAWBYREPETITION;
        return;
    }
    *status = *status == WHITE ? BLACK : WHITE;
    if (!hasSufficientMaterial(board)) *status = DRAWBYMATERIAL;
    if (isStalemate(board, oppKingPos)) *status = STALEMATE;
}

bool hasSufficientMaterial(const Board board) {
    unsigned short int knightCount = 0, bishopCount[2] = {0};
    for (int i = 0; i < BOARD_SIZE; ++i) for (int j = 0; j < BOARD_SIZE; ++j)
        switch (toupper(board[i][j])) {
            case ROOK:
            case QUEEN:
            case PAWN:
                return true;
            case KNIGHT:
                ++knightCount;
                break;
            case BISHOP:
                ++bishopCount[(i + j) % 2];
        }
    return (knightCount > 1 || (bishopCount[0] > 0 && bishopCount[1] > 0) || (bishopCount[0] + bishopCount[1] > 0 && knightCount > 0));
}

bool isStalemate(Board board, KingPosition kingPos) {
    const bool isWhite = hasSameColor(boardAt(kingPos), true);
    Position pieces[BOARD_SIZE * 2];
    unsigned short int count = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) for (int j = 0; j < BOARD_SIZE; ++j) if (hasSameColor(board[i][j], isWhite)) pieces[count++] = (Position){i, j};
    for (int i = 0; i < count; ++i) {
        const char piece = boardAt(pieces[i]);
        if (searchBoard(toupper(piece), ISLEGALMOVE, pieces[i], piece, board, NULL, NULL, &kingPos)) return false;
    }
    return true;
}

// The isWhite parameter gives the color of the victim side. Returns how many enemy pieces can see the target position. Does not return pawns that can move into that square and are on the same column.
bool isInCheck(Board board, const Position pos, bool isWhite, const Move previousMove, Position *attackers, unsigned short int *attackerCount) {
    // Checks for pawns including pawns that can en peasant
    for (int i = 1; i >= 0; --i) {
        for (int j = -1; j < 2; j += 2) {
            if (pos.col + j < 0 || pos.col + j >= BOARD_SIZE) continue;
            const int row = pos.row - (i * (2 * isWhite - 1));
            if (board[row][pos.col + j] == 'P' + isWhite * 32) attackers[(*attackerCount)++] = (Position){row, pos.col + j};
        }
        if (previousMove.destination.row != pos.row
            || previousMove.destination.col != pos.col
            || previousMove.type != DOUBLEPAWNMOVE
        ) break;
    }
    // Search for all other types of pieces
    const int numberOfSearches = 5;
    const PieceType pieceList[numberOfSearches] = { ROOK, KNIGHT, BISHOP, QUEEN, KING };
    for (int i = 0; i < numberOfSearches; ++i) searchBoard(pieceList[i], FINDPIECES, pos, pieceList[i] + isWhite * 32, board, attackers, attackerCount, NULL);
    return *attackerCount > 0;
}

// Checks if making a move would put the player who made the move in check.
bool isPossibleMove(Board board, const Move move, const KingPosition *ownKingPos) {
    if (move.destination.row == move.origin.row && move.destination.col == move.origin.col) return false;
    const char destSquare = boardAt(move.destination);
    const bool isWhite = hasSameColor(move.pieceMoved, true);
    const MoveType type = move.type;
    Position positions[4];
    char pieces[4];
    int locationCount = 0;
    if (hasSameColor(destSquare, isWhite)) return false;
    if (destSquare != ' ' && !move.captures) return false;
    if (destSquare == ' ' && move.captures && type != ENPEASANT) return false;
    if (toupper(move.pieceMoved) == PAWN && move.captures && move.origin.col == move.destination.col) return false;
    if (type == CASTLELONG || type == CASTLESHORT) {
        locationCount = 4;
        const int cList[4] = { 4, 2 + 4 * (type == CASTLESHORT), 0 + 7 * (type == CASTLESHORT), 3 + 2 * (type == CASTLESHORT) };
        const char pList[4] = { isWhite ? 'K' : 'k', ' ', isWhite ? 'R' : 'r', ' ' };
        for (int i = 0; i < 4; ++i) {
            positions[i].row = isWhite ? 7 : 0;
            positions[i].col = cList[i];
            pieces[1] = pList[i];
        }
    } else {
        if (type == ENPEASANT) {
            ++locationCount;
            positions[2].row = move.destination.row - 1 + isWhite * 2;
            positions[2].col = move.destination.col;
            pieces[2] = 'P' + isWhite * 32;
        }
        locationCount += 2;
        pieces[0] = move.pieceMoved;
        positions[0] = move.origin;
        positions[1] = move.destination;
        pieces[1] = boardAt(move.destination);
    }
    if (tolower(move.pieceMoved) != 'n' && !hasClearSight(board, move.origin, move.destination)) return false;
    makeMove(board, move);
    Position problems[16];
    unsigned short int count = 0;
    isInCheck(board, tolower(move.pieceMoved) == 'k' ? move.destination : getRegPos(*ownKingPos), isWhite, move, problems, &count);
    for (int i = 0; i < locationCount; ++i) boardAt(positions[i]) = pieces[i];
    return !count;
}

void makeMove(Board board, const Move move) {
    boardAt(move.origin) = ' ';
    boardAt(move.destination) = move.type == PROMOTION ? move.promotionPiece : move.pieceMoved;
    switch (move.type) {
        case ENPEASANT:
            board[move.destination.row - 1 + ((move.pieceMoved == 'P') * 2)][move.destination.col] = ' ';
            break;
        case CASTLELONG:
            board[move.destination.row][0] = ' ';
            board[move.destination.row][3] = move.pieceMoved == 'K' ? 'R' : 'r';
            break;
        case CASTLESHORT:
            board[move.destination.row][7] = ' ';
            board[move.destination.row][5] = move.pieceMoved == 'K' ? 'R' : 'r';
        default:
            break;
    }
}

bool setMoveToCastle(Move *move, const MoveType type, const KingPosition kingPos) {
    if (type != CASTLESHORT && type != CASTLELONG) return false;
    const bool isWhite = hasSameColor(move->pieceMoved, false);
    if ((!kingPos.canCastleShort && type == CASTLESHORT) || (!kingPos.canCastleLong && type == CASTLELONG)) {
        printf("%s can't castle %s anymore.\n", isWhite ? WHITE_STR : BLACK_STR, type == CASTLESHORT ? "short" : "long");
        return false;
    }
    *move = (Move){ type, { isWhite ? 7 : 0, 4 }, { isWhite ? 7 : 0, type == CASTLESHORT ? 6 : 2 }, 'k' - isWhite * 32, false, ' ' };
    return true;
}

bool validateMove(char *const restrict move, GameState *const restrict state, bool *const restrict specifyRow, bool *const restrict specifyCol) {
    Move newMove;
    Board board = state->board;
    char piece = move[0];
    unsigned short int destinationIndex;
    bool shouldReturn = false, isWhite = state->status == WHITE;
    KingPosition kingPos = state->status == WHITE ? state->whiteKing : state->blackKing;
    if (piece == 'O') {
        bool isShort = strcmp(move, "O-O") == 0 || strcmp(move, "O-O+") == 0 || strcmp(move, "O-O#") == 0;
        bool isLong = strcmp(move, "O-O-O") == 0 || strcmp(move, "O-O-O+") == 0 || strcmp(move, "O-O-O#") == 0;
        if (!isShort && !isLong) return false;
        if ((isShort && !kingPos.canCastleShort) || (isLong && !kingPos.canCastleLong)) return false;
        Position regKingPos = getRegPos(kingPos);
        Position tmp[16];
        unsigned short int tmpCount = 0;
        for (int i = 0; i < 3; ++i) {
            if (isInCheck(board, regKingPos, isWhite, state->move, tmp, &tmpCount)) return false;
            regKingPos.col += isShort ? 1 : -1;
        }
        regKingPos.col -= isShort ? 3 : -3;
        if (!hasClearSight(board, regKingPos, (Position){ kingPos.row, isShort ? 7 : 0 })) return false;
        return setMoveToCastle(&(state->move), isShort ? CASTLESHORT : CASTLELONG, kingPos);
    }
    if (!findDestination(move, &newMove, &destinationIndex)) return false;
    const char destSquare = boardAt(newMove.destination);
    if (tolower(destSquare) == 'k') return false;
    if (isWhite && strchr("PNRBQ", destSquare)) return false;
    if (!isWhite && strchr("pnrbq", destSquare)) return false;
    newMove.type = NORMALMOVE;
    newMove.captures = false;
    int captureIndex = 0;
    for (; captureIndex < MAX_MOVE_SIZE; ++captureIndex) {
        if (move[captureIndex] != 'x') continue;
        if (captureIndex > 3) return false;
        if (move[captureIndex + 1] - 'a' != newMove.destination.col) return false;
        if ('8' - move[captureIndex + 2] != newMove.destination.row) return false;
        newMove.captures = true;
        break;
    }
    if (isCol(piece)) {
        newMove.pieceMoved = 'p' - isWhite * 32;
        newMove.origin.col = piece - 'a';
        int ogRow = newMove.destination.row - 1 + isWhite * 2;
        if (ogRow < 1 || ogRow > 6) return false;
        newMove.origin.row = ogRow;
        if (isRow(move[1])) {
            for (int i = 0; i < 2; ++i) {
                char square = board[ogRow][newMove.destination.col];
                if (square == newMove.pieceMoved) {
                    newMove.origin.row = ogRow;
                    if (i == 1) newMove.type = DOUBLEPAWNMOVE;
                    break;
                }
                if (newMove.destination.row - isWhite != 3 || i == 1) return false;
                ogRow += 2 * isWhite - 1;
            }
        } else if (captureIndex == 1) {
            if (board[ogRow][newMove.origin.col] != newMove.pieceMoved) return false;
            if (destSquare == ' ') {
                if (state->move.destination.row != newMove.destination.row - 1 + isWhite * 2
                    || state->move.destination.col != newMove.destination.col
                    || state->move.type != DOUBLEPAWNMOVE
                ) return false;
                newMove.type = ENPEASANT;
            }
        } else return false;
        int i = 1 + newMove.captures * 3;
        if (move[i] == '=') {
            if (newMove.destination.row != 7 - isWhite * 7) return false;
            if (!strchr("RNBQ", move[++i])) return false;
            newMove.type = PROMOTION;
            newMove.promotionPiece = move[i] + !isWhite * 32;
        }
        if ((newMove.destination.row == 0 || newMove.destination.row == 7) && newMove.type != PROMOTION) return 20;
    } else if (strchr("KNRBQ", piece)) {
        piece += !isWhite ? 32 : 0;
        newMove.pieceMoved = piece;
        Position candidates[8];
        unsigned short int candidateCount = 0;
        searchBoard(toupper(piece), FINDPIECES, newMove.destination, piece, board, candidates, &candidateCount, NULL);
        if (candidateCount > 1) {
            Position newCandidates[8];
            unsigned short int newCandidateCount = 0;
            Move testMove = { newMove.type, {0}, newMove.destination, newMove.pieceMoved, newMove.captures, ' ' };
            for (int i = 0; i < candidateCount; ++i) {
                testMove.origin = candidates[i];
                if (isPossibleMove(board, testMove, &kingPos)) newCandidates[newCandidateCount++] = testMove.origin;
            }
            if (newCandidateCount == 1) {
                newMove.origin = newCandidates[0];
                state->move = newMove;
                shouldReturn = true;
            } else if (newCandidateCount > 1) {
                if (destinationIndex <= 1) return false;
                candidateCount = 0;
                if (isCol(move[1])) {
                    for (int i = 0; i < newCandidateCount; ++i) {
                        if (newCandidates[i].col == move[1] - 'a') {
                            candidates[candidateCount++] = newCandidates[i];
                            *specifyCol = true;
                        }
                    }
                    if (candidateCount > 1) {
                        if (!isRow(move[2])) return false;
                        for (int i = 0; i < candidateCount; ++i) {
                            if (candidates[i].row == move[2] - '1') {
                                newMove.origin = candidates[i];
                                *specifyRow = true;
                                shouldReturn = true;
                            }
                        }
                        if (!shouldReturn) return false;
                    }
                } else if (isRow(move[1])) {
                    for (int i = 0; i < newCandidateCount; ++i) {
                        if (newCandidates[i].row != move[1] - '1') continue;
                        candidates[candidateCount++] = newCandidates[i];
                        *specifyRow = true;
                    }
                    if (candidateCount > 1) return false;
                    newMove.origin = candidates[0];
                    shouldReturn = true;
                } else return false;
            }
        }
        if (!shouldReturn && candidateCount == 1) {
            newMove.origin = candidates[0];
        } else if (!shouldReturn) return false;
    } else return false;
    if (!shouldReturn && !isPossibleMove(board, newMove, &kingPos)) return false;
    if (newMove.captures || toupper(newMove.pieceMoved) == PAWN) state->movesWithoutCaptures = 0;
    state->move = newMove;
    return true;
}

bool findDestination(char *const restrict rawMove, Move *const restrict move, unsigned short int *const restrict destinationIndex) {
    for (int i = 2; i < MAX_MOVE_SIZE; ++i) {
        if (isalnum(rawMove[i])) continue;
        if (!isRow(rawMove[--i])) return false;
        if (!isCol(rawMove[--i])) return false;
        *destinationIndex = i;
        move->destination.col = rawMove[i] - 'a';
        move->destination.row = '8' - rawMove[++i];
        return true;
    }
    return false;
}

bool initializeGameLog(GameLog *restrict game) {
    game->chunkCount = 0;
    Chunk *ptr = malloc(sizeof(Chunk));
    if (!ptr) {
        puts("\n[ERROR] Unable to start the game.");
        return false;
    }
    game->log = ptr;
    game->moveCounter = 0;
    return true;
}

bool resize(GameLog *restrict game) {
    Chunk *tmp = realloc(game->log, sizeof(Chunk) * (++game->chunkCount + 1));
    if (!tmp) {
        --game->chunkCount;
        puts("\n[ERROR] Unable to resize game log.");
        return false;
    }
    game->log = tmp;
    return true;
}

bool logMove(GameLog *game, GameState *state, const bool isCheck, const bool specifyRow, const bool specifyCol) {
    const Move move = state->move;
    const int dRow = move.destination.row, dCol = move.destination.col;
    if (move.type == RESIGN || move.type == PLAYERDRAW) return true;
    if (game->moveCounter >= 50) {
        if (!resize(game)) return false;
        game->moveCounter = 0;
    }
    char formatMove[MAX_MOVE_SIZE];
    int i = 0;
    if (move.type == CASTLESHORT) {
        strcpy(&(formatMove[i]), "O-O");
        i += 3;
    } else if (move.type == CASTLELONG) {
        strcpy(&(formatMove[i]), "O-O-O");
        i += 5;
    } else {
        char piece = toupper(move.pieceMoved);
        if (piece != PAWN) {
            formatMove[i++] = piece;
            if (specifyCol) formatMove[i++] = move.origin.col + 'a';
            if (specifyRow) formatMove[i++] = '8' - move.origin.row;
        }
        if (move.captures) {
            if (piece == PAWN) formatMove[i++] = move.origin.col + 'a';
            formatMove[i++] = 'x';
        }
        formatMove[i++] = dCol + 'a';
        formatMove[i++] = '8' - dRow;
        if (move.type == PROMOTION) {
            formatMove[i++] = '=';
            formatMove[i++] = toupper(state->board[dRow][dCol]);
        }
    }
    if (isCheck) formatMove[i++] = (state->status == WHITE_WON || state->status == BLACK_WON) ? '#' : '+';
    formatMove[i] = '\0';
    strcpy(game->log[game->chunkCount][game->moveCounter % 50], formatMove);
    ++game->moveCounter;
    return true;
}

void fileWriteFormatted(FILE *stream, char *format, ...) {
   va_list args;
   va_start(args, format);
   vfprintf(stream, format, args);
   va_end(args);
}

void createGameFile(GameLog * restrict game, GameStatus status) {
    const char *result = status == WHITE_WON ? "1-0" : status == BLACK_WON ? "0-1" : "1/2-1/2";
    char buffer[BUFFER_SIZE];
    int c;
    do {
        GET_INPUT("What name should the game file have? ")
    } while (0);
    for (c = 0; c <= BUFFER_SIZE - 5 && buffer[c]; ++c);
    strcpy(&buffer[c], ".pgn");
    FILE *file = fopen(buffer, "w");
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    fileWriteFormatted(file, "[Event \"?\"]\n[Site \"?\"]\n[Date \"%d.%02d.%02d\"]\n[EventDate \"?\"]\n[Round \"?\"]\n[Result \"", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    fputs(result, file);
    for (int i = 0; i < 2; ++i) {
        GET_INPUT("Enter the %s player's name: ", i == 0 ? WHITE_STR : BLACK_STR)
        fileWriteFormatted(file, "\"]\n[%s \"%s", i == 0 ? WHITE_STR : BLACK_STR, buffer);
    }
    fputs("\"]\n\n", file);
    for (int i = 0; i < game->chunkCount * CHUNK_SIZE + game->moveCounter; ++i) {
        if (i % 2 == 0) fileWriteFormatted(file, "%d.", 1 + i / 2);
        fputs(game->log[i / CHUNK_SIZE][i % CHUNK_SIZE], file);
        fputc(' ', file);
    }
    fputs(result, file);
    fclose(file);
    puts("The file was made successfully! :)");
}
