#include "parser.hpp"

void parser::load_tokens(std::string fn, std::vector<token>&& t)
{
    this->filename = fn;
    this->tkns = std::move(t);
}

std::vector<diagnostic> const& parser::get_diagnostics()
{
    return this->diagnostics;
}

ast_root& parser::get_ast()
{
    return this->tree;
}

void parser::parse()
{
    this->ok = true;
    //this->tree.root.children.push_back(this->get_stmt());
    // Only gets the top-level stuff
    while(this->ok and !match(tkn_type::eof))
    {
        switch (peek().type)
        {
        case tkn_type::kw_entry:
        {
            skip();
            this->tree.entry = get_entry();
            this->tree.has_entry = true;
            break;
        }
        case tkn_type::kw_import:
        {
            skip();
            this->tree.imports.push_back(get_import());
            break;
        }
        case tkn_type::kw_extern:
        {
            skip();
            this->tree.externs.push_back(get_extern());
            break;
        }
        case tkn_type::kw_func:
        {
            if(is_func_def())
            {
                this->tree.fdefs.push_back(get_func_def());
            }
            else if(is_func_decl())
            {
                this->tree.fdecls.push_back(get_func_decl());
            }
            else
            {
                // Is either a var or an error
                diagnostic e;
                e.l = loc_peek();
                e.msg = "I am no longer a lazy person";
                e.type = diagnostic_type::location_err;
                this->diagnostics.push_back(std::move(e));
                this->ok = false;
            }
            break;
        }
        default:
        {
            this->ok = false;
            diagnostic e;
            e.type = diagnostic_type::location_err;
            e.l = loc_peek();
            e.msg = "Invalid top-level statement";
            this->diagnostics.push_back(std::move(e));
        }
        }
    }
}

bool parser::match(tkn_type v)
{
    // Probably good enough to stop like 99% of bad behaviour
    if (!this->ok)
    {
        return false;
    }

    if (this->peek().type == v)
    {
        cursor++;
        return true;
    }
    return false;
}

bool parser::probe(tkn_type v)
{
    // Probably good enough to stop like 99% of bad behaviour
    if (!this->ok)
    {
        return false;
    }

    return this->peek().type == v;
}

bool parser::expect(tkn_type v)
{
    if (!match(v))
    {
        diagnostic e;
        if (cursor >= this->tkns.size())
        {
            e.type = diagnostic_type::line_err;
            e.l = loc_eof();
            // If line err, doesn't care about start or end, just line
            location l;
            l.filename = this->filename;
            l.line = this->tkns.at(this->tkns.size() - 1).loc.line;
            e.msg = "Unexpected end of file.";
        }
        else
        {
            e.l = loc_peek();
            e.type = diagnostic_type::location_err;
            e.msg = "Unexpected token";
        }
        this->diagnostics.push_back(std::move(e));
    }
    return true;
}

token const& parser::peekb()
{
    if (cursor > 0)
    {
        return this->tkns.at(cursor - 1);
    }
    else
    {
        return this->tkns.back();
    }
}

token const& parser::peek()
{
    if (cursor >= 0 && cursor < this->tkns.size())
    {
        return this->tkns.at(cursor);
    }
    else
    {
        // Assume last token is eof (which it should!)
        return this->tkns.back();
    }
}

token const& parser::peekf()
{
    if (cursor + 1 < this->tkns.size())
    {
        return this->tkns.at(cursor + 1);
    }
    else
    {
        return this->tkns.back();
    }
}

location const& parser::get_loc(token_location loc)
{
    static location const s_invalid("<invalid>");
    if (loc.is_valid())
        return this->tkns.at(loc.loc).loc;
    return s_invalid;
}
