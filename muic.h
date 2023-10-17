/*
muic.h - Muukid and wk1093
Public domain simple single-file C library for compiling C99 into bytecode, and running the bytecode.
No warranty implied; use at your own risk.

Licensed under MIT License or public domain, whichever you prefer.
More explicit license information at the end of file.
*/
#ifndef MUIC_H
#define MUIC_H

#define MUIC_VERSION_MAJOR 0
#define MUIC_VERSION_MINOR 1
#define MUIC_VERSION_PATCH 0

//#ifdef __cplusplus TODO: uncomment
//extern "C" {
//#endif

#ifndef MUICDEF
#ifdef MUIC_STATIC
#define MUICDEF static
#else
#define MUICDEF extern
#endif
#endif

#include "mus.h" // like std::string for C

typedef enum {
    MUIC_STATUS_OK,
    MUIC_STATUS_LEX_ERROR,
    MUIC_STATUS_PARSE_ERROR,
    MUIC_STATUS_GEN_ERROR,
} MUIC_STATUS;

typedef struct {
    unsigned char* bytecode;
    size_t size;
} muic_bytecode;

typedef struct {
    muic_bytecode bytecode;
    MUIC_STATUS status;
    mustring error;
} muic_compile_output;

#define tk(x) MUIC_TOKEN_KW_##x
#define t(x) MUIC_TOKEN_##x
#define tc(x) MUIC_TOKEN_CHAR_##x
typedef enum {
    tk(AUTO),
    tk(BOOL),
    tk(BREAK),
    tk(CASE),
    tk(CHAR),
    tk(COMPLEX),
    tk(CONST),
    tk(CONTINUE),
    tk(DEFAULT),
    tk(DO),
    tk(DOUBLE),
    tk(ELSE),
    tk(ENUM),
    tk(EXTERN),
    tk(FLOAT),
    tk(FOR),
    tk(GOTO),
    tk(IF),
    tk(IMAGINARY),
    tk(INLINE),
    tk(INT),
    tk(LONG),
    tk(REGISTER),
    tk(RESTRICT),
    tk(RETURN),
    tk(SHORT),
    tk(SIGNED),
    tk(SIZEOF),
    tk(STATIC),
    tk(STRUCT),
    tk(SWITCH),
    tk(TYPEDEF),
    tk(UNION),
    tk(UNSIGNED),
    tk(VOID),
    tk(VOLATILE),
    tk(WHILE),
    t(IDENTIFIER),
    t(CONSTANT),
    t(STRING_LITERAL),
    t(ELLIPSIS),
    t(RIGHT_ASSIGN),
    t(LEFT_ASSIGN),
    t(ADD_ASSIGN),
    t(SUB_ASSIGN),
    t(MUL_ASSIGN),
    t(DIV_ASSIGN),
    t(MOD_ASSIGN),
    t(AND_ASSIGN),
    t(XOR_ASSIGN),
    t(OR_ASSIGN),
    t(RIGHT_OP),
    t(LEFT_OP),
    t(INC_OP),
    t(DEC_OP),
    t(PTR_OP),
    t(AND_OP),
    t(OR_OP),
    t(LE_OP),
    t(GE_OP),
    t(EQ_OP),
    t(NE_OP),
    tc(SEMI),
    tc(LBRACE),
    tc(RBRACE),
    tc(COMMA),
    tc(COLON),
    tc(EQUAL),
    tc(LPAREN),
    tc(RPAREN),
    tc(LBRACKET),
    tc(RBRACKET),
    tc(DOT),
    tc(AMPERSAND),
    tc(BANG),
    tc(TILDE),
    tc(MINUS),
    tc(PLUS),
    tc(STAR),
    tc(SLASH),
    tc(PERCENT),
    tc(LT),
    tc(GT),
    tc(CARET),
    tc(BAR),
    tc(QUESTION),
} MUIC_TOKEN_TYPE;
#undef t
#undef tk
#undef tc


typedef struct {
    MUIC_TOKEN_TYPE type;
    mustring value;
} muic_token;

