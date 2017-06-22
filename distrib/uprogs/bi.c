// A mini BASIC interpreter
// Statements: LET INPUT PRINT FOR(ENDFOR) IF-ELSE(ENDIF)
// Basic algebraic operation and logical operation
// Usage: bi filename
// A sample:
/*
PRINT "LINE = "
INPUT A
IF A < 10
    FOR B = 1 TO A
        FOR C = 1 TO A - B
            PRINT " "
        ENDFOR
        FOR D = 1 TO 2*B-1
            PRINT "*"
        ENDFOR
        PRINT "\n"
    ENDFOR
ELSE
    PRINT "NO\n"
ENDIF
*/


#include "xv6/types.h"
#include "xv6/stat.h"
#include "xv6/user.h"

// expression
typedef enum {var_null = 0, var_int, var_string} var_type;
typedef char STRING[128];
typedef struct
{
    var_type type; 
    union
    {
        int i;
        STRING s;
    };
} VARIANT;
#define MEMORY_SIZE 26
VARIANT memory[MEMORY_SIZE];
typedef VARIANT OPERAND;

typedef enum 
{  
    oper_lparen = 0,    // ( 
    oper_rparen,        // )  
    oper_plus,          // +  
    oper_minus,         // -  
    oper_multiply,      // *  
    oper_divide,        // /  
    oper_mod,           // %
 
    oper_lt,            // <  
    oper_gt,            // >  
    oper_eq,            // ==  
    oper_ne,            // !=  
    oper_le,            // <=  
    oper_ge,            // >=  
	
    oper_and,           // &  
    oper_or,            // | 
    oper_not,           // !  
      
    oper_min            // bottom  
} operator_type;  
typedef enum {left2right, right2left} associativity;
typedef struct
{  
    int numbers;        // operands   
    int icp;            // priority
    int isp;            // priority 
    associativity ass;   
    operator_type oper; 
} OPERATOR;  
static const OPERATOR operators[] = 
{  
    {2, 17, 1, left2right, oper_lparen},     
    {2, 17, 17, left2right, oper_rparen},    
    {2, 12, 12, left2right, oper_plus},      
    {2, 12, 12, left2right, oper_minus},     
    {2, 13, 13, left2right, oper_multiply},  
    {2, 13, 13, left2right, oper_divide},    
    {2, 13, 13, left2right, oper_mod},       

    {2, 10, 10, left2right, oper_lt},        
    {2, 10, 10, left2right, oper_gt},         
    {2, 9, 9, left2right, oper_eq},          
    {2, 9, 9, left2right, oper_ne},          
    {2, 10, 10, left2right, oper_le},         
    {2, 10, 10, left2right, oper_ge},       
 
    {2, 5, 5, left2right, oper_and},           
    {2, 4, 4, left2right, oper_or},         
    {1, 15, 15, right2left, oper_not},       
 
    {2, 0, 0, right2left, oper_min}         
};  

typedef enum {token_null = 0, token_operand, token_operator} token_type; 
typedef struct
{  
    token_type type;  
    union 
	{  
        OPERAND var;  
        OPERATOR ator;  
    };  
} TOKEN;  
typedef struct tlist 
{  
    TOKEN token;  
    struct tlist *next;  
} TOKEN_LIST, *PTLIST;
char *e;

// grammar
typedef enum 
{
    key_null = 0, 
    key_let,        
    key_input,       
    key_print,        
    key_for,        
    key_endfor,        
    key_if,            
    key_else,       
    key_endif       
}keywords;

struct 
{
    int id;                
    int ln;                
    int target;        
    int step;        
}stack_for[MEMORY_SIZE];  
int top_for = -1;

// load
#define PROGRAM_SIZE 1000  
typedef struct 
{  
    int ln;            
    STRING line;       
} CODE;  
CODE code[PROGRAM_SIZE];
int cp;
int code_size;


// functions
// stack
PTLIST stack_ini();
void stack_push(PTLIST, TOKEN);
TOKEN stack_pop(PTLIST);
TOKEN stack_top(PTLIST);
void stack_free(PTLIST);

// expression
TOKEN next_token();
VARIANT eval(char *);

// grammar
keywords yacc(STRING);
void exec_input(STRING);
void exec_print(STRING);
void exec_for(STRING);
void exec_endfor(STRING);
void exec_if(STRING);
void exec_else(STRING);
void exec_endif(STRING);
void exec_let(STRING);
void exec_null(STRING);

// load
int isspace(char);
int isalpha(char);
int isdigit(char);
int isalnum(char);
char toupper(char);
char* _strstr(char*, char*);
char* _strcat(char*, char*);
int stricmp(char*, char*, int);
int readline(int, char*);
void load_program(char*);

