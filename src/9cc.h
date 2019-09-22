#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"

// トークンの型を表す値
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 変数 or 関数名
  TK_INT,      // int型
  TK_RETURN,   // return文
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークンの型
typedef struct Token {
  TokenKind kind;     // トークンの型
  struct Token *next; // 次の入力トークン
  int val;            // kindがTK_NUMの場合、その数値
  char *str;          // トークン文字列 (エラーメッセージ用)
  int len; // トークンの長さ kindがTK_RESERVEDのときのみ桁数をセット
  int line; // トークンに対応するコードの行
} Token;

// 現在着目しているトークン
Token *token;

// 型を管理する構造体
typedef struct TypeSymbol {
  char *name;  // 型名 ex) int
  int nameLen; // 型名の長さ
  int tk;      // 型に対応するtokenの種類
} TypeSymbol;

typedef struct Lvar {
  struct Lvar *next; // 次の変数かNULL
  char *name;        // 変数の名前
  int len;           // 名前の長さ
  int offset;        // RBPからのオフセット
} Lvar;

// ローカル変数
Lvar *locals[100];
int functionId;

typedef struct Type {
  enum { INT, PTR } ty;
  struct Type *ptr_to;
} Type;

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,          // +
  ND_SUB,          // -
  ND_MUL,          // *
  ND_DIV,          // /
  ND_LT,           // <
  ND_LTE,          // <=
  ND_EQ,           // ==
  ND_NEQ,          // !=
  ND_NUM,          // 整数
  ND_ASSIGN,       // =
  ND_LVAR,         // ローカル変数
  ND_RETURN,       // return
  ND_IF,           // IF
  ND_IF_ELSE,      // IFでELSEがあるパターン
  ND_IF_ELSE_STMT, // if文のthenとstmtとelseのstmtをもつ
  ND_WHILE,        // while
  ND_FOR,          // ND_FOR
  ND_FOR_CONTINUE, // ND_FORの継続条件
  ND_FOR_LOOP,     // ND_FORの更新
  ND_FOR_STMT,     // ND_FORのSTMT
  ND_BLOCK,        // {}のブロック構文
  ND_FUNC,         // 関数call
  ND_FUNC_DIF,     // 関数定義
  ND_FUNC_DIF_END, // 関数の終了 } が格納
  ND_ADDR,         // & 変数のアドレスを取得
  ND_DEREF, // * 変数の値をアドレスをみなして、そのアドレスの値を取得
  ND_INT    // INT型
} NodeKind;

// Node型の中にNodeがある
// typef struct
// Node　としておくと、lhsとかでNode型がわからないことによるwarningが消えた
typedef struct Node {
  NodeKind kind;    // 演算子かND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // kindがND_NUMの場合のみ使う
  int offset; // kindがND_LVARの場合のみ使う。変数名に応じてスタックのアドレスを静的に決める
  Type *
      lvarType; // kindがND_LVARの場合のみ使う。その変数がポインタ型かINT型かを管理する。
  struct Node *
      *vector; // kindがND_BLOCKの場合、stmtのnode保持する配列へのポインタ
  char *funcName;       // kindがND_FUNCの場合の関数名
  struct Node *argv[3]; // kindがND_FUNCの場合の引数
  int argc;             // kindがND_FUNCの場合の引数の数
} Node;

// ファイル読み込み
#define MAX_LINE 100
#define MAX_COLUMN 80
int readFile(char *path, char input[][MAX_COLUMN]);

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
