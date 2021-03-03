#include "parser.hpp"

bool parser::is_params(bool reset)
{
    bool result = false;
    int op = this->cursor;

    if(match(tkn_type::lparen))
    {
        while(is_var_decl(false) && match(tkn_type::comma)){}
        if(match(tkn_type::rparen))
        {
            result = true;
        }
    }

    if(reset)
    {
        this->cursor = op;
    }
    return result;
}

bool parser::is_func_decl(bool reset)
{
    bool result = false;
    int op = this->cursor;

    if(match(tkn_type::kw_func))
    {
        if(is_datatype(false) && is_identifier(false))
        {
            if(is_params(false))
            {
                result = true;
            }
        }
    }

    if(reset){
        this->cursor = op;
    }

    return result;
}

bool parser::is_func_def(bool reset)
{
    bool result = false;
    int op = this->cursor;

    if(is_func_decl(false))
    {
        if(match(tkn_type::lbrace))
        {
            result = true;
        }
    }

    if(reset){
        this->cursor = op;
    }

    return result;
}

bool parser::is_func_call(bool reset)
{
    bool result = false;
    int og = this->cursor;

    if (is_identifier(false))
    {
        if(match(tkn_type::lparen))
        {
            result = true;

            // Just skips until lparen
            int depth = 1;
            while(depth != 0)
            {
                if(peek().type == tkn_type::lparen){
                    depth++;
                }
                if(peek().type == tkn_type::rparen){
                    depth--;
                }
                this->cursor++;
            }
        }
        else
        {
            result = false;
            this->cursor = og;
        }
    }

    if(reset)
    {
        this->cursor = og;
    }
    return result;
}


parameters parser::get_parameters()
{
    parameters node;
    expect(tkn_type::lparen);
    
    do
    {
        node.body.push_back(get_decl_stmt());
    }
    while(match(tkn_type::comma));

    expect(tkn_type::rparen);
    return node;
}

func_decl_stmt parser::get_func_decl()
{
    func_decl_stmt node;

    // Inherited stuff
    node.type = stmt_type::fdecl;
    node.line = peek().loc.line;
    
    expect(tkn_type::kw_func);
    if(!this->ok){return node;}
    node.data_type = get_datatype();
    if(!this->ok){return node;}
    node.ident = get_identifier();
    if(!this->ok){return node;}
    node.params = get_parameters();

    return node;
}

func_def_stmt parser::get_func_def()
{
    func_def_stmt node;

    // Inherited stuff
    node.type = stmt_type::fdef;
    node.line = peek().loc.line;

    expect(tkn_type::kw_func);
    if(!this->ok){return node;}
    node.data_type = get_datatype();
    if(!this->ok){return node;}
    node.ident = get_identifier();
    if(!this->ok){return node;}
    node.params = get_parameters();
    if(!this->ok){return node;}
    node.body = get_compound_stmt();

    return node;
}


arguments parser::get_arguments()
{
    arguments node;
    expect(tkn_type::lparen);
    
    while (!match(tkn_type::rparen) && !match(tkn_type::eof) && this->ok)
    {
        match(tkn_type::comma);
        node.body.push_back(get_expr());
    }
    
    return node;
}

func_call_stmt parser::get_fcall()
{
    func_call_stmt node;
    node.type = stmt_type::fcall;
    node.ident = get_identifier();
    node.args = get_arguments();
    return node;
}
