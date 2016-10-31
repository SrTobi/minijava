/*

stack s (prec, expr, operator) = [];


parse_exprs()
{
    min_prec = 0
    t = first token
outer:
    stack preop-stack : unop = [];
    while(t is pre-op)
    {
        preop-stack.push(t.operator)
        t <- next token advance
    }

    result = parse_primary()
    t <- next token advance

    while(t is post-op)
    {
        result = t.operator(rusult)
    }

    while(!preop-stack.empty)
    {
        preop <- preop-stack.pop
        result = preop(result)
    }


inner:
    if (t is bin-op) && (t.prec >= min_prec)
    {
        prec = t.prec
        if (t.assoc == left)
        {
            ++prec
        }

        s.push(min_prec, result, t.operator)

        min_prec = prec

        t <- next token advance
        goto outer;
    }

    if(!s.empty)
    {
        (min_prec, lhs, op) <- s.pop
        result <- op(lhs, result)
        goto inner
    }

    return result
}

*/
