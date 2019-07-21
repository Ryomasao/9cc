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
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_LT,     // <
  ND_LTE,    // <=
  ND_EQ,     // ==
  ND_NEQ,    // !=
  ND_NUM,    // 整数
  ND_ASSIGN, // =
  ND_LVAR,   // ローカル変数
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


// トークンの型
typedef struct Token {
  TokenKind kind;        // トークンの型
  struct Token *next;    // 次の入力トークン 
  int val;               // kindがTK_NUMの場合、その数値
  char *str;             // トークン文字列 (エラーメッセージ用)
  int len;               // トークンの長さ kindがTK_RESERVEDのときのみ桁数をセット
} Token;


// 入力プログラム
char *user_input;

// parse機能
void tokenize(char *p);
void program();
// コードジェネレータ
void gen();

// エラー関数
void error(char *fmt, ...);

Node *code[100];