PTLIST 
stack_ini()
{
    PTLIST p;
    p = (PTLIST)malloc(sizeof(TOKEN_LIST));
    if(p == 0)
    {
        return 0;
    }
    else
    {
        p->next = 0;
        return p;
    }
}

void 
stack_push(PTLIST st, TOKEN x)
{
    PTLIST t;
    t = (PTLIST)malloc(sizeof(TOKEN_LIST));
    if(t == 0)
    {
        exit();
    }
    else
    {
        t->token = x;
        t->next = st->next;
        st->next = t;
    }
}

TOKEN 
stack_pop(PTLIST st)
{
    PTLIST t;
    TOKEN to;
    t = st->next;
    to = t->token;
    st->next = t->next;
    free(t);
    return to;
}

TOKEN 
stack_top(PTLIST st)
{
    TOKEN t;
	t.type = token_null;
    if(st->next != 0)
    {
        t = (st->next)->token;
        return t;
    }
    else
    {
        return t;
    }
}

void 
stack_free(PTLIST st)
{
    while(st->next != 0)
    {
        stack_pop(st);
    }
    free(st);
}

// expression

TOKEN 
next_token()
{
    TOKEN token;
    int i;
    STRING str_t;
	token.type = token_null;
    while(*e && isspace(*e))
    {
        e++;
    }
    if(*e == '\0')
    {
        return token;
    }
    // string
    if(*e == '"')
    {
        token.type = token_operand;
        token.var.type = var_string;
        e++;
        for(i = 0; *e && *e != '"'; i++)
        {
            token.var.s[i] = *e;
            e++;
        }
        token.var.s[i] = '\0';
        e++;
    }
    // variable
    else if(isalpha(e[0]) && !isalpha(e[1]))
    {
        token.type = token_operand;
        token.var = memory[*e - 'A'];
        e++;
        if(token.var.type == var_null)
        {
            printf(1, "assignment error\n");
            exit();
        }
    }
    // digit
    else if(isdigit(*e))
    {
        token.type = token_operand;
        token.var.type = var_int;
        for(i = 0; *e && isdigit(*e); i++)
        {
            str_t[i] = *e;
            e++;
        }
        str_t[i] = '\0';
		token.var.i = atoi(str_t);
    }
    // operator
    else
    {
        token.type = token_operator;
        switch(*e)
        {
            case '(':
                token.ator = operators[oper_lparen];
                break;
            case ')':
                token.ator = operators[oper_rparen];
                break;
            case '+':
                token.ator = operators[oper_plus];
                break;
            case '-':
                token.ator = operators[oper_minus];
                break;
            case '*':
                token.ator = operators[oper_multiply];
                break;
            case '/':
                token.ator = operators[oper_divide];
                break;
            case '%':
                token.ator = operators[oper_mod];
                break;
            
            case '<':
                if(*(e + 1) == '=')
                {
                    token.ator = operators[oper_le];
                    e++;
                    break;
                }
                else
                {
                    token.ator = operators[oper_lt];
                    break;
                }
                
            case '>':
                if(*(e + 1) == '=')
                {
                    token.ator = operators[oper_ge];
                    e++;
                    break;
                }
                else
                {
                    token.ator = operators[oper_gt];
                    break;
                }
            case '=':
                if(e[1] == '=')
                {
                    token.ator = operators[oper_eq];
                    e++;
                    break;
                }
                else
                {
                    printf(1, "expression error\n");
                    exit();
                }
            case '!':
                if(*(e + 1) == '=')
                {
                    token.ator = operators[oper_ne];
                    e++;
                    break;
                }
                else
                {
                    token.ator = operators[oper_not];
                    break;
                }
            case '&':
                token.ator = operators[oper_and];
                break;
            case '|':
                token.ator = operators[oper_or];
                break;
            default:
                printf(1, "expression error\n");
        }
        e++;
    }
    
    return token;
}

