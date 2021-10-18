#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch)   ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1to9(ch)   ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/* judge null/true/false */
static int lept_parse_literal(lept_context* c, lept_value* v, const char* cur, int size) {
    int i;
    EXPECT(c, cur[0]);
    for(i=1;i<size;i++)
    {
        if(c->json[i-1]!=cur[i])
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += size-1;
    if(cur[0]=='n')  v->type = LEPT_NULL;
    else if(cur[0] == 't')  v->type = LEPT_TRUE;
    else if(cur[0] == 'f')  v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* ptr=c->json;
    if(*ptr == '-') ptr++;
    if(*ptr == '0') ptr++;
    else
    {
        if(!ISDIGIT1to9(*ptr))  return LEPT_PARSE_INVALID_VALUE;
        for (ptr++; ISDIGIT(*ptr); ptr++){}
    }
    if(*ptr == '.')
    {
        ptr++;
        if(!ISDIGIT(*ptr))  return LEPT_PARSE_INVALID_VALUE;
        for (ptr++; ISDIGIT(*ptr); ptr++){}
    }
    if(*ptr == 'e' || *ptr == 'E')
    {
        ptr++;
        if(*ptr == '+' || *ptr == '-') ptr++;
        if(!ISDIGIT(*ptr))  return LEPT_PARSE_INVALID_VALUE;
        for (ptr++; ISDIGIT(*ptr); ptr++){}
    }
    errno=0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = ptr;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_literal(c, v, "null", 4);
        case 't':  return lept_parse_literal(c, v, "true", 4);
        case 'f':  return lept_parse_literal(c, v, "false", 5);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
        default:   return lept_parse_number(c, v);
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
