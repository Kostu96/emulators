#include "scanner8008.hpp"

namespace ASM8008 {

    Token Scanner::getToken()
    {
        skipWhitespacesAndComments();
        m_start = m_current;
        if (*m_current == '\0') return makeToken(Token::Type::EndOfSource);

        if (isDigit(*m_current))
        {
            while (isDigit(*m_current) || isAlpha(*m_current)) m_current++;
            return numberToken();
        }

        if (isAlpha(*m_current) || *m_current == '@' || *m_current == '?')
        {
            while (isAlpha(*m_current) || isDigit(*m_current)) m_current++;
            return makeToken(identifierType());
        }

        switch (*m_current++)
        {
        case ':':  return makeToken(Token::Type::Colon);
        case ',':  return makeToken(Token::Type::Comma);
        case '$':  return makeToken(Token::Type::Dollar);
        case '(':  return makeToken(Token::Type::LeftParen);
        case ')':  return makeToken(Token::Type::RightParen);
        case '+':  return makeToken(Token::Type::Plus);
        case '-':  return makeToken(Token::Type::Minus);
        case '*':  return makeToken(Token::Type::Star);
        case '/':  return makeToken(Token::Type::Slash);
        case '\n': return makeToken(Token::Type::EndOfLine);
        }

        return makeToken(Token::Type::Unknown);
    }

    Token Scanner::makeToken(Token::Type type)
    {
        Token token;
        token.type = type;
        token.start = m_start;
        token.length = m_current - m_start;
        return token;
    }

    Token Scanner::numberToken()
    {
        Token token;
        token.type = Token::Type::Number;
        token.start = m_start;
        token.length = m_current - m_start;
        token.value = 0;
        return token;
    }

    Token::Type Scanner::identifierType()
    {
        switch (m_start[0])
        {
        case 'H': return checkMnemonic(1, 2, "LT", Token::Type::MN_HLT);
        }

        return Token::Type::Label;
    }

    Token::Type Scanner::checkMnemonic(int m_start, int length, const char* rest, Token::Type type)
    {
        if (m_current - this->m_start == m_start + length &&
            memcmp(this->m_start + m_start, rest, length) == 0) return type;
        return Token::Type::Label;
    }

    void Scanner::skipWhitespacesAndComments()
    {
        while (true)
            switch (*m_current)
            {
            case ' ':
            case '\r':
            case '\t':
                m_current++;
                break;
            case ';':
                while (*m_current != '\n' && *m_current != '\0')
                    m_current++;
                break;
            default:
                return;
            }
    }

    bool Scanner::isDigit(char c)
    {
        return (c >= '0' && c <= '9');
    }

    bool Scanner::isAlpha(char c)
    {
        return (c >= 'A' && c <= 'Z');
    }

}