VARIANT 
eval(char *eval)
{
    PTLIST s_num;        
    PTLIST s_ator;       
    TOKEN token;        
    TOKEN t;           
    TOKEN a, b, c;   
	a.type = token_null;
	b.type = token_null;
	c.type = token_null;
    
    e = eval;
    
    s_num = stack_ini();
    s_ator = stack_ini();
    token.type = token_operator;
    token.ator = operators[oper_min];
    stack_push(s_ator, token);
    
    while(*e)
    {
        token = next_token();
        
        if(token.type == token_operand)
        {
            stack_push(s_num, token);
            continue;
        }
        else if(token.type == token_operator)
        {            
            t = stack_top(s_ator);
            while(token.ator.icp <= t.ator.isp || token.ator.oper == oper_rparen)
            {
                t = stack_pop(s_ator);
                switch(t.ator.oper)
                {
                    case oper_plus :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        if(a.var.type == var_string && b.var.type == var_string)
                        {
                            c.type = token_operand;
                            c.var.type = var_string;
                            _strcat(b.var.s, a.var.s);
                            strcpy(c.var.s, b.var.s);
                            stack_push(s_num, c);
                        }
                        else if(a.var.type == var_int && b.var.type == var_int)
                        {
                            c.var.i = b.var.i + a.var.i;
                            c.type = token_operand;
                            c.var.type = var_int;
                            stack_push(s_num, c);
                        }
                        else
                        {
                            printf(1, "expression error\n");
                        }
                        break;
                    case oper_minus :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i - a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_multiply :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i * a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_divide :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i / a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_lt :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i < a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_gt :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i > a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_eq :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i == a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_le :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i <= a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_ge :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i >= a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_ne :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i != a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_and :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i && a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_or :
                        a = stack_pop(s_num);
                        b = stack_pop(s_num);
                        c.var.i = b.var.i || a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_not :
                        b = stack_pop(s_num);
                        c.var.i = !b.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                        break;
                    case oper_lparen :
                        if(*e == '\0')
                        {
                            goto con;
                        }
                        token = next_token();
                        break;
					default:
						break;
                }
                t = stack_top(s_ator);
            }
            
            stack_push(s_ator, token);
            con:
                continue;
        }
        else
        {
            printf(1, "expression error\n");
        }
    }
        t = stack_top(s_ator);
        while(t.ator.oper != oper_min)
        {
            t = stack_pop(s_ator);
            switch(t.ator.oper)
            {
                case oper_plus :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    if(a.var.type == var_string && b.var.type == var_string)
                    {
                        c.type = token_operand;
                        c.var.type = var_string;
                        _strcat(b.var.s, a.var.s);
                        strcpy(c.var.s, b.var.s);
                        stack_push(s_num, c);
                    }
                    else if(a.var.type == var_int && b.var.type == var_int)
                    {
                        c.var.i = b.var.i + a.var.i;
                        c.type = token_operand;
                        c.var.type = var_int;
                        stack_push(s_num, c);
                    }
                    else
                    {
                        printf(1, "expression error\n");
                    }
                    break;
                case oper_minus :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i - a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_multiply :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i * a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_divide :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i / a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                
                case oper_lt :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i < a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_gt :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i > a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_eq :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i == a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_le :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i <= a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_ge :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i >= a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_ne :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i != a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_and :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i && a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_or :
                    a = stack_pop(s_num);
                    b = stack_pop(s_num);
                    c.var.i = b.var.i || a.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                case oper_not :
                    b = stack_pop(s_num);
                    c.var.i = !b.var.i;
                    c.type = token_operand;
                    c.var.type = var_int;
                    stack_push(s_num, c);
                    break;
                default:
                    break;

            }
            t = stack_top(s_ator);
        }
        c = stack_pop(s_num);
        if(s_num->next == 0)
        {
			stack_free(s_num);
            stack_free(s_ator);
            return c.var;
        }
        else
        {
            printf(1, "expression error\n");
			stack_free(s_num);
            stack_free(s_ator);
			exit();
        }
}

// grammar

keywords 
yacc(STRING line)
{
    if(!stricmp(line, "INPUT ", 6))
    {
        return key_input;
    }
    else if(!stricmp(line, "PRINT ", 6))
    {
        return key_print;
    }
    else if(!stricmp(line, "FOR ", 4))
    {
        return key_for;
    }
    else if(!stricmp(line, "ENDFOR", 6))
    {
        return key_endfor;
    }
    else if(!stricmp(line, "IF ", 3))
    {
        return key_if;
    }
    else if(!stricmp(line, "ELSE", 4))
    {
        return key_else;
    }
    else if(!stricmp(line, "ENDIF", 5))
    {
        return key_endif;
    }
    else if(!stricmp(line, "LET ", 4))
    {
        return key_let;
    }
    else if(*line == '\0')
    {
        return key_null;
    }
    else
    {
        printf(1, "grammar error:yacc\n");
        exit();
    }
}

