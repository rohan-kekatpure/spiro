/*
ppedit - A pattern plate editor for Spiro splines.
Copyright (C) 2007 Raph Levien

Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
<LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
option. This file may not be copied, modified, or distributed
except according to those terms.

*/
#include <stdio.h>
#include <math.h>

#include "sexp.h"

/* This is handcoded to avoid locale problems. */
static int
parse_double(sexp_reader *sr)
{
    double sign = 1.0, val = 0.0;
    int i;
    int numstart;
    const char * const b = sr->tokbuf;
    int is_double = 1;

    i = 0;
    if (b[i] == '+') {
        i++;
    } else if (b[i] == '-') {
        sign = -1.0;
        i++;
    }
    numstart = i;
    while (b[i] >= '0' && b[i] <= '9')
        val = val * 10.0 + b[i++] - '0';
    if (b[i] == '.') {
        double frac = 1.0;

        for (i++; b[i] >= '0' && b[i] <= '9'; i++) {
            frac *= 0.1;
            val += (b[i] - '0') * frac;
        }

        /* A '.' without any digits on either side isn't valid. */
        if (i == numstart + 1)
            is_double = 0;
    }
    if (b[i] == 'e' || b[i] == 'E') {
        int expsign = 1, exp = 0;
        int expstart;

        if (b[i] == '+') {
            i++;
        } else if (b[i] == '-') {
            expsign = -1;
            i++;
        }
        expstart = i;
        while (b[i] >= '0' && b[i] <= '9')
            exp = exp * 10 + b[i++] - '0';

        if (i == expstart)
            is_double = 0;
        val *= pow(10.0, expsign * exp);
    }
    val *= sign;
    sr->d = val;
    if (b[i] != 0) is_double = 0;
    sr->is_double = is_double;
    return is_double;
}

/* Return values: 0 = EOF, 1 = token but not double, 2 = valid double */
int
sexp_token(sexp_reader *sr)
{
    int c;
    int i;

    sr->singlechar = -1;
    if (sr->f == NULL)
        return 0;

    for (;;) {
        c = getc(sr->f);
        if (c == EOF) {
            sr->f = NULL;
            return 0;
        } else if (c == '#') {
            do {
                c = getc(sr->f);
            } while (c != EOF && c != '\r' && c != '\n');
        } else if (c != ' ' && c != '\r' && c != '\n' && c != '\t')
            break;
    }
    sr->tokbuf[0] = c;
    i = 1;
    if (c != '(' && c != ')') {
        for (;;) {
            c = getc(sr->f);
            if (c == EOF) {
                sr->f = NULL;
                break;
            } else if (c == ' ' || c == '\r' || c == '\n' || c == '\t') {
                break;
            } else if (c == '(' || c == ')' || c == '#') {
                ungetc(c, sr->f);
                break;
            } else if (i < sizeof(sr->tokbuf) - 1)
                sr->tokbuf[i++] = c;
        }
    }
    sr->tokbuf[i] = 0;
    if (i == 1)
        sr->singlechar = sr->tokbuf[0];
    return 1 + parse_double(sr);
}
