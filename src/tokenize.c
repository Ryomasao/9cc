#include "9cc.h"

// エラー箇所を報告するための関数
void error_at(char *loc, char *input, int line, char *msg) {
  // 走査中の文字のアドレスからユーザー入力開始位置のアドレスを引く
  // そうすると、走査中の文字が開始位置から何文字目か判別できる
  // ASCは1文字 1byte で16進数で表現するとaは0x61
  // 61は2進数で 01100001で
  int pos = loc - input;

  // debug
  // printf(" address of loc  %p \n", loc);
  // printf(" address of user_input  %p \n", user_input);

  fprintf(stderr, "%d行目\n", line + 1);
  fprintf(stderr, "%s\n", input);
  // pos分スペースを出力する
  // そうするとこんなエラーメッセージになる素敵
  // 123a1
  //    ^ トークナイズできません
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int line) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->line = line;
  cur->next = tok;
  return tok;
}

// 引数から変数名として使える文字列を取得する
int getLVarLength(char *p) {
  char *target = p;
  int length = 0;

  while('a' <= *target && *target <= 'z') {
    length++;
    target++;
  }
  return length;
}

// cがアルファベットか数字であれば1を返す
int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

// pが型である場合、型の文字列の長さを返す
// 型じゃない場合、-1を返す
TypeSymbol isType(char *p) {
  // 型の種類
  TypeSymbol types[] = {
    { "int", 3, TK_INT}, 
    { NULL, -1, -1 }
  };

  int i = 0;
  for(; types[i].tk != -1; i++) {
    int nameLen = types[i].nameLen;
    if(strncmp(p, types[i].name, nameLen) == 0 && !is_alnum(p[nameLen])) {
      return types[i];
    }
  }

  return types[i];
}

// 入力文字列pをトークナイズして、それを返す
void tokenize(char input[][MAX_COLUMN]) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  int line = 0;
  char *p;

  while(input[line][0] != EOF) {
    p = input[line];
    while(*p) {
      // 空白文字はskip
      if(isspace(*p)) {
        p++;
        continue;
      }

      // > と >= は >=を優先してトークナイズする
      if(*p == '>' && !memcmp(p, ">=", 2)) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 2;
        // 2文字進める
        p += 2;
        continue;
      }

      if(*p == '<' && !memcmp(p, "<=", 2)) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 2;
        p += 2;
        continue;
      }

      if(*p == '=' && !memcmp(p, "==", 2)) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 2;
        p += 2;
        continue;
      }

      if(*p == '!' && !memcmp(p, "!=", 2)) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 2;
        p += 2;
        continue;
      }

      if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
         *p == ')' || *p == '<' || *p == '>' || *p == '=' || *p == ';' ||
         *p == '{' || *p == '}' || *p == ',' || *p == '&') {
        cur = new_token(TK_RESERVED, cur, p++, line);
        cur->len = 1;
        continue;
      }

      if(isdigit(*p)) {
        // p++
        // しなくていいのかなとおもったけど、strtolが10進数以外の文字のとこまでのアドレスを
        // pに設定してくれている
        cur = new_token(TK_NUM, cur, p, line);
        cur->val = strtol(p, &p, 10);
        continue;
      }

      // 予約語
      // return
      if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 6;
        p = p + 6;
        continue;
      }

      if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 2;
        p = p + 2;
        continue;
      }

      if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 4;
        p = p + 4;
        continue;
      }

      if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 5;
        p = p + 5;
        continue;
      }

      if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
        cur = new_token(TK_RESERVED, cur, p, line);
        cur->len = 3;
        p = p + 3;
        continue;
      }

      // 型名
      TypeSymbol type = isType(p);

      if(type.name != NULL) {
        cur = new_token(type.tk, cur, p, line);
        cur->len = type.nameLen;
        p = p + type.nameLen;
        continue;
      }

      // 微妙だけど、予約語を変数名より優先して判定するようにする
      // 変数はaからzで始まっているものとする
      if('a' <= *p && *p <= 'z') {
        int length = getLVarLength(p);
        cur = new_token(TK_IDENT, cur, p, line);
        cur->len = length;
        p = p + length;
        continue;
      }

      error_at(p, input[line], line, "トークナイスできません");
    }
    line++;
  }
  new_token(TK_EOF, cur, p, line);
  token = head.next;
}