void 
exec_input(STRING line)
{
    const char *cs = line;
    int n; 
	STRING buf;
    
    cs += 5;
    while(*cs && isspace(*cs))
    {
        cs++;
    }
    if(isalpha(*cs) && *(cs + 1) == '\0')
    {
        n = toupper(*cs) - 'A';
    }
    else
    {
        printf(1, "grammar error:input\n");
        exit();
    }
    gets(buf, 128);
	if(isalpha(buf[0]))
    {
        memory[n].type = var_string;
        strcpy(memory[n].s,buf);
    }
    else
    {
        memory[n].type = var_int;
		memory[n].i = atoi(buf);
    }
}

void 
exec_print(STRING line)
{
    char *s = line;
    VARIANT v;
	char bufc;
	char bufs[2];
    
    s += 6;
    if(*s == '"')
    {
        s++;
        while(*s != '"')
        {
            if(s[0] == '\\' && s[1] == 'n')
            {
                printf(1, "\n");
                s += 2;
            }
            else if(s[0] == '\\' && s[1] == 't')
            {
                printf(1, "\t");
                s += 2;
            }
            else
            {
				bufc = *s;
				bufs[0] = bufc;
				bufs[1] = '\0';
                printf(1, "%s", bufs);
                s++;
            }
        }
    }
    else
    {
        v = eval(s);
        if(v.type == var_string)
        {
            printf(1, "%s", v.s);
        }
        else if(v.type == var_int)
        {
            printf(1, "%d", v.i);
        }
    }
}

void 
exec_for(STRING line)
{
    STRING sl;
    int top;
    char *et = 0;
    char *s;

    strcpy(sl, line);
    s = sl;
    top = top_for + 1;
    s += 4;
    if(top >= MEMORY_SIZE)
    {
        printf(1, "error:for\n");
        exit();
    }

    while(*s && isspace(*s)) s++;
    if(isalpha(*s) && !isalnum(s[1]))
    {
        stack_for[top].id = toupper(*s) - 'A';
        stack_for[top].ln = cp;
        s++;
    }
    else
    {
        printf(1, "grammar error:for\n");
        exit();
    }
 
    while(*s && isspace(*s)) s++;
    if(*s == '=')
    {
        s++;
    }
    else
    {
        printf(1, "grammar error:for\n");
        exit();
    }

    while(*s && isspace(*s)) s++;
    et = _strstr(s, " TO ");
    if(et != 0)
    {
        *et = '\0';
        memory[stack_for[top].id] = eval(s);
        s = et + 4;
    }
    else
    {
        printf(1, "grammar error:for\n");
        exit();
    }
    et = _strstr(s, " STEP ");
    if(et != 0)
    {
        *et = '\0';
        stack_for[top].target = eval(s).i;
        s = et + 5;
        stack_for[top].step = eval(s).i;
        if(stack_for[top].step < 1E-6 && (0-stack_for[top].step) < 1E-6)
        {
            printf(1, "step is too small\n");
            exit();
        }
    }
    else
    {
        stack_for[top].target = eval(s).i;
        stack_for[top].step = 1;
    }
    if( (stack_for[top].step >= 0 &&
         memory[stack_for[top].id].i > stack_for[top].target) ||
         (stack_for[top].step <= 0 &&   
         memory[stack_for[top].id].i < stack_for[top].target) )
    {
        while(yacc(code[cp].line) != key_endfor)
        {
            cp++;
        }
        return;
    }
    else
    {
        top_for++;
    }
}

void 
exec_endfor(STRING line)
{
    if(strcmp(line, "ENDFOR"))
    {
        printf(1, "grammar error:endfor\n");
        exit();
    }
    if(top_for < 0)
    {
        printf(1, "grammar error:endfor\n");
    }
    memory[stack_for[top_for].id].i += stack_for[top_for].step;
    if( stack_for[top_for].step > 0 &&
         memory[stack_for[top_for].id].i > stack_for[top_for].target)
    {
        top_for--;
    }
    else if( stack_for[top_for].step < 0 &&   
            memory[stack_for[top_for].id].i < stack_for[top_for].target ) 
    {  
        top_for--;
    }
    else
    {
        cp = stack_for[top_for].ln;
    }
}

void 
exec_if(STRING line)
{
    char *s = line;
    
    s += 2;
    
    if(eval(s).i)
    {
        return;
    }
    else
    {
        while(yacc(code[cp].line) != key_else && 
            yacc(code[cp].line) != key_endif)
        {
            cp++;
            if(cp > MEMORY_SIZE)
            {
                printf(1, "endif error: line %d\n", cp);
                exit();
            }
        }
    }
}

void 
exec_else(STRING line)
{
    while(yacc(code[cp].line) != key_endif)
    {
        cp++;
        if(cp > MEMORY_SIZE)
        {
            printf(1, "endif error: line %d\n", cp);
        }
    }
}

void 
exec_endif(STRING line)
{
    return;
}