typedef struct {
    muic_token* tokens;
    size_t size;
} muic_tokens;

typedef struct {
    muic_token* tokens;
    size_t size;
    size_t capacity;
} muic_token_buffer;

typedef struct {
    muic_tokens tokens;
    MUIC_STATUS status;
    mustring error;
} muic_lex_output;

typedef struct {
    mustring code;
    size_t index;
} muic_lexer;


MUICDEF muic_compile_output muic_compile(mustring code);
MUICDEF int muic_run(muic_bytecode bytecode);

MUICDEF muic_lexer muic_lexer_create(mustring code);
MUICDEF muic_lex_output muic_lex(muic_lexer lexer);
MUICDEF char muic_lexer_peek(muic_lexer* lexer);
MUICDEF char muic_lexer_consume(muic_lexer* lexer);

MUICDEF muic_token_buffer muic_token_buffer_create();
MUICDEF void muic_token_buffer_push(muic_token_buffer* buffer, muic_token token);


//#ifdef __cplusplus
//}
//#endif

#endif

#ifdef MUIC_IMPLEMENTATION

//#ifdef __cplusplus TODO: uncomment
//extern "C" {
//#endif

#ifndef muic_malloc
#ifndef STDLIB_H_INCLUDED
#include <stdlib.h>
#endif
#define muic_malloc malloc
#endif

#ifndef muic_isalpha
#ifndef CTYPE_H_INCLUDED
#include <ctype.h>
#endif
#define muic_isalpha isalpha
#endif

#ifndef muic_isalnum
#ifndef CTYPE_H_INCLUDED
#include <ctype.h>
#endif
#define muic_isalnum isalnum
#endif

#ifndef muic_isdigit
#ifndef CTYPE_H_INCLUDED
#include <ctype.h>
#endif
#define muic_isdigit isdigit
#endif

#ifndef muic_strcmp
#ifndef STRING_H_INCLUDED
#include <string.h>
#endif
#define muic_strcmp strcmp
#endif


MUICDEF muic_token_buffer muic_token_buffer_create() {
    muic_token_buffer buffer = {0};
    buffer.capacity = 16;
    buffer.tokens = muic_malloc(sizeof(muic_token) * buffer.capacity);
    return buffer;
}

MUICDEF void muic_token_buffer_push(muic_token_buffer* buffer, muic_token token) {
    if (buffer->size >= buffer->capacity) {
        buffer->capacity *= 2;
        buffer->tokens = muic_malloc(sizeof(muic_token) * buffer->capacity);
    }
    buffer->tokens[buffer->size++] = token;
}


MUICDEF muic_lexer muic_lexer_create(mustring code) {
    muic_lexer lexer = {0};
    lexer.code = code;
    lexer.index = 0;
    return lexer;
}

MUICDEF char muic_lexer_peek(muic_lexer* lexer) {
    if (lexer->index >= lexer->code.size) {
        return 0;
    }
    return lexer->code.s[lexer->index];
}

MUICDEF char muic_lexer_consume(muic_lexer* lexer) {
    if (lexer->index >= lexer->code.size) {
        return 0;
    }
    return lexer->code.s[lexer->index++];
}

