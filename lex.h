#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ada.h"

//Lexical elements----------------------------------
// numeric | comment | strings | identifier | specialKey(Op) | CHAR
//--------------------------------------------------

int nbl=1;
int nbc=1;
int savenbc;



//Translate macro value to macro name:
void getMacro(int type) {

  if(type == EOF) {
    printf("T_EOF ");
    return;
  }
  if(type == '\'') {
    printf("T_QUOTE ");
    return;
  }
  FILE *f = fopen("ada.h" , "r");
  char buffer[400];
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
       char identifier[400];
       if(type >= 257 && type <= 804) {
         int valueP;
         if (sscanf (buffer, "#define %s %d", identifier, &valueP) == 2) {
           if(type == valueP) {
             printf("%s\n", identifier);
             return;
           }
         }
       }else {
         char valueC;
         if (sscanf (buffer, "#define %s '%c'", identifier, &valueC) == 2) {
           if(type == valueC) {
             printf("%s\n", identifier);
             return;
           }
         }
       }
    }
}
//Check if str is a reserved keyword
int isReserved(char * strn) {
  FILE *f = fopen("ada.h" , "r");
  char buffer[400];
  while (fgets(buffer, sizeof(buffer), f) != NULL) {
       char identifier[400];
       int valueP;
       if (sscanf (buffer, "#define T_%s %d", identifier, &valueP) == 2) {
           if(strcmp(strn, identifier) == 0) {
             return valueP;
           }
       }
    }
    return 0;
}

void rmSpace() {
  char c = fgetc(fp);
  while(c == ' ' || c == '\n'|| c == '\t'|| c == '\r'){
  	if(c=='\n'){nbl++;nbc=1;}
  	else {savenbc=nbc;nbc++;}
  	c = fgetc(fp);
  }

  ungetc(c, fp);
  

}

// operators:---------------------------- detects also comments

int comment();
void operators() {
  rmSpace();
  memset(str, '\0' , 1024);
  i = 0;
  char c = fgetc(fp);savenbc=nbc++;

  if (c == EOF) {
    token.type = c;
    strcpy(token.val.stringValue , "EOF");
    return;
  }
  if(c == '\'') {
    str[i] = c; i++;
    char cc  = fgetc(fp);savenbc=nbc++;
    str[i] = cc; i++;
    c  = fgetc(fp);savenbc=nbc++;
    if(c == '\'') {
      str[i] = c; i++;
      token.type = 334;
      strcpy(token.val.stringValue,str);
      memset(str, '\0' , 1024);
      return;

    }else {
      ungetc(c,fp);
      if(nbc>1)nbc--;
  	  else {nbl--;nbc=savenbc;}
      ungetc(cc,fp);
      if(nbc>1)nbc--;
  	  else {nbl--;nbc=savenbc;}

    }
  }
  if(c == '<' || c == '>' || c == '/' || c == ':' || c == '*' || c == '-') {
    str[i] = c; i++;
    char cc  = fgetc(fp);savenbc=nbc++;
    if(cc == '=') {
      str[i] = cc; i++;

      if(strcmp(":=",str) == 0) token.type = 800;
      else if(strcmp("<=",str) == 0) token.type = 801;
      else if(strcmp(">=",str) == 0) token.type = 802;
      else if(strcmp("/=",str) == 0) token.type = 803;
      strcpy(token.val.stringValue,str);
    }
    else if(c == cc && cc == '*'){
      str[i] = cc; i++;
      token.type = 804;
      strcpy(token.val.stringValue,str);
    } else if(cc == '-') {
      ungetc(cc,fp);
      if(nbc>1)nbc--;
 	  else {nbl--;nbc=savenbc;}
      ungetc(c,fp);
      if(nbc>1)nbc--;
  	  else {nbl--;nbc=savenbc;}
      comment();
    }
    else {
      ungetc(cc,fp);
      if(nbc>1)nbc--;
  	  else {nbl--;nbc=savenbc;}
      token.type = c;
      strcpy(token.val.stringValue,str);
    }

  }
  else {
    switch (c) {
      case ',' :  case '.': case ';' : case '+' : case '-' : case '=' :
      case '/' : case '(' : case ')' : case '*' : case '&': case '\'':
                str[i] = c; i++;
                token.type = c;
                strcpy(token.val.stringValue,str);
                break;
      default:
              ungetc(c,fp);
              if(nbc>1)nbc--;
  			  else {nbl--;nbc=savenbc;}
    }
  }
  memset(str, '\0' , 1024);
}
// T_NUMERIC
// numeric_literal ::= decimal_literal [| based_literal]
//decimal_literal ::= numeral [ "." numeral ] [ exponent ]
//exponent ::= ( "E" [ "+" ] numeral ) | ( "E" "-" numeral )