void 
exec_let(STRING line)
{
    char *s = line;
    int n;
    int i;
    
    s += 3;

    while(*s && isspace(*s)) s++;
    if(isalpha(*s) && !isalpha(s[1]))
    {
        n = toupper(*s) - 'A';
        s++;
    }
    else
    {
        printf(1, "grammar error:let\n");
        exit();
    }
    while(*s && isspace(*s)) s++;
    if(*s == '=')
    {
        s++;
    }
    else
    {
        printf(1, "grammar error:let\n");
        exit();
    }
    while(*s && isspace(*s)) s++;
    if(*s == '"')
    {
        memory[n].type = var_string;
        s++;
        for(i = 0; *s && *s != '"'; i++)
        {
            (memory[n].s)[i] = *s;
            s++;
        }
        if(*s != '"')
        {
            printf(1, "grammar error:let\n");
            exit();
        }
    }
    else
    {
        memory[n].type = var_int;
        memory[n] = eval(s);
    }
}

void
exec_null(STRING line)
{  
  	return;
}

// load

int 
isspace(char c)
{
	if(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f')
		return 1;
	else
		return 0;
}

int 
isalpha(char c)
{
	if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return 1;
	else
		return 0;
}

int 
isdigit(char c)
{
	if(c >= '0' && c <= '9')
		return 1;
	else
		return 0;
}

int 
isalnum(char c)
{
	if(isdigit(c) || isalpha(c))
		return 1;
	else
		return 0;
}

char 
toupper(char c)
{
	if(c >= 'A' && c <= 'Z')
		return c;
	else
		return (char)(c + 'A' - 'a');
}

char 
*_strstr(char *s1, char * s2)
{
	int n;  
    if (*s2)                      
    {  
        while(*s1)                 
        {  
            for (n = 0; *(s1+n) == *(s2+n); n++)  
            {  
                if (!*(s2+n+1)) 
                {  
                    return (char*)s1;  
                }  
            }  
            s1++;  
        }  
        return 0;  
    }  
    else  
    {  
        return (char*)s1;  
    }  
}

char* 
_strcat(char* str1, char* str2)  
{   
    char *pt = 
	pt = str1;  
    while(*str1!='\0') str1++;  
    while(*str2!='\0') *str1++ = *str2++;  
    *str1 = '\0';  
    return pt;  
}  

int 
stricmp(char* str1, char* str2, int n)
{
	int i;
    for(i = 0; i < n; i++)
    {
        if(*(str1+i) == *(str2+i))
            i++;
        else
            return 1;
    }
    return 0;
}

int 
readline(int fd, char* buff)  
{  
        int rl;
		char c;
		int count;
		rl = -1;      
        count = 0;  
        if(fd < 0)  
        {  
                printf(1, "open file error\n");  
                exit();  
        }  
        while((rl = read(fd,&c,1))>0)  
        {  
 
                buff[count++] = c;  
                if(c == '\n' || c == '\0')  
                {  
                    break;  
                }  
  
        }  
        buff[count] = '\0';  
        return count;  
}  

void 
load_program(STRING filename)
{
    int fp;
    int bg, ed;
	fp = open(filename, 0);
    if(fp < 0)
    {
        printf(1, "open error\n");
        exit();
    }
    cp = 0;

    while(readline(fp, code[cp].line) > 0)
    {
        for(bg = 0; !isalpha(code[cp].line[bg]) && !isdigit(code[cp].line[bg]); bg++) ;
        ed = (int)strlen(code[cp].line + bg) - 1;
        while(ed >= 0 && isspace(code[cp].line[bg + ed]))
        {
            ed--;
        }
        if(ed >= 0)
        {
            memmove(code[cp].line, code[cp].line + bg, ed + 1);
            code[cp].line[ed + 1] = '\0';
        }
        else
        {
            code[cp].line[0] = '\0';
        }
        code[cp].ln = cp + 1;
        cp++;
        if(cp >= PROGRAM_SIZE)
        {
            printf(1, "error: the file is too large\n");
            exit();
        }
    }
    code_size = cp + 1;
    cp = 0;
}

int 
main(int argc, char *argv[])
{
	void (*key_func[])(STRING)=
	{
		exec_null,
		exec_let,
		exec_input,  
		exec_print,
		exec_for,
		exec_endfor,
		exec_if,
		exec_else,
		exec_endif
	};
	if(argc < 2)
		printf(1, "usage: basic_script_file/n");
	load_program(argv[1]);
    for(cp = 0; cp < code_size; cp++)
    {
        (*key_func[yacc(code[cp].line)])(code[cp].line);
    }
	exit();
}
