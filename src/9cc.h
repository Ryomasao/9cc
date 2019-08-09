#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_RETURN,    // return文
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

// トークンの型
typedef struct Token {
  TokenKind kind;        // トークンの型
  struct Token *next;    // 次の入力トークン 
  int val;               // kindがTK_NUMの場合、その数値
  char *str;             // トークン文字列 (エラーメッセージ用)
  int len;               // トークンの長さ kindがTK_RESERVEDのときのみ桁数をセット
} Token;

// 現在着目しているトークン
Token *token;

typedef struct Lvar {
  struct Lvar *next;   // 次の変数かNULL
  char *name;   // 変数の名前
  int len;      // 名前の長さ
  int offset;   // RBPからのオフセット
} Lvar;

// ローカル変数
Lvar *locals;

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,           // +
  ND_SUB,           // -
  ND_MUL,           // *
  ND_DIV,           // /
  ND_LT,            // <
  ND_LTE,           // <=
  ND_EQ,            // ==
  ND_NEQ,           // !=
  ND_NUM,           // 整数
  ND_ASSIGN,        // =
  ND_LVAR,          // ローカル変数
  ND_RETURN,        // return
  ND_IF,            // IF
  ND_IF_ELSE,       // ELSE
  ND_IF_ELSE_STMT,  // if文のthenとstmtとelseのstmtをもつ
} NodeKind;

// Node型の中にNodeがある
// typef struct Node　としておくと、lhsとかでNode型がわからないことによるwarningが消えた
typedef struct Node
{
  NodeKind kind;    // 演算子かND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // kindがND_NUMの場合のみ使う
  int offset;       // kindがND_LVARの場合のみ使う。変数名に応じてスタックのアドレスを静的に決める
} Node;


// ファイル読み込み
#define MAX_LINE 100
#define MAX_COLUMN 80
int readFile(char *path, char input[][MAX_COLUMN]);

// 入力プログラム
char *user_input;

// parse機能
void tokenize(char input[][MAX_COLUMN]);
void program();
Node *expr();
Node *stmt();
// コードジェネレータ
void gen();

// エラー関数
void error(char *fmt, ...);

Node *code[100];