MUICDEF muic_lex_output muic_lex(muic_lexer lexer) {
    char buffer[256] = {0};
    size_t buffer_index = 0;
    muic_token_buffer tokens = muic_token_buffer_create();

    while (muic_lexer_peek(&lexer)) {
        if (muic_isalpha(muic_lexer_peek(&lexer)) || muic_lexer_peek(&lexer) == '_') {
            // append char to end of buffer
            buffer[buffer_index++] = muic_lexer_consume(&lexer);
            while (muic_isalnum(muic_lexer_peek(&lexer)) || muic_lexer_peek(&lexer) == '_') {
                buffer[buffer_index++] = muic_lexer_consume(&lexer);
            }
            buffer[buffer_index] = 0;
#define kw(b, a) if (muic_strcmp(buffer, b) == 0) { \
                muic_token tok = {MUIC_TOKEN_KW_##a, mus_string_create(buffer)}; \
                muic_token_buffer_push(&tokens, tok);                            \
            }
#define ekw(b, a) else kw(b, a)
            kw("auto", AUTO)
            ekw("_Bool", BOOL)
            ekw("break", BREAK)
            ekw("case", CASE)
            ekw("char", CHAR)
            ekw("_Complex", COMPLEX)
            ekw("const", CONST)
            ekw("continue", CONTINUE)
            ekw("default", DEFAULT)
            ekw("do", DO)
            ekw("double", DOUBLE)
            ekw("else", ELSE)
            ekw("enum", ENUM)
            ekw("extern", EXTERN)
            ekw("float", FLOAT)
            ekw("for", FOR)
            ekw("goto", GOTO)
            ekw("if", IF)
            ekw("_Imaginary", IMAGINARY)
            ekw("inline", INLINE)
            ekw("int", INT)
            ekw("long", LONG)
            ekw("register", REGISTER)
            ekw("restrict", RESTRICT)
            ekw("return", RETURN)
            ekw("short", SHORT)
            ekw("signed", SIGNED)
            ekw("sizeof", SIZEOF)
            ekw("static", STATIC)
            ekw("struct", STRUCT)
            ekw("switch", SWITCH)
            ekw("typedef", TYPEDEF)
            ekw("union", UNION)
            ekw("unsigned", UNSIGNED)
            ekw("void", VOID)
            ekw("volatile", VOLATILE)
            ekw("while", WHILE)
#undef kw
#undef ekw
            else {
                muic_token tok = {MUIC_TOKEN_IDENTIFIER, mus_string_create(buffer)};
                muic_token_buffer_push(&tokens, tok);
            }
            buffer_index = 0;\
        } else if (muic_isdigit(muic_lexer_peek(&lexer))) {
            // TODO: 0x stuff
            // TODO: IS and FS
            buffer[buffer_index++] = muic_lexer_consume(&lexer);
            while (muic_isdigit(muic_lexer_peek(&lexer))) {
                buffer[buffer_index++] = muic_lexer_consume(&lexer);
            }
            if (muic_lexer_peek(&lexer) == '.') {
                buffer[buffer_index++] = muic_lexer_consume(&lexer);
                while (muic_isdigit(muic_lexer_peek(&lexer))) {
                    buffer[buffer_index++] = muic_lexer_consume(&lexer);
                }
                if (muic_lexer_peek(&lexer) == 'f' || muic_lexer_peek(&lexer) == 'F') {
                    buffer[buffer_index++] = muic_lexer_consume(&lexer);
                }
            }
            muic_token tok = {MUIC_TOKEN_CONSTANT, mus_string_create(buffer)};
            muic_token_buffer_push(&tokens, tok);
            buffer_index = 0;
        } // TODO: multi-char symbols
        // TODO: single char symbols
        // TODO: other types of literals
    }
}

MUICDEF muic_compile_output muic_compile(mustring code) {
    muic_compile_output output = {0};

    // lex
    muic_lexer lexer = muic_lexer_create(code);
    muic_lex_output lex_output = muic_lex(lexer);
    if (lex_output.status != MUIC_STATUS_OK) {
        output.status = lex_output.status;
        output.error = lex_output.error;
        return output;
    }
//    muic_parser parser = muic_parser_create(lex_output.tokens);
//    muic_parse_output parse_output = muic_parse(parser);
//    if (parse_output.status != MUIC_STATUS_OK) {
//        output.status = parse_output.status;
//        output.error = parse_output.error;
//        return output;
//    }
//    muic_generator generator = muic_generator_create(parse_output.ast);
//    muic_gen_output gen_output = muic_gen(generator);
//    if (gen_output.status != MUIC_STATUS_OK) {
//        output.status = gen_output.status;
//        output.error = gen_output.error;
//        return output;
//    }
//    output.bytecode = gen_output.bytecode;
//    output.status = MUIC_STATUS_OK;
    return output;


}

//#ifdef __cplusplus
//}
//#endif

#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2023 Samuel Carmichael
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/