// decimal_literal ::= numeral [. numeral] [exponent]
// numeral ::= digit { [ "_" ] digit }
// numeral ::= digit ([ "_" ] digit )*
// numeral ::= digit digits
// digits ::=  digit digits | _digit digits | e

int digits() {
  char c = fgetc(fp);nbc++;

  if(isdigit(c)) {
    str[i] = c; i++;
    digits();
  }
  else if(c == '_') {
    str[i] = c; i++;
    c = fgetc(fp);savenbc=nbc++;
    str[i] = c; i++;
    if(isdigit(c)) {
      digits();
    }else {
      int j;
      for (j = strlen(str) - 1; j >= 0; j--) {
        ungetc(str[j],fp);
        if(nbc>1)nbc--;
        else {nbl--;nbc=savenbc;}
      }
      return 0;
    }
  }
  else {
    ungetc(c,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
    return 1;
  }

}

int numeral() {
  int result = 1;
  char c = fgetc(fp);savenbc=nbc++;
  if(isdigit(c)) {
    str[i] = c; i++;
    result = digits();
  }
  else {
    ungetc(c,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
    return 0;
  }
  return result;
}
void decimal_literal() {
  rmSpace();
  i = 0;
  int result = numeral();
  int in = 0;
  int clear = 0;
  char c = fgetc(fp);savenbc=nbc++;


  //traitement des Label !!-----------------------------------------------------------
  if(c=='<'){
      c = fgetc(fp);savenbc=nbc++;
      if(c=='<'){
          int i=0;
          c=fgetc(fp);savenbc=nbc++;
          while(c!=' ' && c!='\r' && c!='\n' ){token.val.stringValue[i]=c;c=fgetc(fp);savenbc=nbc++;i++;}
          if(c=='\n'){
          	nbc=savenbc;
          	nbl++;
          }
          if(token.val.stringValue[i-1]!='>' || token.val.stringValue[i-1]!='>')printf("\033[1;33mLabel must finish with : >>");exit(-1);
          token.val.stringValue[i-2]='\0';
          label();
          decimal_literal();
      }
      else {ungetc(c,fp);c='<';if(nbc>1)nbc--;else {nbl--;nbc=savenbc;}}
  }
  //-----------------------------------------------------------------------------------

  if(c != 'E' && c!= '.' && result) ungetc(c,fp);
  if(c == '.' && result) {
    str[i] = c; i++;
    in = 1;
    if(!numeral()) {
      ungetc(c,fp);
      if(nbc>0)nbc--;
      else {nbl--;nbc=savenbc;}
      str[i-1] ='\0';
    };
  }
  if(c == 'E' && result) {
    str[i] = c; i++;
    in = 1;
    c = fgetc(fp);savenbc=nbc++;
    if(c == '+' || c == '-')  {
      str[i] = c; i++;
    }else {ungetc(c,fp);if(nbc>1)nbc--;else {nbl--;nbc=savenbc;}}
    if(!numeral()){
      clear = 1;
      result = 0;
    }
  }
  if(result == 1) {
    token.type = T_NUMERIC;
    strcpy(token.val.stringValue,str);
  }
  else {
    if(clear == 1) {
      int j;
      for(j = strlen(str) - 1; j >= 0; j--) {
        ungetc(str[j],fp);
        if(nbc>1)nbc--;
        else {nbl--;nbc=savenbc;};
      }
    }else {ungetc(c,fp);if(nbc>1)nbc--;else {nbl--;nbc=savenbc;}}
  }
  i = 0;
  memset(str, '\0' , 1024);

}




// comment ::= --.*\n
int comment() {
  rmSpace();
  str = calloc(1024 , sizeof(char));
  i = 0;
  char c = fgetc(fp);savenbc=nbc++;
  if(c == '-'){
    str[i] = c; i++;
    c = fgetc(fp);savenbc=nbc++;
    if(c == '-') {
      str[i] = c; i++;
      while(c != '\n') {
        c = fgetc(fp);savenbc=nbc++;
        str[i] = c; i++;
      }
      nbl++;
      nbc=1;
      //str[i] = '\0';
      token.type = T_COMMENT;
      strcpy(token.val.stringValue,str);
    }
    else {
      ungetc(c,fp);
      if(nbc>1)nbc--;
      else {nbl--;nbc=savenbc;}
      str[i] = '\0';
    }
  }else{
    ungetc(c,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
  }
  memset(str, '\0' , 1024);
}

// string_literal ::= "quotation mark" { string_element } "quotation mark"
// string_element ::= "pair of quotation mark" | graphic_character

int string_element() {
  char c = fgetc(fp);
  if(c == '"'){
    str[i] = c; i++;
    c = fgetc(fp);savenbc=nbc++;
    if(c == '"'){
      str[i] = c; i++;
      string_element();
    }else {
      i = i-1;
      ungetc(c,fp);
      if(nbc>1)nbc--;
      else {nbl--;nbc=savenbc;}
      ungetc('"',fp);
      if(nbc>1)nbc--;
      else {nbl--;nbc=savenbc;}
      return -1;
    }
  }
  else{
    while(c != '"' && c != '\n') {
      str[i] = c; i++;
      c = fgetc(fp);savenbc=nbc++;
    }
    ungetc(c,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
    if(c == '"') {
      string_element();
    }

  }
}


int string_literal() {
    rmSpace();
    i = 0;
    char c = fgetc(fp);savenbc=nbc++;
    if(c == '"') {
      str[i] = c; i++;
      string_element();
      c = fgetc(fp);savenbc=nbc++;

      if(c == '"') {
        str[i] = c; i++;
        token.type = T_STRING;
        strcpy(token.val.stringValue,str);
        memset(str, '\0' , 1024);
        return 1;
      }
      else {
        int j;
        for(j = strlen(str); j >= 0; j--) {
          ungetc(str[j],fp);
          if(nbc>1)nbc--;
          else {nbl--;nbc=savenbc;}
        }
      }
    }
    ungetc(c,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
    memset(str, '\0' , 1024);
    return -1;
}



//id ::= letter {["_"] (letter | digit)}
//identifier() verify identifier & reserved Keywords
int identifier_name() {
  rmSpace();
  i = 0;
  char c = fgetc(fp);savenbc=nbc++;
  if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
    str[i] = c; i++;
    c = fgetc(fp);savenbc=nbc++;
    while((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || isdigit(c)) {
      str[i] = c; i++;
      c = fgetc(fp);savenbc=nbc++;
      if(c == '_') {
        str[i] = c; i++;
        c = fgetc(fp);savenbc=nbc++;
      }
      else {
        continue;
      }
    }
    int valueP = isReserved(str);
    if(valueP) token.type = valueP;
    else token.type = T_IDENTIFIER;
    strcpy(token.val.stringValue,str);
    ungetc(c ,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
  }
  else {
    ungetc(c ,fp);
    if(nbc>1)nbc--;
    else {nbl--;nbc=savenbc;}
  }
  memset(str, '\0' , 1024);
}

int scanToken() {

  if(fp == NULL) return -1;
  token.type = T_UNKNOWN;
  memset(token.val.stringValue , '\0' , 100);
  if(token.type == T_UNKNOWN) decimal_literal();
  if(token.type == T_UNKNOWN) identifier_name();
  if(token.type == T_UNKNOWN) string_literal();
  if(token.type == T_UNKNOWN) operators();
  //printf("-> ->");getMacro(token.type);printf("\n");
  //printf("-> %s \n", token.val.stringValue);

  //Ignoring comments
  if(token.type == T_COMMENT) scanToken();

}
