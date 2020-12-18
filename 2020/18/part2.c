#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

typedef int64_t val_t;

typedef struct {
    const char *p;
} pstate;

void skipws(pstate *ps)
{
    while (*ps->p == ' ') ps->p++;
}

void next(pstate *ps)
{
    if (ps->p == '\0') return;
    ps->p++;
    skipws(ps);
}

void expect(pstate *ps, char ch)
{
    assert(*ps->p == ch);
    next(ps);
}

val_t expr(pstate *ps);

val_t atom(pstate *ps)
{
    if (*ps->p == '(') {
        next(ps);
        val_t v = expr(ps);
        expect(ps, ')');
        return v;
    } else if (*ps->p >= '0' && *ps->p <= '9') {
        val_t v = 0;
        do {
            v = v * 10 + (*ps->p - '0');
            ps->p++;
        } while (*ps->p >= '0' && *ps->p <= '9');
        skipws(ps);
        return v;
    } else {
        assert(0 && "Syntax error");
        return 0;
    }
}

val_t term(pstate *ps)
{
    val_t v = atom(ps);
    while (*ps->p == '+' || *ps->p == '-') {
        char c = *ps->p;
        next(ps);
        val_t r = atom(ps);
        switch (c) {
        case '+': v += r; break;
        case '-': v -= r; break;
        }
    }
    return v;
}

val_t expr(pstate *ps)
{
    val_t v = term(ps);
    while (*ps->p == '*' || *ps->p == '/') {
        char c = *ps->p;
        next(ps);
        val_t r = term(ps);
        switch (c) {
        case '*': v *= r; break;
        case '/': v /= r; break;
        }
    }
    return v;
}

val_t eval(const char *c)
{
    pstate ps = { c };
    return expr(&ps);
}

val_t main(val_t argc, char **argv)
{
    char buf[1024];
    val_t total = 0;
    while (fgets(buf, sizeof(buf), stdin)) {
        total += eval(buf);
    }
    printf("%" PRId64 "\n", total);
    return 0;
}
