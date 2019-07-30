#include "9cc.h"

// エラー箇所を報告するための関数
void error_at(char *loc, char*msg) {
  // 走査中の文字のアドレスからユーザー入力開始位置のアドレスを引く
  // そうすると、走査中の文字が開始位置から何文字目か判別できる
  // ASCは1文字 1byte で16進数で表現するとaは0x61
  // 61は2進数で 01100001で
  int pos = loc - user_input;

  // debug
  //printf(" address of loc  %p \n", loc);
  //printf(" address of user_input  %p \n", user_input);

  fprintf(stderr, "%s\n", user_input);
  // pos分スペースを出力する
  // そうするとこんなエラーメッセージになる素敵
  // 123a1
  //    ^ トークナイズできません
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 引数から変数名として使える文字列を取得する
int getLVarLength(char *p) {
  char *target = p;
  int length = 0;

  while('a' <= *target && *target <='z') {
    length++;
    target++;
  }
  return length;
}

// 入力文字列pをトークナイズして、それを返す
void tokenize(char *p) {

  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p) {
    // 空白文字はskip
    if(isspace(*p)) {
      p++;
      continue;
    }

    // > と >= は >=を優先してトークナイズする
    if(*p ==  '>' && !memcmp(p, ">=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      // 2文字進める
      p += 2;
      continue;
    }

    if(*p ==  '<' && !memcmp(p, "<=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p ==  '=' && !memcmp(p, "==", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p ==  '!' && !memcmp(p, "!=", 2)) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if(*p == '+' || 
       *p == '-' || 
       *p == '*' || 
       *p == '/' || 
       *p == '(' || 
       *p == ')' ||
       *p == '<' ||
       *p == '>' ||
       *p == '=' || 
       *p == ';'
      ) 
    {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if(isdigit(*p)) {
      // p++ しなくていいのかなとおもったけど、strtolが10進数以外の文字のとこまでのアドレスを
      // pに設定してくれている
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    // 変数はaからzで始まっているものとする
    if('a' <= *p && *p <='z') {
      int length = getLVarLength(p);
      cur = new_token(TK_IDENT, cur, p);
      cur->len = length;
      p = p + length;
      continue;
    }

    error_at(p, "トークナイスできません");
}

  new_token(TK_EOF, cur, p);
  token = head.next;
}